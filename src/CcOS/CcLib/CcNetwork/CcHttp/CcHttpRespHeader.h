/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 * @file     CcHttpRespHeader.h
 * @brief    Class CcHttpRespHeader
 */
#ifndef CcHttpRespHeader_H_
#define CcHttpRespHeader_H_

#include "CcBase.h"
#include "CcString.h"

/**
 * @brief Button for GUI Applications
 */
class CcHttpRespHeader {
public:
  /**
   * @brief Constructor
   */
  CcHttpRespHeader(CcString Parse);
  CcHttpRespHeader(bool init = false);

  /**
   * @brief Destructor
   */
  virtual ~CcHttpRespHeader(void);

  CcString getHeader();

  void parse(CcString& Parse);

private:
  void parseLine(CcString& Parse);
public:
  CcString m_AcceptRanges;
  CcString m_Allow;
  CcString m_CacheControl;
  CcString m_Connection;
  CcString m_ContentEncoding;
  CcString m_ContentLength;
  CcString m_ContentLocation;
  CcString m_ContentMd5;
  CcString m_ContentRange;
  CcString m_ContentType;
  CcString m_Date;
  CcString m_HTTP;
  CcString m_LastModified;
  CcString m_Location;
  CcString m_Refresh;
  CcString m_Server;
  CcString m_SetCookie;
  CcString m_TransferEncoding;
};

#endif /* CcHttpRespHeader_H_ */
