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
 * @file     CcTimer.h
 * @brief    Class CcTimer
 */

#ifndef CCTIMER_H_
#define CCTIMER_H_

#include "CcBase.h"
#include "CcIODevice.h"

class CcTimer : public CcIODevice{
public: //methods
  CcTimer();
  virtual ~CcTimer();

  static void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);

  static void tick( void );

  virtual bool open(uint16 flags);
  virtual bool close(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);

private: //methods
  static uint32 getCounterState(void);

private: //member
  static volatile uint32 s_CountDown;
};

#endif /* CCTIMER_H_ */
