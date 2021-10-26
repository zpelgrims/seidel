// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).



#define UseSeidel    // Seidel abberations
// #define UseSSRT      // Screen Space Ray Tracing

#define ENABLE_ABERRATIONS // Comment to set all Seidel coefficients to 0
#define ENABLE_OPTICAL_VIGNETTING
#define ENABLE_CHROMATICS
//#define USE_APERTURE_SPRITE

#define LOOKUP_SIZE 64
#define SENSOR_SIZE 0.015f
#define APERTURE 0.5f
#define EXPOSURE 1.0f

//
// Template code below
//

// Prevent expansion clashes (when using std::min and std::max):
#define NOMINMAX

#define SCRWIDTH 1280
#define SCRHEIGHT 720

// #define WINDOW_NAME "Prime Focus CPU"

// // Glew should be included first
// #include <GL/glew.h>
// // Comment for autoformatters: prevent reordering these two.
// #include <GL/gl.h>

// #ifdef _WIN32
// // Followed by the Windows header
// #include <Windows.h>

// // Then import wglext: This library tries to include the Windows
// // header WIN32_LEAN_AND_MEAN, unless it was already imported.
// #include <GL/wglext.h>

// // Extra definitions for redirectIO
// #include <fcntl.h>
// #include <io.h>
// #endif

// // External dependencies:
// #include <FreeImage.h>
// #include <SDL2/SDL.h>

// C++ headers
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <string>
#include <iterator>
#include <map>
#include <sstream>

// Namespaced C headers:
#include <cstring>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// #include <windows.h>


// Header for AVX, and every technology before it.
// If your CPU does not support this, include the appropriate header instead.
// See: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>

// clang-format off

// "Leak" common namespaces to all compilation units. This is not standard
// C++ practice but a mere simplification for this small project.
// using namespace std;

// #include "surface.h"
#include "template.h"

using namespace PrimeFocusCPU;

#include "Random.h"
#include "Glass.h"
#include "CIE1931.h"
#include "HelperFunctions.h"
#include "Timer.h"

#include "LensSystem.h"
#include "DOF.h"
#include "Seidel.h"
#include "application.h"

// clang-format on
