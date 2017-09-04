/*
 * Cgroup.cpp
 *
 * Created on: 23.11.2016
 *      Author: sload
 */

#include "../../header/execution_path/Cgroup.hpp"

namespace execution_path
{
unsigned long Cgroup::generateCgroupId() { return lastCgroupId.operator++(); }
atomic<unsigned long> Cgroup::lastCgroupId{0};

Cgroup::Cgroup() : enable_shared_from_this<Cgroup>(), Id(generateCgroupId())
{
    //    this->eventsIndex.reserve(100);
}

Cgroup::Cgroup(vector<unsigned long> events) : enable_shared_from_this<Cgroup>(), Id(generateCgroupId()), events(events)
{
    //    this->eventsIndex.reserve(100);

    size_t index = 0;
    for (auto it = this->events.begin(); it != this->events.end(); it++)
        this->eventsIndex[*it] = index++;

    this->clonedCopy = this->clone();
    this->version.operator++();
}

Cgroup::Cgroup(const Cgroup &other) : enable_shared_from_this<Cgroup>()
{
    //    this->eventsIndex.reserve(100);
    //    this->original = other.getSharedPtr(); // can't set in Ctor
    this->Id = other.Id;
    this->executionPathId = other.executionPathId;

    this->events = other.events;
    this->eventsIndex = other.eventsIndex;
    this->eventsLeft.store(other.eventsLeft.load());

    this->clonedCopy = atomic_load(&other.clonedCopy);

    this->version.store(other.version.load());
}

shared_ptr<Cgroup> Cgroup::clone()
{
    shared_ptr<Cgroup> cgroup = make_shared<Cgroup>(*this);
    //    cgroup->setOriginal(this->getSharedPtr());
    return cgroup;
}

void Cgroup::set(const Cgroup &other)
{
    this->Id = other.Id;
    this->executionPathId = other.executionPathId;
    this->events = other.events;
    this->eventsIndex = other.eventsIndex;
    this->eventsLeft.store(other.eventsLeft.load());

    atomic_store(&this->clonedCopy, other.clonedCopy);

    // should be 'this.version' because other.version is an old version
    this->version.store(this->version.load() + 1);
}

shared_ptr<Cgroup> Cgroup::getSharedPtr() { return shared_from_this(); }

const vector<unsigned long> &Cgroup::getEvents() const { return this->events; }

void Cgroup::pushNewEvent(unsigned long eventSn)
{
    this->events.push_back(eventSn);
    this->eventsIndex[eventSn] = this->events.size() - 1;
    this->eventsLeft--;

    if (this->updateCounter == 40 || this->eventsLeft.load() == 0)
    {
        shared_ptr<Cgroup> temp = this->clone();
        temp->setClonedCopy(nullptr);

        atomic_store(&this->clonedCopy, temp);

        this->version.operator++();
        this->updateCounter = 1;
    }
    else
        this->updateCounter++;
}

void Cgroup::pushNewEvent(unsigned long eventSn, unsigned int decreasingWeight)
{
    this->events.push_back(eventSn);
    this->eventsIndex[eventSn] = this->events.size() - 1;
    this->eventsLeft -= decreasingWeight;

    if (this->updateCounter == 40 || this->eventsLeft.load() == 0)
    {
        shared_ptr<Cgroup> temp = this->clone();
        temp->setClonedCopy(nullptr);

        atomic_store(&this->clonedCopy, temp);

        this->version.operator++();
        this->updateCounter = 1;
    }
    else
        this->updateCounter++;
}

void Cgroup::removeEventsFromBeginning(unsigned int numberOfevents, unsigned int eventsLeft)
{
    // remove elements from eventsIndex map
    if (numberOfevents > 0)
    {
        this->events.erase(this->events.begin(), this->events.begin() + numberOfevents);
       //remap sn to event index
       this->eventsIndex.clear();
       for (size_t index=0 ; index<this->events.size(); index++)
       {
            this->eventsIndex[ this->events[index]] =index;
       }

    }

    this->eventsLeft = eventsLeft;

    shared_ptr<Cgroup> temp = this->clone();
    temp->setClonedCopy(nullptr);

    atomic_store(&this->clonedCopy, temp);

    this->version.operator++();

    this->updateCounter = 1;
}

bool Cgroup::contain(unsigned long eventSn) const
{
    if (this->eventsIndex.find(eventSn) != this->eventsIndex.end())
        return true;
    else
        return false;
}

bool Cgroup::equal(const shared_ptr<Cgroup> &other) const
{
    // simple comparison
    if ((this->Id == other->Id) && (this->version.load() == other->version.load()))
        return true;
    else
        return false;
}

// const shared_ptr<Cgroup> &Cgroup::getOriginal() const { return original; }

// void Cgroup::setOriginal(const shared_ptr<Cgroup> &original) { this->original = original; }

shared_ptr<Cgroup> Cgroup::getClonedCopy() const { return atomic_load(&clonedCopy); }

void Cgroup::setClonedCopy(const shared_ptr<Cgroup> &value) { atomic_store(&clonedCopy, value); }

unsigned long Cgroup::getId() const { return this->Id; }

unsigned long Cgroup::getExecutionPathId() const { return executionPathId; }

void Cgroup::setExecutionPathId(unsigned long executionPathId) { this->executionPathId = executionPathId; }

unsigned int Cgroup::getVersion() const { return version; }

void Cgroup::setVersion(unsigned int version) { this->version.store(version); }

void Cgroup::incrementVersion() { this->version.operator++(); }

Cgroup::Validation Cgroup::getValidation() const { return this->validation; }

void Cgroup::setValidation(Cgroup::Validation validation) { this->validation = validation; }

unsigned int Cgroup::getEventsLeft() const { return eventsLeft.load(); }

void Cgroup::setEventsLeft(unsigned int eventsLeft) { this->eventsLeft.store(eventsLeft); }

void Cgroup::decrementEventsLeft() { this->eventsLeft--; }

Cgroup::~Cgroup() {}
}
