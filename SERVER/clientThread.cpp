/*! ============================================================================
  @file clientThread.cpp

  @date 06.07.2010

  @brief 
    Handles each client in a separate thread.

============================================================================= */
#include "winsock2.h"
#include <windows.h>
#include <iostream>
#include <vector>

#include "clientThread.h"
#include "server.h"
#include "commandScan.h"
#include "utilities.h"

// #define LOUD_SERVER

static void cmdIdentify(ServerHandle* Handle, std::string serializedID);
static void cmdAddFiles(ServerHandle* Handle, std::string SerializedFileList);
static void cmdShowFiles(ServerHandle* Handle);
static void cmdPortJump(ServerHandle* Handle, std::string newPort);
static void cmdFileDownload(ServerHandle* Handle, std::string file);

/*! ----------------------------------------------------------------------------
  @brief 
    Handles a newly spawned thread which handles one client only.

  @param srvHnd   The handle to the core (shared) server architecture
  
  @return         WINAPI
----------------------------------------------------------------------------- */
DWORD WINAPI clientThread(void* srvHnd)
{
  ServerHandle* Handle = static_cast<ServerHandle*>(srvHnd);
  bool connected = true;
  char Buffer[BUFSIZE];
  int BytesRead = 0;
  SOCKET MySocket = Handle->m_Socket;

  SecureZeroMemory(Buffer, BUFSIZE - 1);

  // Unblock Socket
  u_long blocking = 1;
  ioctlsocket(MySocket, FIONBIO, &blocking);

  while (connected)
  {
    // Receive new data
    BytesRead = recv(MySocket, Buffer, BUFSIZE, 0);

    // Error & Disconnect Check
    if (BytesRead == SOCKET_ERROR || BytesRead == 0)
    {
      int nError = WSAGetLastError();
      if (nError != WSAEWOULDBLOCK) 
        break;
    }

    if (!strlen(Buffer)) { continue; }

#ifdef LOUD_SERVER
    std::cout << Buffer << std::endl;
#endif

    // Receive Serialized list of files...
    std::string commandExtract;
    int command = scanForCommands(Buffer, commandExtract);

    if      (command == CMD_DISCONNECT)      // Disconnect user
      break;
    else if (command == CMD_IDENTIFY)
      cmdIdentify(Handle, commandExtract);
    else if (command == CMD_ADDFILE)         // Receive serialized list of files
      cmdAddFiles(Handle, commandExtract);
    else if (command == CMD_SHOWFILES)       // Send list of available files
      cmdShowFiles(Handle);
    else if (command == CMD_PORTJUMP)        // Communicate client port jump
      cmdPortJump(Handle, commandExtract);
    else if (command == CMD_FILEDOWNLOAD)    // File download
      cmdFileDownload(Handle, commandExtract);
    else if (command == CMD_INVALIDCMD)      // Invalid command
    {
      std::string transit = "Unknown Command\n";
      send(MySocket, transit.c_str(), transit.size() + 1, 0);
    }

    Buffer[0] = '\0';
  }

  // Shut down thread
  EnterCriticalSection(Handle->m_CritSection);
  Handle->m_FileLibrary->RemoveFiles(MySocket); // Remove user files
  LeaveCriticalSection(Handle->m_CritSection);

  std::cout << "User at Socket " << MySocket
            << " disconnected with code " << WSAGetLastError() << std::endl;

  // Shutdown
  shutdown(MySocket, SD_BOTH);
  closesocket(MySocket);
  delete Handle;
  return 0;
}

/*! ----------------------------------------------------------------------------
  @brief 
    Client identification procedure

  @param Handle         Server handle
  @param serializedID   The serialized client IP and Port
  
  @return               void
----------------------------------------------------------------------------- */
void cmdIdentify(ServerHandle* Handle, std::string serializedID)
{
  unsigned explode = serializedID.find_first_of(':');
  std::string IP = serializedID.substr(0, explode);
  std::string Port = serializedID.substr(explode + 1, serializedID.size());
  User usr;
  usr.m_IP = IP;
  usr.m_Port = atoi(Port.c_str());

  if (IsValid(usr.m_IP) && IsValid(usr.m_Port))
  {
    EnterCriticalSection(Handle->m_CritSection);
    (*Handle->m_UserMap)[Handle->m_Socket] = usr;
    LeaveCriticalSection(Handle->m_CritSection);
  }
}

