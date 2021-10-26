# PrimeFocusCPU
CPU implementation of Seidel aberrations for screen-space DOF by Niels Asberg.

-----

MIT License

Copyright (c) 2021 Niels Asberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----

Parameters for the simulation are to be supplied through cmd arguments as:

  lens.zmx image.exr samplesPerFrame frameCount fileName

where image.exr must have four channels; RGB and depth.
frameCount specifies the frameCount at which the program saves the rendered image to fileName and quits the simulation.

At the top in precomp.h there are some defines to enable/disable certain parts of the simulation, as well as a few parameters to set for the simulation.


Some other useful information:
- The entrance point of the program is at Application::Init in Application.cpp.
- Seidel.h and Seidel.cpp contain the calculations of the Seidel aberrations.
- LensSystem.cpp contains the (crappy) code that imports ZMX lens designs and calculations of several lens parameters from those designs.
- DOF.cpp contains the actual ray tracing part.
