#include "record.h"
#include "assert.h"
#include "string.h"
#include <stdio.h>



int main() {
    encoded_record_t encoded_record;
    encoded_record.data = NULL;
    encoded_record.data_length = 90;

    free_encoded_record(&encoded_record);

    assert(encoded_record.data == NULL);
    assert(encoded_record.data_length == 0);
    return 0;
}