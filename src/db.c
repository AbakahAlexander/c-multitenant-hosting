#include "db.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

// =============================
// Internal WAL format (boilerplate)
// =============================

// Magic: 'H' 'S' 'T' 'Y'
#define WAL_MAGIC 0x48535459u
#define WAL_VERSION 1u

typedef enum {
    WAL_REC_SET = 1,
    WAL_REC_DEL = 2
} wal_rec_type_t;

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;       // WAL_MAGIC
    uint8_t  version;     // WAL_VERSION
    uint8_t  type;        // wal_rec_type_t
    uint16_t tenant_len;  // bytes
    uint16_t key_len;     // bytes
    uint32_t value_len;   // bytes (0 for DEL)
    // Followed by: tenant bytes, key bytes, value bytes, then crc32 (uint32)
} wal_header_t;
#pragma pack(pop)

// NOTE: CRC32 is not implemented here; you’ll add it.
// Placeholder so record layout is obvious:
static uint32_t crc32_placeholder(const uint8_t *data, size_t len) {
    (void)data; (void)len;
    return 0;
}

// =============================
// Index stub (implement later)
// =============================

// You’ll eventually want: tenant:key -> (offset, value_len, type)
// For now, this is a stub interface so db.c compiles.

typedef struct {
    // TODO: implement real hash table
    int unused;
} kv_index_t;

typedef struct {
    uint64_t offset;      // where header starts
    uint32_t value_len;
    uint8_t  is_deleted;
} kv_index_entry_t;

static kv_index_t *index_create(void) {
    kv_index_t *idx = (kv_index_t *)calloc(1, sizeof(kv_index_t));
    return idx;
}

static void index_destroy(kv_index_t *idx) {
    free(idx);
}

// Put/Update key -> entry. Stub: does nothing.
static db_status_t index_upsert(kv_index_t *idx, const char *tenant, const char *key,
                               const kv_index_entry_t *entry) {
    (void)idx; (void)tenant; (void)key; (void)entry;
    return DB_OK;
}

// Lookup key. Stub: always not found.
static db_status_t index_lookup(kv_index_t *idx, const char *tenant, const char *key,
                                kv_index_entry_t *out_entry) {
    (void)idx; (void)tenant; (void)key; (void)out_entry;
    return DB_ERR_NOT_FOUND;
}

// Remove key. Stub: does nothing.
static db_status_t index_delete(kv_index_t *idx, const char *tenant, const char *key) {
    (void)idx; (void)tenant; (void)key;
    return DB_OK;
}

// =============================
// DB struct
// =============================

struct db {
    FILE *wal_fp;
    char *wal_path;
    db_config_t cfg;

    kv_index_t *index;

    // Concurrency (simple + safe): RW lock
    pthread_rwlock_t rwlock;
};

// =============================
// Helpers
// =============================

db_config_t db_default_config(void) {
    db_config_t c;
    c.max_tenant_len = 32;
    c.max_key_len = 64;
    c.max_value_len = 64 * 1024;
    c.max_record_len = 256 * 1024; // safety cap for replay
    c.fsync_on_commit = 1;
    return c;
}

static int valid_str_len(const char *s, size_t max_len) {
    if (!s) return 0;
    size_t n = strnlen(s, max_len + 1);
    return (n > 0 && n <= max_len);
}

static db_status_t validate_put_args(const db_t *db,
                                    const char *tenant, const char *key,
                                    const uint8_t *value, size_t value_len) {
    if (!db || !tenant || !key || (!value && value_len > 0)) return DB_ERR_INVALID;
    if (!valid_str_len(tenant, db->cfg.max_tenant_len)) return DB_ERR_INVALID;
    if (!valid_str_len(key, db->cfg.max_key_len)) return DB_ERR_INVALID;
    if (value_len > db->cfg.max_value_len) return DB_ERR_INVALID;
    return DB_OK;
}

static db_status_t validate_key_args(const db_t *db, const char *tenant, const char *key) {
    if (!db || !tenant || !key) return DB_ERR_INVALID;
    if (!valid_str_len(tenant, db->cfg.max_tenant_len)) return DB_ERR_INVALID;
    if (!valid_str_len(key, db->cfg.max_key_len)) return DB_ERR_INVALID;
    return DB_OK;
}

static db_status_t wal_fsync(FILE *fp) {
    if (!fp) return DB_ERR_IO;
    int fd = fileno(fp);
    if (fd < 0) return DB_ERR_IO;
    if (fflush(fp) != 0) return DB_ERR_IO;
    if (fsync(fd) != 0) return DB_ERR_IO;
    return DB_OK;
}

// Returns current file offset (start position for next write).
static db_status_t wal_tell(FILE *fp, uint64_t *out_off) {
    if (!fp || !out_off) return DB_ERR_INVALID;
    long pos = ftell(fp);
    if (pos < 0) return DB_ERR_IO;
    *out_off = (uint64_t)pos;
    return DB_OK;
}

// =============================
// WAL Replay (Recovery)
// =============================

