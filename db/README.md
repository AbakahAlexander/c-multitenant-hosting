# DB module

This directory is for the database engine. The database file is our own custom
binary format, not Parquet.

The big idea:

```text
db      connects the modules together
catalog describes what the data should look like
record  converts rows to bytes
page    stores row bytes inside one page in memory
pager   moves whole pages between memory and the database file
table   uses all of those pieces to store and read rows
```

## First milestone files

`db.h`
: Public API for opening the database and calling database operations.

`db.c`
: Top-level database engine code. It will connect the other modules together.

`pager.h` / `pager.c`
: Reads and writes fixed-size pages between memory and the database file.

`page.h` / `page.c`
: Defines the layout of one page in memory: page header, row slots, and free space.

`catalog.h` / `catalog.c`
: Stores metadata about the database, tables, and columns.

`record.h` / `record.c`
: Converts typed row values into raw bytes and converts raw bytes back into row values.

`table.h` / `table.c`
: Manages table rows. It uses the catalog to understand row shape, record to encode rows, page to place row bytes, and pager to save/load pages.

## Insert flow

```text
db receives an insert
table chooses where the row should go
catalog tells table what columns the row has
record converts the row values into bytes
page writes those bytes into a page buffer in memory
pager writes the whole page buffer to the database file
```

## Select flow

```text
db receives a select
table scans the table's pages
pager reads page buffers from the database file into memory
page finds row bytes inside each page buffer
record converts row bytes back into row values
```

## Later files

`wal.h` / `wal.c`
: WAL means write-ahead log. It records changes before they reach the main database file, so the database can recover after a crash. We do not need it first because the first goal is learning pages, rows, and tables.

`btree.h` / `btree.c`
: A B-tree is an index. It makes lookups fast without scanning every row. We do not need it first because a simple table scan is enough while building the basic storage engine.
