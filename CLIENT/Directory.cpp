/*! ============================================================================
  @file Directory.cpp

  @author Maximilian Manndorff
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project A3
  @date 03.07.2010

  @brief 
    Scans directories for files and collects them in a vector.

============================================================================= */
#include "Directory.h"
#include <string>
#include <vector>
#include <sys/types.h>
#include "dirent.h"

namespace CS260
{
namespace Utils
{
  Directory::Directory(std::string dir) : m_directory(dir) { Scan(); }

  std::vector<std::string> Directory::GetFiles(void) const { return m_files; }

  std::vector<std::string> Directory::Scan(void)
  {
    DIR* dp;
    struct dirent *dirp;

    if ((dp = opendir(m_directory.c_str())) == NULL) 
      throw Error(m_directory, __FILE__, __LINE__);

    while ((dirp = readdir(dp)) != NULL) 
    {
      std::string filename = dirp->d_name;
      if (filename == "." || filename == "..") continue;
      m_files.push_back(std::string(dirp->d_name));
    }

    closedir(dp);
    return m_files;
  }

  // Directory Exception Specifications
  Directory::Error::Error(std::string dir, std::string file, unsigned line)
    : m_directory(dir), m_file(file), m_line(line) { }
  std::string Directory::Error::getDirectory(void) const { return m_directory; }
  std::string Directory::Error::getFile(void) const { return m_file; }
  unsigned Directory::Error::getLine(void) const { return m_line; }


  /*! ----------------------------------------------------------------------------
    @brief 
      Overloading the insertion operator to allow output of directories.
  
    @param os   The output stream.
    @param dir  The directory to output.
    
    @return     The modified output stream.
     
    @date 03.07.2010
  
  ----------------------------------------------------------------------------- */
  std::ostream& operator<<(std::ostream& os, Directory const& dir)
  {
    std::vector<std::string> files = dir.GetFiles();
    int size = files.size();

    for (int i = 0; i < size; ++i)
      os << files[i] << std::endl;

    return os;
  }

} // namespace Utils
} // namespace CS260

