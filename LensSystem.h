#pragma once

struct float16
{
	union {
		float data[16];
		__m256 data8[2];
	};

	inline float16 operator+( float16 a )
	{
		float16 output;

		output.data8[0] = _mm256_add_ps( data8[0], a.data8[0] );
		output.data8[1] = _mm256_add_ps( data8[1], a.data8[1] );

		return output;
	}

	inline float16 operator*( float f )
	{
		float16 output;

		__m256 f8 = _mm256_set1_ps( f );

		output.data8[0] = _mm256_mul_ps( data8[0], f8 );
		output.data8[1] = _mm256_mul_ps( data8[1], f8 );

		return output;
	}
};

struct LensData
{
	float B, C, D, E, F; // seidel coefficients
	float focalLength;
	float entrancePupil;
	float exitPupil;
	float entrancePupilRadius;
	float exitPupilRadius;
	float principalPlaneFront;
	float principalPlaneRear;
	float s_prime;
	float dummy1, dummy2, dummy3;
	
	inline LensData operator+( LensData a )
	{
		LensData output;
		output.B = B + a.B;
		output.C = C + a.C;
		output.D = D + a.D;
		output.E = E + a.E;
		output.F = F + a.F;
		output.focalLength = focalLength + a.focalLength;
		output.entrancePupil = entrancePupil + a.entrancePupil;
		output.exitPupil = exitPupil + a.exitPupil;
		output.entrancePupilRadius = entrancePupilRadius + a.entrancePupilRadius;
		output.exitPupilRadius = exitPupilRadius + a.exitPupilRadius;
		output.principalPlaneFront = principalPlaneFront + a.principalPlaneFront;
		output.principalPlaneRear = principalPlaneRear + a.principalPlaneRear;
		output.s_prime = s_prime + a.s_prime;

		return output;
	}

	inline LensData operator*( float f )
	{
		LensData output;
		output.B = B * f;
		output.C = C * f;
		output.D = D * f;
		output.E = E * f;
		output.F = F * f;
		output.focalLength = focalLength * f;
		output.entrancePupil = entrancePupil * f;
		output.exitPupil = exitPupil * f;
		output.entrancePupilRadius = entrancePupilRadius * f;
		output.exitPupilRadius = exitPupilRadius * f;
		output.principalPlaneFront = principalPlaneFront * f;
		output.principalPlaneRear = principalPlaneRear * f;
		output.s_prime = s_prime * f;
		return output;
	}
};

class LensSystem
{
public:
	void ImportFile( string filepath );
	bool TraceRay( float2* O, float2* D, float wavelength, int lowest_element, int highest_element, bool forwards, bool useGaussianOptics, int* hit, bool registerHit );
	bool TraceRay3D( float3* O, float3* D, float wavelength, int lowest_element, int highest_element, bool forwards, bool useGaussianOptics );
	LensData GetLensData( float wavelength, float dist );
	void Precalculate( float aperture );

	int num_aperturestop = 0;
	int num_elements = 0;

	float meanFocalLength;
	float meanFstop;
	float sensorPosition;

	float FOCUS;
	float seidelFocus;

	std::vector<float> dispconstants;
	std::vector<float> radii;
	std::vector<float> centers;
	std::vector<float> apertures;
	std::vector<float> thicknesses;


	std::vector<std::string> materials;

	// Surface* screen;

	byte apertureSprite[65536]; // one channel aperture sprite, 256x256 pixels
	float spriteMultiplier = 1.0f;

private:
	float2 GetNormal( float2 O, float2 D, float2 center );
	bool IntersectRay( float2 O, float2 D, float* t, float2 center, float radius, bool useGaussianOptics );
	bool Refract( float2* D, float n1, float n2, float2 normal, bool useGaussianOptics );

	float3 GetNormal3D( float3 O, float3 D, float3 center );
	bool IntersectRay3D( float3 O, float3 D, float* t, float3 center, float radius, bool useGaussianOptics );
	bool Refract3D( float3* D, float n1, float n2, float3 normal, bool useGaussianOptics );

	float originalAperture;

	LensData lensData[LOOKUP_SIZE * LOOKUP_SIZE];
};
