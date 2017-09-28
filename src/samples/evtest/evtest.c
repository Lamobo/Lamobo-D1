/*
 *  Copyright (c) 1999-2000 Vojtech Pavlik
 *  Copyright (c) 2009-2011 Red Hat, Inc
 */

/**
 * @file
 * Event device test program
 *
 * evtest prints the capabilities on the kernel devices in /dev/input/eventX
 * and their events. Its primary purpose is for kernel or X driver
 * debugging.
 *
 * See INSTALL for installation details or manually compile with
 * gcc -o evtest evtest.c
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */

#define _GNU_SOURCE /* for asprintf */
#include <stdio.h>
#include <stdint.h>

#include <linux/version.h>
#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"

#ifndef EV_SYN
#define EV_SYN 0
#endif
#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT 2
#endif

char *events[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN] = "Sync",			[EV_KEY] = "Key",
	[EV_REL] = "Relative",			[EV_ABS] = "Absolute",
	[EV_MSC] = "Misc",			[EV_LED] = "LED",
	[EV_SND] = "Sound",			[EV_REP] = "Repeat",
	[EV_FF] = "ForceFeedback",		[EV_PWR] = "Power",
	[EV_FF_STATUS] = "ForceFeedbackStatus",	[EV_SW] = "Switch",
};

