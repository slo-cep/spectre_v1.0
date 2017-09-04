/*
 * WorkerThreadStat.cpp
 *
 * Created on: 19.12.2016
 *      Author: sload
 */

#include "../header/WorkerThreadStat.hpp"

namespace profiler
{

WorkerThreadStat::WorkerThreadStat(string outputFileName) { this->outputFileName = outputFileName; }

void WorkerThreadStat::setNewWorker(unsigned long workerId, unsigned long eventsPerWorker)
{
    lock_guard<mutex> lk(this->mtx);
    this->eventsProcessedPerWorker[workerId] = eventsPerWorker;
}

const map<unsigned long, unsigned long> &WorkerThreadStat::getEventsProcessedPerWorker() const
{
    return eventsProcessedPerWorker;
}

void WorkerThreadStat::setEventsProcessedPerWorker(const map<unsigned long, unsigned long> &value)
{
    eventsProcessedPerWorker = value;
}

void WorkerThreadStat::printToFile()
{
    lock_guard<mutex> lk(this->mtx);

    if (this->outputFileName == "")
        this->outputFileName = "eventsProcessedPerWorkers.txt";

    ofstream outputFile;
    outputFile.open(this->outputFileName, ios::out);

    // to format the string
    stringstream strm;

    strm << std::left << std::setw(25) << "Worker Id, ";
    strm << std::left << std::setw(25) << "Number of events" << endl;

    for (auto it = this->eventsProcessedPerWorker.begin(); it != this->eventsProcessedPerWorker.end(); it++)
    {
        strm << std::left << std::setw(25) << it->first << ", ";
        strm << std::left << std::setw(25) << it->second << endl;
    }

    outputFile << strm.str();

    outputFile.close();
}
}
