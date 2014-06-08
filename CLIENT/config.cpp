/*! ============================================================================
  @file config.cpp

  @author Maximilian Manndorff
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project 3
  @date 02.07.2010

  @brief 
    Implementation for configuration wizard. Assumes responsibility for
    reading and writing configuration related properties.

============================================================================= */
#include "config.h"

namespace CS260
{
  namespace Config
  {
    /*! ------------------------------------------------------------------------
      @brief 
        Attempts to read a given configuration file. Should the reading process
        fail an exception of type CS260::Config::Error will be thrown.
    
      @param FileLocation   
      
      @return 
       
      @date 02.07.2010
    
    ------------------------------------------------------------------------- */
    Configuration ReadConfigFile(std::string FileLocation)
    {
      FILE* fp = fopen(FileLocation.c_str(), "rt");
      if (!fp) throw Error(FileLocation, __FILE__, __LINE__);
      Configuration config;
      char buffer[256];

      if (!fscanf(fp, "serverIP %s\n", &buffer)) 
        throw Error(FileLocation, __FILE__, __LINE__);
      config.serverIP = buffer;
      if (!fscanf(fp, "serverPort %i\n", &config.serverPort)) 
        throw Error(FileLocation, __FILE__, __LINE__);
      if (!fscanf(fp, "clientUDPport %i\n", &config.clientUDPport))
        throw Error(FileLocation, __FILE__, __LINE__);
      if (!fscanf(fp, "directoryToStoreReceivedFiles %s\n", &buffer))
        throw Error(FileLocation, __FILE__, __LINE__);
      config.directoryToStoreReceivedFiles = buffer;
      if (!fscanf(fp, "directoryToShareFilesFrom %s", &buffer))
        throw Error(FileLocation, __FILE__, __LINE__);
      config.directoryToShareFilesFrom = buffer;

      fclose(fp);

      return config;
    }

    /*! ------------------------------------------------------------------------
      @brief 
        Constructor for Configuration Exception Error
    
      @param ConfigFile   The configuration file the error is related to
      @param File         The File in which the error occurred
      @param Line         The Line number where the error occurred
      
      @return             void
       
      @date 02.07.2010
    
    ------------------------------------------------------------------------- */
    Error::Error(std::string ConfigFile, std::string File, unsigned Line)
      : m_ConfigFile(ConfigFile), m_File(File), m_Line(Line)
    { }

    /*! ------------------------------------------------------------------------
      @brief 
        Accessor for the file in which the error occurred.
    
      @return   Name of the source file in which the error occurred.
       
      @date 02.07.2010
    
    ------------------------------------------------------------------------- */
    std::string Error::GetFile(void) { return m_File; }

    /*! ------------------------------------------------------------------------
      @brief 
        Accessor to the Line number at which the error occurred.
    
      @return  The line number at which the error occurred.
       
      @date 02.07.2010
    
    ------------------------------------------------------------------------- */
    unsigned Error::GetLine(void) { return m_Line; }

    /*! ------------------------------------------------------------------------
      @brief 
        Accessor to the configuration file that is related to the error.
    
      @return   The location and name of the configuration file that is related
                to the error.
       
      @date 02.07.2010
    
    ------------------------------------------------------------------------- */
    std::string Error::GetConfigFile(void) { return m_ConfigFile; }

  }
}