char *keys[KEY_MAX + 1] = {
	[0 ... KEY_MAX] = NULL,
	[KEY_RESERVED] = "Reserved",		[KEY_ESC] = "Esc",
	[KEY_1] = "1",				[KEY_2] = "2",
	[KEY_3] = "3",				[KEY_4] = "4",
	[KEY_5] = "5",				[KEY_6] = "6",
	[KEY_7] = "7",				[KEY_8] = "8",
	[KEY_9] = "9",				[KEY_0] = "0",
	[KEY_MINUS] = "Minus",			[KEY_EQUAL] = "Equal",
	[KEY_BACKSPACE] = "Backspace",		[KEY_TAB] = "Tab",
	[KEY_Q] = "Q",				[KEY_W] = "W",
	[KEY_E] = "E",				[KEY_R] = "R",
	[KEY_T] = "T",				[KEY_Y] = "Y",
	[KEY_U] = "U",				[KEY_I] = "I",
	[KEY_O] = "O",				[KEY_P] = "P",
	[KEY_LEFTBRACE] = "LeftBrace",		[KEY_RIGHTBRACE] = "RightBrace",
	[KEY_ENTER] = "Enter",			[KEY_LEFTCTRL] = "LeftControl",
	[KEY_A] = "A",				[KEY_S] = "S",
	[KEY_D] = "D",				[KEY_F] = "F",
	[KEY_G] = "G",				[KEY_H] = "H",
	[KEY_J] = "J",				[KEY_K] = "K",
	[KEY_L] = "L",				[KEY_SEMICOLON] = "Semicolon",
	[KEY_APOSTROPHE] = "Apostrophe",	[KEY_GRAVE] = "Grave",
	[KEY_LEFTSHIFT] = "LeftShift",		[KEY_BACKSLASH] = "BackSlash",
	[KEY_Z] = "Z",				[KEY_X] = "X",
	[KEY_C] = "C",				[KEY_V] = "V",
	[KEY_B] = "B",				[KEY_N] = "N",
	[KEY_M] = "M",				[KEY_COMMA] = "Comma",
	[KEY_DOT] = "Dot",			[KEY_SLASH] = "Slash",
	[KEY_RIGHTSHIFT] = "RightShift",	[KEY_KPASTERISK] = "KPAsterisk",
	[KEY_LEFTALT] = "LeftAlt",		[KEY_SPACE] = "Space",
	[KEY_CAPSLOCK] = "CapsLock",		[KEY_F1] = "F1",
	[KEY_F2] = "F2",			[KEY_F3] = "F3",
	[KEY_F4] = "F4",			[KEY_F5] = "F5",
	[KEY_F6] = "F6",			[KEY_F7] = "F7",
	[KEY_F8] = "F8",			[KEY_F9] = "F9",
	[KEY_F10] = "F10",			[KEY_NUMLOCK] = "NumLock",
	[KEY_SCROLLLOCK] = "ScrollLock",	[KEY_KP7] = "KP7",
	[KEY_KP8] = "KP8",			[KEY_KP9] = "KP9",
	[KEY_KPMINUS] = "KPMinus",		[KEY_KP4] = "KP4",
	[KEY_KP5] = "KP5",			[KEY_KP6] = "KP6",
	[KEY_KPPLUS] = "KPPlus",		[KEY_KP1] = "KP1",
	[KEY_KP2] = "KP2",			[KEY_KP3] = "KP3",
	[KEY_KP0] = "KP0",			[KEY_KPDOT] = "KPDot",
	[KEY_ZENKAKUHANKAKU] = "Zenkaku/Hankaku", [KEY_102ND] = "102nd",
	[KEY_F11] = "F11",			[KEY_F12] = "F12",
	[KEY_RO] = "RO",			[KEY_KATAKANA] = "Katakana",
	[KEY_HIRAGANA] = "HIRAGANA",		[KEY_HENKAN] = "Henkan",
	[KEY_KATAKANAHIRAGANA] = "Katakana/Hiragana", [KEY_MUHENKAN] = "Muhenkan",
	[KEY_KPJPCOMMA] = "KPJpComma",		[KEY_KPENTER] = "KPEnter",
	[KEY_RIGHTCTRL] = "RightCtrl",		[KEY_KPSLASH] = "KPSlash",
	[KEY_SYSRQ] = "SysRq",			[KEY_RIGHTALT] = "RightAlt",
	[KEY_LINEFEED] = "LineFeed",		[KEY_HOME] = "Home",
	[KEY_UP] = "Up",			[KEY_PAGEUP] = "PageUp",
	[KEY_LEFT] = "Left",			[KEY_RIGHT] = "Right",
	[KEY_END] = "End",			[KEY_DOWN] = "Down",
	[KEY_PAGEDOWN] = "PageDown",		[KEY_INSERT] = "Insert",
	[KEY_DELETE] = "Delete",		[KEY_MACRO] = "Macro",
	[KEY_MUTE] = "Mute",			[KEY_VOLUMEDOWN] = "VolumeDown",
	[KEY_VOLUMEUP] = "VolumeUp",		[KEY_POWER] = "Power",
	[KEY_KPEQUAL] = "KPEqual",		[KEY_KPPLUSMINUS] = "KPPlusMinus",
	[KEY_PAUSE] = "Pause",			[KEY_KPCOMMA] = "KPComma",
	[KEY_HANGUEL] = "Hanguel",		[KEY_HANJA] = "Hanja",
	[KEY_YEN] = "Yen",			[KEY_LEFTMETA] = "LeftMeta",
	[KEY_RIGHTMETA] = "RightMeta",		[KEY_COMPOSE] = "Compose",
	[KEY_STOP] = "Stop",			[KEY_AGAIN] = "Again",
	[KEY_PROPS] = "Props",			[KEY_UNDO] = "Undo",
	[KEY_FRONT] = "Front",			[KEY_COPY] = "Copy",
	[KEY_OPEN] = "Open",			[KEY_PASTE] = "Paste",
	[KEY_FIND] = "Find",			[KEY_CUT] = "Cut",
	[KEY_HELP] = "Help",			[KEY_MENU] = "Menu",
	[KEY_CALC] = "Calc",			[KEY_SETUP] = "Setup",
	[KEY_SLEEP] = "Sleep",			[KEY_WAKEUP] = "WakeUp",
	[KEY_FILE] = "File",			[KEY_SENDFILE] = "SendFile",
	[KEY_DELETEFILE] = "DeleteFile",	[KEY_XFER] = "X-fer",
	[KEY_PROG1] = "Prog1",			[KEY_PROG2] = "Prog2",
	[KEY_WWW] = "WWW",			[KEY_MSDOS] = "MSDOS",
	[KEY_COFFEE] = "Coffee",		[KEY_DIRECTION] = "Direction",
	[KEY_CYCLEWINDOWS] = "CycleWindows",	[KEY_MAIL] = "Mail",
	[KEY_BOOKMARKS] = "Bookmarks",		[KEY_COMPUTER] = "Computer",
	[KEY_BACK] = "Back",			[KEY_FORWARD] = "Forward",
	[KEY_CLOSECD] = "CloseCD",		[KEY_EJECTCD] = "EjectCD",
	[KEY_EJECTCLOSECD] = "EjectCloseCD",	[KEY_NEXTSONG] = "NextSong",
	[KEY_PLAYPAUSE] = "PlayPause",		[KEY_PREVIOUSSONG] = "PreviousSong",
	[KEY_STOPCD] = "StopCD",		[KEY_RECORD] = "Record",
	[KEY_REWIND] = "Rewind",		[KEY_PHONE] = "Phone",
	[KEY_ISO] = "ISOKey",			[KEY_CONFIG] = "Config",
	[KEY_HOMEPAGE] = "HomePage",		[KEY_REFRESH] = "Refresh",
	[KEY_EXIT] = "Exit",			[KEY_MOVE] = "Move",
	[KEY_EDIT] = "Edit",			[KEY_SCROLLUP] = "ScrollUp",
	[KEY_SCROLLDOWN] = "ScrollDown",	[KEY_KPLEFTPAREN] = "KPLeftParenthesis",
	[KEY_KPRIGHTPAREN] = "KPRightParenthesis", [KEY_F13] = "F13",
	[KEY_F14] = "F14",			[KEY_F15] = "F15",
	[KEY_F16] = "F16",			[KEY_F17] = "F17",
	[KEY_F18] = "F18",			[KEY_F19] = "F19",
	[KEY_F20] = "F20",			[KEY_F21] = "F21",
	[KEY_F22] = "F22",			[KEY_F23] = "F23",
	[KEY_F24] = "F24",			[KEY_PLAYCD] = "PlayCD",
	[KEY_PAUSECD] = "PauseCD",		[KEY_PROG3] = "Prog3",
	[KEY_PROG4] = "Prog4",			[KEY_SUSPEND] = "Suspend",
	[KEY_CLOSE] = "Close",			[KEY_PLAY] = "Play",
	[KEY_FASTFORWARD] = "Fast Forward",	[KEY_BASSBOOST] = "Bass Boost",
	[KEY_PRINT] = "Print",			[KEY_HP] = "HP",
	[KEY_CAMERA] = "Camera",		[KEY_SOUND] = "Sound",
	[KEY_QUESTION] = "Question",		[KEY_EMAIL] = "Email",
	[KEY_CHAT] = "Chat",			[KEY_SEARCH] = "Search",
	[KEY_CONNECT] = "Connect",		[KEY_FINANCE] = "Finance",
	[KEY_SPORT] = "Sport",			[KEY_SHOP] = "Shop",
	[KEY_ALTERASE] = "Alternate Erase",	[KEY_CANCEL] = "Cancel",
	[KEY_BRIGHTNESSDOWN] = "Brightness down", [KEY_BRIGHTNESSUP] = "Brightness up",
	[KEY_MEDIA] = "Media",			[KEY_UNKNOWN] = "Unknown",
	[KEY_OK] = "Ok",
	[KEY_SELECT] = "Select",		[KEY_GOTO] = "Goto",
	[KEY_CLEAR] = "Clear",			[KEY_POWER2] = "Power2",
	[KEY_OPTION] = "Option",		[KEY_INFO] = "Info",
	[KEY_TIME] = "Time",			[KEY_VENDOR] = "Vendor",
	[KEY_ARCHIVE] = "Archive",		[KEY_PROGRAM] = "Program",
	[KEY_CHANNEL] = "Channel",		[KEY_FAVORITES] = "Favorites",
	[KEY_EPG] = "EPG",			[KEY_PVR] = "PVR",
	[KEY_MHP] = "MHP",			[KEY_LANGUAGE] = "Language",
	[KEY_TITLE] = "Title",			[KEY_SUBTITLE] = "Subtitle",
	[KEY_ANGLE] = "Angle",			[KEY_ZOOM] = "Zoom",
	[KEY_MODE] = "Mode",			[KEY_KEYBOARD] = "Keyboard",
	[KEY_SCREEN] = "Screen",		[KEY_PC] = "PC",
	[KEY_TV] = "TV",			[KEY_TV2] = "TV2",
	[KEY_VCR] = "VCR",			[KEY_VCR2] = "VCR2",
	[KEY_SAT] = "Sat",			[KEY_SAT2] = "Sat2",
	[KEY_CD] = "CD",			[KEY_TAPE] = "Tape",
	[KEY_RADIO] = "Radio",			[KEY_TUNER] = "Tuner",
	[KEY_PLAYER] = "Player",		[KEY_TEXT] = "Text",
	[KEY_DVD] = "DVD",			[KEY_AUX] = "Aux",
	[KEY_MP3] = "MP3",			[KEY_AUDIO] = "Audio",
	[KEY_VIDEO] = "Video",			[KEY_DIRECTORY] = "Directory",
	[KEY_LIST] = "List",			[KEY_MEMO] = "Memo",
	[KEY_CALENDAR] = "Calendar",		[KEY_RED] = "Red",
	[KEY_GREEN] = "Green",			[KEY_YELLOW] = "Yellow",
	[KEY_BLUE] = "Blue",			[KEY_CHANNELUP] = "ChannelUp",
	[KEY_CHANNELDOWN] = "ChannelDown",	[KEY_FIRST] = "First",
	[KEY_LAST] = "Last",			[KEY_AB] = "AB",
	[KEY_NEXT] = "Next",			[KEY_RESTART] = "Restart",
	[KEY_SLOW] = "Slow",			[KEY_SHUFFLE] = "Shuffle",
	[KEY_BREAK] = "Break",			[KEY_PREVIOUS] = "Previous",
	[KEY_DIGITS] = "Digits",		[KEY_TEEN] = "TEEN",
	[KEY_TWEN] = "TWEN",			[KEY_DEL_EOL] = "Delete EOL",
	[KEY_DEL_EOS] = "Delete EOS",		[KEY_INS_LINE] = "Insert line",
	[KEY_DEL_LINE] = "Delete line",
	[KEY_VIDEOPHONE] = "Videophone",	[KEY_GAMES] = "Games",
	[KEY_ZOOMIN] = "Zoom In",		[KEY_ZOOMOUT] = "Zoom Out",
	[KEY_ZOOMRESET] = "Zoom Reset",		[KEY_WORDPROCESSOR] = "Word Processor",
	[KEY_EDITOR] = "Editor",		[KEY_SPREADSHEET] = "Spreadsheet",
	[KEY_GRAPHICSEDITOR] = "Graphics Editor", [KEY_PRESENTATION] = "Presentation",
	[KEY_DATABASE] = "Database",		[KEY_NEWS] = "News",
	[KEY_VOICEMAIL] = "Voicemail",		[KEY_ADDRESSBOOK] = "Addressbook",
	[KEY_MESSENGER] = "Messenger",		[KEY_DISPLAYTOGGLE] = "Display Toggle",
	[KEY_SPELLCHECK] = "Spellcheck",	[KEY_LOGOFF] = "Log Off",
	[KEY_DOLLAR] = "Dollar",		[KEY_EURO] = "Euro",
	[KEY_FRAMEBACK] = "Frame Back",	 [KEY_FRAMEFORWARD] = "Frame Forward",
	[KEY_CONTEXT_MENU] = "Context Menu",	[KEY_MEDIA_REPEAT] = "Media Repeat",
	[KEY_DEL_EOL] = "Delete EOL",		[KEY_DEL_EOS] = "Delete EOS",
	[KEY_INS_LINE] = "Insert Line",	 [KEY_DEL_LINE] = "Delete Line",
	[KEY_FN] = "Fn",			[KEY_FN_ESC] = "Fn Esc",
	[KEY_FN_F1] = "Fn F1",			[KEY_FN_F2] = "Fn F2",
	[KEY_FN_F3] = "Fn F3",			[KEY_FN_F4] = "Fn F4",
	[KEY_FN_F5] = "Fn F5",			[KEY_FN_F6] = "Fn F6",
	[KEY_FN_F7] = "Fn F7",			[KEY_FN_F8] = "Fn F8",
	[KEY_FN_F9] = "Fn F9",			[KEY_FN_F10] = "Fn F10",
	[KEY_FN_F11] = "Fn F11",		[KEY_FN_F12] = "Fn F12",
	[KEY_FN_1] = "Fn 1",			[KEY_FN_2] = "Fn 2",
	[KEY_FN_D] = "Fn D",			[KEY_FN_E] = "Fn E",
	[KEY_FN_F] = "Fn F",			[KEY_FN_S] = "Fn S",
	[KEY_FN_B] = "Fn B",
	[KEY_BRL_DOT1] = "Braille Dot 1",	[KEY_BRL_DOT2] = "Braille Dot 2",
	[KEY_BRL_DOT3] = "Braille Dot 3",	[KEY_BRL_DOT4] = "Braille Dot 4",
	[KEY_BRL_DOT5] = "Braille Dot 5",	[KEY_BRL_DOT6] = "Braille Dot 6",
	[KEY_BRL_DOT7] = "Braille Dot 7",	[KEY_BRL_DOT8] = "Braille Dot 8",
	[KEY_BRL_DOT9] = "Braille Dot 9",	[KEY_BRL_DOT10] = "Braille Dot 10",
	[KEY_NUMERIC_0] = "Numeric 0",		[KEY_NUMERIC_1] = "Numeric 1",
	[KEY_NUMERIC_2] = "Numeric 2",		[KEY_NUMERIC_3] = "Numeric 3",
	[KEY_NUMERIC_4] = "Numeric 4",		[KEY_NUMERIC_5] = "Numeric 5",
	[KEY_NUMERIC_6] = "Numeric 6",		[KEY_NUMERIC_7] = "Numeric 7",
	[KEY_NUMERIC_8] = "Numeric 8",		[KEY_NUMERIC_9] = "Numeric 9",
	[KEY_NUMERIC_STAR] = "Numeric *",	[KEY_NUMERIC_POUND] = "Numeric #",
	[KEY_BATTERY] = "Battery",
	[KEY_BLUETOOTH] = "Bluetooth",		[KEY_BRIGHTNESS_CYCLE] = "Brightness Cycle",
	[KEY_BRIGHTNESS_ZERO] = "Brightness Zero", [KEY_DASHBOARD] = "Dashboard",
	[KEY_DISPLAY_OFF] = "Display Off",	[KEY_DOCUMENTS] = "Documents",
	[KEY_FORWARDMAIL] = "Forward Mail",	[KEY_NEW]  = "New",
	[KEY_KBDILLUMDOWN] = "Kbd Illum Down",	[KEY_KBDILLUMUP] = "Kbd Illum Up",
	[KEY_KBDILLUMTOGGLE] = "Kbd Illum Toggle", [KEY_REDO] = "Redo",
	[KEY_REPLY] = "Reply",			[KEY_SAVE] = "Save",
	[KEY_SCALE] = "Scale",			[KEY_SEND] = "Send",
	[KEY_SCREENLOCK] = "Screen Lock",	[KEY_SWITCHVIDEOMODE] = "Switch Video Mode",
	[KEY_UWB] = "UWB",			[KEY_VIDEO_NEXT] = "Video Next",
	[KEY_VIDEO_PREV] = "Video Prev",	[KEY_WIMAX] = "WIMAX",
	[KEY_WLAN] = "WLAN",
#ifdef KEY_RFKILL
	[KEY_RFKILL] = "RF kill",
#endif
#ifdef KEY_WPS_BUTTON
	[KEY_WPS_BUTTON] = "WPS Button",
#endif
#ifdef KEY_TOUCHPAD_TOGGLE
	[KEY_TOUCHPAD_TOGGLE] = "Touchpad Toggle",
	[KEY_TOUCHPAD_ON] = "Touchpad On",
	[KEY_TOUCHPAD_OFF] = "Touchpad Off",
#endif

	[BTN_0] = "Btn0",			[BTN_1] = "Btn1",
	[BTN_2] = "Btn2",			[BTN_3] = "Btn3",
	[BTN_4] = "Btn4",			[BTN_5] = "Btn5",
	[BTN_6] = "Btn6",			[BTN_7] = "Btn7",
	[BTN_8] = "Btn8",			[BTN_9] = "Btn9",
	[BTN_LEFT] = "LeftBtn",			[BTN_RIGHT] = "RightBtn",
	[BTN_MIDDLE] = "MiddleBtn",		[BTN_SIDE] = "SideBtn",
	[BTN_EXTRA] = "ExtraBtn",		[BTN_FORWARD] = "ForwardBtn",
	[BTN_BACK] = "BackBtn",			[BTN_TASK] = "TaskBtn",
	[BTN_TRIGGER] = "Trigger",		[BTN_THUMB] = "ThumbBtn",
	[BTN_THUMB2] = "ThumbBtn2",		[BTN_TOP] = "TopBtn",
	[BTN_TOP2] = "TopBtn2",			[BTN_PINKIE] = "PinkieBtn",
	[BTN_BASE] = "BaseBtn",			[BTN_BASE2] = "BaseBtn2",
	[BTN_BASE3] = "BaseBtn3",		[BTN_BASE4] = "BaseBtn4",
	[BTN_BASE5] = "BaseBtn5",		[BTN_BASE6] = "BaseBtn6",
	[BTN_DEAD] = "BtnDead",			[BTN_A] = "BtnA",
	[BTN_B] = "BtnB",			[BTN_C] = "BtnC",
	[BTN_X] = "BtnX",			[BTN_Y] = "BtnY",
	[BTN_Z] = "BtnZ",			[BTN_TL] = "BtnTL",
	[BTN_TR] = "BtnTR",			[BTN_TL2] = "BtnTL2",
	[BTN_TR2] = "BtnTR2",			[BTN_SELECT] = "BtnSelect",
	[BTN_START] = "BtnStart",		[BTN_MODE] = "BtnMode",
	[BTN_THUMBL] = "BtnThumbL",		[BTN_THUMBR] = "BtnThumbR",
	[BTN_TOOL_PEN] = "ToolPen",		[BTN_TOOL_RUBBER] = "ToolRubber",
	[BTN_TOOL_BRUSH] = "ToolBrush",		[BTN_TOOL_PENCIL] = "ToolPencil",
	[BTN_TOOL_AIRBRUSH] = "ToolAirbrush",	[BTN_TOOL_FINGER] = "ToolFinger",
	[BTN_TOOL_MOUSE] = "ToolMouse",		[BTN_TOOL_LENS] = "ToolLens",
	[BTN_TOUCH] = "Touch",			[BTN_STYLUS] = "Stylus",
	[BTN_STYLUS2] = "Stylus2",		[BTN_TOOL_DOUBLETAP] = "Tool Doubletap",
	[BTN_TOOL_TRIPLETAP] = "Tool Tripletap", [BTN_TOOL_QUADTAP] = "Tool Quadtap",
	[BTN_GEAR_DOWN] = "WheelBtn",
	[BTN_GEAR_UP] = "Gear up",

#ifdef BTN_TRIGGER_HAPPY
	[BTN_TRIGGER_HAPPY1] = "Trigger Happy 1",	[BTN_TRIGGER_HAPPY11] = "Trigger Happy 11",
	[BTN_TRIGGER_HAPPY2] = "Trigger Happy 2",	[BTN_TRIGGER_HAPPY12] = "Trigger Happy 12",
	[BTN_TRIGGER_HAPPY3] = "Trigger Happy 3",	[BTN_TRIGGER_HAPPY13] = "Trigger Happy 13",
	[BTN_TRIGGER_HAPPY4] = "Trigger Happy 4",	[BTN_TRIGGER_HAPPY14] = "Trigger Happy 14",
	[BTN_TRIGGER_HAPPY5] = "Trigger Happy 5",	[BTN_TRIGGER_HAPPY15] = "Trigger Happy 15",
	[BTN_TRIGGER_HAPPY6] = "Trigger Happy 6",	[BTN_TRIGGER_HAPPY16] = "Trigger Happy 16",
	[BTN_TRIGGER_HAPPY7] = "Trigger Happy 7",	[BTN_TRIGGER_HAPPY17] = "Trigger Happy 17",
	[BTN_TRIGGER_HAPPY8] = "Trigger Happy 8",	[BTN_TRIGGER_HAPPY18] = "Trigger Happy 18",
	[BTN_TRIGGER_HAPPY9] = "Trigger Happy 9",	[BTN_TRIGGER_HAPPY19] = "Trigger Happy 19",
	[BTN_TRIGGER_HAPPY10] = "Trigger Happy 10",	[BTN_TRIGGER_HAPPY20] = "Trigger Happy 20",

	[BTN_TRIGGER_HAPPY21] = "Trigger Happy 21",	[BTN_TRIGGER_HAPPY31] = "Trigger Happy 31",
	[BTN_TRIGGER_HAPPY22] = "Trigger Happy 22",	[BTN_TRIGGER_HAPPY32] = "Trigger Happy 32",
	[BTN_TRIGGER_HAPPY23] = "Trigger Happy 23",	[BTN_TRIGGER_HAPPY33] = "Trigger Happy 33",
	[BTN_TRIGGER_HAPPY24] = "Trigger Happy 24",	[BTN_TRIGGER_HAPPY34] = "Trigger Happy 34",
	[BTN_TRIGGER_HAPPY25] = "Trigger Happy 25",	[BTN_TRIGGER_HAPPY35] = "Trigger Happy 35",
	[BTN_TRIGGER_HAPPY26] = "Trigger Happy 26",	[BTN_TRIGGER_HAPPY36] = "Trigger Happy 36",
	[BTN_TRIGGER_HAPPY27] = "Trigger Happy 27",	[BTN_TRIGGER_HAPPY37] = "Trigger Happy 37",
	[BTN_TRIGGER_HAPPY28] = "Trigger Happy 28",	[BTN_TRIGGER_HAPPY38] = "Trigger Happy 38",
	[BTN_TRIGGER_HAPPY29] = "Trigger Happy 29",	[BTN_TRIGGER_HAPPY39] = "Trigger Happy 39",
	[BTN_TRIGGER_HAPPY30] = "Trigger Happy 30",	[BTN_TRIGGER_HAPPY40] = "Trigger Happy 40",
#endif
};

