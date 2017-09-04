/*
 * SequenceOperator.hpp
 *
 *  Created on: Aug 16, 2016
 *      Author: sload
 */

#ifndef OPERATOR_SEQUENCEOPERATOR_HPP_
#define OPERATOR_SEQUENCEOPERATOR_HPP_

#include "AbstractOperator.hpp"

#include "header/util/GlobalTypedef.hpp"
#include "header/util/Helper.hpp"

#include "header/events/AbstractEvent.hpp"
#include "header/events/ComplexEvent.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
using namespace std;

namespace execution_path
{

class SequenceOperator : public AbstractOperator
{
public:

    /**
     * Ctor
 * @param validator: pointer to the Validator
 * @param patternSize:
 * @param eventStates: an array of event types which will be detected in a sequence order
 * @param workLoad: artificial work load in case of generating events
 * @param measurements: for profiling
     */
    SequenceOperator(AbstractValidator *validator, unsigned int patternSize, vector<string> eventStates, unsigned long workLoad,
                     profiler::Measurements* measurements);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    SequenceOperator(const SequenceOperator &other);

    shared_ptr<AbstractOperator> clone() override;

    /**
     * @brief recover: recover operator. Cgroups shouldn't loose their old pointer
     * @param other:rhs
     */
    void recover(AbstractOperator *other) override;

    void main() override;

    void simulateLoad();
    void simulateLoad2();

    virtual ~SequenceOperator();


    void normalOperator();
    void oneCgroupPerWindow();
    void twoCgroupPerWindow();
    void threeCgroupPerWindow();
    void fourCgroupPerWindow();

private:
    unsigned int patternSize;
    vector<string> eventStates;

    bool cgroupHasBeenOpend_1 = false;
    bool cgroupHasBeenOpend_2 = false;
    bool cgroupHasBeenOpend_3 = false;
    bool cgroupHasBeenOpend_4 = false;

//    shared_ptr<execution_path::Cgroup> cgroup = NULL;

    vector<shared_ptr<execution_path::Cgroup>> cgroups;

    // vector of pair ( cgroup Id, vector of partial matches)
//    vector<pair<unsigned long, vector<shared_ptr<AbstractEvent>>>> matchedPatterns;

    //(cgroup Id, cgroup *)
//    unordered_map<size_t, shared_ptr<execution_path::Cgroup>> cgroups;

//     vector <shared_ptr<events::AbstractEvent>> partialMatchedEvents;
    vector< vector <shared_ptr<events::AbstractEvent>>> partialMatchedEvents;

    unsigned long workLoad;   

};

} /* namespace execution_path */

#endif /* OPERATOR_SEQUENCEOPERATOR_HPP_ */
