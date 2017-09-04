/*
 * Speculator.cpp
 *
 * Created on: 17.11.2016
 *      Author: sload
 */

#include "../../header/speculator/Speculator.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"
#include "../../header/path_manager/PathManager.hpp"

using namespace events;

namespace execution_path
{

Speculator::Speculator(ExecutionPath *executionPath) : AbstractSpeculator(executionPath) {}

Speculator::Speculator(const Speculator &other) : AbstractSpeculator(other) {}

shared_ptr<AbstractSpeculator> Speculator::clone() { return make_shared<Speculator>(*this); }

bool Speculator::main(size_t number)
{
    if (this->executionPath->getFinished())
    {
        //        cout<<"Execution path:"<< this->executionPath->getId() <<" is already finished"<<endl;
        return false;
    }

    if (this->executionPath->getCheckpoints().size() == 0)
    {
        this->executionPath->createCheckpoint(nullptr);
        this->executionPath->setInitialCheckpoint(this->executionPath->getCheckpoints()[0]);
    }

    size_t count = 0;
    bool result = false;
    while (number == 0 || count < number)
    {

        // Check if this is a master path. The order is important here!
        bool master = this->executionPath->isMaster();

        // check if it is first time that this execution path become a master
        bool masterForFirstTime = false;
        if (master)
        {
            if (this->executionPath->getMasterForFirstTime())
            {
                masterForFirstTime = true;
            }
            else
                masterForFirstTime = false;
        }

        // Checks fed events and puts new available events in localEvents vector
        result = this->execute(master, masterForFirstTime);

        // Call Feeder to feed events. It returns true if any event is fed
        if (result)
        {
            bool eventsFeeded = this->executionPath->getFeeder()->feedOperatorWithEvents();

            // Call operator instance
            if (eventsFeeded)
                this->executionPath->getOperator()->main();
        }

        // Call Validator
        // If this execution path is Master, call Validator to send complex events to Merger
        if (master)
        {
            if (this->executionPath->getMasterForFirstTime())
                this->executionPath->setMasterForFirstTime(false);
            /*
             *If Speculator already has processed all events then the Speculator sends terminate
             * signal to Validator which sends End_Of_Selection signal to Merger
             */
            bool terminate = false;
            if (this->executionPath->isAllEventsFeeded())
            {
                terminate = true;
            }

            this->executionPath->getValidator()->main(terminate, masterForFirstTime);

            if (terminate)
            {
                this->executionPath->setFinished(true);
                this->executionPath->getPathManager().setMasterExecutionPathFinished(true);
                //                cout << "Speculator: selection Id: " <<
                //                this->executionPath->getAbstractSelection()->getId()
                //                     << ", master finished, execution path Id: " <<
                //                     this->executionPath->getId()<<endl;
                return result;
            }
        }
        else // not master, check whether to create a check point
        {

            if (this->executionPath->hasToCheckpoint())
            {
                int index = this->executionPath->getFeededLocalEventIndex() - 1;
                if (index >= 0 && index < this->executionPath->getLocalEvents().size())
                {
                    shared_ptr<events::AbstractEvent> event = this->executionPath->getLocalEvent(index)->getEvent();
                    this->executionPath->setCheckpointingCounter(0);
                    this->executionPath->createCheckpoint(event);
                }
            }
        }

        count++;
    }

    return result;
}

void Speculator::recover() {}

bool Speculator::execute(bool master, bool masterForFirstTime)
{
    // check if it still needs to fetch the events from global shared list
    if (!this->executionPath->getSelectionCompeleted())
    {
        this->fetchEventsFromGlobalEventStream(1);
        //        this->fetchRangeOfEventsFromGlobalEventStream();
    }

    // if it is master, check for cgroup update only first time
    if (!master || (master && masterForFirstTime))
    {
        if (master || this->cgroupsUpdateCounter == 40)
        {
            this->cgroupsUpdateCounter = 0;
            // check if there is any update in cgroups
            unordered_map<unsigned long, shared_ptr<Cgroup>> cgroups
                = this->executionPath->getUpdatedCgroupsFromParent();

            // one or more cgroups have been updated
            if (cgroups.size() != 0)
                // check if any feeded event belongs to cgroups. if yes recover from checkpoint
                this->checkFeededEvents(cgroups);
        }
        else
            this->cgroupsUpdateCounter++;
    }

    // check if already all events are fed to operator instance.
    if (!this->executionPath->isAllEventsFeeded())
    {
        LocalEvent *localEvent = this->executionPath->getCurrentLocalEvent().get();
        // if all currently available events are already sent to feeder (no more events)
        if (localEvent == nullptr)
        {
            this->executionPath->checkAllEventsFeeded();
            return false;
        }

        bool feedCondition = this->feedEvent(localEvent->getEvent());

        if (feedCondition)
        {
            // send the event to the feeder
            this->executionPath->getFeeder()->pushNewEvent(localEvent->getEvent());
            localEvent->setFeed(true);
        }
        else
        {
            localEvent->setFeed(false);
        }
        this->executionPath->incrementFeededLocalEventIndex();

        if (!master)
            this->executionPath->incrementCheckpointingCounter();
        return true;
    }
    else
        return false;
}

void Speculator::fetchEventsFromGlobalEventStream(int number)
{
    /*
     * pointer to keep the reference to the  returned event and prevent copying it (which my be costly)
     */
    shared_ptr<events::AbstractEvent> *primitiveEvent = NULL;

    for (int i = 0; i < number; i++)
    {
        if (this->executionPath->getSelectionCompeleted())
            return;

        /*
         * check if the selection is finished (the selection is closed and all events already have been read)!
         */
        if (!this->executionPath->getAbstractSelection()->isSelectionCompleted())
        {

            // still there are events to read
            // get the next event iterator from the shared event stream
            auto eventIterator = this->executionPath->getAbstractSelection()->getCurrentPosition();
            primitiveEvent = this->executionPath->getGlobalEventStream().get(eventIterator);

            /*
             * Although the selection is not closed or/and there are/is still event(s) to read,
             * we already have read all events from the global stream.
             * So we should wait for new events.
             * Note: globalEventStream.get() function return NULL to indicate the end of event stream.
             * At this point we are sure that the selection has more events but we should wait for them
             */
            if (primitiveEvent == NULL)
            {
                return;
            }
            else
            {
                bool feed = false;
                shared_ptr<LocalEvent> localEvent = make_shared<LocalEvent>(*primitiveEvent, feed);

                this->executionPath->pushNewLocalEvent(localEvent);
                this->executionPath->getAbstractSelection()->incrementCurrentPosition();

                //                cout<<"Speculator: execution Path: "<<this->executionPath->getId()<<", fetch event:
                //                "<<primitiveEvent->get()->getSn()<<endl;
            }
        }
        else
        {
            this->executionPath->setSelectionCompleted(true);
            return;
        }
    }
}

void Speculator::fetchRangeOfEventsFromGlobalEventStream()
{
    if (this->executionPath->getSelectionCompeleted())
        return;

    // get current event position in the selection
    auto eventIterator = this->executionPath->getAbstractSelection()->getCurrentPosition();
    // get last event sn in the selection
    unsigned long lastEventSn = this->executionPath->getAbstractSelection()->getLastEventSn();

    vector<shared_ptr<AbstractEvent> *> primitiveEvents;
    primitiveEvents = this->executionPath->getGlobalEventStream().get(eventIterator, lastEventSn);

    for (auto it = primitiveEvents.begin(); it != primitiveEvents.end(); it++)
    {
        bool feed = false;
        shared_ptr<LocalEvent> localEvent = make_shared<LocalEvent>(*(*it), feed);

        this->executionPath->pushNewLocalEvent(localEvent);
        this->executionPath->getAbstractSelection()->incrementCurrentPosition();
    }

    //    cout<<"Speculator: SelectionId: "<< this->executionPath->getAbstractSelection()->getId() <<", ExecutionPath
    //    Id:"<<this->executionPath->getId() <<", fetched size: "<<primitiveEvents.size()<<endl;
    // check if the selection is completed

    if (this->executionPath->getAbstractSelection()->isSelectionCompleted())
        this->executionPath->setSelectionCompleted(true);
}

bool Speculator::feedEvent(const shared_ptr<AbstractEvent> &event) const
{
    // check if the event belongs to any Cgroups (local cgroups): if yes don't feed
    for (auto it = this->executionPath->getClonedCgroupsFromParent().begin();
         it != this->executionPath->getClonedCgroupsFromParent().end(); it++)
    {
        if (it->second.get()->contain(event->getSn()))
            return false;
    }

    return true;
}

void Speculator::checkFeededEvents(const unordered_map<unsigned long, shared_ptr<Cgroup>> &cgroups)
{
    /*
     * check if the execution path has used any event from cgroups in case the event added
     * to the cgroup later. Or if the execution didn't used an event that has been later deleted from
     * a cgroup!
     * if yes: then the path is wrong and should recover.
     * However, there still could be earlier events that are sed or not used, and should be considered, i.e
     *   even the check point could be wrong. Therefore, iterate over all cgroups and take the
     * event with smallest (SN) that was wrongly used or not used by this execution path
     */
    unsigned long wrongEventSn = 0;

    for (auto it = cgroups.begin(); it != cgroups.end(); it++)
    {
        auto &UpdatedCgroupEvents = it->second.get()->getEvents();
        if (this->executionPath->getClonedCgroupsFromParent()[it->first] == nullptr)
        {
            for (size_t index = 0; index < UpdatedCgroupEvents.size(); index++)
            {

                if (this->executionPath->islocalEventUsed(UpdatedCgroupEvents[index]))
                {
                    if ((wrongEventSn == 0) || (wrongEventSn > UpdatedCgroupEvents[index]))
                    {
                        wrongEventSn = UpdatedCgroupEvents[index];
                        //                        cout << "Speculator: execution path: " << this->executionPath->getId()
                        //                        << ", in update section"
                        //                             << this->executionPath->getAbstractSelection()->getId() << ",
                        //                             parent cgroup: " << it->first
                        //                             << ", wrong event: " << wrongEvent->getSn() << endl;
                    }
                    break; // go to next updated cgroup
                }
            }
        }

        else
        {
            auto &localCgroupEvents = this->executionPath->getClonedCgroupsFromParent()[it->first]->getEvents();

            for (size_t index = 0; index < UpdatedCgroupEvents.size(); index++)
            {

                /*
                 * Check if an event is deleted from UpdatedCgroupEvents: we only check if the events are in the right
                 * sequence. If the event is deleted, we consider it for the check pointing and stop checking
                 * for this cgroup
                 * Note: in delete case, the event is for sure not used (while in update case we should check)
                 */
                if (index < localCgroupEvents.size()) // check for delete
                {
                    // doesn't match => event is deleted
                    if (localCgroupEvents[index] != UpdatedCgroupEvents[index])
                    {
                        if ((wrongEventSn == 0) || (wrongEventSn > UpdatedCgroupEvents[index]))
                        {
                            wrongEventSn = UpdatedCgroupEvents[index];
                            //                        cout << "Speculator: execution path: " <<
                            //                        this->executionPath->getId()
                            //                             << ", in delete section parent cgroup: " << it->first
                            //                             << ", wrong event: " << wrongEvent->getSn() << endl;
                        }
                        break; // go to next updated cgroup
                    }
                }
                else // check for update
                {

                    if (this->executionPath->islocalEventUsed(UpdatedCgroupEvents[index]))
                    {
                        if ((wrongEventSn == 0) || (wrongEventSn > UpdatedCgroupEvents[index]))
                        {
                            wrongEventSn = UpdatedCgroupEvents[index];
                            //                        cout << "Speculator: execution path: " <<
                            //                        this->executionPath->getId()
                            //                             << ", in update section parent cgroup: " << it->first
                            //                             << ", wrong event: " << wrongEvent->getSn() << endl;
                        }
                        break; // go to next updated cgroup
                    }
                }
            }
        }

        // Now, we update the local cgroup with the updated ones
        this->executionPath->setClonedCgroupsFromParent(it->second);
    }

    if (wrongEventSn != 0) // there is an event that was wrongly used (fed) => recover
    {
        //        cout << "Speculator: selection ID: " << this->executionPath->getAbstractSelection()->getId()
        //             << ", execution path: " << this->executionPath->getId() << ", wrongly used/unused event: " <<
        //             wrongEventSn
        //             << endl;
        shared_ptr<Checkpoint> checkpoint = this->executionPath->getRecoveryCheckpoint(wrongEventSn);
        this->executionPath->recover(checkpoint);
    }
}

Speculator::~Speculator() {}
}
