#ifndef ENGINEKEYCODES_H
#define ENGINEKEYCODES_H

#include "../System.h"

#include <SDL/include/SDL.h>

typedef int32_t Keycode;

#define SCANCODE_MASK (1 << 30)
#define SCANCODE_TO_KEYCODE(X)  (X | SCANCODE_MASK)

enum {
	KEY_UNKNOWN = 0,

	KEY_RETURN = '\r',
	KEY_ESCAPE = '\033',
	KEY_BACKSPACE = '\b',
	KEY_TAB = '\t',
	KEY_SPACE = ' ',
	KEY_EXCLAIM = '!',
	KEY_QUOTEDBL = '"',
	KEY_HASH = '#',
	KEY_PERCENT = '%',
	KEY_DOLLAR = '$',
	KEY_AMPERSAND = '&',
	KEY_QUOTE = '\'',
	KEY_LEFTPAREN = '(',
	KEY_RIGHTPAREN = ')',
	KEY_ASTERISK = '*',
	KEY_PLUS = '+',
	KEY_COMMA = ',',
	KEY_MINUS = '-',
	KEY_PERIOD = '.',
	KEY_SLASH = '/',
	KEY_0 = '0',
	KEY_1 = '1',
	KEY_2 = '2',
	KEY_3 = '3',
	KEY_4 = '4',
	KEY_5 = '5',
	KEY_6 = '6',
	KEY_7 = '7',
	KEY_8 = '8',
	KEY_9 = '9',
	KEY_COLON = ':',
	KEY_SEMICOLON = ';',
	KEY_LESS = '<',
	KEY_EQUALS = '=',
	KEY_GREATER = '>',
	KEY_QUESTION = '?',
	KEY_AT = '@',
	/*
	Skip uppercase letters
	*/
	KEY_LEFTBRACKET = '[',
	KEY_BACKSLASH = '\\',
	KEY_RIGHTBRACKET = ']',
	KEY_CARET = '^',
	KEY_UNDERSCORE = '_',
	KEY_BACKQUOTE = '`',
	KEY_A = 'a',
	KEY_B = 'b',
	KEY_C = 'c',
	KEY_D = 'd',
	KEY_E = 'e',
	KEY_F = 'f',
	KEY_G = 'g',
	KEY_H = 'h',
	KEY_I = 'i',
	KEY_J = 'j',
	KEY_K = 'k',
	KEY_L = 'l',
	KEY_M = 'm',
	KEY_N = 'n',
	KEY_O = 'o',
	KEY_P = 'p',
	KEY_Q = 'q',
	KEY_R = 'r',
	KEY_S = 's',
	KEY_T = 't',
	KEY_U = 'u',
	KEY_V = 'v',
	KEY_W = 'w',
	KEY_X = 'x',
	KEY_Y = 'y',
	KEY_Z = 'z',

