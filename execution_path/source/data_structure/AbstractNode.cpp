/*
 * AbstractNode.cpp
 *
 * Created on: 06.12.2016
 *      Author: sload
 */

#include "../../header/data_structure/AbstractNode.hpp"
namespace execution_path
{

AbstractNode::AbstractNode(NodeType type) : type(type) {}

AbstractNode::AbstractNode(const AbstractNode &other)
    : type(other.type), parentNode(other.parentNode), probability(other.probability)
{
}

AbstractNode::NodeType AbstractNode::getType() { return type; }

bool AbstractNode::isBranchNode() { return type == NodeType::BRANCH_NODE; }

bool AbstractNode::isExecutionPathNode() { return type == NodeType::EXECUTION_PATH_NODE; }

AbstractNode *AbstractNode::getParentNode() const { return parentNode; }

void AbstractNode::setParentNode(AbstractNode *value) { parentNode = value; }

float AbstractNode::getProbability() const { return probability; }

void AbstractNode::setProbability(float probability) { this->probability = probability; }

AbstractNode::~AbstractNode() {}
}
