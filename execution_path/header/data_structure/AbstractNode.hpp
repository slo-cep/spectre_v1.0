/*
 * AbstractNode.hpp
 *
 * Created on: 06.12.2016
 *      Author: sload
 */

#ifndef ABSTRACTNODE_HPP
#define ABSTRACTNODE_HPP

#include <memory>

using namespace std;
namespace execution_path
{
class AbstractNode
{
public:
    enum NodeType
    {
        BRANCH_NODE,
        EXECUTION_PATH_NODE
    };

    virtual shared_ptr<AbstractNode> clone() = 0;

    NodeType getType();
    bool isBranchNode();
    bool isExecutionPathNode();
    AbstractNode *getParentNode() const;
    void setParentNode(AbstractNode *value);

    float getProbability() const;
    void setProbability(float probability);

protected:
    NodeType type;
    AbstractNode *parentNode=NULL;
    float probability;

    AbstractNode(NodeType type);
    /**
     * @brief AbstractNode: Copy constructor
     * @param other: rhs
     */
    AbstractNode(const AbstractNode &other);

    virtual ~AbstractNode();

};
}

#endif // ABSTRACTNODE_HPP
