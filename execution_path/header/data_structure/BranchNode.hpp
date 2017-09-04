/*
 * BranchNode.hpp
 *
 * Created on: 10.11.2016
 *      Author: sload
 */

#ifndef BRANCHNODE_HPP
#define BRANCHNODE_HPP

#include "AbstractNode.hpp"

#include "../../header/execution_path/Cgroup.hpp"

#include <memory>

using namespace std;

namespace execution_path
{

class BranchNode : public AbstractNode
{
public:
    BranchNode();

    /**
     * @brief Ctor
     * @param cgroup: the cgroup that causes the branching decision
     */
    BranchNode(const shared_ptr<Cgroup> &cgroup);

    /**
     * @brief Ctor
     * @param cgroup: the cgroup that causes the branching decision
     * @param leftChildNode: left child branch node or execution path node
     * @param rightChildNode: left child branch node or execution path node
     * Note: left and right children can be branch node(s) or execution path node(s) but not both
     */
    BranchNode(const shared_ptr<Cgroup> &cgroup, const shared_ptr<AbstractNode> &leftChildNode,
               const shared_ptr<AbstractNode> &rightChildNode);

    /**
     * @brief BranchNode: Copy constructor
     * @param other: rhs
     */
    BranchNode(const BranchNode &other);

    shared_ptr<AbstractNode> clone() override;

    const shared_ptr<Cgroup> &getCgroup() const;
    void setCgroup(const shared_ptr<Cgroup> &cgroup);

    shared_ptr<AbstractNode> &getLeftChildNode();
    void setLeftChildNode(shared_ptr<AbstractNode> leftChildNode);
    void resetLeftChildNode();

    shared_ptr<AbstractNode> &getRightChildNode() ;
    void setRightChildNode(shared_ptr<AbstractNode> rightChildNode);
    void resetRightChildNode();

    virtual ~BranchNode();

private:
    shared_ptr<Cgroup> cgroup;

    shared_ptr<AbstractNode> leftChildNode=NULL;  // not consumed
    shared_ptr<AbstractNode> rightChildNode=NULL; // consumed
};
}

#endif // BRANCHNODE_HPP
