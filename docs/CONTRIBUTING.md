# Contributing to Nexus Game Engine

Thank you for your interest in contributing to Nexus! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Coding Standards](#coding-standards)
- [Submitting Changes](#submitting-changes)
- [Reporting Bugs](#reporting-bugs)
- [Feature Requests](#feature-requests)

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Accept constructive criticism
- Focus on what's best for the project
- Show empathy towards other contributors

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- Visual Studio 2022 with C++ Desktop Development
- CMake 3.16 or higher
- Git for version control
- Basic understanding of C++17/20

### Setting Up Development Environment

1. **Fork the repository**
   ```bash
   git clone https://github.com/yourusername/nexus-engine.git
   cd nexus-engine
   ```

2. **Install dependencies**
   ```bash
   cd scripts/setup
   ./setup_dependencies.bat
   ```

3. **Build the project**
   ```bash
   cd scripts/build
   ./build.bat
   ```

4. **Run tests**
   ```bash
   ./build_check.bat
   ```

## Development Process

### Branching Strategy

- `main` - Stable production-ready code
- `develop` - Integration branch for features
- `feature/*` - New features
- `bugfix/*` - Bug fixes
- `hotfix/*` - Critical production fixes

### Workflow

1. Create a feature branch from `develop`
2. Make your changes with clear commits
3. Write/update tests as needed
4. Update documentation
5. Submit a pull request

## Coding Standards

### C++ Style Guide

We follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) with some project-specific conventions:

#### File Naming
- Headers: `PascalCase.h` (e.g., `GraphicsDevice.h`)
- Source: `PascalCase.cpp` (e.g., `GraphicsDevice.cpp`)
- Test files: `*_test.cpp`

#### Code Formatting
We use `.clang-format` for automatic formatting:
```bash
clang-format -i src/**/*.cpp include/**/*.h
```

#### Naming Conventions

**Classes and Structs**:
```cpp
class GraphicsDevice {
    // PascalCase for classes
};
```

**Functions and Methods**:
```cpp
void Initialize();  // PascalCase for public methods
void updateInternal();  // camelCase for private methods
```

**Variables**:
```cpp
int frameCount;  // camelCase
const int MAX_TEXTURES = 256;  // SCREAMING_SNAKE_CASE for constants
```

**Namespaces**:
```cpp
namespace Nexus {
    namespace Graphics {
        // ...
    }
}
```

### Documentation Standards

#### Header Documentation
```cpp
/**
 * @brief Manages the graphics rendering pipeline
 * 
 * The GraphicsDevice handles initialization of DirectX,
 * resource management, and rendering operations.
 * 
 * @note This class is not thread-safe
 */
class GraphicsDevice {
public:
    /**
     * @brief Initializes the graphics device
     * @param window Handle to the application window
     * @return true if initialization succeeded
     */
    bool Initialize(HWND window);
};
```

#### Implementation Comments
- Use `//` for single-line comments
- Explain **why**, not **what**
- Document non-obvious behavior
- Reference related code or tickets

```cpp
// Use triple buffering to prevent screen tearing
// while maintaining low latency
swapChain->Present(1, 0);
```

### Error Handling

**Use exceptions for exceptional situations**:
```cpp
if (!device) {
    throw std::runtime_error("Failed to create D3D device");
}
```

**Use return codes for expected failures**:
```cpp
bool LoadTexture(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    // ...
    return true;
}
```

### Memory Management

- Prefer RAII and smart pointers
- Use `std::unique_ptr` for ownership
- Use `std::shared_ptr` when sharing is necessary
- Raw pointers only for non-owning references

```cpp
std::unique_ptr<GraphicsDevice> device = std::make_unique<GraphicsDevice>();
```

## Submitting Changes

### Pull Request Process

1. **Update Documentation**
   - Update README.md if adding features
   - Update relevant docs in `docs/`
   - Add inline code documentation

2. **Add Tests**
   - Unit tests for new functionality
   - Integration tests for system changes
   - Performance tests for optimization work

3. **PR Description Template**
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Breaking change
   - [ ] Documentation update
   
   ## Testing
   - [ ] Unit tests pass
   - [ ] Manual testing completed
   - [ ] Performance benchmarks run
   
   ## Checklist
   - [ ] Code follows project style
   - [ ] Self-review completed
   - [ ] Documentation updated
   - [ ] No new warnings
   ```

4. **Review Process**
   - At least one approval required
   - CI/CD checks must pass
   - Address review comments
   - Squash commits before merge

## Reporting Bugs

### Bug Report Template

```markdown
**Describe the bug**
Clear description of the issue

**To Reproduce**
Steps to reproduce:
1. ...
2. ...

**Expected behavior**
What should happen

**Actual behavior**
What actually happens

**Environment**
- OS: Windows 11
- Build: Release/Debug
- GPU: [e.g., NVIDIA RTX 3080]

**Additional context**
- Error logs
- Screenshots
- Crash dumps
```

### Where to Report
- GitHub Issues for bugs
- Security issues: email directly (don't create public issue)

## Feature Requests

### Proposal Template

```markdown
**Feature Description**
Clear description of the feature

**Use Case**
Why is this needed?

**Proposed Solution**
How should it work?

**Alternatives Considered**
Other approaches

**Additional Context**
- Examples from other engines
- Reference materials
- Mock-ups or diagrams
```

## Testing Guidelines

### Unit Tests
- Place in `tests/unit/`
- One test file per source file
- Use descriptive test names

```cpp
TEST(GraphicsDevice, InitializationSucceeds) {
    GraphicsDevice device;
    ASSERT_TRUE(device.Initialize(hwnd));
}
```

### Integration Tests
- Place in `tests/integration/`
- Test component interactions
- Mock external dependencies

### Performance Tests
- Place in `tests/performance/`
- Benchmark critical paths
- Compare against baseline

## Documentation

### Types of Documentation

1. **API Documentation**: In-code comments (Doxygen format)
2. **User Guides**: `docs/guides/`
3. **Architecture**: `docs/ARCHITECTURE.md`
4. **Examples**: `examples/`

### Building Documentation

```bash
# Generate API docs with Doxygen
doxygen Doxyfile
```

## Community

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **Discussions**: General questions and ideas
- **Discord**: Real-time chat (link in README)

### Getting Help

- Check existing documentation
- Search closed issues
- Ask in discussions
- Join Discord for quick questions

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Credited in release notes
- Acknowledged in documentation

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to Nexus Game Engine! 🚀
