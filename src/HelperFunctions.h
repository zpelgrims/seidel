#pragma once
#include "precomp.h"

struct int2
{
	int x, y;
	int2( int one, int two )
	{
		x = one;
		y = two;
	}
};

class HelperFunctions
{
  public:
	static bool compare( float4 i, float4 j )
	{
		return j.w > i.w;
	}

	static float CircleOfConfusion( float focus_distance, float dist, float focal_length )
	{
		return SENSOR_SIZE * ( focus_distance - dist ) * focal_length / ( focus_distance * ( dist - focal_length ) );
	}

	// adapted from https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/interpolation/bilinear-filtering
	static LensData bilinear( float tx, float ty, LensData c00, LensData c10, LensData c01, LensData c11 )
	{
#if 0
		LensData a = c00 * ( 1 - tx ) + c10 * tx;
		LensData b = c01 * ( 1 - tx ) + c11 * tx;
		return a * (1) - ty) + b * ty;
#else
		float tx1 = 1.0f - tx;
		float ty1 = 1.0f - ty;

		return c00 * ( tx1 * ty1 ) +
			   c10 * ( tx * ty1 ) +
			   c01 * ( tx1 * ty ) +
			   c11 * ( tx * ty );
#endif
	}

	static float2 IntersectRays( float2 O1, float2 D1, float2 O2, float2 D2 )
	{
		float dx = O2.x - O1.x;
		float dy = O2.y - O1.y;
		float det = D2.x * D1.y - D2.y * D1.x;
		float t1 = ( dy * D2.x - dx * D2.y ) / det;
		float t2 = ( dy * D1.x - dx * D1.y ) / det;

		return O1 + D1 * t1;
	}

	//
	// Gives the refractive index of a material at a specified wavelength (in micrometers)
	//
	static float CalculateRefractiveIndex( float wavelength, float *dc )
	{
		if ( dc[5] > 1 )
		{
			// Sellmeier equation
			float w2 = wavelength * wavelength;
			return sqrtf( 1.0f + ( dc[0] * w2 ) / ( w2 - dc[3] ) + ( dc[1] * w2 ) / ( w2 - dc[4] ) + ( dc[2] * w2 ) / ( w2 - dc[5] ) );
		}
		else
		{
			// Formula from the Hoya glass catalog http://www.hoya-opticalworld.com/english/technical/002.html#2_2
			float w2 = wavelength * wavelength; //um
			float w2inv = 1.0f / w2;
			float w4inv = w2inv * w2inv;
			float w6inv = w4inv * w2inv;
			float w8inv = w4inv * w4inv;
			return sqrtf( dc[0] + dc[1] * w2 + dc[2] * w2inv + dc[3] * w4inv + dc[4] * w6inv + dc[5] * w8inv );
		}
	}

	static uint float4ToUint( float4 input )
	{
		uint r = ( uint )( clamp( 256.0f * input.r, 0.0f, 255.0f ) );
		uint g = ( uint )( clamp( 256.0f * input.g, 0.0f, 255.0f ) );
		uint b = ( uint )( clamp( 256.0f * input.b, 0.0f, 255.0f ) );
		uint a = ( uint )( clamp( 256.0f * input.a, 0.0f, 255.0f ) );
		return b + ( g << 8 ) + ( r << 16 ) + ( a << 24 );
	}

	static float Luminance( float3 color )
	{
		return color.dot( float3( 0.27f, 0.67f, 0.06f ) );
	}

	// static float3 ToneMap( float3 color, __m128 gamma )
	// {
	// 	float max_white_l = 10.0f;

	// 	union {
	// 		float4 output;
	// 		__m128 output128;
	// 	};

	// 	output.rgb = color;

	// 	// reinhard
	// 	float l_old = Luminance( color );
	// 	float numerator = l_old * ( 1.0f + ( l_old / ( max_white_l * max_white_l ) ) );
	// 	float l_new = numerator / ( 1.0f + l_old );
	// 	output.rgb = color * ( l_new / l_old );

	// 	// gamma
	// 	output128 = _mm_pow_ps( output128, gamma );

	// 	return output.rgb;
	// }

};