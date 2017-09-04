/*
 * RIPOperator.cpp
 *
 * Created on: 27.04.2017
 *      Author: sload
 */

#include "header/operator/RIPOperator.hpp"

using namespace std;
using namespace events;

namespace execution_path
{

RIPOperator::RIPOperator(AbstractValidator *validator, unsigned int patternSize, unsigned long workLoad,
                         profiler::Measurements *measurements, set<string> *symbols)
    : AbstractOperator(validator, measurements)
{
    this->patternSize = patternSize;
    this->workLoad = workLoad;
    this->symbols = symbols;

    this->currentState=StateMachine::A;
}

RIPOperator::RIPOperator(const RIPOperator &other) : AbstractOperator(other)
{
    this->patternSize = other.patternSize;
    this->cgroupHasBeenOpend = other.cgroupHasBeenOpend;

    if (other.cgroup)
        this->cgroup = other.cgroup->clone();

    this->partialMatchedEvents = other.partialMatchedEvents;
    this->partialMatchedEventsIndex = other.partialMatchedEventsIndex;
    this->workLoad = other.workLoad;
    this->rise = other.rise;

    this->symbols = other.symbols;

    this->currentState = other.currentState;
    this->state_A = other.state_A;
    this->state_B = other.state_B;
    this->state_C = other.state_C;
    this->state_D = other.state_D;
    this->state_E = other.state_E;
    this->state_F = other.state_F;
    this->state_G = other.state_G;
    this->state_H = other.state_H;
    this->state_I = other.state_I;
    this->state_J = other.state_J;
    this->state_K = other.state_K;
    this->state_L = other.state_L;
    this->state_M = other.state_M;
}

shared_ptr<AbstractOperator> RIPOperator::clone() { return make_shared<RIPOperator>(*this); }

void RIPOperator::recover(AbstractOperator *other)
{
    AbstractOperator::recover(other);

    RIPOperator *rhs = static_cast<RIPOperator *>(other);

    this->patternSize = rhs->patternSize;
    this->cgroupHasBeenOpend = rhs->cgroupHasBeenOpend;

    if (rhs->cgroup)
        this->cgroup->set(*rhs->cgroup);
    else
        this->cgroup = nullptr;

    this->partialMatchedEvents = rhs->partialMatchedEvents;
    this->partialMatchedEventsIndex = rhs->partialMatchedEventsIndex;

    this->workLoad = rhs->workLoad;
    this->rise = rhs->rise;

    this->currentState = rhs->currentState;
    this->state_A = rhs->state_A;
    this->state_B = rhs->state_B;
    this->state_C = rhs->state_C;
    this->state_D = rhs->state_D;
    this->state_E = rhs->state_E;
    this->state_F = rhs->state_F;
    this->state_G = rhs->state_G;
    this->state_H = rhs->state_H;
    this->state_I = rhs->state_I;
    this->state_J = rhs->state_J;
    this->state_K = rhs->state_K;
    this->state_L = rhs->state_L;
    this->state_M = rhs->state_M;
}

void RIPOperator::main()
{
    //    this->headAndShoulders();
//    this->query8();
    this->query9();
}

void RIPOperator::headAndShoulders()
{
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        auto &eventContent = (*it_primitive)->getContent();

        try
        {
            if (this->cgroupHasBeenOpend == false) // can only have one cgroup
            {
                this->state_A = *it_primitive;

                this->currentState = StateMachine::B;
                this->state_B = this->state_A;

                shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                cgroup = newCgroup;
                cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                cgroup->setEventsLeft(this->patternSize - 1);

                // send created cgroup to the Validator
                this->validator->receiveCgroup(cgroup, Cgroup::Status::NEW);

                this->pushEventPartialMatchedEvents(*it_primitive);
                cgroupHasBeenOpend = true;
            }
            else
            {
                switch (this->currentState)
                {
                case StateMachine::A:
                    break;

                case StateMachine::B:
                    if (std::stof(eventContent.at("close")) >= std::stof(state_B->getContent().at("close")))
                    {
                        this->state_B = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else // check for C (left shoulder top)
                    {
                        // the previous event could be C
                        this->state_C = this->state_B;

                        // C (price) greater than A (price)
                        if (std::stof(state_C->getContent().at("close")) > std::stof(state_A->getContent().at("close")))
                        {
                            // The previous event is C==> move to state D directly
                            this->currentState = StateMachine::D;
                            this->state_D = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                        else // the previous event cannot be C so restart
                        {
                            // restart from the beginning
                            this->state_A = *it_primitive;
                            this->currentState = StateMachine::B;
                            this->state_B = this->state_A;

                            // remove events from the cgroup & partial match
                            this->cgroup->removeEventsFromBeginning(this->cgroup->getEvents().size(), patternSize);
                            this->partialMatchedEvents.clear();
                            this->partialMatchedEventsIndex.clear();

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->cgroup->setEventsLeft(this->patternSize - 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                          cout << "cgroupId:" << cgroup->getId()
                            //                                 << ", STATE::B: restart: cgroup.eventsLeft: " <<
                            //                                 this->cgroup->getEventsLeft() << endl;

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }

                    break;

                case StateMachine::C:

                    break;

                case StateMachine::D:

                    if (std::stof(eventContent.at("close")) <= std::stof(state_D->getContent().at("close")))
                    {
                        // New event (price) less than A (price) ==> restart
                        if (std::stof(eventContent.at("close")) <= std::stof(state_A->getContent().at("close")))
                        {
                            // restart from the beginning
                            this->state_A = *it_primitive;
                            this->currentState = StateMachine::B;
                            this->state_B = this->state_A;

                            // remove events from the cgroup & partial match
                            this->cgroup->removeEventsFromBeginning(this->cgroup->getEvents().size(), patternSize);
                            this->partialMatchedEvents.clear();
                            this->partialMatchedEventsIndex.clear();

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->cgroup->setEventsLeft(this->patternSize - 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                            cout << "cgroupId:" << cgroup->getId()
                            //                                 << ", STATE::D: restart: cgroup.eventsLeft: " <<
                            //                                 this->cgroup->getEventsLeft() << endl;

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                        else
                        {
                            this->state_D = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            // don't send any update to validator as the state machine didn't change

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    } // it is E (lower point between left shoulder and head)
                    else
                    {
                        this->state_E = this->state_D;

                        this->currentState = StateMachine::F;
                        this->state_F = *it_primitive;

                        // Add to the cgroup
                        // pattern decreased by two states
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::E:

                    break;

                case StateMachine::F:

                    if (std::stof(eventContent.at("close")) >= std::stof(state_F->getContent().at("close")))
                    {
                        this->state_F = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else // check for G (head)
                    {
                        // the previous event could be G
                        this->state_G = this->state_F;

                        // G (price) larger than C (price)
                        if (std::stof(state_G->getContent().at("close")) > std::stof(state_C->getContent().at("close")))
                        {
                            // move to state H
                            this->currentState = StateMachine::H;
                            this->state_H = *it_primitive;

                            // Add to the cgroup
                            // pattern decreased by two states
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                        else // G smaller than C==> remove all event before E and change the states accordingly
                        {
                            // remove all events before E from the cgroup & partial match
                            unsigned int numberOfEvents = this->partialMatchedEventsIndex[this->state_E->getSn()];

                            this->currentState = StateMachine::D;
                            this->state_A = this->state_E;
                            this->state_B = this->state_F;
                            this->state_C = this->state_G;
                            this->state_D = *it_primitive;

                            this->cgroup->removeEventsFromBeginning(numberOfEvents, patternSize - 3);

                            this->removeEventsFromPartialMatchFromBeginning(numberOfEvents);

                            // Add the current event to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                            cout << "cgroupId:" << cgroup->getId()
                            //                                 << ", STATE::F: restart: cgroup.eventsLeft: " <<
                            //                                 this->cgroup->getEventsLeft() << endl;

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }

                    break;

                case StateMachine::G:

                    break;

                case StateMachine::H:

                    if (std::stof(eventContent.at("close")) <= std::stof(state_H->getContent().at("close")))
                    {
                        // New event (price) less than A (price) ==> restart
                        if (std::stof(eventContent.at("close")) <= std::stof(state_A->getContent().at("close")))
                        {
                            // restart from the beginning
                            this->state_A = *it_primitive;
                            this->currentState = StateMachine::B;
                            this->state_B = this->state_A;

                            // remove events from the cgroup & partial match
                            this->cgroup->removeEventsFromBeginning(this->cgroup->getEvents().size(), patternSize);
                            this->partialMatchedEvents.clear();
                            this->partialMatchedEventsIndex.clear();

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->cgroup->setEventsLeft(this->patternSize - 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                            cout << "cgroupId:" << cgroup->getId()
                            //                                 << ", STATE::H: restart: cgroup.eventsLeft: " <<
                            //                                 this->cgroup->getEventsLeft() << endl;

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                        else
                        {
                            this->state_H = *it_primitive;

                            // Add the current event to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            // don't send any update to validator as the state machine didn't change

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    } // it is E (lower point between left shoulder and head)
                    else
                    {
                        this->state_I = this->state_H;

                        this->currentState = StateMachine::J;
                        this->state_J = *it_primitive;

                        // Add to the cgroup
                        // pattern decreased by two states
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::I:

                    break;

                case StateMachine::J:
                    if (std::stof(eventContent.at("close")) >= std::stof(state_J->getContent().at("close")))
                    {
                        this->state_J = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change
                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else // check for K (right shoulder)
                    {
                        // the previous event could be K
                        this->state_K = this->state_J;

                        // K (price) smaller than G (price)
                        if (std::stof(state_K->getContent().at("close")) < std::stof(state_G->getContent().at("close")))
                        {
                            // move to state L
                            this->currentState = StateMachine::L;
                            this->state_L = *it_primitive;

                            this->pushEventPartialMatchedEvents(*it_primitive);

                            // if the New event less than E and I, pattern is completed ==> we are in M
                            if (std::stof(eventContent.at("close")) < std::stof(state_E->getContent().at("close"))
                                && std::stof(eventContent.at("close")) < std::stof(state_I->getContent().at("close")))
                            {
                                this->currentState = StateMachine::M;
                                this->state_M = *it_primitive;
                                // Add to the cgroup
                                // pattern decreased by four states  ==> cgroup completed
                                this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 4);
                                this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                                // create a complex event
                                unsigned long sn = this->cgroup->getEvents()[0];
                                shared_ptr<ComplexEvent> complexEvent
                                    = make_shared<ComplexEvent>(sn, this->partialMatchedEvents, this->measurements);

                                // set the complex event timestamp as the timestamp of last event in the pattern
                                complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                                // set the complex event real timestamp as the real timestamp of last event in the
                                // pattern
                                complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                                this->validator->receiveComplexEvent(complexEvent);
                            }
                            else
                            {
                                // Add to the cgroup
                                // pattern decreased by two states

                                this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                                this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);
                            }
                        }
                        else // K larger than G ==> remove all event before E and change the states accordingly
                        // previous G becomes left shoulder and this shoulder becomes head
                        {
                            // remove all events before E from the cgroup & partial match
                            unsigned int numberOfEvents = this->partialMatchedEventsIndex[this->state_E->getSn()];

                            this->currentState = StateMachine::H;
                            this->state_A = this->state_E;
                            this->state_B = this->state_F;
                            this->state_C = this->state_G;
                            this->state_D = this->state_H;
                            this->state_E = this->state_I;
                            this->state_F = this->state_J;
                            this->state_G = this->state_K;
                            this->state_H = *it_primitive;

                            this->cgroup->removeEventsFromBeginning(numberOfEvents, patternSize - 7);

                            this->removeEventsFromPartialMatchFromBeginning(numberOfEvents);

                            // Add the current event to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                            cout << "cgroupId:" << cgroup->getId()
                            //                                 << ", SSTATE::J: restart: cgroup.eventsLeft: " <<
                            //                                 this->cgroup->getEventsLeft()
                            //                                 << endl;

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::K:

                    break;

                case StateMachine::L:
                    if (std::stof(eventContent.at("close")) <= std::stof(state_L->getContent().at("close")))
                    {
                        this->state_L = *it_primitive;

                        this->pushEventPartialMatchedEvents(*it_primitive);

                        // if the New event less than E and I, pattern is completed ==> we are in M
                        if (std::stof(eventContent.at("close")) < std::stof(state_E->getContent().at("close"))
                            && std::stof(eventContent.at("close")) < std::stof(state_I->getContent().at("close")))
                        {
                            this->currentState = StateMachine::M;
                            this->state_M = *it_primitive;
                            // Add to the cgroup
                            // pattern decreased by two states ==> cgroup completed
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            // create a complex event
                            unsigned long sn = this->cgroup->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent
                                = make_shared<ComplexEvent>(sn, this->partialMatchedEvents, this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the
                            // pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                        else
                        {
                            // Add to the cgroup
                            // pattern state doesn't change
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);
                        }
                    }
                    else // New event larger than L ==> remove all event before I and change the states accordingly
                    {
                        // remove all events before I from the cgroup & partial match
                        unsigned int numberOfEvents = this->partialMatchedEventsIndex[this->state_I->getSn()];

                        this->currentState = StateMachine::F;
                        this->state_A = this->state_I;
                        this->state_B = this->state_J;
                        this->state_C = this->state_K;
                        this->state_D = this->state_L;
                        this->state_E = this->state_L;
                        this->state_F = *it_primitive;

                        this->cgroup->removeEventsFromBeginning(numberOfEvents, patternSize - 5);

                        this->removeEventsFromPartialMatchFromBeginning(numberOfEvents);

                        // Add the current event to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        //                        cout << "cgroupId:" << cgroup->getId()
                        //                             << ", STATE::L: restart: cgroup.eventsLeft: " <<
                        //                             this->cgroup->getEventsLeft() << endl;

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::M:

                    break;
                }
            }
        }
        catch (...)
        {
            // Not a Stock trading event
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void RIPOperator::query8()
{
    float limit1 = 152;
    float limit2 = 156;
    float limit3 = 162;

    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        auto &eventContent = (*it_primitive)->getContent();

        try
        {
            if (this->cgroupHasBeenOpend == false) // can only have one cgroup
            {
                if (std::stof(eventContent.at("close")) < limit1)
                {
                    this->state_A = *it_primitive;

                    this->currentState = StateMachine::B;
                    this->state_B = this->state_A;

                    shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                    cgroup = newCgroup;
                    cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                    cgroup->setEventsLeft(this->patternSize - 1);

                    // send created cgroup to the Validator
                    this->validator->receiveCgroup(cgroup, Cgroup::Status::NEW);

                    this->pushEventPartialMatchedEvents(*it_primitive);
                    cgroupHasBeenOpend = true;
                }
            }
            else
            {
                switch (this->currentState)
                {
                case StateMachine::A:
                    break;

                case StateMachine::B:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_B = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) < limit1)
                        {
                            this->currentState = StateMachine::C;
                            this->state_C = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::C:
                    if (std::stof(eventContent.at("close")) < limit1)
                    {
                        this->state_C = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit1
                            && std::stof(eventContent.at("close")) < limit2)
                        {
                            this->currentState = StateMachine::D;
                            this->state_D = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::D:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_D = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit2
                            && std::stof(eventContent.at("close")) < limit3)
                        {
                            this->currentState = StateMachine::E;
                            this->state_E = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::E:
                    if (std::stof(eventContent.at("close")) > limit2 && std::stof(eventContent.at("close")) < limit3)
                    {
                        this->state_E = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit3)
                        {
                            // complex event completed
                            this->currentState = StateMachine::F;
                            this->state_F = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 2);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            //                            cout<<"cgroup: event left: "<<
                            //                            this->cgroup->getEventsLeft()<<endl;
                            this->pushEventPartialMatchedEvents(*it_primitive);

                            // create a complex event
                            unsigned long sn = this->cgroup->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent
                                = make_shared<ComplexEvent>(sn, this->partialMatchedEvents, this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the
                            // pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                    }
                    break;
                }
            }
        }
        catch (...)
        {
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void RIPOperator::query9()
{
    float limit1 = 151.0;
    float limit2 = 164.7;

    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

        auto &eventContent = (*it_primitive)->getContent();

        try
        {
            if (this->cgroupHasBeenOpend == false) // can only have one cgroup
            {
                if (std::stof(eventContent.at("close")) < limit1)
                {
                    this->state_A = *it_primitive;

                    this->currentState = StateMachine::A;

                    shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                    cgroup = newCgroup;
                    cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                    cgroup->setEventsLeft(this->patternSize - 1);

                    // send created cgroup to the Validator
                    this->validator->receiveCgroup(cgroup, Cgroup::Status::NEW);

                    this->pushEventPartialMatchedEvents(*it_primitive);
                    cgroupHasBeenOpend = true;
                }
            }
            else
            {
                switch (this->currentState)
                {
                case StateMachine::A:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::B;
                        this->state_B = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::B:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_B = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit2)
                        {
                            this->currentState = StateMachine::C;
                            this->state_C = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::C:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::D;
                        this->state_D = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }

                    break;

                case StateMachine::D:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_D = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) < limit1)
                        {
                            this->currentState = StateMachine::E;
                            this->state_E = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::E:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::F;
                        this->state_F = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::F:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_F = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit2)
                        {
                            this->currentState = StateMachine::G;
                            this->state_G = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::G:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::H;
                        this->state_G = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::H:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_H = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) < limit1)
                        {
                            this->currentState = StateMachine::I;
                            this->state_I = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::I:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::J;
                        this->state_J = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::J:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_J = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) > limit2)
                        {
                            this->currentState = StateMachine::K;
                            this->state_K = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);
                        }
                    }
                    break;

                case StateMachine::K:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->currentState = StateMachine::L;
                        this->state_L = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                        this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    break;

                case StateMachine::L:
                    if (std::stof(eventContent.at("close")) > limit1 && std::stof(eventContent.at("close")) < limit2)
                    {
                        this->state_L = *it_primitive;

                        // Add to the cgroup
                        this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 0);
                        // don't send any update to validator as the state machine didn't change

                        this->pushEventPartialMatchedEvents(*it_primitive);
                    }
                    else
                    {
                        if (std::stof(eventContent.at("close")) < limit1)
                        {
                            this->currentState = StateMachine::M;
                            this->state_M = *it_primitive;

                            // Add to the cgroup
                            this->cgroup->pushNewEvent(it_primitive->get()->getSn(), 1);
                            this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                            this->pushEventPartialMatchedEvents(*it_primitive);

                            // create a complex event
                            unsigned long sn = this->cgroup->getEvents()[0];
                            shared_ptr<ComplexEvent> complexEvent
                                = make_shared<ComplexEvent>(sn, this->partialMatchedEvents, this->measurements);

                            // set the complex event timestamp as the timestamp of last event in the pattern
                            complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                            // set the complex event real timestamp as the real timestamp of last event in the
                            // pattern
                            complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                            this->validator->receiveComplexEvent(complexEvent);
                        }
                    }
                    break;

                case StateMachine::M:

                    break;
                }
            }
        }
        catch (...)
        {
            cout<<"problem!"<<endl;
        }
    } // end of for outer loop

    this->primitiveEvents.clear();
}

void RIPOperator::pushEventPartialMatchedEvents(shared_ptr<AbstractEvent> event)
{
    this->partialMatchedEvents.push_back(event);
    this->partialMatchedEventsIndex[event->getSn()] = this->partialMatchedEvents.size() - 1;
}

void RIPOperator::removeEventsFromPartialMatchFromBeginning(unsigned int numberOfEvents)
{
    // remove elements from the map
    this->partialMatchedEvents.erase(this->partialMatchedEvents.begin(),
                                     this->partialMatchedEvents.begin() + numberOfEvents);
    this->partialMatchedEventsIndex.clear();

    for (unsigned int index = 0; index < this->partialMatchedEvents.size(); index++)
    {
        this->partialMatchedEventsIndex[this->partialMatchedEvents[index]->getSn()] = index;
    }
}

RIPOperator::~RIPOperator() {}
}
