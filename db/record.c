#include "record.h"
#include <stdlib.h>
#include <string.h>



record_status_t record_encode(schema_definition_t schema, record_t record, encoded_record_t *encoded_record){
    if (encoded_record==NULL){
        return RECORD_INVALID_INPUT;
    }

    encoded_record->data = NULL;
    encoded_record->data_length=0;

    if (schema.column_count < 1){
        return RECORD_INVALID_INPUT;
    }

    if (schema.column_count > 0 && schema.columns==NULL){
        return RECORD_INVALID_INPUT;
    }

    if (record.value_count < 1){
        return RECORD_INVALID_INPUT;
    }

    if (record.value_count > 0 && record.values == NULL){
        return RECORD_INVALID_INPUT;
    }

    if (schema.column_count != record.value_count){
        return RECORD_INVALID_INPUT;
    }


    for (size_t i=0; i < record.value_count; i++){
        if(record.values[i].value_type != schema.columns[i].column_type){
            return RECORD_TYPE_MISMATCH;
        }

        if (record.values[i].value_type==RECORD_TYPE_TEXT && ((record.values[i].data.text_value.length > schema.columns[i].max_text_length) || (record.values[i].data.text_value.length > 0 && record.values[i].data.text_value.data==NULL))){
            return RECORD_INVALID_INPUT;
        }
        
    }

    
    /*Calculate memory needed*/
    size_t value_mem = sizeof(uint32_t);  //adds mem for the value count

    for (size_t i=0; i < record.value_count; i++){

        if(record.values[i].value_type == RECORD_TYPE_TEXT){
            value_mem += sizeof(int32_t); //add mem for the text length
            value_mem += sizeof(char) * record.values[i].data.text_value.length; //add mem for the actual text
        }

        else {
            value_mem += sizeof(int64_t);  //adds mem for int type data
        }
    }

    //Allocate the memroy for the encoded record
    encoded_record->data = malloc(value_mem);
    encoded_record->data_length = value_mem;

    if (encoded_record->data == NULL) {
        return RECORD_OUT_OF_MEMORY;
    }

    ///Write data to memory
    uint32_t value_count = (uint32_t)record.value_count;
    size_t offset = 0;

    memcpy(encoded_record->data + offset, &value_count, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for (size_t i = 0; i < record.value_count; i++) {
        if (record.values[i].value_type == RECORD_TYPE_TEXT) {
            uint32_t text_length = (uint32_t)record.values[i].data.text_value.length;

            memcpy(encoded_record->data + offset, &text_length, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            memcpy(
                encoded_record->data + offset,
                record.values[i].data.text_value.data,
                record.values[i].data.text_value.length
            );
            offset += record.values[i].data.text_value.length;
        } else {
            memcpy(
                encoded_record->data + offset,
                &record.values[i].data.int_value,
                sizeof(int64_t)
            );
            offset += sizeof(int64_t);
        }
    }

    return RECORD_OK;

}


record_status_t record_decode(schema_definition_t schema, encoded_record_t encoded_record, record_t *record){
    if (record==NULL){
        return RECORD_INVALID_INPUT;
    }

    record->values = NULL;
    record->value_count = 0;

    if (schema.column_count < 1){
        return RECORD_INVALID_INPUT;
    }

    if (schema.column_count > 0 && schema.columns==NULL){
        return RECORD_INVALID_INPUT;
    }

    if (encoded_record.data_length <= 0){
        return RECORD_INVALID_INPUT;
    }

    if (encoded_record.data==NULL){
        return RECORD_INVALID_INPUT;
    }

    if (encoded_record.data_length < sizeof(uint32_t)) {
        return RECORD_BUFFER_CORRUPT;
    }

    uint32_t stored_value_count;
    memcpy(&stored_value_count, encoded_record.data, sizeof(uint32_t));

    if (stored_value_count != schema.column_count) {
        return RECORD_BUFFER_CORRUPT;
    }

    record->values = calloc(schema.column_count, sizeof(value_t));
    if (record->values == NULL){
        record->value_count = 0;
        return RECORD_OUT_OF_MEMORY;
    }
    record->value_count = schema.column_count;

    size_t offset = 0;

    offset += sizeof(uint32_t);

    for (size_t i=0; i < schema.column_count; i++){
        if (schema.columns[i].column_type == RECORD_TYPE_INT){
            if (sizeof(int64_t) + offset > encoded_record.data_length){
                free_record(record);
                return RECORD_BUFFER_CORRUPT;
            }
            record->values[i].value_type = RECORD_TYPE_INT;
            memcpy(&record->values[i].data.int_value, encoded_record.data + offset, sizeof(int64_t));
            offset += sizeof(int64_t);
        }
        else{
            if (sizeof(uint32_t) + offset > encoded_record.data_length){
                free_record(record);
                return RECORD_BUFFER_CORRUPT;
            }

            record->values[i].value_type = RECORD_TYPE_TEXT;
            uint32_t text_length = 0;
            memcpy(&text_length, encoded_record.data + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            if (text_length > schema.columns[i].max_text_length){
                free_record(record);
                return RECORD_INVALID_INPUT;
            }

            if (text_length + offset > encoded_record.data_length){
                free_record(record);
                return RECORD_BUFFER_CORRUPT;
            }

            if (text_length == 0){
                record->values[i].data.text_value.data = NULL;
                record->values[i].data.text_value.length = 0;
            }

            else{
                record->values[i].data.text_value.data = malloc(text_length);
                if (record->values[i].data.text_value.data == NULL){
                    free_record(record);
                    return RECORD_OUT_OF_MEMORY;
                }

                memcpy(record->values[i].data.text_value.data, encoded_record.data + offset, text_length);
                record->values[i].data.text_value.length = text_length;
            }

            offset += text_length;
        }
    }

    if (offset != encoded_record.data_length){
                free_record(record);
                return RECORD_BUFFER_CORRUPT;
            }

    return RECORD_OK;
}

void free_encoded_record(encoded_record_t *encoded_record){
    if (encoded_record==NULL){
        return;
    }

    free(encoded_record->data);
    encoded_record->data = NULL;
    encoded_record->data_length = 0;

}

void free_record(record_t *record){
    if (record== NULL){
        return;
    }

    for(size_t i=0; i < record->value_count; i++){
        if (record->values[i].value_type==RECORD_TYPE_TEXT){
            free(record->values[i].data.text_value.data);
            record->values[i].data.text_value.data = NULL;
            record->values[i].data.text_value.length = 0;
        }
    }

    free(record->values);
    record->values=NULL;
    record->value_count= 0;
}