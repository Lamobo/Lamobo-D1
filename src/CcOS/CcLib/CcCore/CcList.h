/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcList.h
 * @brief    Class CcList
 */
#ifndef CCLIST_H_
#define CCLIST_H_

#include "CcBase.h"
#include <vector>
#include <list>

/**
 * @brief List-class with vector as base.
 */
template <typename TYPE>
class CcList {
public:
  /**
   * @brief Constructor
   */
  CcList(){}
  /**
   * @brief Destructor
   */
  virtual ~CcList(){clear();}

  /**
   * @brief Add an Object at the end of list
   *
   * @param toAppend: Object to add
   */
  void append( TYPE toAppend ){
    m_List.push_back(toAppend);
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
  TYPE at(uint32 uiPos ){
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
  void deleteAt(uint32 uiPos){
    m_List.erase(m_List.begin() + uiPos);
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
  void insertAt(uint32 uiPos, TYPE item){
    m_List.insert(m_List.begin() +uiPos, item);
  }

  /**
   * @brief check if item is allready added to List
   * @return true if list contains item, otherwise false
   */
  bool contains(TYPE item){
    for (size_t i = 0; i < size(); i++)
    { 
      if (item == at(i))
        return true;
    }
    return false;
  }

  /**
   * @brief Get Next Item and iterate to next.
   * @return Element on next Position
   */
  TYPE getNext(void){
    return m_List.pop_back();
  }

  /**
  * @brief Get Next Item and iterate to next.
  * @return
  */
  void toBegin(void){
    m_It = m_List.begin();
  }

  TYPE operator[](uint32 uiPos){ return at(uiPos);}
  void operator+(TYPE item) { append(item);}
  void operator-(TYPE item) { deleteItem(item);}
private:
  std::list<TYPE> m_List; ///< vector with saved Items
  typename std::list<TYPE>::iterator m_It; ///< vector with saved Items
};

#endif /* CCLIST_H_ */
