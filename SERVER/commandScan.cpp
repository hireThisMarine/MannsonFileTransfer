#include "commandScan.h"

int scanForCommands (std::string input, std::string& extract)
{
  int status = CMD_NOCMD;
  extract.clear ();

  if (input.size() && input[0] == '\\') // Commands begin with backslash
  {
    int cmd_start = 0;
    int cmd_end = input.find_first_of (' ');

    // note that cmd_end might be npos. In this case the command finding
    // conditionals below still work due to the way the string object works
    std::string command = input.substr(cmd_start, cmd_end);

    if (cmd_end != std::string::npos)
    {
      ++cmd_end;
      extract = input.substr (cmd_end, input.size ());
    }

    if      (command == "\\identify")
      status = CMD_IDENTIFY;
    else if (command == "\\addfiles")
      status = CMD_ADDFILE;
    else if (command == "\\show")
      status = CMD_SHOWFILES;
    else if (command == "\\portjump")
      status = CMD_PORTJUMP;
    else if (command == "\\download")
      status = CMD_FILEDOWNLOAD;
    else if (command == "\\disconnect")
      status = CMD_DISCONNECT;
    else
    {
      extract.clear(); // Just to be sure
      status = CMD_INVALIDCMD;
    }
  }

  return status;
}
