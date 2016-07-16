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
    // NOTE(Andrei): json examples here http://json-schema.org/
    char *pFileContent =  LoadInMemory("test.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    JS_NODE *pRootNode = json_root();
    json_parser(pRootNode, &Tokenizer);

    json_sanitize(pRootNode);
    int pArray[1] = { 1 };

    char *s = json_value(pRootNode, "root.address.streetAddress");
    int i = json_size(pRootNode, "root.phoneNumbers");

    s = json_value(pRootNode, "root.phoneNumbers.type", pArray, 1);
    s = json_value(pRootNode, "root.phoneNumbers.number", pArray, 1);

    json_clear(pRootNode);
    free(pFileContent);

    return EXIT_SUCCESS;
}