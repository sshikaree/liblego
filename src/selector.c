#include "../src/selector.h"


// returns `true` if @first < @second (strictly), false otherwise
bool Specifity_less(Specificity first, Specificity second) {
    for(int i = 0; i < SPEC_LEN; ++i) {
        if (first[i] < second[i]) {
            return true;
        }
        if (first[i] > second[i]) {
            return false;
        }
    }
    return false;
}

// Appends @src to @dst. Returns &dst.
int* Specificity_add(Specificity dst, Specificity src) {
    for (int i = 0; i < SPEC_LEN; ++i) {
        dst[i] += src[i];
    }
	return dst;
}


//const int TagSelectorSpecifity[SPEC_LEN] = {0, 0, 1};
//const char* TagSelectorPseudoElement = "";

//const int ClassSelectorSpecifity[SPEC_LEN] = {0, 1, 0};
//const char* ClassSelectorPseudoElement = "";

//const int IDSelectorSpecifity[SPEC_LEN] = {1, 0, 0};
//const char* IDSelectorPseudoElement = "";



// **************
// SimpleSelector
// **************

// Creates new SimpleSelector
SimpleSelector* SimpleSelector_new(SimpleSelectorType type) {
	SimpleSelector* sel = malloc(sizeof (SimpleSelector));
    if (!sel) {
        return NULL;
    }
    sel->type = type;
	sel->val = string_new(NULL);
	if (!sel->val) {
        free(sel);
        return NULL;
    }
	sel->pseudo_element = string_new(NULL);
	if (!sel->pseudo_element) {
		free(sel->val);
        free(sel);
        return NULL;
    }
//    switch (type) {
//    case SimpleSelectorType_ID:
//        sel->spec[0] = 1;
//        sel->spec[1] = 0;
//        sel->spec[2] = 0;
//        break;
//    case SimpleSelectorType_CLASS:
//        sel->spec[0] = 0;
//        sel->spec[1] = 1;
//        sel->spec[2] = 0;
//        break;
//    case SimpleSelectorType_TAG:
//        sel->spec[0] = 0;
//        sel->spec[1] = 0;
//        sel->spec[2] = 1;
//        break;
//    case SimpleSelectorType_PSEUDO:
//        break;
//    }

    return sel;
}

void SimpleSelector_free(SimpleSelector* sel) {
	if (!sel) {
		return;
	}
	if (sel->val) {
		string_free(sel->val);
	}
	if (sel->pseudo_element) {
		string_free(sel->pseudo_element);
	}

	if (sel->type == SimpleSelectorType_ATTR) {
		if (sel->key) {
			string_free(sel->key);
		}
		if (sel->operation) {
			string_free(sel->operation);
		}

		if (sel->regexp) {
			string_free(sel->regexp);
		}
	}

	free(sel);
}


// Returns specificity of @sel.
void SimpleSelector_specificity(SimpleSelector *sel, Specificity spec) {
	switch (sel->type) {
	case SimpleSelectorType_ID:
		spec[0] = 1;
		spec[1] = 0;
		spec[2] = 0;
		break;
	case SimpleSelectorType_CLASS:
		spec[0] = 0;
		spec[1] = 1;
		spec[2] = 0;
		break;
	case SimpleSelectorType_TAG:
		spec[0] = 0;
		spec[1] = 0;
		spec[2] = 1;
		break;
	case SimpleSelectorType_ATTR:
		spec[0] = 0;
		spec[1] = 1;
		spec[2] = 0;
		break;
	case SimpleSelectorType_PSEUDO:
		//
		// TODO!!
		break;
	}
}

