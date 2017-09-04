/*
 * helper.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/util/Helper.hpp"
namespace util
{

/**
 * as in java:
 *@return the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
 */
unsigned long Helper::currentTimeMillis()
{
    using namespace std::chrono;

	auto epoch = high_resolution_clock::from_time_t(0);
	auto now = high_resolution_clock::now();

    auto mseconds = duration_cast<milliseconds>(now - epoch).count();
    return static_cast<unsigned long>(mseconds);

}

unsigned long Helper::currentTimeMicros()
{
    using namespace std::chrono;

    auto epoch = high_resolution_clock::from_time_t(0);
    auto now = high_resolution_clock::now();

    auto mseconds = duration_cast<microseconds>(now - epoch).count();
    return static_cast<unsigned long>(mseconds);
}

/**
 * split string using delimiter
 * @param text: string to be splitted
 * @param delimiter: the delimiter
 * @return vector of splitted string
 */
vector<string> Helper::split(string text, char delimiter)
{
	vector<string> elements;
	string item;
	stringstream ss(text);
	while (getline(ss, item, delimiter))
		elements.push_back(item);
	return elements;
}

/**
 * split string using delimiter
 * @param text: string to be splitted
 * @param delimiter: the delimiter
 * @param elements: the container to hold the extracted elements (splitted string)
 * @return vector of splitted string
 */
vector<string>& Helper::split(string text, char delimiter, vector<string>& elements)
{
	string item;
	stringstream ss(text);
	while (getline(ss, item, delimiter))
		elements.push_back(item);
	return elements;
}
}

