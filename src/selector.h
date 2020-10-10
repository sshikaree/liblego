#ifndef SELECTOR_H
#define SELECTOR_H

#include <stdbool.h>

#include "util/dynamic_string.h"
#include <tidy/tidy.h>

#define SPEC_LEN			3	// Specifity array length
#define MAX_SELECTORS_NUM	32	// Max SimpleSelectors array size in CompoundSelector. TODO: Should it be dynamic array??
#define SELECTOR_GROUP_LEN	16	// Max SelectorGroup array size. TODO: Should it be dynamic array??

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
	SelectorType_COMBINED_SELECTOR,
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
void			SimpleSelector_specificity(SimpleSelector* sel, Specificity spec);
bool			SimpleSelector_match(SimpleSelector* sel, TidyNode node);
String*			SimpleSelector_string(SimpleSelector* sel);


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
void				CompoundSelector_free(CompoundSelector* comp_sel);
void				CompoundSelector_addSelector(CompoundSelector* comp_sel, SimpleSelector* smpl_sel);
void				CompoundSelector_specificity(CompoundSelector* comp_sel, Specificity spec);
bool				CompoundSelector_match(CompoundSelector* comp_sel, TidyNode node);
String*				CompoundSelector_string(CompoundSelector* comp_sel);


// ****************
// CombinedSelector
// ****************

typedef struct CombinedSelector {
	CompoundSelector*	first;
	CompoundSelector*	second;
	char			combinator;
} CombinedSelector;

CombinedSelector*	CombinedSelector_new(void);
void				CombinedSelector_free(CombinedSelector* comb_sel);
void				CombinedSelector_specificity(CombinedSelector* comb_sel, Specificity spec);
bool				CombinedSelector_match(CombinedSelector* comb_sel, TidyNode tnod);
String*				CombinedSelector_string(CombinedSelector* comb_sel);


// ********
// Selector
// ********

typedef struct Selector {
    SelectorType  type;
    union {
		SimpleSelector		smpl_sel;
//        AttrSelector*        asel;
		CompoundSelector	comp_sel;
		CombinedSelector	comb_sel;
    };
} Selector;

Selector*	Selector_new(SelectorType sel_type, SimpleSelectorType smpl_sel_type);
void		Selector_free(Selector* sel);
void		Selector_specificity(Selector* sel, Specificity spec);
bool		Selector_match(Selector* sel, TidyNode* tnod);



// *************
// SelectorGroup
// *************

// Must be zero-terminated
typedef CombinedSelector* SelectorGroup[SELECTOR_GROUP_LEN];

void SelectorGroup_free(SelectorGroup sg);


// Callback function prototype to use with findFirst() and findAll() functions.
typedef void (*callBackFunc)(TidyDoc tdoc, TidyNode node, void* userdata);

// Finds first matching element starting from @root node and applies @cb function.
void findFirst(TidyDoc tdoc, TidyNode root, SelectorGroup sg, callBackFunc cb, void* userdata);

// Finds all matching elements starting from @root node and applies @cb function.
void findAll(TidyDoc tdoc, TidyNode root, SelectorGroup sg, callBackFunc cb, void* userdata);



#endif // SELECTOR_H