bool SimpleSelector_match(SimpleSelector* sel, TidyNode node) {
	TidyAttr tattr = NULL;
	ctmbstr	 tattr_value = NULL;
	switch (sel->type) {
	case SimpleSelectorType_ID:
		tattr = tidyAttrGetById(node, TidyAttr_ID);
		if (tattr) {
			tattr_value = tidyAttrValue(tattr);
		}
		return (tattr_value && (strcmp(tattr_value, sel->val->str) == 0));
	case SimpleSelectorType_CLASS:
		tattr = tidyAttrGetById(node, TidyAttr_CLASS);
		if (tattr) {
			tattr_value = tidyAttrValue(tattr);
		}
		return (tattr_value && (strcmp(tattr_value, sel->val->str) == 0));
	case SimpleSelectorType_TAG:
		tattr_value = tidyNodeGetName(node);
		return (tattr_value && (strcmp(tattr_value, sel->val->str) == 0));
	case SimpleSelectorType_ATTR:
		//
		// TBD
		//
		break;
	}

	return false;
}

// Returns string representation of the SimpleSelector.
String* SimpleSelector_string(SimpleSelector* sel) {
	String* ret_s = string_new(NULL);
	switch (sel->type) {
	case SimpleSelectorType_TAG:
		ret_s = string_append(ret_s, sel->val->str);
		break;
	case SimpleSelectorType_ID:
		ret_s = string_append_c(ret_s, '#');
		ret_s = string_append(ret_s, sel->val->str);
		break;
	case SimpleSelectorType_CLASS:
		ret_s = string_append_c(ret_s, '.');
		ret_s = string_append(ret_s, sel->val->str);
		break;
	case SimpleSelectorType_ATTR:
		//
		// TODO
		//
		break;

	}

	return ret_s;

}

// ****************
// CompoundSelector
// ****************

CompoundSelector* CompoundSelector_new(void) {
	CompoundSelector* sel = malloc(sizeof (CompoundSelector));
	if (!sel) {
		return NULL;
	}
	sel->sel_num = 0;
	sel->pseudo_element = string_new(NULL);
	if (!sel->pseudo_element) {
		free(sel);
		return NULL;
	}

	// Should we memset csel->selectors with '0' here?
	return sel;
}

void CompoundSelector_free(CompoundSelector* sel) {
	if (!sel) {
		return;
	}
	if (sel->pseudo_element) {
		string_free(sel->pseudo_element);
	}
	for (size_t i = 0; i < sel->sel_num; ++i) {
		SimpleSelector_free(sel->selectors[i]);
	}
	free(sel);
}

// TODO:
// - Return an error if array overflows
void CompoundSelector_addSelector(CompoundSelector* csel, SimpleSelector* ssel) {
	if (!csel || !ssel) {
		return;
	}
	if (csel->sel_num >= MAX_SELECTORS_NUM) {
		fprintf(stderr, "Compound selector buffer overflow\n");
		return;
	}
	csel->selectors[csel->sel_num] = ssel;
	csel->sel_num++;
}


void CompoundSelector_specificity(CompoundSelector* sel, Specificity spec) {
	memset(spec, 0, SPEC_LEN);
	Specificity tmp = {0};
	for (size_t i = 0; i < sel->sel_num; ++i) {
		SimpleSelector_specificity(sel->selectors[i], tmp);
		Specificity_add(spec, tmp);
	}
	if (sel->pseudo_element->len > 0) {
		// https://drafts.csswg.org/selectors-3/#specificity
		Specificity_add(spec, (Specificity){0, 0, 1});
	}
}

// Matches elements if each sub-selectors matches.
bool CompoundSelector_match(CompoundSelector* comp_sel, TidyNode node) {
	if (comp_sel->sel_num == 0) {
		return tidyNodeGetType(node) == TidyNode_StartEnd;
	}
	for (size_t i = 0; i < comp_sel->sel_num; ++ i) {
		if (!SimpleSelector_match(comp_sel->selectors[i], node)) {
			return false;
		}
	}
	return true;
}

// Returns string representation of the selector.
String* CompoundSelector_string(CompoundSelector* comp_sel) {
	if (comp_sel->sel_num == 0 && comp_sel->pseudo_element->len == 0) {
		return string_new("*");
	}
	String* ret_s = string_new(NULL);
	String* ss = string_new(NULL);
	for (size_t i = 0; i < comp_sel->sel_num; ++i) {
		ss = SimpleSelector_string(comp_sel->selectors[i]);
		ret_s = string_append(ret_s, ss->str);
		string_free(ss);
	}
	if (comp_sel->pseudo_element->len != 0) {
		ret_s = string_append(ret_s, "::");
		ret_s = string_append(ret_s, comp_sel->pseudo_element->str);
	}
	return ret_s;
}


