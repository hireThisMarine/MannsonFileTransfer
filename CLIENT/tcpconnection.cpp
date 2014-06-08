/*! ============================================================================
  @file tcpconnection.cpp

  @author Maximilian Manndorff
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par ID 50006909
  @par Course CS260
  @par Project 3
  @date 05.07.2010

  @brief 
    Definition of the TCPConnection object which manages TCP/IP connection
    in a cleaner way. Intended for basic usage only.

============================================================================= */
#include "tcpconnection.h"
namespace CS260
{
namespace Con
{


////////////////////////////////////////////////////////////////////////////////
// PUBLIC AUTOS

TCPConnection::TCPConnection(void)
  : m_WsaVersion(MAKEWORD(2, 2)), m_LocalPort(0), m_RemotePort(0), 
    m_Connected(false), m_Listening(false), m_BlockingMode(0)
{
  WSAStartup(m_WsaVersion, &m_WSAData);
  m_LocalHost = gethostbyname("");
  m_LocalIP = (inet_ntoa(*(in_addr*)*m_LocalHost->h_addr_list));
}

TCPConnection::~TCPConnection(void) 
{ 
  Disconnect();
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC ACCESSORS

std::string TCPConnection::GetLocalIP(void) const
{
  return m_LocalIP;
}

std::string TCPConnection::GetRemoteIP(void) const
{
  return m_RemoteIP;
}

unsigned TCPConnection::GetLocalPort(void) const
{
  return m_LocalPort;
}

unsigned TCPConnection::GetRemotePort(void) const
{
  return m_RemotePort;
}

bool TCPConnection::IsConnected(void) const
{
  return m_Connected;
}

bool TCPConnection::IsListening(void) const
{
  return m_Listening;
}

WORD TCPConnection::GetVersion(void) const
{
  return m_WsaVersion;
}

int TCPConnection::GetLastError(void) const
{
  return WSAGetLastError();
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC MUTATORS

void TCPConnection::SetRemoteIP(std::string _IP)
{
  m_RemoteIP = _IP;
}

void TCPConnection::SetRemotePort(unsigned _port)
{
  m_RemotePort = _port;
}

void TCPConnection::SetLocalPort(unsigned _port)
{
  m_LocalPort = _port;
}

void TCPConnection::SetWSAVersion(WORD _version)
{
  m_WsaVersion = _version;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS

void TCPConnection::Connect(void)
{
  if (m_Listening) 
    throw Error(*this, __FILE__, __LINE__, 
                "Can not establish new connection while listening!");

  if (!m_Connected)
    Connect(m_RemoteIP, m_RemotePort, m_LocalPort);
}

void TCPConnection::Connect(std::string _rIP, unsigned _rPort, unsigned _lPort)
{
  if (m_Listening)
    throw Error(*this, __FILE__, __LINE__,
                "Can not establish connection while listening.");

  if (m_Connected)
    throw Error(*this, __FILE__, __LINE__, 
                "Can not establish new connection while still connected.");

  if (!IsValid(_rIP) && !IsValid(_rPort) && !(_lPort == 0 || IsValid(_lPort)))
    throw Error(*this, __FILE__, __LINE__);

  m_RemoteIP = _rIP;
  m_RemotePort = _rPort;
  if (_lPort) m_LocalPort = _lPort;

  int result = 0;

  m_LocalEndPoint.sin_family = AF_INET;
  m_LocalEndPoint.sin_addr.s_addr = inet_addr(m_LocalIP.c_str());
  m_LocalEndPoint.sin_port = htons(m_LocalPort);
  m_RemoteEndPoint.sin_family = AF_INET;
  m_RemoteEndPoint.sin_addr.s_addr = inet_addr(m_RemoteIP.c_str());
  m_RemoteEndPoint.sin_port = htons(m_RemotePort);
  m_Connection = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

  if (m_Connection == INVALID_SOCKET) throw Error(*this, __FILE__, __LINE__);

  result = 
    bind(m_Connection, (SOCKADDR*)&m_LocalEndPoint, sizeof(m_LocalEndPoint));

  if (result == SOCKET_ERROR) throw Error(*this, __FILE__, __LINE__);

  result = 
    connect(m_Connection, (SOCKADDR*)&m_RemoteEndPoint, sizeof(m_RemoteEndPoint));

  if (result == SOCKET_ERROR) throw 
    Error(*this, __FILE__, __LINE__, "Can't connect to remote host");

  if (m_BlockingMode) 
    result = ioctlsocket(m_Connection, FIONBIO, &m_BlockingMode);

  if (result == SOCKET_ERROR) throw Error(*this, __FILE__, __LINE__);

  m_Connected = true;
}

void TCPConnection::Listen(void)
{
  if (m_Connected)
    throw Error(*this, __FILE__, __LINE__, 
                "Can not start listening while connected");

  if (!m_Listening)
    Listen(m_LocalPort);
}

void TCPConnection::Listen(unsigned _lPort)
{
  if (m_Connected)
    throw Error(*this, __FILE__, __LINE__, 
                "Can not start listening while connected");

  if (m_Listening)
    throw Error(*this, __FILE__, __LINE__,
                "Can not start listening while already listening");

  if (!IsValid(_lPort)) 
    throw Error(*this, __FILE__, __LINE__, "Invalid local port");

  m_LocalPort = _lPort;
  int result = 0;

  m_LocalEndPoint.sin_family = AF_INET;
  m_LocalEndPoint.sin_addr.s_addr = inet_addr(m_LocalIP.c_str());
  m_LocalEndPoint.sin_port = htons(m_LocalPort);
  m_Connection = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

  if (m_Connection == INVALID_SOCKET) 
    throw Error(*this, __FILE__, __LINE__, "Unable to start WinSock");

  result = 
    bind(m_Connection, (SOCKADDR*)&m_LocalEndPoint, sizeof(m_LocalEndPoint));

  if (result == SOCKET_ERROR) 
    throw Error(*this, __FILE__, __LINE__, "Couldn't bind local end point");

  result = listen(m_Connection, MAX_BACK_LOG);

  if (result == SOCKET_ERROR) 
    throw Error(*this, __FILE__, __LINE__, "Couldn't start listening");

  m_Listening = true;
}

SOCKET TCPConnection::Accept(void)
{
  if (!m_Listening)
    throw Error(*this, __FILE__, __LINE__, 
                "Can't accept incoming connection when not listening");

  sockaddr_in RemoteEndPoint;
  int RemoteEndPointSize = sizeof(RemoteEndPoint);

  SOCKET ClientSocket = NULL;

  int result = 
    accept(m_Connection, (sockaddr*)&RemoteEndPoint, &RemoteEndPointSize);

  if (ClientSocket == INVALID_SOCKET)
    throw Error(*this, __FILE__, __LINE__, 
                "Error while accepting incoming connection");

  return ClientSocket;
}

void TCPConnection::Disconnect(void)
{
  shutdown(m_Connection, SD_BOTH);
  closesocket(m_Connection);
  m_Connected = false;
}

void TCPConnection::Reset(void)
{
  Disconnect();
  m_RemoteIP.clear();

  m_WsaVersion   = MAKEWORD(2, 2);
  m_LocalHost    = gethostbyname("");
  m_LocalIP      = inet_ntoa(*(in_addr*)*m_LocalHost->h_addr_list);
  m_LocalPort    = 0;
  m_RemotePort   = 0;
  m_Connected    = false;
  m_Listening    = false;
  m_BlockingMode = 0;
}

void TCPConnection::SetVersion(int _high, int _low)
{
  m_WsaVersion = MAKEWORD(_high, _low);
}

void TCPConnection::Unblock(void)
{
  if (m_Connected || m_Listening)
    throw Error(*this, __FILE__, __LINE__, 
                "Has to be unblocked before connection");

  m_BlockingMode = 1;
}

void TCPConnection::Block(void)
{
  if (m_Connected || m_Listening)
    throw Error(*this, __FILE__, __LINE__, 
    "Has to be blocked before connection");

  m_BlockingMode = 0;
}

int TCPConnection::send(char const* _buf, int _bufSize, int _flags)
{
  if (!m_Connected)
    throw Error(*this, __FILE__, __LINE__, "Can't send while disconnected");

  int ret = ::send(m_Connection, _buf, _bufSize, _flags);

  if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != 0)
    Disconnect();

  return ret;
}

int TCPConnection::recv(char* _buf, int _bufSize, int _flags)
{
  if (!m_Connected)
    throw Error(*this, __FILE__, __LINE__, "Can't receive while disconnected");

  _buf[0] = '\0';
  int ret = ::recv(m_Connection, _buf, _bufSize, _flags);

  if (ret == 0 || WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != 0)
    Disconnect();

  return ret;
}


////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS

bool TCPConnection::IsValid(std::string _ip) const
{
  unsigned int n1, n2, n3, n4;

  if (sscanf (_ip.c_str(), "%u.%u.%u.%u", &n1, &n2, &n3, &n4) != 4) 
    return false;

  if ((n1 <= 255) && (n2 <= 255) && (n3 <= 255) && (n4 <= 255)) 
    return true;

  return false;
}

bool TCPConnection::IsValid(unsigned _port) const
{
  if (_port >= MIN_PORT && _port <= MAX_PORT)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATE ERROR

TCPConnection::Error::Error(TCPConnection _con)
  : m_Line(-1), m_Port(_con.GetRemotePort()), 
    m_WinSockError(_con.GetLastError()), m_IP(_con.GetRemoteIP()), 
    m_File("Undefined")
{ 
}

TCPConnection::Error::Error(TCPConnection _con, std::string _file, 
  unsigned _line, std::string _message)
  : m_Line(_line), m_Port(_con.GetRemotePort()), 
    m_WinSockError(_con.GetLastError()), m_IP(_con.GetRemoteIP()), m_File(_file),
    m_Message(_message)
{
}

std::string TCPConnection::Error::GetFile(void) const
{
  return m_File;
}

unsigned TCPConnection::Error::GetLine(void) const
{
  return m_Line;
}

std::string TCPConnection::Error::GetIP(void) const
{
  return m_IP;
}

unsigned TCPConnection::Error::GetPort(void) const
{
  return m_Port;
}

int TCPConnection::Error::GetWSAError(void) const
{
  return m_WinSockError;
}

std::string TCPConnection::Error::GetMessage(void) const
{
  return m_Message;
}


} // namespace Con
} // namespace CS260
