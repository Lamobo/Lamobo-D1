/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcString.h
 * @brief    Implementation of class CcString
 */
#include "CcString.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>

#ifndef WIN32
  #define TO_STRING_DEPRECATED
#endif
CcString::CcString() :
  m_String(""),
  m_LineOffset(0)
{
}
// Copyconstructor
CcString::CcString(const CcString& rhs)
{
  if (this!=&rhs)
  {
    append(const_cast<CcString&>(rhs));
  }
}

CcString::CcString(const char* cString) :
  m_String(cString)
{
}

CcString::CcString(std::string stdString) :
  m_String(stdString)
{
}

CcString::CcString(char* cString, size_t uiLength){
  if(uiLength != SIZE_MAX){
    char *cArray = new char[uiLength+1];
    memcpy(cArray, cString, uiLength);
    cArray[uiLength] = '\n';
    append(cArray);
  }
  else{
    size_t temp = strlen(cString);
    append(cString);
  }
}

CcString::~CcString() {
}


std::string &CcString::getStdString(void){
  return m_String;
}

CcString CcString::substr(size_t pos, size_t len){
  if (len != 0)
    return CcString(m_String.substr(pos, len));
  else
    return CcString(m_String.substr(pos, std::string::npos));
}

CcString &CcString::strReplace(CcString needle, CcString replace){
  size_t pos=0;
  while (pos < m_String.length())
  {
    pos = contains(needle, pos);
    if (pos != SIZE_MAX){
      erase(pos, needle.length());
      insert(pos, replace);
    }
  }
  return *this;
}

CcString CcString::getStringBetween(CcString preStr, CcString postStr, size_t offset, size_t *pos){
  CcString sRet;
  size_t posFirst = m_String.find_first_of(preStr.getStdString(), offset) ;
  if (posFirst != std::string::npos)
  {
    posFirst += preStr.length();
    size_t posSecond = m_String.find_first_of(postStr.getStdString(), posFirst);
    if (posSecond != std::string::npos)
    {
      size_t len = posSecond - posFirst;
      std::string temp = m_String.substr(posFirst, len);
      sRet.append(temp);
      if (pos != 0)
        *pos = posFirst + preStr.length();
    }
  }
  return sRet;
}

bool CcString::compare( CcString sToCompare, bool bSensitivity ){
  if(bSensitivity==0)
  {
    if(strcmp(m_String.c_str(), sToCompare.getStdString().c_str()))
      return false;
    return true;
  }
  else //TODO Caseinsensitive mode
    return false;
}

size_t CcString::findLast(CcString sToFind){
  size_t iRet = SIZE_MAX;
  size_t temp=0;
  while ((temp = contains(sToFind, temp)) != SIZE_MAX){
    iRet = temp;
    temp++;
  }
  return iRet;
}

size_t CcString::contains(CcString sToFind, size_t offset){
  return m_String.find(sToFind.getStdString(), offset);
}

bool CcString::begins(CcString sToCompare){
  if (sToCompare.length() > m_String.length()){
    return false;
  }
  else{
    for (size_t i = 0; i < sToCompare.length(); i++){
      if (sToCompare.at(i) != m_String.at(i))
        return false;
    }
  }
  return true;
}

uint32 CcString::toUint32( bool *bOk){
  uint32 uiRet = atoi(m_String.c_str());
  if (bOk!=0)
    *bOk=true;
  return uiRet;
}

uint16 CcString::toUint16(bool *bOk){
  uint16 uiRet = 0x0000ffff & atoi(m_String.c_str());
  if (bOk != 0)
    *bOk=true;
  return uiRet;
}


CcString& CcString::appendNumber(int32 number)
{
#ifdef TO_STRING_DEPRECATED
  char temp[20];
  sprintf(temp, "%d", number);
  m_String.append(temp);
#else
  std::string toAdd = std::to_string( number );
  m_String.append(toAdd);
#endif
  return *this;
}
/*
bool CcString::appendNumber(float number)
{
#ifdef TO_STRING_DEPRECATED
  char temp[20];
  sprintf(temp, "%f", number);
  m_String.append(temp);
#else
  std::string toAdd = std::to_string(number);
  m_String.append(toAdd);
#endif
  return true;
}*/

size_t CcString::length( void )
{
  return m_String.length();
}

char CcString::at(size_t pos)
{
  return m_String.at(pos);
}

char* CcString::getCharString(void ) {
  char* ret= (char*)m_String.c_str();
  return ret;
}

