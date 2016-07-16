#include "js_reader.h"
#include <stdlib.h>

#define cast(type, data) (type)(data)
#define DefaultName "root"

typedef enum JS_TOKENS
{
    Token_Unknown,          //
    Token_Field,            // "

    Token_Comma,            // ,
    Token_Colon,            // :

    Token_OpenBraket,       // [
    Token_CloseBraket,      // ]

    Token_OpenBrace,        // {
    Token_CloseBrace,       // }

    Token_EndOfStream       // '\0'
} JS_TOKENS;

typedef struct JS_TOKEN
{
    char *Name;             // NOTE(Andrei): Token Name
    size_t Size;            // NOTE(Andrei): Size of the name, we don't use null terminated string
    JS_TOKENS Type;         // NOTE(Andrei): Token types, '[' | ']', '{' | '}', ':' , '"', ',', '\0'
} JS_TOKEN;

typedef struct queue_t      // NOTE(Andrei): This is used to store the current json node as a parent.
{                           // if we found and array or an object we push the actual json node to queue,
    struct queue_t *Prev;   // this way when we end parsing the structure we came back to node where we
    JS_NODE *Data;          // were before parsing the structure
} queue_t;

inline bool IsEndOfLine(char C)
{
    bool l_bEndOfLine = (C == '\n') || (C == '\r');
    return l_bEndOfLine;
}

inline bool IsWhiteSpace(char C)
{
    bool l_bWhiteSPace = ((C == ' ') || (C == '\t') || (C == '\v') || (C == '\f') || IsEndOfLine(C));
    return l_bWhiteSPace;
}

inline bool IsAlpha(char C)
{
    bool l_bAlpha = (((C >= 'a') && (C <= 'z')) || ((C >= 'A') && (C <= 'Z')));
    return l_bAlpha;
}

inline bool IsNumber(char C)
{
    bool l_bIsNumeric = ((C >= '0') && (C <= '9'));
    return l_bIsNumeric;
}

inline void RemoveSpace(JS_TOKENIZER *pTokenizer)
{
    while (IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }
}

static JS_TOKEN GetToken(JS_TOKENIZER *pTokenizer)
{
    JS_TOKEN Token = { 0 };
    RemoveSpace(pTokenizer);

    Token.Name = pTokenizer->At;
    Token.Size = 1;

    char ActualChar = pTokenizer->At[0];
    ++pTokenizer->At;

    switch(ActualChar)
    {
        case '\0' : Token.Type = Token_EndOfStream; break;

        case '{'  : Token.Type = Token_OpenBrace; break;
        case '}'  : Token.Type = Token_CloseBrace; break;

        case '['  : Token.Type = Token_OpenBraket; break;
        case ']'  : Token.Type = Token_CloseBraket; break;

        case '"'  : Token.Type = Token_Field; break;

        case ','  : Token.Type = Token_Comma; break;
        case ':'  : Token.Type = Token_Colon; break;
        default   : Token.Type = Token_Unknown; break;
    }

    return Token;
}

static void GetFieldName(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    RemoveSpace(pTokenizer);
    pNode->Name = pTokenizer->At;

    while(pTokenizer->At[0] != '\0' && pTokenizer->At[0] != '"')
    {
        ++pTokenizer->At;
    }

    json_set_name_size(pNode->Size, cast(int, pTokenizer->At - pNode->Name));
    if(pTokenizer->At[0] == '"') ++pTokenizer->At;
}

