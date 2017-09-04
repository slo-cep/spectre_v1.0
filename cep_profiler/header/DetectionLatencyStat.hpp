/*
 * DetectionLatency.hpp
 *
 * Created on: 20.12.2016
 *      Author: sload
 */

#ifndef DETECTIONLATENCYSTAT_HPP
#define DETECTIONLATENCYSTAT_HPP

#include <fstream>
#include <iomanip> // format string
#include <iostream>
#include <sstream> // stringstream
#include <string>
#include<memory>

#include <map>

using namespace std;

namespace profiler
{
class DetectionLatencyStat
{
public:
    DetectionLatencyStat(string outputFileName);

    void addNewEvent(unsigned long eventId,unsigned long detectionLatency);

    const map<unsigned long, unsigned long>& getEventLatencyMap() const;
    void setEventLatencyMap(const map<unsigned long, unsigned long> &value);

    void printToFile();

private:
    // event Id, detection latency
    map<unsigned long, unsigned long> eventLatencyMap;

    string outputFileName;

};
}

#endif // DETECTIONLATENCYSTAT_HPP
