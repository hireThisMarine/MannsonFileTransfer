/*! ============================================================================
  @file config.h

  @author Maximilian Manndorff
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project 3
  @date 02.07.2010

  @brief 
    Declarations for the configuration wizard.

============================================================================= */
#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>

namespace CS260
{
  namespace Config
  {
    enum { NOT_SET = -1 };

    /*! Holds and maintains configuration settings during runtime */
    struct Configuration
    {
      Configuration(void) : serverPort(NOT_SET), clientUDPport(NOT_SET) { };

      std::string serverIP;
      unsigned serverPort;
      unsigned clientUDPport;
      std::string directoryToStoreReceivedFiles;
      std::string directoryToShareFilesFrom;
    };

    /*! Configuration Error throw object */
    class Error
    {
    public:
      Error(std::string ConfigFile, std::string File, unsigned Line);
      std::string GetFile(void);
      unsigned GetLine(void);
      std::string GetConfigFile(void);
    private:
      std::string m_File;
      unsigned m_Line;
      std::string m_ConfigFile;
    };

    Configuration ReadConfigFile(std::string FileLocation);
  }
}

#endif