/*
 * CcCalibrationButton.h
 *
 *  Created on: 13.09.2015
 *      Author: Andreas
 */

#ifndef CCCALIBRATIONBUTTON_H_
#define CCCALIBRATIONBUTTON_H_

#include "CcButton.h"
#include "CcCross.h"
#include "CcPos.h"
#include "dev/CcTouch.h"
#include "CcText.h"

/**
 * @brief Structure of Data defined for calculating a 3-Point Calibration
 */
typedef struct{
  struct {
    int16 X1;  ///< X value from first Cross on Display for calibration
    int16 Y1;  ///< Y value from first Cross on Display for calibration
    int16 X2;  ///< X value from second Cross on Display for calibration
    int16 Y2;  ///< Y value from second Cross on Display for calibration
    int16 X3;  ///< X value from third Cross on Display for calibration
    int16 Y3;  ///< Y value from third Cross on Display for calibration
  } display;
  struct {
    int16 X1;  ///< X value from first Cross on Display for calibration
    int16 Y1;  ///< Y value from first Cross on Display for calibration
    int16 X2;  ///< X value from second Cross on Display for calibratio
    int16 Y2;  ///< Y value from second Cross on Display for calibratio
    int16 X3;  ///< X value from third Cross on Display for calibration
    int16 Y3;  ///< Y value from third Cross on Display for calibration
  } touch;
} sCcTouchCalibrationData;

class CcCalibrationButton: public CcButton {
public:
  CcCalibrationButton(CcWidget *parent);
  virtual ~CcCalibrationButton();

  void draw( void );
  void drawButton( void );
  void drawText( void );
  void onClick(CcInputEvent* event);
  void fillCalibData( void );
  void calcCalibration( void );
  void registerOnDone(CcObject *object, uint8 nr);
  void onClick(CcPos *pos);
  CcPos simulateCalibration(CcPos input);

private:
  CcText  m_TextWidget;
  CcCross m_cross;
  uint8   m_buttonNr;
  CcPos   m_PosAbsolute;
  CcPos   m_PosRelative;
  CcPos   Pos1;
  CcPos   Pos2;
  CcPos   Pos3;
  sCcTouchCalibrationData m_calibData;
  sCcTouchMatrix m_CalibMatrix;
  bool    m_Done;
  CcObjectHandler m_onDone;
};

#endif /* CCCALIBRATIONBUTTON_H_ */
