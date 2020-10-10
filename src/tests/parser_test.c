#include <stdio.h>

//#include "tokens_array.h"
#include "../parser.h"

#define SAMPLES_NUM 7

static char samples[][128] = {
    ".class1.class2 sub_class3",
    "  .class1 sub_class2",
    "a.class1 sub_class2, h1",
	"a.class1.class2 sub_class3",
    "  /*comment*/ a.class1 sub_class2.class3",
	"#id",
	"li"
};

static char identifierTestSamples[2][64][128] = {
    {"x", "96", "-x", R"(r\e9 sumé)", R"(r\0000e9 sumé)", R"(r\0000e9sumé)", R"(a\"b)"},
    {"x", "", "-x", "résumé", "résumé", "résumé", "a\"b"}
};

static char quotedTestSamples[2][64][128] = {
    {"\"x\"", "'x'", "'x`", "'x\\\r\nx'", R"("r\e9 sumé")", R"("r\0000e9 sumé")", R"("r\0000e9sumé")", R"("a\"b")"},
    {"x", "x", "", "xx", "résumé", "résumé", "résumé", "a\"b"}
};


extern bool Parser_skipWhitespace(Parser* p);

int main() {

    Parser p;

    // parseIdentifier Tests
    puts("parseIdentifier Tests\n");
	for (int i = 0; i < SAMPLES_NUM; ++i) {
        printf("Source string: '%s'\n", identifierTestSamples[0][i]);
        printf("Wanted: '%s'\n", identifierTestSamples[1][i]);

        Parser_init(&p, identifierTestSamples[0][i]);
		skipWhitespace(&p);

        String* result = string_new(NULL);
        ParserError err = parseIdentifier(&p, result);
        if (err != ParserError_NO_ERROR) {
            fprintf(stderr, "%s\n", ParserError_toString(err));
            result->str[0] = '\0';
        }
        printf("Result: '%s'\n", result->str);
        if (strcmp(result->str, identifierTestSamples[1][i]) == 0) {
            puts("Passed!");
        } else {
            puts("Failed!");
        }
        string_free(result);
		printf("***********************************\n");
    }

    // parseQuoted Tests
    puts("parseQuoted Tests\n");
    for (int i = 0; i < 8; ++i) {
        printf("Source string: '%s'\n", quotedTestSamples[0][i]);
        printf("Wanted: '%s'\n", quotedTestSamples[1][i]);
        Parser_init(&p, quotedTestSamples[0][i]);
		skipWhitespace(&p);

        String* result = string_new(NULL);
        ParserError err = parseQuoted(&p, result);
        if (err != ParserError_NO_ERROR) {
            fprintf(stderr, "%s\n", ParserError_toString(err));
            result->str[0] = '\0';
        }
        printf("Result: '%s'\n", result->str);
        if (strcmp(result->str, quotedTestSamples[1][i]) == 0) {
            puts("Passed!");
        } else {
            puts("Failed!");
        }
        string_free(result);
        printf("***********************************\n");
    }
	
	return 0;
}
