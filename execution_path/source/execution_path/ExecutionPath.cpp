/*
 * ExecutionPath.cpp
 *
 * Created on: 06.10.2016
 *      Author: sload
 */

#include "../../header/execution_path/ExecutionPath.hpp"
#include "../../header/execution_path/Checkpoint.hpp"
#include "../../header/operator/AbstractOperator.hpp"
#include "../../header/path_manager/PathManager.hpp"
#include "../../header/speculator/AbstractSpeculator.hpp"
#include "../../header/speculator/Speculator.hpp"
#include "../../header/validator/AbstractValidator.hpp"
#include "../../header/validator/DeterministicValidator.hpp"

using namespace selection;
using namespace util;
using namespace events;
using namespace merger;
using namespace profiler;

namespace execution_path
{
// factory class
ExecutionPathFactory::ExecutionPathFactory(GlobalEventStreamTypedef *globalEventStream, PathManager *pathManager,
                                           MarkovMatrix *markovMatrix, Merger *merger, OperatorFactory *operatorFactory,
                                           unsigned int checkpointingFrequency, Measurements *measurements)
    : globalEventStream(globalEventStream), pathManager(pathManager), markovMatrix(markovMatrix), merger(merger),
      operatorFactory(operatorFactory), checkpointingFrequency(checkpointingFrequency), measurements(measurements)

{
}

shared_ptr<ExecutionPath> ExecutionPathFactory::createNewExecutionPath(shared_ptr<AbstractSelection> abstractSelection)
{
    shared_ptr<ExecutionPath> executionPath = make_shared<ExecutionPath>(
        *this->globalEventStream, *this->pathManager, abstractSelection, *this->markovMatrix, *this->merger,
        *this->operatorFactory, this->checkpointingFrequency, this->measurements);

    //    executionPath->getSpeculator()->setExecutionPath(executionPath->getSharedPtr());
    //    executionPath->getValidator()->setExecutionPath(executionPath->getSharedPtr());
    //    executionPath->getFeeder()->setExecutionPath(executionPath->getSharedPtr());

    return executionPath;
}

void ExecutionPathFactory::setPathManager(PathManager *pathManager) { this->pathManager = pathManager; }

ExecutionPathFactory::~ExecutionPathFactory() {}

// execution path class
//----------------------------------------------------------------
unsigned long ExecutionPath::generateExecutionPathId() { return lastExecutionPathId.operator++(); }
atomic<unsigned long> ExecutionPath::lastExecutionPathId{0};

ExecutionPath::ExecutionPath(GlobalEventStreamTypedef &globalEventStream, PathManager &pathManager,
                             shared_ptr<AbstractSelection> abstractSelection, MarkovMatrix &markovMatrix,
                             Merger &merger, const OperatorFactory &operatorFactory,
                             unsigned int checkpointingFrequency, Measurements *measurements)
    : Id(generateExecutionPathId()), globalEventStream(globalEventStream), pathManager(pathManager),
      markovMatrix(markovMatrix), abstractSelection(abstractSelection), checkpointingFrequency(checkpointingFrequency),
      measurements(measurements)
{

    this->validator = make_shared<DeterministicValidator>(this, merger, measurements); //*this

    this->abstractOperator = operatorFactory.createOperator(this->validator.get());

    this->feeder
        = make_shared<DeterministicFeeder>(globalEventStream, this, this->abstractOperator.get(), measurements);

    this->speculator = make_shared<Speculator>(this);

    lastFeededEventSN.store(abstractSelection->getStartPosition()->get()->getSn());

    this->cgroupsFromParent.reserve(10);
    this->clonedCgroupsFromParent.reserve(10);
}

ExecutionPath::ExecutionPath(const ExecutionPath &other)
    : enable_shared_from_this<ExecutionPath>(), globalEventStream(other.globalEventStream),
      pathManager(other.pathManager), markovMatrix(other.markovMatrix)
{
    this->master.store(other.master.load());
    this->masterForFirstTime = other.masterForFirstTime;

    this->speculator = other.speculator.get()->clone(); // deep copy
    this->speculator->setExecutionPath(this);

    this->validator = other.validator.get()->clone(); // deep copy
    this->validator->setExecutionPath(this);

    this->abstractOperator = other.abstractOperator.get()->clone(); // deep copy
    this->abstractOperator->setValidator(this->validator.get());

    this->feeder = other.feeder.get()->clone(); // deep copy
    this->feeder->setExecutionPath(this);
    this->feeder->setAbstractOperator(this->abstractOperator.get());

    this->abstractSelection = other.abstractSelection.get()->clone(); // deep copy

    this->localEvents = other.localEvents;
    this->localEventsIndex = other.localEventsIndex;
    this->feededLocalEventIndex = other.feededLocalEventIndex;

    this->selectionCompeleted = other.selectionCompeleted;
    this->allEventsFeeded = other.allEventsFeeded;

    this->finished = other.finished;

    this->lastFeededEventSN.store(other.lastFeededEventSN.load());

    this->checkpoints = other.checkpoints;
    this->initialCheckpoint = other.initialCheckpoint;
    this->checkpointingFrequency = other.checkpointingFrequency;
    this->checkpointingCounter = other.checkpointingCounter;

    this->measurements = other.measurements;

    this->cgroupsFromParent = other.cgroupsFromParent;
    this->clonedCgroupsFromParent = other.clonedCgroupsFromParent;

    // clone cgroups that have been sent to path manager (to children)
    // as already operator clones cgroups there is no need to clone them again
    //    for (auto it = other.cgroupsToChildren.begin(); it != other.cgroupsToChildren.end(); it++)
    //    {
    //        this->cgroupsToChildren[it->first] = it->second->clone(); // clone cgroups
    //    }
    this->cgroupsToChildren = other.cgroupsToChildren;
}

shared_ptr<ExecutionPath> ExecutionPath::clone()
{

    shared_ptr<ExecutionPath> executionPath = make_shared<ExecutionPath>(*this);
    executionPath->Id = generateExecutionPathId(); // assign unique Id
    return executionPath;
}

shared_ptr<ExecutionPath> ExecutionPath::cloneForCheckpointing()
{
    shared_ptr<ExecutionPath> executionPath = make_shared<ExecutionPath>(*this);
    // no need to assign new id (unique id) because we generate an unique Id when we use this clone
    executionPath->Id = this->Id;
    return executionPath;
}

shared_ptr<ExecutionPath> ExecutionPath::getSharedPtr() { return shared_from_this(); }

unsigned long ExecutionPath::getId() { return this->Id; }

bool ExecutionPath::isMaster() const { return this->master.load(); }

void ExecutionPath::setMaster(bool value) { this->master.store(value); }

bool ExecutionPath::getMasterForFirstTime() const { return this->masterForFirstTime; }

void ExecutionPath::setMasterForFirstTime(bool masterForFirstTime) { this->masterForFirstTime = masterForFirstTime; }

const ExecutionPath::GlobalEventStreamTypedef &ExecutionPath::getGlobalEventStream() { return this->globalEventStream; }

PathManager &ExecutionPath::getPathManager() { return this->pathManager; }

MarkovMatrix &ExecutionPath::getMarkovMatrix() const { return this->markovMatrix; }

const shared_ptr<AbstractSpeculator> &ExecutionPath::getSpeculator() const { return this->speculator; }

const shared_ptr<AbstractFeeder> &ExecutionPath::getFeeder() const { return this->feeder; }

const shared_ptr<AbstractValidator> &ExecutionPath::getValidator() const { return this->validator; }

const shared_ptr<AbstractOperator> &ExecutionPath::getOperator() const { return this->abstractOperator; }

const shared_ptr<AbstractSelection> &ExecutionPath::getAbstractSelection() const { return this->abstractSelection; }

void ExecutionPath::setOperator(const shared_ptr<AbstractOperator> &abstractOperator)
{
    this->abstractOperator = abstractOperator;
}

void ExecutionPath::createCheckpoint(const shared_ptr<AbstractEvent> &checkpointingEvent)
{
    shared_ptr<ExecutionPath> executionPath = this->cloneForCheckpointing();

    shared_ptr<Checkpoint> checkpoint = make_shared<Checkpoint>(executionPath, checkpointingEvent);

    this->checkpoints.push_back(checkpoint);
}

void ExecutionPath::pushNewLocalEvent(const shared_ptr<LocalEvent> &localEvent)
{
    this->localEvents.push_back(localEvent);
    this->localEventsIndex[localEvent->getEvent()->getSn()] = this->localEvents.size() - 1;
}

shared_ptr<LocalEvent> ExecutionPath::getLocalEvent(ExecutionPath::GlobalEventStreamTypedef_iterator eventIterator)
{
    return this->getLocalEvent(*eventIterator);
}

shared_ptr<LocalEvent> ExecutionPath::getLocalEvent(const shared_ptr<AbstractEvent> &event)
{
    if (this->localEventsIndex.count(event->getSn()) == 0)
        return nullptr;

    size_t index = this->localEventsIndex[event->getSn()];
    return this->localEvents[index];
}

shared_ptr<LocalEvent> ExecutionPath::getLocalEvent(size_t index) const
{
    if (index >= this->localEvents.size())
        return nullptr;

    return this->localEvents[index];
}

bool ExecutionPath::containLocalEvent(unsigned long eventSn) const
{
    if (this->localEventsIndex.count(eventSn) == 0)
        return false;
    else
        return true;
}

bool ExecutionPath::islocalEventUsed(unsigned long eventSn)
{
    // the event outside the current available events in the execution path
    if (this->localEventsIndex.count(eventSn) == 0)
        return false;
    else
    {
        size_t index = this->localEventsIndex[eventSn];
        // return whether the event is fed or not!
        return this->localEvents[index]->getFeed();
    }
}

shared_ptr<LocalEvent> ExecutionPath::getCurrentLocalEvent()
{
    if (this->feededLocalEventIndex >= this->localEvents.size())
        return nullptr;

    return this->getLocalEvent(this->feededLocalEventIndex);
}

const vector<shared_ptr<LocalEvent>> &ExecutionPath::getLocalEvents() const { return this->localEvents; }

void ExecutionPath::setFeededLocalEventIndex(size_t feededLocalEventIndex)
{
    this->feededLocalEventIndex = feededLocalEventIndex;
}

size_t ExecutionPath::incrementFeededLocalEventIndex()
{
    this->feededLocalEventIndex++;

    if (this->selectionCompeleted && (this->feededLocalEventIndex == this->localEvents.size()))
        allEventsFeeded = true;

    return this->feededLocalEventIndex;
}

size_t ExecutionPath::getFeededLocalEventIndex() const { return this->feededLocalEventIndex; }

bool ExecutionPath::getSelectionCompeleted() const { return selectionCompeleted; }

void ExecutionPath::setSelectionCompleted(bool selectionCompleted) { selectionCompeleted = selectionCompleted; }

bool ExecutionPath::isAllEventsFeeded() const { return this->allEventsFeeded; }

void ExecutionPath::setAllEventsFeeded(bool allEventsFeeded) { this->allEventsFeeded = allEventsFeeded; }

bool ExecutionPath::checkAllEventsFeeded()
{
    if (this->selectionCompeleted && (this->feededLocalEventIndex == this->localEvents.size()))
        allEventsFeeded = true;

    return allEventsFeeded;
}

unsigned long ExecutionPath::getLastFeededEventSN() const { return lastFeededEventSN; }

void ExecutionPath::setLastFeededEventSN(unsigned long lastFeededEventSN)
{
    this->lastFeededEventSN.store(lastFeededEventSN);
}

void ExecutionPath::incrementLastFeededEventSN() { this->lastFeededEventSN.fetch_add(1); }

unsigned long ExecutionPath::getEventLeftInSelection(unsigned long selectionSize)
{
    unsigned long processedEvents
        = (lastFeededEventSN.load() - this->abstractSelection->getStartPosition()->get()->getSn());

    return selectionSize > processedEvents ? selectionSize - processedEvents : 0;
}

const vector<shared_ptr<Checkpoint>> &ExecutionPath::getCheckpoints() const { return this->checkpoints; }

void ExecutionPath::setCheckpoints(const vector<shared_ptr<Checkpoint>> &checkpoints)
{
    this->checkpoints = checkpoints;
}

shared_ptr<Checkpoint> ExecutionPath::getCheckpoint(size_t index) const
{
    if (index >= this->checkpoints.size())
        return nullptr;
    else
        return this->checkpoints[index];
}

void ExecutionPath::addCheckpoint(const shared_ptr<Checkpoint> &checkpoint) { this->checkpoints.push_back(checkpoint); }

const shared_ptr<Checkpoint> &ExecutionPath::getInitialCheckpoint() const { return initialCheckpoint; }

void ExecutionPath::setInitialCheckpoint(const shared_ptr<Checkpoint> &checkpoint)
{
    this->initialCheckpoint = checkpoint;
}

unsigned int ExecutionPath::getCheckpointingFrequency() const { return checkpointingFrequency; }

void ExecutionPath::setCheckpointingFrequency(unsigned int value) { checkpointingFrequency = value; }

unsigned int ExecutionPath::getCheckpointingCounter() const { return checkpointingCounter; }

void ExecutionPath::setCheckpointingCounter(unsigned int value) { checkpointingCounter = value; }

void ExecutionPath::incrementCheckpointingCounter() { this->checkpointingCounter++; }

bool ExecutionPath::hasToCheckpoint()
{
    if ((this->checkpointingFrequency != 0) && (this->localEvents.size() != 0)
        && (this->checkpointingFrequency - this->checkpointingCounter == 0))
        return true;
    else
        return false;
}

shared_ptr<Checkpoint> ExecutionPath::getRecoveryCheckpoint(unsigned long eventSn)
{
    if (eventSn == 0)
    {
        // remove all checkpoints except the initial one
        this->checkpoints.erase(this->checkpoints.begin() + 1, this->checkpoints.end());

        //        cout << "selection ID: " << this->getAbstractSelection()->getId() << ",  execution Path: " << this->Id
        //             << ", recovery check point at position: 0" << endl;
        // initial checkpoint in the execution path (first checkpoint)
        return this->checkpoints[0];
    }

    for (int index = this->checkpoints.size() - 1; index >= 0; index--)
    {
        // initial checkpoint in the execution path (first checkpoint)
        if (index == 0)
        {
            // remove all checkpoints except the initial one
            this->checkpoints.erase(this->checkpoints.begin() + 1, this->checkpoints.end());

            //            cout << "selection ID: " << this->getAbstractSelection()->getId() << ", execution Path: " <<
            //            this->Id
            //                 << ", recovery check point at position: 0" << endl;
            return this->checkpoints[index];
        }

        // checkpointing event is before the passed event=> valid checkpoint
        if (this->checkpoints[index]->getCheckpointingEvent()->getSn() < eventSn)
        {
            //            cout << "selection ID: " << this->getAbstractSelection()->getId() << ", execution Path: " <<
            //            this->Id
            //                 << ", recovery check point at position: " << index << endl;

            return this->checkpoints[index];
        }
        else
        {
            // invalid checkpoint=> remove it
            this->checkpoints.erase(this->checkpoints.begin() + index);
        }
    }
    // nothing found (it is impossible!)
    return nullptr;
}

void ExecutionPath::recover(const shared_ptr<Checkpoint> &checkpoint)
{
    shared_ptr<ExecutionPath> other = checkpoint->getExecutionPath()->cloneForCheckpointing();
    this->feeder = other->feeder;
    this->feeder->setExecutionPath(this);
    this->feeder->setAbstractOperator(this->abstractOperator.get());

    this->validator = other->validator;
    this->validator->setExecutionPath(this);

    //    this->abstractOperator = other->abstractOperator;
    this->abstractOperator->recover(other->abstractOperator.get());

    this->feededLocalEventIndex = other->feededLocalEventIndex;

    this->allEventsFeeded = other->allEventsFeeded;

    this->finished = other->finished;

    this->lastFeededEventSN.store(other->lastFeededEventSN.load());

    /*
     * No need to copy checkpoints vector as all invalid check points are deleted when
     * getRecoveryCheckpoint searches for the right check point
     */
    this->checkpointingCounter = other->checkpointingCounter;

    /*
     * cgroupsToChildren:
     * - this code block to notify path manager and increment version in the updated cgroups
     * so the children can recover!
     * -check generated cgroups which are invalid and already have been sent to the path manager
     */

    for (auto it_this = this->cgroupsToChildren.begin(); it_this != this->cgroupsToChildren.end();)
    {
        // check if the cgroup was in the check point, if not it should be deleted from children
        if (other->cgroupsToChildren.count(it_this->first) == 0) // was not in the check point
        {
            this->validator->receiveCgroup(it_this->second, Cgroup::Status::DELETE);
            it_this = this->cgroupsToChildren.erase(it_this);
        }
        else // check for update
        {
            // as the operator do recovery and set cgroup content to the one in check point, there is no need to
            // do it here again

            /*
             * -It was in the check point, so check if it is correct
             * - Each used event after the check point is wrong
             */
            /* if (other->cgroupsToChildren[it_this->first]->getVersion() != it_this->second->getVersion())
             {
                 // update cgroup fields
                 it_this->second->set(*other->cgroupsToChildren[it_this->first]);

                 // it is updated, update the version so the children can check for update
                 it_this->second->setVersion(it_this->second->getVersion() + 1);
             }*/

            it_this++;
        }
    }
}

unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &ExecutionPath::getCgroupsFromParent()
{
    return this->cgroupsFromParent;
}

void ExecutionPath::setCgroupsFromParent(
    const unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &cgroupsFromParent)
{
    this->cgroupsFromParent = cgroupsFromParent;
}

void ExecutionPath::addCgroupFromParent(const shared_ptr<Cgroup> &cgroup, unsigned int version)
{
    this->cgroupsFromParent[cgroup->getId()] = make_pair(cgroup, version);
}

unordered_map<unsigned long, shared_ptr<Cgroup>> ExecutionPath::getUpdatedCgroupsFromParent()
{
    unordered_map<unsigned long, shared_ptr<Cgroup>> cgroups;

    //    cout<<"Execution Path: "<< this->Id<<", cgroup parent size:"<< cgroupsFromParent.size()<<endl;
    // check if the cgroups are changed by parent. use original pointer and version
    for (auto it = cgroupsFromParent.begin(); it != cgroupsFromParent.end(); it++)
    {

        //        if (it->second->getOriginal() == nullptr)
        //            continue;

        // if cgroup is changed in the parent, clone it from the parent!
        unsigned int parentVersion = it->second.first->getVersion();
        if (it->second.second != parentVersion)
        {

            // add it to updated cgroups
            cgroups[it->first] = it->second.first->getClonedCopy();
            // replicate to bring cgroup to local worker's memory and to reduce the contention between workers
//            cgroups[it->first] = cgroups[it->first]->clone();
            it->second.second = parentVersion;
        }
    }

    return cgroups;
}

unordered_map<unsigned long, shared_ptr<Cgroup>> &ExecutionPath::getClonedCgroupsFromParent()
{
    return this->clonedCgroupsFromParent;
}

void ExecutionPath::setClonedCgroupsFromParent(
    const unordered_map<unsigned long, shared_ptr<Cgroup>> &clonedCgroupsFromParent)
{
    this->clonedCgroupsFromParent = clonedCgroupsFromParent;
}

void ExecutionPath::setClonedCgroupsFromParent(const shared_ptr<Cgroup> &cgroup)
{
    this->clonedCgroupsFromParent[cgroup->getId()] = cgroup;
}

void ExecutionPath::addClonedCgroupFromParent(const shared_ptr<Cgroup> &cgroup)
{
    this->clonedCgroupsFromParent[cgroup->getId()] = cgroup;
}

unordered_map<unsigned long, shared_ptr<Cgroup>> &ExecutionPath::getCgroupsToChildren()
{
    return this->cgroupsToChildren;
}

void ExecutionPath::setCgroupsToChildren(const unordered_map<unsigned long, shared_ptr<Cgroup>> &cgroupsToChildren)
{
    this->cgroupsToChildren = cgroupsToChildren;
}

void ExecutionPath::addCgroupssToChildren(const shared_ptr<Cgroup> &cgroup)
{
    this->cgroupsToChildren.insert(make_pair(cgroup->getId(), cgroup));
}

bool ExecutionPath::getFinished() const { return finished; }

void ExecutionPath::setFinished(bool value) { finished = value; }

void ExecutionPath::writeLock() const { this->mtx.lock(); }

bool ExecutionPath::writeTryLock() const { return this->mtx.try_lock(); }

void ExecutionPath::writeUnlock() const { this->mtx.unlock(); }

ExecutionPath::~ExecutionPath() {}
}