char *absval[6] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat ", "Resolution "};

char *relatives[REL_MAX + 1] = {
	[0 ... REL_MAX] = NULL,
	[REL_X] = "X",			[REL_Y] = "Y",
	[REL_Z] = "Z",			[REL_RX] = "Rx",
	[REL_RY] = "Ry",		[REL_RZ] = "Rz",
	[REL_HWHEEL] = "HWheel",
	[REL_DIAL] = "Dial",		[REL_WHEEL] = "Wheel",
	[REL_MISC] = "Misc",
};

char *absolutes[ABS_MAX + 1] = {
	[0 ... ABS_MAX] = NULL,
	[ABS_X] = "X",			[ABS_Y] = "Y",
	[ABS_Z] = "Z",			[ABS_RX] = "Rx",
	[ABS_RY] = "Ry",		[ABS_RZ] = "Rz",
	[ABS_THROTTLE] = "Throttle",	[ABS_RUDDER] = "Rudder",
	[ABS_WHEEL] = "Wheel",		[ABS_GAS] = "Gas",
	[ABS_BRAKE] = "Brake",		[ABS_HAT0X] = "Hat0X",
	[ABS_HAT0Y] = "Hat0Y",		[ABS_HAT1X] = "Hat1X",
	[ABS_HAT1Y] = "Hat1Y",		[ABS_HAT2X] = "Hat2X",
	[ABS_HAT2Y] = "Hat2Y",		[ABS_HAT3X] = "Hat3X",
	[ABS_HAT3Y] = "Hat 3Y",		[ABS_PRESSURE] = "Pressure",
	[ABS_DISTANCE] = "Distance",	[ABS_TILT_X] = "XTilt",
	[ABS_TILT_Y] = "YTilt",		[ABS_TOOL_WIDTH] = "Tool Width",
	[ABS_VOLUME] = "Volume",	[ABS_MISC] = "Misc",
#ifdef ABS_MT_BLOB_ID
	[ABS_MT_TOUCH_MAJOR] = "Touch Major",
	[ABS_MT_TOUCH_MINOR] = "Touch Minor",
	[ABS_MT_WIDTH_MAJOR] = "Width Major",
	[ABS_MT_WIDTH_MINOR] = "Width Minor",
	[ABS_MT_ORIENTATION] = "Orientation",
	[ABS_MT_POSITION_X] = "Position X",
	[ABS_MT_POSITION_Y] = "Position Y",
	[ABS_MT_TOOL_TYPE] = "Tool Type",
	[ABS_MT_BLOB_ID] = "Blob ID",
#endif
#ifdef ABS_MT_TRACKING_ID
	[ABS_MT_TRACKING_ID] = "Tracking ID",
#endif
#ifdef ABS_MT_PRESSURE
	[ABS_MT_PRESSURE] = "Pressure",
#endif
#ifdef ABS_MT_SLOT
	[ABS_MT_SLOT] = "Slot",
#endif

};

