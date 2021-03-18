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

    int *left = (int *)palloc(VARHDRSZ * n1);
    int *right = (int *)palloc(VARHDRSZ * n2);

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

    pfree(left);
    pfree(right);
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
    int str_len = strlen(str);
    int flag = 0;
    int comma_flag = 1;
    int number_flag = 0;
    int bracket[2] = {0, 0};

    // first check the head and tail
    for(int i = 0; i < str_len; i++){

        // check bracket
        if(str[i] == '{'){
            bracket[0]++;
        }
        else if(str[i] == '}'){
            bracket[1]++;
        }

        // check invalid character
        if(str[i] != ' ' && str[i] != ',' && str[i] != '{' && str[i] != '}' && !isdigit(str[i])){
            printf("invalid character!\n");
            return 0;
        }

        // check is digit
        if(isdigit(str[i])){
            // when the digit is the last char
            if(i + 1 ==  str_len){
                printf("number should not be the last digit!\n");
                return 0;
            }
            else{
                //if next digit still a number
                if(isdigit(str[i+1])){
                    continue;
                }
                //current number finish
                else{
                    // if current number did not have comma before
                    if(comma_flag != 1){
                        printf("loose a comma before!\n");
                        return 0;
                    }
                    // is valid number, hence reverse comma flag and mark number flag
                    else{
                        flag = 1;
                        number_flag = 1;
                        comma_flag = 0;
                    }
                }
            }
        }
        // if is comma, check pervious number or space
        else if(str[i] == ','){
            // if no number detect before
            if(number_flag != 1){
                printf("No number detect before!\n");
                return 0;
            }
            //valid comma, reverse comma and number
            else{
                number_flag = 0;
                comma_flag = 1;
            }
        }

        // means when the string ending
        else if(str[i] == '}'){
            if(comma_flag == 1 && flag == 1){
                return 0;
            }
        }
    }

    // if the bracket not semi
    if(bracket[0] != 1 || bracket[1] != 1){
        printf("bracket is invalid\n");
        return 0;
    }

    // pass all the check, is valid number
    return 1;
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_in);

Datum
intset_in(PG_FUNCTION_ARGS){
    //input string
    char *str = PG_GETARG_CSTRING(0);
    intSet *result = NULL;

    char delim[5] = "{, }";
    char *substring = NULL;
    int input_len = strlen(str);
    int *temp = NULL;
    int size_count = 0;
    int *final_array = NULL;
    int final_array_size = 1;

    // input string check
    if(check_valid(str) == 0){
        ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type %s: \"%s\"",
						"intSet", str)));
    }

    // load the data from string
    substring = strtok(str, delim);
    temp = (int *)palloc(VARHDRSZ*input_len);
    while(substring != NULL){
        temp[size_count] = atoi(substring);
        substring = strtok(NULL, delim);
        size_count++;
    }

    // check if nothing in the string
    if(size_count == 0){
        pfree(temp);

        result = (intSet *)palloc(VARHDRSZ * 2 + VARHDRSZ * 2);
        SET_VARSIZE(result, VARHDRSZ * 2 + VARHDRSZ * 2);

        result->array_size = 0;
        result->array[0] = -1;

    }
    else{
        // sort and resize the array
        mergeSort(temp, 0, size_count-1);
        
        final_array = (int *)palloc(VARHDRSZ * size_count);
        final_array[0] = temp[0];
        for(int i = 1; i < size_count; i++){
            if(temp[i] <= final_array[final_array_size-1]){
                continue;
            }
            else{
                final_array[final_array_size] = temp[i];
                final_array_size++;
            }
        }

        pfree(temp);

        // allocate memory for result pointer and save data
        // an array length (int32) + array (len * int32)
        result = (intSet *)palloc(VARHDRSZ * 2 + VARHDRSZ * final_array_size);
        SET_VARSIZE(result, VARHDRSZ * 2 + VARHDRSZ * final_array_size);
        result->array_size = final_array_size;
        for (int i = 0; i < final_array_size; i++){
            result->array[i] = final_array[i];
        }

        pfree(final_array);
    }

    // elog(INFO, "size is %d", result->array_size);
    // for(int x=0; x<size_count;x++){
    //     elog(INFO, " <%d>", result->array[x]);
    // }

    //free and return
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(intset_out);

Datum
intset_out(PG_FUNCTION_ARGS){
    intSet *int_set = (intSet *) PG_GETARG_POINTER(0);

    int number_len = 0;
    char number_str[32];
    char *result = NULL;

    if (int_set->array_size == 0){
        result = (char *)palloc(sizeof(char) * 3);
        result[0] = '\0';
        strcat(result, "{}");
    }
    else{
        // elog(INFO, "there are %d in set: ", int_set->array_size);

        for (int i = 0; i < int_set->array_size; i++){
            sprintf(number_str, "%d", int_set->array[i]);
            number_len = number_len + strlen(number_str) + 1;
        }
        
        result = (char *)palloc(number_len + 8);

        // init string
        result[0] = '\0';
        
        // concate the first byte
        strcat(result, "{");
        sprintf(number_str, "%d", int_set->array[0]);
        strcat(result, number_str);
        // elog(INFO, "<%s>", result);

        //concate later string
        for (int i = 1; i < int_set->array_size; i++){
            strcat(result, ",");
            sprintf(number_str, "%d", int_set->array[i]);
            strcat(result, number_str);
            // elog(INFO, "<%s>", result);
        }
        
        //end
        strcat(result, "}");
    }
    // result[number_len+1] = '\0';
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


PG_FUNCTION_INFO_V1(intset_cardinality);
Datum
intset_cardinality(PG_FUNCTION_ARGS)
{
    intSet *input = (intSet *) PG_GETARG_POINTER(0);

    PG_RETURN_INT32(input->array_size);
}


PG_FUNCTION_INFO_V1(intset_contain);
Datum
intset_contain(PG_FUNCTION_ARGS)
{
    int32 number = PG_GETARG_INT32(0);
    intSet *int_set = (intSet *) PG_GETARG_POINTER(1);

    int flag = 0;

    for (int i = 0; i < int_set->array_size; i++){
        if(int_set->array[i] == number){
            flag = 1;
            break;
        }
    }

    PG_RETURN_BOOL(flag);
}


PG_FUNCTION_INFO_V1(intset_superset);
Datum
intset_superset(PG_FUNCTION_ARGS)
{
    intSet *a = (intSet *) PG_GETARG_POINTER(0);
    intSet *b = (intSet *) PG_GETARG_POINTER(1);

    int result = 1;

    for (int i = 0; i < b->array_size; i++){
        int flag = 0;
        for (int j = 0; j < a->array_size; j++){
            if(b->array[i] == a->array[j]){
                flag = 1;
            }
        }

        if(flag != 1){
            result = 0;
            break;
        }    
    }

    PG_RETURN_BOOL(result);
}
