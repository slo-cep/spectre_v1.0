/*
 * BranchNode.cpp
 *
 * Created on: 10.11.2016
 *      Author: sload
 */

#include "../../header/data_structure/BranchNode.hpp"

using namespace profiler;

namespace execution_path
{

BranchNode::BranchNode() : AbstractNode(NodeType::BRANCH_NODE) {}

BranchNode::BranchNode(const shared_ptr<Cgroup> &cgroup) : AbstractNode(NodeType::BRANCH_NODE), cgroup(cgroup) {}

BranchNode::BranchNode(const shared_ptr<Cgroup> &cgroup, const shared_ptr<AbstractNode> &leftChild,
                       const shared_ptr<AbstractNode> &rightChild)
    : AbstractNode(NodeType::BRANCH_NODE), cgroup(cgroup), leftChildNode(leftChild), rightChildNode(rightChild)
{
}

BranchNode::BranchNode(const BranchNode &other) : AbstractNode(other)
{
    this->cgroup = other.cgroup;
    this->leftChildNode = other.leftChildNode;
    this->rightChildNode = other.rightChildNode;
}

shared_ptr<AbstractNode> BranchNode::clone() { return make_shared<BranchNode>(*this); }

const shared_ptr<Cgroup> &BranchNode::getCgroup() const { return cgroup; }

void BranchNode::setCgroup(const shared_ptr<Cgroup> &cgroup) { this->cgroup = cgroup; }

shared_ptr<AbstractNode> &BranchNode::getLeftChildNode() { return leftChildNode; }

void BranchNode::setLeftChildNode(shared_ptr<AbstractNode> leftChildNode)
{
    this->leftChildNode = leftChildNode;
}

void BranchNode::resetLeftChildNode() { this->leftChildNode = nullptr; }

shared_ptr<AbstractNode> &BranchNode::getRightChildNode() { return rightChildNode; }

void BranchNode::setRightChildNode(shared_ptr<AbstractNode> rightChildNode)
{
    this->rightChildNode = rightChildNode;
}

void BranchNode::resetRightChildNode() { this->rightChildNode = nullptr; }

BranchNode::~BranchNode() {}
}
