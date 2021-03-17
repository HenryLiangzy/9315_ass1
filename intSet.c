#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0


/**
 * Declare
*/
void merge(int *arr, int l, int m, int r);
void mergeSort(int *arr, int l, int r);
int check_valid(char *str);


PG_MODULE_MAGIC;

typedef struct intSet
{
    int32 length;
    int array_size;
    int array[FLEXIBLE_ARRAY_MEMBER];
}intSet;


/**
 * Sort function below
*/

void merge(int *arr, int l, int m, int r){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int left[m - l + 1], right[r - m];

    //copy
    for(i = 0; i < n1; i++){
        left[i] = arr[l + i];
    }
    for(j = 0; j < n2; j++){
        right[j] = arr[m+1 + j];
    }

    i = 0;
    j = 0;
    k = l;

    //combine
    while (i < n1 && j < n2){
        if(left[i] <= right[j]){
            arr[k] = left[i];
            i++;
        }
        else{
            arr[k] = right[j];
            j++;
        }
        k++;
    }

    // copy remain
    while (i < n1){
        arr[k] = left[i];
        i++;
        k++;
    }

    while (j < n2){
        arr[k] = right[j];
        j++;
        k++;
    }
}

void mergeSort(int *arr, int l, int r){
    if(l < r){
        int m = (l + r) / 2;

        // divide
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);

        //merge
        merge(arr, l, m, r);
    }
}

int check_valid(char *str){
    int flag = 1;
    int str_len = strlen(str);

    // first check the head and tail
    if(str[0] != '{' && str[str_len-1] != '}'){
        flag = 0;
        return flag;
    }

    // for (int i = 0; i < str_len; i++){
    //     /* code */
    // }
    
    return flag;
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_in);

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
    int size_count = 1;
    while(substring != NULL){
        temp[size_count] = atoi(substring);
        substring = strtok(NULL, delim);
        size_count++;
    }

    // sort the array
    mergeSort(temp, 0, size_count-1);

    // allocate memory for result pointer and save data
    // an array length (int32) + array (len * int32)
    result = (intSet *)palloc(VARHDRSZ + VARHDRSZ * size_count);
    SET_VARSIZE(result, VARHDRSZ + VARHDRSZ * size_count);
    result->array_size = size_count
    for (int i = 0; i < size_count; i++){
        result->array[i] = temp[i];
    }

    //free and return
    pfree(temp);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(intset_out);

Datum
inset_out(PG_FUNCTION_ARGS){
    intSet *int_set = (intSet *) PG_GETARG_POINTER(0);

    int available_len = int_set->array_size * 8;
    int number_len = 0;
    char number_str[32];
    char *result = (char *)palloc(available_len + 2);
    
    // concate the first byte
    strcat(result, "{");
    sprintf(number_str, "%d", array[0]);
    strcat(result, number_str);

    //concate later string
    for (int i = 1; i < int_set->array_size; i++){
        strcat(result, ", ");
        sprintf(number_str, "%d", array[i]);
        number_len = strlen(number_str);
        if(strlen(result) + number_len >= available_len){
            result = realloc(result, available_len + 512);
            available_len = available_len + 512;
        }
        strcat(result, number_str);
    }
    
    //end
    strcat(result, "}");
    PG_RETURN_CSTRING(psprintf("%s", result));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Complex datatype would provide much more than this, of course.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_eq);

Datum
intset_eq(PG_FUNCTION_ARGS)
{
	intSet *a = (intSet *) PG_GETARG_POINTER(0);
	intSet *b = (intSet *) PG_GETARG_POINTER(1);

    int flag = 1;

    if(a->array_size != b->array_size){
        flag = 0;
    }
    else{
        for (int i = 0; i < a->array_size; i++){
            if (a->array[i] != b->array[i]){
                flag = 0;
                break;
            }
        }
    }

	PG_RETURN_BOOL(flag);
}
