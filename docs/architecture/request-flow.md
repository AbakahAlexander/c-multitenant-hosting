
---

# `docs/request-flow.md`

```md
# Request Flow

## 1. Overview

Every incoming request passes through a fixed sequence of steps so that request handling remains consistent and easy to reason about.

## 2. Request Lifecycle

```text
1. Nginx receives HTTPS request
2. Nginx forwards request to Hosty on localhost
3. Hosty accept loop accepts socket
4. Socket is pushed onto worker queue
5. Worker thread reads request
6. HTTP parser validates request line and headers
7. Tenant resolver extracts tenant from Host header
8. Router decides destination:
   - /metrics
   - /api/*
   - static file path
9. Selected subsystem handles request
10. Response is written to socket
11. Access log and metrics are updated
12. Connection is closed
