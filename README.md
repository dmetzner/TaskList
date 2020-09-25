# TaskList

This project provides the RestAPI implementations for a simple TaskList.

Currently, following implementations exist:

- C
- GO
- NodeJS
- PHP
- Python
- Rust

A simple frontend used by all sub-projects is provided.

### Setup

After installing the dependencies for each project,
every API server can be configured and started with the `make` commands:
```
make prepare
make build
make run
```

The setting can be configured in the config.mk files.
- default port: 5000
- default SQLite database: tasks.db

### Testing

**Pytests** check the API. **GitHub Actions** automatically run them on every contribution for every server.

