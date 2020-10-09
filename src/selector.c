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
		break;
	}

	return false;
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
	++csel->sel_num;
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
