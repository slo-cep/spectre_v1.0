/*
 * LocalEvent.cpp
 *
 * Created on: 10.11.2016
 *      Author: sload
 */

#include "../header/execution_path/LocalEvent.hpp"

using  namespace events;
namespace execution_path
{

LocalEvent::LocalEvent(shared_ptr<AbstractEvent> event, bool feed) : event(event), feed(feed) {}

shared_ptr<AbstractEvent> LocalEvent::getEvent() const { return this->event; }

void LocalEvent::setEvent(const shared_ptr<AbstractEvent> &event) { this->event = event; }

void LocalEvent::setFeed(bool feed) { this->feed = feed; }

bool LocalEvent::getFeed() const { return this->feed; }

LocalEvent::~LocalEvent() {}
}
