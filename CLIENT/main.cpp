#include <string>
#include <map>

#include "config.h"
#include "tcpconnection.h"
#include "Directory.h"
#include "conio.h"
#include "p2pcon.h"
#include "utilities.h"

int main(void)
{
  std::cout << "P2P Client loading..." << std::endl;

  char Buffer[512];
  SecureZeroMemory(Buffer, 512);
  conioInit();

  // SHARED BETWEEN THREADS:
  CRITICAL_SECTION Section;
  InitializeCriticalSection (&Section);
  std::vector<HANDLE> ThreadHandles;
  std::map<std::string, TrnsfrInf> Transfers;

  try 
  { 
    CS260::Config::Configuration CFG 
      = CS260::Config::ReadConfigFile("config.cfg");
    CS260::Con::TCPConnection ServerConnection;
    CS260::Utils::Directory Shared(CFG.directoryToShareFilesFrom);

    ServerConnection.Unblock();
    ServerConnection.Connect(CFG.serverIP, CFG.serverPort);

    // START RECEIVER
    ReceiverInfo* con = new ReceiverInfo;
    con->CritSec = &Section;
    con->Port = CFG.clientUDPport;
    con->Transfers = &Transfers;
    con->shutDownSequence = false;
    con->DownloadDir = CFG.directoryToStoreReceivedFiles;
    HANDLE thisThread = CreateThread(NULL, 0, Receiver, (void*)con, 0, NULL);
    if (thisThread) ThreadHandles.push_back(thisThread);

    // Identify Client with server
    std::string transit = "\\identify ";
    transit += ServerConnection.GetLocalIP(); 
    transit += ":";
    sprintf(Buffer, "%i", CFG.clientUDPport);
    transit += Buffer;
    Buffer[0] = '\0';
    ServerConnection.send(transit.c_str(), transit.size() + 1);

    Sleep(1000); // enforce processing

    // Register shared files with server
    std::vector<std::string> files = Shared.GetFiles();
    transit.clear();
    transit = "\\addfiles ";
    for (unsigned i = 0; i < files.size(); ++i)
      transit.append("@" + files[i]);
    ServerConnection.send(transit.c_str(), transit.size() + 1);

    std::cout << "Hit 'C' to enter commands." << std::endl;
    std::cout << "\n\\show\t\tShow list of available downloads." << std::endl;
    std::cout << "\\download file\tDownload \"file\"" << std::endl;
    std::cout << "\\status\t\tShow progress of all downloads" << std::endl;
    std::cout << "\\disconnect\tDisconnect from server" << std::endl;
    std::cout << "\\quit\t\tShut down the program." << std::endl;
    std::cout << "\n\n" << std::endl;

    while (ServerConnection.IsConnected())
    {
      inputUpdate();
      ServerConnection.recv(Buffer, 500);

      // Handle Commands
      std::string command = Buffer;
      if (command.size() && command[0] == '\\')
      {
        unsigned cmd_end = command.find_first_of(':');
        std::string cmd = command.substr(0, cmd_end);

        if (cmd == "\\get")
        {
          unsigned file_end = command.find_first_of('@', cmd_end + 1);
          std::string FileName = command.substr(cmd_end + 1, file_end - cmd_end - 1);
          unsigned ip_end = command.find_first_of(':', file_end + 1);
          std::string IP = command.substr(file_end + 1, ip_end - file_end -  1);
          unsigned Port = atoi(command.substr(ip_end + 1, command.size() + 1).c_str());

          if (!IsValid(IP) || !IsValid(Port)) continue;

          TrnsfrInf info;
          info.LastPackage = 0;
          info.TotalPackages = 0;
          info.AvrgRTT = timeGetTime();
          info.TimeRemaining = 0;
          info.SndrPort = Port;
          info.SndrIp = IP;
          Transfers[FileName] = info;
          std::cout << "Receiving file " << FileName << std::endl;
        }
        else if (cmd == "\\send")
        {
          SenderInfo* con = new SenderInfo;
          con->CritSec = &Section;
          con->ShareDir = CFG.directoryToShareFilesFrom;

          unsigned file_end = command.find_first_of('@', cmd_end + 1);
          con->FileName = command.substr(cmd_end + 1, file_end - cmd_end - 1);
          unsigned ip_end = command.find_first_of(':', file_end + 1);
          con->IP = command.substr(file_end + 1, ip_end - file_end -  1);
          con->Port = atoi(command.substr(ip_end + 1, command.size() + 1).c_str());

          //Spawn new thread that sends file
          HANDLE thisThread = CreateThread(NULL, 0, SendFile, (void*)con, 0, NULL);
          if (thisThread) ThreadHandles.push_back(thisThread);
        }
        
        Buffer[0] = '\0';
      }

      if (strlen(Buffer)) std::cout << Buffer << std::endl;

      if (isTriggered('c'))
      {
        char input[124] = {0};

        std::cout << " >> ";
        std::cin.getline(input, 123, '\n');

        if (input[0] == '\\')
        {
          std::string command = input;
          if (command == "\\status")
          {
            EnterCriticalSection(&Section);
            std::map<std::string, TrnsfrInf> info = Transfers;
            LeaveCriticalSection(&Section);

            if (info.size())
            {
              for (std::map<std::string, TrnsfrInf>::iterator i = info.begin(); i != info.end(); ++i)
              {
                std::string filename = i->first;
                TrnsfrInf info = i->second;
                double status = ((double)info.LastPackage / (double)info.TotalPackages) * 100.0;
                std::cout.precision(02);
                std::cout.setf(std::ios_base::fixed, std::ios::floatfield);
                std::cout << filename << " (" 
                          << (((double)info.TotalPackages * (double)UDP_PACK_DATA_SIZE)) / 1048576.0
                          << " Mb): ";
                std::cout.width(02);
                
                // Compute time remaining
                int sec = info.TimeRemaining / 1000;
                int min = sec / 60;
                sec %= 60;
                int hr = min / 60;
                min %= 60;
                int day = hr / 24;
                hr %= 24;

                std::cout << status << "%\tETA: "
                          << day << "days "
                          << hr << "hrs " 
                          << min << "mins " 
                          << sec << "sec"
                          << std::endl;
              }
            }
            else
              std::cout << "No Downloads are running." << std::endl;

            input[0] = 0;
          }
          else if (command == "\\quit")
          {
            std::cout << "Good bye." << std::endl;
            system("PAUSE");
            return 0;
          }
        }

        if (strlen(input))
          ServerConnection.send(input, strlen(input) + 1);
      }

      Buffer[0] = '\0';
    }

    EnterCriticalSection(&Section);
    con->shutDownSequence = true;
    LeaveCriticalSection(&Section);

    std::cout << "Connection to server lost, waiting for transfers to complete." 
              << "(CTRL+C to quit)"
              << std::endl;

    // WAIT FOR ALL THREADS TO SIGNAL GREEN
    for (unsigned i = 0; i < ThreadHandles.size(); ++i)
    {
      WaitForSingleObject(ThreadHandles[i], INFINITE);
    }

    std::cout << "Good bye." << std::endl;

////////////////////////////////////////////////////////////////////////////////
// Error handling
  }
  catch (CS260::Config::Error e)
  {
    std::cout << "Configuration failed: '" <<  e.GetConfigFile() << "' "
              << "missing or corrupted in " << e.GetFile() << " at line"
              << e.GetLine() << std::endl;
  }
  catch (CS260::Con::TCPConnection::Error e)
  {
    std::cout << "Server Connection Error " << e.GetWSAError()
              << " in file " << e.GetFile() << " at line " << e.GetLine()
              << std::endl;
    std::cout << "Failed with message: '" << e.GetMessage() << "' "
              << "while attempting to connect to " << e.GetIP() 
              << ":" << e.GetPort()
              << std::endl;
  }
  catch (CS260::Utils::Directory::Error e)
  {
    std::cout << "File Error: Can not find or open directory " 
              << e.getDirectory() << " in file " << e.getFile() << " at line "
              << e.getLine() << std::endl;
  }
  catch (const char* e)
  {
    std::cout << "Abnormal stop: " << e << std::endl;
  }

  system("PAUSE");
  return 0;
}