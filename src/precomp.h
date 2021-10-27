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


// Prevent expansion clashes (when using std::min and std::max):
#define NOMINMAX

#define SCRWIDTH 1280
#define SCRHEIGHT 720


// C++ headers
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <iterator>
#include <map>
#include <sstream>
#include <cstring>


// Header for AVX, and every technology before it.
// If your CPU does not support this, include the appropriate header instead.
// See: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>



#include "template.h"

using namespace PrimeFocusCPU;

#include "Random.h"
#include "Glass.h"
#include "CIE1931.h"
#include "HelperFunctions.h"
#include "LensSystem.h"
#include "DOF.h"
#include "Seidel.h"
#include "application.h"
#include "ImageIO.h"