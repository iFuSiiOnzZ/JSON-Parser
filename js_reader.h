#ifndef JS_READER_H_
#define JS_READER_H_

typedef struct JS_TYPES
{
    typedef enum
    {
        JS_UNDEFINED,   // NOTE(Andrei): We don't know what it is
        JS_BASIC,       // NOTE(Andrei): String, number, bool, null (not used)

        JS_OBJECT,      // NOTE(Andrei): A new object, creates new child
        JS_ARRAY,       // NOTE(Andrei): A new array, creates new child

        JS_NUMBER,      // NOTE(Andrei): A number, integer of floating point
        JS_STRING,      // NOTE(Andrei): String, has to start and end with '"'

        JS_BOOL,        // NOTE(Andrei): Is a boolean, true or false
        JS_NULL,        // NOTE(Andrei): NUll field
    } Enum;
} JS_TYPES;

typedef struct JS_NODE
{
    struct JS_NODE *Sibling;    // NOTE(Andrei): Keeps track of he data in the same structure
    struct JS_NODE *Childs;     // NOTE(Andrei): Contains the data inside a object or array

    char *Name;                 // NOTE(Andrei): Name of the field
    char *Value;                // NOTE(Andrei): In case is a data (not object or array) it will contain the data

    unsigned int Size;          // NOTE(Andrei): Size of the data and name, string is not null terminated [XXXX XXXX YYYY YYYY YYYY YYYY YYYY YYYY] (X -> name size, Y -> value size)
    JS_TYPES::Enum Type;        // NOTE(Andrei): This tells us what king of node we deal with, basic type (string, bool, etc), object or array
} JS_NODE;                      // NOTE(Andrei): json_sanitize sets null terminator to all strings

typedef struct JS_TOKENIZER
{
    char *At;
} JS_TOKENIZER;

JS_NODE * json_parser(char *json);

void json_free(JS_NODE *pNode);
void json_print(JS_NODE *pNode);

char * json_value(JS_NODE *pNode, char *pQuery, int *pArray = 0, int ArrayCount = 0);
int json_size(JS_NODE *pNode, char *pQuery, int *pArray = 0, int ArrayCount = 0);

#endif
