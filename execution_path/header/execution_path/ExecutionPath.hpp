/*
 * ExecutionPath.hpp
 *
 * Created on: 06.10.2016
 *      Author: sload
 */

#ifndef EXECUTIONPATH_HPP
#define EXECUTIONPATH_HPP

#include "header/selection/AbstractSelection.hpp"
#include "header/shared_memory/AbstractGlobalEventStream.hpp"
#include "header/shared_memory/LockfreeQueue.hpp"
#include "header/shared_memory/ThreadSafeQueue.hpp"
#include "header/util/Constants.hpp"
#include "header/util/GlobalParameters.hpp"
#include "header/util/GlobalTypedef.hpp"

#include "header/markovmatrix.h"

#include "../../header/feeder/AbstractFeeder.hpp"
#include "../../header/feeder/DeterministicFeeder.hpp"
#include "../../header/operator/OperatorFactory.hpp"
#include "Cgroup.hpp"
#include "LocalEvent.hpp"



#include "header/Merger.hpp"

#include "header/Measurements.hpp"

#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <list>
#include <memory> //unique_ptr + shared_ptr
#include <mutex>
#include <unordered_map>
#include <vector>
#include <set>
using namespace std;

namespace execution_path
{
class PathManager;
class ExecutionPathFactory
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    ExecutionPathFactory(GlobalEventStreamTypedef* globalEventStream, PathManager* pathManager,
                         MarkovMatrix* markovMatrix, merger::Merger* merger,
                         OperatorFactory* operatorFactory, unsigned int checkpointingFrequency,
                         profiler::Measurements *measurements);

    shared_ptr<ExecutionPath> createNewExecutionPath(shared_ptr<selection::AbstractSelection> abstractSelection);

    void setPathManager(PathManager *pathManager);

    ~ExecutionPathFactory();



private:
    GlobalEventStreamTypedef* globalEventStream;
    PathManager* pathManager;
    MarkovMatrix* markovMatrix;
    merger::Merger* merger;
    OperatorFactory* operatorFactory;
    unsigned int checkpointingFrequency;
    profiler::Measurements* measurements;
};

class AbstractSpeculator;
class AbstractValidator;
class Checkpoint;


