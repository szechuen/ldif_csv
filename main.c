/* See https://github.com/szechuen/ldif_csv for README and LICENSE */

#include <stdio.h>
#include <string.h>

#include "base64.h"


/* START OF CONFIG */

/* Note: Affects memory usage
 * Default uses ~150KB
 * Increase only if necessary
 */

#define MAX_LINE_LEN 1024
#define MAX_KEY 128
#define MAX_KEY_LEN 128
#define MAX_VAL_LEN 1024

/*  END OF CONFIG  */


int is_empty(char * str);
int is_comment(char * str);
int strip_lead(char ** str);
int trim_nl(char * str);
int replace_quote(char * str);
int replace_nl(char * str);

int decode_base64(char * str);
int strncat_delimit(char * dest, char * src);

int split_keyval(char * keyval, char ** key_ptr, char ** val_ptr);


int main(int argc, char * argv[])
{
    /* Initialize ALL variables (darn ANSI) */

    char line[MAX_LINE_LEN];

    /* Cursors to key and value in line */
    char * key = NULL;
    char * val = NULL;

    char keys[MAX_KEY][MAX_KEY_LEN];
    unsigned long key_count = 0;
    unsigned long cur_key; /* For loop counter */

    char vals[MAX_KEY][MAX_VAL_LEN];
    int new_record; /* Actually a boolean */
    unsigned long record_count;


    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    
    /* Open files for loading and dumping */

    FILE * in_ptr = fopen(argv[1], "r");

    if(in_ptr == NULL)
    {
        fprintf(stderr, "Error opening file for reading\n");
        return 1;
    }

    FILE * out_ptr = fopen(argv[2], "w");

    if(out_ptr == NULL)
    {
        fprintf(stderr, "Error opening file for writing\n");
        return 1;
    }


    /* Pass 1: Detect all keys used */

    fprintf(stdout, "Detecting keys...\n");

    while(fgets(line, MAX_LINE_LEN, in_ptr) != NULL)
    {
        if(!is_empty(line) && !is_comment(line) && (split_keyval(line, &key, &val) == 0))
        {
            /* Search array for current key */
            for(cur_key = 0; cur_key < key_count; cur_key++)
            {
                if(strncmp(key, keys[cur_key], MAX_KEY_LEN) == 0) break;
            }

            /* Current key not seen so append to array */
            if(cur_key == key_count)
            {
                strncpy(keys[key_count], key, MAX_KEY_LEN);
                fprintf(stdout, "Key Detected: %s\n", keys[key_count]);
                key_count++;
            }
        }
    }

    fprintf(stdout, "Key Count: %lu\n\n", key_count);


    /* Pass 2: Parse and serialize into CSV using detected keys */

    rewind(in_ptr);

    fprintf(stdout, "Serializing CSV...\n");

    /* Print header row of detected keys and initialize value array
     * conveniently in the same loop
     */
    for(cur_key = 0; cur_key < key_count; cur_key++)
    {
        if(cur_key != 0) fputs(",", out_ptr);
        replace_quote(keys[cur_key]);
        fprintf(out_ptr, "\"%s\"", keys[cur_key]);

        *(vals[cur_key]) = '\0';
    }

    fputs("\n", out_ptr);

    new_record = 0; /* Set to boolean false */
    record_count = 0;

    while(fgets(line, MAX_LINE_LEN, in_ptr) != NULL)
    {
        /* End of new record */
        if(is_empty(line) && new_record)
        {
            /* Print record row and reinitialize value array */
            for(cur_key = 0; cur_key < key_count; cur_key++)
            {
                if(cur_key != 0) fputs(",", out_ptr);
                replace_quote(vals[cur_key]);
                fprintf(out_ptr, "\"%s\"", vals[cur_key]);

                *(vals[cur_key]) = '\0';
            }

            fputs("\n", out_ptr);

            new_record = 0; /* Reset to boolean false */
            record_count++;
        }

        /* Continuation of current record */
        else if(!is_comment(line))
        {
            if(split_keyval(line, &key, &val) == 0)
            {
                /* Search array for current key */
                for(cur_key = 0; cur_key < key_count; cur_key++)
                {
                    if(strncmp(key, keys[cur_key], MAX_KEY_LEN) == 0)
                    {
                        /* Decode Base64 fields */
                        if((*val == ':') && (*(val+1) == ' ')) decode_base64(val);

                        /* Value for current key has not been set */
                        if(*(vals[cur_key]) == '\0') strncpy(vals[cur_key], val, MAX_VAL_LEN);
                        /* Value for current key has been set, append to existing value */
                        else strncat_delimit(vals[cur_key], val);

                        new_record = 1; /* Set to boolean true */
                        break;
                    }
                }
            }
        }
    }

    /* Handle missing blank line at end of file */
    if(new_record)
    {
        /* Print record row and reinitialize value array */
        for(cur_key = 0; cur_key < key_count; cur_key++)
        {
            if(cur_key != 0) fputs(",", out_ptr);
            replace_quote(vals[cur_key]);
            fprintf(out_ptr, "\"%s\"", vals[cur_key]);

            *(vals[cur_key]) = '\0';
        }

        fputs("\n", out_ptr);

        new_record = 0; /* Reset to boolean false */
        record_count++;
    }

    fprintf(stdout, "Set Count: %lu\n\n", record_count);


    /* And... we're done! */
    fclose(out_ptr);
    fclose(in_ptr);

    return 0;
}


int split_keyval(char * keyval, char ** key_ptr, char ** val_ptr)
{
    char * key = keyval;
    char * val = keyval;

    while((*val != ':') && (*val != '\0')) val++;
    if(*val == '\0') return 1;

    *val = '\0';
    val++;

    strip_lead(&key);
    if(*key == '\0') return 1;

    strip_lead(&val);
    if(*val == '\0') return 1;
    trim_nl(val);

    *key_ptr = key;
    *val_ptr = val;

    return 0;
}


int decode_base64(char * str)
{ 
    char tmp[MAX_VAL_LEN];
    strncpy(tmp, str+2, MAX_VAL_LEN);

    /* Safe without length limit since decoded string should be shorter than
     * Base64 encoded string */
    Base64decode(str, tmp);
    replace_nl(str);

    return 0;
}

int strncat_delimit(char * dest, char * src)
{
    char tmp[MAX_VAL_LEN];

    strncpy(tmp, "; ", MAX_VAL_LEN);
    strncat(tmp, src, MAX_VAL_LEN-strlen(tmp));
    strncat(dest, tmp, MAX_VAL_LEN-strlen(dest));

    return 0;
}


int is_empty(char * str)
{
    if(*str == '\n') return 1;
    else return 0;
}

int is_comment(char * str)
{
    if(*str == '#') return 1;
    else return 0;
}

int strip_lead(char ** str)
{
    while(**str == ' ') (*str)++;

    return 0;
}

int trim_nl(char * str)
{
    while((*str != '\n') && (*str != '\0')) str++;

    *str = '\0';

    return 0;
}

int replace_quote(char * str)
{
    while(*str != '\0')
    {
        if(*str == '\"') *str = '\'';
        str++;
    }

    return 0;
}

int replace_nl(char * str)
{
    while(*str != '\0')
    {
        if(*str == '\n') *str = ' ';
        else if(*str == '\r') *str = ',';
        str++;
    }

    return 0;
}