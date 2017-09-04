/*
 * Merger.cpp
 *
 *  Created on: Aug 2, 2016
 *      Author: sload
 */

#include "../header/Merger.hpp"

using namespace events;
using namespace selection;
using namespace util;
using namespace profiler;
namespace merger
{

Merger::Merger(Measurements *measurements) { this->measurements = measurements; }

/**
 * @brief receive event (Complex or Pulse) and store this event in a temporal
 * storage
 * protected by lock (mutex)
 * @param selectionId: Id of the selection that the event come from
 * @param event: complex or pulse event
 */
void Merger::receiveEvent(unsigned long selectionId, shared_ptr<events::AbstractEvent> event)
{
    lock_guard<mutex> lk(mtx_events);
    this->waiting_eventsFromSelections[selectionId].push_back(event);

    //    cout << "Merger.cpp: receiveEvent(), received new event from selectionId= " << selectionId
    //         << ", waiting_eventsFromSelections.size()= " << this->waiting_eventsFromSelections[selectionId].size() <<
    //         endl;
}

/**
 * store the new created selection
 * -It is important to reorder the complex events from different selections
 * -protected by lock (mutex)
 */
void Merger::newSelection(shared_ptr<AbstractSelection> abstractSelection)
{
    lock_guard<mutex> lk(mtx_selections);
    this->waiting_abstractSelections[abstractSelection.get()->getId()] = abstractSelection;
}

void Merger::setTerminate() { this->terminate.store(terminate); }

void Merger::main()
{
    // start time
//    unsigned long startTime = Helper::currentTimeMillis();

    while (!terminate.load() || !this->waiting_abstractSelections.empty())
    {
        do
        {
            this->copyWaitingSelectionAndEvents();

            this->reorder();

        } while (!this->abstractSelections.empty() || !this->waiting_abstractSelections.empty());
    }

    // end time
//    unsigned long endTime = Helper::currentTimeMillis();

//    this->measurements.get()->setTime(Measurements::ComponentName::MERGER, endTime - startTime);

    this->printComplexEvents();
}

/**
 * @brief copy waiting_selections and waiting_eventsFromSelections
 * protected by lock (mutex)
 */
void Merger::copyWaitingSelectionAndEvents()
{
    unique_lock<mutex> lk_selections(mtx_selections);
    // cout<<"Merger.cpp: "<< this->waiting_selections.size()<<endl;
    // add waiting selections to selections map
    for (auto it = this->waiting_abstractSelections.begin(); it != this->waiting_abstractSelections.end(); it++)
    {
        this->abstractSelections[it->first] = it->second;
    }
    this->waiting_abstractSelections.clear();
    lk_selections.unlock(); // free  waiting_selections

    unique_lock<mutex> lk_events(mtx_events);

    for (auto it_selection = this->waiting_eventsFromSelections.begin();
         it_selection != this->waiting_eventsFromSelections.end(); it_selection++)
    {
        // iterator over all waiting events and copy them to eventsFromSelection
        for (auto it_event = it_selection->second.begin(); it_event != it_selection->second.end(); it_event++)
        {
            /*
             *Filter pulse events: (I did not use filterPulseEvents() it costs more)
             *  before adding the event to a selection, check if the last event is pulse event
             *  if it is pulse event, remove it and then add the new event
             */
            if (this->eventsFromSelections[it_selection->first].size() != 0)
                if (this->eventsFromSelections[it_selection->first]
                                              [this->eventsFromSelections[it_selection->first].size() - 1]
                                                  .get()
                                                  ->getType()
                    == Constants::EventType::PULSE)
                {
                    this->eventsFromSelections[it_selection->first].erase(
                        --this->eventsFromSelections[it_selection->first].end());
                }

            this->eventsFromSelections[it_selection->first].push_back(*it_event);
        }
        // clear the waiting events vector
        this->waiting_eventsFromSelections[it_selection->first].clear();
    }
    lk_events.unlock();
}

/**
 * @brief reorder the complex events
 */
void Merger::reorder()
{
    //  cout<<"Merger.cpp: eventsFromSelections.size()= "<<this->eventsFromSelections.size()<<endl;
    bool selectionDone;
    // iterator over all selection
    for (auto it_selection = this->eventsFromSelections.begin(); it_selection != this->eventsFromSelections.end();)
    {
        selectionDone = false;
        // iterator over all events from a specific selection
        for (auto it_event = it_selection->second.begin(); it_event != it_selection->second.end();)
        {
            /*
             * if the event is end of selection event, remove the selection from Merger
             * and move to the next selection (removeSelection() returns the next selection)
             */
            if (it_event->get()->getType() == Constants::EventType::END_OF_SELECTION)
            {
                it_selection = this->removeSelection(it_selection);
                break; // break the iteration on the events and move to the next selection
            }
            // if it is pulse event move to the next selection
            // Pulse event can only be the last event in any selection (because we filter them on addition)
            else if (it_event->get()->getType() == Constants::EventType::PULSE)
            {
                it_selection++;
                break; // break the iteration on the events and move to the next selection
            }

            // it is a complex event

            // iterator over all other selections
            for (auto it_selection_other = this->eventsFromSelections.begin();
                 it_selection_other != this->eventsFromSelections.end(); it_selection_other++)
            {
                if (it_selection == it_selection_other) // same selection
                    continue;

                /*
                 * if a selection doesn't contain any event till now
                 */
                if (it_selection_other->second.size() == 0)
                {
                    /*if its start time is earlier than
                     *the complex event time, then we cannot reorder (should wait for events from this
                     * it_selection_other
                     */
                    if (this->abstractSelections[it_selection_other->first].get()->getStartTimestamp()
                        <= it_event->get()->getTimestamp())
                    {
                        selectionDone = true;
                        break;
                    }
                }
                // size!=0 the selection contains some events
                // if the complex event (it_event) has higher timestamp than the other, stop reordering for this event
                else if (it_event->get()->getTimestamp() > it_selection_other->second[0].get()->getTimestamp())
                {
                    // only we need to compare with the first event of each selection (it_selection_other)
                    selectionDone = true;
                    break;
                }
            }

            if (selectionDone) // this event cannot be reordered,  proceed to next selection
            {
                it_selection++;
                break; // break second for loop (it_event loop)
            }
            // the event is the earliest event among all other events of all other selections
            else
            {
                pair<unsigned long, shared_ptr<AbstractEvent>> p(it_selection->first, *it_event);
                this->reorderedComplexEvents.push_back(p);

                auto it_event_temp = it_event;
                it_event = this->eventsFromSelections[it_selection->first].erase(it_event);

                // this event is the last event in the selection
                if (this->eventsFromSelections[it_selection->first].size() == 0)
                {
                    /*Add Pulse event to the selection using the timestamp of the
                     * deleted event to enable other selection to proceed
                     */
                    EventFactory eventFactory(Constants::EventType::PULSE, this->measurements);
                    shared_ptr<AbstractEvent> pulseEvent = eventFactory.createNewEvent();
                    pulseEvent.get()->setTimestamp(it_event_temp->get()->getTimestamp());
                    this->eventsFromSelections[it_selection->first].push_back(pulseEvent);

                    break; // break second for loop (it_event loop)
                }
            }
        }
    }
}

/**
 * @brief remove selection from selections,  eventsFromSelections and waiting_eventsFromSelections
 * @param it_selection: iterator to the current element to delete (from eventsFromSelections map)
 * @return iterator to the next element of the eventsFromSelections map (after removing current element)
 */
// to make the function more clear, I used typedef
Merger::EventIterator Merger::removeSelection(EventIterator it_selection)
{

    // erase by selectionId
    // the selection is still in waitings_selection!
    if (this->abstractSelections.count(it_selection->first) == 0)
    {
        this->waiting_abstractSelections.erase(it_selection->first);
    }
    else
    {
        this->abstractSelections.erase(it_selection->first);
    }
    // erase by iterator
    auto it = this->eventsFromSelections.erase(it_selection);

    unique_lock<mutex> lk(mtx_events);
    // erase by selectionId
    this->waiting_eventsFromSelections.erase(it_selection->first);
    lk.unlock();

    return it;
}

void Merger::printComplexEvents()
{
    ofstream outputFile;

    outputFile.open("/home/sload/multicore_cep/experimental_results/merger_reordered.txt", ios::out);
//    outputFile.open("./experimental_results/merger_reordered.txt", ios::out);
    for (auto it_selection = this->reorderedComplexEvents.begin(); it_selection != this->reorderedComplexEvents.end();
         it_selection++)
    {
//        this->measurements.get()->increaseTime(Measurements::ComponentName::COMPLEX_EVENTS_COUNT, 1);
        outputFile << "{SelectionId:" << it_selection->first << "}," << it_selection->second.get()->toString() << endl;
    }
    outputFile.close();
}

Merger::~Merger()
{
    // TODO Auto-generated destructor stub
}

} /* namespace merger */
