#include "data.h"

Data::Data() {}

void Data::addSpeedup(double value)
{
    speedup.push_back(value);
}

void Data::addEfficiency(double value)
{
    efficiency.push_back(value);
}

QVector<double> Data::getSpeedup()
{
    return speedup;
}

QVector<double> Data::getEfficiency()
{
    return efficiency;
}
