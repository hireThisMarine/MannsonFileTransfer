#include "winsock2.h"
#include <iostream>
#include <windows.h>
#include <cmath>

#include "p2pcon.h"

DWORD WINAPI SendFile(void* handle)
{
  SenderInfo* Handle = (SenderInfo*)handle;

  EnterCriticalSection(Handle->CritSec);
  std::cout << "Started sending file '" << Handle->FileName << "' "
            << "to " << Handle->IP << ":" << Handle->Port << std::endl;
  std::string fileloc = Handle->ShareDir + "/" + Handle->FileName;
  LeaveCriticalSection(Handle->CritSec);

  sockaddr_in local;
  hostent* localhost = gethostbyname("");
  local.sin_family = AF_INET;
  local.sin_port = htons(0);
  local.sin_addr.s_addr = inet_addr(inet_ntoa(*(in_addr*)*localhost->h_addr_list));
  SOCKET LOCAL = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (LOCAL == INVALID_SOCKET) { delete handle; return -1; }
  int ret = bind(LOCAL,(SOCKADDR*)&local, sizeof(local));

  sockaddr_in remote;
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = inet_addr(Handle->IP.c_str());
  remote.sin_port = htons(Handle->Port);

  unsigned current_package_id = 1; // Which part is currently sent, initial 1
  unsigned maximum_package_id;     // Of how many parts

  EnterCriticalSection(Handle->CritSec);
  FILE* fp = fopen(fileloc.c_str(), "rb");
  if (!fp) { delete handle; return -1; }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);
  LeaveCriticalSection(Handle->CritSec);

  maximum_package_id = (unsigned)ceil((float)(size / (float)(UDP_PACK_DATA_SIZE)));

  while (current_package_id < maximum_package_id + 1)
  {
    // Prepare Package
    UDP_PACKAGE pack_out;
    htonl(pack_out.part = current_package_id);
    htonl(pack_out.total = maximum_package_id);
    unsigned offset = (current_package_id - 1) * UDP_PACK_DATA_SIZE;
    EnterCriticalSection(Handle->CritSec);
    FILE* fp = fopen(fileloc.c_str(), "rb");
    if (!fp) { delete handle; return -1; }
    fseek(fp, offset, 0);
    unsigned packsize = fread(pack_out.data, sizeof(char), UDP_PACK_DATA_COUNT, fp);
    fclose(fp);
    LeaveCriticalSection(Handle->CritSec);
    htonl(pack_out.size = packsize);

    // Send Package
    ret = sendto(LOCAL, (char*)&pack_out, sizeof(UDP_PACKAGE), 0, (SOCKADDR*)&remote, sizeof(remote));

    // Wait for confirmation and resend if necessary
    unsigned ack = 0;
    DWORD start = timeGetTime();
    int attempt = 0;
    int size = sizeof(remote);
    ret = 0;

    while (!ret || ack != pack_out.part)
    {
      ret = recvfrom(LOCAL, (char*)&ack, sizeof(ack), 0, (SOCKADDR*)&remote, &size);
      htonl(ack);
      if (start + 5000 < timeGetTime()) // resend after 5 seconds without ack
      {
        if (++attempt >= 5) { delete handle; return -1; }
        ret = sendto(LOCAL, (char*)&pack_out, sizeof(UDP_PACKAGE), 0, (SOCKADDR*)&remote, sizeof(remote));
        ret = 0;
        start = timeGetTime();
      }
    }

    // Increment package and handle wraparound
    if (++current_package_id == (unsigned)(-1)) current_package_id = 1;
  }

  delete handle;
  return 0;
}

