
#include "parser.h"
#include "lego.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "util/dynamic_string.h"
#include "util/slist.h"


// ParserError_toString returns error string, corresponding to given error code.
// Do not try to free resulting string!
const char* lego_ParserError_toString(lego_ParserError err) {
//    static const char* str = "";
    switch (err) {
    case ParserError_UNEXPECTED_EOF:            return "Unexpected EOF!";
    case ParserError_UNEXPECTED_EOL:            return "Unexpected EOL!";
    case ParserError_UNEXPECTED_CHAR:           return "Unexpected character!";
    case ParserError_WRONG_IDENTIFIER:          return "Wrong identifier!";
    case ParserError_WRONG_ID:                  return "Wrong ID!";
    case ParserError_WRONG_CLASSNAME:           return "Wrong classname!";
    case ParserError_WRONG_ATTRIBUTE:           return "Wrong attribute!";
    case ParserError_WRONG_ATTRIBUTE_OPERATOR:  return "Wrong attribute operator!";
    case ParserError_WRONG_PSEUDOCLASS:         return "Wrong pseudoclass!";
    case ParserError_INVALID_ESCAPE_SEQUENCE:   return "Invalid escape sequence!";
    case ParserError_SEL_BUF_OVERFLOW:          return "Selectors buffer overflow!";
	case ParserError_MEMORY_ERROR:				return "Memory allocation error!";
    // TODO: Fill up rest of error codes.

	default: return  "Unknown error!";
    }
}

static void Parser_init(lego_Parser* p, char* source_string) {
    p->s = source_string;
    p->s_len = strlen(source_string);
    p->pos = source_string;
	memset(p->sel_group, 0, SELECTOR_GROUP_LEN);
}


static bool hexDigit(char c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}


// nameStart returns whether @c can be the first character of an identifier
// (not counting an initial hyphen, or an escape sequence).
static bool nameStart(byte c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || c > 127;
}

// nameChar returns whether @c can be a character within an identifier
// (not counting an escape sequence).
static bool nameChar(byte c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' ||
            c == '-' || ('0' <= c && c <= '9') || c > 127  ;
}

// wide char to UTF8
// returns number of bytes, occupied by unicode simbol
int wcToUTF8(char dest[4], uint32_t ch) {
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (char)((ch>>6) | 0xC0);
        dest[1] = (char)((ch & 0x3F) | 0x80);
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (char)((ch>>12) | 0xE0);
        dest[1] = (char)(((ch>>6) & 0x3F) | 0x80);
        dest[2] = (char)((ch & 0x3F) | 0x80);
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (char)((ch>>18) | 0xF0);
        dest[1] = (char)(((ch>>12) & 0x3F) | 0x80);
        dest[2] = (char)(((ch>>6) & 0x3F) | 0x80);
        dest[3] = (char)((ch & 0x3F) | 0x80);
        return 4;
    }
    return 0;
}


// toLowerASCII returns s with all ASCII capital letters lowercased.
// It alteres source string.
static void toLowerASCII(char* s) {
    for (char* c_ptr = s; *c_ptr; ++c_ptr) {
        if ('A' <= *c_ptr && *c_ptr <= 'Z') {
            *c_ptr = *c_ptr + ('a' - 'A');
        }
    }
}

// skipWhitespace consumes whitespace characters and comments.
// It returns true if there was actually anything to skip.
bool skipWhitespace(lego_Parser* p) {
    char* c_ptr = p->pos;
    for (; *c_ptr; ) {
        switch (*c_ptr) {
        case ' ': case '\t': case '\r': case '\n': case '\f':
            ++c_ptr;
            continue;

        case '/':
            if (*(c_ptr+1) == '*') {
                char* end = strstr(c_ptr, "*/");
                if (end) {
                    c_ptr = end + 2;
                    continue;
                }
            }
        }
        break;
    }
    if (c_ptr > p->pos) {
        p->pos = c_ptr;
        return true;
    }
    return false;
}

// consumeParenthesis consumes an opening parenthesis and any following
// whitespace. It returns true if there was actually a parenthesis to skip.
static bool consumeParenthesis(lego_Parser* p) {
    if (p->pos < p->s + p->s_len && *p->pos == '(') {
        p->pos++;
        skipWhitespace(p);
        return true;
    }
    return false;
}

