#ifndef LOCKFREEQUEUE_H
#define LOCKFREEQUEUE_H

#include <boost/lockfree/queue.hpp>


namespace shared_memory
{

template <typename T>
class LockfreeQueue
{
public:
    /**
     * Constructor to limit the queue size
     * @param size_limit: underlying queue size, zero means ubounded queue
     */
    LockfreeQueue(size_t size_limit) : queue(size_limit){}

    /**
     * @brief ~LockfreeQueue Destructor
     */
    virtual ~LockfreeQueue()
    {
        Container *c;
        while (queue.pop(c))
        {
            delete c;
        }
    }

    /**
     * push an event to the head of the stream
     * @param event: the string representation of an event
     * 			which should be parsed in the Splitter to a real event
     * @return true: on success else false
     */
    bool push(T const & t)
    {
        auto newElement = new Container(t);
        return queue.push(newElement);
    }

    /**
     * if pop operation is successful, object will be copied to ret.
     * @return true, if the pop operation is successful, false if queue was empty
     */
    bool pop(T & ret)
    {
        Container *c;
        if (queue.pop(c))
        {
            ret = c->content;
            delete c;
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * check if empty
     * @return true or false
     */
    bool isEmpty(void) const{return queue.empty();}


private:
    class Container
    {
    public:
        Container(T t) : content(t){}
        T content;
    };

    boost::lockfree::queue<Container*> queue;
};
}

#endif // LOCKFREEQUEUE_H
