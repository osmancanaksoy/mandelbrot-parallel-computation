#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <omp.h>
#include <complex.h>
#include <fstream>

#include <QPixmap>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    maxThreads = omp_get_max_threads();
    ui->threadValue->setText(QString::number(maxThreads));

    ui->clearButton->setVisible(false);
    ui->showButton->setVisible(false);
    ui->saveButton->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}

void MainWindow::computeMandelBrotSequential()
{
    QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32);

    QElapsedTimer sequentialTimer;
    sequentialTimer.start();

    for (int y = 0; y < IMAGE_HEIGHT; y++)
    {
        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            long double real = MIN_REAL + (MAX_REAL - MIN_REAL) * x / IMAGE_WIDTH;
            long double imaginary = MIN_IMAGINARY + (MAX_IMAGINARY - MIN_IMAGINARY) * y / IMAGE_HEIGHT;

            std::complex<double> z(0, 0);
            std::complex<double> c(real, imaginary);
            int iterations = 0;

            while (std::abs(z) <= 2 && iterations < MAX_ITERATIONS)
            {
                z = z * z + c;
                iterations++;
            }

            if (iterations < MAX_ITERATIONS)
            {
                long double hue = 360.0 * iterations / MAX_ITERATIONS;
                QColor color = QColor::fromHsvF(hue / 360.0, 1.0, 1.0);

                image.setPixelColor(x, y, color);
            }
            else
            {
                image.setPixelColor(x, y, Qt::black);
            }
        }
    }

    sequentialTime = sequentialTimer.elapsed() / 1000.0;
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image));
}

void MainWindow::computeMandelBrotParallel()
{
    QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32);

    int maxThreads = omp_get_max_threads();

    for(int numThreads = 1; numThreads <= maxThreads; numThreads++) {
        omp_set_num_threads(numThreads);

        double startTime = omp_get_wtime();

#pragma omp parallel for schedule(dynamic)
        for (int y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (int x = 0; x < IMAGE_WIDTH; x++)
            {
                long double real = MIN_REAL + (MAX_REAL - MIN_REAL) * x / IMAGE_WIDTH;
                long double imaginary = MIN_IMAGINARY + (MAX_IMAGINARY - MIN_IMAGINARY) * y / IMAGE_HEIGHT;

                std::complex<double> z(0, 0);
                std::complex<double> c(real, imaginary);
                int iterations = 0;
                while (std::abs(z) < 2 && iterations < MAX_ITERATIONS)
                {
                    z = pow(z, POWER_Z) + c;
                    iterations++;
                }

                if (iterations < MAX_ITERATIONS)
                {
                    long double hue = 360.0 * iterations / MAX_ITERATIONS;
                    QColor color = QColor::fromHsvF(hue / 360.0, 1.0, 1.0);

#pragma omp critical
                    {
                        image.setPixelColor(x, y, color);
                    }
                }
                else
                {
#pragma omp critical
                    {
                        image.setPixelColor(x, y, Qt::black);
                    }
                }
            }
        }

        double endTime = omp_get_wtime();
        parallelTime = endTime - startTime;

        double speedup = sequentialTime / parallelTime;
        speedups.push_back(speedup);

        double efficiency = speedup / numThreads;
        efficiencies.push_back(efficiency);
    }

    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image));
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(ui->zoomCheckBox->isChecked()) {
        const QPointF numDegrees = event->angleDelta() / 8;
        const QPointF numSteps = numDegrees / 15;
        qreal factor = qPow(1.2, -numSteps.y());
        qreal oldWidth = MAX_REAL - MIN_REAL;
        qreal oldHeight = MAX_IMAGINARY - MIN_IMAGINARY;

        qreal newWidth = oldWidth * factor;
        qreal newHeight = oldHeight * factor;

        qreal mouseX = MIN_REAL + (MAX_REAL - MIN_REAL) * event->position().x() / IMAGE_WIDTH;
        qreal mouseY = MIN_IMAGINARY + (MAX_IMAGINARY - MIN_IMAGINARY) * event->position().y() / IMAGE_HEIGHT;

        qreal newMinReal = mouseX - (mouseX - MIN_REAL) * (newWidth / oldWidth);
        qreal newMaxReal = mouseX + (MAX_REAL - mouseX) * (newWidth / oldWidth);
        qreal newMinImaginary = mouseY - (mouseY - MIN_IMAGINARY) * (newHeight / oldHeight);
        qreal newMaxImaginary = mouseY + (MAX_IMAGINARY - mouseY) * (newHeight / oldHeight);

        MIN_REAL = newMinReal;
        MAX_REAL = newMaxReal;
        MIN_IMAGINARY = newMinImaginary;
        MAX_IMAGINARY = newMaxImaginary;

        computeMandelBrotParallel();
        event->accept();
    }
}