// consumeClosingParenthesis consumes a closing parenthesis and any preceding
// whitespace. It returns true if there was actually a parenthesis to skip.
static bool consumeClosingParenthesis(lego_Parser* p) {
    char* c_ptr = p->pos;
    skipWhitespace(p);
    if (p->pos < p->s + p->s_len && *p->pos == ')') {
        p->pos++;
        return true;
    }
    p->pos = c_ptr;
    return false;
}

// parseEscape parses a backslash escape.
// @buf must be at least 5 bytes long.
// Returns null-terminated @buf.
static lego_ParserError parseEscape(lego_Parser* p, char buf[5]) {
    if ((p->pos > p->s + p->s_len - 2) || *p->pos != '\\') {
        return ParserError_INVALID_ESCAPE_SEQUENCE;
    }
    char* c_ptr = p->pos+1;
    if (*c_ptr == '\r' || *c_ptr == '\n' || *c_ptr == '\f') {
        return ParserError_UNEXPECTED_EOL;
    }
    if (hexDigit(*c_ptr)) { // unicode escape (hex)
        for (; *c_ptr && c_ptr - (p->pos+1) < 6 && hexDigit(*c_ptr); ++c_ptr) {
            // just iterate to the end of unicode code point
        }
        uint32_t val = (uint32_t)strtol(p->pos+1, &c_ptr, 16);
//        char tmp[4] = {0};
        int amount = wcToUTF8(buf, val);
//        memcpy(buf, tmp, (size_t)amount);
        buf[amount] = '\0';
        switch (*c_ptr) {
        case '\r':
            ++c_ptr;
            if (*c_ptr == '\n') {
                c_ptr++;
            }
            break;
        case ' ': case '\t': case '\n': case '\f':
            ++c_ptr;
            break;
        }

        p->pos = c_ptr;
        return ParserError_NO_ERROR;
    }

    p->pos = c_ptr+1;
    // Return the literal character after the backslash.
    buf[0] = *c_ptr;
    buf[1] = '\0';
    return ParserError_NO_ERROR;
}



// parseName parses a name (which is like an identifier, but doesn't have
// extra restrictions on the first character).
static lego_ParserError parseName(lego_Parser* p, String* result) {
    for (; *p->pos; ) {
        if (nameChar((byte)*p->pos)) {
            string_append_c(result, *p->pos);
            p->pos++;
        } else if (*p->pos == '\\') {
            char buf[5];
			lego_ParserError err = parseEscape(p, buf);
            if (err != ParserError_NO_ERROR) {
                return err;
            }
            string_append(result, buf);
        } else {
//            p->pos++;
			break;
        }
    }
    if (result->len < 1) {
        return ParserError_UNEXPECTED_EOF;
    }

    return ParserError_NO_ERROR;
}

