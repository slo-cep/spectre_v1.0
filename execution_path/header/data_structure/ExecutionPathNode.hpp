/*
 * ExecutionPathNode.hpp
 *
 * Created on: 07.12.2016
 *      Author: sload
 */

#ifndef EXECUTIONPATHNODE_HPP
#define EXECUTIONPATHNODE_HPP

#include "AbstractNode.hpp"

#include "../../header/execution_path/ExecutionPath.hpp"


#include <memory>

using namespace std;

namespace execution_path
{
class ExecutionPathNode : public AbstractNode
{
public:
    ExecutionPathNode();

    /**
     * @brief ExecutionPathNode: Ctor
     * @param executionPath: pointer to an execution path
     */
    ExecutionPathNode(const shared_ptr<ExecutionPath> &executionPath);

    /**
     * @brief ExecutionPathNode: Copy constructor
     * @param other: rhs
     */
    ExecutionPathNode(const ExecutionPathNode& other);

    shared_ptr<AbstractNode> clone() override;


    shared_ptr<ExecutionPath> &getExecutionPath();
    void setExecutionPath(const shared_ptr<ExecutionPath> &value);

    shared_ptr<AbstractNode> &getChildNode();
    void setChildNode(const shared_ptr<AbstractNode> &childNode);
    void resetChildNode();

    ~ExecutionPathNode();

private:
    shared_ptr<ExecutionPath> executionPath=NULL;

    shared_ptr<AbstractNode> childNode=NULL;
};
}

#endif // EXECUTIONPATHNODE_HPP
