/*
 * WorkerThreadStat.hpp
 *
 * Created on: 19.12.2016
 *      Author: sload
 */

#include <fstream>
#include <iomanip> // format string
#include <iostream>
#include <sstream> // stringstream
#include <string>
#include <mutex>

#include <map>

using namespace std;
namespace profiler
{
class WorkerThreadStat
{
public:
    WorkerThreadStat(string outputFileName);

    void setNewWorker(unsigned long workerId, unsigned long eventsPerWorker);

    const map<unsigned long, unsigned long>& getEventsProcessedPerWorker() const;
    void setEventsProcessedPerWorker(const map<unsigned long, unsigned long> &value);

    void printToFile();

private:
    // worker Id, #events
    map<unsigned long, unsigned long> eventsProcessedPerWorker;

    string outputFileName;

    mutable mutex mtx;
};
}
