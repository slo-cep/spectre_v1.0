/*
 * ThroughputStat.cpp
 *
 * Created on: 19.12.2016
 *      Author: sload
 */
#include "../header/ThroughputStat.hpp"

namespace profiler
{

ThroughputStat::ThroughputStat(string outputFileName) { this->outputFileName = outputFileName; }

void ThroughputStat::set(unsigned long startTime, unsigned long endTime, unsigned long numberOfEvents)
{
    this->startTime = startTime;
    this->endTime = endTime;
    this->numberOfEvents = numberOfEvents;
}

unsigned long ThroughputStat::getStartTime() const { return startTime; }

void ThroughputStat::setStartTime(unsigned long value) { startTime = value; }

unsigned long ThroughputStat::getEndTime() const { return endTime; }

void ThroughputStat::setEndTime(unsigned long value) { endTime = value; }

unsigned long ThroughputStat::getNumberOfEvents() const { return numberOfEvents; }

void ThroughputStat::setNumberOfEvents(unsigned long value) { numberOfEvents = value; }

void ThroughputStat::printToFile()
{
    if (this->outputFileName == "")
        this->outputFileName = "throughput.txt";

    ofstream outputFile;
    outputFile.open(this->outputFileName, ios::out);

    // to format the string
    stringstream strm;

    strm << std::left << std::setw(25) << "Start time, ";
    strm << std::left << std::setw(25) << "End time, ";
    strm << std::left << std::setw(25) << "Number of events" << endl;

    strm << std::left << std::setw(25) << this->startTime << ", ";
    strm << std::left << std::setw(25) << this->endTime << ", ";
    strm << std::left << std::setw(25) << this->numberOfEvents << endl;

    outputFile << strm.str();

    outputFile.close();
}
}
