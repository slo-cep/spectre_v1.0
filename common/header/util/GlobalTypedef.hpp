/*
 * GlobalTypedef.hpp
 *
 *  Created on: Aug 5, 2016
 *      Author: sload
 */

#ifndef HEADER_UTIL_GLOBALTYPEDEF_HPP_
#define HEADER_UTIL_GLOBALTYPEDEF_HPP_
#include "../events/AbstractEvent.hpp"
#include "../shared_memory/AbstractGlobalEventStream.hpp"
#include "../shared_memory/AbstractSourceSplitterStream.hpp"

#include <memory> //shared_ptr
#include <list>

using namespace std;
namespace util
{
class GlobalTypedef
{
public:
    typedef list<shared_ptr<events::AbstractEvent>> Container;

    typedef shared_memory::AbstractGlobalEventStream<Container> GlobalEventStreamTypedef;
	typedef typename GlobalEventStreamTypedef::iterator GlobalEventStreamTypedef_iterator;

    typedef shared_memory::AbstractSourceSplitterStream SourceSplitterStreamTypedef;
};
}

#endif /* HEADER_UTIL_GLOBALTYPEDEF_HPP_ */