static db_status_t db_replay_wal(db_t *db) {
    // You’ll implement this properly:
    // - seek to start
    // - read header
    // - sanity-check lengths
    // - read payload + crc32
    // - update index: SET upsert, DEL delete
    // - stop safely on partial/corrupt tail
    if (!db || !db->wal_fp) return DB_ERR_INVALID;

    if (fseek(db->wal_fp, 0, SEEK_SET) != 0) return DB_ERR_IO;

    // Placeholder: do nothing (empty index). Your first real task is to implement replay.
    // NOTE: After replay, seek to end for appends:
    if (fseek(db->wal_fp, 0, SEEK_END) != 0) return DB_ERR_IO;

    return DB_OK;
}

// =============================
// Public API
// =============================

db_status_t db_open(db_t **out_db, const char *wal_path, const db_config_t *cfg) {
    if (!out_db || !wal_path) return DB_ERR_INVALID;

    db_t *db = (db_t *)calloc(1, sizeof(db_t));
    if (!db) return DB_ERR_NOMEM;

    db->cfg = cfg ? *cfg : db_default_config();
    db->wal_path = strdup(wal_path);
    if (!db->wal_path) {
        free(db);
        return DB_ERR_NOMEM;
    }

    // Open WAL file for read/update, create if missing.
    db->wal_fp = fopen(wal_path, "ab+"); // append+read; creates file if not exists
    if (!db->wal_fp) {
        free(db->wal_path);
        free(db);
        return DB_ERR_IO;
    }

    // Initialize index + lock
    db->index = index_create();
    if (!db->index) {
        fclose(db->wal_fp);
        free(db->wal_path);
        free(db);
        return DB_ERR_NOMEM;
    }

    if (pthread_rwlock_init(&db->rwlock, NULL) != 0) {
        index_destroy(db->index);
        fclose(db->wal_fp);
        free(db->wal_path);
        free(db);
        return DB_ERR_IO;
    }

    // Replay WAL into index
    db_status_t st = db_replay_wal(db);
    if (st != DB_OK) {
        db_close(db);
        return st;
    }

    *out_db = db;
    return DB_OK;
}

void db_close(db_t *db) {
    if (!db) return;

    // Best-effort flush
    if (db->wal_fp) fflush(db->wal_fp);

    pthread_rwlock_destroy(&db->rwlock);

    if (db->index) index_destroy(db->index);
    if (db->wal_fp) fclose(db->wal_fp);
    free(db->wal_path);
    free(db);
}

db_status_t db_put(db_t *db,
                   const char *tenant, const char *key,
                   const uint8_t *value, size_t value_len) {
    db_status_t st = validate_put_args(db, tenant, key, value, value_len);
    if (st != DB_OK) return st;

    // Writers take write lock
    pthread_rwlock_wrlock(&db->rwlock);

    // TODO: build WAL record bytes (header + payload + crc32)
    // TODO: append to wal_fp, fsync if configured
    // TODO: update in-memory index to newest offset

    // Placeholder: pretend we appended at current end
    uint64_t off = 0;
    st = wal_tell(db->wal_fp, &off);
    if (st == DB_OK) {
        kv_index_entry_t e = { .offset = off, .value_len = (uint32_t)value_len, .is_deleted = 0 };
        st = index_upsert(db->index, tenant, key, &e);
    }

    pthread_rwlock_unlock(&db->rwlock);
    return st;
}

db_status_t db_get(db_t *db,
                   const char *tenant, const char *key,
                   db_value_t *out) {
    if (!out) return DB_ERR_INVALID;
    out->data = NULL;
    out->len = 0;

    db_status_t st = validate_key_args(db, tenant, key);
    if (st != DB_OK) return st;

    // Readers take read lock
    pthread_rwlock_rdlock(&db->rwlock);

    kv_index_entry_t e;
    st = index_lookup(db->index, tenant, key, &e);
    if (st != DB_OK) {
        pthread_rwlock_unlock(&db->rwlock);
        return st; // NOT_FOUND likely
    }

    // TODO: seek to e.offset, read record, extract value bytes
    // For now: stub returns NOT_FOUND because index is stubbed anyway.
    pthread_rwlock_unlock(&db->rwlock);
    return DB_ERR_NOT_FOUND;
}

db_status_t db_del(db_t *db, const char *tenant, const char *key) {
    db_status_t st = validate_key_args(db, tenant, key);
    if (st != DB_OK) return st;

    pthread_rwlock_wrlock(&db->rwlock);

    // TODO: append DEL record to WAL + fsync if configured
    // TODO: remove from index (or mark tombstone)
    st = index_delete(db->index, tenant, key);

    pthread_rwlock_unlock(&db->rwlock);
    return st;
}

void db_value_free(db_value_t *v) {
    if (!v) return;
    free(v->data);
    v->data = NULL;
    v->len = 0;
}

db_status_t db_compact(db_t *db) {
    (void)db;
    // TODO (Phase 2): write new WAL with only live keys, atomically swap.
    return DB_ERR_UNSUPPORTED;
}