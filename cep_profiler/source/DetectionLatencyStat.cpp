/*
 * DetectionLatency.cpp
 *
 * Created on: 20.12.2016
 *      Author: sload
 */

#include "../header/DetectionLatencyStat.hpp"

namespace profiler
{



DetectionLatencyStat::DetectionLatencyStat(string outputFileName)
{
    this->outputFileName=outputFileName;
}
void DetectionLatencyStat::addNewEvent(unsigned long eventId, unsigned long detectionLatency)
{
    this->eventLatencyMap[eventId]= detectionLatency;
}

const map<unsigned long, unsigned long> &DetectionLatencyStat::getEventLatencyMap() const
{
    return eventLatencyMap;
}

void DetectionLatencyStat::setEventLatencyMap(const map<unsigned long, unsigned long> &value)
{
    eventLatencyMap = value;
}

void DetectionLatencyStat::printToFile()
{
    if (this->outputFileName == "")
        this->outputFileName = "detectionLatency.txt";

    ofstream outputFile;
    outputFile.open(this->outputFileName, ios::out);

    // to format the string
    stringstream strm;

    strm << std::left << std::setw(25) << "Event Id, ";
    strm << std::left << std::setw(25) << "Detection Latency" << endl;

    for (auto it = this->eventLatencyMap.begin(); it != this->eventLatencyMap.end(); it++)
    {
        strm << std::left << std::setw(25) << it->first << ", ";
        strm << std::left << std::setw(25) << it->second << endl;
    }

    outputFile << strm.str();

    outputFile.close();
}


}