static void ParseBool(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = (pTokenizer->At - 1);
    pNode->Type = JS_BOOL;

    RemoveSpace(pTokenizer);
    while(pTokenizer->At[0] && pTokenizer->At[0] != ',' && pTokenizer->At[0] != ']' && pTokenizer->At[0] != '}' && !IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
}

static void ParseNumber(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = (pTokenizer->At - 1);
    pNode->Type = JS_NUMBER;

    while(pTokenizer->At[0] && pTokenizer->At[0] != ',' && pTokenizer->At[0] != ']' && pTokenizer->At[0] != '}' && !IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
}

static void ParseNull(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = (pTokenizer->At - 1);
    pNode->Type = JS_NULL;

    while(pTokenizer->At[0] && pTokenizer->At[0] != ',' && pTokenizer->At[0] != ']' && pTokenizer->At[0] != '}' && !IsWhiteSpace(pTokenizer->At[0]))
    {
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
}

static void ParseString(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    pNode->Value = pTokenizer->At;
    pNode->Type = JS_STRING;

    while(pTokenizer->At[0] != '\0' && pTokenizer->At[0] != '"')
    {
        ++pTokenizer->At;
    }

    json_set_data_size(pNode->Size, cast(int, pTokenizer->At - pNode->Value));
    if(pTokenizer->At[0] == '"') ++pTokenizer->At;
}

static void GetFieldData(JS_TOKENIZER *pTokenizer, JS_NODE *pNode)
{
    RemoveSpace(pTokenizer);
    char ActualChar = *pTokenizer->At++;

    switch(ActualChar)
    {
        case '"'  :
        {
            ParseString(pTokenizer, pNode);
        } break;

        case 'n'  :
        case 'N'  :
        {
            ParseNull(pTokenizer, pNode);
        } break;

        case 't'  :
        case 'T'  :
        case 'f'  :
        case 'F'  :
        {
            ParseBool(pTokenizer, pNode);
        }break;

        case '-'  :
        case '0'  :
        case '1'  :
        case '2'  :
        case '3'  :
        case '4'  :
        case '5'  :
        case '6'  :
        case '7'  :
        case '8'  :
        case '9'  :
        {
            ParseNumber(pTokenizer, pNode);
        } break;

        case '{'  :
        {
            pNode->Type = JS_OBJECT;
            --pTokenizer->At;
        } break;

        case '['  :
        {
            pNode->Type = JS_ARRAY;
            --pTokenizer->At;
        } break;

        case '\0' :
        default   :  break;
    }
}

static queue_t *g_Queue = NULL;

static void push(JS_NODE *pNode)
{
    if(g_Queue == NULL)
    {
        g_Queue = (queue_t *) malloc (sizeof(queue_t));
        g_Queue->Prev = NULL;
    }
    else
    {
        queue_t *pNewNode = (queue_t *) malloc (sizeof(queue_t));
        pNewNode->Prev = g_Queue;
        g_Queue = pNewNode;
    }

    g_Queue->Data = pNode;
}

static JS_NODE * pop()
{
    if(g_Queue == NULL)
    {
        return NULL;
    }

    JS_NODE *pDataNode = g_Queue->Data;
    queue_t *pQueueNode = g_Queue;

    g_Queue = g_Queue->Prev;
    free(pQueueNode);

    return pDataNode;
}

void json_parser(JS_NODE *pRootNode, JS_TOKENIZER *pTokenizer)
{
    RemoveSpace(pTokenizer);
    JS_TOKEN Token = GetToken(pTokenizer);

    switch(Token.Type)
    {
        case Token_EndOfStream:
        {
            return;
        } break;

        case Token_CloseBraket: case Token_CloseBrace:
        {
             pRootNode = pop();
        } break;

        case Token_OpenBrace:
        {
            if(pRootNode->Type == JS_UNDEFINED)
            {
                pRootNode->Name = DefaultName;
            }

            pRootNode->Type = JS_OBJECT;
            pRootNode->Childs = (JS_NODE *) malloc (sizeof(JS_NODE));

            push(pRootNode);
            json_default(pRootNode->Childs);

            pRootNode = pRootNode->Childs;
        } break;

        case Token_OpenBraket:
        {
            if(pRootNode->Type == JS_UNDEFINED)
            {
                pRootNode->Name = DefaultName;
            }

            pRootNode->Type = JS_ARRAY;
            pRootNode->Childs = (JS_NODE *) malloc (sizeof(JS_NODE));

            push(pRootNode);
            json_default(pRootNode->Childs);

            pRootNode = pRootNode->Childs;
        } break;

        case Token_Comma:
        {
            pRootNode->Sibling = (JS_NODE *) malloc (sizeof(JS_NODE));
            json_default(pRootNode->Sibling);
            pRootNode = pRootNode->Sibling;
        } break;

        case Token_Field:
        {
            GetFieldName(pTokenizer, pRootNode);
        } break;

        case Token_Colon:
        {
            GetFieldData(pTokenizer, pRootNode);
        } break;

        default:
        {
            --pTokenizer->At;
            GetFieldData(pTokenizer, pRootNode);
        } break;
    }

    json_parser(pRootNode, pTokenizer);
}

void json_clear(JS_NODE *pNode)
{
    if(pNode == NULL) return;

    json_clear(pNode->Childs);
    json_clear(pNode->Sibling);

    free(pNode);
}

void json_sanitize(JS_NODE * pNode)
{
    if(pNode == NULL) return;

    json_sanitize(pNode->Childs);
    json_sanitize(pNode->Sibling);

    int n = json_get_name_size(pNode->Size);
    int v = json_get_data_size(pNode->Size);

    if(n) pNode->Name[n] = 0;
    if(v) pNode->Value[v] = 0;
}

void json_default(JS_NODE *pNode)
{
    pNode->Childs = NULL;
    pNode->Sibling = NULL;

    pNode->Name = NULL;
    pNode->Value = NULL;

    pNode->Size = 0;
    pNode->Type = JS_UNDEFINED;
}

JS_NODE * json_root()
{
    JS_NODE *pRootNode = (JS_NODE *) malloc (sizeof(JS_NODE));
    json_default(pRootNode);

    return pRootNode;
}