#pragma comment(lib, "ws2_32.lib")

#include "winsock2.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <map>

#include "server.h"
#include "Files.h"
#include "clientThread.h"
#include "notMain.cpp"


/* These need their own file */
void winsockFailMsg(void);
void localIPinfoMsg(hostent*, std::string);
void connectSuccessMSG(SOCKET&);
void invalidSocketMSG(void);
void srvrShutdownMSG(void);
void listenFailMSG(void);
int  portRequest(void);

int main(void)
{
  myServer * ServerObj = new myServer;

  WSADATA wsData_; // Start Winsock
  ServerObj->result = WSAStartup(MAKEWORD(2,2), &wsData_);

  if (ServerObj->result)
  {
    winsockFailMsg();
    return ServerObj->result;
  }

  // Get local host information and print it.  This is done to communicate to 
  // the client needed data.
  hostent* localhost_ = gethostbyname("");
  ServerObj->localIP_.assign (inet_ntoa(*(in_addr*)*localhost_->h_addr_list));
  localIPinfoMsg(localhost_, ServerObj->localIP_);

  ServerObj->localPort_ = portRequest();

  sockaddr_in localEnd_;  // Set up the local end
  localEnd_.sin_family = AF_INET;
  localEnd_.sin_port = htons(ServerObj->localPort_);
  localEnd_.sin_addr.s_addr  = inet_addr(ServerObj->localIP_.c_str());

  ServerObj->listenerSocket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

  if(ServerObj->listenerSocket_ == INVALID_SOCKET)
  {
    listenFailMSG();
    WSACleanup();	
    return 0;
  }

  // Bind local end to socket
  ServerObj->result = bind(ServerObj->listenerSocket_, (SOCKADDR*)&localEnd_, sizeof(localEnd_));

  if(ServerObj->result == SOCKET_ERROR)
  {
    int errCode = WSAGetLastError ();
    std::cout << "Attempt to bind local endpoint to socket failed with code "
      << errCode << "." << std::endl;
    return errCode;
  }

  ServerObj->result = listen(ServerObj->listenerSocket_, 10);	    // Put socket into a listening state

  if(ServerObj->result == SOCKET_ERROR)
  {
    int errCode = WSAGetLastError ();
    std::cout << "Attempt to start listening failed with code "
      << errCode << "." << std::endl;
    return errCode;
  }

  sockaddr_in remoteEndpoint;
  int endpointSize = sizeof(remoteEndpoint);
  SecureZeroMemory(&remoteEndpoint, endpointSize);
 
  CRITICAL_SECTION Section;
  InitializeCriticalSection (&Section);

  Files FILE_LIBRARY;
  std::map<SOCKET, User> USER_MAP;

  while(true)
  {
    SOCKET clientSocket_ = NULL;

    clientSocket_ = accept(ServerObj->listenerSocket_, (sockaddr*)&remoteEndpoint, &endpointSize);

    if(clientSocket_ == INVALID_SOCKET)
    {
      invalidSocketMSG();
      clientSocket_ = 0;
    }
    else
    {
      connectSuccessMSG(clientSocket_);
      ServerHandle* s  = new ServerHandle;
      s->m_FileLibrary = &FILE_LIBRARY;
      s->m_UserMap     = &USER_MAP;
      s->m_CritSection = &Section;
      s->m_Socket      = clientSocket_;

      // Create thread.h No need to get the handle since we will never
      // communicate from the thread from now on. If the server goes down, all
      // threads will go with it, this is intended.
      CreateThread(NULL, 0, clientThread, (void*)s, 0, NULL);
    }
  }


  DeleteCriticalSection (&Section);

  srvrShutdownMSG();
  shutdown(ServerObj->listenerSocket_, SD_BOTH);
  closesocket(ServerObj->listenerSocket_);

  WSACleanup();
  std::cin.get();

  delete ServerObj;

  return 0;
}

/* UI messages*/
void invalidSocketMSG(void)
{
  std::cout << "error: " << WSAGetLastError() << std::endl;
}

void connectSuccessMSG(SOCKET & socket_)
{
  std::cout << "User at socket " << socket_ << " connected." 
            << std::endl;
}

void winsockFailMsg(void)
{
  std::cout << "Could not start Winsock. :-(" << std::endl;
  return;
}

void localIPinfoMsg(hostent* localHostName, std::string localIP)
{
  std::cout << "Localhost: " << localHostName->h_name << std::endl;
  std::cout << "Local IP: " << localIP << std::endl;
}

void srvrShutdownMSG(void)
{
  std::cout << "Server received shutdown." << std::endl;
}

void listenFailMSG(void)
{
  std::cout << "Attempt to start listening failed with error " 
    << WSAGetLastError () << "." << std::endl;
}

int portRequest(void)
{
  do 
  {
    int port;
    std::cout << "Please enter a valid port to operate on: ";
    std::cin >> port;

    if ( (port < 1) || (port > MAXPORT) )
    {
      std::cout << "Sorry, port must be between 0 and " 
                << MAXPORT + 1 << "." <<std::endl;
    }
    else
    { 
      std::cout << "Listening to port " << port <<std::endl ;
      return port;
      break;
    }

  } while (true);// I really want to change this, seems simplistic, dangerous.

}