DWORD WINAPI Receiver(void* handle)
{
  ReceiverInfo* Handle = (ReceiverInfo*)handle;
  EnterCriticalSection(Handle->CritSec);
  unsigned ListeningPort = Handle->Port;
  LeaveCriticalSection(Handle->CritSec);

  sockaddr_in local;
  hostent* localhost = gethostbyname("");
  local.sin_family = AF_INET;
  local.sin_port = htons(ListeningPort);
  local.sin_addr.s_addr = inet_addr(inet_ntoa(*(in_addr*)*localhost->h_addr_list));
  SOCKET LOCAL = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (LOCAL == INVALID_SOCKET) throw "Can not create listening socket";
  int ret = bind(LOCAL, (SOCKADDR*)&local, sizeof(local));
  if (ret == SOCKET_ERROR) throw "Can not bind listening socket";

  unsigned long unblocked = 1;
  ret = ioctlsocket(LOCAL, FIONBIO, &unblocked);

  EnterCriticalSection(Handle->CritSec);
  std::cout << "Started listening on port " << Handle->Port << std::endl;
  LeaveCriticalSection(Handle->CritSec);

  while(1)
  {
    EnterCriticalSection(Handle->CritSec);
    if (Handle->shutDownSequence && !Handle->Transfers->size())
    {
      LeaveCriticalSection(Handle->CritSec);
      break;
    }
    std::map<std::string, TrnsfrInf> RECVLIST = *Handle->Transfers;
    LeaveCriticalSection(Handle->CritSec);


    // Iterate through all sending sockets and receive
    for (std::map<std::string, TrnsfrInf>::iterator i = RECVLIST.begin(); 
      i != RECVLIST.end(); ++i)
    {
      std::string filename = (*i).first;
      std::string fileloc = Handle->DownloadDir + "/" + filename;
      TrnsfrInf Sender = (*i).second;

      sockaddr_in remote;
      remote.sin_family = AF_INET;
      remote.sin_addr.s_addr = inet_addr(Sender.SndrIp.c_str());
      remote.sin_port = htons(Sender.SndrPort);

      UDP_PACKAGE pack_in;
      int size = sizeof(remote);
      int ret = recvfrom(LOCAL, (char*)&pack_in, sizeof(UDP_PACKAGE), 0, (SOCKADDR*)&remote, &size);

      EnterCriticalSection(Handle->CritSec);
      DWORD timeout = timeGetTime() - (*Handle->Transfers)[filename].AvrgRTT;
      LeaveCriticalSection(Handle->CritSec);

      if (ret != sizeof(UDP_PACKAGE))
      {
        if (ret == 0 || timeout > 10000 || (ret == -1 && WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != 0))
        {
          std::cout << "Lost connection to host, download of " << filename 
            << " aborted." << std::endl;
          EnterCriticalSection(Handle->CritSec);
          Handle->Transfers->erase(filename);
          LeaveCriticalSection(Handle->CritSec);
          break;

        }
        else continue;
      }

      ntohl(pack_in.total);
      ntohl(pack_in.part);
      ntohl(pack_in.size);

      if (ret > 0)
      {
        DWORD now = timeGetTime();

        EnterCriticalSection(Handle->CritSec);
        DWORD last = (*Handle->Transfers)[filename].AvrgRTT;
        (*Handle->Transfers)[filename].TimeRemaining = (now - last) * (pack_in.total - pack_in.part);
        (*Handle->Transfers)[filename].AvrgRTT += now;
        (*Handle->Transfers)[filename].AvrgRTT /= 2;
        (*Handle->Transfers)[filename].TotalPackages = pack_in.total;
        (*Handle->Transfers)[filename].LastPackage = pack_in.part;
        LeaveCriticalSection(Handle->CritSec);
      }

      FILE* fp;
      if (pack_in.part == 1) fp = fopen(fileloc.c_str(), "wb"); //replace already existing file
      else fp = fopen(fileloc.c_str(), "ab+");

      if (!fp) 
      {
        std::cout << "File locked, terminating download of " << filename << std::endl;
        EnterCriticalSection(Handle->CritSec);
        Handle->Transfers->erase(filename);
        LeaveCriticalSection(Handle->CritSec);
        continue;
      }

      fwrite(pack_in.data, sizeof(char), pack_in.size, fp);
      fclose(fp);

      unsigned ack = pack_in.part;
      ret = sendto(LOCAL, (char*)&ack, sizeof(ack), 0, (SOCKADDR*)&remote, sizeof(remote));

      if (pack_in.part == pack_in.total) // Completion
      {
        EnterCriticalSection(Handle->CritSec);
        Handle->Transfers->erase(filename);
        LeaveCriticalSection(Handle->CritSec);
        std::cout << "Download of " << filename << " is complete." << std::endl;
      }
    }
  }

  delete handle;
  return 0;
}
