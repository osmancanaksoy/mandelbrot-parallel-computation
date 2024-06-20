#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QMouseEvent>


#include "data.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void computeMandelBrotSequential();
    void computeMandelBrotParallel();    
    void wheelEvent(QWheelEvent *event);

private slots:
    void on_calculateButton_clicked();
    void on_clearButton_clicked();

    void on_showButton_clicked();

    void on_saveButton_clicked();

    void on_actionData_triggered();

    void on_action1_triggered();

    void on_action2_triggered();

    void on_action3_triggered();

    void on_action4_triggered();

    void on_action5_triggered();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene;

    int maxThreads;

    int IMAGE_WIDTH;
    int IMAGE_HEIGHT;
    int MAX_ITERATIONS;
    int POWER_Z;
    double MIN_REAL;
    double MAX_REAL;
    double MIN_IMAGINARY;
    double MAX_IMAGINARY;

    double sequentialTime = 0.0;
    double parallelTime = 0.0;

    QVector<double> speedups;
    QVector<double> efficiencies;

    QVector<Data> data;


};
#endif // MAINWINDOW_H
