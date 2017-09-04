/*
 * CountBasedPredicate.hpp
 *
 * Created on: 27.10.2016
 *      Author: sload
 */

#ifndef COUNTBASEDPREDICATE_HPP
#define COUNTBASEDPREDICATE_HPP

#include "AbstractPredicate.hpp"

#include "header/events/AbstractEvent.hpp"
#include "header/selection/AbstractSelection.hpp"
#include "header/util/GlobalParameters.hpp"

namespace splitter
{

class CountBasedPredicate: public AbstractPredicate
{
public:

    /**
     * @brief CountBasedPredicate
     * @param count: window (partition) size
     */
    CountBasedPredicate(unsigned int count);

    bool ps(const events::AbstractEvent & event) const override;
    bool pc(const events::AbstractEvent & event, selection::AbstractSelection & abstractSelection) override;

    virtual ~CountBasedPredicate();
private:
    unsigned int count;
};

} /* namespace splitter */

#endif // COUNTBASEDPREDICATE_HPP
