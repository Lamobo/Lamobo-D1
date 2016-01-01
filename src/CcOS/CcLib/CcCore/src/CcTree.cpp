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
