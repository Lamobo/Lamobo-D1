/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcString.h
 * @brief    Class CcString
 */
#ifndef CCSTRING_H_
#define CCSTRING_H_

#include "CcBase.h"
#include "CcTypes.h"
#include "CcCharArray.h"
#include <string>
#include <cstring>

/**
 * @brief String-class for basic string handling
 *
 *  The class is based on std::string and adds some usefull functions
 */
class CcString {
private:

public: //methods
  /**
   * @brief Create a empty string-class.
   */
  CcString();

  /**
   * @brief Copyconstructor
   */
   CcString(const CcString& rhs);

  /**
   * @brief Create a String-class with initialized const char array
   * @param cString: pointer to char array containing a null terminated string
   */
  CcString(const char* cString);

  /**
  * @brief Create a String-class with initialized std string
  * @param stdString: 
  */
  CcString(std::string stdString);

  /**
   * @brief Create a String-class with an initialized string of variable length
   * @param cString:  pointer to char array to be inserted
   * @param uiLength: Size of char-string to be stored in class
   */
  CcString(char* cString, size_t uiLength=SIZE_MAX);

  /**
   * @brief Clean up and free all requested Memory
   */
  virtual ~CcString();

  /**
   * @brief Get internal String for Operations
   * @return internal std::string
   */
  std::string &getStdString(void);

  /**
  * @brief Get a needle of String stack
  * @param pos: start position
  * @param len: length of string, or 0 until end of String
  * @return Needle, or "" if failed
  */
  CcString substr(size_t pos, size_t len = 0);

  /**
  * @brief Replace every needle with other value;
  * @param needle: String to find in Haystack
  * @param replac: String replaces the needle;
  * @return Needle, or "" if failed
  */
  CcString &strReplace(CcString needle, CcString replace);

  /**
  * @brief Get String between two strings
  * @param preStr: String to find
  * @param posStr: first String to find after position of preStr
  * @param offset: offset for begin of searching, default 0
  * @param enpos [out]: position of found string if required
  * @return Needle, or "" if failed
  */
  CcString getStringBetween(CcString preStr, CcString postStr, size_t offset = 0, size_t *pos=0);

  /**
   * @brief Compare a String with content if they are the same
   * @param sToCompare: String to compare to
   * @param bSensitivity: true if comparing with ignored sensitivity
   * @return true if same, otherwise false
   */
  bool compare( CcString sToCompare, bool bSensitivity = false);

  size_t findLast(CcString sToFind);

  size_t contains(CcString sToFind, size_t offset=0);

  bool begins(CcString sToCompare);

  /**
   * @brief Convert string into a unsigned int 32bit
   * @param bOk: is set to true if conversion was successfully, otherwise false
   * @return returns the converted value, or 0 if conversion fails
   */
  uint32 toUint32(bool *bOk = 0);

  /**
  * @brief Convert string into a unsigned int 16bit
  * @param bOk: is set to true if conversion was successfully, otherwise false
  * @return returns the converted value, or 0 if conversion fails
  */
  uint16 toUint16(bool *bOk = 0);

  /**
   * @brief Compare a String with content if they are the same
   * @param number: value to add
   * @return true if conversion was successful, otherwise false
   */
  CcString& appendNumber(int32 number);

  /**
   * @brief Compare a String with content if they are the same
   * @param number: value to add
   * @return true if conversion was successful, otherwise false
   
  CcString& appendNumber(float number);*/

  /**
   * @brief Append a CcString
   * @param toAppend: String to append to existing String
   */
  CcString& append(CcString &toAppend);

  /**
  * @brief Append a char String
  * @param toAppend: null terminated char array;
  */
  CcString& append(const char* toAppend);

  /**
  * @brief Append a sincle Character
  * @param toAppend: null terminated char array;
  */
  CcString& append(const char toAppend);

  /**
  * @brief Append a sincle Character
  * @param toAppend: null terminated char array;
  */
  CcString& append(char *toAppend, size_t length);

  /**
  * @brief Append a sincle Character
  * @param toAppend: null terminated char array;
  */
  CcString& append(CcCharArray &toAppend, size_t pos = 0, size_t length = SIZE_MAX);

  /**
  * @brief Append a std String
  * @param toAppend: null terminated char array;
  */
  CcString& append(std::string &toAppend);

  /**
  * @brief Append a std String
  * @param toAppend: null terminated char array;
  */
  CcString& appendIp(ipv4_t &ipAddr);

  /**
  * @brief Append a std String
  * @param toAppend: null terminated char array;
  */
  CcString& insert(size_t pos, CcString toInsert);
  /**
   * @brief Get Length of String
   * @return String-lenth
   */
  size_t length( void );

  /**
   * @brief Get char at position
   * @param pos: Position of target
   * @return char at pos
   */
  char at(size_t pos);

  /**
   * @brief get reference to this String
   * @return pointer to String
   */
  CcString *getString(void);

  void clear( void ){m_String.clear();}

  CcString &erase(size_t pos = 0, size_t len = SIZE_MAX);

  char* getCharString( void );

  CcString getLine(size_t offset = 0);

  CcString &normalizePath(void);
  CcString extractFilename(void);
  CcString calcPathAppend(CcString &toAppend);

  CcString &trimL(void);
  CcString &trimR(void);
  CcString &trim(void);

  char operator[](size_t pos){
    return at(pos);
  }
  CcString &operator+=(CcString toAdd){ return append(toAdd); }
  CcString &operator+=(const char *toAdd){ return append(toAdd); }
  CcString operator+(CcString toAdd);
  CcString operator+(const char *toAdd);
  CcString operator=(const char *toAdd){ clear();  return append(toAdd);}
  CcString operator=(char *toAdd){ clear(); return append(toAdd);}
  CcString &operator=(CcString rhs);
  bool operator!=(CcString toCompare){ return !compare(toCompare); }
  bool operator==(CcString toCompare){ return compare(toCompare); }
  bool operator<(CcString toCompare);
  bool operator>(CcString toCompare);
protected:
  std::string m_String;
  size_t m_LineOffset;
};

typedef struct{
  CcString name;
  CcString value;
} CcStringPair;


#endif /* CCSTRING_H_ */
