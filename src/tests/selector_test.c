#include "../parser.h"
#include "../selector.h"

#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#define SAMPLES_NUM	3

static char html_samples[][128] = {
	"<body><address>This address...</address></body>",
	"<p id=\"foo\"><p id=\"bar\">",
	"<div class=\"test\">"
};

static char selector_samples[][128] = {
	"address",
	"#foo",
	"div.teST"
};


static void tree_traversal(TidyDoc tdoc, TidyNode root, CombinedSelector** sel_group) {
	for (TidyNode child = tidyGetChild(root); child; child = tidyGetNext(child)) {
//		printf("Node name: %s\n", tidyNodeGetName(child));
		for (CombinedSelector** sel_ptr = sel_group; *sel_ptr; ++sel_ptr) {
			if (!(*sel_ptr)->first) {
				continue;
			}
			SimpleSelector* sel = (*sel_ptr)->first->selectors[0];
			if (sel) {
				if (SimpleSelector_match(sel, child)) {
					TidyBuffer buf = {0};
					tidyNodeGetText(tdoc, child, &buf);
					printf("Selector: '%s' matches -> node '%s'\n",
						   sel->val->str,
//						   tidyNodeGetName(child)
						   buf.bp
					);
					tidyBufFree(&buf);

//					printf("Node type: %d\n", tidyNodeGetType(child));
				}
			}
		}
		tree_traversal(tdoc, child, sel_group);
	}
}

int main() {
	Parser p;

	#define SELECTOR_GROUP_LEN	16
	CombinedSelector* selector_group[SELECTOR_GROUP_LEN];

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
		if (rc > 0) {
			printf("\nDiagnostics:\n\n%s", errbuf.bp);
		}
		printf("\nAnd here is the result:\n\n%s", output.bp);
		puts("**********************************\n");

		TidyNode root = tidyGetRoot(tdoc);
		if (!root) {
			fprintf(stderr, "Can't get root node of '%s'\nSkipping..\n", output.bp);
			continue;
		}

		ParserError err = parseSelectorGroup(&p, selector_group, SELECTOR_GROUP_LEN);
		if (err) {
			fprintf(stderr, "%s\n", ParserError_toString(err));
			continue;
		}


		// traversal here
		tree_traversal(tdoc, root, selector_group);


		for (CombinedSelector** sel_ptr = &selector_group[0]; *sel_ptr; ++sel_ptr) {
			CombinedSelector_free(*sel_ptr);
		}
		tidyBufFree(&output);
		tidyBufFree(&errbuf);
	}

exit:

//	tidyBufFree(&output);
//	tidyBufFree(&errbuf);
	tidyRelease(tdoc);

	return 0;
}
