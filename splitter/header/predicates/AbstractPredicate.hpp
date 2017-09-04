/*
 * AbstractPredicate.hpp
 *
 *  Created on: Aug 24, 2016
 *      Author: sload
 */

#ifndef HEADER_PREDICATES_ABSTRACTPREDICATE_HPP_
#define HEADER_PREDICATES_ABSTRACTPREDICATE_HPP_

#include "header/events/AbstractEvent.hpp"
#include "header/util/GlobalParameters.hpp"

#include "header/selection/AbstractSelection.hpp"

namespace splitter
{
class AbstractPredicate
{
public:

	virtual bool ps(const events::AbstractEvent & event) const=0;
    virtual bool pc(const events::AbstractEvent & event, selection::AbstractSelection & abstractSelection)=0;


    virtual ~AbstractPredicate()
	{
	}
protected:
    AbstractPredicate(){}
};
}

#endif /* HEADER_PREDICATES_ABSTRACTPREDICATE_HPP_ */
