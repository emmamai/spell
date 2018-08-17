#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 256
#define INPUT_BUFFER_SIZE 2048

char* codeString = "\
( set true ( fn ( istrue isfalse ) ( istrue ) ) )\
( set false ( fn ( istrue isfalse) ( isfalse ) ) )\
( set if ( fn ( pred istrue isfalse ) ( pred istrue isfalse ) ) )\
\
( set testval true )\
\
( if testval\
    (printlist (value was true))\
    (printlist (value was false))\
)\

";

typedef enum {
    TYPE_NIL,
    TYPE_STRING,
    TYPE_LIST
} listItemType_t;

typedef struct listItem_s {
    listItemType_t type;
    union {
        char* asString;
        struct listItem_s* asList;
        void* asVoid;
    };
    struct listItem_s* next;
} listItem_t;

typedef listItem_t list_t;

list_t* ListAppend( listItem_t* item, list_t* list ) {
    list_t* l = list;

    if ( item == NULL )
        return list;
    if ( list == NULL )
        return item;

    while( l->next != NULL )
        l = l->next;

    l->next = item;
    return list;
}

listItem_t* CreateListItemString( char* str ) {
    listItem_t* item = calloc( 1, sizeof( listItem_t ) );

    item->type = TYPE_STRING;
    item->asString = calloc( 1, strlen( str ) + 1 );
    strcpy( item->asString, str );

    return item;
}

listItem_t *CreateListItemList( listItem_t* list ) {
    listItem_t *item = calloc(1, sizeof(listItem_t));

    item->type = TYPE_LIST;
    item->asList = list;

    return item;
}

int PrimEval( list_t* list ) {

}

listItem_t* ParseRecursiveParens( char* str, int* skip_ret ) {
    int len = strlen( str );
    int i = 0;
    int skip;
    list_t* list = NULL;
    listItem_t* item;
    listItem_t* subList;
    int tokenStart, tokenLen;
    char buf[MAX_TOKEN_LEN];
    bool inToken = false;
    bool inList = false;


    //go forward until we hit a left paren, or the end
    for ( ; ( i < len ) && ( str[i] != '(' ); i++ );
    if ( i++ >= len ) {
        fprintf( stderr, "warning: reached end of file without parsing any lists." );
        return NULL;
    }
    printf( "start of new list\n" );
    inList = true;
    do {
        switch ( str[i] ) {
            case ')':
                inList = false;
            case ' ':
            case '\n':
            case '\t':
                if ( inToken ) {
                    //create new list item out of token
                    tokenLen = i - tokenStart;
                    if ( tokenLen > MAX_TOKEN_LEN - 1) {
                        fprintf( stderr, "error: token exceeds max token length\n" );
                        return NULL;
                    }
                    memset( buf, '\0', MAX_TOKEN_LEN );
                    strncpy( &buf[0], &str[tokenStart], tokenLen );
                    printf( "new token: '%s'\n", buf );
                    item = CreateListItemString( buf );
                    list = ListAppend( CreateListItemString( buf ), list );
                    inToken = false;
                }
                break;
            case '(':
                if ( inToken )
                    break;
                subList = ParseRecursiveParens( str + i, &skip );
                if ( subList == NULL ) {
                    return NULL;
                }
                item = CreateListItemList( subList );
                list = ListAppend( item, list );
                printf( "new list at 0x%08X\nskipping %i\n", (unsigned int)subList, skip );
                i += skip;
                break;
            default:
                if ( ! inToken ) {
                    inToken = true;
                    tokenStart = i;
                }
                break;
        }
    } while ( str[i++] != ')' && i < len );

    if ( str[i-1] == ')' )
        inList = false;

    if ( ( inList || inToken ) ) {
        fprintf( stderr, "error: missing end paren\n" );
        return NULL;
    }

    if ( str[i-1] != ')' ) {
        fprintf( stderr, "warning: extraneous characters after end paren\n" );
    }

    if ( skip_ret != NULL )
        *skip_ret = i - 1;

    printf( "end of list\n" );

    return list;
}

void PrintListRecursive( list_t* list ) {
    if ( !list )
        return;
    printf( "( " );
    do {
        switch( list->type ) {
            case TYPE_STRING:
                    if ( list->asString )
                        printf( "%s ", list->asString );
                    break;
            case TYPE_LIST:
                    if ( list->asList ) {
                        PrintListRecursive( list->asList );
                        printf(" ");
                    }
                    break;
            default:
                    break;
        }
        list = list->next;
    } while ( list != NULL );
    printf( ")" );
}

int main( int argc, char** argv ) {
    list_t* list;
    char* buf = malloc( 2048 );
    int pos, skip;

    while ( true ) {
        memset( buf, 0, INPUT_BUFFER_SIZE );
        printf( "spell> " );
        scanf( "%s", buf );
        pos = 0;
        while ( pos < strlen( buf ) ) {
            list = ParseRecursiveParens( &buf[pos], &skip );
            if ( list == NULL )
                break;
            pos += skip;
            PrintListRecursive( list );
            printf( "\n" );
        }
    }

    printf( "input is as follows:\n\n%s\n\n", codeString );
    printf( "parsing input\n" );
    list = ParseRecursiveParens( codeString, NULL );
    if ( list == NULL )
        return 1;
    printf( "\nfinal list at 0x%08X\n", (unsigned int)list->asList );
    printf("\nprinting list from data structures:\n" );
    PrintListRecursive( list );
    printf("\n");
    return 0;
}
