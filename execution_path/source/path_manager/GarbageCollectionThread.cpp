#include "header/path_manager/GarbageCollectionThread.hpp"

using namespace std;

execution_path::GarbageCollectionThread::GarbageCollectionThread() : garbageQueue(500000) {}

bool execution_path::GarbageCollectionThread::pushToQueue(const shared_ptr<execution_path::AbstractNode> &object)
{
    return this->garbageQueue.push(object);
}

void execution_path::GarbageCollectionThread::main()
{
    running.store(true);
    shared_ptr<AbstractNode> temp;
    list<shared_ptr<AbstractNode>> waitingList;
    bool empty = false;
    while (running.load())
    {
        if (this->garbageQueue.pop(temp))
        {
            //         while (temp.unique() == false);
            if (temp.unique() == false)
            {
                waitingList.push_back(temp);
            }
            else
            {
                temp = nullptr;

            }

            empty = false;
        }
        else
            empty = true;

        if (waitingList.size() == 5 || empty)
            for (auto it = waitingList.begin(); it != waitingList.end();)
            {
                if (it->unique() == false)
                    it++;
                else
                {
                    *it = nullptr;
                    it = waitingList.erase(it);
                }
            }
    }
}

void execution_path::GarbageCollectionThread::stopRunning() { running.store(false); }
