#ifndef SELECTOR_H
#define SELECTOR_H

#include <stdbool.h>

#include "../../dynamic_string/dynamic_string.h"
#include <tidy/tidy.h>

#define SPEC_LEN 3 // Specifity array length
#define MAX_SELECTORS_NUM 32 // Max SimpleSelectors array size in CompoundSelector

// Specificity is the CSS specificity as defined in
// https://www.w3.org/TR/selectors/#specificity-rules
// with the convention Specificity = [A,B,C].
typedef int Specificity[SPEC_LEN];

bool Specifity_less(Specificity first, Specificity second);
int* Specificity_add(Specificity dst, Specificity src);


typedef enum {
    SimpleSelectorType_UNIVERSAL, // '*'
    SimpleSelectorType_TAG,
    SimpleSelectorType_CLASS,
    SimpleSelectorType_ID,
    SimpleSelectorType_ATTR,
    SimpleSelectorType_PSEUDO,

    // must be the last
    SimpleSelectorType_NUM
} SimpleSelectorType;

typedef enum {
    SelectorType_SIMPLE_SELECTOR,
    SelectorType_COMPOUND_SELECTOR,
    SelectorType_COMPLEX_SELECTOR,

    SelectorType_NUM
} SelectorType;

//typedef struct AttrSelector {
//	String* key;
//	String* val;
//	String* operation;
//	String* regexp;
//} AttrSelector;


// **************
// SimpleSelector
// **************
typedef struct SimpleSelector {
    SimpleSelectorType  type;
	String* val;

	// next 3 fields are for the SimpleSelectorType_ATTR type
	String* key;
	String* operation;
	String* regexp;

//     Should we store specificity? Or create func which returns spec based on type?
//    int                 spec[SPEC_LEN]; // specificity
	String*             pseudo_element;
} SimpleSelector;

SimpleSelector* SimpleSelector_new(SimpleSelectorType type);
void			SimpleSelector_free(SimpleSelector* sel);

//String*			SimpleSelector_string(SimpleSelector* sel);
void			SimpleSelector_specificity(SimpleSelector* sel, Specificity spec);

//typedef String TagSelector;
//extern const int TagSelectorSpecifity[SPEC_LEN];
//extern const char* TagSelectorPseudoElement;

//typedef String ClassSelector;
//extern const int ClassSelectorSpecifity[SPEC_LEN];
//extern const char* ClassSelectorPseudoElement;

//typedef String IDSelector;
//extern const int IDSelectorSpecifity[SPEC_LEN];
//extern const char* IDSelectorPseudoElement;


// ****************
// CompoundSelector
// ****************
// Maybe ComplexSelector??
typedef struct CompoundSelector {
	String*         pseudo_element;
	SimpleSelector* selectors[MAX_SELECTORS_NUM];
	size_t			sel_num; // Current selectors number
} CompoundSelector;

CompoundSelector*	CompoundSelector_new(void);
void				CompoundSelector_free(CompoundSelector* sel);
void				CompoundSelector_addSelector(CompoundSelector* csel, SimpleSelector* ssel);
void				CompoundSelector_specificity(CompoundSelector* sel, Specificity spec);


// ****************
// CombinedSelector
// ****************
typedef struct CombinedSelector {
	SimpleSelector*	first;
	SimpleSelector*	second;
	byte			combinator;
} CombinedSelector;

CombinedSelector*	CombinedSelector_new(void);
void				CombinedSelector_free(CombinedSelector* comb_sel);
bool				CombinedSelector_match(CombinedSelector* comb_sel, TidyNode* tnod);
void				CombinedSelector_specificity(CombinedSelector* comb_sel, Specificity spec);


// ********
// Selector
// ********
typedef struct Selector {
    SelectorType  type;
    union {
		SimpleSelector      smpl_sel;
//        AttrSelector        asel;
		CompoundSelector    comp_sel;
    };
} Selector;

Selector*	Selector_new(SelectorType sel_type, SimpleSelectorType smpl_sel_type);
void		Selector_free(Selector* sel);

bool		Selector_match(Selector* sel, TidyNode* tnod);
//int			Selector_string(Selector* sel, String* s);
void		Selector_specificity(Selector* sel, Specificity spec);

#endif // SELECTOR_H
