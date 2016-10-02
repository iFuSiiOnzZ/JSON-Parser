#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "js_reader.h"

#define UNUSED(X) (void)X
#define CAT(X, Y) X##Y

int g_NumTests = 0;
int g_NumTestsOK = 0;


void test_check(JS_NODE *pRootNode, char *jPath, char *jValue, int *pArray = NULL, int i = 0)
{
    printf("%-40s --> %s\n", jPath, jValue);
    char *jFoundValue = json_value(pRootNode, jPath, pArray, i);

    int OK = !strcmp(jFoundValue, jValue);
    printf("%-40s --> %s\n\n", OK ? "OK" : "FAIL", jFoundValue);

    ++g_NumTests;
    if(OK) ++g_NumTestsOK;
}

//////////////////////////////////////////////////////////////////////////
#include <sys/stat.h>
unsigned int GetFileSize(const char *pFileName)
{
    struct stat stat_buf = { 0 };
    int rc = stat(pFileName, &stat_buf);

    return rc == 0 ? stat_buf.st_size : 0;
}

char *LoadInMemory(const char *pFileName)
{
    char *pFileContent = nullptr;
    FILE *pFileHandle = nullptr;

    fopen_s(&pFileHandle, pFileName, "r");
    if (!pFileHandle) return pFileContent;

    unsigned int FileSize = GetFileSize(pFileName);
    pFileContent = (char *) malloc(sizeof(char) * (FileSize + 1));

    size_t r = fread(pFileContent, 1, FileSize, pFileHandle);
    pFileContent[r] = '\0';

    fclose(pFileHandle);
    return pFileContent;
}
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    // NOTE(Andrei): json examples here http://json-schema.org/
    UNUSED(argc); UNUSED(argv);

    //http://json-schema.org/example2.html (/etc/fstab)
    char *pFileContent =  LoadInMemory("json_test_2.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    JS_NODE *pRootNode = json_root();
    json_parser(pRootNode, &Tokenizer);

    json_sanitize(pRootNode);
    json_print(pRootNode);

    json_clear(pRootNode);
    free(pFileContent);

    return EXIT_SUCCESS;
}