#ifndef _STRING_SIZE_LIMITS_HEADER_
#define _STRING_SIZE_LIMITS_HEADER_

enum StringSizeLimits
{
	KEY_SEQUENCE_MAX_COUNT = 1024,
	RECENT_TEXT_MAX_LENGTH = 1024,
	MAX_CALC_EVAL_STRING_SIZE = 1024,
	MAX_WEB_STRING_SIZE = 150 // in the worst case, each unicode char is translater to 6 chars (%xx%yy). The string size * 6 must be somewhat below 1024
};

#endif // _STRING_SIZE_LIMITS_HEADER_