char *misc[MSC_MAX + 1] = {
	[ 0 ... MSC_MAX] = NULL,
	[MSC_SERIAL] = "Serial",	[MSC_PULSELED] = "Pulseled",
	[MSC_GESTURE] = "Gesture",	[MSC_RAW] = "RawData",
	[MSC_SCAN] = "ScanCode",
};

char *leds[LED_MAX + 1] = {
	[0 ... LED_MAX] = NULL,
	[LED_NUML] = "NumLock",		[LED_CAPSL] = "CapsLock",
	[LED_SCROLLL] = "ScrollLock",	[LED_COMPOSE] = "Compose",
	[LED_KANA] = "Kana",		[LED_SLEEP] = "Sleep",
	[LED_SUSPEND] = "Suspend",	[LED_MUTE] = "Mute",
	[LED_MISC] = "Misc",
};

char *repeats[REP_MAX + 1] = {
	[0 ... REP_MAX] = NULL,
	[REP_DELAY] = "Delay",		[REP_PERIOD] = "Period"
};

char *sounds[SND_MAX + 1] = {
	[0 ... SND_MAX] = NULL,
	[SND_CLICK] = "Click",		[SND_BELL] = "Bell",
	[SND_TONE] = "Tone"
};

char *syns[3] = {
	[SYN_REPORT] = "Report Sync",
	[SYN_CONFIG] = "Config Sync",
#ifdef SYN_MT_REPORT
	[SYN_MT_REPORT] = "MT Sync"
#endif
};

