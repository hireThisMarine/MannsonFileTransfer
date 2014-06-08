#include "utilities.h"

bool IsValid(unsigned port)
{
  if (port >= MIN_PORT && port <= MAX_PORT)
    return true;

  return false;
}

bool IsValid(std::string ipaddr)
{
  unsigned int n1, n2, n3, n4;

  if (sscanf (ipaddr.c_str(), "%u.%u.%u.%u", &n1, &n2, &n3, &n4) != 4) 
    return false;

  if ((n1 <= 255) && (n2 <= 255) && (n3 <= 255) && (n4 <= 255)) 
    return true;

  return false;
}

