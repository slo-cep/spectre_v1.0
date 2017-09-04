#include "header/markovmatrix.h"
#include <iostream>
#include <unsupported/Eigen/MatrixFunctions>
//#include "header/execution_path/Cgroup.hpp"

MarkovMatrix::MarkovMatrix(double alpha, unsigned int eventsPerMatrix, unsigned int stepSize, unsigned int numSteps,
                           unsigned int matrixMinFactor, bool garbageState)
{
    this->alpha = alpha;
    this->eventsPerMatrix = eventsPerMatrix / matrixMinFactor;
    this->stepSize = stepSize / matrixMinFactor;
    if (this->stepSize == 0)
    {
        this->stepSize = 1;
    }
    this->numPreCalc = numSteps;
    this->divNum = matrixMinFactor;
    this->garbageState = garbageState;
    this->sampleData = std::unique_ptr<MarkovMatrix::SampleData>(new SampleData);
    //    sampleData->numEventsProcessed = 0;
    futureMatrix = std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(1 + garbageState, 1 + garbageState));
    //    sampleData->results = std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(1 + garbageState, 1 +
    //    garbageState));
    data = std::unique_ptr<Data>(new Data(1 + garbageState));
    taskQueue = new boost::lockfree::queue<abstractTask *>(0);
    running = false;
}

MarkovMatrix::~MarkovMatrix()
{
    MarkovMatrix::abstractTask *task;
    while (!taskQueue->empty())
    {
        if (taskQueue->pop(task))
        {
            delete task;
        }
    }
    delete taskQueue;
}

void MarkovMatrix::addData(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples,
                           unsigned int numEventsProcessed)
{
    //    mutexAddDataLock.lock();

    unsigned int newsize = numSamples.size();
    unsigned int oldsize = sampleData->numSamples.size();
    if (newsize > oldsize)
    {
        resizeMatrix(sampleData->results.get(), oldsize, newsize);
        sampleData->numSamples.resize(newsize);
    }
    else if (newsize < oldsize)
    {
        resizeMatrix(results.get(), newsize, oldsize);
    }
    (*sampleData->results) += (*results);
    for (unsigned int i = 0; i < newsize; ++i)
    {
        sampleData->numSamples[i] += numSamples[i];
    }

    sampleData->numEventsProcessed += numEventsProcessed;

    if (sampleData->numEventsProcessed >= eventsPerMatrix)
    {
        // Calc next Matrix
        calcFutureMatrix();
        sampleData->numEventsProcessed = 0;
        recalcCurrentMatrix();
    }

    //    mutexAddDataLock.unlock();
}

void MarkovMatrix::setDiv(unsigned int divNum) { this->divNum = divNum; }
/*
void MarkovMatrix::setEventSnFunc(unsigned int (*statesModFunc)(unsigned int))
{
    this->statesModFunc = statesModFunc;
}*/
/*
void MarkovMatrix::addToQueue(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples, int
numEventsProcessed)
{
    queue->push(new MarkovMatrix::SampleData(results, numSamples, numEventsProcessed));
    queueConditionVariable.notify_one();
}*/

