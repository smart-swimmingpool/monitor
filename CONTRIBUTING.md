# Contributing to Pool Monitor

Thank you for your interest in contributing to the **Smart Swimming Pool Monitor** project!

This document provides guidelines for contributing. Please read it carefully
before submitting your first pull request.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Contribute](#how-to-contribute)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Pull Request Process](#pull-request-process)
- [Commit Message Guidelines](#commit-message-guidelines)
- [Quality Gates](#quality-gates)
- [Additional Resources](#additional-resources)

---

## Code of Conduct

This project adheres to the
[Contributor Covenant](https://www.contributor-covenant.org/version/1/4/code-of-conduct.html).
By participating, you are expected to uphold this code. Please report unacceptable
behavior to
[project maintainers](https://github.com/smart-swimmingpool/monitor/graphs/contributors).

See also: [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)

---

## How to Contribute

### Reporting Bugs

- **Check existing issues**: Search
  [GitHub Issues](https://github.com/smart-swimmingpool/monitor/issues)
  before creating a new one.
- **Use the issue template**: Provide detailed information about the bug.
- **Include**:
  - Firmware version (from serial monitor or Preferences)
  - Hardware setup (ESP32 board variant, E-Ink display)
  - Steps to reproduce
  - Serial monitor output (if applicable)
  - Photos of the display (if display-related)

### Suggesting Enhancements

- **Check the roadmap**: See [Readme.md](Readme.md) for planned features.
- **Discuss first**: Open a
  [GitHub Discussion](https://github.com/smart-swimmingpool/smart-swimmingpool.github.io/discussions)
  to discuss your idea.
- **Check for duplicates**: Search existing issues and PRs.

### Submitting Pull Requests

- **Fork the repository** and create your branch from `main`.
- **Use descriptive branch names** (e.g., `feat/add-temperature-history`).
- **Follow coding standards** (see below).
- **Test your changes** — ensure it builds and passes `platformio check`.
- **Update documentation** if applicable.

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/)
- [Git](https://git-scm.com/)
- Basic C++ and ESP32 development knowledge
- Understanding of MQTT and Home Assistant MQTT Discovery

### Setting Up the Development Environment

```bash
# Clone the repository
git clone https://github.com/smart-swimmingpool/monitor.git
cd monitor

# Build the firmware (automatically downloads dependencies)
pio run -e LILYGO_T5_V231

# Run static analysis
pio check --environment LILYGO_T5_V231 --skip-packages

# Flash to device
pio run -e LILYGO_T5_V231 --target upload
```

### Project Structure

```text
monitor/
├── platformio.ini           # Build configuration
├── Makefile                 # Local dev tasks (lint, build, format)
├── CPPLINT.cfg              # C++ linting config
├── src/
│   ├── main.cpp             # Arduino entry point (setup, loop)
│   └── PoolMonitor/         # Subsystem classes (namespace PoolMonitor)
│       ├── Config.hpp
│       ├── PoolMonitorContext.{hpp,cpp}
│       ├── DisplayManager.{hpp,cpp}
│       ├── NetworkManager.{hpp,cpp}
│       ├── OtaUpdater.{hpp,cpp}
│       ├── SystemMonitor.{hpp,cpp}
│       └── TimeClientHelper.{hpp,cpp}
├── docs/                    # Documentation (users, hardware, software)
├── lib/                     # External libraries (managed by PlatformIO)
├── .github/workflows/       # GitHub Actions CI
└── .editorconfig, .clang-format, etc.
```

---

## Development Workflow

Before committing changes, always run:

```bash
# Format code
make format

# Build
make build

# Run static analysis
pio check --environment LILYGO_T5_V231 --skip-packages
```

---

## Coding Standards

This project follows the same coding standards as the Pool Controller project.

### Key Guidelines

1. **Code Formatting**: Use `clang-format` with the provided `.clang-format`
2. **Line Length**: Maximum 130 characters
3. **Indentation**: 2 spaces (no tabs)
4. **Pointer Alignment**: Right (`int* ptr`)
5. **Naming**: Use descriptive names, follow existing patterns
6. **Error Handling**: Always handle errors explicitly, no silent failures
7. **Memory Management**: Stack-allocated buffers preferred (ESP32 constraints)
8. **Buffer Safety**: Always `snprintf()` instead of `sprintf()`
9. **Resource Cleanup**: Always call `preferences.end()` before deep sleep/restart

---

## Pull Request Process

1. **Fork the repository** and create your branch from `main`.
2. **Make your changes** following the coding standards.
3. **Test your changes**: `make build && pio check --environment LILYGO_T5_V231 --skip-packages`.
4. **Update documentation** if applicable (see `docs/`).
5. **Run quality checks** (see [Quality Gates](#quality-gates)).
6. **Commit** with a clear, descriptive message (see [Commit Message Guidelines](#commit-message-guidelines)).
7. **Push to your fork** and submit a pull request.

### Pull Request Requirements

- ✅ All CI checks pass (Super-Linter, PlatformIO CI)
- ✅ Code follows project standards
- ✅ Documentation is updated (if applicable)
- ✅ No breaking changes (unless discussed)
- ✅ Clear commit messages

---

## Commit Message Guidelines

### Format

```text
type(scope): subject

body

footer
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code formatting (no functional changes)
- `refactor`: Code refactoring
- `test`: Adding or fixing tests
- `chore`: Maintenance tasks

### Example

```text
feat(display): add support for new e-ink display model

- Add GxDEPG0213BN display driver
- Update display initialization code
- Add configuration options for display type

Closes #456
```

---

## Quality Gates

All contributions must pass the following quality checks:

1. **Super-Linter**: Code quality and style checks (Docker-based, via `make lint`)
2. **PlatformIO CI**: Build verification and static analysis (`pio check`)
3. **Manual Review**: Code review by maintainers

### Local Quality Checks

```bash
# Auto-format code
make format

# Run Super-Linter (requires Docker)
make lint

# Build firmware
make build

# Static analysis (no Docker needed)
pio check --environment LILYGO_T5_V231 --skip-packages
```

> **Note**: `make lint` requires Docker to run
> [Super-Linter](https://github.com/super-linter/super-linter).
> Without Docker, run `pio check` and the individual linters listed in
> [Readme.md](Readme.md#quality-checks) instead.

---

## Additional Resources

- [Pool Controller Documentation](https://github.com/smart-swimmingpool/pool-controller)
- [Smart Swimming Pool Website](https://smart-swimmingpool.com)
- [GitHub Discussions](https://github.com/smart-swimmingpool/smart-swimmingpool.github.io/discussions)
