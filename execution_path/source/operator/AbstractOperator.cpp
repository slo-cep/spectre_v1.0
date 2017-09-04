/*
 * AbstractOperator.cpp
 *
 *  Created on: Aug 10, 2016
 *      Author: sload
 */
#include "../../header/operator/AbstractOperator.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace  events;
namespace execution_path
{
/**
 * Ctor
 *@param validator: pointer to the Validator
 */
AbstractOperator::AbstractOperator(AbstractValidator *validator, profiler::Measurements *measurements)
    : validator(validator), measurements(measurements)
{
}

AbstractOperator::AbstractOperator(const AbstractOperator &other)
    : validator(other.validator), measurements(other.measurements)
{
    this->primitiveEvents = other.primitiveEvents;
    this->tempEvents = other.tempEvents;
    this->validator = other.validator;
}

void AbstractOperator::recover(AbstractOperator *other)
{
    this->validator = other->validator;
    this->measurements=other->measurements;
    this->primitiveEvents = other->primitiveEvents;
    this->tempEvents = other->tempEvents;
}

void AbstractOperator::setValidator(AbstractValidator *validator) { this->validator = validator; }

void AbstractOperator::receivePrimitiveEvent(const shared_ptr<AbstractEvent> &primitiveEvent)
{
    this->primitiveEvents.push_back(primitiveEvent);

    this->tempEvents.push_back(primitiveEvent);
}

AbstractOperator::~AbstractOperator() {}
}
