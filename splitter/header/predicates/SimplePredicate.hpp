/*
 * SimplePredicate.hpp
 *
 *  Created on: Aug 24, 2016
 *      Author: sload
 */

#ifndef HEADER_PREDICATES_SIMPLEPREDICATE_HPP_
#define HEADER_PREDICATES_SIMPLEPREDICATE_HPP_

#include "AbstractPredicate.hpp"

#include "header/events/AbstractEvent.hpp"
#include "header/selection/AbstractSelection.hpp"
#include "header/util/GlobalParameters.hpp"

namespace splitter
{

class SimplePredicate: public AbstractPredicate
{
public:

	SimplePredicate();

	bool ps(const events::AbstractEvent & event) const override;
    bool pc(const events::AbstractEvent & event, selection::AbstractSelection & abstractSelection) override;

	virtual ~SimplePredicate();
};

} /* namespace splitter */

#endif /* HEADER_PREDICATES_SIMPLEPREDICATE_HPP_ */
