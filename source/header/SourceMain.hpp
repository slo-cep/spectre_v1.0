/*
 * SourceMain.h
 *
 *  Created on: Jul 28, 2016
 *      Author: sload
 */

#ifndef SOURCEMAIN_H_
#define SOURCEMAIN_H_
#include "header/util/Constants.hpp"
#include "header/util/Helper.hpp"
#include "header/events/AbstractEvent.hpp"
#include "header/events/SimpleValueEvent.hpp"
#include "header/events/EventFactory.hpp"
#include "header/shared_memory/SourceSplitterQueue.hpp"
#include "header/util/GlobalTypedef.hpp"

#include "header/Measurements.hpp"

#include <string>
#include <sstream> //istringstream
#include <fstream>
#include <exception>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <memory> //shared_ptr
#include <time.h>
#include <stdlib.h> //srand, rand
#include <iostream>

//Stuff for tcp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;


namespace source
{
class SourceMain
{
public:

    /**
	 * Ctor: generate events
	 * @param sourceSpliterStream: shared queue between source and Splitter
	 * @param play: play events from file or not (here if not we want to generate it)
	 * @param scenarioOrEventType: the type of used events (e.g.  Simple Value events
	 * @param numberOfEventsToBeProduced: number of events to generate
	 * @param interProductionTime: the time between event generation
	 * @param eventStates: event state (type!!) e.g. A, B, etc.
     * @param randomEventSize: number of random events (random events range)
     * @param measurements: for profiling
	 */
    SourceMain(shared_memory::AbstractSourceSplitterStream& sourceSpliterStream, bool play,
            util::Constants::EventType scenarioOrEventType, unsigned long numberOfEventsToBeProduced,
            unsigned long interProductionTime, vector<string> eventStates,  unsigned int randomEventSize, profiler::Measurements* measurements);


    /**
     * Ctor: generate events
     * @param sourceSpliterStream: shared queue between source and Splitter
     * @param play: play events from file or not (here if not we want to generate it)
     * @param scenarioOrEventType: the type of used events (e.g.  Simple Value events
     * @param portNumber: port number
     * @param measurements: for profiling
     */
    SourceMain(shared_memory::AbstractSourceSplitterStream& sourceSpliterStream, bool play,
            util::Constants::EventType scenarioOrEventType, int portNumber, profiler::Measurements *measurements);


    /**
     * @brief set the seed for random number generator, where default seed is time(NULL)
     * @param seed
     */
    void setSeed(unsigned int seed);

	void main();
	virtual ~SourceMain();

    void initSocket(int portno);
    unsigned long getNumberOfReceivedEvents() const;

private:
    shared_memory::AbstractSourceSplitterStream& sourceSpliterStream;

	bool play;
	util::Constants::EventType scenarioOrEventType;
	string path;
	unsigned long numberOfEventsToBeProduced = 0;
	unsigned long interProductionTime = 0; // in ms
    unsigned int seed;
	vector<string> eventStates;
    unsigned int randomEventSize=0;

    //for measurement
    unsigned long numberOfReceivedEvents=0;

    events::EventFactory* simpleValueEventFactory;
    profiler::Measurements*  measurements;


	/**
	 * generate events
     */
    void generateEvents();

    void generateEventsRandomly();

    bool generateEventContentFromString(string &input, unordered_map<string, string> &targetMap, unsigned long &timestamp);

    //Stuff for TCP
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    void readSocket();
};
}
#endif /* SOURCEMAIN_H_ */
