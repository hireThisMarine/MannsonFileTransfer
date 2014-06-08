#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

#define MAX_PORT 65535
#define MIN_PORT 1

bool IsValid(unsigned port);
bool IsValid(std::string IP);

#endif