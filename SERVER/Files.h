#ifndef FILES_H
#define FILES_H

#include <string>
#include <vector>
#include "winsock2.h"


class Files
{
public:
  // METHODS
  std::vector<std::string> GetFiles(void) const;
  void AddFiles(std::vector<std::string> files, SOCKET client);
  void AddFile(std::string file, SOCKET client);
  void RemoveFiles(SOCKET client);
  SOCKET FindFile(std::string filename);

private:
  // AGGREGATES
  struct UserFiles
  {
    SOCKET                   m_host;
    std::vector<std::string> m_usrFiles;
  };

  // DATA
  std::vector<UserFiles> m_Files;
};

#endif