# Hosty Architecture Overview

## Overview

Hosty is a multi-tenant static website hosting platform written in C. It allows multiple tenants to host static websites under subdomains and provides a lightweight authenticated API backed by a custom persistent key-value database.

The system is intentionally infrastructure-focused and consists of:

- an HTTP server
- a virtual host resolver
- a secure static file server
- an API layer
- authentication and abuse protection
- a custom write-ahead-log-based storage engine

## High-Level Stack

```text
Browser
  ↓ HTTPS
Nginx
  ↓ HTTP on localhost
Hosty (C server)
  ├─ HTTP parser
  ├─ Tenant resolver
  ├─ Request router
  ├─ Static file server
  ├─ API layer
  ├─ Auth / rate limiting / quotas
  └─ Thread pool
       ↓
Custom key-value database
  ├─ WAL
  └─ In-memory index
  Goals

support multi-tenant static hosting

isolate tenants by hostname and filesystem root

provide a simple API for frontend JavaScript

use a custom persistent database

keep deployment simple on a single Linux VM

Non-Goals

server-side scripting

SQL or relational queries

arbitrary user code execution

billing or dashboard UI

distributed replication
