#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#include <stdio.h>
#include <ctype.h>

PG_MODULE_MAGIC;

typedef struct intSet
{
    /* data */
    int length;
    int *array;
}intSet;


PG_FUNCTION_INFO_V1(intset_in):

Datum
inset_in(PG_FUNCTION_ARGS){
    char *str = PG_GETARG_CSTRING(0);
    intSet *result;
}

PG_FUNCTION_INFO_V1(intset_out):

Datum
inset_out(PG_FUNCTION_ARGS){

}