void MainWindow::on_calculateButton_clicked()
{
    IMAGE_WIDTH = ui->widthValue->text().toInt();
    IMAGE_HEIGHT = ui->heightValue->text().toInt();
    MAX_ITERATIONS = ui->iterationValue->text().toInt();
    POWER_Z = ui->powerZValue->text().toInt();

    MIN_REAL = ui->minRealValue->text().toDouble();
    MAX_REAL = ui->maxRealValue->text().toDouble();

    MIN_IMAGINARY = ui->minImagValue->text().toDouble();
    MAX_IMAGINARY = ui->maxImagValue->text().toDouble();
    computeMandelBrotSequential();
    computeMandelBrotParallel();

    ui->seqValue->setText(QString::number(sequentialTime) + " sn");
    ui->parValue->setText(QString::number(parallelTime) + " sn");

    ui->clearButton->setVisible(true);
    ui->showButton->setVisible(true);
    ui->saveButton->setVisible(true);
}



void MainWindow::on_clearButton_clicked()
{
    ui->graphicsView->scene()->clear();
    ui->clearButton->setVisible(false);
    ui->showButton->setVisible(false);
    ui->saveButton->setVisible(false);

    ui->seqValue->clear();
    ui->parValue->clear();
    speedups.clear();
    efficiencies.clear();
}


void MainWindow::on_showButton_clicked()
{
    QLineSeries *speedupSeries = new QLineSeries();
    for (int i = 1; i <= speedups.size(); i++) {
        speedupSeries->append(i, speedups[i -1]);
    }

    QChart *chart = new QChart();
    chart->addSeries(speedupSeries);
    chart->setTitle("Speedup");

    QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%d"); // Tam sayı formatı
    axisX->setTitleText("Number of Threads");
    axisX->setRange(0, maxThreads);
    chart->addAxis(axisX, Qt::AlignBottom);
    speedupSeries->attachAxis(axisX);

    double maxSpeedup = *std::max_element(speedups.constBegin(), speedups.constEnd());
    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("Speedup");
    axisY->setRange(0, maxSpeedup);
    chart->addAxis(axisY, Qt::AlignLeft);
    speedupSeries->attachAxis(axisY);

    chart->setAnimationOptions(QChart::AllAnimations);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QLineSeries *efficiencySeries = new QLineSeries();
    for (int i = 1; i <= efficiencies.size(); i++) {
        efficiencySeries->append(i, efficiencies[i - 1]);
    }

    QChart *chart2 = new QChart();
    efficiencySeries->setColor(QColorConstants::Svg::orange);
    chart2->addSeries(efficiencySeries);
    chart2->setTitle("Efficiency");

    QValueAxis *axisX2 = new QValueAxis;
    axisX2->setLabelFormat("%d"); // Tam sayı formatı
    axisX2->setTitleText("Number of Threads");
    axisX2->setRange(0, maxThreads);
    chart2->addAxis(axisX2, Qt::AlignBottom);
    efficiencySeries->attachAxis(axisX2);

    double maxEfficiency = *std::max_element(efficiencies.constBegin(), efficiencies.constEnd());
    QValueAxis *axisY2 = new QValueAxis;
    axisY2->setLabelFormat("%.2f");
    axisY2->setTitleText("Efficiency");
    axisY2->setRange(0, maxEfficiency);
    chart2->addAxis(axisY2, Qt::AlignLeft);
    efficiencySeries->attachAxis(axisY2);

    chart2->setAnimationOptions(QChart::AllAnimations);

    QChartView *chartView2 = new QChartView(chart2);
    chartView2->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(chartView);
    layout->addWidget(chartView2);

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    QMainWindow *window = new QMainWindow();
    window->setCentralWidget(widget);
    window->setWindowTitle("Results");
    setWindowIcon(QIcon(":/icon.png"));
    window->resize(1600, 600);
    window->show();
}


