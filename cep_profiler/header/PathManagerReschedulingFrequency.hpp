/*
 * PathManagerReschedulingFrequency.hpp
 *
 * Created on: 16.01.2017
 *      Author: sload
 */
#include<string>
#include <sstream> // stringstream
#include <iomanip> // format string
#include<fstream>
#include <iostream>

using namespace std;
namespace profiler
{
class PathManagerReschedulingFrequency
{

public:
    PathManagerReschedulingFrequency(string outputFileName);

    void set(unsigned long startTime, unsigned long endTime, unsigned long numberOfCycles);

    unsigned long getStartTime() const;

    void setStartTime(unsigned long value);

    unsigned long getEndTime() const;
    void setEndTime(unsigned long value);

    unsigned long getNumberOfCycles() const;
    void setNumberOfCycles(unsigned long value);

    string getOutputFileName() const;
    void setOutputFileName(const string &value);

    void printToFile();

private:
    unsigned long startTime;
    unsigned long endTime;
    unsigned long numberOfCycles;

    string outputFileName;
};

}
