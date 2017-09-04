/*
 * SourceMain.cpp
 *
 *  Created on: Jul 28, 2016
 *      Author: sload
 */

#include "../header/SourceMain.hpp"

using namespace util;
using namespace events;
using namespace profiler;
using namespace shared_memory;

namespace source
{

SourceMain::SourceMain(AbstractSourceSplitterStream &sourceSpliterStream, bool play,
                       Constants::EventType scenarioOrEventType, unsigned long numberOfEventsToBeProduced,
                       unsigned long interProductionTime, vector<string> eventStates, unsigned int randomEventSize,
                       Measurements *measurements)
    : sourceSpliterStream(sourceSpliterStream)

{
    this->play = play;
    this->scenarioOrEventType = scenarioOrEventType;
    this->numberOfEventsToBeProduced = numberOfEventsToBeProduced;
    this->interProductionTime = interProductionTime;
    this->seed = static_cast<unsigned int>(time(NULL));
    this->eventStates = eventStates;
    this->randomEventSize = randomEventSize;

    this->simpleValueEventFactory = new EventFactory(Constants::EventType::SIMPLE_VALUE, measurements);
    this->measurements = measurements;
}

SourceMain::SourceMain(shared_memory::AbstractSourceSplitterStream &sourceSpliterStream, bool play,
                       Constants::EventType scenarioOrEventType, int portNumber, Measurements *measurements)
    : sourceSpliterStream(sourceSpliterStream)

{
    this->play = play;
    this->scenarioOrEventType = scenarioOrEventType;
    //    this->portno = portNumber;
    initSocket(portNumber);

    this->simpleValueEventFactory = new EventFactory(Constants::EventType::SIMPLE_VALUE, measurements);
    this->measurements = measurements;
}

void SourceMain::setSeed(unsigned int seed) { this->seed = seed; }

void SourceMain::main()
{
    //	unsigned long sourceId;
    // start time
    // unsigned long startTime = Helper::currentTimeMillis();

    if (!play) // generate
    {
//        this->generateEvents();

                this->generateEventsRandomly();
    }
    else
    {
        if (scenarioOrEventType == Constants::EventType::SCENARIO_PLAYLOG)
        {
            string eventString;
            ifstream fileReader;
            try
            {
                fileReader.open(path, ios::in);
                unsigned long startPlay = Helper::currentTimeMillis();
                while (getline(fileReader, eventString))
                {
                    unsigned long timeStamp;
                    try
                    {
                        vector<string> elements = Helper::split(eventString, Constants::PARAMETER_DELIMITER);
                        timeStamp = stol(elements[0]);
                        while ((Helper::currentTimeMillis() - startPlay) < timeStamp)
                            this_thread::sleep_for(chrono::milliseconds(1));
                    }
                    catch (exception &e)
                    {
                        cout << e.what();
                        fileReader.close();
                        exit(EXIT_FAILURE);
                    }
                    // TODO this->sourceSpliterQueue.push(eventString);
                }
            }
            catch (exception &e)
            {
                cout << e.what();
                fileReader.close();
                exit(EXIT_FAILURE);
            }

            fileReader.close();
        }
    }
    // else { // events are produced live, not played from an event trace file}

    // end time
    //    unsigned long endTime = Helper::currentTimeMillis();

    //    this->measurements.get()->setTime(Measurements::ComponentName::SOURCE, endTime - startTime);

    return;
}

/**
 * generate events
 */
void SourceMain::generateEventsRandomly()
{
    size_t eventIndex;

    if (this->scenarioOrEventType == Constants::EventType::SIMPLE_VALUE)
    {
        EventFactory eventFactory(Constants::EventType::SIMPLE_VALUE, this->measurements);

        // initialize random seed:
        srand(this->seed);

        size_t moduleSize = this->randomEventSize;

        //        ofstream outputFile;
        //        outputFile.open("./generated_events.txt", ios::out);
        for (unsigned long count = 0; count < numberOfEventsToBeProduced; count++)
        {
            // generate random number (eventIndex) between 0 and eventStates.size() - 1
            eventIndex = rand() % moduleSize;

            string eventContent;
            if (eventIndex >= this->eventStates.size())
                eventContent = to_string(eventIndex);
            else
                eventContent = this->eventStates[eventIndex];

            shared_ptr<AbstractEvent> event = eventFactory.createNewEvent();
            // set the content
            static_pointer_cast<SimpleValueEvent>(event).get()->setContent(eventContent);

            while (!this->sourceSpliterStream.push(event))
                ;

            //            outputFile << event.get()->toString() << endl;
            // sleep
            //             this_thread::sleep_for(chrono::microseconds(this->interProductionTime));
        }

        //        outputFile.close();

        // send end of stream event
        EventFactory EOFeventFactory(Constants::EventType::END_OF_STREAM, this->measurements);
        shared_ptr<AbstractEvent> event = EOFeventFactory.createNewEvent();
        while (!this->sourceSpliterStream.push(event))
            ;

        this->numberOfReceivedEvents = this->numberOfEventsToBeProduced;
    }
    else
    {
        cout << "error: event type is not supported!" << endl;
        return;
    }
}

void SourceMain::generateEvents()
{
    string stringbuffer = "";
    unsigned long timestamp;
    unordered_map<string, string> contents;

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
    {
        cout << "ERROR on accept";
        exit(-1);
    }

    if (this->scenarioOrEventType == Constants::EventType::SIMPLE_VALUE)
    {
        //        EventFactory eventFactory(Constants::EventType::SIMPLE_VALUE, this->measurements);

        //        ofstream outputFile;
        //        outputFile.open("./generated_events.txt", ios::out);
        while (1)
        {
            readSocket();
            if (buffer[0] == 0) // terminator
            {
                // send end of stream event
                //                EventFactory EOFeventFactory(Constants::EventType::END_OF_STREAM, this->measurements);
                //                shared_ptr<AbstractEvent> event = EOFeventFactory.createNewEvent();
                //                this->sourceSpliterStream.push(event);
                break;
            }
            stringbuffer += buffer;

            while (generateEventContentFromString(stringbuffer, contents, timestamp))
            {
                shared_ptr<AbstractEvent> event = simpleValueEventFactory->createNewEvent();

                // set the content
                static_pointer_cast<SimpleValueEvent>(event).get()->setContent(contents);
                static_pointer_cast<SimpleValueEvent>(event).get()->setTimestamp(timestamp);
                static_pointer_cast<SimpleValueEvent>(event).get()->setRealTimeStamp(Helper::currentTimeMillis());

                while (!this->sourceSpliterStream.push(event))
                    ;

                this->numberOfReceivedEvents++;

                //                outputFile << event.get()->toString() << endl;
            }
        }
        close(newsockfd);
        close(sockfd);

        //        outputFile.close();

        // send end of stream event
        EventFactory EOFeventFactory(Constants::EventType::END_OF_STREAM, this->measurements);
        shared_ptr<AbstractEvent> event = EOFeventFactory.createNewEvent();
        while (!this->sourceSpliterStream.push(event))
            ;
    }
    else
    {
        cout << "error: event type is not supported!" << endl;
        return;
    }
}

bool SourceMain::generateEventContentFromString(string &input, unordered_map<string, string> &targetMap,
                                                unsigned long &timestamp)
{
    string subStr;
    size_t pos1 = input.find('\n');
    size_t posNext = input.find('\n');

    size_t pos;

    if (pos1 == std::string::npos)
    {
        return false;
    }
    subStr = input.substr(0, pos1);
    input.erase(0, pos1 + 1);
    pos = subStr.find(",");
    targetMap["SS"] = subStr.substr(0, pos);
    posNext = subStr.find(",", pos + 1);
    try
    {
        timestamp = std::stoul(subStr.substr(pos + 1, posNext - pos - 1));
    }
    catch (...)
    {
        cout << "source main crashes: " << input << ": " << subStr << ": " << pos << ": " << std::string::npos
             << ": pos1" << pos1 << endl;
    }

    pos = posNext;
    posNext = subStr.find(",", pos + 1);
    targetMap["open"] = subStr.substr(pos + 1, posNext - pos - 1);
    pos = posNext;
    posNext = subStr.find(",", pos + 1);
    targetMap["high"] = subStr.substr(pos + 1, posNext - pos - 1);
    pos = posNext;
    posNext = subStr.find(",", pos + 1);
    targetMap["low"] = subStr.substr(pos + 1, posNext - pos - 1);
    pos = posNext;
    posNext = subStr.find(",", pos + 1);
    targetMap["close"] = subStr.substr(pos + 1, posNext - pos - 1);
    return true;
}

void SourceMain::readSocket()
{
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0)
        cout << "ERROR reading from socket";
}

SourceMain::~SourceMain() {}

void SourceMain::initSocket(int portno)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "ERROR opening socket";
        exit(-1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    this->portno = portno;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "ERROR on binding";
        exit(-1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
}

unsigned long SourceMain::getNumberOfReceivedEvents() const { return numberOfReceivedEvents; }
}
