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
 * Link list structure function below
*/




/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_in):

Datum
inset_in(PG_FUNCTION_ARGS){
    //input string
    char *str = PG_GETARG_CSTRING(0);
    intSet *result;

    char delim[5] = "{, }";
    char *substring;

    int input_len = strlen(str);
    substring = strtok(str, delim);

    int *temp = palloc(VARHDRSZ*input_len);
    int size = 0;
    while(substring != NULL){
        temp[size] = atoi(substring);
        substring = strtok(NULL, delim);
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