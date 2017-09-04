/*
 * workerthread.hpp
 *
 * Created on: 06.10.2016
 *      Author: sload
 */

#ifndef WORKERTHREAD_HPP
#define WORKERTHREAD_HPP

#include "header/selection/AbstractSelection.hpp"
#include "header/shared_memory/LockfreeQueue.hpp"
#include "header/util/Helper.hpp"

#include "header/Measurements.hpp"

#include "../../header/execution_path/ExecutionPath.hpp"
#include "../../header/speculator/AbstractSpeculator.hpp"

#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <thread>

using namespace std;

namespace execution_path
{
class WorkerThread
{
public:
    enum ExecutionResult
    {
        NOT_FINISHED,
        NONE,
        EXECUTION_PATH_READY_TO_BE_REMOVED
    };

    WorkerThread(profiler::Measurements *measurements);

    // run in an infinite loop
    void main();

    void receiveExecutionPath(const shared_ptr<ExecutionPath> &newExecutionPath);

    unsigned long getId();
    void setTerminate();

    unsigned long getEventsProcessedPerWorker() const;
    void setEventsProcessedPerWorker(unsigned long value);

private:
    unsigned long id;
    shared_ptr<ExecutionPath> runningExecutionPath = nullptr;
    shared_ptr<ExecutionPath> newExecutionPath = nullptr;
    atomic<bool> newTask = {false};

    shared_memory::LockfreeQueue<shared_ptr<ExecutionPath>> executionPathQueue;

    atomic_bool terminate = {false};

    //    ExecutionResult executionResult;

    // for debugging/measurement only
    unsigned long eventsProcessedPerWorker = 0;
    unsigned long Idletime = 0;
    unsigned long startTime = 0;

    profiler::Measurements *measurements;

    static unsigned long lastWokerId;

    void execute();
};
}
#endif // WORKERTHREAD_HPP
