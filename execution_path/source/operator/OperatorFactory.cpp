/*
 * OperatorFactory.cpp
 *
 * Created on: 16.12.2016
 *      Author: sload
 */

#include "../../header/operator/OperatorFactory.hpp"

using namespace util;

namespace execution_path
{

OperatorFactory::OperatorFactory(Constants::OperatorType operatorType, unsigned int patternSize,
                                 vector<string> eventStates, unsigned long workLoad,
                                 profiler::Measurements *measurements)
    : operatorType(operatorType), patternSize(patternSize), eventStates(eventStates), workLoad(workLoad),
      measurements(measurements)
{
    this->symbols = make_shared<set<string>>();
/*
    *this->symbols
           = {"AAAP", "FLWS", "FCCY", "VNET", "TWOU", "JOBS", "CAFD", "EGHT", "AVHI", "SHLM", "AAON", "ABAX", "ABEO",
              "ABMD", "AXAS", "ACIU", "ACIA", "ACTG", "ACHC", "ACAD", "ACST", "AXDX", "XLRN", "ANCX", "ARAY"};
*/

    *this->symbols = {"AAAP",  "FLWS", "XOM",  "PTR", "BHP",  "PBR", "HSBC", "WMT", "CHL",
                      "RDS.A", "CVX",  "AAPL", "JPM", "MSFT", "AIG", "WFC",  "IBM"};
}

shared_ptr<AbstractOperator> OperatorFactory::createOperator(AbstractValidator *validator) const
{
    shared_ptr<AbstractOperator> abstractOperator;
    switch (this->operatorType)
    {
    case Constants::OperatorType::SEQUENCE_OPERATOR:
        abstractOperator
            = make_shared<SequenceOperator>(validator, this->patternSize,this->eventStates, this->workLoad, this->measurements);
        break;
    case Constants::OperatorType::STOCK_RISE_OPERATOR:
        abstractOperator = make_shared<StockRiseOperator>(validator, this->patternSize, this->workLoad,
                                                          this->measurements, this->symbols.get());
        break;
    case Constants::OperatorType::HEAD_AND_SHOULDERS_OPERATOR:
        abstractOperator = make_shared<RIPOperator>(validator, this->patternSize, this->workLoad,
                                                          this->measurements, this->symbols.get());
        break;
    }

    return abstractOperator;
}

OperatorFactory::~OperatorFactory() {}
}
