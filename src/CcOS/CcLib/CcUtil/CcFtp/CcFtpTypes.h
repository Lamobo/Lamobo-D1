/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcFtpTypes.h
* @brief    Class CcFtpTypes
*/
#ifndef CcFtpTypes_H_
#define CcFtpTypes_H_

typedef enum{
  FTP_UNKNOWN = 0,
  FTP_ABOR,//		Abort an active file transfer.
  FTP_ACCT,//		Account information.
  FTP_ADAT,//	RFC 2228	Authentication / Security Data
  FTP_ALLO,//		Allocate sufficient disk space to receive a file.
  FTP_APPE,//		Append.
  FTP_AUTH,//	RFC 2228	Authentication / Security Mechanism
  FTP_CCC,//	RFC 2228	Clear Command Channel
  FTP_CDUP,//		Change to Parent Directory.
  FTP_CONF,//	RFC 2228	Confidentiality Protection Command
  FTP_CWD,//		Change working directory.
  FTP_DELE,//		Delete file.
  FTP_ENC,//	RFC 2228	Privacy Protected Channel
  FTP_EPRT,//	RFC 2428	Specifies an extended address and port to which the server should connect.
  FTP_EPSV,//	RFC 2428	Enter extended passive mode.
  FTP_FEAT,//	RFC 2389	Get the feature list implemented by the server.
  FTP_HELP,//		Returns usage documentation on a command if specified, else a general help document is returned.
  FTP_LANG,//	RFC 2640	Language Negotiation
  FTP_LIST,//		Returns information of a file or directory if specified, else information of the current working directory is returned.
  FTP_LPRT,//	RFC 1639	Specifies a long address and port to which the server should connect.
  FTP_LPSV,//	RFC 1639	Enter long passive mode.
  FTP_MDTM,//	RFC 3659	Return the last - modified time of a specified file.
  FTP_MIC,//	RFC 2228	Integrity Protected Command
  FTP_MKD,//		Make directory.
  FTP_MLSD,//	RFC 3659	Lists the contents of a directory if a directory is named.
  FTP_MLST,//	RFC 3659	Provides data about exactly the object named on its command line, and no others.
  FTP_MODE,//		Sets the transfer mode(Stream, Block, or Compressed).
  FTP_NLST,//		Returns a list of file names in a specified directory.
  FTP_NOOP,//		No operation(dummy packet; used mostly on keepalives).
  FTP_OPTS,//	RFC 2389	Select options for a feature.
  FTP_PASS,//		Authentication password.
  FTP_PASV,//		Enter passive mode.
  FTP_PBSZ,//	RFC 2228	Protection Buffer Size
  FTP_PORT,//		Specifies an address and port to which the server should connect.
  FTP_PROT,//	RFC 2228	Data Channel Protection Level.
  FTP_PWD,//	Print working directory.Returns the current directory of the host.
  FTP_QUIT,//		Disconnect.
  FTP_REIN,//		Re initializes the connection.
  FTP_REST,//		Restart transfer from the specified point.
  FTP_RETR,//		Retrieve a copy of the file
  FTP_RMD,//		Remove a directory.
  FTP_RNFR,//		Rename from.
  FTP_RNTO,//		Rename to.
  FTP_SITE,//		Sends site specific commands to remote server.
  FTP_SIZE,//	RFC 3659	Return the size of a file.
  FTP_SMNT,//		Mount file structure.
  FTP_STAT,//		Returns the current status.
  FTP_STOR,//		Accept the data and to store the data as a file at the server site
  FTP_STOU,//		Store file uniquely.
  FTP_STRU,//		Set file transfer structure.
  FTP_SYST,//		Return system type.
  FTP_TYPE,//		Sets the transfer mode(ASCII / Binary).
  FTP_USER,//		Authentication username.
  FTP_XCUP,//	RFC 775	Change to the parent of the current working directory
  FTP_XMKD,//	RFC 775	Make a directory
  FTP_XPWD,//	RFC 775	Print the current working directory
  FTP_XRCP,//	RFC 743
  FTP_XRMD,//	RFC 775	Remove the directory
  FTP_XRSQ,//	RFC 743
  FTP_XSEM,//	RFC 737	Send, mail if cannot
  FTP_XSEN,//	RFC 737	Send to terminal
} eFtpCommands;


typedef struct{
  eFtpCommands eCommand;
  const char *strCommand;
} sFtpCommandMap;

extern sFtpCommandMap sFtpCommandList[];
extern size_t sFtpCommandListSize;

#define FTP_200 "200 Command successfully procressed \r\n"
#define FTP_230 "230 User login complete \r\n"
#define FTP_215 "215 UNIX Type: l8\r\n"
#define FTP_211 "211 \r\n"
#define FTP_226 "226 Transfer complete. \r\n"
#define FTP_250 "250 Requested file action okay, completed.\r\n"

#define FTP_425 "425 no connection established\r\n"
#define FTP_426 "426 Connection closed; transfer aborted.\r\n"
#define FTP_430 "430 Invalid username or password.\r\n"
#define FTP_450 "450 Requested file action not taken.\r\n"
#define FTP_451 "451 Requested action aborted.Local error in processing.\r\n"
#define FTP_452 "452 Requested action not taken.Insufficient storage space in system.File unavailable(e.g., file busy).\r\n"

#define FTP_500 "500 Syntax error"
#define FTP_501 "501 Connection Error.\r\n"
#define FTP_550 "550 Requested action not taken. File unavailable (e.g., file not found, no access).\r\n"

#endif /* CcFtpTypes_H_ */
