#ifndef MARKOVMATRIX_H
#define MARKOVMATRIX_H
#include "../Eigen/Eigen/Core"
#include <boost/lockfree/queue.hpp>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <vector>

#include "header/events/AbstractEvent.hpp"
#include "header/execution_path/Cgroup.hpp"
namespace execution_path
{
class Cgroup;
}

namespace events
{
class AbstractEvent;
}

// reduceStates to keep the Matrix small
/*
unsigned int div100State(unsigned int orginal);
unsigned long div100Sn(unsigned long orginal);
unsigned int keep(unsigned int orginal){return orginal;}
unsigned long keepSn(unsigned long orginal){return orginal;}*/
unsigned int stateDiv(unsigned int orginal, unsigned int divNumber);
unsigned long snDiv(unsigned long orginal, unsigned int divNumber);

/**
 * @brief The MarkovMatrix class
 * stores and (re-)calculates the markov matrix
 */
class MarkovMatrix
{
private:
    class Data
    {
    public:
        Data(unsigned int size);
        std::unique_ptr<Eigen::MatrixXd> currentMatrix;              // Current Matrix Used
        std::vector<std::unique_ptr<Eigen::MatrixXd>> preCalcMatrix; // Precalculated Matrixes
        unsigned int stepSize;   // Stepsize of the precalculated matrixes (steps starting with 1
        unsigned int numPreCalc; // Number of steps to be precalculated
        unsigned int size;
        void preCalculate();
    };
    class SampleData
    {
    public:
        SampleData(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples,
                   unsigned int numEventsProcessed);
        SampleData();

        std::shared_ptr<Eigen::MatrixXd> results;
        std::vector<int> numSamples;
        unsigned int numEventsProcessed;
        unsigned long lastUpdate;                            // Number of the last event
        std::map<unsigned long, unsigned int> eventsLeftMap; // Map from CGroupID to number of evetsLeft
    };

    class abstractTask
    {
    public:
        abstractTask(MarkovMatrix *parent) { this->parent = parent; }
        MarkovMatrix *parent;
        virtual void executeTask() = 0;
        virtual ~abstractTask();
        virtual std::string toString() = 0;
    };

    class TaskReceiveCGroup : public abstractTask
    {
    private:
        unsigned int eventsLeft; unsigned long id; unsigned long currentEvent;

    public:
        TaskReceiveCGroup(MarkovMatrix *parent, unsigned long id, unsigned int eventsLeft, unsigned long currentEvent)
            : abstractTask(parent)
        {
            this->eventsLeft= eventsLeft;
            this->id=id;
            this->currentEvent=currentEvent;
        }
        virtual void executeTask() override;
        virtual std::string toString() override;
    };

    class TaskExecutionPathFinished : public abstractTask
    {
    private:
        unsigned long lastEvent;
        bool finished;

    public:
        TaskExecutionPathFinished(MarkovMatrix *parent, unsigned long lastEvent, bool finished) : abstractTask(parent)
        {
            this->lastEvent = lastEvent;
            this->finished = finished;
        }
        virtual void executeTask() override;
        virtual std::string toString() override;
    };

private:
    std::shared_ptr<Data> data;
    std::unique_ptr<Eigen::MatrixXd> futureMatrix; // The next Matrix (Usually empty)
    unsigned int stepSize;                         // Stepsize of the precalculated matrixes (steps starting with 1)
    unsigned int numPreCalc;                       // Number of steps to be precalculated

    std::unique_ptr<SampleData> sampleData;
    /*    std::unique_ptr<Eigen::MatrixXd> futureMatrixResults;           //Matrix storing the number of times a state
       has been reached
        std::vector<int> futureMatrixSamples;                           //Number of samples for each row of the Matrix
        int currentEventsNextMatrix;                                    //Counter of Events for the next Matrix
    */
    double alpha;                 // Parameter: how is the next Matrix weighted
    unsigned int eventsPerMatrix; // Number of events per Matrix
    bool garbageState;            // true: a garbage state is present (the first row and colomn is used)

    //    boost::lockfree::queue<SampleData*> *queue;                     //Queue fo data to be processed

    //mutable std::mutex mutexAddDataLock;             // Mutex for adding Data, just one write possible
    //mutable boost::shared_mutex sharedMutexDataLock; // Mutex for the Data one write or many read