class ExecutionPath : public enable_shared_from_this<ExecutionPath>
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    /**
     * @brief generateExecutionPathId: generate unique Ids for the execution paths
     * @return: new unique Id
     */
    static unsigned long generateExecutionPathId();

    /**
     * Constructor
     * @param globalEventStream: reference to the shared event stream between all instances
     * @param pathManager: reference to the path manager
     * @param abstractSelection: selection information (it is shared between Splitter, Feeder and Validator)
     * @param markovMatrix: markov model for speculation
     * @param merger: a reference to the Merger
     * @param checkpointingFrequency: frequency of the checkpointing
     * @param measurements: for profiling
     */
    ExecutionPath(GlobalEventStreamTypedef &globalEventStream, PathManager &pathManager,
                  shared_ptr<selection::AbstractSelection> abstractSelection, MarkovMatrix &markovMatrix,
                  merger::Merger &merger, const OperatorFactory &operatorFactory, unsigned int checkpointingFrequency,
                  profiler::Measurements* measurements);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    ExecutionPath(const ExecutionPath &other);

    // deep copy
    shared_ptr<ExecutionPath> clone();

    // deep copy
    shared_ptr<ExecutionPath> cloneForCheckpointing();

    /**
     * @brief getSharedPtr: return a shared pointer to this execution path
     * @return shared_ptr to this execution path
     */
    shared_ptr<ExecutionPath> getSharedPtr();

    /**
     * @brief pushNewLocalEvent: add new local event (this function will be called by Speculator)
     * @param localEvent
     */
    void pushNewLocalEvent(const shared_ptr<LocalEvent>& localEvent);

    const shared_ptr<AbstractSpeculator>& getSpeculator() const;
    const shared_ptr<AbstractFeeder>& getFeeder() const;
    const shared_ptr<AbstractValidator>& getValidator() const;
    const shared_ptr<AbstractOperator>& getOperator() const;
    const shared_ptr<selection::AbstractSelection>& getAbstractSelection() const;
    void setOperator(const shared_ptr<AbstractOperator>& abstractOperator);

    /**
     * @brief getId: return global execution path Id
     * @return execution path global Id
     */
    unsigned long getId();

    /**
     * @brief isMaster: check if this path is the master path
     * @return true if the path is master else false
     */
    bool isMaster() const;
    void setMaster(bool value);

    bool getMasterForFirstTime() const;
    void setMasterForFirstTime(bool masterForFirstTime);

    const GlobalEventStreamTypedef &getGlobalEventStream();

    PathManager &getPathManager();

    MarkovMatrix &getMarkovMatrix() const;

    /**
     * @brief getLocalEvent
     * @param eventIterator: iterator to the event (use the event Sn to return LocalEvent using map)
     * @return LocalEvent
     */
    shared_ptr<LocalEvent> getLocalEvent(GlobalEventStreamTypedef_iterator eventIterator);

    /**
     * @brief getLocalEvent
     * @param event: the event (use the event Sn to return LocalEvent using map)
     * @return LocalEvent
     */
    shared_ptr<LocalEvent> getLocalEvent(const shared_ptr<events::AbstractEvent> &event);

    /**
     * @brief getLocalEvent
     * @param index: index of the event
     * @return LocalEvent
     */
    shared_ptr<LocalEvent> getLocalEvent(size_t index) const;

    /**
     * @brief containLocalEvent: check if localEvents contains the passed event
     * @param event:
     * @return true or false :)
     */
    bool containLocalEvent(unsigned long eventSn) const;

    /**
     * @brief islocalEventUsed: check if the execution path has processed the passed event
     * @param event
     * @return true or false
     */
    bool islocalEventUsed(const shared_ptr<events::AbstractEvent> &event);

    /**
     * @brief islocalEventUsed: check if the execution path has processed the passed event
     * @param event sn
     * @return true or false
     */
    bool islocalEventUsed(unsigned long eventSn);

    /**
     * @brief getCurrentLocalEvent: get the event in index (feededLocalEventIndex)
     * NOTE: feededLocalEventIndex doesn't increment in this function. You should increment it!
     * @return  or null if index out of vector range
     *
     */
    shared_ptr<LocalEvent> getCurrentLocalEvent();

    /**
     * @brief getLocalEvents: return all local events (all events processed by Speculator)
     * @return : reference to vector of LocalEvents
     */
    const vector<shared_ptr<LocalEvent>> &getLocalEvents() const;

    void setFeededLocalEventIndex(size_t feededLocalEventIndex);
    size_t incrementFeededLocalEventIndex();
    size_t getFeededLocalEventIndex() const;

    bool getSelectionCompeleted() const;
    void setSelectionCompleted(bool selectionCompleted);


    bool isAllEventsFeeded() const;
    void setAllEventsFeeded(bool allEventsFeeded);
    bool checkAllEventsFeeded();

    unsigned long getLastFeededEventSN() const;
    void setLastFeededEventSN(unsigned long lastFeededEventSN);
    void incrementLastFeededEventSN();
    /**
     * @brief getEventLeftInSelection: return number of events left in the Selection
     * @param selectionSize:  a predicted Selection size
     * @return number of events left in the Selection
     */
    unsigned long getEventLeftInSelection(unsigned long selectionSize);

    /**
     * @brief createCheckpoint: save the state of the execution path
     * @param checkpointEvent: where the checkpoint takes place
     */
    void createCheckpoint(const shared_ptr<events::AbstractEvent> &checkpointingEvent);

    const vector<shared_ptr<Checkpoint>>& getCheckpoints() const;
    void setCheckpoints(const vector<shared_ptr<Checkpoint>> &checkpoints);
    shared_ptr<Checkpoint> getCheckpoint(size_t index) const;
    void addCheckpoint(const shared_ptr<Checkpoint> &checkpoint);

    const shared_ptr<Checkpoint>& getInitialCheckpoint() const;
    void setInitialCheckpoint(const shared_ptr<Checkpoint>& checkpoint);

    unsigned int getCheckpointingFrequency() const;
    void setCheckpointingFrequency(unsigned int value);

    unsigned int getCheckpointingCounter() const;
    void setCheckpointingCounter(unsigned int value);
    void incrementCheckpointingCounter();

    bool hasToCheckpoint();

    /**
     * @brief getRecoveryCheckpoint: search for any checkpoint before the passed event
     * @param eventSn: the event that causes invalidate the execution path
     * @return checkpoint
     */
    shared_ptr<Checkpoint> getRecoveryCheckpoint(unsigned long  eventSn);

    void recover(const shared_ptr<Checkpoint> &checkpoint);

    // cgroups
    unordered_map<unsigned long, pair< shared_ptr<Cgroup>, unsigned int> > &getCgroupsFromParent();
    void setCgroupsFromParent(const unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int> > &cgroupsFromParent);
    void addCgroupFromParent(const shared_ptr<Cgroup> &cgroup, unsigned int version);

    /**
     * @brief getUpdatedCgroupsFromParent: return a copy modified cgroups (update means that in a cgroup could be
     * the event are deleted or only new events are add)
     * @return: map of modified cgroups
     */
    unordered_map<unsigned long, shared_ptr<Cgroup>> getUpdatedCgroupsFromParent();

    unordered_map<unsigned long, shared_ptr<Cgroup> > &getClonedCgroupsFromParent();
    void setClonedCgroupsFromParent(const unordered_map<unsigned long, shared_ptr<Cgroup> > &clonedCgroupsFromParent);
    void setClonedCgroupsFromParent(const shared_ptr<Cgroup>  &cgroup);
    void addClonedCgroupFromParent(const shared_ptr<Cgroup> &cgroup);

    unordered_map<unsigned long, shared_ptr<Cgroup>> &getCgroupsToChildren();
    void setCgroupsToChildren(const unordered_map<unsigned long, shared_ptr<Cgroup>> &cgroupsToChildren);
    void addCgroupssToChildren(const shared_ptr<Cgroup>& cgroup);

    bool getFinished() const;
    void setFinished(bool value);

    void writeLock() const;
    bool writeTryLock() const;
    void writeUnlock() const;

    virtual ~ExecutionPath();


    int getNumberOfCgroupDelete() const;