char *switches[SW_MAX + 1] = {
	[0 ... SW_MAX] = NULL,
	[SW_LID] = "Lid",
	[SW_TABLET_MODE] = "Tablet Mode",
	[SW_HEADPHONE_INSERT] = "Headphone Insert",
	[SW_RFKILL_ALL] = "RFKILL",
	[SW_MICROPHONE_INSERT] = "Microphone Insert",
	[SW_DOCK] = "Dock",
	[SW_LINEOUT_INSERT] = "Lineout Insert",
	[SW_JACK_PHYSICAL_INSERT] = "Jack Physical Insert",
#ifdef SW_VIDEOOUT_INSERT
	[SW_VIDEOOUT_INSERT] = "Video Out Insert",
#endif
#ifdef SW_CAMERA_LENS_COVER
	[SW_CAMERA_LENS_COVER] = "Camera Lens Cover",
	[SW_KEYPAD_SLIDE] = "Keypad Slide",
	[SW_FRONT_PROXIMITY] = "Front Proximity Sensor",
#endif
#ifdef SW_ROTATE_LOCK
	[SW_ROTATE_LOCK] = "Rotate Lock",
#endif
};

char **names[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
	[EV_SYN] = events,			[EV_KEY] = keys,
	[EV_REL] = relatives,			[EV_ABS] = absolutes,
	[EV_MSC] = misc,			[EV_LED] = leds,
	[EV_SND] = sounds,			[EV_REP] = repeats,
	[EV_SW] = switches,
};

