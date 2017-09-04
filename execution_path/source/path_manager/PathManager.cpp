/*
 * PathManager.cpp
 *
 * Created on: 05.12.2016
 *      Author: sload
 */

#include "../../header/path_manager/PathManager.hpp"
#include "../../header/execution_path/Checkpoint.hpp"
#include "../../header/execution_path/ExecutionPath.hpp"

using namespace selection;
using namespace util;
using namespace profiler;
namespace execution_path
{

PathManager::PathManager(vector<shared_ptr<WorkerThread>> workerThreads,
                         shared_ptr<ExecutionPathFactory> executionPathFactory, shared_ptr<MarkovMatrix> markovMatrix,
                         unsigned long initialSelectionSize, unsigned int treeSize,
                         GarbageCollectionThread *garbageCollectionThread, Measurements *measurements)
    : workerThreads(workerThreads), executionPathFactory(executionPathFactory), markovMatrix(markovMatrix),
      garbageCollectionThread(garbageCollectionThread), selectionSize(initialSelectionSize), treeSize(treeSize),
      measurements(measurements)
{
}

void PathManager::main()
{
    unsigned long reschedulingCounter = 0;
    unsigned long treeSize=0;

    unsigned long startTime = Helper::currentTimeMillis();

    while (!this->terminate.load() || (this->root != nullptr) || (this->newSelections.isEmpty() == false))
    {
        if (this->root != nullptr)
            this->assignExecutionPathToWorkerThread();
        else
            this->topKExecutionPaths.clear();

        this->checkNewSelections();

        this->processNewCgroups();
        this->processDeletedCgroups();
        this->processUpdatedCgroups();

        // master finished
        if (this->masterExecutionPathFinished.load())
        {
            this->masterExecutionPathFinished.store(false); // reset
            // check also cgroups
            this->processNewCgroups();
            this->processDeletedCgroups();
            this->processUpdatedCgroups();

            ExecutionPathNode *currentMasterExPN = this->executionPathExecutionPathNodeMap[this->masterExecutionPathId];

            auto selectionId = currentMasterExPN->getExecutionPath()->getAbstractSelection()->getId();
            cout << "PathManager: Master, Selection ID: " << selectionId
                 << ", execution path Id: " << this->masterExecutionPathId << endl;

//            if(selectionId==4)
//            {
//                cout<<"selection Id 4: check it"<<endl;
//            }
            shared_ptr<ExecutionPathNode> newMasterExPN;
            if (currentMasterExPN->getChildNode() != nullptr)
            {
                if (currentMasterExPN->getChildNode()->isBranchNode())
                {
                    newMasterExPN = this->findNextRoot(currentMasterExPN->getChildNode());
                }
                else // the child is execution path node
                {
                    newMasterExPN = static_pointer_cast<ExecutionPathNode>(currentMasterExPN->getChildNode());
                }
            }
            else
                newMasterExPN = nullptr;

            /*
             * After updating and/or deleting cgroups, the next child should be the correct execution path
             * So make it as master
             */

            size_t index = this->executionPathWorkThreadMap[this->masterExecutionPathId];
            this->workerThreads[index]->receiveExecutionPath(nullptr);
            this->executionPathExecutionPathNodeMap.erase(this->masterExecutionPathId);
            this->executionPathWorkThreadMap.erase(this->masterExecutionPathId);
            this->markovMatrix->executionPathFinished(
                currentMasterExPN->getExecutionPath()->getAbstractSelection()->getLastEventSn(), true);
            this->treeDepth--;

            // Make the selection as ready to be removed. So Splitter can remove it events!
            currentMasterExPN->getExecutionPath()->getAbstractSelection()->setReadyToBeRemoved(true);

            if (newMasterExPN != nullptr)
            {
                newMasterExPN->setParentNode(nullptr);

                ExecutionPath *newMasterExP = newMasterExPN->getExecutionPath().get();
                newMasterExP->setMaster(true);
                this->masterExecutionPathId = newMasterExP->getId();

                /*
                 *move root to the garbage collector, so path manager doesn't need to spend
                 * time on deleting the old root
                 */
                this->garbageCollectionThread->pushToQueue(this->root);

                this->root = newMasterExPN;

                //                cout << "PathManager: new master: " << newMasterExP->getId() << endl;
            }
            else
            {
                //                if (this->executionPathExecutionPathNodeMap.size() != 0)
                //                {
                //                    cout << "Path manager: there are still(" <<
                //                    this->executionPathExecutionPathNodeMap.size()
                //                         << ") but can't schedule" << endl;
                //                }
                this->root = nullptr;
            }
        }

        reschedulingCounter++;
        if(treeSize< this->executionPathExecutionPathNodeMap.size())
            treeSize=this->executionPathExecutionPathNodeMap.size();

    }
    ofstream treeSizeFile;
    treeSizeFile.open("./tree_size", ios::out);
    treeSizeFile<< treeSize;
    treeSizeFile.close();

    unsigned long endTime = Helper::currentTimeMillis();

    this->measurements->getPathManagerReschedulingFrequency()->set(startTime, endTime, reschedulingCounter);

    cout << "Path Manager counter: " << reschedulingCounter << endl;
    cout << "Path Manager cgroups New counter: " << this->cgroupNewCounter << endl;
    cout << "Path Manager cgroups delete counter: " << this->cgroupDeleteCounter << endl;
    cout << "Path Manager cgroups update counter: " << this->cgroupUpdateCounter << endl;

    // terminate all worker threads
    for (auto it = this->workerThreads.begin(); it != this->workerThreads.end(); it++)
        it->get()->setTerminate();
}

shared_ptr<ExecutionPathNode> PathManager::findNextRoot(const shared_ptr<AbstractNode> &startNode)
{
    // No execution path nodes in the tree
    if (startNode == nullptr)
        return nullptr;

    if (startNode->isBranchNode())
    {
        shared_ptr<BranchNode> tempBN = static_pointer_cast<BranchNode>(startNode);

        // for some reason the validation was late and the master couldn't detect it
        if (tempBN->getCgroup()->getValidation() == Cgroup::Validation::VALID)
        {
            this->bufferedRootCgroups.push_back(tempBN->getCgroup());

            // call recursively using, cgroup is valid, so take right child that used this cgroup
            return this->findNextRoot(tempBN->getRightChildNode());
        }

        else if (tempBN->getCgroup()->getValidation() == Cgroup::Validation::INVALID
                 || tempBN->getCgroup()->getValidation() == Cgroup::Validation::DELETED)
        {
            // cgroup is Invalid or deleted, so take left child that didn't use this cgroup
            return findNextRoot(tempBN->getLeftChildNode());
        }
        else // if (tempBN->getCgroup()->getValidation() == Cgroup::Validation::NONE)

        {
            cout << "Path Manager: found a cgroup with NONE validation, cgroup: " << tempBN->getCgroup()->getId()
                 << endl;
            exit(-1);

            /*
             * no information about this cgroup. it is impossible, so we choose both children
             *  and try to find first execution path node
             */
            auto result = findNextRoot(tempBN->getLeftChildNode());
            if (result != nullptr)
                return result;

            result = findNextRoot(tempBN->getRightChildNode());

            return result;
        }
    }
    else // found the execution path
    {
        return static_pointer_cast<ExecutionPathNode>(startNode);
    }
}

void PathManager::receiveNewSelection(const shared_ptr<selection::AbstractSelection> &abstractSelection)
{
    this->newSelections.push(abstractSelection);
}

void PathManager::checkNewSelections()
{
    // don't add too much selection to not overload the path manager
    if (this->treeDepth > this->treeSize)
        return;

    /* get all Selections from newSelections queue and
     * create new execution path for each Selection. Then create new execution path
     *  node and call addNewExecutionPathToTreeNode
     */
    shared_ptr<AbstractSelection> newSelection;
    while (this->newSelections.pop(newSelection))
    {
        shared_ptr<ExecutionPath> executionPath = this->executionPathFactory->createNewExecutionPath(newSelection);

        // check cgroups from earlier parents
        this->checkValidBufferedRootCgroups(newSelection->getStartPosition()->get()->getSn());
        unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> onPathCgroups;
        for (auto cgroup : this->bufferedRootCgroups)
        {
            onPathCgroups[cgroup->getId()] = make_pair(cgroup, 0);
        }

        shared_ptr<ExecutionPathNode> executionPathNode = make_shared<ExecutionPathNode>(executionPath);

        if (root == nullptr) // empty tree
        {
            executionPath->setCgroupsFromParent(onPathCgroups);
            // create the initial check point
            //            executionPath->createCheckpoint(nullptr);
            //            executionPath->setInitialCheckpoint(executionPath->getCheckpoint(0));

            root = executionPathNode;
            executionPath->setMaster(true);
            this->masterExecutionPathId = executionPath->getId();

            // map execution path with execution path node
            this->executionPathExecutionPathNodeMap[executionPath->getId()] = executionPathNode.get();

            this->treeDepth = 1;
        }
        else
        {
            this->treeDepth++;
            this->addExecutionPathNodeToTreeNode(this->root.get(), executionPathNode, onPathCgroups);
        }

        // don't add too much selection to not overload the path manager
        //        if (this->executionPathExecutionPathNodeMap.size() >= this->treeDepth)
        if (this->treeDepth > this->treeSize)
            return;
    }
}

void PathManager::checkValidBufferedRootCgroups(unsigned long selectionStartEventSn)
{
    for (auto it = this->bufferedRootCgroups.begin(); it != this->bufferedRootCgroups.end();)
    {
        size_t size = it->get()->getEvents().size();
        // there is no overlap, delete the buffered cgroup
        if (size != 0 && it->get()->getEvents()[size - 1] < selectionStartEventSn)
        {
            it = this->bufferedRootCgroups.erase(it);
        }
        else // no need to check next cgroups, because mostly they are ordered and checking costs time
            return;
    }
}

void PathManager::addExecutionPathNodeToTreeNode(
    AbstractNode *startNode, const shared_ptr<ExecutionPathNode> &executionPathNode,
    unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &cgroups)
{
    /*
     * Then traverse tree node and add the new execution path to its leaves
     * ** deep first**
     * - collect all cgroups from root to the leaf and add them to right execution path child of a branch node
     *      * collect only on right children of branch nodes
     */

    if (startNode->isBranchNode())
    {
        BranchNode *temp = static_cast<BranchNode *>(startNode);

        // left child
        if (temp->getLeftChildNode() != nullptr)
            addExecutionPathNodeToTreeNode(temp->getLeftChildNode().get(), executionPathNode, cgroups); // recursively
        // leaf node=> add the execution path to it
        else
        {
            shared_ptr<ExecutionPathNode> tempExPN = static_pointer_cast<ExecutionPathNode>(executionPathNode->clone());

            // clone execution path inside the execution path node
            auto executionPath = tempExPN->getExecutionPath()->clone();
            executionPath->setCgroupsFromParent(cgroups);
            // create the initial check point
            //            executionPath->createCheckpoint(nullptr);
            //            executionPath->setInitialCheckpoint(executionPath->getCheckpoint(0));

            // add the execution path to the execution path node
            tempExPN->setExecutionPath(executionPath);

            // set parent node
            tempExPN->setParentNode(temp);

            temp->setLeftChildNode(tempExPN);

            // map execution path with execution path node
            this->executionPathExecutionPathNodeMap[executionPath->getId()] = tempExPN.get();
        }

        // right child
        if (temp->getRightChildNode() != nullptr)
        {
            /*
             *clone cgroup to avoid data race on the cgroup (write by parent, read by children.
             * One copy for all children.
             * Whenever the cgroup changes, each children performs a local copy!
             */
            // lock because the parent can change it in the meanwhile
            //            temp->getCgroup()->readLock();
            //            cgroups[temp->getCgroup()->getId()] = temp->getCgroup()->clone(); // collect cgroups
            //            temp->getCgroup()->readUnlock();

            cgroups[temp->getCgroup()->getId()] = make_pair(temp->getCgroup(), 0);

            addExecutionPathNodeToTreeNode(temp->getRightChildNode().get(), executionPathNode, cgroups); // recursively

            cgroups.erase(temp->getCgroup()->getId()); // remove cgroups
        }
        else // leaf node=> add the execution path to it
        {
            // add current cgroup (it is the right child)
            //            temp->getCgroup()->readLock();
            //            auto currentCgroup = temp->getCgroup()->clone(); // collect cgroups
            //            temp->getCgroup()->readUnlock();
            auto currentCgroup = temp->getCgroup();

            shared_ptr<ExecutionPathNode> tempExPN = static_pointer_cast<ExecutionPathNode>(executionPathNode->clone());

            // clone execution path inside the execution path node
            auto executionPath = tempExPN->getExecutionPath()->clone();
            executionPath->setCgroupsFromParent(cgroups);
            executionPath->addCgroupFromParent(currentCgroup, 0);
            // create the initial check point
            //            executionPath->createCheckpoint(nullptr);
            //            executionPath->setInitialCheckpoint(executionPath->getCheckpoint(0));

            // add the execution path to the execution path node
            tempExPN->setExecutionPath(executionPath);

            // set parent node
            tempExPN->setParentNode(temp);

            temp->setRightChildNode(tempExPN);

            // map execution path with execution path node
            this->executionPathExecutionPathNodeMap[tempExPN->getExecutionPath()->getId()] = tempExPN.get();
        }
    }
    else // it is execution path node
    {
        ExecutionPathNode *temp = static_cast<ExecutionPathNode *>(startNode);

        // either there is child execution path node or child branch node but not both together (or not both)!
        if (temp->getChildNode() != nullptr)
            addExecutionPathNodeToTreeNode(temp->getChildNode().get(), executionPathNode, cgroups); // recursively
        else // if leaf node=> add the execution path to it
        {
            shared_ptr<ExecutionPathNode> tempExPN = static_pointer_cast<ExecutionPathNode>(executionPathNode->clone());

            // clone execution path inside the execution path node
            auto executionPath = tempExPN->getExecutionPath()->clone();
            executionPath->setCgroupsFromParent(cgroups);
            // create the initial check point
            //            executionPath->createCheckpoint(nullptr);
            //            executionPath->setInitialCheckpoint(executionPath->getCheckpoint(0));

            // add the execution path to the execution path node
            tempExPN->setExecutionPath(executionPath);

            // set parent node
            tempExPN->setParentNode(temp);

            temp->setChildNode(tempExPN);

            // map execution path with execution path node
            this->executionPathExecutionPathNodeMap[tempExPN->getExecutionPath()->getId()] = tempExPN.get();
        }
    }
}

void PathManager::assignExecutionPathToWorkerThread()
{
    size_t k = workerThreads.size();
    vector<bool> freeIndex(k, true); // check if working
    vector<shared_ptr<ExecutionPath>> toBeScheduled;
    //    vector<shared_ptr<WorkerThread>> freeWorkerThreads = this->workerThreads;
    this->topKExecutionPaths = this->getTopKExecutionPath(root, k);
    std::unordered_map<unsigned long, unsigned long> executionPathWorkThreadMapNext;

    /*
     * check if the execution path is already scheduled on any worker thread
     * if yes: don't assign it again because (worker logic) is designed to work on
     * the scheduled execution path till finishes it unless there is a new execution path
     */
    for (size_t i = 0; i < this->topKExecutionPaths.size(); ++i) // Loop executionPaths
    {
        auto pos = executionPathWorkThreadMap.find(topKExecutionPaths[i]->get()->getId());

        // If Execution path with that Id is not scheduled yet -> add it to the toBeScheduled List
        if (pos == executionPathWorkThreadMap.end())
        {
            toBeScheduled.push_back(*topKExecutionPaths[i]);
        }
        else // else remove the thread executing it from the freeIndex list
        {
            freeIndex[pos->second] = false;
            executionPathWorkThreadMapNext.insert(make_pair(topKExecutionPaths[i]->get()->getId(), pos->second));
        }
    }
    size_t j = 0;
    for (size_t i = 0; i < toBeScheduled.size(); ++i) // Loop toBeScheduled
    {
        while (freeIndex[j] == false)
        {
            ++j;
        }
        this->workerThreads[j]->receiveExecutionPath(toBeScheduled[i]);
        executionPathWorkThreadMapNext.insert(make_pair(toBeScheduled[i]->getId(), j));
        freeIndex[j] = false;
        ++j;
    }

    for (size_t j = 0; j < k; j++)
    {
        if (freeIndex[j])
            this->workerThreads[j]->receiveExecutionPath(nullptr);
    }

    // Update Map
    executionPathWorkThreadMap = executionPathWorkThreadMapNext;
}

vector<shared_ptr<ExecutionPath> *> PathManager::getTopKExecutionPath(shared_ptr<ExecutionPathNode> root, size_t k)
{
    vector<shared_ptr<ExecutionPath> *> A;
    A.reserve(k);
    priority_queue<AbstractNode *, vector<AbstractNode *>, LessThanByP> B;
    root->setProbability(1.0f);
    B.push(root.get());
    AbstractNode *currentNode;
    float probability;
    while (A.size() < k && !B.empty())
    {
        currentNode = B.top();
        B.pop();
        if (currentNode->isExecutionPathNode()) // if current Node is Execution Path
        {
            ExecutionPathNode *currentNodeEx = static_cast<ExecutionPathNode *>(currentNode);

            if (currentNodeEx->getChildNode())
            {
                currentNodeEx->getChildNode()->setProbability(currentNodeEx->getProbability());
                B.push(currentNodeEx->getChildNode().get());
            }
            A.push_back(&currentNodeEx->getExecutionPath());

        }
        else
        {
            BranchNode *currentNodeBn = static_cast<BranchNode *>(currentNode);
            // get execution path that generated this cgroup to compute windowLeft
            auto tempExP = this->executionPathExecutionPathNodeMap[currentNodeBn->getCgroup()->getExecutionPathId()]
                               ->getExecutionPath()
                               .get();

            unsigned long windowLeft = tempExP->getEventLeftInSelection(this->selectionSize.load()); // TODO
            if (windowLeft == 0) // If the window is still open at least one more event is expected
            {
                windowLeft = 1;
            }
            probability = static_cast<float>(
                markovMatrix->getProbability(windowLeft, currentNodeBn->getCgroup()->getEventsLeft()));

            if (probability < 0)
            {
                probability = 0.0f;
            }

            if (currentNodeBn->getLeftChildNode())
            {
                currentNodeBn->getLeftChildNode()->setProbability(currentNodeBn->getProbability() * (1 - probability));
                B.push(currentNodeBn->getLeftChildNode().get());
            }
            if (currentNodeBn->getRightChildNode())
            {
                currentNodeBn->getRightChildNode()->setProbability(currentNodeBn->getProbability() * probability);
                B.push(currentNodeBn->getRightChildNode().get());
            }
        }
    }
    return A;
}

void PathManager::receiveCgroup(const shared_ptr<Cgroup> &cgroup, Cgroup::Status status)
{

    switch (status)
    {
    case Cgroup::Status::NEW:
        this->newCgroups.push(cgroup);
        break;
    case Cgroup::Status::UPDATE:
        this->updatedCgroups.push(cgroup);
        break;
    case Cgroup::Status::DELETE:
        this->deletedCgroups.push(cgroup);
        break;
    }
}

void PathManager::processNewCgroups()
{
    shared_ptr<Cgroup> cgroup;
    while (this->newCgroups.pop(cgroup))
    {
        this->cgroupNewCounter++;

        // Check if the execution path already has been deleted from the tree
        if (this->executionPathExecutionPathNodeMap.count(cgroup->getExecutionPathId()) == 0)
            continue;

        //        if (this->workerThreads.size() == 16)
        //        {
        //            size_t index = 0;
        //            unsigned long CgExecutionPathId = cgroup->getExecutionPathId();
        //            for (; index < 8; index++)
        //                if (CgExecutionPathId == this->topKExecutionPaths[index]->getId())
        //                    break;

        //            if (index == 8)
        //            {
        //                this->newCgroupsList.push_back(cgroup);
        //                continue;
        //            }
        //        }

        addNewCgroup(cgroup);
    }
    for (auto it = this->newCgroupsList.begin(); it != this->newCgroupsList.end();)
    {

        // Check if the execution path already has been deleted from the tree
        if (this->executionPathExecutionPathNodeMap.count(it->get()->getExecutionPathId()) == 0)
        {
            it = this->newCgroupsList.erase(it);
            continue;
        }

        //        if (this->workerThreads.size() == 16)
        //        {
        //            size_t index = 0;
        //            unsigned long CgExecutionPathId = it->get()->getExecutionPathId();
        //            for (; index < 8; index++)
        //                if (CgExecutionPathId == this->topKExecutionPaths[index]->getId())
        //                    break;

        //            if (index == 8)
        //            {
        //                it++;
        //                continue;
        //            }
        //        }

        addNewCgroup(*it);
        it = this->newCgroupsList.erase(it);
    }
}

void PathManager::addNewCgroup(const shared_ptr<Cgroup> &cgroup)
{
    // clean the tree for the execution path that generated the new cgroup
    /*
     *To prevent adding a new cgroup which is already deleted by parent execution path:
     * check if the cgroup is deleted in the meanwhile. If not:
     *Also to prevent adding the cgroup as a child of a deleted cgroup from same execution path,
     *  we first check if there is any deleted cgroup from this path and delete them.
     *We can delete all cgroup in deletedCgroupMap, but to reduce the delay for the new cgroup
     * check and delete only the cgroup from this execution path
     */
    this->readDeletedCgroups();
    // If the new cgroup already was deleted, erase it from deletedCgroupMap and continue to next cgroup
    if (this->deletedCgroupsMap.count(cgroup->getId()) != 0)
    {
        this->deletedCgroupsMap.erase(cgroup->getId());
        return;
    }
    else // The new cgroup is not deleted, check if any other cgroup from the same execution path is deleted
    {
        for (auto it = this->deletedCgroupsMap.begin(); it != this->deletedCgroupsMap.end();)
        {
            if (it->second->getExecutionPathId() == cgroup->getExecutionPathId()) // same execution path
            {
                bool result = this->deleteCgroup(it->second); // delete the cgroup
                if (result == false)                          // not found in the tree, so not deleted
                    it++;
                else
                    it = this->deletedCgroupsMap.erase(it);
            }
            else
                it++;
        }
    }

    // get the execution path that has generated this cgroup
    ExecutionPathNode *executionPathNode = executionPathExecutionPathNodeMap[cgroup->getExecutionPathId()];

    shared_ptr<BranchNode> newBranchNode = make_shared<BranchNode>(cgroup);

    if (executionPathNode->getChildNode() == nullptr)
    {
        // set parent node
        newBranchNode->setParentNode(executionPathNode);

        executionPathNode->setChildNode(newBranchNode);

        //            cout << "PathManager: execution path: " << cgroup->getExecutionPathId() << ", cgroup:" <<
        //            cgroup->getId()
        //                 << " added as a leaf node to the execution path node itself" << endl;
    }
    else
    {
        // add the new cgroup
        branch(executionPathNode, executionPathNode->getChildNode(), newBranchNode);
    }
}

void PathManager::branch(AbstractNode *parentNode, shared_ptr<AbstractNode> &startNode,
                         const shared_ptr<BranchNode> &newBranchNode)
{
    if (startNode->isBranchNode())
    {
        BranchNode *temp = static_cast<BranchNode *>(startNode.get());

        // left child

        if (temp->getLeftChildNode() != nullptr)
        {
            branch(temp, temp->getLeftChildNode(), newBranchNode);
        }
        else // it is  leaf node, so add the newBranchNode to its children
        {
            auto tempBN = newBranchNode->clone();
            // set the parent node
            tempBN->setParentNode(temp);

            temp->setLeftChildNode(tempBN);

            //            auto cgroup = (static_cast<BranchNode *>(tempBN.get()))->getCgroup();
            //            cout << "PathManager: execution path: " << cgroup->getExecutionPathId()
            //                 << ", New cgroup:" << cgroup->getId() << " added as a leaf node to left child of a branch
            //                 node"
            //                 << endl;
        }

        // right child
        if (temp->getRightChildNode() != nullptr)
        {
            branch(temp, temp->getRightChildNode(), newBranchNode);
        }
        else // it is leaf node, so add the newBranchNode to its children
        {
            auto tempBN = newBranchNode->clone();
            // set parent node
            tempBN->setParentNode(temp);

            temp->setRightChildNode(tempBN);

            //            auto cgroup = (static_cast<BranchNode *>(tempBN.get()))->getCgroup();
            //            cout << "PathManager: execution path: " << cgroup->getExecutionPathId()
            //                 << ", New cgroup:" << cgroup->getId() << " added as a leaf node to right child of a
            //                 branch node "
            //                 << endl;
        }
    }
    else // execution path node. Above this execution path node, we insert newBranchNode
    {
        shared_ptr<BranchNode> tempBN = static_pointer_cast<BranchNode>(newBranchNode->clone());
        // insert a copy of newBranchNode. (left child, doesn't change any thing)
        tempBN->setLeftChildNode(startNode);

        // set the parent node
        tempBN->setParentNode(parentNode);

        startNode = tempBN;

        //        auto cgroup = tempBN->getCgroup();
        //        cout << "PathManager: execution path: " << cgroup->getExecutionPathId() << ", New cgroup:" <<
        //        cgroup->getId()
        //             << " added as a parent node for the next (child) execution path node" << endl;

        /*
         * clone the whole sub tree starting from this execution path node
         * and add it as a right child of newBranchNode (also feed all cloned execution path with the new
         * cgroup)
         */
        //        shared_ptr<AbstractNode> startCloneNode = tempBN->getLeftChildNode();
        //        cloneSubTree(tempBN.get(), startCloneNode, tempBN->getCgroup());
        //        tempBN->setRightChildNode(startCloneNode);

        //-----------------------------------
        /*
         *clone cgroup to avoid data race on the cgroup (write by parent, read by children.
         * One copy for all children.
         * Whenever the cgroup changes, each children performs a local copy!
         */
        // lock because the parent can change it in the meanwhile
        //        tempBN->getCgroup()->readLock();
        //        shared_ptr<Cgroup> cgroupClone = tempBN->getCgroup()->clone();
        //        tempBN->getCgroup()->readUnlock();
        shared_ptr<Cgroup> cgroupClone = tempBN->getCgroup();

        shared_ptr<AbstractNode> rightStartCloneNode = NULL;
        //        AbstractNode *leftStartNode = tempBN->getLeftChildNode().get();
        ExecutionPathNode *leftStartNode = static_cast<ExecutionPathNode *>(tempBN->getLeftChildNode().get());

        // take a copy of cgroups, take from the initial check point to avoid data race;
        unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> cgroups;
        //        cgroups =
        //        leftStartNode->getExecutionPath()->getInitialCheckpoint()->getExecutionPath()->getCgroupsFromParent();
        cgroups = leftStartNode->getExecutionPath()->getCgroupsFromParent();

        // filter  all finished and not overlapped cgroups!
        for (auto it = cgroups.begin(); it != cgroups.end();)
        {
            // the execution path is already finished (old master)
            if (it->second.first->getExecutionPathId() < this->masterExecutionPathId)
            {
                //                size_t cgroupSize = it->second->getOriginal()->getEvents().size();
                //                unsigned long lastEventSn = it->second.first.ge->getOriginal()->getEvents()[cgroupSize
                //                - 1]->getSn();
                shared_ptr<Cgroup> tempCgroup = it->second.first->getClonedCopy();
                size_t cgroupSize = tempCgroup->getEvents().size();
                unsigned long lastEventSn = tempCgroup->getEvents()[cgroupSize - 1];

                // check if it doesn't overlap
                if (lastEventSn
                    < leftStartNode->getExecutionPath()->getAbstractSelection()->getStartPosition()->get()->getSn())
                    it = cgroups.erase(it);
                else
                {
                    it->second.second = 0; // reset version number, so new execution path will for sure copy clonedCopy
                    it++;
                }
            }
            else
            {
                it->second.second = 0; // reset version number, so new execution path will for sure copy clonedCopy
                it++;
            }
        }

        cgroups[cgroupClone->getId()] = make_pair(cgroupClone, 0);

        cloneSubTree(tempBN.get(), rightStartCloneNode, leftStartNode, cgroups);
        tempBN->setRightChildNode(rightStartCloneNode);
    }
}

void PathManager::cloneSubTree(AbstractNode *parentNode, shared_ptr<AbstractNode> &rightStartCloneNode,
                               AbstractNode *startNode,
                               const unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &cgroups)
{
    // traverse over all node starting from startNode and replicate them to construct a new branch
    /*
     * Clone only on execution path per a window (Selection)
     */

    if (startNode->isBranchNode())
    {
        BranchNode *tempBN = static_cast<BranchNode *>(startNode);
        /*
         * traversing left child is enough.
         * Note: we take only one initial copy of only one execution path per a Selection
         */
        if (tempBN->getLeftChildNode() != nullptr)
            cloneSubTree(parentNode, rightStartCloneNode, tempBN->getLeftChildNode().get(), cgroups);
    }

    else // Execution path node
    {
        ExecutionPathNode *tempStartNode = static_cast<ExecutionPathNode *>(startNode);
        // get the initial check point
        //        shared_ptr<Checkpoint> checkpoint = tempStartNode->getExecutionPath()->getInitialCheckpoint();

        // clone the check point
        //        shared_ptr<ExecutionPath> executionPath = checkpoint->getExecutionPath()->clone();
        auto selection = tempStartNode->getExecutionPath()->getAbstractSelection()->clone();
        selection->setCurrentPosition(selection->getStartPosition());
        shared_ptr<ExecutionPath> executionPath = this->executionPathFactory->createNewExecutionPath(selection);
        // add the cgroups to the newly created execution path
        executionPath->setCgroupsFromParent(cgroups);
        // create the initial check point
        //        executionPath->createCheckpoint(nullptr);
        //        executionPath->setInitialCheckpoint(executionPath->getCheckpoint(0));

        // create a new execution path node
        shared_ptr<ExecutionPathNode> newExPN = make_shared<ExecutionPathNode>(executionPath);

        // set the parent node
        newExPN->setParentNode(parentNode);

        // assign the new execution path to rightStartCloneNode (cloning)
        rightStartCloneNode = newExPN;

        this->executionPathExecutionPathNodeMap[executionPath->getId()] = newExPN.get();

        //        cout << "PathManager: execution path: , New cgroup:  added as a parent node for the next (child)
        //        execution "
        //                "path node (cloneSubTree)"
        //             << endl;
        //        cout << "PathManager: execution path: " << cgroup->getExecutionPathId() << ", New cgroup:" <<
        //        cgroup->getId()
        //             << " added as a parent node for the next (child) execution path node (cloneSubTree)" << endl;

        // call recursively for leftStartNode's child
        if (tempStartNode->getChildNode().get() != nullptr)
            cloneSubTree(newExPN.get(), newExPN->getChildNode(), tempStartNode->getChildNode().get(), cgroups);
    }
}

void PathManager::processUpdatedCgroups()
{
    vector<shared_ptr<Cgroup>> unfoundedCgroups;

    shared_ptr<Cgroup> cgroup;
    while (this->updatedCgroups.pop(cgroup))
    {

        // Check if the execution path already has been deleted from the tree
        if (this->executionPathExecutionPathNodeMap.count(cgroup->getExecutionPathId()) == 0)
            continue;

        // get the execution path that has generated this cgroup
        ExecutionPathNode *executionPathNode = executionPathExecutionPathNodeMap.at(cgroup->getExecutionPathId());

        bool result = this->deleteBranch(executionPathNode->getChildNode(), cgroup);

        // cgroup not found, so return it back to the queue till the cgroup is fetched from newCgroups queue
        if (result == false)
            unfoundedCgroups.push_back(cgroup);

        cgroupUpdateCounter++;
    }

    for (auto it = unfoundedCgroups.begin(); it != unfoundedCgroups.end(); it++)
        this->updatedCgroups.push(*it);
}

void PathManager::readDeletedCgroups()
{
    shared_ptr<Cgroup> cgroup;
    while (this->deletedCgroups.pop(cgroup))
    {
        this->deletedCgroupsMap[cgroup->getId()] = cgroup;
        this->cgroupDeleteCounter++;
    }
}

void PathManager::processDeletedCgroups()
{
    vector<shared_ptr<Cgroup>> unfoundedCgroups;
    shared_ptr<Cgroup> cgroup;
    while (this->deletedCgroups.pop(cgroup))
    {
        //        // Check if the execution path already has been deleted from the tree
        //        if (this->executionPathExecutionPathNodeMap.count(cgroup->getExecutionPathId()) == 0)
        //            continue;

        //        // get the execution path that has generated this cgroup
        //        ExecutionPathNode *executionPathNode =
        //        executionPathExecutionPathNodeMap.at(cgroup->getExecutionPathId());

        //        bool result = this->deleteBranch(executionPathNode->getChildNode().get(), cgroup);

        bool result = this->deleteCgroup(cgroup);
        // cgroup not found, so return it back to the queue till the cgroup is fetched from newCgroups queue
        // cgroup not found, so return it back to the map till the cgroup is fetched from newCgroups queue
        if (result == false)
            unfoundedCgroups.push_back(cgroup);
        cgroupDeleteCounter++;
    }

    for (auto it = this->deletedCgroupsMap.begin(); it != this->deletedCgroupsMap.end();)
    {
        bool result = this->deleteCgroup(it->second);
        // cgroup not found, so don't erase cgroup, till the cgroup is fetched from newCgroups queue
        if (result == false)
            it++;
        else
            it = this->deletedCgroupsMap.erase(it);
    }

    // Insert undeleted cgroups (which are fetched from the queue) to the map
    for (auto it = unfoundedCgroups.begin(); it != unfoundedCgroups.end(); it++)
        //        this->deletedCgroups.push(*it);
        this->deletedCgroupsMap[it->get()->getId()] = *it;
}

bool PathManager::deleteCgroup(const shared_ptr<Cgroup> &cgroup)
{
    // Check if the execution path already has been deleted from the tree
    if (this->executionPathExecutionPathNodeMap.count(cgroup->getExecutionPathId()) == 0)
        return true;

    // get the execution path that has generated this cgroup
    ExecutionPathNode *executionPathNode = executionPathExecutionPathNodeMap.at(cgroup->getExecutionPathId());

    bool result = this->deleteBranch(executionPathNode->getChildNode(), cgroup);

    return result;
}

bool PathManager::deleteBranch(const shared_ptr<AbstractNode> &startNode, const shared_ptr<Cgroup> &cgroup)
{
    if (startNode == nullptr)
    {
        // cgroup not found, so return false  it back to the queue till the cgroup is fetched from newCgroups queue
        return false;
    }
    if (startNode->isBranchNode())
    {
        shared_ptr<BranchNode> tempBN = static_pointer_cast<BranchNode>(startNode);
        if (tempBN->getCgroup()->getId() == cgroup->getId()) // we reached the deleted/ validated cgroup
        {
            // remove all execution paths that are children of this path from  ExecutionPathExecutionPathNodeMap

            AbstractNode *toBeDeletedChild = nullptr;
            shared_ptr<AbstractNode> toBeKeptChild = nullptr;

            // Valid, delete left child of the branch node and mark the branch node as valid
            if (cgroup->getValidation() == Cgroup::Validation::VALID)
            {
                toBeDeletedChild = tempBN->getLeftChildNode().get(); // delete right child
                toBeKeptChild = tempBN->getRightChildNode();

                // add the cgroup to the buffer so it can be used by later coming Selections
                this->bufferedRootCgroups.push_back(cgroup);
            }

            // if the cgroup has invalid/deleted status, the right child and the branch node itself will be deleted
            else if (cgroup->getValidation() == Cgroup::Validation::INVALID
                     || cgroup->getValidation() == Cgroup::Validation::DELETED)
            {

                toBeDeletedChild = tempBN->getRightChildNode().get(); // delete right child
                toBeKeptChild = tempBN->getLeftChildNode();
            }

            else // NONE, impossible status
            {
                cout << "Path Manager, deletebranch() has bad cgroup: " << cgroup->getId()
                     << " from the execution path: " << cgroup->getExecutionPathId() << endl;

                exit(-1);
            }

            // Start to delete!

            if (toBeDeletedChild != nullptr)
            {
                this->cleanExecutionPathExecutionPathMap(toBeDeletedChild); // recursive function
            }

            // delete the link to this branch node (i.e. that contains the deleted cgroup)
            //(delete the branch node)
            AbstractNode *parentNode = tempBN->getParentNode();
            if (toBeKeptChild != nullptr)
                toBeKeptChild->setParentNode(parentNode);

            if (parentNode->isBranchNode())
            {
                BranchNode *tempBNParent = static_cast<BranchNode *>(parentNode);
                if (tempBNParent->getLeftChildNode().get() == tempBN.get()) // tempBN is left child
                {
                    tempBNParent->setLeftChildNode(toBeKeptChild);

                    //                    cout << "PathManager: execution path: " << cgroup->getExecutionPathId()
                    //                         << ", Delete cgroup:" << cgroup->getId() << ", parent is branch node
                    //                         (left child)" << endl;
                }
                else // tempBN is right child
                {
                    tempBNParent->setRightChildNode(toBeKeptChild);

                    //                    cout << "PathManager: execution path: " << cgroup->getExecutionPathId()
                    //                         << ", Delete cgroup:" << cgroup->getId() << ", parent is branch node
                    //                         (right child) " << endl;
                }
            }
            else // parentNode is an execution path node
            {
                ExecutionPathNode *tempExPNParent = static_cast<ExecutionPathNode *>(parentNode);
                tempExPNParent->setChildNode(toBeKeptChild);

                //                cout << "PathManager: execution path: " << cgroup->getExecutionPathId()
                //                     << ", Delete cgroup:" << cgroup->getId() << ", parent is the execution path node
                //                     itself " << endl;
            }

            if (!garbageCollectionThread->pushToQueue(tempBN))
                cout << "PathManager: garbage collection queue is full" << endl;
            // cgroup has been found and deleted
            return true;
        }
        else // still didn't get the branch node that contains the deleted/validated cgroup
        {
            /*
             * The cgroup is  either in both children or not at all in the tree. Therefore, we search first in right
             * branch. If the cgroup is not
             * found in the right child, it is for sure not on left child as well. So return false.
             */
            bool result = deleteBranch(tempBN->getRightChildNode(), cgroup);
            if (!result) // cgroup is not found
                return false;

            deleteBranch(tempBN->getLeftChildNode(), cgroup);

            return true;
        }
    }
    else // execution path node:
    {
        // cgroup not found, so return it back to the queue till the cgroup is fetched from newCgroups queue
        return false;
    }
}

void PathManager::cleanExecutionPathExecutionPathMap(AbstractNode *startNode)
{
    // remove all execution paths starting from 'startNode' from  ExecutionPathExecutionPathNodeMap
    if (startNode == nullptr)
        return;

    if (startNode->isBranchNode())
    {
        BranchNode *temp = static_cast<BranchNode *>(startNode);
        cleanExecutionPathExecutionPathMap(temp->getLeftChildNode().get());
        cleanExecutionPathExecutionPathMap(temp->getRightChildNode().get());
    }
    else // execution path node
    {
        ExecutionPathNode *temp = static_cast<ExecutionPathNode *>(startNode);
        this->executionPathExecutionPathNodeMap.erase(temp->getExecutionPath()->getId());

        cleanExecutionPathExecutionPathMap(temp->getChildNode().get());
    }
}

void PathManager::setMasterExecutionPathFinished(bool value) { this->masterExecutionPathFinished.store(value); }

unsigned long PathManager::getSelectionSize() const { return selectionSize.load(); }

void PathManager::setSelectionSize(unsigned long value) { selectionSize.store(value); }

void PathManager::setTerminate() { this->terminate.store(true); }

PathManager::~PathManager() {}
}
