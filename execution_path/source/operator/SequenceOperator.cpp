/*
 * SequenceOperator.cpp
 *
 *  Created on: Aug 16, 2016
 *      Author: sload
 */

#include "../../header/operator/SequenceOperator.hpp"
#include "../header/execution_path/ExecutionPath.hpp"

using namespace events;
using namespace util;
using namespace profiler;

namespace execution_path
{

/**
 * Ctor
 * @param validator: pointer to the Validator
 * @param eventStates: an array of event types which will be detected in a sequence order
 * @param workLoad: artificial work load in case of generating events
 * @param timeMeasurement: for profiling
 */
SequenceOperator::SequenceOperator(AbstractValidator *validator, unsigned int patternSize, vector<string> eventStates,
                                   unsigned long workLoad, Measurements *measurements)
    : AbstractOperator(validator, measurements), cgroups(4, nullptr), partialMatchedEvents(4)
{
    this->patternSize = patternSize;
    this->eventStates = eventStates;
    this->workLoad = workLoad;
}

SequenceOperator::SequenceOperator(const SequenceOperator &other)
    : AbstractOperator(other), cgroups(4, nullptr), partialMatchedEvents(4)
{
    this->patternSize = other.patternSize;
    this->eventStates = other.eventStates;
    //    this->cgroupHasBeenOpend = other.cgroupHasBeenOpend;
    this->cgroupHasBeenOpend_1 = other.cgroupHasBeenOpend_1;
    this->cgroupHasBeenOpend_2 = other.cgroupHasBeenOpend_2;
    this->cgroupHasBeenOpend_3 = other.cgroupHasBeenOpend_3;
    this->cgroupHasBeenOpend_4 = other.cgroupHasBeenOpend_4;

    for (size_t i = 0; i < other.cgroups.size(); i++)
        if (other.cgroups[i] != nullptr)
            this->cgroups[i] = other.cgroups[i];
    //    if (other.cgroup)
    //        this->cgroup = other.cgroup->clone();

    this->partialMatchedEvents = other.partialMatchedEvents;

    this->workLoad = other.workLoad;
}

shared_ptr<AbstractOperator> SequenceOperator::clone() { return make_shared<SequenceOperator>(*this); }

void SequenceOperator::recover(AbstractOperator *other)
{
    AbstractOperator::recover(other);
    SequenceOperator *rhs = static_cast<SequenceOperator *>(other);

    this->patternSize = rhs->patternSize;
    this->eventStates = rhs->eventStates;
    //    this->cgroupHasBeenOpend = rhs->cgroupHasBeenOpend;
    this->cgroupHasBeenOpend_1 = rhs->cgroupHasBeenOpend_1;
    this->cgroupHasBeenOpend_2 = rhs->cgroupHasBeenOpend_2;
    this->cgroupHasBeenOpend_3 = rhs->cgroupHasBeenOpend_3;
    this->cgroupHasBeenOpend_4 = rhs->cgroupHasBeenOpend_4;

    //    if (rhs->cgroup)
    //        this->cgroup->set(*rhs->cgroup);
    //    else
    //        this->cgroup = nullptr;

    for (size_t i = 0; i < rhs->cgroups.size(); i++)
        if (rhs->cgroups[i] != nullptr)
            this->cgroups[i]->set(*rhs->cgroups[i]);
        else
            this->cgroups[i] = nullptr;

    this->partialMatchedEvents = rhs->partialMatchedEvents;

    this->workLoad = rhs->workLoad;
}

void SequenceOperator::main()
{
    //    this->normalOperator();

//            this->oneCgroupPerWindow();
//    this->twoCgroupPerWindow();
//        this->threeCgroupPerWindow();
            this->fourCgroupPerWindow();
}

void SequenceOperator::normalOperator()
{
    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        this->simulateLoad();

        // get event's content from AbstractEvent( it contains only the event type i.e. A, B, etc.)
        string eventContent;
        for (auto it_eventType = it_primitive->get()->getContent().begin();
             it_eventType != it_primitive->get()->getContent().end(); it_eventType++)
            eventContent = it_eventType->second;

        // find its position in eventType vector (to define its position in the sequence operator)
        int position = -1;
        for (size_t index = 0; index < this->eventStates.size(); index++)
            if (eventContent == this->eventStates[index])
            {
                position = index;
                break;
            }

        if (position == -1)
            continue;

        // Pattern size is one, so each event is a Complex event :)!
        if (this->eventStates.size() == 1)
        {
            vector<shared_ptr<AbstractEvent>> cplxEvents;
            cplxEvents.push_back(*it_primitive);
            shared_ptr<ComplexEvent> complexEvent
                = make_shared<ComplexEvent>(it_primitive->get()->getSn(), cplxEvents, this->measurements);

            this->validator->receiveComplexEvent(complexEvent);
        }
        else // Pattern size is greater than one
        {
            size_t patternPartIndex = 0;
            if (this->cgroupHasBeenOpend_1 == false) // can only have one cgroup
            {
                if (position == 0) // fist event, only we need to add it to the automate
                {
                    //                    this->simulateLoad2();
                    // create new cgroup and add it to cgroups map
                    shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                    cgroups[patternPartIndex] = newCgroup;

                    //                cgroup->pushNewEvent(*it_primitive);
                    cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                    cgroups[patternPartIndex]->setEventsLeft(this->patternSize - 1);

                    // send created cgroup to the Validator
                    this->validator->receiveCgroup(cgroups[patternPartIndex], Cgroup::Status::NEW);

                    // create a new partial match
                    this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);
                    cgroupHasBeenOpend_1 = true;
                }
            }
            else
            {
                // find the event position in the automate (seq operator)
                //                if (this->cgroups[patternPartIndex]->getEventsLeft() > 0
                //                    && this->partialMatchedEvents[patternPartIndex].size() == position)

                // any event from 38 events B..26
                if (this->cgroups[patternPartIndex]->getEventsLeft() > 0 && position != 0 && position <= 38)

                {
                    //                    this->simulateLoad2();
                    this->cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                    this->validator->receiveCgroup(this->cgroups[patternPartIndex], Cgroup::Status::UPDATE);
                    this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);

                    if (this->cgroups[patternPartIndex]->getEventsLeft() == 0)
                    {
                        //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                        unsigned long sn = this->cgroups[patternPartIndex]->getEvents()[0];
                        shared_ptr<ComplexEvent> complexEvent = make_shared<ComplexEvent>(
                            sn, this->partialMatchedEvents[patternPartIndex], this->measurements);

                        // set the complex event timestamp as the timestamp of last event in the pattern
                        complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                        // set the complex event real timestamp as the real timestamp of last event in the pattern
                        complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                        this->validator->receiveComplexEvent(complexEvent);
                    }
                }
            }
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void SequenceOperator::oneCgroupPerWindow()
{
    // start time
    //    unsigned long startTime = Helper::currentTimeMillis();

    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        //        this->simulateLoad();

        // get event's content from AbstractEvent( it contains only the event type i.e. A, B, etc.)
        string eventContent;
        for (auto it_eventType = it_primitive->get()->getContent().begin();
             it_eventType != it_primitive->get()->getContent().end(); it_eventType++)
            eventContent = it_eventType->second;

        // find its position in eventType vector (to define its position in the sequence operator)
        int position = -1;
        for (size_t index = 0; index < this->eventStates.size(); index++)
            if (eventContent == this->eventStates[index])
            {
                position = index;
                break;
            }

        if (position == -1)
            continue;

        // Pattern size is one, so each event is a Complex event :)!
        if (this->eventStates.size() == 1)
        {
            vector<shared_ptr<AbstractEvent>> cplxEvents;
            cplxEvents.push_back(*it_primitive);
            shared_ptr<ComplexEvent> complexEvent
                = make_shared<ComplexEvent>(it_primitive->get()->getSn(), cplxEvents, this->measurements);

            this->validator->receiveComplexEvent(complexEvent);
        }
        else // Pattern size is greater than one
        {
            size_t patternPartIndex = 0;
            if (this->cgroupHasBeenOpend_1 == false) // can only have one cgroup
            {
                if (position == 0) // fist event, only we need to add it to the automate
                {
                    //                    this->simulateLoad2();
                    // create new cgroup and add it to cgroups map
                    shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                    cgroups[patternPartIndex] = newCgroup;

                    //                cgroup->pushNewEvent(*it_primitive);
                    cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                    cgroups[patternPartIndex]->setEventsLeft(this->patternSize - 1);

                    // send created cgroup to the Validator
                    this->validator->receiveCgroup(cgroups[patternPartIndex], Cgroup::Status::NEW);

                    // create a new partial match
                    this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);
                    cgroupHasBeenOpend_1 = true;
                }
            }
            else
            {
                // find the event position in the automate (seq operator)
                //                if (this->cgroups[patternPartIndex]->getEventsLeft() > 0
                //                    && this->partialMatchedEvents[patternPartIndex].size() == position)
                if (this->cgroups[patternPartIndex]->getEventsLeft() > 0 && position != 0 && position <= 12)
                {
                    //                    this->simulateLoad2();
                    this->cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                    this->validator->receiveCgroup(this->cgroups[patternPartIndex], Cgroup::Status::UPDATE);
                    this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);

                    if (this->cgroups[patternPartIndex]->getEventsLeft() == 0)
                    {
                        //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                        unsigned long sn = this->cgroups[patternPartIndex]->getEvents()[0];
                        shared_ptr<ComplexEvent> complexEvent = make_shared<ComplexEvent>(
                            sn, this->partialMatchedEvents[patternPartIndex], this->measurements);

                        // set the complex event timestamp as the timestamp of last event in the pattern
                        complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                        // set the complex event real timestamp as the real timestamp of last event in the pattern
                        complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                        this->validator->receiveComplexEvent(complexEvent);
                    }
                }
            }
        }
    } // end of for outer loop

    this->primitiveEvents.clear();

    // end time
    //    unsigned long endTime = Helper::currentTimeMillis();

    //    this->measurements.get()->increaseTime(Measurements::ComponentName::OPERATOR, endTime - startTime);
}