// parseIdentifier parses an identifier
// @result - where parsed selector pointer will be stored
lego_ParserError parseIdentifier(lego_Parser *p, String* result) {
    char* c_ptr = p->pos;
    if (*c_ptr == '-') {
        string_append_c(result, '-');
        ++c_ptr;
    }
    if (c_ptr >= p->s+p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (!nameStart((byte)*c_ptr) || *c_ptr == '\\') {
        return ParserError_WRONG_IDENTIFIER;
    }
    p->pos = c_ptr;

	lego_ParserError err = parseName(p, result);
    if (err != ParserError_NO_ERROR) {
        return err;
    }

    return ParserError_NO_ERROR;
}


// parseQuoted parses a single- or double-quoted string.
lego_ParserError parseQuoted(lego_Parser* p, String* result) {
    if (p->pos > p->s + p->s_len - 2) {
        return ParserError_UNEXPECTED_EOF;
    }
    char* c_ptr = p->pos;
    const char quote = *c_ptr;
    ++c_ptr;
    for (; *c_ptr && *c_ptr != quote; ) {
        switch (*c_ptr) {
        case '\\':
            if (*(c_ptr+1) == '\r' && *(c_ptr+2) == '\n') {
                c_ptr += 3;
                continue;
            }
            if (*(c_ptr+1) == '\n' || *(c_ptr+1) == '\f') {
                c_ptr += 2;
                continue;
            }

            p->pos = c_ptr;
            char buf[5];
			lego_ParserError err = parseEscape(p, buf);
            if (err != ParserError_NO_ERROR) {
                return err;
            }
            c_ptr = p->pos;
            string_append(result, buf);
            break;

        case '\r': case '\n': case '\f':
            return ParserError_UNEXPECTED_EOL;

        default:
            for (; *c_ptr; ) {
                if (*c_ptr == quote || *c_ptr == '\\' || *c_ptr == '\r' || *c_ptr == '\n' || *c_ptr == '\n') {
                    break;
                }
                string_append_c(result, *c_ptr);
                ++c_ptr;
            }
        }
    }

    // Consume the final quote
    if (*c_ptr == quote){
        ++c_ptr;
        p->pos = c_ptr;
        return ParserError_NO_ERROR;
    }
    return ParserError_UNEXPECTED_EOF;
}


// parseTypeSelector parses a type selector (one that matches by tag name).
static lego_ParserError parseTypeSelector(lego_Parser *p, SimpleSelector* tag_selector) {
	tag_selector->type = SimpleSelectorType_TAG;
	lego_ParserError err = parseIdentifier(p, tag_selector->val);
    if (err != ParserError_NO_ERROR) {
        return err;
    }
	toLowerASCII(tag_selector->val->str);

    return ParserError_NO_ERROR;
}

// parseIDSelector parses a selector that matches by id attribute.
static lego_ParserError parseIDSelector(lego_Parser *p, SimpleSelector* id_selector) {
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (*p->pos != '#') {
        return ParserError_WRONG_ID;
    }
    p->pos++;
	id_selector->type = SimpleSelectorType_ID;
	return parseName(p, id_selector->val);
}

// parseClassSelector parses a selector that matches by class attribute.
static lego_ParserError parseClassSelector(lego_Parser *p, SimpleSelector* class_selector) {
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (*p->pos != '.') {
        return ParserError_WRONG_CLASSNAME;
    }
    p->pos++;
	class_selector->type = SimpleSelectorType_CLASS;
	return parseIdentifier(p, class_selector->val);
}


// parseAttributeSelector parses a selector that matches by attribute value.
static lego_ParserError parseAttributeSelector(lego_Parser *p, SimpleSelector* attr_selector) {
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (*p->pos != '[') {
        return ParserError_WRONG_ATTRIBUTE;
    }
    p->pos++;
    skipWhitespace(p);
	attr_selector->type = SimpleSelectorType_ATTR;
	lego_ParserError err = parseIdentifier(p, attr_selector->key);
    if (err != ParserError_NO_ERROR) {
        return err;
    }
	toLowerASCII(attr_selector->key->str);
    skipWhitespace(p);
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (*p->pos == ']') {
        p->pos++;
		attr_selector->operation->str[0] = '\0';
		attr_selector->operation->len = 0;
        return ParserError_NO_ERROR;
    }
    if (p->pos + 2 > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
	string_append_len(attr_selector->operation, p->pos, 2);
	if (attr_selector->operation->str[0] == '=') {
		attr_selector->operation->str[1] = '\0';
		attr_selector->operation->len = 1;
	} else if (attr_selector->operation->str[1] != '=') {
        return ParserError_WRONG_ATTRIBUTE;
    }
	p->pos += strlen(attr_selector->operation->str);
    skipWhitespace(p);
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
	if (attr_selector->operation->str[0] == '#' && attr_selector->operation->str[1] == '=') {
//        !!!!
//        TODO: call parseRegex() here
//        !!!!
    } else {
        switch (*p->pos) {
        case '\'': case '"':
			err = parseQuoted(p, attr_selector->val);
            break;
        default:
			err = parseIdentifier(p, attr_selector->val);
			break;
        }
    }
    if (err != ParserError_NO_ERROR) {
        return err;
    }
    skipWhitespace(p);
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
    if (*p->pos != ']') {
        return ParserError_WRONG_ATTRIBUTE;
    }
    p->pos++;
	char* op_str = attr_selector->operation->str;
    if (
		strcmp(op_str, "=")	 != 0 && strcmp(op_str, "!=") != 0 &&
		strcmp(op_str, "~=") != 0 && strcmp(op_str, "|=") != 0 &&
		strcmp(op_str, "^=") != 0 && strcmp(op_str, "$=") != 0 &&
		strcmp(op_str, "*=") != 0 && strcmp(op_str, "#=") != 0
    ) {
        return ParserError_WRONG_ATTRIBUTE_OPERATOR;
    }

    return ParserError_NO_ERROR;
}


// parseSimpleSelectorSequence parses a selector sequence that applies to
// a single element.
static lego_ParserError parseSimpleSelectorSequence(lego_Parser *p, CompoundSelector* csel) {
    if (p->pos > p->s + p->s_len) {
        return ParserError_UNEXPECTED_EOF;
    }
	lego_ParserError err = ParserError_NO_ERROR;
    switch (*p->pos) {
    case '*':
        // It's the universal selector. Just skip over it, since it doesn't affect the meaning.
        ++(p->pos);
        break;
    case '#': case '.': case '[': case ':':
        // There's no type selector. Wait to process the other till the main loop.
        break;
	default:{
		SimpleSelector* sel = SimpleSelector_new(SimpleSelectorType_TAG);
		err = parseTypeSelector(p, sel);
		if (err != ParserError_NO_ERROR) {
			SimpleSelector_free(sel);
			return err;
		}
		CompoundSelector_addSelector(csel, sel);
		break;
		}
    }
    for (;*p->pos;) {
        switch (*p->pos) {
		case '#':{
			SimpleSelector* sel = SimpleSelector_new(SimpleSelectorType_ID);
			err = parseIDSelector(p, sel);
			if (err != ParserError_NO_ERROR) {
				SimpleSelector_free(sel);
				return err;
			}
			CompoundSelector_addSelector(csel, sel);
            break;
			}
		case '.':{
			SimpleSelector* sel = SimpleSelector_new(SimpleSelectorType_CLASS);
			err = parseClassSelector(p, sel);
			if (err != ParserError_NO_ERROR) {
				SimpleSelector_free(sel);
				return err;
			}
			CompoundSelector_addSelector(csel, sel);
			break;
			}
		case '[':{
			SimpleSelector* sel = SimpleSelector_new(SimpleSelectorType_ATTR);
			err = parseAttributeSelector(p, sel);
			if (err != ParserError_NO_ERROR) {
				SimpleSelector_free(sel);
				return err;
			}
			CompoundSelector_addSelector(csel, sel);
			break;
			}
        case ':':
            //TODO: call Parser_parsePseudoClassSelector() here
            break;
        default:
            goto loop_exit;

        // TODO: Impement pseudo-elements
        }
    }

loop_exit:

    return 0;
}


// Should rename to parseCombinedSelector ??
// parseSelector parses a selector that may include combinators.
static lego_ParserError parseSelector(lego_Parser* p, CombinedSelector* comb_sel) {
    skipWhitespace(p);
	CompoundSelector* first_sel = CompoundSelector_new();
	lego_ParserError err = parseSimpleSelectorSequence(p, first_sel);
	if (err != ParserError_NO_ERROR) {
		CompoundSelector_free(first_sel);
		return err;
	}
	comb_sel->first = first_sel;
//	char combinator = 0;
	comb_sel->combinator = 0;
	for (;;) {
		if (skipWhitespace(p)) {
			comb_sel->combinator = ' ';
		}
		if (p->pos >= p->s + p->s_len) {
			return ParserError_NO_ERROR;
		}
		switch (*p->pos) {
		case '+': case '>': case '~':
			comb_sel->combinator = *p->pos;
			p->pos++;
			skipWhitespace(p);
			break;
		case ',': case ')':
			// These characters can't begin a selector, but they can legally occur after one.
			return ParserError_NO_ERROR;
		}
		if (comb_sel->combinator == 0) {
			return ParserError_NO_ERROR;
		}
		CompoundSelector* second_sel = CompoundSelector_new();
		lego_ParserError err = parseSimpleSelectorSequence(p, second_sel);
		if (err != ParserError_NO_ERROR) {
			CompoundSelector_free(first_sel);
			CompoundSelector_free(second_sel);
			return err;
		}
		comb_sel->second = second_sel;
	}
}

// parseSelectorGroup parses a group of selectors, separated by commas.
static lego_ParserError parseSelectorGroup(lego_Parser *p){
	int i = 0;
	memset(p->sel_group, 0, (size_t)SELECTOR_GROUP_LEN);
	CombinedSelector* comb_sel = CombinedSelector_new();
	lego_ParserError err = parseSelector(p, comb_sel);
	if (err != ParserError_NO_ERROR) {
		CombinedSelector_free(comb_sel);
		return err;
	}
	p->sel_group[i] = comb_sel;
	i++;
	if (i >= SELECTOR_GROUP_LEN) {
		return ParserError_SEL_BUF_OVERFLOW;
	}

	for (; *p->pos; ) {
		if (*p->pos != ',') {
			break;
        }
		p->pos++;
		comb_sel = CombinedSelector_new();
		err = parseSelector(p, comb_sel);
		if (err != ParserError_NO_ERROR) {
			CombinedSelector_free(comb_sel);
			return err;
		}
		p->sel_group[i] = comb_sel;
		i++;
		if (i >= SELECTOR_GROUP_LEN) {
			return ParserError_SEL_BUF_OVERFLOW;
		}
    }
	return ParserError_NO_ERROR;
}



// Returns first matching node.
TidyNode lego_FindFirst(TidyDoc tdoc, TidyNode root, lego_Parser* p) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = p->sel_group; *sel_ptr; ++sel_ptr) {
			if (CombinedSelector_match(*sel_ptr, child)) {
				return child;
			}
		}
		TidyNode ch = lego_FindFirst(tdoc, child, p);
		if (ch) {
			return ch;
		}
	}
	return NULL;
}


