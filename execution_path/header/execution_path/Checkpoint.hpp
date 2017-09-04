/*
 * Checkpoint.hpp
 *
 * Created on: 18.11.2016
 *      Author: sload
 */

#ifndef CHECKPOINT_HPP
#define CHECKPOINT_HPP

#include "header/util/GlobalTypedef.hpp"


#include <memory>

using namespace std;

namespace execution_path
{
class ExecutionPath;
class Checkpoint
{
public:
    typedef util::GlobalTypedef::GlobalEventStreamTypedef GlobalEventStreamTypedef;
    typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    /**
     * @brief generateCheckpointId: generate unique Ids for the checkpoints
     * @return: new unique Id
     */
    static unsigned long generateCheckpointId();

    Checkpoint();
    Checkpoint(shared_ptr<ExecutionPath> executionPath, shared_ptr<events::AbstractEvent> checkpointingEvent);


    shared_ptr<ExecutionPath> getExecutionPath() const;
    void setExecutionPath(const shared_ptr<ExecutionPath> &executionPath);

    const shared_ptr<events::AbstractEvent>& getCheckpointingEvent() const;
    void setCheckpointingEvent(const shared_ptr<events::AbstractEvent> &checkpointingEvent);

    unsigned int getId() const;
    void setId(unsigned int Id);

    ~Checkpoint();
private:
    shared_ptr<ExecutionPath> executionPath;
    shared_ptr<events::AbstractEvent> checkpointingEvent;
    unsigned int Id;

    /*
     * used to generate unique Ids for the checkpoints.
     * Use atomic because it is accessible from multiple workers
     */
    static atomic<unsigned long> lastCheckpointId;
};

}

#endif // CHECKPOINT_HPP
