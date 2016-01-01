/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcTree.h
* @brief    Class CcTree
*/
#ifndef CCTREE_H_
#define CCTREE_H_

#include "CcBase.h"
#include "stdlib.h"
#include <list>

/**
* @brief Button for GUI Applications
*/
class CcTree {
public:
  /**
  * @brief Constructor
  */
  CcTree(void);

  /**
  * @brief Destructor
  */
  virtual ~CcTree(void);

  void addSubTree( CcTree *toAdd );

  CcTree *getNext(void);

  CcTree *getAt(uint32 pos);

  CcTree *begin();

  void delSubTree(CcTree* toDel);

  void clear(void);

  size_t size(void);

private:
  std::list<CcTree*> m_TreeList;
  std::list<CcTree*>::iterator m_It;
};

#endif /* CCTREE_H_ */
