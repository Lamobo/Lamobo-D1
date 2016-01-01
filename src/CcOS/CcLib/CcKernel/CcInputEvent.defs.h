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
 * @file     CcInputEvent.h
 * @brief    Class CcInputEvent
 */

/**
 * @def_group EVT Input Event Type Definitions
 * @{
 */
#define EVT_UNDEFINED   0
#define EVT_KEYBOARD    1
#define EVT_MOUSE       2
#define EVT_TOUCH       3
#define EVT_JOYSTICK    4
/** @} */

/**
 * @def_group EVV Input Event Codes Definitions
 * @{
 */

#define EVC_UNDEFINED   0
#define EVC_KEYDOWN     1

#define EVC_MOUSE_L_DOWN 1
#define EVC_MOUSE_R_DOWN 2
#define EVC_MOUSE_C_DOWN 3
#define EVC_MOUSE_L_UP   4
#define EVC_MOUSE_R_UP   5
#define EVC_MOUSE_C_UP   6
/** @} */

/**
 * @def_group EVV Input Event Values Definitions
 * @{
 */
#define EVC_UNDEFINED   0

#define EVC_KEY_0       1
#define EVC_KEY_1       2
#define EVC_KEY_2       3
#define EVC_KEY_3       4
#define EVC_KEY_4       5
#define EVC_KEY_5       6
#define EVC_KEY_6       7
#define EVC_KEY_7       8
#define EVC_KEY_8       9
#define EVC_KEY_9      10

#define EVC_KEY_A       0x01
#define EVC_KEY_B       0x01
#define EVC_KEY_C       0x01
#define EVC_KEY_D       0x01
#define EVC_KEY_E       0x01
#define EVC_KEY_F       0x01
#define EVC_KEY_G       0x01
#define EVC_KEY_H       0x01
#define EVC_KEY_I       0x01
#define EVC_KEY_J       0x01
#define EVC_KEY_K       0x01
#define EVC_KEY_L       0x01
#define EVC_KEY_M       0x01
#define EVC_KEY_N       0x01
#define EVC_KEY_O       0x01
#define EVC_KEY_P       0x01
#define EVC_KEY_Q       0x01
#define EVC_KEY_R       0x01
#define EVC_KEY_S       0x01
#define EVC_KEY_T       0x01
#define EVC_KEY_U       0x01
#define EVC_KEY_V       0x01
#define EVC_KEY_W       0x01
#define EVC_KEY_X       0x01
#define EVC_KEY_Y       0x01
#define EVC_KEY_Z       0x01

#define EVC_KEY_NUM_0      0x01
#define EVC_KEY_NUM_1      0x01
#define EVC_KEY_NUM_2      0x01
#define EVC_KEY_NUM_3      0x01
#define EVC_KEY_NUM_4      0x01
#define EVC_KEY_NUM_5      0x01
#define EVC_KEY_NUM_6      0x01
#define EVC_KEY_NUM_7      0x01
#define EVC_KEY_NUM_8      0x01
#define EVC_KEY_NUM_9      0x01
#define EVC_KEY_NUM_PLUS   0x01
#define EVC_KEY_NUM_MINUS  0x01
#define EVC_KEY_NUM_DIV    0x01
#define EVC_KEY_NUM_MUL    0x01
#define EVC_KEY_NUM_ENTER  0x01
#define EVC_KEY_NUM_DOT    0x01
/** @}*/
