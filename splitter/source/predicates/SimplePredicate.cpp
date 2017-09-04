/*
 * SimplePredicate.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: sload
 */

#include "../../header/predicates/SimplePredicate.hpp"

using namespace events;
using namespace selection;
using namespace util;

namespace splitter
{

SimplePredicate::SimplePredicate(): AbstractPredicate()
{
	// TODO Auto-generated constructor stub

}

bool SimplePredicate::ps(const AbstractEvent & event) const
{
	if ((event.getContent().size() == 0) || (GlobalParameter::EVENT_STATE.size() == 0))
		return false;
	auto start_it = event.getContent().begin();

	/*
	 * if first event is A open a new selection
	 */
	if (start_it->second == GlobalParameter::EVENT_STATE[0])
		return true;
	else
		return false;
}

bool SimplePredicate::pc(const AbstractEvent & event, AbstractSelection & abstractSelection)
{
	if ((event.getContent().size() == 0) || (GlobalParameter::EVENT_STATE.size() == 0))
		return false;

	auto end_it = event.getContent().begin();

	/*
	 * if last event is C close the selection
	 */
	if (end_it->second == GlobalParameter::EVENT_STATE[GlobalParameter::EVENT_STATE.size() - 1])
		return true;
	else
		return false;
}

SimplePredicate::~SimplePredicate()
{
	// TODO Auto-generated destructor stub
}

} /* namespace splitter */
