/**
* @author         Andreas Dirmeier
*
* <br>copyright   (c) Technische Software Entwicklung Plazotta © 2014
*
* <br>Version     1.00
*
* <br>Language    C++ ANSI V3
*
* <br>History
*   15.12.2015 / AD
*      Changed to TCP Connection
*/
/**
*   @file       CScpi.cpp
*   @brief      Simple Scpi-Receiver
**************************************************************************/

#include "CScpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string>
#include <atlstr.h>

#define SCPI_PORT   "12321"
#define SCPI_IP     "127.0.0.1"

CScpi::CScpi(void):
m_RawReceiver(NULL)
{
  Init();
}

CScpi::~CScpi(void)
{
  Deinit();
}

//Initialize Mailslot
BOOL CScpi::Init(void)
{
  BOOL bSuccess(TRUE);
  return bSuccess;
}

void CScpi::Deinit(void)
{
}

BOOL CScpi::Run()
{
  BOOL bResult(TRUE);
  while (bResult == TRUE)
  {
    if (InitSocket())
    {
      if (!LoopReceive())
      {
        bResult = FALSE;
      }
    }
    else
    {
      bResult = FALSE;
    }
  }
  return 0;
}

bool CScpi::InitSocket(void)
{
  WSADATA wsaData;
  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;
  m_ClientSocket = INVALID_SOCKET;

  struct addrinfo *result = NULL;
  struct addrinfo hints;


  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return false;
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  iResult = getaddrinfo(NULL, SCPI_PORT, &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    WSACleanup();
    return false;
  }

  // Create a SOCKET for connecting to server
  ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return false;
  }

  // Setup the TCP listening socket
  iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ListenSocket);
    WSACleanup();
    return false;
  }

  freeaddrinfo(result);

  iResult = listen(ListenSocket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    printf("listen failed with error: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    WSACleanup();
    return false;
  }

  // Accept a client socket
  m_ClientSocket = accept(ListenSocket, NULL, NULL);
  if (m_ClientSocket == INVALID_SOCKET) {
    printf("accept failed with error: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    WSACleanup();
    return false;
  }

  // No longer need server socket
  closesocket(ListenSocket);
  return true;
}

bool CScpi::LoopReceive(void)
{
  int     iResult;
  size_t  iSendResult;
  int     iBufLen = SCPI_BUFLEN;
  char    cBuf[SCPI_BUFLEN];
  std::string sToCallback;
  std::string sFromCallback;
  // Receive until the peer shuts down the connection
  do {

    iResult = recv(m_ClientSocket, cBuf, iBufLen, 0);
    if (iResult > 0) {
      sToCallback.append(cBuf, iResult);
      if (sToCallback.compare("SYST:SHUT") == 0)
      {
        return false;
      }
      else if (m_RawReceiver != NULL)
      {
        m_RawReceiver(sToCallback, &sFromCallback);
      }
      // Echo the buffer back to the sender
      memcpy(cBuf, sFromCallback.c_str(), SCPI_BUFLEN);
      iSendResult = send(m_ClientSocket, cBuf, sFromCallback.size(), 0);
      if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(m_ClientSocket);
        WSACleanup();
        return false;
      }
      sToCallback.clear();
    }
    else if (iResult == 0)
      printf("Connection closing...\n");
    else  {
      printf("recv failed with error: %d\n", WSAGetLastError());
      closesocket(m_ClientSocket);
      WSACleanup();
      return false;
    }

  } while (iResult > 0);

  // shutdown the connection since we're done
  iResult = shutdown(m_ClientSocket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    printf("shutdown failed with error: %d\n", WSAGetLastError());
    closesocket(m_ClientSocket);
    WSACleanup();
    return false;
  }

  // cleanup
  closesocket(m_ClientSocket);
  WSACleanup();
  return true;
}
