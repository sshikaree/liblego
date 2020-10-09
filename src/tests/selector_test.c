#include "../parser.h"
#include "../selector.h"

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


int main() {
	Parser p;
	#define SELECTOR_GROUP_LEN	16
	CombinedSelector* selector_group[SELECTOR_GROUP_LEN];

	for (int i = 0; i < SAMPLES_NUM; ++i) {
		Parser_init(&p, selector_samples[i]);
		memset(selector_group, 0, SELECTOR_GROUP_LEN);

		ParserError err = parseSelectorGroup(&p, selector_group, SELECTOR_GROUP_LEN);
		if (err) {
			fprintf(stderr, "%s\n", ParserError_toString(err));
			continue;
		}
		for (CombinedSelector** sel_ptr = &selector_group[0]; *sel_ptr; ++sel_ptr) {
			if (!(*sel_ptr)->first) {
				continue;
			}
			char* s = (*sel_ptr)->first->selectors[0]->val->str;
			if (s) {
				printf("%s\n", s);
			}
			CombinedSelector_free(*sel_ptr);
		}
	}



	return 0;
}
