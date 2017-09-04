/*
 * main.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: sload
 */

#include "header/events/events.hpp"
#include "header/shared_memory/shared_memory.hpp"
#include "header/util/util.hpp"

#include "header/path_manager/GarbageCollectionThread.hpp"
#include "header/path_manager/PathManager.hpp"
#include "header/worker/WorkerThread.hpp"

#include "header/markovmatrix.h"

#include "header/SourceMain.hpp"

#include "header/predicates/CountBasedPredicate.hpp"
#include "header/predicates/SlidingWindowPredicate.hpp"
#include "header/predicates/IBMtimePredicate.hpp"
#include "header/splitter/Splitter.hpp"

#include "header/Merger.hpp"

#include "header/Measurements.hpp"

#include <boost/filesystem.hpp>

#include <exception>
#include <memory>
#include <stdio.h>
#include <string>
#include <thread>

using namespace events;
using namespace shared_memory;
using namespace util;

using namespace execution_path;

using namespace source;

using namespace merger;

using namespace splitter;

using namespace profiler;

using namespace std;

#ifdef NDEBUG
const int offset = 1;
#else
const int offset = 1;
#endif

enum ArgvOffset
{
    NUMBER_OF_WORKER_THREADS = offset,
    EVENT_TYPE,
    OPERATOR_TYPE,
    INITIAL_EVENTS_PER_SELECTION,
    PATTERN_SIZE,
    SELECTION_SIZE,
    TREE_SIZE,
    CHECKPOINTING_FREQUENCY,
    PORT_NUMBER,

    // Markov
    ALPHA,
    EVENTS_PER_MATRIX,
    STEP_SIZE,
    NUM_PRE_CALC,
    MATRIX_MIN_FACTOR,
    GARBAGE_STATE,

    // Profiling files
    DETECTION_LATENCY_FILE,
    EVENTS_PROCESSED_PER_WORKERS_FILE,
    THROUGHPUT_FILE,
    RESCHEDULING_FREQUENCY_FILE
};

struct MarkovParameters
{
    double alpha = 0.7;
    unsigned int eventsPerMatrix = 10000;
    unsigned int stepSize = 30;
    unsigned int numPreCalc = 10;
    unsigned int matrixMinFactor = 1;
    bool garbageState = false;
};

// Variables--------------------------
static shared_ptr<Measurements> measurements;
// static shared_ptr<SourceSplitterQueue<shared_ptr<AbstractEvent>>> sourceSplitterQueue;
static shared_ptr<SourceSplitterLockfreeQueue> sourceSplitterQueue;
static shared_ptr<GlobalEventStreamList> globalEventStreamList;
static shared_ptr<SourceMain> sourceMain;
static shared_ptr<Merger> mergerObj;
static shared_ptr<MarkovMatrix> markovMatrix;
static shared_ptr<OperatorFactory> operatorFactory;
static shared_ptr<ExecutionPathFactory> executionPathFactory;
static vector<shared_ptr<WorkerThread>> workers;
static shared_ptr<PathManager> pathManager;
static shared_ptr<GarbageCollectionThread> garbageCollection;
static shared_ptr<AbstractPredicate> predicate;
static shared_ptr<Splitter> splitterObj;

// End of variables selection

// Parameters ------------------------

static unsigned int numberOfWorkerThreads = 0;
static Constants::EventType eventType = Constants::EventType::SIMPLE_VALUE;
static Constants::OperatorType operatorType = Constants::OperatorType::STOCK_RISE_OPERATOR;
static unsigned int initialEventsPerSelection = 300;
static bool play = false;
static vector<string> eventStates = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M"};
static unsigned int workLoad = 0;
static unsigned int patternSize = 200;    // how many events are in the pattern
static unsigned long selectionSize = 120; // window size in seconds
static unsigned int treeSize = 32;
static unsigned int checkpointingFrequency = 10; // the number of events between two consecutive check points
static int portNumber = 4000;
static MarkovParameters markovParameters;

