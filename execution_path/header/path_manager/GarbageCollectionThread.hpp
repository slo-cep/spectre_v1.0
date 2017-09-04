#ifndef GARBAGECOLLECTIONTHREAD_HPP
#define GARBAGECOLLECTIONTHREAD_HPP

#include "header/data_structure/AbstractNode.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <queue>
#include <atomic>
#include<list>
#include <iostream>

using namespace std;
namespace execution_path
{
class GarbageCollectionThread
{
public:
    GarbageCollectionThread();
    virtual ~GarbageCollectionThread() = default;

    bool pushToQueue(const shared_ptr<AbstractNode> &object);

    void main();
    void stopRunning();

private:
    boost::lockfree::spsc_queue<std::shared_ptr<AbstractNode>> garbageQueue;
//    std::queue<shared_ptr<AbstractNode>> stillInUseQueue;

    atomic<bool> running = {false};

};

}

#endif // GARBAGECOLLECTIONTHREAD_HPP
