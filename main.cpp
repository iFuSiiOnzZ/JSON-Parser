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
    char *pFileContent =  LoadInMemory("json_test_1.json");
    JS_TOKENIZER Tokenizer = { pFileContent };

    JS_NODE *pRootNode = json_root();
    json_parser(pRootNode, &Tokenizer);

    json_sanitize(pRootNode);
    int pArray[1] = { 0 };

    test_check(pRootNode, "root./.storage.type", "disk");
    test_check(pRootNode, "root./.storage.device", "/dev/sda1");
    test_check(pRootNode, "root./.fstype", "btrfs");
    test_check(pRootNode, "root./.readonly", "true");

    printf("%s\n", "==============================");

    test_check(pRootNode, "root./var.storage.type", "disk");
    test_check(pRootNode, "root./var.storage.label", "8f3ba6f4-5c70-46ec-83af-0d5434953e5f");
    test_check(pRootNode, "root./var.fstype", "ext4");
    test_check(pRootNode, "root./var.options", "nosuid", pArray, 1);

    printf("%s\n", "==============================");

    test_check(pRootNode, "root./tmp.storage.type", "tmpfs");
    test_check(pRootNode, "root./tmp.storage.sizeInMB", "64");

    printf("%s\n", "==============================");

    test_check(pRootNode, "root./var/www.storage.type", "nfs");
    test_check(pRootNode, "root./var/www.storage.server", "my.nfs.server");
    test_check(pRootNode, "root./var/www.storage.remotePath", "/exports/mypath");

    printf("%s\n", "==============================");
    printf("%.02f%% %s\n", (g_NumTestsOK * 100.0f / g_NumTests), " of the tests where OK");

    json_clear(pRootNode);
    free(pFileContent);

    return EXIT_SUCCESS;
}