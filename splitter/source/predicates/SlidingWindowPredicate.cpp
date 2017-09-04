/*
 * SlidingWindowPredicate.cpp
 *
 * Created on: 29.10.2016
 *      Author: sload
 */

#include "../../header/predicates/SlidingWindowPredicate.hpp"

using namespace events;
using namespace selection;
using namespace util;

namespace splitter
{

SlidingWindowPredicate::SlidingWindowPredicate(unsigned int selectionSize, unsigned int slidingSize): AbstractPredicate()
{
    if(selectionSize== 0 || slidingSize==0)
    {
        cout<<"SlidingWindowPredicate.cpp: error partitionSize or slidingSize must be greater than zero!"<<endl;
        exit(1);
    }

    this->selectionSize=selectionSize;
    this->slidingSize=slidingSize;

}

bool SlidingWindowPredicate::ps(const AbstractEvent & event) const
{
    /*
     * open new partition each a (slidingSize) event
     */
    this->currentPosition++;

    if ((currentPosition-1) %slidingSize == 0)
        return true;
    else
        return false;
}

bool SlidingWindowPredicate::pc(const AbstractEvent & event, AbstractSelection & abstractSelection)
{
    /*
     * if partition size equal to count close the selection
     */
    if (event.getSn() - abstractSelection.getStartPosition()->get()->getSn() +1  == this->selectionSize)
        return true;
    else
        return false;
}

SlidingWindowPredicate::~SlidingWindowPredicate()
{
    // TODO Auto-generated destructor stub
}

} /* namespace splitter */