double MarkovMatrix::getProbability(unsigned long windowLeft, unsigned int patternLeft) const
{
//    return 1.0;
    shared_ptr<Data> used_data = atomic_load(&this->data);

//    std::shared_lock<boost::shared_mutex> lock(sharedMutexDataLock);
    //    windowLeft = this->eventSnFunc(windowLeft);
    //    patternLeft = this->statesModFunc(patternLeft);
    unsigned long windowLeftLocal = snDiv(windowLeft, divNum);
    unsigned int patternLeftLocal = stateDiv(patternLeft, divNum);

    if (patternLeftLocal == 0)
    {
        return 1.0;
    }
    if (windowLeftLocal == 0)
    {
        return 0.0;
    }


    int resultColomn = 0;
    if (garbageState)
    {
        patternLeftLocal++;
        resultColomn++;
    }

    if (patternLeftLocal >= used_data->size)
    {
        // Error
        return -1.0;
    }
    if (used_data->preCalcMatrix.size() == 0)
        return 0;

    if (windowLeftLocal >= 1 + (numPreCalc - 1) * stepSize)
    {

        return (*used_data->preCalcMatrix.back())(patternLeftLocal, resultColomn);
    }
/*
    std::cout << *data->preCalcMatrix[0] << "\n\n";
    std::cout << *data->preCalcMatrix[1] << "\n\n";
    std::cout << *data->preCalcMatrix[2] << "\n\n";
    std::cout << *data->preCalcMatrix[3] << "\n\n";
*/
    size_t lowerIndex = (windowLeftLocal - 1) / stepSize;
    size_t upperIndex = lowerIndex + 1;
    size_t lowerPlus = (windowLeftLocal - 1) % stepSize;
    double upperResult = ((*used_data->preCalcMatrix[upperIndex])(patternLeftLocal, resultColomn));
    double lowerResult = ((*used_data->preCalcMatrix[lowerIndex])(patternLeftLocal, resultColomn));
    return (upperResult - lowerResult) / (stepSize) * (lowerPlus) + lowerResult;
}
/*
void MarkovMatrix::main()
{
    //Thread can only run once
    if (running)
    {
        return;
    }
    //Whyever I need this...
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    running = true;
    MarkovMatrix::SampleData *sampleData;
    while (running)
    {
        while (queue->empty())
        {
            queueConditionVariable.wait(lock);
            if (!running)
            {
                return;
            }
        }
        if (queue->pop(sampleData))
        {
            addData(sampleData->results, sampleData->numSamples, sampleData->numEventsProcessed);
            delete sampleData;
        }
    }
}*/
void MarkovMatrix::main()
{
    // Thread can only run once
    if (running)
    {
        return;
    }
    // Whyever I need this...
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    MarkovMatrix::abstractTask *task;

    running = true;
    while (running)
    {
        while (taskQueue->empty())
        {
            queueConditionVariable.wait(lock);
            if (!running)
            {
                return;
            }
        }
        while (!taskQueue->empty() && running)
        {
            if (taskQueue->pop(task))
            {
                this->queueSize--;
//                std::cout << "QueueSize = "  << queueSize << "\n";
//                ofstream outputFile;
//                outputFile.open("./Markov.txt", ios::app);
//                outputFile << task->toString() << endl;
//                outputFile.close();
                task->executeTask();
                delete task;
            }
        }
    }
}

void MarkovMatrix::stopRunning()
{
    running = false;
    queueConditionVariable.notify_one();
}

void MarkovMatrix::receiveCGroup(unsigned long id, unsigned int eventsLeft, unsigned long currentEvent)
{
    taskQueue->push(new MarkovMatrix::TaskReceiveCGroup(this, id, eventsLeft, currentEvent));
    this->queueSize++;
    queueConditionVariable.notify_one();
}

void MarkovMatrix::executionPathFinished(unsigned long lastEvent, bool finished)
{
    taskQueue->push(new MarkovMatrix::TaskExecutionPathFinished(this, lastEvent, finished));
    this->queueSize++;
    queueConditionVariable.notify_one();
}

// Keep the Matrix and fill it with 0
void MarkovMatrix::resizeMatrix(Eigen::MatrixXd *matrix, unsigned int oldsize, unsigned int newsize)
{
    matrix->conservativeResize(newsize, newsize);
    for (unsigned int i = 0; i < oldsize; ++i)
    {
        for (unsigned int j = oldsize; j < newsize; ++j)
        {
            (*matrix)(i, j) = 0.0;
        }
    }
    for (unsigned int i = oldsize; i < newsize; ++i)
    {
        for (unsigned int j = 0; j < newsize; ++j)
        {
            (*matrix)(i, j) = 0.0;
        }
    }
}

void MarkovMatrix::resizeMatrix(Eigen::MatrixXd *matrix, unsigned int newsize)
{
    resizeMatrix(matrix, matrix->cols(), newsize);
}

void MarkovMatrix::calcFutureMatrix()
{
    unsigned int size = sampleData->numSamples.size();
    futureMatrix->resize(size, size);
    for (unsigned int i = 0; i < size; ++i)
    {
        if (sampleData->numSamples[i] == 0)
        {
            for (unsigned int j = 0; j < size; ++j)
            {
                (*futureMatrix)(i, j) = 0.0;
            }
            (*futureMatrix)(i, i) = 1.0;
        }
        else
        {
            for (unsigned int j = 0; j < size; ++j)
            {
                (*futureMatrix)(i, j) = (*sampleData->results)(i, j) / sampleData->numSamples[i];
                if ((*futureMatrix)(i, j) < 0)
                    std::cout << "Markov error\n";
            }
        }
    }
    //    std::cout << "Future Matrix:\n" << *futureMatrix;
}

