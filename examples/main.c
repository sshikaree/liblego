#include <stdio.h>
#include <stdlib.h>

// for libtidy
typedef unsigned int uint;
typedef unsigned long ulong;

#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#include "lego.h"
#include "util/slist.h"

// Callback function
static void cb_printFunc(TidyDoc tdoc, TidyNode node, void* userdata) {
	TidyBuffer buf = {0};
	tidyNodeGetText(tdoc, node, &buf);
	// Get rid of trailing '\n'-s
	for (; buf.size-1 > 0 && buf.bp[buf.size-1] == '\n'; --buf.size, buf.bp[buf.size] = '\0');
	printf("Matched node: '%s'\n",
		   buf.bp
		   );
	tidyBufFree(&buf);
}

int main() {
	#define FILE_NAME "wikipedia.html"
	
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;
	TidyDoc tdoc = tidyCreate();
	tidySetErrorBuffer(tdoc, &errbuf);

	rc  = tidyParseFile(tdoc, FILE_NAME);
	if (rc >= 0) {
		rc = tidyCleanAndRepair(tdoc);
	}
	if (rc >=0) {
		rc = tidyRunDiagnostics(tdoc);
	}
	if (rc > 1) {
		rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
	}
	if (rc >= 0) {
		rc = tidySaveBuffer(tdoc, &output);
	}
	if (rc < 0) {
		fprintf(stderr,
				"libtide error [%d] parsing file '%s'\n", rc, FILE_NAME
		);
	}

	TidyNode root = tidyGetRoot(tdoc);
	if (!root) {
		fprintf(stderr, "Can't get root node of '%s'\nSkipping..\n", output.bp);
		goto exit;
	}

	lego_Parser* p = lego_ParserNew();
	lego_ParserError err = lego_Compile(p, "#mp-itn ul");
	if (err) {
		fprintf(stderr, "%s\n", lego_ParserError_toString(err));
		goto exit;
	}
	puts("Find news block:");
	puts("-----------");
	lego_FindFirstWithCB(tdoc, root, p, cb_printFunc, NULL);
	puts("*****************************************************\n");

	puts("Find all news in news block:");
	puts("---------");
	TidyNode news_block = lego_FindFirst(tdoc, root, p);
	if (!news_block) {
		puts("Match not found. Exiting.");
		goto exit;
	}
	// Clean up Parser for reuse.
	lego_Clean(p);

	err = lego_Compile(p, "li");
	if (err) {
		fprintf(stderr, "%s\n", lego_ParserError_toString(err));
		goto exit;
	}

	NodeList* lst = NodeList_new();
	err = lego_FindAll(news_block, p, lst);
	if (err != ParserError_NO_ERROR) {
		fprintf(stderr, "%s\n", lego_ParserError_toString(err));
	}
	printf("List size: %lu\n", lst->size);
	for (NodeListElement* elm = lst->first; elm; elm = elm->next) {
		TidyBuffer tb = {0};
		if (elm->tnode && tidyNodeGetText(tdoc, elm->tnode, &tb)) {
			printf("Matched node: '%s'\n", tb.bp);
			tidyBufFree(&tb);
		}
	}
	NodeList_destroy(lst);
	lego_Destroy(p);


exit:

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	return 0;
}
