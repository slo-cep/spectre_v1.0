/*
 * EndOfSelection.hpp
 *
 *  Created on: Sep 05, 2016
 *      Author: sload
 */

#ifndef ENDOFSELECTION_HPP
#define ENDOFSELECTION_HPP

#include "AbstractEvent.hpp"
#include "../../header/util/Constants.hpp"

namespace events
{
class EndOfSelection : public AbstractEvent
{
public:
    /**
     * Constructor
     * @param sn: sequence number
     * @param measurements: for profiling
     */
    EndOfSelection(unsigned long sn, profiler::Measurements *measurements);


    /**
     * no binary content, nothing to do
     */
    void marshalBinaryContent() override;
    /**
     * no binary content, nothing to do
     */
    void unmarshalBinaryContent() override;

    ~EndOfSelection();
};
}

#endif // ENDOFSELECTION_HPP
