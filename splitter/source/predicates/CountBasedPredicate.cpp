/*
 * CountBasedPredicate.cpp
 *
 * Created on: 27.10.2016
 *      Author: sload
 */

#include "../../header/predicates/CountBasedPredicate.hpp"

using namespace events;
using namespace selection;
using namespace util;

namespace splitter
{

CountBasedPredicate::CountBasedPredicate(unsigned int count) : AbstractPredicate()
{
    if (count == 0)
    {
        cout << "CountBasedPredicate.cpp: error count must be greater than zero!" << endl;
        exit(1);
    }

    this->count = count;
}

bool CountBasedPredicate::ps(const AbstractEvent &event) const
{
    if ((event.getContent().size() == 0) || (GlobalParameter::EVENT_STATE.size() == 0))
        return false;
    auto start_it = event.getContent().begin();

    /*
     * if first event is A open a new selection
     */
//    if (start_it->second == GlobalParameter::EVENT_STATE[0])
    if (start_it->second == "A")
        return true;
    else
        return false;
}

bool CountBasedPredicate::pc(const AbstractEvent &event, AbstractSelection &abstractSelection)
{
    //    int  size=abstractSelection.getSize();

    /*
     * if partition size equal to count close the selection
     */
    //    if ( size== this->count)
    //        return true;
    //    else
    //        return false;

    if (event.getSn() - abstractSelection.getStartPosition()->get()->getSn() +1  == this->count)
        return true;
    else
        return false;
}

CountBasedPredicate::~CountBasedPredicate()
{
    // TODO Auto-generated destructor stub
}

} /* namespace splitter */
