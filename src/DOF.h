#pragma once

class DOF
{
  public:
	float3 sprite[65536];
	LensData meanLensData;

	void Apply( float4 *inputImage, float4 *accumulator, float* cocMap, int x, int y, LensSystem *lensSystem, float brightness, bool fillCocMap );
	float2 ApplySeidel( bool *valid, LensSystem *lensSystem, float focus_distance, float wavelength, LensData lensData, float2 Ps, float z, float2 Pprime0, float2 Pprime1, float theta, float rho );
	float2 ApplySSRT( bool *valid, LensSystem *lensSystem, float focus_distance, float wavelength, LensData lensData, float2 Ps, float z, float2 Pprime0 );
};
