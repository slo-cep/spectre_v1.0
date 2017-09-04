/*
 * LocalEvent.hpp
 *
 * Created on: 10.11.2016
 *      Author: sload
 */

#ifndef LOCALEVENT_HPP
#define LOCALEVENT_HPP

#include "header/util/GlobalTypedef.hpp"

using namespace std;

namespace execution_path
{
class LocalEvent
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    /**
     * @brief contain the event that was fetched from global event stream (Local copy).
     * @param event: the event
     * @param feed: boolean value indicate whether this event should be feeded or not
     */
    LocalEvent(shared_ptr<events::AbstractEvent> event,  bool feed);

    shared_ptr<events::AbstractEvent> getEvent() const;
    void setEvent(const shared_ptr<events::AbstractEvent> &event);

    void setFeed(bool feed);
    bool getFeed() const;

    ~LocalEvent();

private:
    /*
    *  - event
    *  - feeding plan created by Speculator
    */
    shared_ptr<events::AbstractEvent> event;
    bool feed;
};
}

#endif // LOCALEVENT_HPP