// profiling files
static string detectionLatencyFile = "detectionLatency.txt";
static string eventsProcessedPerWorkersFile = "eventsProcessedPerWorkers.txt";
static string throughputFile = "throughput.txt";
static string rescheduligFrequencyFile = "reschedulingFrequency.txt";
// End of parameters section

void parseArguments(int argc, char *argv[])
{
    string usage
        = "usage:[./main #workerThreads eventType operatorType initialEventsPerSelection patternSize \n"
          "selectionSize treeSize checkpointingFrequency  portNumber alpha eventPerMatrix stepSize numPreCalc \n"
          "matrixMinFactor garbageState detectionLatencyFile eventsProcessedPerWorkersFile throughputFile]\n"
          "Example: \n"
          "./main 4 0 1 300 200 120 32 10 4000 0.7 10000 30 10 1 false latencyDetection.txt \n"
          "eventsProcessedPerWorkers.txt  throughput.txt";

    if (argc <= 1)
    {
        unsigned int numberOfCores = thread::hardware_concurrency();
        if (numberOfCores == 0) // if thread::hardware_concurrency() returned 0
            numberOfCores = 8;

        //-4 because one core for splitter, one for source, one for path manager and one for merger
        if ((numberOfCores - 4) <= 1)
            numberOfWorkerThreads = 1;
        else
            numberOfWorkerThreads = numberOfCores - 4;
    }
    else
    {
        try
        {
            numberOfWorkerThreads = static_cast<unsigned int>(stoi(argv[ArgvOffset::NUMBER_OF_WORKER_THREADS]));
            eventType = static_cast<Constants::EventType>(stoi(argv[ArgvOffset::EVENT_TYPE]));
            operatorType = static_cast<Constants::OperatorType>(stoi(argv[ArgvOffset::OPERATOR_TYPE]));
            initialEventsPerSelection = static_cast<unsigned int>(stoi(argv[ArgvOffset::INITIAL_EVENTS_PER_SELECTION]));
            patternSize = static_cast<unsigned int>(stoi(argv[ArgvOffset::PATTERN_SIZE]));
            selectionSize = stoul(argv[ArgvOffset::SELECTION_SIZE]);
            treeSize = static_cast<unsigned int>(stoi(argv[ArgvOffset::TREE_SIZE]));
            checkpointingFrequency = static_cast<unsigned int>(stoi(argv[ArgvOffset::CHECKPOINTING_FREQUENCY]));
            portNumber = stoi(argv[ArgvOffset::PORT_NUMBER]);

            // Markov parameters
            markovParameters.alpha = stod(argv[ArgvOffset::ALPHA]);
            markovParameters.eventsPerMatrix = static_cast<unsigned int>(stoi(argv[ArgvOffset::EVENTS_PER_MATRIX]));
            markovParameters.stepSize = static_cast<unsigned int>(stoi(argv[ArgvOffset::STEP_SIZE]));
            markovParameters.numPreCalc = static_cast<unsigned int>(stoi(argv[ArgvOffset::NUM_PRE_CALC]));
            markovParameters.matrixMinFactor = static_cast<unsigned int>(stoi(argv[ArgvOffset::MATRIX_MIN_FACTOR]));
            istringstream(argv[ArgvOffset::GARBAGE_STATE]) >> std::boolalpha >> markovParameters.garbageState;

            // Profiling files
            detectionLatencyFile = argv[ArgvOffset::DETECTION_LATENCY_FILE];
            eventsProcessedPerWorkersFile = argv[ArgvOffset::EVENTS_PROCESSED_PER_WORKERS_FILE];
            throughputFile = argv[ArgvOffset::THROUGHPUT_FILE];
            rescheduligFrequencyFile = argv[ArgvOffset::RESCHEDULING_FREQUENCY_FILE];
        }
        catch (exception &e)
        {
            cout << usage << endl;
            cout << "error: " << e.what() << endl;
            exit(-1);
        }
    }

    // print the parameters------------------
    cout << "argc: " << argc << endl;

    int printformat = 35;
    cout << "General parameters:" << endl;
    cout << std::left << std::setw(printformat) << "Number of workers: " << numberOfWorkerThreads << endl;
    cout << std::setw(printformat) << "Event Type: " << eventType << endl;
    cout << std::setw(printformat) << "Operator Type: " << operatorType << endl;
    cout << std::setw(printformat) << "Initial Events Per Selection: " << initialEventsPerSelection << endl;
    cout << std::setw(printformat) << "Pattern Size: " << patternSize << endl;
    cout << std::setw(printformat) << "Selection Size: " << selectionSize << endl;
    cout << std::setw(printformat) << "Tree Size: " << treeSize << endl;
    cout << std::setw(printformat) << "Checkpointing Frequency: " << checkpointingFrequency << endl;
    cout << std::setw(printformat) << "Port Number: " << portNumber << endl;

    // Markov parameters
    cout << "Markov parameters:" << endl;
    cout << std::setw(printformat) << "Alpha: " << markovParameters.alpha << endl;
    cout << std::setw(printformat) << "Events Per Matrix: " << markovParameters.eventsPerMatrix << endl;
    cout << std::setw(printformat) << "Step Size: " << markovParameters.stepSize << endl;
    cout << std::setw(printformat) << "Num Pre Calc: " << markovParameters.numPreCalc << endl;
    cout << std::setw(printformat) << "Matrix Min Factor: " << markovParameters.matrixMinFactor << endl;
    cout << std::setw(printformat) << "Garbage State: " << markovParameters.garbageState << endl;

    // Profiling files
    cout << "Profiling files:" << endl;
    cout << std::setw(printformat) << "Detection Latency File: " << detectionLatencyFile << endl;
    cout << std::setw(printformat) << "Events Processed Per Workers File: " << eventsProcessedPerWorkersFile << endl;
    cout << std::setw(printformat) << "Throughput File: " << throughputFile << endl;
    cout << std::setw(printformat) << "Rescheduling Frequency File: " << rescheduligFrequencyFile << endl;
}

