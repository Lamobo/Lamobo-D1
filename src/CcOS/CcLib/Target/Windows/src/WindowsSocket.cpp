/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     WindowsSocket.cpp
* @brief    Class WindowsSocket
*/
#include "WindowsSocket.h"
#include <io.h>
#include <fcntl.h>

//Statics
bool WindowsSocket::g_sWsaStarted = false;

WindowsSocket::WindowsSocket(eSocketType type) :
  m_Type(type),
  m_ClientSocket(INVALID_SOCKET)
{
  startWSA();
}

WindowsSocket::WindowsSocket(SOCKET socket, sockaddr sockAddr, int sockAddrlen) :
  m_ClientSocket(socket),
  m_sockAddr(sockAddr),
  m_sockAddrlen(sockAddrlen)
{
  startWSA();
}

void WindowsSocket::startWSA(void){
  if (!g_sWsaStarted){
    WSADATA wsaData;
    g_sWsaStarted = true;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
  }
}


WindowsSocket::~WindowsSocket( void )
{
  int iResult;
  if (m_ClientSocket != INVALID_SOCKET){
    iResult = shutdown(m_ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
      close();
    }
  }
}

bool WindowsSocket::bind(ipv4_t ipAddress, int16 Port)
{
  int iResult;
  struct addrinfo *result = NULL;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  CcString sPort;
  sPort.appendNumber(Port);
  iResult = getaddrinfo(NULL, sPort.getCharString(), &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    return false;
  }

  // Create a SOCKET for connecting to server
  m_ClientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (m_ClientSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    return false;
  }

  // Setup the TCP listening socket
  iResult = ::bind(m_ClientSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    close();
    return false;
  }
  return true;
}

bool WindowsSocket::connect(ipv4_t ipAddress, int16 Port)
{
  CcString sHostname;
  sHostname.appendIp(m_ConnectionIP);
  CcString sPort;
  sPort.appendNumber(Port);
  return connect(sHostname, sPort);
}

bool WindowsSocket::connect(CcString &hostName, CcString &hostPort)
{
  WSADATA wsaData;
  struct addrinfo *result = NULL,
                  *ptr = NULL,
                  hints;
  int iResult;

  ZeroMemory( &hints, sizeof(hints) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  iResult = getaddrinfo(hostName.getCharString(), hostPort.getCharString(), &hints, &result);
  if ( iResult != 0 ) {
      printf("getaddrinfo failed with error: %d\n", iResult);
      return 1;
  }

  // Attempt to connect to an address until one succeeds
  for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

      // Create a SOCKET for connecting to server
    m_ClientSocket = socket(ptr->ai_family, ptr->ai_socktype,
          ptr->ai_protocol);
    if (m_ClientSocket == INVALID_SOCKET) {
          printf("socket failed with error: %ld\n", WSAGetLastError());
          return 1;
      }

      // Connect to server.
    iResult = ::connect(m_ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      close();
      m_ClientSocket = INVALID_SOCKET;
      continue;
    }
    break;
  }
  freeaddrinfo(result);
  if (m_ClientSocket == INVALID_SOCKET) {
      printf("Unable to connect to server!\n");
      return 0;
  }
  return 1;
}

bool WindowsSocket::listen(void)
{
  if(!::listen(m_ClientSocket, 0))
    return true;
  return false;
}

CcSocket* WindowsSocket::accept(void)
{
  // Accept a client socket
  CcSocket *sRet = 0;
  SOCKET Temp;
  sockaddr sockAddr;
  int sockAddrlen=sizeof(sockAddr);
  Temp = ::accept(m_ClientSocket, &sockAddr, &sockAddrlen);
  if (Temp == INVALID_SOCKET) {
    printf("accept failed with error: %d\n", WSAGetLastError());
    close();
  }
  else{
    sRet = new WindowsSocket(Temp, sockAddr, sockAddrlen);
  }
  return sRet;
}

size_t WindowsSocket::read(char *buf, size_t bufSize)
{
  int recSize = SIZE_MAX;
  if (m_ClientSocket != INVALID_SOCKET)
  {
    recSize = ::recv(m_ClientSocket, buf, bufSize, 0);
    if (recSize == SOCKET_ERROR){
      printf("accept failed with error: %d\n", WSAGetLastError());
      close();
    }
  }
  return recSize;
}

size_t WindowsSocket::write(char *buf, size_t bufSize)
{
  // Send an initial buffer
  int iResult;
  iResult = ::send(m_ClientSocket, buf, bufSize, 0);
  if (iResult == SOCKET_ERROR) {
    printf("accept failed with error: %d\n", WSAGetLastError());
    close();
    return SIZE_MAX;
  }
  return iResult;
}

bool WindowsSocket::getHostByName(CcString &hostname, ipv4_t *addr)
{
  return false;
}

bool WindowsSocket::close(void)
{
  bool bRet(false);
  if (SOCKET_ERROR != closesocket(m_ClientSocket)){
    bRet = true;
    m_ClientSocket = INVALID_SOCKET;
  }
  return bRet;
}

bool WindowsSocket::cancel(void){
  return true;
}

size_t WindowsSocket::readTimeout(char *buf, size_t bufSize, time_t timeout) {
  size_t iRet = 0;
  fd_set readfds;
  struct timeval tv;
  int rv;
  // clear the set ahead of time
  FD_ZERO(&readfds);

  // add our descriptors to the set
  FD_SET(m_ClientSocket, &readfds);

  // since we got s2 second, it's the "greater", so we use that for
  // the n param in select()

  // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
  tv.tv_sec = 0;
  tv.tv_usec = timeout;
  rv = select(m_ClientSocket+1, &readfds, NULL, NULL, &tv);

  if (rv == -1) {
    perror("select"); // error occurred in select()
  }
  else if (rv == 0) {
    printf("Timeout occurred!  No data after 10.5 seconds.\n");
  }
  else {
    // one or both of the descriptors have data
    if (FD_ISSET(m_ClientSocket, &readfds)) {
      iRet = recv(m_ClientSocket, buf, bufSize, 0);
    }
  }
  return iRet;
}
