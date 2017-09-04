/*
 * DeterministicValidator.cpp
 *
 * Created on: 12.10.2016
 *      Author: sload
 */

#include "../../header/validator/DeterministicValidator.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"
#include "../../header/path_manager/PathManager.hpp"

using namespace selection;
using namespace events;
using namespace merger;
using namespace util;
using namespace profiler;

namespace execution_path
{
DeterministicValidator::DeterministicValidator(ExecutionPath *executionPath, Merger &merger, Measurements *measurements)
    : AbstractValidator(executionPath, merger, measurements)
{
}

DeterministicValidator::DeterministicValidator(const DeterministicValidator &other)
    : AbstractValidator(other), complexEvents(other.complexEvents), bufferedCgroups(other.bufferedCgroups)
{
}

shared_ptr<AbstractValidator> DeterministicValidator::clone() { return make_shared<DeterministicValidator>(*this); }

void DeterministicValidator::main(bool terminate, bool masterForFirstTime)
{
    // the execution path becomes master for first time
    if (masterForFirstTime)
        this->processFirstTimeMaster();

    writeComplexEventsToFile();

    // The execution path will terminate (it is master path)
    //    sendComplexEventsToMerger();

    if (terminate)
    {
        inValidateUncompletedCgroups();
        //        sendSelectionTerminationEventToMerger();
    }
}

void DeterministicValidator::receiveComplexEvent(shared_ptr<ComplexEvent> complexEvent)
{
    this->complexEvents.push_back(complexEvent);
}

void DeterministicValidator::receiveCgroup(shared_ptr<Cgroup> cgroup, Cgroup::Status status)
{
    switch (status)
    {
    case Cgroup::Status::NEW:
        processNewCgroup(cgroup);
        break;
    case Cgroup::Status::UPDATE:
        processUpdatedCgroup(cgroup);
        break;
    case Cgroup::Status::DELETE:
        processDeletedCgroup(cgroup);
        break;
    }
}

size_t DeterministicValidator::getComplexEventSize() const { return this->complexEvents.size(); }

void DeterministicValidator::processNewCgroup(const shared_ptr<Cgroup> &cgroup)
{
    //        cout << "Validator: selection Id: " << this->executionPath->getAbstractSelection()->getId()
    //             << ", execution path: " << this->executionPath->getId() << ", new Cgroup: " << cgroup->getId() <<
    //             endl;

    // set execution path Id
    cgroup->setExecutionPathId(this->executionPath->getId());
    // set original pointer
    //    cgroup->setOriginal(cgroup->getSharedPtr());

    this->executionPath->addCgroupssToChildren(cgroup);

    // buffer all cgroups if this is not master path
    if (!this->executionPath->isMaster())
    {
        bufferedCgroups[cgroup->getId()].push_back(
            pair<unsigned int, unsigned long>(cgroup->getEventsLeft(), cgroup->getEvents().back()));
    }
    else
    {
        // send the cgroup to Markov model
        this->executionPath->getMarkovMatrix().receiveCGroup(cgroup->getId(), cgroup->getEventsLeft(),
                                                             cgroup->getEvents().back());
    }

    // clone to avoid data race on cgroup (write by parent, read by children and Markov model)
    // clone is done in path manager. See branch function!
    //    shared_ptr<Cgroup> cgroupClone = cgroup->clone();

    // send cgroup to path manager
    this->executionPath->getPathManager().receiveCgroup(cgroup, Cgroup::Status::NEW);
}

void DeterministicValidator::processUpdatedCgroup(const shared_ptr<Cgroup> &cgroup)
{
    if (this->executionPath->isMaster() && !this->executionPath->getMasterForFirstTime())
    {
        // check if the cgroup is completed
        if (cgroup->getEventsLeft() == 0) // completed
        {
            cgroup->setValidation(Cgroup::Validation::VALID);
            // send cgroup to path manager
            this->executionPath->getPathManager().receiveCgroup(cgroup, Cgroup::Status::UPDATE);

            this->executionPath->getCgroupsToChildren().erase(cgroup->getId());

            //                        cout << "Validator: selection Id: " <<
            //                        this->executionPath->getAbstractSelection()->getId()
            //                             << ", execution path: " << this->executionPath->getId()
            //                             << ", Update Cgroup (VALID): " << cgroup->getId() << endl;
        }

        // send the cgroup to Markov model
        this->executionPath->getMarkovMatrix().receiveCGroup(cgroup->getId(), cgroup->getEventsLeft(),
                                                             cgroup->getEvents().back());
    }
    else
    {
        bufferedCgroups[cgroup->getId()].push_back(
            pair<unsigned int, unsigned long>(cgroup->getEventsLeft(), cgroup->getEvents().back()));
    }
}

void DeterministicValidator::processDeletedCgroup(const shared_ptr<Cgroup> &cgroup)
{
    //        cout << "Validator: execution path " << this->executionPath->getId() << ", Delete Cgroup: " <<
    //        cgroup->getId()
    //             << endl;

    cgroup->setValidation(Cgroup::Validation::DELETED);
    // send cgroup to path manager
    this->executionPath->getPathManager().receiveCgroup(cgroup, Cgroup::Status::DELETE);

    // if the cgroup is already in the buffer, remove it
    for (auto it = bufferedCgroups.begin(); it != bufferedCgroups.end();)
        if (it->first == cgroup->getId())
        {
            it = bufferedCgroups.erase(it);
            break;
        }
        else
            it++;
}

void DeterministicValidator::processFirstTimeMaster()
{
    // send completed cgroups to path manager
    for (auto it = this->executionPath->getCgroupsToChildren().begin();
         it != this->executionPath->getCgroupsToChildren().end();)
    {
        // check if the cgroup is completed
        if (it->second->getEventsLeft() == 0) // completed
        {
            it->second->setValidation(Cgroup::Validation::VALID);
            // send cgroup to path manager
            this->executionPath->getPathManager().receiveCgroup(it->second, Cgroup::Status::UPDATE);

            //                        cout << "Validator: selection Id: " <<
            //                        this->executionPath->getAbstractSelection()->getId()
            //                             << ", execution path: " << this->executionPath->getId()
            //                             << ", Update Cgroup (VALID): " << it->second->getId() << endl;
            it = this->executionPath->getCgroupsToChildren().erase(it);
            //            it++;
        }
        else
            it++;
    }

    // If any cgroups was buffered, send them to Markov model
    this->sendBufferedCgroupsToMarkov();
}

void DeterministicValidator::sendComplexEventsToMerger()
{
    unsigned long selectionId = this->executionPath->getAbstractSelection()->getId();
    for (auto it = this->complexEvents.begin(); it != this->complexEvents.end(); it++)
        this->merger.receiveEvent(selectionId, *it);
}

void DeterministicValidator::sendSelectionTerminationEventToMerger()
{
    // send end of selection event to merger to indicate the end of the selection
    unsigned long selectionId = this->executionPath->getAbstractSelection()->getId();
    EventFactory eventFactory(Constants::EventType::END_OF_SELECTION, this->measurements);
    shared_ptr<AbstractEvent> endOfSelectionEvent = eventFactory.createNewEvent();
    this->merger.receiveEvent(selectionId, endOfSelectionEvent);
}

void DeterministicValidator::inValidateUncompletedCgroups()
{
    // send uncompleted cgroups to path manager
    for (auto it = this->executionPath->getCgroupsToChildren().begin();
         it != this->executionPath->getCgroupsToChildren().end(); it++)
    {
        // check if the cgroup is completed
        if (it->second->getEventsLeft() != 0) // not completed
        {
            //            it->second->setValidation(Cgroup::Validation::INVALID);
            //            // send cgroup to path manager
            //            this->executionPath.getPathManager().receiveCgroup(it->second, Cgroup::Status::UPDATE);

            // The above commented code is correct. However, invalidate also is same as delete So

            it->second->setValidation(Cgroup::Validation::INVALID);
            this->executionPath->getPathManager().receiveCgroup(it->second, Cgroup::Status::DELETE);

            //                        cout << "Validator: selection Id: " <<
            //                        this->executionPath->getAbstractSelection()->getId()
            //                             << ", execution path: " << this->executionPath->getId() << ", InValid Cgroup:
            //                             " <<
            //                             it->second->getId()
            //                             << endl;
        }
    }
}

void DeterministicValidator::sendBufferedCgroupsToMarkov()
{
    map<unsigned long, vector<pair<unsigned int, unsigned long>>> orderedBuffer(bufferedCgroups.begin(),
                                                                                bufferedCgroups.end());

    for (auto it = orderedBuffer.begin(); it != orderedBuffer.end(); it++)
    {
        for (auto it_vec = it->second.begin(); it_vec != it->second.end(); it_vec++)
        {
            this->executionPath->getMarkovMatrix().receiveCGroup(it->first, it_vec->first, it_vec->second);
        }
    }

    bufferedCgroups.clear();
}

void DeterministicValidator::writeComplexEventsToFile()
{
    ofstream outputFile;
    outputFile.open("./complexEvents.txt", ios::app);

    unsigned long selectionId = this->executionPath->getAbstractSelection()->getId();
    for (auto it = this->complexEvents.begin(); it != this->complexEvents.end(); it++)
    {
        unsigned long detectionLatency = Helper::currentTimeMillis() - it->get()->getRealTimeStamp();
        measurements->getDetectionLatencyStat()->addNewEvent(it->get()->getSn(), detectionLatency);

        outputFile << "{SelectionId:" << selectionId << "}, " << it->get()->toString() << endl;
    }

    outputFile.close();
    this->complexEvents.clear();
}

DeterministicValidator::~DeterministicValidator()
{
    // TODO Auto-generated destructor stub
}
}
/* namespace execution_path */
