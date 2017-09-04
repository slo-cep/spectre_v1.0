/*
 * Splitter.hpp
 *
 *  Created on: Aug 22, 2016
 *      Author: sload
 */

#ifndef HEADER_SPLITTER_HPP_
#define HEADER_SPLITTER_HPP_
#include "header/events/AbstractEvent.hpp"
#include "header/events/SimpleValueEvent.hpp"

#include "header/shared_memory/AbstractSourceSplitterStream.hpp"
#include "header/shared_memory/GlobalEventStreamList.hpp"

#include "header/util/Constants.hpp"
#include "header/util/GlobalParameters.hpp"
#include "header/util/GlobalTypedef.hpp"

#include "header/selection/AbstractSelection.hpp"
#include "header/selection/SimpleSelection.hpp"
#include "header/selection/SimpleSelectionWithMinimumLock.hpp"
#include "header/selection/SimpleSelectionWithoutLock.hpp"

#include "header/path_manager/PathManager.hpp"

#include "../../header/predicates/AbstractPredicate.hpp"

#include "header/Merger.hpp"

#include "header/Measurements.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace  profiler;

namespace splitter
{

class Splitter
{
public:
    typedef util::GlobalTypedef::Container Container;
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef util::GlobalTypedef::GlobalEventStreamTypedef_iterator GlobalEventStreamTypedef_iterator;
    typedef util::GlobalTypedef::SourceSplitterStreamTypedef SourceSplitterStreamTypedef;

    /**
     *Ctor
     *@param sourceSplitterStream: the stream between sources and splitter
     *@param globalEventStream: global event stream which is shared between the splitter an all operator instances
     *@param merger: splitter sends the selection to the merger
     *@param predicate: the predicate to be used by splitter
     *@param pathManager: pointer to the path manager to send the execution paths to it.
     * @param eventsPerSelection: how many events each Selection contains?
     *@param measurements: for profiling
     */
    Splitter(SourceSplitterStreamTypedef* sourceSplitterStream,
             GlobalEventStreamTypedef* globalEventStream, merger::Merger* meger,
             AbstractPredicate* predicate, execution_path::PathManager* pathManager, unsigned int eventsPerSelection,
             Measurements* measurements);

    /**
     *Ctor
     *@param sourceSplitterStream: the stream between sources and splitter
     *@param globalEventStream: global event stream which is shared between the splitter an all operator instances
     *@param merger: splitter sends the selection to the merger
     *@param predicate: the predicate to be used by splitter
     *@param pathManager: pointer to the path manager to send the execution paths to it.
     *@param workLoad: artificial work load in case of generating events
     *@param eventsPerSelection: how many events each Selection contains?
     *@param measurements: for profiling
     */
    Splitter(SourceSplitterStreamTypedef *sourceSplitterStream,
             GlobalEventStreamTypedef *globalEventStream, merger::Merger *meger,
             AbstractPredicate *predicate, execution_path::PathManager *pathManager,
             unsigned long workLoad, unsigned int eventsPerSelection, Measurements *measurements);

    void main();

    void setTerminate(bool terminate);

    virtual ~Splitter();

private:
    SourceSplitterStreamTypedef* sourceSplitterStream;
    GlobalEventStreamTypedef* globalEventStream;
    merger::Merger* merger;
    AbstractPredicate* predicate;
    execution_path::PathManager* pathManager;

    unsigned int eventsPerSelection=0;

    vector<shared_ptr<selection::AbstractSelection>> activeSelections;
    // for debugging purpose only
    vector<shared_ptr<selection::AbstractSelection>> closedSelections;

    // artificial work load in case of generating events
    unsigned long workLoad;

    profiler::Measurements* measurements;

    // for debugging/measurement only!
    unsigned long numberOfSelections = 0;

    atomic<bool> terminate={false};


    void cleanGlobalEventStream();

    /**
     * only for debugging purpose
     */
    void printSelection(selection::AbstractSelection *selection);
};
} /* namespace execution_path */

#endif /* HEADER_SPLITTER_HPP_ */
