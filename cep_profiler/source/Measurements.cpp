/*
 * TimeMeasurement.cpp
 *
 * Created on: 10.10.2016
 *      Author: sload
 */

#include "../header/Measurements.hpp"

namespace profiler
{

Measurements::Measurements() {}

Measurements::Measurements(string latencyDetectionFile, string eventsProcessedPerWorkersFile, string throughputFile,
                           string rescheduligFrequencyFile)
{
    this->throughputStat = make_shared<ThroughputStat>(throughputFile);
    this->workerThreadStat = make_shared<WorkerThreadStat>(eventsProcessedPerWorkersFile);
    this->detectionLatencyStat = make_shared<DetectionLatencyStat>(latencyDetectionFile);
    this->pathManagerReschedulingFrequency = make_shared<PathManagerReschedulingFrequency>(rescheduligFrequencyFile);

    //    this->stringNames[ComponentName::MAIN] = "Main";
}

const shared_ptr<ThroughputStat> &Measurements::getThroughputStat() const { return throughputStat; }

const shared_ptr<WorkerThreadStat> &Measurements::getWorkerThreadStat() const { return workerThreadStat; }

const shared_ptr<DetectionLatencyStat> &Measurements::getDetectionLatencyStat() const { return detectionLatencyStat; }

const shared_ptr<PathManagerReschedulingFrequency> &Measurements::getPathManagerReschedulingFrequency() const
{
    return pathManagerReschedulingFrequency;
}

void Measurements::printToFiles()
{
    this->throughputStat->printToFile();
    this->workerThreadStat->printToFile();
    this->detectionLatencyStat->printToFile();
    this->pathManagerReschedulingFrequency->printToFile();
}

// void Measurements::print(string fileName)
//{
//    // count number of lines

//    ifstream inputFile(fileName);
//    // new lines will be skipped unless we stop it:
//    inputFile.unsetf(std::ios_base::skipws);
//    // count the newlines
//    unsigned long lineCount = count(istream_iterator<char>(inputFile), std::istream_iterator<char>(), '\n');

//    // to format the string
//    stringstream strm;

//    ofstream outputFile;
//    outputFile.open(fileName, ios::app);

//    if (lineCount == 0)
//    {
//        strm << std::left << std::setw(25) << "Number of workers:";

//        for (auto it = this->stringNames.begin(); it != this->stringNames.end();)
//        {
//            string temp = it->second;
//            it++;
//            if (it != this->stringNames.end())
//                temp += ":";

//            strm << std::left << std::setw(25) << temp;
//        }

//        strm << endl;
//    }

//    strm << std::left << std::setw(25) << to_string(this->numberOfWorkerThreads) + ":";

//    for (auto it = this->stringNames.begin(); it != this->stringNames.end();)
//    {
//        string temp = to_string(this->timeDurations[it->first]);
//        it++;
//        if (it != this->stringNames.end())
//            temp += ":";

//        strm << std::left << std::setw(25) << temp;
//    }
//    strm << endl;

//    outputFile << strm.str();

//    outputFile.close();
//}

Measurements::~Measurements() {}
}