	KEY_CAPSLOCK = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),

	KEY_F1 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
	KEY_F2 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
	KEY_F3 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
	KEY_F4 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
	KEY_F5 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
	KEY_F6 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
	KEY_F7 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
	KEY_F8 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
	KEY_F9 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
	KEY_F10 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
	KEY_F11 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
	KEY_F12 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),

	KEY_PRINTSCREEN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
	KEY_SCROLLLOCK = SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
	KEY_PAUSE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
	KEY_INSERT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
	KEY_HOME = SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
	KEY_PAGEUP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
	KEY_DELETE = '\177',
	KEY_END = SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
	KEY_PAGEDOWN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
	KEY_RIGHT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
	KEY_LEFT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
	KEY_DOWN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
	KEY_UP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),

	KEY_NUMLOCKCLEAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
	KEY_KEYPAD_DIVIDE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
	KEY_KEYPAD_MULTIPLY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
	KEY_KEYPAD_MINUS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
	KEY_KEYPAD_PLUS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
	KEY_KEYPAD_ENTER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
	KEY_KEYPAD_1 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
	KEY_KEYPAD_2 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
	KEY_KEYPAD_3 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
	KEY_KEYPAD_4 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
	KEY_KEYPAD_5 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
	KEY_KEYPAD_6 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
	KEY_KEYPAD_7 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
	KEY_KEYPAD_8 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
	KEY_KEYPAD_9 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
	KEY_KEYPAD_0 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
	KEY_KEYPAD_PERIOD = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),

	KEY_APPLICATION = SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION),
	KEY_POWER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER),
	KEY_KEYPAD_EQUALS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS),
	KEY_F13 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13),
	KEY_F14 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14),
	KEY_F15 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15),
	KEY_F16 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16),
	KEY_F17 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17),
	KEY_F18 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18),
	KEY_F19 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19),
	KEY_F20 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20),
	KEY_F21 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21),
	KEY_F22 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22),
	KEY_F23 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23),
	KEY_F24 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24),
	KEY_EXECUTE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE),
	KEY_HELP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP),
	KEY_MENU = SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU),
	KEY_SELECT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT),
	KEY_STOP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP),
	KEY_AGAIN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN),
	KEY_UNDO = SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO),
	KEY_CUT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT),
	KEY_COPY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY),
	KEY_PASTE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE),
	KEY_FIND = SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND),
	KEY_MUTE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE),
	KEY_VOLUMEUP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP),
	KEY_VOLUMEDOWN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN),
	KEY_KEYPAD_COMMA = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA),
	KEY_KEYPAD_EQUALSAS400 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400),

	KEY_ALTERASE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE),
	KEY_SYSREQ = SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ),
	KEY_CANCEL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL),
	KEY_CLEAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR),
	KEY_PRIOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR),
	KEY_RETURN2 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2),
	KEY_SEPARATOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR),
	KEY_OUT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT),
	KEY_OPER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER),
	KEY_CLEARAGAIN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN),
	KEY_CRSEL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL),
	KEY_EXSEL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL),

	KEY_KEYPAD_00 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00),
	KEY_KEYPAD_000 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000),
	KEY_THOUSANDSSEPARATOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR),
	KEY_DECIMALSEPARATOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR),
	KEY_CURRENCYUNIT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT),
	KEY_CURRENCYSUBUNIT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT),
	KEY_KEYPAD_LEFTPAREN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN),
	KEY_KEYPAD_RIGHTPAREN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN),
	KEY_KEYPAD_LEFTBRACE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE),
	KEY_KEYPAD_RIGHTBRACE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE),
	KEY_KEYPAD_TAB = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB),
	KEY_KEYPAD_BACKSPACE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE),
	KEY_KEYPAD_A = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A),
	KEY_KEYPAD_B = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B),
	KEY_KEYPAD_C = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C),
	KEY_KEYPAD_D = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D),
	KEY_KEYPAD_E = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E),
	KEY_KEYPAD_F = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F),
	KEY_KEYPAD_XOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR),
	KEY_KEYPAD_POWER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER),
	KEY_KEYPAD_PERCENT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT),
	KEY_KEYPAD_LESS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS),
	KEY_KEYPAD_GREATER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER),
	KEY_KEYPAD_AMPERSAND = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND),
	KEY_KEYPAD_DBLAMPERSAND = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND),
	KEY_KEYPAD_VERTICALBAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR),
	KEY_KEYPAD_DBLVERTICALBAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR),
	KEY_KEYPAD_COLON = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON),
	KEY_KEYPAD_HASH = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH),
	KEY_KEYPAD_SPACE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE),
	KEY_KEYPAD_AT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT),
	KEY_KEYPAD_EXCLAM = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM),
	KEY_KEYPAD_MEMSTORE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE),
	KEY_KEYPAD_MEMRECALL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL),
	KEY_KEYPAD_MEMCLEAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR),
	KEY_KEYPAD_MEMADD = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD),
	KEY_KEYPAD_MEMSUBTRACT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT),
	KEY_KEYPAD_MEMMULTIPLY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY),
	KEY_KEYPAD_MEMDIVIDE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE),
	KEY_KEYPAD_PLUSMINUS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS),
	KEY_KEYPAD_CLEAR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR),
	KEY_KEYPAD_CLEARENTRY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY),
	KEY_KEYPAD_BINARY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY),
	KEY_KEYPAD_OCTAL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL),
	KEY_KEYPAD_DECIMAL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL),
	KEY_KEYPAD_HEXADECIMAL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL),

	KEY_LCTRL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
	KEY_LSHIFT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
	KEY_LALT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
	KEY_LGUI = SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
	KEY_RCTRL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
	KEY_RSHIFT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
	KEY_RALT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
	KEY_RGUI = SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

	KEY_MODE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),

	KEY_AUDIONEXT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT),
	KEY_AUDIOPREV = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV),
	KEY_AUDIOSTOP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP),
	KEY_AUDIOPLAY = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY),
	KEY_AUDIOMUTE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE),
	KEY_MEDIASELECT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT),
	KEY_WWW = SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW),
	KEY_MAIL = SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL),
	KEY_CALCULATOR = SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR),
	KEY_COMPUTER = SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER),
	KEY_AC_SEARCH = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH),
	KEY_AC_HOME = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME),
	KEY_AC_BACK = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK),
	KEY_AC_FORWARD = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD),
	KEY_AC_STOP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP),
	KEY_AC_REFRESH = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH),
	KEY_AC_BOOKMARKS = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS),

	KEY_BRIGHTNESSDOWN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN),
	KEY_BRIGHTNESSUP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP),
	KEY_DISPLAYSWITCH = SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH),
	KEY_KBDILLUMTOGGLE = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE),
	KEY_KBDILLUMDOWN = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN),
	KEY_KBDILLUMUP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP),
	KEY_EJECT = SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT),
	KEY_SLEEP = SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP),
	KEY_APP1 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP1),
	KEY_APP2 = SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP2),

	KEY_AUDIOREWIND = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOREWIND),
	KEY_AUDIOFASTFORWARD = SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOFASTFORWARD)
};


#endif