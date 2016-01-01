#include "CcCrossButton.h"
#include "CcPainter.h"

CcCrossButton::CcCrossButton(CcWidget *parent):
  CcButton(parent)
{

}

CcCrossButton::~CcCrossButton()
{

}

void CcCrossButton::setCrossColor(uint8 R, uint8 G, uint8 B){
  m_R = R;
  m_G = G;
  m_B = B;
}

void CcCrossButton::setCrossSize(uint8 size){
  m_CrossSize = size;
}

void CcCrossButton::drawCross( void ){
  uint16 startX = (m_sizeX - m_CrossSize)/2;
  uint16 startY = (m_sizeY - m_CrossSize)/2;
  for(uint16 i=0; i < m_CrossSize; i++)
  {
    CcPainter Painter(getWindow());
    Painter.setColor(m_R, m_G, m_B);
    Painter.drawLine(startX+i, 0, startX+i, m_sizeX-1);
    Painter.drawLine(0, startY+i, m_sizeY-1, startX+i);
  }
}
