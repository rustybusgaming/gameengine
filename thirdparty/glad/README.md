# GLAD - OpenGL Loader

GLAD is an OpenGL loading library that simplifies accessing OpenGL functions.

## About

- **Version**: OpenGL 4.5 Core Profile
- **Generator**: https://glad.dav1d.de/
- **License**: MIT/Public Domain

## Setup

GLAD files need to be generated from the official OpenGL registry.

### Option 1: Use the Web Generator (Recommended)

1. Go to https://glad.dav1d.de/
2. Configure:
   - **Language**: C/C++
   - **Specification**: OpenGL
   - **API gl**: Version 4.5 or higher
   - **Profile**: Core
   - **Extensions**: Leave default or add specific extensions if needed
   - Check "Generate a loader"
3. Click **Generate**
4. Download the zip file
5. Extract contents:
   - Copy `include/glad/` → `thirdparty/glad/include/glad/`
   - Copy `include/KHR/` → `thirdparty/glad/include/KHR/`
   - Copy `src/glad.c` → `thirdparty/glad/src/glad.c`

### Option 2: Use Python Generator

```bash
pip install glad
glad --profile=core --api=gl=4.5 --generator=c --spec=gl --out-path=thirdparty/glad
```

### Option 3: Download Pre-generated Files

Pre-generated GLAD files for OpenGL 4.5 Core are available in many repositories.
A common source: https://github.com/Dav1dde/glad

## Files Structure

```
thirdparty/glad/
├── include/
│   ├── glad/
│   │   └── glad.h          # Main GLAD header
│   └── KHR/
│       └── khrplatform.h   # Khronos platform definitions
├── src/
│   └── glad.c              # GLAD implementation
└── README.md               # This file
```

## Usage in Code

```cpp
#include <glad/glad.h>
#include <SDL2/SDL.h>

// After creating OpenGL context with SDL:
SDL_GLContext context = SDL_GL_CreateContext(window);

// Load OpenGL functions:
if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    // Error: Failed to initialize GLAD
}

// Now you can use OpenGL functions:
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
```

## Integration Status

- ✅ Header structure defined
- ⏳ **Action Required**: Generate GLAD files (see options above)
- ✅ CMakeLists.txt integration ready
- ✅ Used in OpenGLDevice.cpp and OpenGLResource.cpp

## Notes

- GLAD must be initialized after creating an OpenGL context
- The loader is context-specific on some platforms
- For multi-context applications, call `gladLoadGL()` for each context
