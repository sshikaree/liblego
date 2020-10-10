#include "../parser.h"
#include "../selector.h"

#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#define SAMPLES_NUM	7

static char html_samples[][128] = {
	"<body><address>This address...</address></body>",
	"<!-- comment --><html><head></head><body>text</body></html>",
	"<html><head></head><body></body></html>",
	"<p id=\"foo\"><p id=\"bar\">",
	"<ul><li id=\"t1\"><p id=\"t1\">",
	"<ol><li id=\"t4\"><li id=\"t44\">,"
	"<div class=\"test\">",
};

static char selector_samples[][128] = {
	"address",
	"*",
	"*",
	"#foo",
	"li#t1",
	"*#t4",
	"div.teST"
};

static void cb_printFunc(TidyDoc tdoc, TidyNode node, void* userdata) {
	TidyBuffer buf = {0};
	tidyNodeGetText(tdoc, node, &buf);
	// Get rid of trailing '\n'-s
	for (; buf.size-1 > 0 && buf.bp[buf.size-1] == '\n'; --buf.size, buf.bp[buf.size-1] = '\0');
//	String* s = CombinedSelector_string(*sel_ptr);
	printf("Matched -> node '%s'\n",
		   buf.bp
		   );
	tidyBufFree(&buf);
//	string_free(s);
}

static void tree_traversal(TidyDoc tdoc, TidyNode root, SelectorGroup sel_group) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = sel_group; *sel_ptr; ++sel_ptr) {
//			String* s = CombinedSelector_string(*sel_ptr);
//			printf("Selector: %s\n", s->str);
//			string_free(s);
			if (CombinedSelector_match(*sel_ptr, child)) {
				TidyBuffer buf = {0};
				tidyNodeGetText(tdoc, child, &buf);
				// Get rid of trailing '\n'-s
				for (; buf.size-1 > 0 && buf.bp[buf.size-1] == '\n'; --buf.size, buf.bp[buf.size-1] = '\0');
				String* s = CombinedSelector_string(*sel_ptr);
				printf("Selector: '%s' matches -> node '%s'\n",
					   s->str,
					   buf.bp
					   );
				tidyBufFree(&buf);
				string_free(s);
			}
		}
		tree_traversal(tdoc, child, sel_group);
	}
}

int main() {
	Parser p;

	SelectorGroup selector_group = {0};

	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;
	TidyDoc tdoc = tidyCreate();
	tidySetErrorBuffer(tdoc, &errbuf);
	if (!tidyOptSetBool(tdoc, TidyXhtmlOut, yes)) {
		puts("tidyOptSetBool() error");
		goto exit;
	}

	for (int i = 0; i < SAMPLES_NUM; ++i) {
		printf("Selector sample string: '%s'\n", selector_samples[i]);
		Parser_init(&p, selector_samples[i]);
		memset(selector_group, 0, SELECTOR_GROUP_LEN);

		rc = tidyParseString(tdoc, html_samples[i]);
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
					"libtide error [%d] parsing string '%s'\nSkipping..\n", rc, html_samples[i]
			);
			continue;
		}
//		if (rc > 0) {
//			printf("\nDiagnostics:\n\n%s", errbuf.bp);
//		}
//		printf("\nAnd here is the result:\n\n%s\n", output.bp);

		TidyNode root = tidyGetRoot(tdoc);
		if (!root) {
			fprintf(stderr, "Can't get root node of '%s'\nSkipping..\n", output.bp);
			continue;
		}

		ParserError err = Parser_compile(&p, selector_group);
		if (err) {
			fprintf(stderr, "%s\n", ParserError_toString(err));
			continue;
		}


		// traversal here
//		tree_traversal(tdoc, root, selector_group);
		findAll(tdoc, root, selector_group, cb_printFunc, NULL);
		puts("**********************************\n");


		for (CombinedSelector** sel_ptr = &selector_group[0]; *sel_ptr; ++sel_ptr) {
			CombinedSelector_free(*sel_ptr);
		}
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
	}

exit:

	tidyRelease(tdoc);

	return 0;
}
