/*
 * TimeMeasurement.hpp
 *
 * Created on: 10.10.2016
 *      Author: sload
 */

#ifndef TIMEMEASUREMENT_HPP
#define TIMEMEASUREMENT_HPP

#include "DetectionLatencyStat.hpp"
#include "ThroughputStat.hpp"
#include "WorkerThreadStat.hpp"
#include "PathManagerReschedulingFrequency.hpp"

#include <algorithm> //count
#include <fstream>
#include <iomanip> // format string
#include <iostream>
#include <iterator> //istream_iterator
#include <map>
#include <memory>
#include <mutex>
#include <sstream> // stringstream
#include <string>
#include <vector>

using namespace std;

namespace profiler
{
class Measurements
{
public:
    Measurements();

    Measurements(string latencyDetectionFile, string eventsProcessedPerWorkersFile, string throughputFile, string rescheduligFrequencyFile);

    const shared_ptr<ThroughputStat> &getThroughputStat() const;

    const shared_ptr<WorkerThreadStat> &getWorkerThreadStat() const;

    const shared_ptr<DetectionLatencyStat> &getDetectionLatencyStat() const;

    const shared_ptr<PathManagerReschedulingFrequency>& getPathManagerReschedulingFrequency() const;


    void printToFiles();

    //    void print(string fileName);

    ~Measurements();



private:
    shared_ptr<ThroughputStat> throughputStat;
    shared_ptr<WorkerThreadStat> workerThreadStat;
    shared_ptr<DetectionLatencyStat> detectionLatencyStat;
    shared_ptr<PathManagerReschedulingFrequency> pathManagerReschedulingFrequency;
};
}

#endif // TIMEMEASUREMENT_HPP
