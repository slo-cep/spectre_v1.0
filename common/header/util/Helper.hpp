/*
 * helper.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_UTIL_HELPER_HPP_
#define HEADER_UTIL_HELPER_HPP_

#include <chrono> //high resolution clock
#include <string>
#include <vector>
#include <sstream>
#include <atomic>

using namespace std;

namespace util
{

/**
 * contains a set of helper functions
 */

class Helper
{
public:
	/**
	 * as in java:
	 *@return the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
	 */

	static unsigned long currentTimeMillis();

    static unsigned long currentTimeMicros();


	/**
	 * split string using delimiter
	 * @param text: string to be splitted
	 * @param delimiter: the delimiter
	 * @return vector of splitted string
	 */
	static vector<string> split(string text, char delimiter);

	/**
	 * split string using delimiter
	 * @param text: string to be splitted
	 * @param delimiter: the delimiter
	 * @param elements: the container to hold the extracted elements (splitted string)
	 * @return the elements vector after pushing the elements (splitted string)
	 */
	static vector<string>& split(string text, char delimiter, vector<string>& elements);
};

}

#endif /* HEADER_UTIL_HELPER_HPP_ */
