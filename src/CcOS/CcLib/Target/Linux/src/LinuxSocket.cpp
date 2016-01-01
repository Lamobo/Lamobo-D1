/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     LinuxSocket.cpp
* @brief    Class LinuxSocket
*/
#include "LinuxSocket.h"
#include <fcntl.h>
#include <stdio.h>
#include "errno.h"


LinuxSocket::LinuxSocket(eSocketType type) :
  m_Type(type),
  m_ClientSocket(0)
{
}

LinuxSocket::LinuxSocket(int socket, sockaddr sockAddr, int sockAddrlen) :
  m_ClientSocket(socket),
  m_sockAddr(sockAddr),
  m_sockAddrlen(sockAddrlen)
{
  m_FromIP.ip4 = sockAddr.sa_data[2];
  m_FromIP.ip3 = sockAddr.sa_data[3];
  m_FromIP.ip2 = sockAddr.sa_data[4];
  m_FromIP.ip1 = sockAddr.sa_data[5];
}

LinuxSocket::~LinuxSocket( void )
{
  int iResult;
  if (m_ClientSocket != 0){
    iResult = shutdown(m_ClientSocket, SHUT_RDWR);
    if (iResult != 0) {
      close();
    }
  }
}

bool LinuxSocket::bind(ipv4_t ipAddress, uint16 Port)
{
  int iResult;
  struct addrinfo *result = 0;
  struct addrinfo aiHints;
  memset(&aiHints, 0, sizeof(aiHints));
  aiHints.ai_family = AF_INET;
  aiHints.ai_socktype = SOCK_STREAM;
  aiHints.ai_protocol = IPPROTO_TCP;
  aiHints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  CcString sPort;
  sPort.appendNumber(Port);
  iResult = getaddrinfo(NULL, sPort.getCharString(), &aiHints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    return false;
  }

  // Create a SOCKET for connecting to server
  m_ClientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (m_ClientSocket < 0) {
    printf("socket failed with error: %d\n", errno);
    freeaddrinfo(result);
    return false;
  }

  // Setup the TCP listening socket
  iResult = ::bind(m_ClientSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult != 0) {
    printf("bind failed with error: %d\n", errno);
    freeaddrinfo(result);
    return false;
  }
  return true;
}

bool LinuxSocket::connect(ipv4_t ipAddress, uint16 Port)
{
  CcString sHostname;
  sHostname.appendIp(ipAddress);
  CcString sPort;
  sPort.appendNumber(Port);
  return connect(sHostname, sPort);
}

bool LinuxSocket::connect(CcString &hostName, CcString &hostPort)
{
  struct addrinfo *result = NULL,
                  *ptr = NULL,
                  hints;
  int iResult;

  memset( &hints, 0, sizeof(hints) );
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
    if (m_ClientSocket < 0) {
          printf("socket failed with error: %d\n", errno);
          return 1;
      }

      // Connect to server.
    iResult = ::connect(m_ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult < 0) {
      close();
      m_ClientSocket = -1;
      continue;
    }
    break;
  }
  freeaddrinfo(result);
  return 1;
}

bool LinuxSocket::listen(void)
{
  if(!::listen(m_ClientSocket, 0))
    return true;
  return false;
}

CcSocket* LinuxSocket::accept(void)
{
  // Accept a client socket
  CcSocket *sRet = 0;
  int Temp;
  sockaddr sockAddr;
  socklen_t sockAddrlen=sizeof(sockAddr);
  Temp = ::accept(m_ClientSocket, &sockAddr, &sockAddrlen);
  if (Temp < 0) {
    printf("accept failed with error: \n");
    close();
  }
  else{
    sRet = new LinuxSocket(Temp, sockAddr, sockAddrlen);
  }
  return sRet;
}

size_t LinuxSocket::read(char *buf, size_t bufSize)
{
  int recSize = SIZE_MAX;
  if (m_ClientSocket >= 0)
  {
    recSize = ::recv(m_ClientSocket, buf, bufSize, 0);
    if (recSize == -1){
      printf("accept failed with error:\n");
      close();
    }
  }
  return recSize;
}

size_t LinuxSocket::write(char *buf, size_t bufSize)
{
  // Send an initial buffer
  int iResult;
  iResult = ::send(m_ClientSocket, buf, bufSize, 0);
  if (iResult == -1) {
    printf("accept failed with error:\n");
    close();
    return SIZE_MAX;
  }
  return iResult;
}

bool LinuxSocket::getHostByName(CcString &hostname, ipv4_t *addr)
{
  return false;
}

bool LinuxSocket::close(void)
{
  bool bRet=false;
  if(m_ClientSocket != 0){
    bRet = ::close(m_ClientSocket);
  }
  return bRet;
}

bool LinuxSocket::cancel(void){
  bool bRet(false);
  if (-1 != shutdown(m_ClientSocket, SHUT_RDWR)){
    bRet = true;
    m_ClientSocket = 0;
  }
  return bRet;
}

size_t LinuxSocket::readTimeout(char *buf, size_t bufSize, time_t timeout) {
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