void SequenceOperator::twoCgroupPerWindow()
{

    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        //        this->simulateLoad();

        // get event's content from AbstractEvent( it contains only the event type i.e. A, B, etc.)
        string eventContent;
        for (auto it_eventType = it_primitive->get()->getContent().begin();
             it_eventType != it_primitive->get()->getContent().end(); it_eventType++)
            eventContent = it_eventType->second;

        // find its position in eventType vector (to define its position in the sequence operator)
        int position = -1;
        for (size_t index = 0; index < this->eventStates.size(); index++)
            if (eventContent == this->eventStates[index])
            {
                position = index;
                break;
            }

        if (position == -1)
            continue;

        // Pattern size is one, so each event is a Complex event :)!
        if (this->eventStates.size() == 1)
        {
            vector<shared_ptr<AbstractEvent>> cplxEvents;
            cplxEvents.push_back(*it_primitive);
            shared_ptr<ComplexEvent> complexEvent
                = make_shared<ComplexEvent>(it_primitive->get()->getSn(), cplxEvents, this->measurements);

            this->validator->receiveComplexEvent(complexEvent);
        }
        else // Pattern size is greater than one
        {
            size_t patternPartIndex = 0;
            bool openNewCgroup = false;
            bool addEventToCgroup = false;
            int minAllowedPosition = 0;
            int maxAllowedPosition = 12;

            if (position >= 0 && position <= 12) // first cgroup
            {
                patternPartIndex = 0;
                if (cgroupHasBeenOpend_1 == false)
                {
                    if (position == 0)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_1 = true;
                    }
                }
                else if (position != 0)
                    addEventToCgroup = true;
            }
            else if (position >= 13 && position <= 25) // second cgroup
            {
                minAllowedPosition = 13;
                maxAllowedPosition = 25;
                patternPartIndex = 1;
                if (cgroupHasBeenOpend_2 == false)
                {
                    if (position == 13)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_2 = true;
                    }
                }
                else if (position != 13)
                    addEventToCgroup = true;
            }

            if (openNewCgroup)
            {

                //                    this->simulateLoad2();
                // create new cgroup and add it to cgroups map
                shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                cgroups[patternPartIndex] = newCgroup;

                //                cgroup->pushNewEvent(*it_primitive);
                cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                cgroups[patternPartIndex]->setEventsLeft(this->patternSize - 1);

                // send created cgroup to the Validator
                this->validator->receiveCgroup(cgroups[patternPartIndex], Cgroup::Status::NEW);

                // create a new partial match
                this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);
            }
            else
            {
                if (addEventToCgroup)
                {
                    // find the event position in the automate (seq operator)
                    //                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0
                    //                        && this->partialMatchedEvents[patternPartIndex].size() == position)
                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0 && position > minAllowedPosition
                        && position <= maxAllowedPosition)
                    {
                        //                    this->simulateLoad2();
                        this->cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                        this->validator->receiveCgroup(this->cgroups[patternPartIndex], Cgroup::Status::UPDATE);
                        this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);

                        if (this->cgroups[patternPartIndex]->getEventsLeft() == 0)
                        {
                            //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                            unsigned long sn = this->cgroups[patternPartIndex]->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent = make_shared<ComplexEvent>(
                                sn, this->partialMatchedEvents[patternPartIndex], this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                    }
                }
            }
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void SequenceOperator::threeCgroupPerWindow()
{
    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        //        this->simulateLoad();

        // get event's content from AbstractEvent( it contains only the event type i.e. A, B, etc.)
        string eventContent;
        for (auto it_eventType = it_primitive->get()->getContent().begin();
             it_eventType != it_primitive->get()->getContent().end(); it_eventType++)
            eventContent = it_eventType->second;

        // find its position in eventType vector (to define its position in the sequence operator)
        int position = -1;
        for (size_t index = 0; index < this->eventStates.size(); index++)
            if (eventContent == this->eventStates[index])
            {
                position = index;
                break;
            }

        if (position == -1)
            continue;

        // Pattern size is one, so each event is a Complex event :)!
        if (this->eventStates.size() == 1)
        {
            vector<shared_ptr<AbstractEvent>> cplxEvents;
            cplxEvents.push_back(*it_primitive);
            shared_ptr<ComplexEvent> complexEvent
                = make_shared<ComplexEvent>(it_primitive->get()->getSn(), cplxEvents, this->measurements);

            this->validator->receiveComplexEvent(complexEvent);
        }
        else // Pattern size is greater than one
        {
            size_t patternPartIndex = 0;
            bool openNewCgroup = false;
            bool addEventToCgroup = false;
            int minAllowedPosition = 0;
            int maxAllowedPosition = 12;

            if (position >= 0 && position <= 12) // first cgroup
            {
                patternPartIndex = 0;
                if (cgroupHasBeenOpend_1 == false)
                {
                    if (position == 0)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_1 = true;
                    }
                }
                else if (position != 0)
                    addEventToCgroup = true;
            }
            else if (position >= 13 && position <= 25) // second cgroup
            {
                minAllowedPosition = 13;
                maxAllowedPosition = 25;
                patternPartIndex = 1;
                if (cgroupHasBeenOpend_2 == false)
                {
                    if (position == 13)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_2 = true;
                    }
                }
                else if (position != 13)
                    addEventToCgroup = true;
            }
            else if (position >= 26 && position <= 38) // second cgroup
            {
                minAllowedPosition = 26;
                maxAllowedPosition = 38;
                patternPartIndex = 2;
                if (cgroupHasBeenOpend_3 == false)
                {
                    if (position == 26)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_3 = true;
                    }
                }
                else if (position != 26)
                    addEventToCgroup = true;
            }

            if (openNewCgroup)
            {

                //                    this->simulateLoad2();
                // create new cgroup and add it to cgroups map
                shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                cgroups[patternPartIndex] = newCgroup;

                //                cgroup->pushNewEvent(*it_primitive);
                cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                cgroups[patternPartIndex]->setEventsLeft(this->patternSize - 1);

                // send created cgroup to the Validator
                this->validator->receiveCgroup(cgroups[patternPartIndex], Cgroup::Status::NEW);

                // create a new partial match
                this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);
            }
            else
            {
                if (addEventToCgroup)
                {
                    // find the event position in the automate (seq operator)
                    //                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0
                    //                        && this->partialMatchedEvents[patternPartIndex].size() == position)
                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0 && position > minAllowedPosition
                        && position <= maxAllowedPosition)
                    {
                        //                    this->simulateLoad2();
                        this->cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                        this->validator->receiveCgroup(this->cgroups[patternPartIndex], Cgroup::Status::UPDATE);
                        this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);

                        if (this->cgroups[patternPartIndex]->getEventsLeft() == 0)
                        {
                            //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                            unsigned long sn = this->cgroups[patternPartIndex]->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent = make_shared<ComplexEvent>(
                                sn, this->partialMatchedEvents[patternPartIndex], this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                    }
                }
            }
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void SequenceOperator::fourCgroupPerWindow()
{
    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        //        this->simulateLoad();

        // get event's content from AbstractEvent( it contains only the event type i.e. A, B, etc.)
        string eventContent;
        for (auto it_eventType = it_primitive->get()->getContent().begin();
             it_eventType != it_primitive->get()->getContent().end(); it_eventType++)
            eventContent = it_eventType->second;

        // find its position in eventType vector (to define its position in the sequence operator)
        int position = -1;
        for (size_t index = 0; index < this->eventStates.size(); index++)
            if (eventContent == this->eventStates[index])
            {
                position = index;
                break;
            }

        if (position == -1)
            continue;

        // Pattern size is one, so each event is a Complex event :)!
        if (this->eventStates.size() == 1)
        {
            vector<shared_ptr<AbstractEvent>> cplxEvents;
            cplxEvents.push_back(*it_primitive);
            shared_ptr<ComplexEvent> complexEvent
                = make_shared<ComplexEvent>(it_primitive->get()->getSn(), cplxEvents, this->measurements);

            this->validator->receiveComplexEvent(complexEvent);
        }
        else // Pattern size is greater than one
        {
            size_t patternPartIndex = 0;
            bool openNewCgroup = false;
            bool addEventToCgroup = false;
            int minAllowedPosition = 0;
            int maxAllowedPosition = 12;


            if (position >= 0 && position <= 12) // first cgroup
            {
                patternPartIndex = 0;
                if (cgroupHasBeenOpend_1 == false)
                {
                    if (position == 0)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_1 = true;
                    }
                }
                else
                    if(position!=0)
                    addEventToCgroup = true;
            }
            else if (position >= 13 && position <= 25) // second cgroup
            {
                minAllowedPosition = 13;
                maxAllowedPosition = 25;
                patternPartIndex = 1;
                if (cgroupHasBeenOpend_2 == false)
                {
                    if (position == 13)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_2 = true;
                    }
                }
                else
                 if(position!=13)
                    addEventToCgroup = true;

            }
            else if (position >= 26 && position <= 38) // third cgroup
            {
                minAllowedPosition = 26;
                maxAllowedPosition = 38;
                patternPartIndex = 2;
                if (cgroupHasBeenOpend_3 == false)
                {
                    if (position == 26)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_3 = true;
                    }
                }
                else
             if(position!=26)
                 addEventToCgroup = true;
            }
            else if (position >= 39 && position <= 51) // fourth cgroup
            {
                minAllowedPosition = 39;
                maxAllowedPosition = 51;
                patternPartIndex = 3;
                if (cgroupHasBeenOpend_4 == false)
                {
                    if (position == 39)
                    {
                        openNewCgroup = true;
                        cgroupHasBeenOpend_4 = true;
                    }
                }
                else
                    if(position!=39)
                    addEventToCgroup = true;
            }

            if (openNewCgroup)
            {

                //                    this->simulateLoad2();
                // create new cgroup and add it to cgroups map
                shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                cgroups[patternPartIndex] = newCgroup;

                //                cgroup->pushNewEvent(*it_primitive);
                cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                cgroups[patternPartIndex]->setEventsLeft(this->patternSize - 1);

                // send created cgroup to the Validator
                this->validator->receiveCgroup(cgroups[patternPartIndex], Cgroup::Status::NEW);

                // create a new partial match
                this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);
            }
            else
            {
                if (addEventToCgroup)
                {
                    // find the event position in the automate (seq operator)
//                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0
//                        && this->partialMatchedEvents[patternPartIndex].size() == position)
                    if (this->cgroups[patternPartIndex]->getEventsLeft() > 0 && position > minAllowedPosition
                        && position <= maxAllowedPosition)
                    {
                        //                    this->simulateLoad2();
                        this->cgroups[patternPartIndex]->pushNewEvent(it_primitive->get()->getSn());
                        this->validator->receiveCgroup(this->cgroups[patternPartIndex], Cgroup::Status::UPDATE);
                        this->partialMatchedEvents[patternPartIndex].push_back(*it_primitive);

                        if (this->cgroups[patternPartIndex]->getEventsLeft() == 0)
                        {
                            //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                            unsigned long sn = this->cgroups[patternPartIndex]->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent = make_shared<ComplexEvent>(
                                sn, this->partialMatchedEvents[patternPartIndex], this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                    }
                }
            }
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void SequenceOperator::simulateLoad()
{
    unsigned long startTimeLoad = Helper::currentTimeMicros();
    unsigned long endTimeLoad = 0;
    do
    {
        endTimeLoad = Helper::currentTimeMicros();
    } while ((endTimeLoad - startTimeLoad) < 10);
}

void SequenceOperator::simulateLoad2()
{
    unsigned long startTimeLoad = Helper::currentTimeMicros();
    unsigned long endTimeLoad = 0;
    do
    {
        endTimeLoad = Helper::currentTimeMicros();
    } while ((endTimeLoad - startTimeLoad) < 2000);
}
SequenceOperator::~SequenceOperator()
{
    // TODO Auto-generated destructor stub
}

} /* namespace execution_path */
