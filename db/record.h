#ifndef RECORD_H
#define RECORD_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    RECORD_TYPE_INT=1,
    RECORD_TYPE_TEXT=2
} value_type_t;

typedef struct
{
    char *data;
    size_t length;
} text_value_t;

typedef enum {
    RECORD_OK=0,
    RECORD_INVALID_INPUT=1,
    RECORD_TYPE_MISMATCH=2,
    RECORD_OUT_OF_MEMORY=3,
    RECORD_BUFFER_CORRUPT=4
} record_status_t;


typedef struct
{
    value_type_t value_type;
    union value
    {
        int64_t int_value;
        text_value_t text_value;
    } data;
    
} value_t;


typedef struct
{
    char *column_name;
    value_type_t column_type;
    size_t max_text_length;
} column_definition_t;


typedef struct
{
    column_definition_t *columns;
    size_t column_count;
} schema_definition_t;


typedef struct 
{
    value_t *values;
    size_t value_count;
    
} record_t;


typedef struct
{
    uint8_t *data;
    size_t data_length;
} encoded_record_t;

/* This function encodes a record to bytes*/
record_status_t record_encode(schema_definition_t schema, record_t record, encoded_record_t *encoded_record);

/*This function decodes an already decoded byte record back to the actual record*/
record_status_t record_decode(schema_definition_t schema, encoded_record_t encoded_record, record_t *record);

void free_encoded_record(encoded_record_t *encoded_record);

void free_record(record_t *record);

#endif