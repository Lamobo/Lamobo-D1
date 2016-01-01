/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpRequHeader.h
 * @brief    Class CcHttpRequHeader
 */
#ifndef CcHttpRequHeader_H_
#define CcHttpRequHeader_H_

#include "CcBase.h"
#include "CcString.h"

#define HTTP_GET          0x01
#define HTTP_HEAD         0x02
#define HTTP_POST_URLENC  0x03
#define HTTP_POST_MULTIP  0x04

/**
 * @brief Button for GUI Applications
 */
class CcHttpRequHeader {
public:
  /**
   * @brief Constructor
   */
  CcHttpRequHeader(CcString Parse);

  CcHttpRequHeader(bool init = 0);

  /**
   * @brief Destructor
   */
  virtual ~CcHttpRequHeader( void );

  CcString getHeader(void);

  void parse(CcString& Parse);

  void setHost(CcString Host);
  void setUserAgent(CcString Host);
  void setRquest(CcString Request);
  void setContentType(uint8 flags, CcString additional="");
  void setContentSize(size_t size);
private:
  void parseLine(CcString& Parse);
public:
  CcString m_HTTPTarget;
  CcString m_HTTPMethod;
  CcString m_Accept;
  CcString m_AcceptCharset;
  CcString m_AcceptEncoding;
  CcString m_AcceptLanguage;
  CcString m_Authorization;
  CcString m_CacheControl;
  CcString m_Connection;
  CcString m_Cookie;
  CcString m_ContentLength;
  CcString m_ContentType;
  CcString m_Host;
  CcString m_Range;
  CcString m_Request;
  CcString m_Referer;
  CcString m_TransferEncoding;
  CcString m_UserAgent;
};

#endif /* CcHttpRequHeader_H_ */
