#include <stdio.h>
#include <stdlib.h>

// for libtidy
typedef unsigned int uint;
typedef unsigned long ulong;

#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#include "lego.h"
#include "util/slist.h"


static char* readFile(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		perror("Error opening file");
		return NULL;
	}
	fseek(fp, 0L, SEEK_END);
	long int file_size = ftell(fp);
	if (file_size < 1) {
		fprintf(stderr, "Empty file.\n");
		fclose(fp);
		return NULL;
	}
	rewind(fp);
	char* buffer = (char*)malloc(sizeof(char) * (size_t)file_size + 1);
	if (!buffer) {
		perror("Memory allocation error");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	size_t bytes_num = fread(buffer, sizeof(char), (size_t)file_size, fp);
	if ((bytes_num < 1) || ferror(fp)){
		perror("Error reading file");
		free(buffer);
		fclose(fp);
		return NULL;	
	}
	fclose(fp);
	buffer[bytes_num] = '\0';
	return buffer;
}

static void cb_printFunc(TidyDoc tdoc, TidyNode node, void* userdata) {
	TidyBuffer buf = {0};
	tidyNodeGetText(tdoc, node, &buf);
	// Get rid of trailing '\n'-s
	for (; buf.size-1 > 0 && buf.bp[buf.size-1] == '\n'; --buf.size, buf.bp[buf.size] = '\0');
//	String* s = CombinedSelector_string(*sel_ptr);
	printf("Matched node: '%s'\n",
		   buf.bp
		   );
	tidyBufFree(&buf);
//	string_free(s);
}

int main() {
	#define FILE_NAME "wikipedia.html"

	char* content = readFile(FILE_NAME);
	if (content == NULL) {
		fprintf(stderr, "Error reading file %s\n", FILE_NAME);
		exit(EXIT_FAILURE);
	}
	
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;
	TidyDoc tdoc = tidyCreate();
	tidySetErrorBuffer(tdoc, &errbuf);

	rc = tidyParseString(tdoc, content);
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
				"libtide error [%d] parsing string '%s'\n", rc, content
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
	printf("News block name: %s\n", tidyNodeGetName(news_block));
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

	free(content);

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	return 0;
}
