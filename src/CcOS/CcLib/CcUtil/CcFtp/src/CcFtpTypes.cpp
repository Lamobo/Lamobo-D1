/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcFtpTypes.h
* @brief    CcFtpTypes values
*/

#include "CcFtpTypes.h"

sFtpCommandMap sFtpCommandList[] =
{
  { FTP_OPTS, "OPTS" },
  { FTP_USER, "USER" },
  { FTP_PASS, "PASS" },
  { FTP_AUTH, "AUTH" },
  { FTP_SYST, "SYST" },
  { FTP_FEAT, "FEAT" },
  { FTP_CWD,  "CWD"  },
  { FTP_PWD,  "PWD"  },
  { FTP_NLST, "NLST" },
  { FTP_TYPE, "TYPE" },
  { FTP_PASV, "PASV" },
  { FTP_LIST, "LIST" },
  { FTP_NLST, "NLST" },
  { FTP_CDUP, "CDUP" },
  { FTP_RETR, "RETR" },
  { FTP_RNFR, "RNFR" },
  { FTP_RNTO, "RNTO" },
  { FTP_STOR, "STOR" },
  { FTP_MKD,  "MKD"  },
  { FTP_RMD,  "RMD"  },
  { FTP_DELE, "DELE" },
  { FTP_PORT, "PORT" },
  { FTP_SIZE, "SIZE" },
  { FTP_MDTM, "MDTM" },
};

size_t sFtpCommandListSize = sizeof(sFtpCommandList) / sizeof(sFtpCommandMap);