void MainWindow::on_saveButton_clicked()
{
    std::ofstream outFile("data.txt", std::ios_base::out | std::ios_base::app);

    for(int i = 0; i < maxThreads; i++) {
        outFile << speedups[i] << " ";
    }
    outFile << std::endl;
    for(int i = 0; i < maxThreads; i++) {
        outFile << efficiencies[i] << " ";
    }
    outFile << std::endl;

    outFile.close();

    QMessageBox message(QMessageBox::Information, "Information", "Data saved...",QMessageBox::Ok);
    message.exec();
}


void MainWindow::on_actionData_triggered()
{
    QFile file("data.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Dosya açılamadı";
    }

    // Dosyanın satır sayısını al
    QTextStream countStream(&file);
    int lineCount = 0;
    while (!countStream.atEnd()) {
        countStream.readLine();
        lineCount++;
    }

    // Data sınıfından nesneler oluştur
    int dataCount = std::ceil(lineCount / 2.0); // Dosyanın yarısı kadar nesne oluştur
    QVector<Data> dataList(dataCount);

    // Dosyayı başa al ve verileri oku
    file.seek(0);

    QTextStream in(&file);
    QString satir1, satir2;
    int currentDataIndex = 0;
    while (!in.atEnd()) {
        // Birinci satırı oku
        if (!in.atEnd()) {
            satir1 = in.readLine().trimmed(); // Trim kullanarak sondaki boşlukları kaldır
            QStringList parcalar1 = satir1.split(" ");
            for (const QString& parca : parcalar1) {
                if (!parca.isEmpty()) { // Boş string değilse ekle
                    dataList[currentDataIndex].addSpeedup(parca.toDouble());
                }
            }
        } else {
            break;
        }

        // İkinci satırı oku
        if (!in.atEnd()) {
            satir2 = in.readLine().trimmed(); // Trim kullanarak sondaki boşlukları kaldır
            QStringList parcalar2 = satir2.split(" ");
            for (const QString& parca : parcalar2) {
                if (!parca.isEmpty()) { // Boş string değilse ekle
                    dataList[currentDataIndex].addEfficiency(parca.toDouble());
                }
            }
        } else {
            break;
        }

        // Bir sonraki Data nesnesine geç
        currentDataIndex++;
        if (currentDataIndex >= dataCount) {
            break; // Dosyanın yarısına ulaştık, döngüyü bitir
        }
    }

    // Dosyayı kapat
    file.close();

    data = dataList;

    QChart *chart = new QChart();

    // Her bir Data nesnesi için ayrı bir doğru oluştur
    for (int i = 0; i < data.size(); ++i) {
        QLineSeries *series = new QLineSeries();
        series->setName(QString("Data %1").arg(i+1)); // Doğrunun adını ayarla

        // Her bir Data nesnesinin speedup verilerini doğruya ekle
        QVector<double> speedup = data[i].getSpeedup();
        for (int j = 0; j < speedup.size(); ++j) {
            series->append(j, speedup[j]);
        }

        // Doğruyu grafiğe ekle
        chart->addSeries(series);
    }

    // Eksenleri ayarla
    QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%d"); // Tam sayı formatı
    axisX->setTitleText("Number of Threads");
    axisX->setRange(0, maxThreads);
    chart->addAxis(axisX, Qt::AlignBottom);


    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("Speedup");
    chart->addAxis(axisY, Qt::AlignLeft);

    // Doğruları eksenlere bağla
    QList<QAbstractSeries *> seriesList = chart->series();
    for (QAbstractSeries *series : seriesList) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    // Grafik görüntüleyici oluştur
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QChart *chart2 = new QChart();

    // Her bir Data nesnesi için ayrı bir doğru oluştur
    for (int i = 0; i < data.size(); ++i) {
        QLineSeries *series2 = new QLineSeries();
        series2->setName(QString("Data %1").arg(i+1)); // Doğrunun adını ayarla

        // Her bir Data nesnesinin speedup verilerini doğruya ekle
        QVector<double> efficiency = data[i].getEfficiency();
        for (int j = 0; j < efficiency.size(); ++j) {
            series2->append(j, efficiency[j]);
        }

        // Doğruyu grafiğe ekle
        chart2->addSeries(series2);
    }

    // Eksenleri ayarla
    QValueAxis *axisX2 = new QValueAxis;
    axisX2->setLabelFormat("%d"); // Tam sayı formatı
    axisX2->setTitleText("Number of Threads");
    axisX2->setRange(0, maxThreads);
    chart2->addAxis(axisX2, Qt::AlignBottom);

    QValueAxis *axisY2 = new QValueAxis;
    axisY2->setLabelFormat("%.2f");
    axisY2->setTitleText("Efficiency");
    chart2->addAxis(axisY2, Qt::AlignLeft);

    // Doğruları eksenlere bağla
    QList<QAbstractSeries *> seriesList2 = chart2->series();
    for (QAbstractSeries *series2 : seriesList2) {
        series2->attachAxis(axisX2);
        series2->attachAxis(axisY2);
    }

    // Grafik görüntüleyici oluştur
    QChartView *chartView2 = new QChartView(chart2);
    chartView2->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(chartView);
    layout->addWidget(chartView2);

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    QMainWindow *window = new QMainWindow();
    window->setCentralWidget(widget);
    window->setWindowTitle("Results");
    window->setWindowIcon(QIcon(":/icon.png"));
    window->resize(1600, 600);
    window->show();
}


