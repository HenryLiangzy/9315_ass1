#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

PG_MODULE_MAGIC;

typedef struct intSet
{
    int32 length;
    int32 array[FLEXIBLE_ARRAY_MEMBER];
}intSet;


/**
 * Structure function below
*/

int check_valid(char *str){
    int flag = 1;
    int str_len = strlen(str);

    // first check the head and tail
    if(str[0] != '{' && str[str_len-1] != '}'){
        flag = 0;
        return flag;
    }

    for (int i = 0; i < str_len; i++){
        /* code */
    }
    
    return flag;
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_in):

Datum
inset_in(PG_FUNCTION_ARGS){
    //input string
    char *str = PG_GETARG_CSTRING(0);
    intSet *result = NULL;

    // input string check
    if(check_valid(str) == 0){
        ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type %s: \"%s\"",
						"intSet", str)));
    }

    char delim[5] = "{, }";
    char *substring = NULL;

    int input_len = strlen(str);
    substring = strtok(str, delim);

    int *temp = palloc(VARHDRSZ*input_len);
    int size = 0;
    while(substring != NULL){
        temp[size] = atoi(substring);
        substring = strtok(NULL, delim);
        size++;
    }

    for (int i = 0; i < size; i++){
        
    }
    

    //free and return
    pfree(temp);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(intset_out):

Datum
inset_out(PG_FUNCTION_ARGS){

}