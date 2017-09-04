/*
 * Splitter.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: sload
 */

#include "../../header/splitter/Splitter.hpp"

using namespace events;
using namespace util;
using namespace shared_memory;
using namespace selection;
using namespace execution_path;
using namespace merger;
using namespace profiler;
using namespace selection;
namespace splitter
{

Splitter::Splitter(SourceSplitterStreamTypedef *sourceSplitterStream, GlobalEventStreamTypedef *globalEventStream,
                   Merger *merger, AbstractPredicate *predicate, PathManager *pathManager,
                   unsigned int eventsPerSelection, Measurements *measurements)
    : sourceSplitterStream(sourceSplitterStream), globalEventStream(globalEventStream), merger(merger),
      predicate(predicate), pathManager(pathManager), eventsPerSelection(eventsPerSelection), measurements(measurements)
{
}

Splitter::Splitter(Splitter::SourceSplitterStreamTypedef *sourceSplitterStream,
                   Splitter::GlobalEventStreamTypedef *globalEventStream, Merger *merger, AbstractPredicate *predicate,
                   PathManager *pathManager, unsigned long workLoad, unsigned int eventsPerSelection,
                   Measurements *measurements)
    : sourceSplitterStream(sourceSplitterStream), globalEventStream(globalEventStream), merger(merger),
      predicate(predicate), pathManager(pathManager), eventsPerSelection(eventsPerSelection), workLoad(workLoad),
      measurements(measurements)
{
}

void Splitter::main()
{
    // start time
    unsigned long startTime = Helper::currentTimeMillis();

    /*
     * prevent any execution path to access to last event in the global event list for the following reason:
     *  When the execution path access the last event it increases its selection's currentPosition which
     *  will point to the end of the list. So  the selection can't any more retrieve any other event.
     * Therefore, we prevent the execution path to access to the last event in the global event list.
     */
    unsigned long lastEventSn = -1;

    while (true)
    {
        shared_ptr<AbstractEvent> event = this->sourceSplitterStream->pop(); // if there is no event pop returns NULL

        if (event == NULL) // spin till get an event
        {
            this_thread::yield();
            continue;
        }

        // no more events
        if (event.get()->getType() == Constants::EventType::END_OF_STREAM)
            break;

        auto it_globalEventStream = this->globalEventStream->insert(event);

        /*
         * check if the event open new selection
         */
        bool open_selection = predicate->ps(*event.get());
        if (open_selection)
        {
            AbstractSelection *predecessorSelection
                = this->activeSelections.size() > 0 ? this->activeSelections[this->activeSelections.size() - 1].get()
                                                    : NULL;

            unsigned long Id = AbstractSelection::generateNextId();
            shared_ptr<AbstractSelection> abstractSelection = make_shared<SimpleSelectionWithoutLock>(
                Id, *this->globalEventStream, predecessorSelection, it_globalEventStream, this->measurements);
//            abstractSelection->setLastEventSn(event->getSn());

            //            shared_ptr<AbstractSelection> abstractSelection = make_shared<SimpleSelectionWithMinimumLock>(
            //                Id, *this->globalEventStream, predecessorSelection, it_globalEventStream,
            //                this->measurements);

            this->activeSelections.push_back(abstractSelection);

            //            this->merger.get()->newSelection(abstractSelection);

            this->pathManager->receiveNewSelection(abstractSelection);

            this->numberOfSelections++;
        }

        /*
         * check if the event close any opened selection
         */
        /*
        for (auto it_selection = this->abstractSelections.begin(); it_selection != this->abstractSelections.end();)
        {
            bool close_selection = this->predicate.get()->pc(*event.get(), *it_selection->get());
            if (close_selection)
            {
                it_selection->get()->setLastPosition(it_globalEventStream); // set the lastIndex

                // move the selection to closedSelection for debugging purpose only
                this->closedAbstractSelections.push_back(*it_selection);
                // remove the selection from the splitter
                it_selection = this->abstractSelections.erase(it_selection);
            }
            else
            {
                it_selection->get()->pushNewEvent(it_globalEventStream);

                it_selection++;
            }
        }*/

        for (auto it_selection = this->activeSelections.begin(); it_selection != this->activeSelections.end();)
        {
            bool close_selection = this->predicate->pc(*event.get(), *it_selection->get());
            if (close_selection)
            {
                //                it_selection->get()->setLastPosition(it_globalEventStream); // set the lastIndex
                it_selection->get()->setLastEventSn(event->getSn());

                // gather statistics about number of events per a selection
                unsigned long numEvents
                    = it_globalEventStream->get()->getSn() - it_selection->get()->getStartPosition()->get()->getSn();
                this->eventsPerSelection = (this->eventsPerSelection + numEvents) / 2;

                this->pathManager->setSelectionSize(eventsPerSelection);

                this->closedSelections.push_back(*it_selection);

//                 this->printSelection(it_selection->get());

                // remove the selection from the splitter
                it_selection = this->activeSelections.erase(it_selection);
            }
            else
                break;
        }

        for (auto it_selection = this->activeSelections.begin(); it_selection != this->activeSelections.end();)
        {
            it_selection->get()->setLastEventSn(event->getSn());

            it_selection++;
        }

        lastEventSn++;
        this->globalEventStream->setLastEventSn(lastEventSn);

        // clean the closed selections and global events stream
        if (this->closedSelections.size() > 10)
        {
            for (auto it_selection = this->closedSelections.begin(); it_selection != this->closedSelections.end();)
            {
                // the selection is ready to be removed so we can delete all its events
                // delete only the events that don't overlap with other selections

                if (it_selection->get()->isReadyToBeRemoved()
                    && it_selection->get()->getBasicSelection().use_count() == 1)
                {
                    cleanGlobalEventStream();
                    it_selection = this->closedSelections.erase(it_selection);
                }
                else
                    // we delete in a sequential manner
                    break;
            }
        }

        //        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // end time
    unsigned long endTime = Helper::currentTimeMillis();

    /*
     * not totally OK but for this prototype maybe it is sufficient
     * problems:
     * - return current position to last_it, in case it has already reached pulse_it. This will
     *      cause last_it to be feeded twice
     */

    // close all remaining unclosed selections
    /*
     * deactivate the last event:
     * -In case the currentIndex is already referring to pulse event (fake event), it should be returned to
     *   the last event.
     */
    //    this->globalEventStream->setActivateLastEvent(false);

    for (auto it_selection = this->activeSelections.begin(); it_selection != this->activeSelections.end();)
    {

        auto last_it = --this->globalEventStream->getEndIterator();
        it_selection->get()->setLastEventSn(last_it->get()->getSn());

//                this->printSelection(it_selection->get());

        this->closedSelections.push_back(*it_selection);
        // remove the selection from the splitter
        it_selection = this->activeSelections.erase(it_selection);
    }

    EventFactory factory(Constants::EventType::PULSE, measurements);
    shared_ptr<AbstractEvent> fakeEvent = factory.createNewEvent();
    this->globalEventStream->insert(fakeEvent);

    lastEventSn++;
    this->globalEventStream->setLastEventSn(lastEventSn);

    // clean
    while (this->closedSelections.size() != 0 && !this->terminate.load())
    {
        for (auto it_selection = this->closedSelections.begin(); it_selection != this->closedSelections.end();)
        {
            // the selection is ready to be removed so we can delete all its events
            // delete only the events that don't overlap with other selections
            if (it_selection->get()->isReadyToBeRemoved() && it_selection->get()->getBasicSelection().use_count() == 1)
            {
                cleanGlobalEventStream();
                it_selection = this->closedSelections.erase(it_selection);
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(100));
                // we delete in a sequential manner
                break;
            }
        }
    }

    cout << "Splitter time:" << endTime - startTime << endl;

    // end time
    //    unsigned long endTime = Helper::currentTimeMillis();

    //    this->measurements.get()->setTime(Measurements::ComponentName::SPLITTER, endTime - startTime);
    //    this->measurements.get()->setTime(Measurements::ComponentName::SELECTIONS, this->numberOfSelections);
}

void Splitter::setTerminate(bool terminate) { this->terminate.store(terminate); }

void Splitter::cleanGlobalEventStream()
{
    if (this->closedSelections.size() == 0)
    {
        cout << "Splitter; error calling function cleanGlobalEventStream as closedAbstractSelections size is zero"
             << endl;
        exit(-1);
    }

    AbstractSelection *startSelection = this->closedSelections[0].get();

    AbstractSelection *nextSelection = nullptr;

    // check if there is any closed selection
    if (this->closedSelections.size() > 1)
        nextSelection = this->closedSelections[1].get();
    else // check if there is any opened selection
        if (this->activeSelections.size() != 0)
        nextSelection = this->activeSelections[0].get();

    // count how many events (to where) should delete

    GlobalEventStreamTypedef_iterator start_it = this->globalEventStream->getStartIterator();
    GlobalEventStreamTypedef_iterator end_it;

    // there is no other Selections, so we can delete all events
    if (nextSelection == nullptr)
    {
        end_it = this->globalEventStream->getEndIterator();
        end_it--; // keep the fake event
    }
    else // delete till next Selection
        end_it = nextSelection->getStartPosition();

    this->globalEventStream->clean(start_it, end_it);

    //    cout << "Splitter: clear Selection: " << startSelection->getId() << ", last event: " << end_it->get()->getSn()
    //         << endl;
}

void Splitter::printSelection(AbstractSelection *selection)
{
    ofstream outputFile;
    outputFile.open("./selections_splitter.txt", ios::app);

    //    for (auto it_selection = this->closedAbstractSelections.begin();
    //         it_selection != this->closedAbstractSelections.end(); it_selection++)
    //    {
    //        outputFile << it_selection->get()->toString() << endl;
    //    }

    outputFile << selection->toString() << endl;

    outputFile.close();
}

Splitter::~Splitter()
{
    // TODO Auto-generated destructor stub
}

} /* namespace execution_path */
