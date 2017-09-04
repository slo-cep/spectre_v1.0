/*
 * Marshaller.cpp
 *
 *  Created on: Jul 28, 2016
 *      Author: sload
 */

#include "../../header/util/Marshaller.hpp"

namespace util
{

/**
 * build event object from String representation
 *
 * the string is split in two steps:
 * (1) split by parameter
 * (2) split each parameter by key value delimiter to determine key and value
 * (3) if key is content, determine the different content key value pairs in the same way as (1) and (2); however, the pairs have a different delimiter
 * @param eventString: the event in string state
 * @return share_ptr to created event
 */
shared_ptr<events::AbstractEvent> Marshaller::unmarsharllerEvent(string eventString)
{
	shared_ptr<events::AbstractEvent> result;

	/*
	 * parameters of the event built from eventString
	 * p = time stamp
	 * v = validity time
	 * n = source id
	 * s = sequence number
	 * t = type
	 * c= content
	 */

//    unsigned long timestamp ;
//    unsigned long overlap;
//    unsigned long sn ;
//    unsigned long sourceId;
//	string type = "";
//	string contentString = "";

//	vector<string> temp = Helper::split(eventString, Constants::PARAMETER_DELIMITER);

//	/*
//	 * step (1)
//	 */
//	for (unsigned int i = 0; i < temp.size(); i++)
//	{
//		vector<string> keyvalue;
//		keyvalue = Helper::split(temp[i], Constants::PARAMETER_KEY_VALUE_DELIMITER);

//		switch (keyvalue[0].c_str()[0])
//		{
//			case 'p':
//			{
//                timestamp =static_cast<unsigned long>(stol(keyvalue[1]));
//				break;
//			}
//			case 'o':
//			{
//                overlap = static_cast<unsigned long>(stol(keyvalue[1]));
//				break;
//			}
//			case 'n':
//			{
//                sourceId = static_cast<unsigned long>(stol(keyvalue[1]));
//				break;
//			}
//			case 's':
//			{
//                sn = static_cast<unsigned long>(stol(keyvalue[1]));
//				break;
//			}
//			case 't':
//			{
//				type = keyvalue[1];
//				break;
//			}
//			case 'c':
//			{
//				contentString = keyvalue[1];

//			}
//		}
//	}

//	/*
//	 * build event and meta data:
//	 * - event based on type; sn and type are assigned in constructor
//	 * - sourceId
//	 */
//	switch (type.c_str()[0])
//	{
//		case Constants::EVENT_TYPE_SIMPLE_VALUE:
//		{
//			//result = make_shared<SimpleValueEvent>(sn);
//			break;
//		}
//		default:
//		{
//			cout<<"unknown type in Marshaller: cannot build event. This shouldn't happen!";
//			cout<< eventString;
//			return NULL;
//		}
//	}

	return result;
}
}

