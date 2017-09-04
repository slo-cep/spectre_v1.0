/*
 * ExecutionPathNode.cpp
 *
 * Created on: 05.12.2016
 *      Author: sload
 */

#include "../../header/data_structure/ExecutionPathNode.hpp"



namespace execution_path
{

ExecutionPathNode::ExecutionPathNode() : AbstractNode(NodeType::EXECUTION_PATH_NODE) {}

ExecutionPathNode::ExecutionPathNode(const shared_ptr<ExecutionPath> &executionPath)
    : AbstractNode(NodeType::EXECUTION_PATH_NODE)
{
    this->executionPath = executionPath;
}

ExecutionPathNode::ExecutionPathNode(const ExecutionPathNode &other) : AbstractNode(other)
{
    this->executionPath = other.executionPath;
    this->childNode = other.childNode;
}

shared_ptr<AbstractNode> ExecutionPathNode::clone() { return make_shared<ExecutionPathNode>(*this); }

shared_ptr<ExecutionPath> &ExecutionPathNode::getExecutionPath()  { return executionPath; }

void ExecutionPathNode::setExecutionPath(const shared_ptr<ExecutionPath> &executionPath)
{
    this->executionPath = executionPath;
}

shared_ptr<AbstractNode> &ExecutionPathNode::getChildNode()  { return childNode; }

void ExecutionPathNode::setChildNode( const shared_ptr<AbstractNode>& childNode) { this->childNode = childNode; }

void ExecutionPathNode::resetChildNode() { this->childNode = nullptr; }

ExecutionPathNode::~ExecutionPathNode() {}
}