/**
 * Filter for the AutoDevProbe scandir on /dev/input.
 *
 * @param dir The current directory entry provided by scandir.
 *
 * @return Non-zero if the given directory entry starts with "event", or zero
 * otherwise.
 */
static int is_event_device(const struct dirent *dir) {
	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

/**
 * Scans all /dev/input/event*, display them and ask the user which one to
 * open.
 *
 * @return The event device file name of the device file selected. This
 * string is allocated and must be freed by the caller.
 */
static char* scan_devices(void)
{
	struct dirent **namelist;
	int i, ndev, devnum;
	char *filename;

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, alphasort);
	if (ndev <= 0)
		return NULL;

	fprintf(stderr, "Available devices:\n");

	for (i = 0; i < ndev; i++)
	{
		char fname[64];
		int fd = -1;
		char name[256] = "???";

		snprintf(fname, sizeof(fname),
			 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);

		fprintf(stderr, "%s:	%s\n", fname, name);
		close(fd);
		free(namelist[i]);
	}

	fprintf(stderr, "Select the device event number [0-%d]: ", ndev - 1);
	scanf("%d", &devnum);

	if (devnum >= ndev || devnum < 0)
		return NULL;

	asprintf(&filename, "%s/%s%d",
		 DEV_INPUT_EVENT, EVENT_DEV_NAME,
		 devnum);

	return filename;
}

