#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#define QUEUE_CAPACITY        32
#define SIMULATION_TIME       100
#define CLOCK_CHECK           2000 // To avoid writing at same clock cycle
#define UNIVERSAL_ISSUE_TIME  2000
#define SEPARATE_ROWS         false

int CPU = 2;
std::string BaseFilename = "t";
std::string InputDir = "Input/";

#endif