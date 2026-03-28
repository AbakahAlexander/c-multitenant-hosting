
---

# `docs/security-deployment.md`

```md
# Security and Deployment

## 1. Deployment Model

Hosty is deployed as a single service on a Linux VM.

### Components

- Nginx listens on ports 80 and 443
- Hosty listens only on `127.0.0.1:8080`
- Hosty runs as a non-root user under `systemd`

## 2. Filesystem Layout

Suggested layout:

```text
/srv/sites/<tenant>/public/     tenant static files
/var/lib/hosty/data.wal         database WAL
/etc/hosty/hosty.conf           service configuration
/var/log/hosty/access.log       access logs
