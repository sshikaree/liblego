#include <stdbool.h>
#include <stddef.h>

#include <tidy/tidy.h>

#include "parser_error.h"

// ******************************
// *	Public API functions	*
// ******************************


typedef struct lego_Parser lego_Parser;
typedef struct NodeList NodeList;


// Callback function prototype to use with findFirst() and findAll() functions.
typedef void (*lego_CallBackFunc)(TidyDoc tdoc, TidyNode node, void* userdata);

// Returns first matching node.
TidyNode lego_FindFirst(TidyDoc tdoc, TidyNode root, lego_Parser* p);

/** Fills up given #NodeList with all matching nodes.
 * @param lst - list to fill up.
**/
lego_ParserError lego_FindAll(TidyNode root, lego_Parser* p, NodeList* lst);

// Finds first matching element starting from @root node and applies @cb function.
bool lego_FindFirstWithCB(TidyDoc tdoc, TidyNode root, lego_Parser* p, lego_CallBackFunc cb, void* userdata);

// Finds all matching elements starting from @root node and applies @cb function.
void lego_FindAllWithCB(TidyDoc tdoc, TidyNode root, lego_Parser* p, lego_CallBackFunc cb, void* userdata);

// Creates new instance of #lego_Parser.
lego_Parser* lego_ParserNew(void);

// Parses a selector, or a group of selectors separated by commas.
lego_ParserError lego_Compile(lego_Parser* p, char* source_string);

// Cleans up Parser
void lego_Clean(lego_Parser* p);

// Cleans up and frees Parser.
void lego_Destroy(lego_Parser* p);
