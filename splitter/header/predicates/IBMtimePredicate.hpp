#ifndef IBMTIMEPREDICATE_H
#define IBMTIMEPREDICATE_H

#include "AbstractPredicate.hpp"

#include <set>

namespace splitter
{
class IBMtimePredicate : public AbstractPredicate
{
public:
    IBMtimePredicate(unsigned long time);

    virtual bool ps(const events::AbstractEvent &event) const;
    virtual bool pc(const events::AbstractEvent &event, selection::AbstractSelection &abstractSelection);
    virtual ~IBMtimePredicate() {}

private:
    unsigned long timeOpen;
    set<string> symbols;
};
}
#endif // IBMTIMEPREDICATE_H