/** Fills up given #NodeList with all matching nodes.
 *
 * @param lst - list to fill up.
**/
lego_ParserError lego_FindAll(TidyNode root, lego_Parser* p, NodeList* lst) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = p->sel_group; *sel_ptr; ++sel_ptr) {
			if (CombinedSelector_match(*sel_ptr, child)) {
				if (!NodeList_append(lst, child)) {
					return ParserError_MEMORY_ERROR;
				}
//				printf("List size: %lu\n", lst->size);
			}
		}
		lego_ParserError err = lego_FindAll(child, p, lst);
		if (err != ParserError_NO_ERROR){
			return err;
		}
	}
	return ParserError_NO_ERROR;
}


// Finds first matching element starting from @root node and applies @cb function.
bool lego_FindFirstWithCB(TidyDoc tdoc, TidyNode root, lego_Parser* p, lego_CallBackFunc cb, void* userdata) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = p->sel_group; *sel_ptr; ++sel_ptr) {
			if (CombinedSelector_match(*sel_ptr, child)) {
				cb(tdoc, child, userdata);
				return true;
			}
		}
		if (lego_FindFirstWithCB(tdoc, child, p, cb, userdata)) {
			return true;
		}
	}
	return false;
}



// Finds all matching elements starting from @root node and applies @cb function.
void lego_FindAllWithCB(TidyDoc tdoc, TidyNode root, lego_Parser* p, lego_CallBackFunc cb, void* userdata) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = p->sel_group; *sel_ptr; ++sel_ptr) {
//			String* s = CombinedSelector_string(*sg);
//			printf("Selector: %s\n", s->str);
//			string_free(s);
			if (CombinedSelector_match(*sel_ptr, child)) {
				cb(tdoc, child, userdata);
			}
		}
		lego_FindAllWithCB(tdoc, child, p, cb, userdata);
	}
}



// Creates new instance of lego_Parser.
lego_Parser* lego_ParserNew(void) {
	return (lego_Parser*)malloc(sizeof(lego_Parser));
}


// Parses a selector, or a group of selectors separated by commas.
lego_ParserError lego_Compile(lego_Parser* p, char* source_string) {
	Parser_init(p, source_string);
	return parseSelectorGroup(p);
}

// Cleans up parser
void lego_Clean(lego_Parser* p) {
	SelectorGroup_free(p->sel_group);
}

// Cleans up and frees Parser.
void lego_Destroy(lego_Parser* p) {
	lego_Clean(p);
	free(p);
}



