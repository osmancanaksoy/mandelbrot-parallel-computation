#ifndef DATA_H
#define DATA_H

#include <QVector>


class Data
{
public:
    Data();
    void addSpeedup(double value);
    void addEfficiency(double value);

    QVector<double> getSpeedup();
    QVector<double> getEfficiency();
private:
    QVector<double> speedup;
    QVector<double> efficiency;

};

#endif // DATA_H
