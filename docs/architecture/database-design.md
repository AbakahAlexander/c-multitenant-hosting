
---

# `docs/database-design.md`

```md
# Database Design

## 1. Overview

Hosty uses a custom embedded key-value database.

The database is designed for:

- simple implementation in C
- durable writes
- fast point lookups
- sequential startup recovery

The storage engine consists of:

- an append-only write-ahead log (WAL)
- an in-memory hash table index

## 2. Data Model

Each value is scoped by:

- tenant
- key

Logical identifier:

```text
(tenant, key)
