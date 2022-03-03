#ifndef _KEYBOARD_DEFINITIONS_HEADER_
#define _KEYBOARD_DEFINITIONS_HEADER_

// operators
// '+' another key is pressed
// '-' pressed key is removed
// '++' key is double clicked
// '--' all pressed keys are removed
enum KEY_OPERATOR
{
	KEY_OPERATOR_NONE = 0x00000000,
	KEY_OPERATOR_PLUS = 0x00010000,
	KEY_OPERATOR_MINUS = 0x00020000,
	KEY_OPERATOR_PLUSPLUS = 0x00030000, // a key was pressed a short period after pressed previously
	KEY_OPERATOR_MINUSMINUS = 0x00040000, // any of the pressed keys is removed

	KEY_NON_PRINTABLE	= 0x00100000,
	KEY_SHIFT_IS_DOWN	= 0x00200000,
	KEY_SIMULATED		= 0x00400000,

	VIRTUAL_KEY_MASK = 0x0000FFFF,
	KEY_OPERATOR_MASK = 0x000F0000,
};

#endif // _KEYBOARD_DEFINITIONS_HEADER_