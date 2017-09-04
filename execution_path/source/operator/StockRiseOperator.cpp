#include "header/operator/StockRiseOperator.h"
#include "header/execution_path/ExecutionPath.hpp"

using namespace events;
using namespace util;
using namespace profiler;

namespace execution_path
{

/**
 * Ctor
 * @param validator: pointer to the Validator
 * @param eventStates: an array of event types which will be detected in a sequence order
 * @param workLoad: artificial work load in case of generating events
 * @param measurements: for profiling
 */
StockRiseOperator::StockRiseOperator(AbstractValidator *validator, unsigned int patternSize, unsigned long workLoad,
                                     Measurements *measurements, set<string> *symbols)
    : AbstractOperator(validator, measurements)
{
    this->patternSize = patternSize;
    this->workLoad = workLoad;
    this->symbols = symbols;
}

StockRiseOperator::StockRiseOperator(const StockRiseOperator &other) : AbstractOperator(other)
{
    this->patternSize = other.patternSize;
    this->cgroupHasBeenOpend = other.cgroupHasBeenOpend;

    if (other.cgroup)
        this->cgroup = other.cgroup->clone();

    this->partialMatchedEvents = other.partialMatchedEvents;
    this->workLoad = other.workLoad;
    this->rise = other.rise;

    this->symbols = other.symbols;
}

shared_ptr<AbstractOperator> StockRiseOperator::clone() { return make_shared<StockRiseOperator>(*this); }

void StockRiseOperator::recover(AbstractOperator *other)
{
    AbstractOperator::recover(other);

    StockRiseOperator *rhs = static_cast<StockRiseOperator *>(other);

    this->patternSize = rhs->patternSize;
    this->cgroupHasBeenOpend = rhs->cgroupHasBeenOpend;

    if (rhs->cgroup)
        this->cgroup->set(*rhs->cgroup);
    else
        this->cgroup = nullptr;

    this->partialMatchedEvents = rhs->partialMatchedEvents;

    this->workLoad = rhs->workLoad;
    this->rise = rhs->rise;
}

void StockRiseOperator::main()
{
    // start time
    //    unsigned long startTime = Helper::currentTimeMillis();

    // iterate over all available primitive events
    for (auto it_primitive = this->primitiveEvents.begin(); it_primitive != this->primitiveEvents.end(); it_primitive++)
    {

       auto &eventContent = (*it_primitive)->getContent();

        try
        {
            if (this->cgroupHasBeenOpend == false) // can only have one cgroup
            {
//                this->simulateLoad();
                // if (eventContent.at("SS") == "IBM")// && std::stof(eventContent.at("close")) >
                // std::stof(eventContent.at("open"))) //IBM rising

                if (this->symbols->find(eventContent.at("SS")) != this->symbols->end())
                //                if (eventContent.at("SS") == "AAAP" || eventContent.at("SS") == "FLWS")
                {
                    if (std::stof(eventContent.at("close")) > std::stof(eventContent.at("open")))
                    {
                        rise = true;
                    }
                    else
                    {
                        rise = false;
                    }
                    shared_ptr<Cgroup> newCgroup = make_shared<Cgroup>();
                    cgroup = newCgroup;
                    //                    cgroup->pushNewEvent(*it_primitive);
                    cgroup->pushNewEvent(it_primitive->get()->getSn());
                    cgroup->setEventsLeft(this->patternSize - 1);

                    // send created cgroup to the Validator
                    this->validator->receiveCgroup(cgroup, Cgroup::Status::NEW);

                    this->partialMatchedEvents.push_back(*it_primitive);
                    cgroupHasBeenOpend = true;
                }
            }
            //            else if (this->cgroup && eventContent.at("SS") != this->openEvent->getContent().at("SS")
            //                     && (rise
            //                         == (std::stof(eventContent.at("close")) > std::stof(eventContent.at("open")))))
            //                         // if cgroup
            //            exists and other is rising
            else if (this->cgroup && this->cgroup->getEventsLeft() > 0
                     && this->symbols->count(eventContent.at("SS")) == 0
                     && (rise == (std::stof(eventContent.at("close")) > std::stof(eventContent.at("open")))))
            {
//                this->simulateLoad();

                // Add to the cgroup
                this->cgroup->pushNewEvent(it_primitive->get()->getSn());
                this->validator->receiveCgroup(this->cgroup, Cgroup::Status::UPDATE);

                this->partialMatchedEvents.push_back(*it_primitive);

                // If cgroup is complete

                if (this->cgroup->getEventsLeft() == 0)
                {
                    //                    unsigned long sn = this->cgroup->getEvents()[0]->getSn();
                    unsigned long sn = this->cgroup->getEvents()[0];
                    shared_ptr<ComplexEvent> complexEvent
                        = make_shared<ComplexEvent>(sn, this->partialMatchedEvents, this->measurements);

                    // set the complex event timestamp as the timestamp of last event in the pattern
                    complexEvent.get()->setTimestamp(it_primitive->get()->getTimestamp());
                    // set the complex event real timestamp as the real timestamp of last event in the pattern
                    complexEvent.get()->setRealTimeStamp(it_primitive->get()->getRealTimeStamp());

                    this->validator->receiveComplexEvent(complexEvent);

                    // remove cgroup from local state
                    // this->cgroup = nullptr;
                }
            }
        }
        catch (...)
        {
            // Not a Stock trading event
        }
    } // end of for outer loop

    this->primitiveEvents.clear();

    // end time
    //    unsigned long endTime = Helper::currentTimeMillis();

    //    this->measurements.get()->increaseTime(Measurements::ComponentName::OPERATOR, endTime - startTime);
}


void StockRiseOperator::simulateLoad()
{
    unsigned long startTimeLoad = Helper::currentTimeMicros();
    unsigned long endTimeLoad = 0;
    do
    {
        endTimeLoad = Helper::currentTimeMicros();
    } while ((endTimeLoad - startTimeLoad) < 10);
}

StockRiseOperator::~StockRiseOperator()
{
    // TODO Auto-generated destructor stub
}

} /* namespace execution_path */