void initialization()
{
    measurements = make_shared<Measurements>(detectionLatencyFile, eventsProcessedPerWorkersFile, throughputFile,
                                             rescheduligFrequencyFile);

    //    sourceSplitterQueue = make_shared<SourceSplitterQueue<shared_ptr<AbstractEvent>>>();
    sourceSplitterQueue = make_shared<SourceSplitterLockfreeQueue>();

    //    EventFactory factory(Constants::EventType::PULSE, measurements.get());
    //    shared_ptr<AbstractEvent> fakeEvent = factory.createNewEvent();

    //    globalEventStreamList
    //        = make_shared<GlobalEventStreamList<shared_ptr<AbstractEvent>>>(fakeEvent, measurements.get());

    globalEventStreamList = make_shared<GlobalEventStreamList>(measurements.get());

//    sourceMain = make_shared<SourceMain>(*sourceSplitterQueue.get(), play, eventType, portNumber, measurements.get());


// /*
     unsigned long numberOfEventsToBeProduced = 3000000;
     unsigned long interProductionTime = 0;
     unsigned int randomEventSize=300;

     vector<string> eventStatesSource = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                                         "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
                                         "1", "2","3", "4","5","6","7","8","9","10","11","12","13",
                                          "14", "15", "16","17","18", "19", "20", "21", "22", "23", "24","25","26"};

     sourceMain = make_shared<SourceMain>(*sourceSplitterQueue.get(), play, eventType, numberOfEventsToBeProduced,
                                          interProductionTime, eventStatesSource, randomEventSize, measurements.get());


    sourceMain->setSeed(10000);

// */


    mergerObj = make_shared<Merger>(measurements.get());

    markovMatrix = make_shared<MarkovMatrix>(markovParameters.alpha, markovParameters.eventsPerMatrix,
                                             markovParameters.stepSize, markovParameters.numPreCalc,
                                             markovParameters.matrixMinFactor, markovParameters.garbageState);

