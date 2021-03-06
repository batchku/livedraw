/*
Written by John MacCallum, The Center for New Music and Audio Technologies,
University of California, Berkeley.  Copyright (c) 2009, The Regents of
the University of California (Regents). 
Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "osc_match.h"

int osc_match(const char *pattern, const char *address, int *pattern_offset, int* address_offset) {

	int r;

	const char *pattern_start;
	const char *address_start;

	pattern_start = pattern;
	address_start = address;

	*pattern_offset = 0;
	*address_offset = 0;

	while(*address != '\0' && *pattern != '\0'){
		if(*pattern == '*'){
			if(!osc_match_star(pattern, address)){
				return 0;
			}
			while(*pattern != '/' && *pattern != '\0'){
				pattern++;
			}
			while(*address != '/' && *address != '\0'){
				address++;
			}
		}else{
			if(!osc_match_single_char(pattern, address)){
				return 0;
			}
			if(*pattern == '[' || *pattern == '{'){
				while(*pattern != ']' && *pattern != '}'){
					pattern++;
				}
			}
			pattern++;
			address++;
		}
	}

	*pattern_offset = pattern - pattern_start;
	*address_offset = address - address_start;

	r = 0;

	if(*address == '\0') {
		r |= OSC_MATCH_ADDRESS_COMPLETE;
	}

	if(*pattern == '\0') {
		r |= OSC_MATCH_PATTERN_COMPLETE;
	}

	return r;
}

int osc_match_star(const char *pattern, const char *address){
	const char *address_start = address;
	const char *pattern_start = pattern;
	int num_stars = 0;
	if(*address == '\0') { return 0; }
	while(*address != '/' && *address != '\0'){
		address++;
	}
	while(*pattern != '/' && *pattern != '\0'){
		if(*pattern == '*'){
			num_stars++;
		}
		pattern++;
	}
	pattern--;
	address--;
	switch(num_stars){
	case 1:
		{
			const char *pp = pattern, *aa = address;
			while(*pp != '*'){
				//printf("%c %c\n", *pp, *aa);
				if(!(osc_match_single_char(pp, aa))){
					return 0;
				}
				if(*pp == ']' || *pp == '}'){
					while(*pp != '[' && *pp != '{'){
						pp--;
					}
				}
				pp--;
				aa--;
			}
		}
		break;
	case 2:
#if (OSC_MATCH_ENABLE_2STARS == 1)
		{
			const char *pp = pattern, *aa = address;
			while(*pp != '*'){
				if(!(osc_match_single_char(pp, aa))){
					return 0;
				}
				if(*pp == ']' || *pp == '}'){
					while(*pp != '[' && *pp != '{'){
						pp--;
					}
				}
				pp--;
				aa--;
			}
			aa++; // we want to start one character forward to allow the star to match nothing
			const char *star2 = pp;
			const char *test = aa;
			int i = 0;
			while(test > address_start){
				pp = star2 - 1;
				aa = test - 1;
				i++;
				while(*pp != '*'){
					if(!osc_match_single_char(pp, aa)){
						break;
					}
					if(*pp == ']' || *pp == '}'){
						while(*pp != '[' && *pp != '{'){
							pp--;
						}
					}
					pp--;
					aa--;
				}
				if(pp == pattern_start){
					return 1;
				}
				test--;
			}
			return 0;
		}
		break;
#else
		return 0;
#endif
	default:
#if (OSC_MATCH_ENABLE_NSTARS == 1)
		return osc_match_star_r(pattern_start, address_start);
		break;
#else
		return 0;
#endif
	}
	return 1;
}

#if (OSC_MATCH_ENABLE_NSTARS == 1)
int osc_match_star_r(const char *pattern, const char *address){
	if(*address == '/' || *address == '\0'){
		if(*pattern == '/' || *pattern == '\0' || ((*pattern == '*' && (*(pattern + 1) == '/')) || *(pattern + 1) == '\0')){
			return 1;
		}else{
			return 0;
		}
	}
	if(*pattern == '*'){
		if(osc_match_star_r(pattern + 1, address)){
			return 1;
		}else{
			return osc_match_star_r(pattern, address + 1);
		}
	}else{
		if(!osc_match_single_char(pattern, address)){
			return 0;
		}
		if(*pattern == '[' || *pattern == '{'){
			while(*pattern != ']' && *pattern != '}'){
				pattern++;
			}
		}
		//pattern++;
		//address++;
		return osc_match_star_r(pattern + 1, address + 1);
	}
}
#endif

int osc_match_single_char(const char *pattern, const char *address){
	switch(*pattern){
	case '[':
		return osc_match_bracket(pattern, address);
	case ']':
		while(*pattern != '['){
			pattern--;
		}
		return osc_match_bracket(pattern, address);
	case '{':
		return osc_match_curly_brace(pattern, address);
	case '}':
		while(*pattern != '{'){
			pattern--;
		}
		return osc_match_curly_brace(pattern, address);
	case '?':
		return 1;
	default:
		if(*pattern == *address){
			return 1;
		}else{
			return 0;
		}
	}
	return 0;
}

int osc_match_bracket(const char *pattern, const char *address){
	int matched = 0;
	pattern++;
	while(*pattern != ']' && *pattern != '\0'){
		// the character we're on now is the beginning of a range
		if(*(pattern + 1) == '-'){
			if(*address >= *pattern && *address <= *(pattern + 2)){
				matched = 1;
				break;
			}else{
				pattern += 3;
			}
		}else{
			// just test the character
			if(*pattern == *address){
				matched = 1;
				break;
			}
			pattern++;
		}
	}
	return matched;
	/*
	if(matched){
		while(*pattern != ']' && *pattern != '\0'){
			pattern++;
		}
		pattern++;
		address++;
	}else{
		return 0;
	}
	return 1;
	*/
}

int osc_match_curly_brace(const char *pattern, const char *address){
	int matched = 0;
	pattern++;
	while(*pattern != '}' && *pattern != '\0' && *pattern != '/'){
		if(*pattern == *address){
			matched = 1;
			break;
		}else{
			pattern++;
			if(*pattern == ','){
				pattern++;
			}
		}
	}

	if(matched){
		while(*pattern != '}' && *pattern != '\0' && *pattern != '/'){
			pattern++;
		}
		pattern++;
		address++;
	}else{
		return 0;
	}

	return 1;
}

/*

#include "stdio.h"

int main() {

	char* a[6] = {
		"/foo",
		"/foo/bar",
		"/foo/bar/baz",
		"/fu",
		"/foo/bar/bing/ping/8/3",
		"/foo/jdlkalsdjalsjd/bing/8/lkj 890"
	};

	int rootLevels = 1;
	char* root[rootLevels] = {
		"/livedraw"
	};
	
	int secondLevels = 3;
	char * layers[secondLevels] = {
		"/layout",
		"/session",
		"/layers"
	}
	
	
	// full path
	
	
	

	int t;
	int q;
	int r;

	int i;

	for (i = 0; i < 6; i++) {
		r = osc_match(p, a[i], &t, &q);
		printf("in=>%s dest=>%s => &t=%s &q=%s R=(%d)\n", p, a[i], p+t, a[i]+q, r);
	}
	
	// const char *pattern, const char *address, int *pattern_offset, int* address_offset
	
	
	

	return 1;
}

*/