// ****************
// CombinedSelector
// ****************

CombinedSelector* CombinedSelector_new(void) {
	CombinedSelector* comb_sel = malloc(sizeof (CombinedSelector));
	if (!comb_sel) {
		return NULL;
	}
	comb_sel->first = NULL;
	comb_sel->second = NULL;
	comb_sel->combinator = 0;
	return comb_sel;
}

void CombinedSelector_free(CombinedSelector* comb_sel) {
	if (!comb_sel) {
		return;
	}
	if (comb_sel->first) {
		CompoundSelector_free(comb_sel->first);
	}
	if (comb_sel->second) {
		CompoundSelector_free(comb_sel->second);
	}
	free(comb_sel);
}

void CombinedSelector_specificity(CombinedSelector* comb_sel, Specificity spec){
	CompoundSelector_specificity(comb_sel->first, spec);
	if (comb_sel->second != NULL) {
		Specificity tmp = {0};
		CompoundSelector_specificity(comb_sel->second, tmp);
		Specificity_add(spec, tmp);
	}
}

// matches an element if it matches @second and has an ancestor that matches @first.
static bool descendantMatch(CompoundSelector* first, CompoundSelector* second, TidyNode node) {
	if (!CompoundSelector_match(second, node)) {
		return false;
	}
	for (TidyNode parent = tidyGetParent(node); parent; parent = tidyGetParent(parent)) {
		if (CompoundSelector_match(first, parent)) {
			return true;
		}
	}
	return false;
}

// matches an element if it matches &second and its parent matches @first.
static bool childMatch(CompoundSelector* first, CompoundSelector* second, TidyNode node) {
	TidyNode parent = tidyGetParent(node);
	return (
			CompoundSelector_match(second, node) &&
			parent &&
			CompoundSelector_match(first, parent)
	);
}

// matches an element if it matches @second and is preceded by an element that matches @first.
// If @adjacent is true, the sibling must be immediately before the element.
static bool siblingMatch(CompoundSelector* first,
						 CompoundSelector* second,
						 bool adjacent,
						 TidyNode node) {
	if (!CompoundSelector_match(second, node)) {
		return false;
	}
	if (adjacent) {
		TidyNodeType t;
		for (node = tidyGetPrev(node); node; node = tidyGetPrev(node)) {
			t = tidyNodeGetType(node);
			if (t == TidyNode_Text || t == TidyNode_Comment) {
				continue;
			}
			return CompoundSelector_match(first, node);
		}
		return false;
	}

	// Walk backwards looking for element that matches s1
	// TODO: Is new variable needed here?? Maybe just use node?
	for (TidyNode c = tidyGetPrev(node); c; c = tidyGetPrev(c)) {
		if (CompoundSelector_match(first, c)) {
			return true;
		}
	}
	return false;
}

bool CombinedSelector_match(CombinedSelector* comb_sel, TidyNode node) {
	if (!comb_sel->first) {
		return false;
	}
	switch (comb_sel->combinator) {
	case 0:
		return CompoundSelector_match(comb_sel->first, node);
	case ' ':
		return descendantMatch(comb_sel->first, comb_sel->second, node);
	case '>':
		return childMatch(comb_sel->first, comb_sel->second, node);
	case '+':
		return siblingMatch(comb_sel->first, comb_sel->second, true, node);
	case '~':
		return siblingMatch(comb_sel->first, comb_sel->second, false, node);
	}
	return false;
}


// Returns string repersentation of the selector.
// TODO: Add spaces between selectors and combinator??
String*	CombinedSelector_string(CombinedSelector* comb_sel) {
	String* ret_s = CompoundSelector_string(comb_sel->first);
	if (comb_sel->second) {
		ret_s = string_append_c(ret_s, comb_sel->combinator);
		String* ss = CompoundSelector_string(comb_sel->second);
		ret_s = string_append(ret_s, ss->str);
		string_free(ss);
	}
	return ret_s;
}
