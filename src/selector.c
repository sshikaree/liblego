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

// Appends @src to @dst
void Specifity_add(Specificity dst, Specificity src) {
    for (int i = 0; i < SPEC_LEN; ++i) {
        dst[i] += src[i];
    }
}


//const int TagSelectorSpecifity[SPEC_LEN] = {0, 0, 1};
//const char* TagSelectorPseudoElement = "";

//const int ClassSelectorSpecifity[SPEC_LEN] = {0, 1, 0};
//const char* ClassSelectorPseudoElement = "";

//const int IDSelectorSpecifity[SPEC_LEN] = {1, 0, 0};
//const char* IDSelectorPseudoElement = "";

SimpleSelector *SimpleSelector_new(SimpleSelectorType type) {
    SimpleSelector* sel = malloc(sizeof (sel));
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

// Returns specificity of @sel.
void Selector_specificity(SimpleSelector *sel, Specificity spec) {
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
		break;
	}
}
