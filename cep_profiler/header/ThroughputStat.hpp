/*
 * ThroughputStat.hpp
 *
 * Created on: 19.12.2016
 *      Author: sload
 */

#ifndef THROUGHPUTSTAT_HPP
#define THROUGHPUTSTAT_HPP

#include <iostream>
#include <fstream>
#include <iomanip> // format string
#include <sstream> // stringstream
#include <string>

using namespace std;

namespace profiler
{
class ThroughputStat
{
public:
    ThroughputStat(string outputFileName);

    void set(unsigned long startTime, unsigned long endTime, unsigned long numberOfEvents);


    unsigned long getStartTime() const;
    void setStartTime(unsigned long value);

    unsigned long getEndTime() const;
    void setEndTime(unsigned long value);

    unsigned long getNumberOfEvents() const;
    void setNumberOfEvents(unsigned long value);

    void printToFile();

private:
    unsigned long startTime;
    unsigned long endTime;
    unsigned long numberOfEvents;

    string outputFileName;
};
}

#endif // THROUGHPUTSTAT_HPP