/**
 * Print usage information.
 */
static void usage(void)
{
	printf("Usage: evtest /dev/input/eventX\n");
	printf("Where X = input device number\n");
}

/**
 * Parse the commandline arguments.
 *
 * @return The filename of the device file to open, or NULL in case of
 * error. This string is allocated and must be freed by the caller.
 */
static char* parse_args(int argc, char **argv)
{
	char *filename;

	if (argc < 2) {
		fprintf(stderr, "No device specified, trying to scan all of %s/%s*\n",
			DEV_INPUT_EVENT, EVENT_DEV_NAME);

		if (getuid() != 0)
			fprintf(stderr, "Not running as root, no devices may be available.\n");

		filename = scan_devices();
		if (!filename) {
			usage();
			return NULL;
		}
	} else
		filename = strdup(argv[argc - 1]);

	return filename;
}

/**
 * Print additional information for absolute axes (min/max, current value,
 * etc.).
 *
 * @param fd The file descriptor to the device.
 * @param axis The axis identifier (e.g. ABS_X).
 */
static void print_absdata(int fd, int axis)
{
	int abs[6] = {0};
	int k;

	ioctl(fd, EVIOCGABS(axis), abs);
	for (k = 0; k < 6; k++)
		if ((k < 3) || abs[k])
			printf("      %s %6d\n", absval[k], abs[k]);
}

