/*
 * OperatorFactory.hpp
 *
 * Created on: 16.12.2016
 *      Author: sload
 */

#ifndef OPERATORFACTORY_HPP
#define OPERATORFACTORY_HPP

#include "header/util/Constants.hpp"

#include "AbstractOperator.hpp"
#include "SequenceOperator.hpp"
#include "StockRiseOperator.h"
#include "RIPOperator.hpp"

#include "../../header/validator/AbstractValidator.hpp"

#include <memory>
#include <string>
#include <set>

using namespace std;

namespace execution_path
{

class OperatorFactory
{
public:
    /**
     * Ctor
     * @param operatorType: which operator?
     * @param patternSize: size of pattern for stock operator
     * @param eventStates: an array of event types which will be detected in a sequence order
     * @param workLoad: artificial work load in case of generating events
     * @param measurements: for profiling
     */
    OperatorFactory(util::Constants::OperatorType operatorType, unsigned int patternSize, vector<string> eventStates,
                    unsigned long workLoad, profiler::Measurements* measurements);

    /**
     * @brief createOperator: create new operator
     * @param validator: pointer to the Validator
     * @return pointer to the created operator
     */
    shared_ptr<AbstractOperator> createOperator(AbstractValidator* validator) const;

    ~OperatorFactory();

private:
    util::Constants::OperatorType operatorType;
    unsigned int patternSize;
    vector<string> eventStates;
    unsigned long workLoad;
    profiler::Measurements* measurements;
      shared_ptr<set<string>> symbols;
};
}

#endif // OPERATORFACTORY_HPP
