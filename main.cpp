#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "js_reader.h"

#define UNUSED(X) (void)X
#define CAT(X, Y) X##Y

int g_NumTests = 0;
int g_NumTestsOK = 0;

void check_value(JS_NODE *pRootNode, char *jPath, char *jValue, int *pArray = NULL, int i = 0)
{
    printf("%-40s --> %s\n", jPath, jValue);
    char *jFoundValue = json_value(pRootNode, jPath, pArray, i);

    int OK = jFoundValue ? strcmp(jFoundValue, jValue) == 0 : 1;
    printf("%-40s --> %s\n\n", OK ? "OK" : "FAIL", jFoundValue);

    ++g_NumTests;
    if(OK) ++g_NumTestsOK;
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    UNUSED(argc); UNUSED(argv);
    const char fmt[] =
        "{"
        "    \"squadName\": \"Super hero squad\","
        "    \"homeTown\": \"Metro City\","
        "    \"formed\": 2016,"
        "    \"secretBase\": \"Super tower\","
        "    \"active\": true,"
        "    \"members\": [{"
        "            \"name\": \"Molecule Man\","
        "            \"members\": 29,"
        "            \"secretIdentity\": \"Dan Jukes\","
        "            \"powers\": ["
        "                \"Radiation resistance\","
        "                \"Turning tiny\","
        "                \"Radiation blast\""
        "            ]"
        "        },"
        "        {"
        "            \"name\": \"Madame Uppercut\","
        "            \"age\": 39,"
        "            \"secretIdentity\": \"Jane Wilson\","
        "            \"powers\": ["
        "                \"Million tonne punch\","
        "                \"Damage resistance\","
        "                \"Superhuman reflexes\""
        "            ]"
        "        }"
        "    ]"
        "}";

    char jsonExample[sizeof(fmt) + 1];
    strcpy_s(jsonExample, fmt);

    JS_NODE *pRootNode = json_parser(jsonExample);
    json_print(pRootNode);

    check_value(pRootNode, "root.squadName", "Super hero squad");
    check_value(pRootNode, "root.secretBase", "Super tower");

    int arr1[] = { 1 }; // root.members[1].age
    check_value(pRootNode, "root.members.age", "39", arr1, sizeof(arr1));

    int arr2[] = { 0 }; // root.members[0].members
    check_value(pRootNode, "root.members.members", "29", arr2, sizeof(arr2));

    int arr3[] = { 1, 2 }; // root.members[1].powers[2]
    check_value(pRootNode, "root.members.powers", "Superhuman reflexes", arr3, sizeof(arr3));

    json_free(pRootNode);
    return EXIT_SUCCESS;
}
