/*! ============================================================================
  @file directory.h

  @author Maximilian Manndorff
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project 3
  @date 03.07.2010

  @brief 
    Manages the download and upload directories.

============================================================================= */
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>

namespace CS260
{
namespace Utils
{
  class Directory
  {
  private: // DATA
    std::string                m_directory;
    std::vector<std::string>   m_files;


  public: // AUTOS
    Directory(std::string dir);
    std::vector<std::string> GetFiles(void) const;
    std::vector<std::string> Scan(void);


  public: // AGGREGATE Error
    class Error
    {
    private: // DATA
      std::string m_file;
      unsigned    m_line;
      std::string m_directory;


    public: // AUTOS
      Error(std::string dir, std::string file, unsigned line);


    public: // ACCESSORS
      std::string getDirectory(void) const;
      std::string getFile(void) const;
      unsigned getLine(void) const;
    };
  };

  std::ostream& operator<<(std::ostream& os, Directory const& dir);

} // namespace Utils
} // namespace CS260

#endif
