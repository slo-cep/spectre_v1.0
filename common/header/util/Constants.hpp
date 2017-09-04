/*
 * Constants.hpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#ifndef HEADER_UTIL_CONSTANTS_HPP_
#define HEADER_UTIL_CONSTANTS_HPP_

#include <string>

using namespace std;

namespace util
{

/*
 * Constants
 *
 * set up of the system
 *
 * marshalling of events and selections
 */

class Constants
{
public:
	static const long PULSE_TIME_MS = 10; // pulse time for sources (important in multi-source scenarios)

	static const string ADAPTATION_MODE_CPU; // adapt the degree based on CPU utilization of the hosts
	static const string ADAPTATION_MODE_QT; // the qt adaptation mode is implementing our approach
	static const string ADAPTATION_MODE_NONE; // the parallelization degree is not adapted, but remains static

	/*-------------------------------
	 * Source Play: Scenarios
	 * ------------------------------
	 */
	static const string PLAY_SCENARIO_PLAYLOG; // play a log file that contains events (each line one event)
	static const string PLAY_SCENARIO_FACES; // play images that contain faces
	static const string PLAY_SCENARIO_FACE_RECOGNITION_QUERIES; // play face recognition queries
	static const string PLAY_SYNTHETIC_FACES; // plays synthetic face scenario (not real data)

	/* -------------------------
	 * Allowed Operator Types
	 * ------------------------
	 */
	enum OperatorType
	{
        SEQUENCE_OPERATOR,
        STOCK_RISE_OPERATOR,
        HEAD_AND_SHOULDERS_OPERATOR
	};
	static const string OPERATOR_DUMMY; // does nothing, just takes events and deletes them. sends no ACKs back
	static const string OPERATOR_TRAFFIC; // monitors the no passing zone
	static const string OPERATOR_FACE_RECOGNITION; // recognizes persons in face pictures
	static const string FACE_FILE_EXTENSION; // format of files to save pictures of faces
	static const string OPERATOR_EXPONENTIAL_WAIT;
	static const string OPERATOR_UNIFORM_WAIT;
	static const string OPERATOR_PARETO_WAIT;
	static const string OPERATOR_LOGNORMAL_WAIT;
	static const string OPERATOR_TREE;
	static const string OPERATOR_POWERSET;

	/*-------------------------
	 * Predicates
	 * ------------------------
	 */
	static const string PREDICATE_TUPLE_WINDOW; // the tuple-based sliding window
	static const string PREDICATE_TIME_WINDOW; // the time-based sliding window
	static const string PREDICATE_TRAFFIC; // predicate for traffic monitoring operator
	static const string PREDICATE_FACE_RECOGNITION; // predicate for the face recognition operator

	/* -------------------
	 * Queueing Theory
	 * -------------------
	 */
	static const long T_H = 60000; // time in ms needed to adapt the parallelization degree = prediction horizon
	static const int BL_OMEGA = 15; // the boundary of the buffer for the operator
	static constexpr double P_REQUIRED = 0.95; // the required probability of keeping the buffer limit BL_OMEGA

	/*------------------------------
	 * Adaptation on CPU utilization
	 * -----------------------------
	 */
	static constexpr double CPU_BOUND_HIGH = 0.7; // scale up at this bound
	static constexpr double CPU_BOUND_LOW = 0.5; // scale down at this bound
	static const long CPU_UTILIZATION_REPORT_TIME = 5000; // report CPU utilization over x seconds
	static const int CPU_SCALE_NO_OF_MEASUREMENTS = 2; // scale decision after x measurements

	/*------------------
	 * Cost measurement
	 * -----------------
	 */
	static const long COST_MEASUREMENT_GRANULARITY = 60000; // time in milliseconds for the production of 1 Cost Unit

	/*-----------------
	 * Events
	 * - Note: Event identifier should be a unique string that can be casted to an Integer
	 * ----------------
	 */
	enum EventType
	{
		SIMPLE_VALUE,
        PULSE,
        END_OF_STREAM,
        Complex,
        END_OF_SELECTION,
		SCENARIO_PLAYLOG
	};

	static const string EVENT_TYPE_DEMAND_STATUS; // demand-response events
	static const string EVENT_TYPE_GRID_STATUS; // report: energy production and consumption aggregated
	static const string EVENT_TYPE_VEHICLE_POS_1; // vehicle detected at pos_1
	static const string EVENT_TYPE_VEHICLE_POS_2; // vehicle detected at pos_2
	static const string EVENT_TYPE_VEHICLE_VIOLATOR; // vehicle detected to have violated the traffic rules
	static const char EVENT_TYPE_SIMPLE_VALUE = '5'; //just a simple double value in the contents "v=double"
	static const string EVENT_TYPE_POWERSET_EVENT; // power set events consist of a set id (= selection id, id tuple) and a content char
	static const char EVENT_TYPE_STREAM_ENDS; // the stream has finished, and the components are asked to shut down.

	/*
	 * value delimiter in  events
	 */
	static const string EVENT_TYPE_SIMPLE_VALUE_CONTENT_1_KEY; //key of first content
	static const string EVENT_TYPE_VEHICLE_CONTENT_1_KEY; // = number plate

	/*
	 * other events
	 */
	static const string EVENT_TYPE_FACE;
	static const string EVENT_TYPE_FACE_CONTENT_KEY;
	static const string EVENT_TYPE_FACE_RECOGNITION_QUERY;
	static const string EVENT_TYPE_FACE_RECOGNITION_QUERY_CONTENT_KEY;
	static const string EVENT_TYPE_FACE_RECOGNITION_QUERY_FACES_OF_FRIEND_DELIMITER; //separate the marshalled faces of a friend (no base64 character)
	static const string EVENT_TYPE_FACE_MATCH;
	static const char EVENT_TYPE_PULSE;