/*! ----------------------------------------------------------------------------
  @brief 
    Adds files from the client to the server.

  @param Handle               Server handle
  @param SerializedFileList   Serialized list of files to add
  
  @return                     void
----------------------------------------------------------------------------- */
void cmdAddFiles(ServerHandle* Handle, std::string SerializedFileList)
{
  while (SerializedFileList.size())
  {
    unsigned start = SerializedFileList.find_first_of('@', 0);
    unsigned end = SerializedFileList.find_first_of('@', start + 1);
    std::string file = SerializedFileList.substr(start + 1, end - 1);
    SerializedFileList.erase(start, end - start);

    EnterCriticalSection(Handle->m_CritSection);
    Handle->m_FileLibrary->AddFile(file, Handle->m_Socket);
    LeaveCriticalSection(Handle->m_CritSection);
  }
}

/*! ----------------------------------------------------------------------------
  @brief 
    Sends a list of all files on the server to the client.

  @param Handle   The server handle
  
  @return         void
----------------------------------------------------------------------------- */
void cmdShowFiles(ServerHandle* Handle)
{
  std::vector<std::string> files;
  std::string FileString;
  EnterCriticalSection(Handle->m_CritSection);
  files = Handle->m_FileLibrary->GetFiles();
  LeaveCriticalSection(Handle->m_CritSection);

  if (files.size())
  {
    for (unsigned i = 0; i < files.size(); ++i)
    {
      FileString.append(files[i]);
      if ((i + 1) % 3) { FileString.append("\t\t");}
      else FileString.append("\n");
    }
    send(Handle->m_Socket, FileString.c_str(), FileString.size() + 1, 0);
  }
  else
  {
    std::string transit = "No Files available at this time.\n";
    send(Handle->m_Socket, transit.c_str(), transit.size() + 1, 0);
  }
}

/*! ----------------------------------------------------------------------------
  @brief 
    Registers a UDP port jump of a client.

  @param Handle   The server handle.
  @param newPort  The new UDP port the client is listening on.
  
  @return         void
----------------------------------------------------------------------------- */
void cmdPortJump(ServerHandle* Handle, std::string newPort)
{
  unsigned port = atoi(newPort.c_str());

  if (port > 0 && port <= MAXPORT)
  {
    EnterCriticalSection(Handle->m_CritSection);
    (*Handle->m_UserMap)[Handle->m_Socket].m_Port = atoi(newPort.c_str());
    LeaveCriticalSection(Handle->m_CritSection);
  }
}

/*! ----------------------------------------------------------------------------
  @brief 
    Registers a file download request and synchronizes both peer's connection.

  @param Handle   The server handle.
  @param file     The file that was requested.
  
  @return 
----------------------------------------------------------------------------- */
void cmdFileDownload(ServerHandle* Handle, std::string file)
{
  EnterCriticalSection(Handle->m_CritSection);
  SOCKET Sender = Handle->m_FileLibrary->FindFile(file);
  LeaveCriticalSection(Handle->m_CritSection);

  if (!Sender || Sender == Handle->m_Socket)
  {
    std::string transit = "Can't get that file.\n";
    send(Handle->m_Socket, transit.c_str(), transit.size() + 1, 0);
    return;
  }

  EnterCriticalSection(Handle->m_CritSection);
  std::string SenderIp = (*Handle->m_UserMap)[Sender].m_IP;
  unsigned SenderPort = (*Handle->m_UserMap)[Sender].m_Port;
  std::string  ReceiverIp = (*Handle->m_UserMap)[Handle->m_Socket].m_IP;
  unsigned ReceiverPort = (*Handle->m_UserMap)[Handle->m_Socket].m_Port;
  LeaveCriticalSection(Handle->m_CritSection);

  char buf[10] = {0};
  sprintf(buf, "%i", SenderPort);
  std::string transit = "\\get:" + file + "@" + SenderIp + ":" + buf;
  send(Handle->m_Socket, transit.c_str(), transit.size() + 1, 0);

  sprintf(buf, "%i", ReceiverPort);
  transit.clear();
  transit = "\\send:" + file + "@" + ReceiverIp + ":" + buf;
  send(Sender, transit.c_str(), transit.size() + 1, 0);
}