// /*
      operatorFactory
        = make_shared<OperatorFactory>(operatorType, patternSize, eventStatesSource, workLoad, measurements.get());
//   */

//    operatorFactory
//        = make_shared<OperatorFactory>(operatorType, patternSize, eventStates, workLoad, measurements.get());

    executionPathFactory = make_shared<ExecutionPathFactory>(globalEventStreamList.get(), pathManager.get(),
                                                             markovMatrix.get(), mergerObj.get(), operatorFactory.get(),
                                                             checkpointingFrequency, measurements.get());


    for (size_t i = 0; i < numberOfWorkerThreads; i++)
    {
        workers.push_back(make_shared<WorkerThread>(measurements.get()));
    }

    garbageCollection = make_shared<GarbageCollectionThread>();

    pathManager = make_shared<PathManager>(workers, executionPathFactory, markovMatrix, initialEventsPerSelection,
                                           treeSize, garbageCollection.get(), measurements.get());

    executionPathFactory->setPathManager(pathManager.get());

//    predicate = make_shared<IBMtimePredicate>(selectionSize);
//        predicate = make_shared<CountBasedPredicate>(selectionSize);
//  /*
    unsigned int slidingSize=500;
    predicate = make_shared<SlidingWindowPredicate>(selectionSize, slidingSize);
//*/
    splitterObj
        = make_shared<Splitter>(sourceSplitterQueue.get(), globalEventStreamList.get(), mergerObj.get(),
                                predicate.get(), pathManager.get(), initialEventsPerSelection, measurements.get());
}

void cleanFiles()
{
    // clean existing files (complex event, generated events, splitting result files)
    remove("./complexEvents.txt");

    remove("./selections_splitter.txt");

    remove("./generated_events.txt");
}

int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
    cleanFiles();
    initialization();

    // start time
    unsigned long startTime = Helper::currentTimeMillis();

    // start the threads (source, splitter, merger, worker threads and path manager)
    thread sourceThread(&SourceMain::main, sourceMain.get());
    thread splitterThread(&Splitter::main, splitterObj.get());
    thread markovThread(&MarkovMatrix::main, markovMatrix.get());
    thread pathManagerThread(&PathManager::main, pathManager.get());
    thread garbageCollectionThread(&GarbageCollectionThread::main, garbageCollection.get());

    vector<thread> workerThreads(numberOfWorkerThreads);

    for (size_t i = 0; i < numberOfWorkerThreads; i++)
    {
        thread temp(&WorkerThread::main, workers[i].get());
        workerThreads[i] = std::move(temp);
    }

    //    thread mergerThread(&Merger::main, mergerObj.get());

    // wait for other threads
    sourceThread.join();
    cout << "Source thread finished!" << endl;

    splitterObj->setTerminate(true);
    splitterThread.join();
    cout << "Splitter thread finished!" << endl;

    // send terminate after splitter finishes
    pathManager->setTerminate();
    pathManagerThread.join();
    cout << "Path manager thread finished!" << endl;

    garbageCollection->stopRunning();
    garbageCollectionThread.join();
    cout << "Garbage Collection thread finished!" << endl;

    for (size_t i = 0; i < numberOfWorkerThreads; i++)
    {
        workerThreads[i].join();
    }

    cout << "Worker threads finished!" << endl;

    //    mergerObj.get()->setTerminate();
    //    mergerThread.join();
    cout << "Merger thread finished!" << endl;

    markovMatrix->stopRunning();
    markovThread.join();
    cout << "Markov Matrix thread finished!" << endl;

    // end time
    unsigned long endTime = Helper::currentTimeMillis();

    //    measurements.get()->setTime(Measurements::ComponentName::MAIN, endTime - startTime);

    measurements->getThroughputStat()->set(startTime, endTime, sourceMain->getNumberOfReceivedEvents());
    measurements->printToFiles();

    cout << " Program duration: " << (endTime - startTime) << " ms" << endl;
    cout << "all events have been successfully processed!" << endl;
}