private:
    // global unique Id: the execution paths have unique Ids
    unsigned long Id;

    // is this master path?
    atomic<bool> master={false};
    bool masterForFirstTime = true;

    GlobalEventStreamTypedef &globalEventStream;

    PathManager &pathManager;

    MarkovMatrix &markovMatrix;

    shared_ptr<AbstractSpeculator> speculator;
    shared_ptr<AbstractFeeder> feeder;
    shared_ptr<AbstractValidator> validator;
    shared_ptr<AbstractOperator> abstractOperator;
    shared_ptr<selection::AbstractSelection> abstractSelection;

    /*
     * following vector store all events that are processed by speculator and
     * that are forwarded to feeder
     */
    // vector to keep the event order
    vector<shared_ptr<LocalEvent>> localEvents;
    // quick access by using unordered_map (Sn (event) , index (in vector) )
    unordered_map<unsigned long, size_t> localEventsIndex;

    /*
     * - index of next event to feeded (send to feeder) from localEvents vector
     * - it has an iterator semantic where 0 means that the next event to feed is at index (0)
     */
    size_t feededLocalEventIndex = 0;

    // indicates whether the selection is closed and all events are fetched to the local vector (localEvents)
    bool selectionCompeleted = false;

    // indicates whether all events are sent to Feeder
    bool allEventsFeeded = false;

    bool finished=false;

    atomic<unsigned long> lastFeededEventSN;

    // all checkpoints
    vector<shared_ptr<Checkpoint>> checkpoints;
    shared_ptr<Checkpoint> initialCheckpoint;

    // Frequency of the checkpointing
    unsigned int checkpointingFrequency;
    // counter number of events to do checkpointing
    unsigned int checkpointingCounter = 0;

    profiler::Measurements* measurements;

    /*
     * store cgroups from parent (groupId, <cgroup*, version>):
     * Keep a copy of parent cgroups to avoid contention with the parent
     * Whenever the parent changes a cgroup (known by version), the local copy is updated using
     * original field in the cgroup which points to parent cgroup (not a copy of it)
     */
    unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int> > cgroupsFromParent;
    unordered_map<unsigned long, shared_ptr<Cgroup>> clonedCgroupsFromParent;

    // store the generated cgroups (groupId, cgroup*)
    unordered_map<unsigned long, shared_ptr<Cgroup>> cgroupsToChildren;

    // generate global unique Ids
    /*
     * used to generate unique Ids for the execution paths .
     * Use atomic because it is accessible from multiple workers
     */
    static atomic<unsigned long> lastExecutionPathId;

    mutable mutex mtx;

};
}

#endif // EXECUTIONPATH_HPP
