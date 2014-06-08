#ifndef COMMANDSCAN_H
#define COMMANDSCAN_H

#include <string>

enum 
{
  CMD_NOCMD,
  CMD_INVALIDCMD,
  CMD_IDENTIFY,
  CMD_ADDFILE,
  CMD_SHOWFILES,
  CMD_PORTJUMP,
  CMD_FILEDOWNLOAD,
  CMD_DISCONNECT
};

int scanForCommands (std::string input, std::string& extract);

#endif