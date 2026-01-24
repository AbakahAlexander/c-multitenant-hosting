#ifndef HOSTY_DB_H
#define HOSTY_DB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------- Public Types ----------

typedef struct db db_t;

typedef enum {
    DB_OK = 0,
    DB_ERR_INVALID = -1,
    DB_ERR_IO = -2,
    DB_ERR_NOMEM = -3,
    DB_ERR_NOT_FOUND = -4,
    DB_ERR_CORRUPT = -5,
    DB_ERR_UNSUPPORTED = -6
} db_status_t;

typedef struct {
    size_t max_tenant_len;   // e.g. 32
    size_t max_key_len;      // e.g. 64
    size_t max_value_len;    // e.g. 64*1024
    size_t max_record_len;   // safety cap for WAL replay
    int fsync_on_commit;     // 1 for durability; 0 for speed during dev
} db_config_t;

// For GET results (caller-owned buffer).
typedef struct {
    uint8_t *data;
    size_t len;
} db_value_t;

// ---------- API ----------

// Open or create database at wal_path. Replays WAL into in-memory index.
db_status_t db_open(db_t **out_db, const char *wal_path, const db_config_t *cfg);

// Flush + close. Frees memory.
void db_close(db_t *db);

// Set value for (tenant, key). Durable when fsync_on_commit=1.
db_status_t db_put(db_t *db,
                   const char *tenant, const char *key,
                   const uint8_t *value, size_t value_len);

// Get value for (tenant, key). Allocates output buffer; caller must free with db_value_free.
db_status_t db_get(db_t *db,
                   const char *tenant, const char *key,
                   db_value_t *out);

// Delete key for tenant.
db_status_t db_del(db_t *db, const char *tenant, const char *key);

// Free a db_value_t returned from db_get.
void db_value_free(db_value_t *v);

// Optional: manually trigger compaction later
db_status_t db_compact(db_t *db);

// Utility: default configuration
db_config_t db_default_config(void);

#ifdef __cplusplus
}
#endif

#endif // HOSTY_DB_H