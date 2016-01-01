#ifndef CCCROSSBUTTON_H
#define CCCROSSBUTTON_H

#include "CcBase.h"
#include "CcWidget.h"
#include "CcButton.h"

class CcCrossButton : public CcButton
{
public:
  CcCrossButton(CcWidget *parent);
  virtual ~CcCrossButton();

  void setCrossColor(uint8 R, uint8 G, uint8 B);
  void setCrossSize(uint8 size);
  void drawCross( void );

private:
  uint8 m_R;
  uint8 m_G;
  uint8 m_B;
  uint8 m_CrossSize;
};

#endif // CCCROSSBUTTON_H