/**
 * Print static device information (no events). This information includes
 * version numbers, device name and all bits supported by this device.
 *
 * @param fd The file descriptor to the device.
 * @return 0 on success or 1 otherwise.
 */
static int print_device_info(int fd)
{
	int i, j;
	int version;
	unsigned short id[4];
	char name[256] = "Unknown";
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		perror("evtest: can't get version");
		return 1;
	}

	printf("Input driver version is %d.%d.%d\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);

	ioctl(fd, EVIOCGID, id);
	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Input device name: \"%s\"\n", name);

	memset(bit, 0, sizeof(bit));
	ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
	printf("Supported events:\n");

	for (i = 0; i < EV_MAX; i++)
		if (test_bit(i, bit[0])) {
			printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
			if (!i) continue;
			ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
			for (j = 0; j < KEY_MAX; j++)
				if (test_bit(j, bit[i])) {
					printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
					if (i == EV_ABS)
						print_absdata(fd, j);
				}
		}

	return 0;
}

/**
 * Print device events as they come in.
 *
 * @param fd The file descriptor to the device.
 * @return 0 on success or 1 otherwise.
 */
static int print_events(int fd)
{
	struct input_event ev[64];
	int i, rd;

	while (1) {
		rd = read(fd, ev, sizeof(struct input_event) * 64);

		if (rd < (int) sizeof(struct input_event)) {
			printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
			perror("\nevtest: error reading");
			return 1;
		}

		for (i = 0; i < rd / sizeof(struct input_event); i++) {
			printf("Event: time %ld.%06ld, ", ev[i].time.tv_sec, ev[i].time.tv_usec);

			if (ev[i].type == EV_SYN) {
				if (ev[i].code == SYN_MT_REPORT)
					printf("++++++++++++++ %s ++++++++++++\n", syns[ev[i].code]);
				else
					printf("-------------- %s ------------\n", syns[ev[i].code]);
			} else {
				printf("type %d (%s), code %d (%s), ",
					ev[i].type,
					events[ev[i].type] ? events[ev[i].type] : "?",
					ev[i].code,
					names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?");
				if (ev[i].type == EV_MSC && (ev[i].code == MSC_RAW || ev[i].code == MSC_SCAN))
					printf("value %02x\n", ev[i].value);
				else
					printf("value %d\n", ev[i].value);
			}
		}

	}
}

/**
 * Grab and immediately ungrab the device.
 *
 * @param fd The file descriptor to the device.
 * @return 0 if the grab was successful, or 1 otherwise.
 */
static int test_grab(int fd)
{
	int rc;

	rc = ioctl(fd, EVIOCGRAB, (void*)1);

	if (!rc)
		ioctl(fd, EVIOCGRAB, (void*)0);

	return rc;
}

int main (int argc, char **argv)
{
	int fd;
	char *filename;

	filename = parse_args(argc, argv);

	if (!filename)
		return 1;

	if ((fd = open(filename, O_RDONLY)) < 0) {
		perror("evtest");
		if (errno == EACCES && getuid() != 0)
			fprintf(stderr, "You do not have access to %s. Try "
					"running as root instead.\n",
					filename);
		return 1;
	}

	free(filename);

	if (!isatty(fileno(stdout)))
		setbuf(stdout, NULL);

	if (print_device_info(fd))
		return 1;

	printf("Testing ... (interrupt to exit)\n");

	if (test_grab(fd))
	{
		printf("***********************************************\n");
		printf("  This device is grabbed by another process.\n");
		printf("  No events are available to evtest while the\n"
		       "  other grab is active.\n");
		printf("  In most cases, this is caused by an X driver,\n"
		       "  try VT-switching and re-run evtest again.\n");
		printf("***********************************************\n");
	}

	return print_events(fd);
}

/* vim: set noexpandtab tabstop=8 shiftwidth=8: */
