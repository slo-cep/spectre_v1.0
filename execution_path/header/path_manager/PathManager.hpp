/*
 * PathManager.hpp
 *
 * Created on: 05.12.2016
 *      Author: sload
 */

#ifndef PATHMANAGER_HPP
#define PATHMANAGER_HPP

#include "header/events/events.hpp"
#include "header/selection/AbstractSelection.hpp"
#include "header/shared_memory/LockfreeQueue.hpp"
#include "header/shared_memory/ConcurrentQueueWrapper.hpp"

#include "header/Measurements.hpp"

#include "header/util/Helper.hpp"

#include "../../header/data_structure/AbstractNode.hpp"
#include "../../header/data_structure/BranchNode.hpp"
#include "../../header/data_structure/ExecutionPathNode.hpp"

#include "GarbageCollectionThread.hpp"

#include "../../header/worker/WorkerThread.hpp"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>
using namespace std;
namespace execution_path
{
class ExecutionPath;
class ExecutionPathFactory;
class PathManager
{
public:
    /**
     * @brief PathManager
     * @param workerThreads: worker threads
     * @param executionPathFactory: responsible for execution path creation
     * @param markovMatrix
     * @param initialSelectionSize: initial size of a selection (window)
     * @param treeSize: maximum number of execution path nodes (execution paths) in the tree. A large number can
     * overload the path manager!
     * @param garbageCollectionThread: pointer to garbage collection
     * @param measurements: for profiling
     */
    PathManager(vector<shared_ptr<WorkerThread>> workerThreads, shared_ptr<ExecutionPathFactory> executionPathFactory,
                shared_ptr<MarkovMatrix> markovMatrix, unsigned long initialSelectionSize, unsigned int treeSize,
                GarbageCollectionThread *garbageCollectionThread, profiler::Measurements*  measurements);

    void main();

    // Selection
    void receiveNewSelection(const shared_ptr<selection::AbstractSelection> &abstractSelection);

    // Cgroups
    void receiveCgroup(const shared_ptr<Cgroup> &cgroup, Cgroup::Status status);

    /**
     * @brief setMasterExecutionPathFinished: whenever the master execution paths finishes, it call this function
     * and set the masterExecutionPathFinished flag to true;
     */
    void setMasterExecutionPathFinished(bool value);

    unsigned long getSelectionSize() const;
    void setSelectionSize(unsigned long value);

    /**
     * @brief setTerminate: terminate the path manager
     */
    void setTerminate();

    ~PathManager();

private:
    vector<shared_ptr<WorkerThread>> workerThreads;
    shared_ptr<ExecutionPathFactory> executionPathFactory;
    // Markov matrix to calculate the Probabilities
    shared_ptr<MarkovMatrix> markovMatrix;
    GarbageCollectionThread *garbageCollectionThread;
    atomic<unsigned long> selectionSize;
    unsigned int treeSize;
    unsigned int treeDepth = 0;

    // store Id of the current master execution path
    unsigned long masterExecutionPathId;
    atomic<bool> masterExecutionPathFinished = {false};

    // terminate path manager
    atomic_bool terminate = {false};

    // map of scheduled execution path Ids and index of worker threads
    unordered_map<unsigned long, size_t> executionPathWorkThreadMap;
    /*
      * map an execution path to an execution path node in the tree
      * (execution path Id, pointer to execution path node)
      */
    unordered_map<unsigned long, ExecutionPathNode *> executionPathExecutionPathNodeMap;

    // Tree node root: the root node is always an execution path node!
    shared_ptr<ExecutionPathNode> root;

    list<shared_ptr<Cgroup>> bufferedRootCgroups;

    // queue of all new selections that are sent by Splitter
//    shared_memory::LockfreeQueue<shared_ptr<selection::AbstractSelection>> newSelections{0};
    shared_memory::ConcurrentQueueWrapper<shared_ptr<selection::AbstractSelection>> newSelections;
//     boost::lockfree::spsc_queue<shared_ptr<selection::AbstractSelection>> newSelections{100000};

    // cgroups
//    shared_memory::LockfreeQueue<shared_ptr<Cgroup>> newCgroups{0};
    shared_memory::ConcurrentQueueWrapper<shared_ptr<Cgroup>> newCgroups;
    list<shared_ptr<Cgroup>> newCgroupsList;
//    shared_memory::LockfreeQueue<shared_ptr<Cgroup>> updatedCgroups{0};
    shared_memory::ConcurrentQueueWrapper<shared_ptr<Cgroup>> updatedCgroups;
//    shared_memory::LockfreeQueue<shared_ptr<Cgroup>> deletedCgroups{0};
    shared_memory::ConcurrentQueueWrapper<shared_ptr<Cgroup>> deletedCgroups;
    unordered_map<unsigned long, shared_ptr<Cgroup>> deletedCgroupsMap;

    // TopK result
    vector<shared_ptr<ExecutionPath>*> topKExecutionPaths;

    unsigned long cgroupNewCounter = 0;
    unsigned long cgroupUpdateCounter = 0;
    unsigned long cgroupDeleteCounter = 0;


    profiler::Measurements*  measurements;

