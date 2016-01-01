/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpClient.cpp
 * @brief    Implementation of Class CcFtpClient
 */
#include "CcFtpClient.h"
#include "CcKernel.h"

CcFtpClient::CcFtpClient(void) :
m_Socket(0),
m_Done(false)
{
}

CcFtpClient::~CcFtpClient(void) {
}