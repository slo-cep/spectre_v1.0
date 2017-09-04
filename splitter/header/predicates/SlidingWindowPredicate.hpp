/*
 * slidingwindowpredicate
 *
 * Created on: 29.10.2016
 *      Author: sload
 */

#ifndef SLIDINGWINDOWPREDICATE_HPP
#define SLIDINGWINDOWPREDICATE_HPP

#include "AbstractPredicate.hpp"

#include "header/events/AbstractEvent.hpp"
#include "header/selection/AbstractSelection.hpp"
#include "header/util/GlobalParameters.hpp"

namespace splitter
{

class SlidingWindowPredicate: public AbstractPredicate
{
public:

    /**
     * @brief SlidingWindowPredicate
     * @param selectionSize: window size
     * @param slidingSize: size of sliding
     */
    SlidingWindowPredicate(unsigned int selectionSize, unsigned int slidingSize);

    bool ps(const events::AbstractEvent & event) const override;
    bool pc(const events::AbstractEvent & event, selection::AbstractSelection & abstractSelection) override;

    virtual ~SlidingWindowPredicate();
private:
    unsigned int selectionSize;
    unsigned int slidingSize;

    mutable unsigned long currentPosition=0;
};

} /* namespace splitter */


#endif // SLIDINGWINDOWPREDICATE_HPP