void MarkovMatrix::recalcCurrentMatrix()
{
    Data *newData;

    unsigned int newsize = futureMatrix->rows();
    unsigned int oldsize = data->size;
    if (newsize == oldsize)
    {
        newData = new Data(newsize);
        (*newData->currentMatrix) = *(data->currentMatrix) * (1 - alpha) + (*futureMatrix) * alpha;
    }
    else if (newsize > oldsize) // In case the Matrix has different sizes grow the smaller one and fill the Empty spots
    {
        newData = new Data(newsize);
        Eigen::MatrixXd *tmp = new Eigen::MatrixXd(oldsize, oldsize);
        (*tmp) = (*data->currentMatrix);
        resizeMatrix(tmp, oldsize, newsize);
        (*tmp) = (*tmp) * (1 - alpha) + (*futureMatrix) * alpha;
        for (unsigned int i = 0; i < oldsize; ++i)
        {
            for (unsigned int j = oldsize; j < newsize; ++j)
            {
                (*tmp)(i, j) = (*futureMatrix)(i, j);
            }
        }
        for (unsigned int i = oldsize; i < newsize; ++i)
        {
            for (unsigned int j = 0; j < newsize; ++j)
            {
                (*tmp)(i, j) = (*futureMatrix)(i, j);
            }
        }
        (*newData->currentMatrix) = *tmp;
        delete tmp;
    }
    else // newsize < oldsize
    {
        newData = new Data(oldsize);
        resizeMatrix(futureMatrix.get(), newsize, oldsize);
        (*newData->currentMatrix) = (*data->currentMatrix) * (1 - alpha) + (*futureMatrix) * alpha;
        for (unsigned int i = 0; i < oldsize; ++i)
        {
            for (unsigned int j = oldsize; j < newsize; ++j)
            {
                (*newData->currentMatrix)(i, j) = (*data->currentMatrix)(i, j);
            }
        }
        for (unsigned int i = oldsize; i < newsize; ++i)
        {
            for (unsigned int j = 0; j < newsize; ++j)
            {
                (*newData->currentMatrix)(i, j) = (*data->currentMatrix)(i, j);
            }
        }
    }
    (*newData->currentMatrix)(0,0)=1;

    newData->numPreCalc = numPreCalc;
    newData->stepSize = stepSize;
    newData->preCalculate();

//    std::unique_lock<boost::shared_mutex> lock(sharedMutexDataLock);

//    data = std::shared_ptr<Data>(newData);
    atomic_store(&data, std::shared_ptr<Data>(newData));
}

MarkovMatrix::Data::Data(unsigned int size)
{
    currentMatrix = std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(size, size));
    this->size = size;
}

void MarkovMatrix::Data::preCalculate()
{
    Eigen::MatrixPower<Eigen::MatrixXd> power(*currentMatrix);
    Eigen::MatrixXd powerN(power(stepSize));
    //std::cerr << *currentMatrix << "\n\n";


    preCalcMatrix.resize(numPreCalc);

    preCalcMatrix[0] = (std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(*currentMatrix)));

    for (unsigned int i = 1; i < numPreCalc; ++i)
    {
        preCalcMatrix[i] = (std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(*preCalcMatrix[i - 1] * powerN)));
        //preCalcMatrix[i] = (std::unique_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(power(stepSize * i + 1))));
        // std::cout << *preCalcMatrix[i] << "\n\n";
    }
}

MarkovMatrix::SampleData::SampleData(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples,
                                     unsigned int numEventsProcessed)
{
    this->results = results;
    this->numSamples = numSamples;
    this->numEventsProcessed = numEventsProcessed;
    this->lastUpdate = 0;
}

MarkovMatrix::SampleData::SampleData()
{
    this->results = std::shared_ptr<Eigen::MatrixXd>(new Eigen::MatrixXd(1, 1));
    this->numSamples.resize(1, 0);
    this->numEventsProcessed = 0;
    this->lastUpdate = 0;
}

