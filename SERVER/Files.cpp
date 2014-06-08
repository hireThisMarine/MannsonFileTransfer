#include <vector>
#include <string>

#include "Files.h"
#include "utilities.h"


std::vector<std::string> Files::GetFiles(void) const
{
  std::vector<std::string> ret; //Why did we go with the name "ret"?

  // Build complete list of files
  for (unsigned i = 0; i < m_Files.size(); ++i)
  {
    for (unsigned j = 0; j < m_Files[i].m_usrFiles.size(); ++j)
    {
      ret.push_back(m_Files[i].m_usrFiles[j]);    
    }
  }
  return ret;
}

void Files::AddFiles(std::vector<std::string> files, SOCKET client)
{
  UserFiles newUsr;
  newUsr.m_host = client;
  newUsr.m_usrFiles = files;

  m_Files.push_back(newUsr);
}

void Files::AddFile(std::string file, SOCKET client)
{
  for (unsigned i = 0; i < m_Files.size(); ++i)
    if (m_Files[i].m_host == client)
    {
      m_Files[i].m_usrFiles.push_back(file);
      return;                                                            // EXIT
    }

  // If runtime gets here we know that the Host has not added any files at all
  // yet (the client attempting to add files does not exist in the vector yet)
  std::vector<std::string> usrFiles;
  usrFiles.push_back(file);
  AddFiles(usrFiles, client);
}

void Files::RemoveFiles(SOCKET client)
{
  std::vector<UserFiles>::iterator i = m_Files.begin();

  for (; i != m_Files.end(); ++i)
  {
    if (i->m_host == client)
    {
      m_Files.erase(i);
      break;
    }
  }
}

SOCKET Files::FindFile(std::string filename)
{
  //TODO: This hurts performance, consider a different architecture
  for (unsigned i = 0; i < m_Files.size(); ++i)
  {
    for (unsigned j = 0; j < m_Files[i].m_usrFiles.size(); ++j)
    {
      if (m_Files[i].m_usrFiles[j] == filename)
      {
        return m_Files[i].m_host;
      }
    }
  }

  return NULL;
}

