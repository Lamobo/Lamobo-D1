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
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 * @file     CcVector.h
 * @brief    Class CcVector
 */
#ifndef CCVECTOR_H_
#define CCVECTOR_H_

#include "CcBase.h"
#include <vector>

/**
 * @brief List-class with vector as base.
 */
template <typename TYPE>
class CcVector {
public:
  /**
   * @brief Constructor
   */
  CcVector() : m_Buffer(0){}

  /**
  * @brief Constructor
  * @param item: Item to add on load
  */
  CcVector(TYPE item){append(item);}

  /**
  * @brief Constructor
  * @param items: Pointer to Items to add on load
  * @param number: Count of Items on Pointer
  */
  CcVector(TYPE* items, size_t count) {append(items, count);}

  /**
   * @brief Destructor
   */
  virtual ~CcVector(){
    clear();
    if (m_Buffer != 0)
      delete m_Buffer;
  }

  /**
   * @brief Add an Object at the end of list
   *
   * @param toAppend: Object to add
   */
  void append(CcVector<TYPE> toAppend){
    m_List.insert(m_List.end(), toAppend.getStdVector()->begin(), toAppend.getStdVector()->end());
  }

  /**
  * @brief Add an Object at the end of list
  *
  * @param toAppend: Object to add
  */
  void append(TYPE toAppend){
    m_List.push_back(toAppend);
  }

  void append(TYPE *toAppend, size_t count){
    m_List.insert(m_List.end(), toAppend, toAppend + count);
  }

  /**
   * @brief Get the number of items containing in list.
   *
   * @return Number of Items
   */
  size_t size( void ){
    return m_List.size();
  }

  /**
   * @brief Get the Object stored at requested position
   *
   * @param uiPos: position of requested Object, must be lower than size()
   * @return requested Object
   */
  TYPE &at(size_t uiPos ){
    return m_List.at(uiPos);
  }

  /**
   * @brief Deletes all entries in list.
   */
  void clear( void ){
    m_List.clear();
  }

  /**
   * @brief Delete Item on defined Position
   * @param uiPos: Position of Item
   */
  void deleteAt(size_t uiPos, size_t len = SIZE_MAX){
    if (len == SIZE_MAX)
    {
      m_List.erase(m_List.begin() + uiPos);
    }
    else
    {
      m_List.erase(m_List.begin() + uiPos, m_List.begin() + uiPos + len);
    }
  }
  
  /**
   * @brief Delete a specific Item in List
   * @param item: item to delete
   */
  void deleteItem( TYPE item){
    for(uint32 i=0; i<size(); i++)
      if(at(i) == item)
        deleteAt(i);
  }

  /**
   * @brief Insert a Item at a defined Position.
   * @param uiPos: Position to store at
   * @param item: Item to store
   */
  void insertAt(size_t uiPos, TYPE item){
    m_List.insert(m_List.begin() +uiPos, item);
  }

  /**
   * @brief check if item is allready added to List
   * @return true if list contains item, otherwise false
   */
  size_t contains(TYPE item){
    size_t i;
    for (i = 0; i < size(); i++)
    { 
      if (item == at(i))
        return i;
    }
    return SIZE_MAX;
  }

  /**
  * @brief check if item is allready added to List
  * @return true if list contains item, otherwise false
  */
  size_t contains(CcVector<TYPE> list){
    size_t iRet = SIZE_MAX;
    bool bFound(false);
    if (size() >= list.size())
    {
      size_t length = size()-(list.size()-1);
      for (size_t i = 0; i < length && bFound == false; i++)
      {
        if (at(i) == list.at(0))
        {
          size_t j = 0;
          for (; j < list.size(); j++)
          {
            if (list.at(j) != at(i + j))
              break;
          }
          if (j == list.size())
          {
            bFound = true;
            iRet = i;
          }
        }
      }
    }
    return iRet;
  }

  TYPE *getContent(size_t pos = 0, size_t len = SIZE_MAX){
    if (len == SIZE_MAX)
      len = size()-pos;
    if (m_Buffer != NULL)
      delete m_Buffer;
    m_Buffer = new TYPE[len];
    for (size_t i = pos; i < len; i++){
      m_Buffer[i] = at(i);
    }
    return m_Buffer;
  }

  std::vector<TYPE> *getStdVector(){ return &m_List; }

  TYPE operator[](size_t uiPos){ return at(uiPos);}
  void operator+(TYPE item) { append(item);}
  void operator-(TYPE item) { deleteItem(item);}

private:
  std::vector<TYPE> m_List; ///< vector with saved Items
  TYPE* m_Buffer;
};

#endif /* CCVECTOR_H_ */
