/*
 * Constants.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: sload
 */

#include "../../header/util/Constants.hpp"

using namespace std;
namespace util {

const string Constants::ADAPTATION_MODE_CPU = "cpu"; // adapt the degree based on CPU utilization of the hosts
const string Constants::ADAPTATION_MODE_QT = "qt"; // the qt adaptation mode is implementing our approach
const string Constants::ADAPTATION_MODE_NONE = "none"; // the parallelization degree is not adapted, but remains static

/*-------------------------------
 * Source Play: Scenarios
 * ------------------------------
 */
const string Constants::PLAY_SCENARIO_PLAYLOG = "playlog"; // play a log file that contains events (each line one event)
const string Constants::PLAY_SCENARIO_FACES = "faces"; // play images that contain faces
const string Constants::PLAY_SCENARIO_FACE_RECOGNITION_QUERIES = "facerec"; // play face recognition queries
const string Constants::PLAY_SYNTHETIC_FACES = "faces_synthetic"; // plays synthetic face scenario (not real data)

/* -------------------------
 * Allowed Operator Types
 * ------------------------
 */
const string Constants::OPERATOR_DUMMY = "dummy_operator"; // does nothing, just takes events and deletes them. sends no ACKs back
const string Constants::OPERATOR_TRAFFIC = "traffic_operator"; // monitors the no passing zone
const string Constants::OPERATOR_FACE_RECOGNITION = "face_recognition_operator"; // recognizes persons in face pictures
const string Constants::FACE_FILE_EXTENSION = "png"; // format of files to save pictures of faces
const string Constants::OPERATOR_EXPONENTIAL_WAIT = "exponential_wait_operator";
const string Constants::OPERATOR_UNIFORM_WAIT = "uniform_wait_operator";
const string Constants::OPERATOR_PARETO_WAIT = "pareto_wait_operator";
const string Constants::OPERATOR_LOGNORMAL_WAIT = "lognormal_wait_operator";
const string Constants::OPERATOR_TREE = "tree_operator";
const string Constants::OPERATOR_POWERSET = "powerset_operator";

/*-------------------------
 * Predicates
 * ------------------------
 */
const string Constants::PREDICATE_TUPLE_WINDOW = "tu"; // the tuple-based sliding window
const string Constants::PREDICATE_TIME_WINDOW = "ti"; // the time-based sliding window
const string Constants::PREDICATE_TRAFFIC = "traffic"; // predicate for traffic monitoring operator
const string Constants::PREDICATE_FACE_RECOGNITION = "face_recognition"; // predicate for the face recognition operator

/*-----------------
 * Events
 * - Note: Event identifier should be a unique string that can be casted to an Integer
 * ----------------
 */
const string Constants::EVENT_TYPE_DEMAND_STATUS = "0"; // demand-response events
const string Constants::EVENT_TYPE_GRID_STATUS = "1"; // report: energy production and consumption aggregated
const string Constants::EVENT_TYPE_VEHICLE_POS_1 = "2"; // vehicle detected at pos_1
const string Constants::EVENT_TYPE_VEHICLE_POS_2 = "3"; // vehicle detected at pos_2
const string Constants::EVENT_TYPE_VEHICLE_VIOLATOR = "4"; // vehicle detected to have violated the traffic rules
//const char Constants::EVENT_TYPE_SIMPLE_VALUE = '5'; //just a simple double value in the contents "v=double"
const string Constants::EVENT_TYPE_POWERSET_EVENT = "9"; // power set events consist of a set id (= selection id, id tuple) and a content char
const char Constants::EVENT_TYPE_STREAM_ENDS = 232; // the stream has finished, and the components are asked to shut down. 232=phi

/*
 * value delimiter in  events
 */
const string Constants::EVENT_TYPE_SIMPLE_VALUE_CONTENT_1_KEY = "v"; //key of first content
const string Constants::EVENT_TYPE_VEHICLE_CONTENT_1_KEY = "p"; // = number plate

/*
 * other events
 */
const string Constants::EVENT_TYPE_FACE = "6";
const string Constants::EVENT_TYPE_FACE_CONTENT_KEY = "f";
const string Constants::EVENT_TYPE_FACE_RECOGNITION_QUERY = "7";
const string Constants::EVENT_TYPE_FACE_RECOGNITION_QUERY_CONTENT_KEY = "q";
const string Constants::EVENT_TYPE_FACE_RECOGNITION_QUERY_FACES_OF_FRIEND_DELIMITER = "|"; //separate the marshalled faces of a friend (no base64 character)
const string Constants::EVENT_TYPE_FACE_MATCH = "8";
const char Constants::EVENT_TYPE_PULSE = '9';

/*
 * content is delimited separately for a simple implementation of the parsing
 */
const char Constants::CONTENT_DELIMITER = ';'; // delimiter of different content key/value pairs
const char Constants::CONTENT_KEY_VALUE_DELIMITER = ':'; // delimiter between key and value of content

/*-----------------
 * Marshalling and Unmarshalling
 * ----------------
 */
const char Constants::PARAMETER_DELIMITER = ','; // delimiter of different parameters in its string representation
const char Constants::PARAMETER_KEY_VALUE_DELIMITER = '!'; // delimiter between key and value in event string representation"

/*--------------------------
 * Scheduling
 * ---------------------------
 */
const string Constants::SCHEDULING_ROUND_ROBIN = "rr"; // classic round robin
const string Constants::SCHEDULING_FIXED_BATCH = "fixed_scheduling"; // fixed batch size scheduling
const string Constants::SCHEDULING_REACTIVE = "reactive_scheduling"; // scheduling reactive to latency measurements
const string Constants::SCHEDULING_PREDICTIVE = "predictive_scheduling"; // predictive scheduling
const string Constants::SCHEDULING_BOUNDARY = "boundary_scheduling"; // batch scheduling based on worst-and best-case analysis
const string Constants::SCHEDULING_AVERAGE_BOUNDARY = "avg_boundary_scheduling"; // batch scheduling based on average case analysis

}

