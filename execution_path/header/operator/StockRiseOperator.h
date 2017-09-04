#ifndef STOCKRISEOPERATOR_H
#define STOCKRISEOPERATOR_H

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
#include<map>
#include<set>

using namespace std;

namespace execution_path
{

class StockRiseOperator : public AbstractOperator
{
public:

    /**
     * Ctor
 * @param validator: pointer to the Validator
 * @param patternSize: an array of event types which will be detected in a sequence order
 * @param workLoad: artificial work load in case of generating events
 * @param measurements: for profiling
     */
    StockRiseOperator(AbstractValidator *validator, unsigned int patternSize, unsigned long workLoad,
                      profiler::Measurements* measurements, set<string>* symbols);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    StockRiseOperator(const StockRiseOperator &other);

    shared_ptr<AbstractOperator> clone() override;

    /**
     * @brief recover: recover operator. Cgroups shouldn't loose their old pointer
     * @param other:rhs
     */
    void recover(AbstractOperator *other) override;

    void main() override;

    void simulateLoad();

    virtual ~StockRiseOperator();

private:
    unsigned int patternSize;

    bool cgroupHasBeenOpend = false;
    shared_ptr<execution_path::Cgroup> cgroup = NULL;

    // vector of pair ( cgroup Id, vector of partial matches)
    // vector< pair<unsigned long , vector<shared_ptr<events::AbstractEvent>>> > matchedPatterns;

    vector <shared_ptr<events::AbstractEvent>> partialMatchedEvents;
    //(cgroup Id, cgroup *)
    // unordered_map<size_t, shared_ptr<execution_path::Cgroup> > cgroups;

    unsigned long workLoad;

    bool rise;

    set<string>* symbols;

};

} /* namespace execution_path */

#endif // STOCKRISEOPERATOR_H