    bool running;                                   // Thread is running
    std::condition_variable queueConditionVariable; // Notify when queue is filled

    boost::lockfree::queue<abstractTask *> *taskQueue; // Queue for the incomming Tasks created from
                                                       // receiveConsumptionGroup(), executionPathFinished() and
                                                       // executionPathDiscard()
    unsigned long queueSize = 0;
    //    std::map<unsigned long, std::shared_ptr<MarkovMatrix::CollectingSampleData>> map;

    //    unsigned int (*statesModFunc)(unsigned int) = keep;
    //    unsigned long (*eventSnFunc)(unsigned long) = keepSn;
    unsigned int divNum = 1;

public:
    /**
     * @brief MarkovMatrix
     * @param alpha: weight of the next matrix
     * @param eventsPerMatrix: number of events considered for each matrix
     * @param stepSize: gap between 2 Matrixes
     * @param numPreCalc: number of Matrixes to be calculated
     * @param garbageState: a garbage State is used (has to be considered when adding data)
     */
    MarkovMatrix(double alpha, unsigned int eventsPerMatrix, unsigned int stepSize, unsigned int numPreCalc,
                 unsigned int matrixMinFactor = 1, bool garbageState = false);
    virtual ~MarkovMatrix();
    /**
     * @brief addData add data to the matrix (use only if not used as a thread)
     * @param results: Matrix storing the number of results from each start state
     * @param numSamples: Number of samples from each starting state
     * @param numEventsProcessed: Number of events included
     */
    void addData(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples,
                 unsigned int numEventsProcessed);

    /**
     * @brief setStatesModFunc set Function to Modify the number of remaining events
     * @param statesModFunc the funcion look like unsigned int funcName(unsigned int)
     * funcName(0) must return 0
     * funcName(not 0) must not return 0
     * funcName should be monoton growing
     * if not set stateModFunc(x) returns x
     */
    //    void setStatesModFunc(unsigned int (*statesModFunc)(unsigned int));

    /**
     * @brief setStatesModFunc set Function to Modify the number of remaining events
     * @param statesModFunc the funcion look like unsigned int funcName(unsigned int)
     * funcName(0) must return 0
     * funcName should be monoton growing
     * if not set stateModFunc(x) returns x
     */
    //    void setEventSnFunc(unsigned int(*statesModFunc)(unsigned int));

    /**
     * @brief setDiv
     * @param divNum number each event and state left is divided by (1 if not set)
     */
    void setDiv(unsigned int divNum);

    /**
     * @brief addToQueue add data to the queue. To be processed, the main thread has to run (not working)
     * @param results: Matrix storing the number of results from each start state
     * @param numSamples: Number of samples from each starting state
     * @param numEventsProcessed: Number of events included
     */
    // void addToQueue(std::shared_ptr<Eigen::MatrixXd> results, std::vector<int> numSamples, int numEventsProcessed);
    /**
     * @brief getPropability get the propability a pattern is completed (needs read lock)
     * @param windowLeft: Number of events left in the window
     * @param patternLeft: number of events missing from the pattern
     * @return Propability read from the closesed calculated Matrix
     */
    double getProbability(unsigned long windowLeft, unsigned int patternLeft) const;

    /**
     * @brief main loop function to process the data in the queue
     */
    void main();

    /**
     * @brief stopRunning stop the main function
     */
    void stopRunning();

    /**
     * @brief receiveCGroup
     * @param executionPathID
     * @param CGroup
     */
    void receiveCGroup(unsigned long id,unsigned int eventsLeft,  unsigned long currentEvent);
    void executionPathFinished(unsigned long lastEvent, bool finished = true);

private:
    /**
     * @brief resizeMatrix the matrix and fill with 0.0
     * @param matrix
     * @param oldsize
     * @param newsize
     */
    static void resizeMatrix(Eigen::MatrixXd *matrix, unsigned int oldsize, unsigned int newsize);
    static void resizeMatrix(Eigen::MatrixXd *matrix, unsigned int newsize);
    /**
     * @brief calcFutureMatrix calulate the future Matrix
     */
    void calcFutureMatrix();
    /**
     * @brief recalcCurrentMatrix calculate the (next) current matrix (needs write lock)
     */
    void recalcCurrentMatrix();
};

#endif // MARKOVMATRIX_H
