/*
 * Merger.hpp
 *
 *  Created on: Aug 2, 2016
 *      Author: sload
 */

#ifndef HEADER_MERGER_HPP_
#define HEADER_MERGER_HPP_

#include "header/events/events.hpp"

#include "header/selection/AbstractSelection.hpp"

#include "header/util/Constants.hpp"

#include "header/Measurements.hpp"

#include <fstream>
#include <string>
#include <iostream>


#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <map>
#include <utility> //pair

#include<boost/filesystem.hpp>

using namespace std;
namespace merger
{
/**
 * Merger brings the complex events in the right order!
 */
class Merger
{
public:

    Merger(profiler::Measurements* measurements);

    /**
     * @brief receive event (Complex or Pulse) and store this event in a temporal storage
     * protected by lock (mutex)
     * @param selectionId: Id of the selection that the event come from
     * @param event: complex or pulse event
     */
    void receiveEvent(unsigned long selectionId, shared_ptr<events::AbstractEvent> event);

    /**
     * store the new created selection
     * -It is important to reorder the complex events from different selections
     * -protected by lock (mutex)
     */
    void newSelection(shared_ptr<selection::AbstractSelection> abstractSelection);

    void setTerminate();

    void main();

    virtual ~Merger();

private:
   vector<pair<unsigned long ,shared_ptr<events::AbstractEvent> > > reorderedComplexEvents;

    // each selection has its specific vector to store events (Complex + Pulse)
    map<unsigned long ,vector<shared_ptr<events::AbstractEvent> >> eventsFromSelections;

    //temporal storage to reduce the locking overhead (selection Id, events)
    map <unsigned long,  vector<shared_ptr<events::AbstractEvent> >> waiting_eventsFromSelections;


    // store all selections
    map<unsigned long, shared_ptr<selection::AbstractSelection>> abstractSelections;
    /*
     * Temporary  store selections till they are copied to selections
     * -It should reduce the lock overhead
     */
     map<unsigned long, shared_ptr<selection::AbstractSelection>>  waiting_abstractSelections;

    mutable mutex mtx_events;
    mutable mutex mtx_selections;
    atomic_bool terminate={false};


    profiler::Measurements* measurements;

    /**
     * @brief copy waiting_selections and waiting_eventsFromSelections
     * protected by lock (mutex)
     */
    void copyWaitingSelectionAndEvents();


    /**
     * @brief reorder the complex events
     */
    void reorder();

    /**
     * @brief remove selection from selections,  eventsFromSelections and waiting_eventsFromSelections
     * @param it_selection: iterator to the current element to delete (from eventsFromSelections map)
     * @return iterator to the next element of the eventsFromSelections map (after removing current element)
     */
    //to make the function more clear, I used typedef
     typedef map <unsigned long,  vector<shared_ptr<events::AbstractEvent> >>::iterator EventIterator;
     EventIterator removeSelection(EventIterator it_selection);

    void printComplexEvents();

};

} /* namespace merger */

#endif /* HEADER_MERGER_HPP_ */
