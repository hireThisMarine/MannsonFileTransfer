#ifndef P2PCON_H
#define P2PCON_H

#include <windows.h>
#include <string>
#include <map>

struct TrnsfrInf
{
  std::string SndrIp;
  unsigned SndrPort;
  unsigned TotalPackages;
  unsigned LastPackage;
  DWORD AvrgRTT;
  DWORD TimeRemaining;
};

struct SenderInfo
{
  CRITICAL_SECTION* CritSec;
  std::map<std::string, TrnsfrInf>* Transfers;
  std::string IP;
  unsigned Port;
  std::string FileName;
  std::string ShareDir;
};

struct ReceiverInfo
{
  CRITICAL_SECTION* CritSec;
  std::map<std::string, TrnsfrInf>* Transfers;
  unsigned Port;
  bool shutDownSequence;
  std::string DownloadDir;
};

DWORD WINAPI SendFile(void* handle);
DWORD WINAPI Receiver(void* handle);

// UDP PACKAGE ENCAPSLATION
#define UDP_PACK_DATA_SIZE 500 * sizeof(char)
#define UDP_PACK_DATA_COUNT 500
struct UDP_PACKAGE
{
  unsigned part;
  unsigned total;
  unsigned size;
  char data[UDP_PACK_DATA_SIZE];
};

#endif