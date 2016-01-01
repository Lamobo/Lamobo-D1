/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-09
* @par        Language   C++ ANSI V3
*/
/**
* @file     main.cpp
* @brief    Development default CLI-Application for testing new Implementations
*/

#include "CcBase.h"
#include "CcKernel.h"
#include "CcFtp/CcFtpServer.h"
#include "CcNetwork/CcHttpServer.h"
#include "stdio.h"


// Application entry point. 
int main(int argc, char **argv)
{
  Kernel.setArg(argc, argv);
  Kernel.initCLI();

  CcFtpServer FtpServer(27521);
  FtpServer.start();
  CcHttpServer HttpServer(27580);
  HttpServer.start();

  Kernel.start();
  return 0;
}
