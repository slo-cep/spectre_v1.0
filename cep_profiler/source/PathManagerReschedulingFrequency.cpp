/*
 * PathManagerReschedulingFrequency.cpp
 *
 * Created on: 16.01.2017
 *      Author: sload
 */

#include "../header/PathManagerReschedulingFrequency.hpp"

namespace profiler
{

PathManagerReschedulingFrequency::PathManagerReschedulingFrequency(string outputFileName)
    : outputFileName(outputFileName)
{
}

void PathManagerReschedulingFrequency::set(unsigned long startTime, unsigned long endTime, unsigned long numberOfCycles)
{
    this->startTime = startTime;
    this->endTime = endTime;
    this->numberOfCycles = numberOfCycles;
}

void PathManagerReschedulingFrequency::setStartTime(unsigned long value) { startTime = value; }
unsigned long PathManagerReschedulingFrequency::getStartTime() const { return startTime; }

unsigned long PathManagerReschedulingFrequency::getEndTime() const { return endTime; }

void PathManagerReschedulingFrequency::setEndTime(unsigned long value) { endTime = value; }

unsigned long PathManagerReschedulingFrequency::getNumberOfCycles() const { return numberOfCycles; }

void PathManagerReschedulingFrequency::setNumberOfCycles(unsigned long value) { numberOfCycles = value; }

string PathManagerReschedulingFrequency::getOutputFileName() const { return outputFileName; }

void PathManagerReschedulingFrequency::setOutputFileName(const string &value) { outputFileName = value; }

void PathManagerReschedulingFrequency::printToFile()
{

    if (this->outputFileName == "")
        this->outputFileName = "reschedulingFrequency.txt";

    ofstream outputFile;
    outputFile.open(this->outputFileName, ios::out);

    // to format the string
    stringstream strm;

    strm << std::left << std::setw(25) << "Start time, ";
    strm << std::left << std::setw(25) << "End time, ";
    strm << std::left << std::setw(25) << "Number of cycles,";
    strm << std::left << std::setw(25) << "Rescheduling frequency" << endl;

    strm << std::left << std::setw(25) << this->startTime << ", ";
    strm << std::left << std::setw(25) << this->endTime << ", ";
    strm << std::left << std::setw(25) << this->numberOfCycles <<", ";

    unsigned long reschedulingFrequency= this->numberOfCycles/(this->endTime - this->startTime);

    strm << std::left << std::setw(25) << reschedulingFrequency << endl;

    outputFile << strm.str();

    outputFile.close();
}
}
