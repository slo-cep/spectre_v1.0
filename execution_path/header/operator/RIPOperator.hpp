/*
 * RIPOperator.hpp
 *
 * Created on: 27.04.2017
 *      Author: sload
 */

#ifndef RIPOPERATOR_HPP
#define RIPOPERATOR_HPP

#include "AbstractOperator.hpp"

#include "header/events/AbstractEvent.hpp"
#include "header/events/ComplexEvent.hpp"

namespace execution_path
{
class RIPOperator : public AbstractOperator
{
private:
    enum StateMachine
    {
        A=0,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M
    };

public:
    /**
     * Ctor
 * @param validator: pointer to the Validator
 * @param patternSize: an array of event types which will be detected in a sequence order
 * @param workLoad: artificial work load in case of generating events
 * @param measurements: for profiling
     */
    RIPOperator(AbstractValidator *validator, unsigned int patternSize, unsigned long workLoad,
                      profiler::Measurements *measurements, set<string> *symbols);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    RIPOperator(const RIPOperator &other);

    shared_ptr<AbstractOperator> clone() override;

    /**
     * @brief recover: recover operator. Cgroups shouldn't loose their old pointer
     * @param other:rhs
     */
    void recover(AbstractOperator *other) override;

    void main() override;

    virtual ~RIPOperator();
private:
    unsigned int patternSize;

    bool cgroupHasBeenOpend = false;
    shared_ptr<execution_path::Cgroup> cgroup = NULL;

    vector <shared_ptr<events::AbstractEvent>> partialMatchedEvents;
    // map event Sn with event index ( event(Sn), index)
    unordered_map<unsigned long, unsigned int> partialMatchedEventsIndex;

    unsigned long workLoad;

    bool rise;

    set<string>* symbols;

    //head and shoulders states
    shared_ptr<events::AbstractEvent> state_A=nullptr;
    shared_ptr<events::AbstractEvent> state_B=nullptr;
    shared_ptr<events::AbstractEvent> state_C=nullptr;
    shared_ptr<events::AbstractEvent> state_D=nullptr;
    shared_ptr<events::AbstractEvent> state_E=nullptr;
    shared_ptr<events::AbstractEvent> state_F=nullptr;
    shared_ptr<events::AbstractEvent> state_G=nullptr;
    shared_ptr<events::AbstractEvent> state_H=nullptr;
    shared_ptr<events::AbstractEvent> state_I=nullptr;
    shared_ptr<events::AbstractEvent> state_J=nullptr;
    shared_ptr<events::AbstractEvent> state_K=nullptr;
    shared_ptr<events::AbstractEvent> state_L=nullptr;
    shared_ptr<events::AbstractEvent> state_M=nullptr;

    StateMachine currentState;

    void headAndShoulders();
    void query8();
    void query9();

    void pushEventPartialMatchedEvents(shared_ptr<events::AbstractEvent> event);

    void removeEventsFromPartialMatchFromBeginning(unsigned int numberOfEvents);
};


}

#endif // RIPOPERATOR_HPP