void MarkovMatrix::TaskReceiveCGroup::executeTask()
{
    MarkovMatrix::SampleData *csd = parent->sampleData.get();

    // FUTURE_TODO Trashstate
    // unsigned int eventsLeft = this->parent->statesModFunc(this->CGroup->getEventsLeft());
    unsigned int eventsLeft = stateDiv(this->eventsLeft, parent->divNum);
    unsigned long id = this->id;
    //    unsigned long currentEvent = this->parent->eventSnFunc(this->CGroup->getEvents().back()->getSn());
    unsigned long currentEvent = snDiv(this->currentEvent, parent->divNum);
    unsigned int newMatrixsize = eventsLeft + 1;
    unsigned long eventsSinceLastUpdate = currentEvent - csd->lastUpdate;

    if (!csd->eventsLeftMap.empty())
        csd->numEventsProcessed += eventsSinceLastUpdate;

    // Check and resize Matrix and Vector
    if (newMatrixsize > csd->numSamples.size())
    {
        csd->numSamples.resize(newMatrixsize, 0);
        resizeMatrix(csd->results.get(), newMatrixsize);
    }

    // Iterate over Map and treat each CGroup as if they stayed in their current state
    for (auto &iter : csd->eventsLeftMap)
    {
        csd->numSamples[iter.second] += eventsSinceLastUpdate;
        if (csd->numSamples[iter.second] < 0)
            std::cout << "csd->numSamples[iter.second] 1 < 0\n";
        (*csd->results)(iter.second, iter.second) += eventsSinceLastUpdate;
    }

    // get the iterator of the changed state
    auto it = csd->eventsLeftMap.find(id);

    if (it == csd->eventsLeftMap.end()) // The CGroup is new
    {
        csd->eventsLeftMap[id] = eventsLeft;
    }
    else // The CGroup exists
    {
        // decrement Matrix since the state has changed
        unsigned int lastState = it->second;
        (*csd->results)(lastState, lastState) -= 1;

        // set state to current
        it->second = eventsLeft;

        // increment the current statechange
        (*csd->results)(lastState, eventsLeft) += 1;
    }
    csd->lastUpdate = currentEvent;

    // If the number of events is reached -> calc the next Matrix
    if (parent->sampleData->numEventsProcessed >= parent->eventsPerMatrix)
    {
        // Calc next Matrix
        parent->calcFutureMatrix();
        parent->sampleData->numEventsProcessed = 0;
        parent->recalcCurrentMatrix();
    }
}

string MarkovMatrix::TaskReceiveCGroup::toString()
{
    std::string returnStr;
    returnStr = "TaskReceiveCGroup: Cgroup ID: " + std::to_string(id) + " Eventsleft: " + std::to_string(eventsLeft) + " currentEvent: " + std::to_string(currentEvent);
    return returnStr;
}

void MarkovMatrix::TaskExecutionPathFinished::executeTask()
{
    MarkovMatrix::SampleData *csd = parent->sampleData.get();

    // FUTURE_TODO Trashstate
    unsigned long currentEvent = snDiv(this->lastEvent, parent->divNum);
    unsigned long eventsSinceLastUpdate = currentEvent - csd->lastUpdate;

    csd->numEventsProcessed += eventsSinceLastUpdate;

    // Iterate over Map and treat each CGroup as if they stayed in their current state
    for (auto &iter : csd->eventsLeftMap)
    {
        csd->numSamples[iter.second] += eventsSinceLastUpdate;
        (*csd->results)(iter.second, iter.second) += eventsSinceLastUpdate;
    }
    parent->addData(csd->results, csd->numSamples, csd->numEventsProcessed);

    std::fill(csd->numSamples.begin(), csd->numSamples.end(), 0);
    (*csd->results).setZero();

    if (this->finished)
    {
        csd->eventsLeftMap.clear();
    }

    // If the number of events is reached -> calc the next Matrix
    if (parent->sampleData->numEventsProcessed >= parent->eventsPerMatrix)
    {
        // Calc next Matrix
        parent->calcFutureMatrix();
        parent->sampleData->numEventsProcessed = 0;
        parent->recalcCurrentMatrix();
    }
}

string MarkovMatrix::TaskExecutionPathFinished::toString()
{
    std::string returnStr;
    returnStr = "TaskExecutionPathFinished: lastEvent: " + std::to_string(lastEvent) + " finished: " + std::to_string(finished);
    return returnStr;
}
/*
//round up
unsigned int div100State(unsigned int orginal)
{
    return (orginal+99)/100;
}

unsigned long div100Sn(unsigned long orginal)
{
    return orginal / 100;
}*/

MarkovMatrix::abstractTask::~abstractTask() {}

unsigned int stateDiv(unsigned int orginal, unsigned int divNumber) { return (orginal + divNumber - 1) / divNumber; }

unsigned long snDiv(unsigned long orginal, unsigned int divNumber) { return orginal / divNumber; }
