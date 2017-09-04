/*
 * AbstractOperator.hpp
 *
 *  Created on: Aug 5, 2016
 *      Author: sload
 */

#ifndef HEADER_ABSTRACTOPERATOR_HPP_
#define HEADER_ABSTRACTOPERATOR_HPP_
#include "header/util/GlobalTypedef.hpp"

#include "header/validator/AbstractValidator.hpp"

#include "../../header/execution_path/Cgroup.hpp"

#include "header/Measurements.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
using namespace std;
namespace execution_path
{

/**
 * represents operator interface where each operator should implement it
 */
class AbstractOperator
{
public:
    //AbstractOperator()=delete;
	//this should be moved to protected section!

    virtual shared_ptr<AbstractOperator> clone()=0;

	/**
	 * the main method which is called by our framework
	 */
	virtual void main() =0;

    /**
     * @brief recover: recover operator. Cgroups shouldn't loose their old pointer
     * @param other:rhs
     */
    virtual void recover(AbstractOperator* other);

    void setValidator(AbstractValidator *validator);

	/**
	 * add new event to the local vector of primitive events
	 */
    void receivePrimitiveEvent(const shared_ptr<events::AbstractEvent> &primitiveEvent);

	virtual ~AbstractOperator();
protected:
    vector<shared_ptr<events::AbstractEvent>> primitiveEvents;
    AbstractValidator* validator;
    profiler::Measurements*  measurements;

	/**
	 * Ctor
     *@param validator: pointer to the Validator
	 */
    AbstractOperator(AbstractValidator* validator, profiler::Measurements*  measurements);

    /**
     * @brief Copy constructor
     * @param other: rhs
     */
    AbstractOperator(const AbstractOperator& other);


     vector<shared_ptr<events::AbstractEvent>> tempEvents;
};

} /* namespace execution_path */

#endif /* HEADER_ABSTRACTOPERATOR_HPP_ */
