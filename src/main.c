#include "db.h"
#include <stdio.h>

int main(void) {
    db_t *db = NULL;
    db_config_t cfg = db_default_config();
    cfg.fsync_on_commit = 0; // dev mode

    db_status_t st = db_open(&db, "data.wal", &cfg);
    if (st != DB_OK) {
        fprintf(stderr, "db_open failed: %d\n", st);
        return 1;
    }

    printf("DB opened successfully.\n");
    db_close(db);
    printf("DB closed successfully.\n");
    return 0;
}