void MainWindow::on_action1_triggered()
{
    ui->widthValue->setText("800");
    ui->heightValue->setText("600");
    ui->iterationValue->setText("1000");
    ui->powerZValue->setText("2");

    ui->minRealValue->setText("-2.0");
    ui->maxRealValue->setText("1.0");

    ui->minImagValue->setText("-1.5");
    ui->maxImagValue->setText("1.5");

    QMessageBox message(QMessageBox::Information, "Information", "Fractal - 1 values ​​have been adjusted... ",QMessageBox::Ok);
    message.exec();
}


void MainWindow::on_action2_triggered()
{
    ui->widthValue->setText("800");
    ui->heightValue->setText("600");
    ui->iterationValue->setText("2000");
    ui->powerZValue->setText("3");

    ui->minRealValue->setText("-2.0");
    ui->maxRealValue->setText("1.0");

    ui->minImagValue->setText("-1.5");
    ui->maxImagValue->setText("1.5");

    QMessageBox message(QMessageBox::Information, "Information", "Fractal - 2 values ​​have been adjusted... ",QMessageBox::Ok);
    message.exec();
}


void MainWindow::on_action3_triggered()
{
    ui->widthValue->setText("1024");
    ui->heightValue->setText("768");
    ui->iterationValue->setText("1000");
    ui->powerZValue->setText("4");

    ui->minRealValue->setText("-2.0");
    ui->maxRealValue->setText("1.0");

    ui->minImagValue->setText("-1.5");
    ui->maxImagValue->setText("1.5");

    QMessageBox message(QMessageBox::Information, "Information", "Fractal - 3 values ​​have been adjusted... ",QMessageBox::Ok);
    message.exec();
}


void MainWindow::on_action4_triggered()
{
    ui->widthValue->setText("1024");
    ui->heightValue->setText("768");
    ui->iterationValue->setText("2000");
    ui->powerZValue->setText("5");

    ui->minRealValue->setText("-2.0");
    ui->maxRealValue->setText("1.0");

    ui->minImagValue->setText("-1.5");
    ui->maxImagValue->setText("1.5");

    QMessageBox message(QMessageBox::Information, "Information", "Fractal - 4 values ​​have been adjusted... ",QMessageBox::Ok);
    message.exec();
}


void MainWindow::on_action5_triggered()
{
    ui->widthValue->setText("1920");
    ui->heightValue->setText("1080");
    ui->iterationValue->setText("1000");
    ui->powerZValue->setText("6");

    ui->minRealValue->setText("-2.0");
    ui->maxRealValue->setText("1.0");

    ui->minImagValue->setText("-1.5");
    ui->maxImagValue->setText("1.5");

    QMessageBox message(QMessageBox::Information, "Information", "Fractal - 5 values ​​have been adjusted... ",QMessageBox::Ok);
    message.exec();
}

