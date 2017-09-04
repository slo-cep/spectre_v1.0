/*
 * workerthread.cpp
 *
 * Created on: 06.10.2016
 *      Author: sload
 */

#include "../../header/worker/WorkerThread.hpp"

using namespace selection;
using namespace profiler;
using namespace util;

namespace execution_path
{
unsigned long WorkerThread::lastWokerId = 0;

WorkerThread::WorkerThread(Measurements *measurements) : executionPathQueue(0), measurements(measurements)
{
    this->id = WorkerThread::lastWokerId;
    WorkerThread::lastWokerId++;
}

void WorkerThread::main()
{
    while (!this->terminate)
    {
        shared_ptr<ExecutionPath> temp;
        //        if (this->executionPathQueue.pop(temp))
        //        {
        //            // Unlock current execution path, then get the new execution path
        //            if (this->runningExecutionPath != nullptr)
        //            {
        //                this->runningExecutionPath->writeUnlock();
        //            }

        //            this->runningExecutionPath = temp; // new execution path
        //            if (this->runningExecutionPath != nullptr)
        //            {
        //                this->runningExecutionPath->writeLock();
        //            }
        //        }

        if (newTask == true)
        {
            newTask=false;
            temp = atomic_load(&this->newExecutionPath);

            if (this->runningExecutionPath != nullptr)
            {
                this->runningExecutionPath->writeUnlock();
            }

            if (this->startTime != 0)
            {
                unsigned long stopTime = Helper::currentTimeMillis();
                this->Idletime += stopTime - this->startTime;
                this->startTime = 0;
            }

            this->runningExecutionPath = temp; // new execution path

            if (this->runningExecutionPath != nullptr)
            {
                while (!this->runningExecutionPath->writeTryLock())
                {
                    /*
                     * couldn't acquire the lock which can be locked by other worker (rare case but could happen).
                     * check meanwhile if there is new execution path
                     */
                    if (newTask == true)
                    {
                        newTask=false;
                        temp = atomic_load(&this->newExecutionPath);
                        this->runningExecutionPath = temp; // new execution path

                        if (this->runningExecutionPath == nullptr)
                            break;
                    }

                    cout << "Worker Thread: Execution Path Id:" << this->runningExecutionPath->getId() << ", try lock!"
                         << endl;
                }
            }
        }

        if (this->runningExecutionPath != nullptr)
        {
            this->execute();
        }
        else
        {
            //            this_thread::sleep_for(chrono::nanoseconds(1));
//                        cout<<"Worker Thread: " << this->id<<", has nothing to do (yield)"<< endl;
//            this_thread::yield();
        }
    }

    string out = "Worker Thread: " + std::to_string(this->id) + ",  idle time: " + to_string(this->Idletime) + "\n";
    cout << out;
    measurements->getWorkerThreadStat()->setNewWorker(this->id, this->eventsProcessedPerWorker);
}

void WorkerThread::receiveExecutionPath(const shared_ptr<ExecutionPath> &newExecutionPath)
{
    //    shared_ptr<ExecutionPath> temp;
    //    executionPathQueue.pop(temp);

    //    executionPathQueue.push(newExecutionPath);

    atomic_store(&this->newExecutionPath, newExecutionPath);
    newTask=true;
}

void WorkerThread::execute()
{
    if (this->runningExecutionPath->getSpeculator()->main(1))
        this->eventsProcessedPerWorker++;
    else
    {
        if (this->runningExecutionPath->getFinished() && this->startTime == 0)
            this->startTime = Helper::currentTimeMillis();
        //        if (this->runningExecutionPath->getFinished())
        //        {
        //            this->runningExecutionPath = nullptr;
        //            this->startTime = Helper::currentTimeMillis();
        //        }
    }
}

unsigned long WorkerThread::getId() { return this->id; }

void WorkerThread::setTerminate() { this->terminate=true; }

unsigned long WorkerThread::getEventsProcessedPerWorker() const { return eventsProcessedPerWorker; }

void WorkerThread::setEventsProcessedPerWorker(unsigned long value) { eventsProcessedPerWorker = value; }
}