	/*
	 * content is delimited separately for a simple implementation of the parsing
	 */
	static const char CONTENT_DELIMITER; // delimiter of different content key/value pairs
	static const char CONTENT_KEY_VALUE_DELIMITER; // delimiter between key and value of content

//	static const long EVENT_DEFAULT_VALIDITY_TIME = 10000; // default validity time of an event in ms (in operators where this plays a role)

	/*-----------------
	 * Prefixes for messages
	 * ----------------
	 */
	static const char BOUNDARY_PREDICTION_CODE = 'B'; // prefix that signals the line contains a BoundarySchedulerPrediction
	static const char CPU_MEASUREMENT_CODE = 'C'; // prefix that signals the line contains a CPU measurement
	static const char COST_MEASUREMENT_CODE = 'M'; //prefix that signals the line contains a cost measurement 'M for money'
	static const char OPERATIONAL_LATENCY_MEASUREMENT_CODE = 'L'; // prefix that signals the line contains an instance's operational latency measurement
	static const char PROCESSING_LATENCY_MEASUREMENT_CODE = 'P'; // prefix that signals the line contains an Instance Monitor's processing latency measurement
	static const char QUEUE_LENGTH_FEEDBACK_CODE = 'Q'; // signals the line contains queue length feedback from an operator instance
	static const char STATISTICS_PROCESSING_LATENCY_CODE = 'W'; // prefix that signals the lines contains an Instance's Monitoring Window's processing latency measurements statistics
	static const char EVENT_CODE = 'E'; // prefix that signals the line contains an Event
	static const char SELECTION_CODE = 'S'; // prefix that signals the line contains a selection
	static const char SOURCE_CODE = '$'; // prefix that signals the line contains a source registration
	static const char INSTANCE_CODE = 'I'; // prefix that signals the line contains an instance registration

	/*-----------------
	 * Marshalling and Unmarshalling
	 * ----------------
	 */
	static const char PARAMETER_DELIMITER; // delimiter of different parameters in its string representation
	static const char PARAMETER_KEY_VALUE_DELIMITER; // delimiter between key and value in event string representation"

	/*--------------------------
	 * Scheduling
	 * ---------------------------
	 */
	static const string SCHEDULING_ROUND_ROBIN; // classic round robin
	static const string SCHEDULING_FIXED_BATCH; // fixed batch size scheduling
	static const string SCHEDULING_REACTIVE; // scheduling reactive to latency measurements
	static const string SCHEDULING_PREDICTIVE; // predictive scheduling
	static const string SCHEDULING_BOUNDARY; // batch scheduling based on worst-and best-case analysis
	static const string SCHEDULING_AVERAGE_BOUNDARY; // batch scheduling based on average case analysis

	// batch scheduling parameters
	static constexpr double R_LINEAR = -1.1; // threshold of Pearson's r indicating LINEAR dependency of lat on pos
	static constexpr double R_DEPENDENT = 0.1; // threshold of Pearson's r indicating ANY dependency of lat on pos
//	static constexpr double D_C = 10; // constant of d_max in milliseconds; d_max is the threshold of standard deviation that signals high dispersion / content-dependency
//	static constexpr double D_M = 0.5; // slope of d_max; d_max = d_m * average(x) + d_c
//	static const int DISPERSION_ANALYSIS_MIN_MEASUREMENTS = 10; // minimal number of measurements at a given position to calculate meaningful standard deviation
//	static const int PLM_BUILDING_MIN_MEASUREMENTS = 20000; // number of measurements needed for each event type to build the PLM / dispersion analysis / linear correlation analysis
	static constexpr double PLM_BUILDING_WC_QUANTILE = 0.9; // quantile*100 for the worst-case analysis of measured values at a given pos when building the latency model

//	static const int PLM_INSTANCE_MONITOR_START_AFTER_MILLISECONDS = 1200000; // value for evaluation run
//	static const int PLM_INSTANCE_MONITOR_END_AFTER_MILLISECONDS = 2400000; // value for evaluation run
	static const int PLM_INSTANCE_MONITOR_START_AFTER_MILLISECONDS = 60000; // start measuring processing latency in op instances after that amount of milliseconds (warmup phase)
	static const int PLM_INSTANCE_MONITOR_END_AFTER_MILLISECONDS = 300000; // end measuring processing latency in op instances after that amount of milliseconds

	static const int PARAMETER_PREDICTION_TSA_NUMBER_OF_MEASUREMENTS = 10; // number of measurements (finished selections) used in the parameter prediction time series analysis

//	static const int BATCHING_MAX_KNOWN_EVENTS = 5000000; // value for evaluation run
	static const int BATCHING_MAX_KNOWN_EVENTS = 100000; // at this number of known events in a batch, a new batch shall be automatically started; else, some instances never get assigned any work.

	// reactive scheduling parameters
	static const long LATENCY_MEASUREMENT_NUMBER = 100; // size of measurements window of which a percentile is the current latency
	static constexpr double LATENCY_MEASUREMENT_PERCENTILE = 0.95; // percentile of the window values that is regarded as the current latency

	/*-----------------------------------------------------------------
	 * Definitions for Video and Image Processing
	 * ----------------------------------------------------------------
	 */
	static const int VIDEO_FRAME_WIDTH = 1280; // width of a video frame
	static const int VIDEO_FRAME_HEIGHT = 720; // height of a video frame

	static const int FACE_WIDTH = 640; // width of face image
	static const int FACE_HEIGHT = 480; // height of face image

	/*
	 * enums
	 */
	enum PositionLatencyDependency
	{
		LINEAR, NONLINEAR, NONE
	};

};

}

#endif /* HEADER_UTIL_CONSTANTS_HPP_ */
