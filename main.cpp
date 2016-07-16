#include <stdio.h>
#include <stdlib.h>

#include "js_reader.h"

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
    char *pFileContent =  LoadInMemory("test.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    JS_NODE *pRootNode = json_root();
    json_parser(pRootNode, &Tokenizer);

    json_sanitize(pRootNode);
    json_clear(pRootNode);

    free(pFileContent);
    return EXIT_SUCCESS;
}