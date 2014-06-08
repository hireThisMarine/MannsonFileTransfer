#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#pragma comment(lib, "ws2_32.lib")
#include "winsock2.h"
#include <windows.h>
#include <string>
#define MAX_BACK_LOG 10
#define MIN_PORT 1
#define MAX_PORT 65535


namespace CS260
{
namespace Con
{

class TCPConnection
{
private: // DATA
  WSADATA         m_WSAData;
  WORD            m_WsaVersion;
  HOSTENT*        m_LocalHost;
  std::string     m_LocalIP;
  std::string     m_RemoteIP;
  unsigned        m_LocalPort;
  unsigned        m_RemotePort;
  sockaddr_in     m_LocalEndPoint;
  sockaddr_in     m_RemoteEndPoint;
  SOCKET          m_Connection;
  bool            m_Connected;
  bool            m_Listening;
  u_long          m_BlockingMode;


public: // AUTOS
  TCPConnection(void);
  ~TCPConnection(void);


public: // ACCESSORS
  std::string GetLocalIP(void) const;
  std::string GetRemoteIP(void) const;
  unsigned GetLocalPort(void) const;
  unsigned GetRemotePort(void) const;
  bool IsConnected(void) const;
  bool IsListening(void) const;
  WORD GetVersion(void) const;
  int GetLastError(void) const;



public: // MUTATORS
  void SetRemoteIP(std::string _ip);
  void SetRemotePort(unsigned _port);
  void SetLocalPort(unsigned _port);
  void SetWSAVersion(WORD _version);


public: // METHODS
  void Connect(void);
  void Connect(std::string IP, unsigned _rPort, unsigned _lPort = 0);
  void Listen(void);
  void Listen(unsigned _lPort);
  SOCKET Accept(void);
  void Disconnect(void);
  void Reset(void);
  void SetVersion(int _high, int _low);
  void Unblock(void);
  void Block(void);
  int send(char const* _buf, int _bufSize, int _flags = 0);
  int recv(char* _buf, int _bufSize, int _flags = 0);


private: // METHODS
  bool IsValid(std::string IP) const;
  bool IsValid(unsigned Port) const;


public: // AGGREGATE ERROR
  class Error
  {
  public: // AUTOS
    Error(TCPConnection con);
    Error(TCPConnection con, std::string file, unsigned port, 
      std::string message = "Undefined"                   );


  public: // ACCESSORS
    std::string GetFile(void) const;
    unsigned GetLine(void) const;
    std::string GetIP(void) const;
    unsigned GetPort(void) const;
    int GetWSAError(void) const;
    std::string GetMessage(void) const;


  private: // DATA
    unsigned        m_Line;
    unsigned        m_Port;
    int             m_WinSockError;
    std::string     m_IP;
    std::string     m_File;
    std::string     m_Message;
  };


};
} // namespace Con
} // namespace CS260
#endif
