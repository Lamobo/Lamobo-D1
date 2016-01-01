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
 * @file     WindowsTimer.h
 * @brief    Class WindowsTimer
 **/
#ifndef WindowsTimer_H_
#define WindowsTimer_H_

#include "CcBase.h"
#include "dev/CcTimer.h"

class WindowsTimer : public CcTimer {
public: //methods
  WindowsTimer();
  virtual ~WindowsTimer();


  static void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);

  static void tick(void);
  bool open(uint16 flags);
  bool close(void);
  size_t read(char* buffer, size_t size);
  size_t write(char* buffer, size_t size);

private: //methods
  static uint32 getCounterState(void);

private: //member
  static uint32 s_CountDown;
};

#endif /* WindowsTimer_H_ */
