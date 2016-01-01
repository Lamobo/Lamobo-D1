/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcTree.cpp
* @brief    Implementation of Class CcTreeAv
*/
#include "CcTree.h"

CcTree::CcTree(void)
{}

CcTree::~CcTree(void)
{
  clear();
}

void CcTree::addSubTree(CcTree *toAdd){
  m_TreeList.push_back(toAdd);
}

CcTree* CcTree::getNext(void){
  CcTree *ret(0);
  if (m_It != m_TreeList.end()){
    m_It++;
    ret = *m_It;
  }
  return ret;
}

CcTree* CcTree::getAt(uint32 pos){
  if (pos < m_TreeList.size()){
    std::list<CcTree*>::iterator it = m_TreeList.begin();
    std::advance(it, pos);
    return *it;
  }
  else
    return 0;
}

CcTree* CcTree::begin( void ){
  m_It = m_TreeList.begin();
  return *m_It;
}

void CcTree::delSubTree(CcTree* toDel){
  m_TreeList.remove(toDel);
}

void CcTree::clear(void){
  while (!m_TreeList.empty())
  {
    m_TreeList.back()->clear();
    m_TreeList.pop_back();
  }
}

size_t CcTree::size(void){
  return m_TreeList.size();
}