CcString CcString::getLine(size_t offset){
  CcString sRet;
  bool bDone = false;
  if (offset == 0)
  {
    offset = m_LineOffset;
  }
  while (offset < length() && bDone == false){
    if (at(offset) == '\n'){
      bDone = true;
    }
    else if (at(offset) == '\r'){
      bDone = true;
      if (offset + 1 < length() && at(offset + 1) == '\n')
        offset++;
    }
    else
      sRet.append(at(offset));
    offset++;
  }
  m_LineOffset = offset;
  return sRet;
}

CcString &CcString::normalizePath(void){
  strReplace("\\", "/");
  return *this;
}

CcString CcString::extractFilename(void){
  CcString sRet;
  size_t pos1 = findLast("/");
  size_t pos2 = findLast("\\");
  size_t subStrPos = SIZE_MAX;
  if (pos1 != SIZE_MAX){
    if (pos2 != SIZE_MAX){
      if (pos1 > pos2)
        subStrPos = pos1;
      else
        subStrPos = pos2;
    }
    else{
      subStrPos = pos1;
    }
  }
  else{
    subStrPos = pos2;
  }
  if (subStrPos != SIZE_MAX)
    sRet = substr(subStrPos);
  else 
    sRet = *this;
  return sRet;
}

CcString CcString::calcPathAppend(CcString &toAppend){
  CcString sRet;
  CcString seperator = "";
  if (at(length() - 1) != '/'){
    seperator = "/";
  }
  if (toAppend.begins("/") || toAppend.at(1) == ':'){
    sRet = toAppend;
  }
  else{
    if (toAppend.begins("./"))
      sRet = *this + seperator + toAppend.substr(2);
    else
      sRet = *this + seperator + toAppend;
  }
  return sRet;
}

CcString &CcString::trimL(void){
  size_t pos = 0;
  if (length() > 0)
  {
    while (at(pos) == ' ' ||
      at(pos) == '\r' ||
      at(pos) == '\n' ||
      at(pos) == '\t')
    {
      m_String.erase(m_String.begin());
      pos++;
      if (pos >= m_String.length()) break;
    }
  }
  return *this;
}

CcString &CcString::trimR(void){
  size_t pos = length() - 1;
  while (pos < length() && (
    at(pos) == ' ' ||
    at(pos) == '\r' ||
    at(pos) == '\n' ||
    at(pos) == '\t'))
  {
    m_String.erase(pos);
    pos--;
  }
  return *this;
}

CcString &CcString::trim(void){
  trimL();
  trimR();
  return *this;
}

CcString& CcString::append(CcString &toAppend){
  m_String.append(toAppend.getStdString().c_str());
  return *this;
}

CcString& CcString::append(const char* toAppend){
  m_String.append(toAppend);
  return *this;
}

CcString& CcString::append(const char toAppend){
  m_String += toAppend;
  return *this;
}

CcString& CcString::append(char *toAppend, size_t length){
  m_String.insert(m_String.end(), toAppend, toAppend + length);
  return *this;
}

CcString& CcString::append(CcCharArray &toAppend, size_t pos, size_t len){
  if (len == SIZE_MAX)
    len = toAppend.size()-pos;
  char* arr = toAppend.getContent(pos, len);
  m_String.append(arr, len);
  return *this;
}

CcString& CcString::append(std::string &toAppend){
  m_String.append(toAppend);
  return *this;
}

CcString& CcString::appendIp(ipv4_t &ipAddr){
  appendNumber(ipAddr.ip4);
  append('.');
  appendNumber(ipAddr.ip3);
  append('.');
  appendNumber(ipAddr.ip2);
  append('.');
  appendNumber(ipAddr.ip1);
  return *this;
}

CcString& CcString::insert(size_t pos, CcString toInsert){
  m_String.insert(pos, toInsert.getStdString());
  return *this;
}

CcString *CcString::getString(void){
  return this;
}

CcString &CcString::erase(size_t pos, size_t len){
  if (len == SIZE_MAX)
    len = length();
  m_String.erase(pos, len);
  return *this;
}

CcString CcString::operator+(CcString toAdd){ 
  CcString ret = *this;
  ret.append(toAdd);
  return ret; 
}

CcString CcString::operator+(const char *toAdd){
  CcString ret = *this;
  ret.append(toAdd);
  return ret;
}

bool CcString::operator<(CcString toCompare){
  if (m_String < toCompare.getStdString())
    return true;
  return false;
}

bool CcString::operator>(CcString toCompare){
  if (m_String > toCompare.getStdString())
    return true;
  return false;
}

// Assignmentoperator
CcString& CcString::operator=(CcString rhs)
{
  clear();
  append((rhs));
  return(*this);
}
