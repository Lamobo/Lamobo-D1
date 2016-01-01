/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpClient.h
 * @brief    Class CcHttpClient
 */
#ifndef CcHttpClient_H_
#define CcHttpClient_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcUrl.h"
#include "CcHttpRequHeader.h"
#include "CcHttpRespHeader.h"

/**
 * @brief Button for GUI Applications
 */
class CcHttpClient : public CcThreadObject {
public:
  /**
   * @brief Constructor
   */
  CcHttpClient( void );

  /**
   * @brief Destructor
   */
  virtual ~CcHttpClient( void );

  bool execGet(CcUrl url);
  bool execHead(CcUrl url);
  bool execPost(CcUrl url, CcVector<CcStringPair> *PostData = 0);
  bool execPostMultip(CcUrl url, CcVector<CcStringPair> *PostData = 0, CcVector<CcStringPair> *files=0);

  CcCharArray *getCharArray(void);

  void setOutputDevice(CcIODevice* output);

  bool isDone(void);

  void run();
private: //methods
  bool connectSocket(void);

private:
  CcUrl m_Url;
  CcSocket *m_Socket;
  CcString m_WD;
  CcIODevice *m_Output;
  CcHttpRequHeader m_Header; 
  CcHttpRespHeader m_RespHeader;
  bool m_Done;
  CcCharArray m_Buffer;
};

#endif /* CcHttpClient_H_ */