    // functions ---------------------------------------------------------------------
    /**
     * @brief findNextRoot: in case the cgroup of the current master are not appropriately cleaned,
     * this function clean them and try to find the next master (next execution path node)
     * @param startNode: the first child of the current master
     * @return Null if there is no other execution path nodes in the tree
     * else the new master (execution path node)
     */
    shared_ptr<ExecutionPathNode> findNextRoot(const shared_ptr<AbstractNode> &startNode);

    /**
     * @brief checkNewSelections: check whether there is new Selections. For each new Selection
     * create an execution path and call AddNewExecutionPathToTreeNode() to add it to the Tree
     */
    void checkNewSelections();

    /**
     * @brief checkValidBufferedRootCgroups: check whether the buffered cgroups from earlier parents are
     * still needed by a new Selection. Not needed cgroups are removed from the buffer.
     * @param selectionStartEventSn: Sn of the first event in the Selection to detect the overlap
     */
    void checkValidBufferedRootCgroups(unsigned long selectionStartEventSn);

    /**
     * @brief addExecutionPathNodeToTreeNode:  Traverse the tree node and add the execution path node
     * as a child for each tree branch node or execution path node
     * @param startNode:
     * @param  executionPathNode
     * @par cgroups: all cgroups on the right children of the branch nodes
     */
    void addExecutionPathNodeToTreeNode(AbstractNode *startNode, const shared_ptr<ExecutionPathNode> &executionPathNode,
                                        unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &cgroups);

    // scheduling
    void assignExecutionPathToWorkerThread();
    vector<shared_ptr<ExecutionPath> *> getTopKExecutionPath(shared_ptr<ExecutionPathNode> root, size_t k);

    // Cgroups

    /**
     * @brief processNewCgroups: get cgroups from the newCgroups queue and call branch function. see below
     */
    void processNewCgroups();

    void addNewCgroup(const shared_ptr<Cgroup>& cgroup);

    /**
     * @brief branch: add new branches
     * traverse tree node starting from the execution path that generated the cgroup till
     *      next execution path node, i.e. child execution path (child_path):
     *  - insert the new cgroup as a parent of the child_path
     *  - replicate all child_path and all its children
     *       * run new instances (branching) of child_path and its children from their local check points.
     *       * pass the new cgroup to all new branches
     * @param parentNode: pointer to parent node
     * @param startNode: child of the execution path that generated the cgroup
     * @param newBranchNode: the branch node that contains the new cgroup
     */
    void branch(AbstractNode *parentNode, shared_ptr<AbstractNode> &startNode,
                const shared_ptr<BranchNode> &newBranchNode);

    /**
     * @brief cloneSubTree: replicate the tree starting from a specific node (startNode).
     * @param parentNode: pointer to parent node
     * @param rightStartCloneNode: out variable where every execution path (beginning from
     * startNode) is cloned and appended to rightStartCloneNode.
     * @param startNode: **reference** to the node where the clone starts.
     * @param cgroups: the newly generated cgroup + all old cgroups that will be add to each newly cloned execution
     * path.
     */
    void cloneSubTree(AbstractNode *parentNode, shared_ptr<AbstractNode> &rightStartCloneNode, AbstractNode *startNode,
                      const unordered_map<unsigned long, pair<shared_ptr<Cgroup>, unsigned int>> &cgroups);

    /**
     * @brief processUpdatedCgroups: check if this cgroups become valid or invalid
     * if yes delete the invalid branches
     */
    void processUpdatedCgroups();

    /**
     * @brief readDeletedCgroups: get all deleted cgroups from deletedCgroups queue and insert them in
     * deletedCgroupsMap
     */
    void readDeletedCgroups();
    /**
     * @brief processDeletedCgroups: delete branches
     */
    void processDeletedCgroups();

    bool deleteCgroup(const shared_ptr<Cgroup> &cgroup);

    /**
     * @brief deleteBranch: delete branches as a result of deleted or valid cgroup
     *- Only one child (right or left) of the branch nodes that contains the deleted or valid cgroup should
     *   be deleted (other child does/ doesn't use the cgroup)
     * - The function search for the cgroup starting from startNode and then it delete
     * the branch node the contains this cgroup. It assigns the left child on this branch node
     * to to it parent (branch node's parent) as the left child doesn't use the cgroup
     * @param startNode: first child of the execution path that generated this deleted cgroup.
     * Or the first node we deleteBranch will start to look for the cgroup
     * @param cgroup: the deleted cgroup and valid cgroup
     * check it validation field to decide whether it is valid or not:
     * - if invalid delete right child
     * - if valid delete left child
     * @return true is the cgroup was found and deleted
     * else false which can happen (that may be the case if the cgroup is still in newCgroups queue but not in the tree)
     */
    bool deleteBranch(const shared_ptr<AbstractNode> &startNode, const shared_ptr<Cgroup> &cgroup);

    void cleanExecutionPathExecutionPathMap(AbstractNode *startNode);

    /**
     * @brief checkMasterExecutionPathFinished: check if the master execution path has finished
     * - if yes:
     * @return true if master finished else false
     */
    bool checkMasterExecutionPathFinished();
};

struct LessThanByP
{
    bool operator()(AbstractNode *lhs, AbstractNode *rhs) const
    {
        return lhs->getProbability() < rhs->getProbability();
    }
};
}

#endif // PATHMANAGER_HPP
