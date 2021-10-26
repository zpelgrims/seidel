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

	static float3 ToneMap( float3 color, __m128 gamma )
	{
		float max_white_l = 10.0f;

		union {
			float4 output;
			__m128 output128;
		};

		output.rgb = color;

		// reinhard
		float l_old = Luminance( color );
		float numerator = l_old * ( 1.0f + ( l_old / ( max_white_l * max_white_l ) ) );
		float l_new = numerator / ( 1.0f + l_old );
		output.rgb = color * ( l_new / l_old );

		// gamma
		output128 = _mm_pow_ps( output128, gamma );

		return output.rgb;
	}

	
	

	// static void readImage( float4 *buffer, const char *filename, int width, int height )
	// {
	// 	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// 	fif = FreeImage_GetFileType( filename, 0 );
	// 	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( filename );
	// 	FIBITMAP *dib = FreeImage_Load( fif, filename );
	// 	uint pitch = FreeImage_GetPitch( dib );
	// 	uint bpp = FreeImage_GetBPP( dib );

	// 	if ( !dib )
	// 	{
	// 		std::cout << "ERROR: Couldn't load image." << std::endl;
	// 		return;
	// 	}

	// 	if ( FreeImage_GetWidth( dib ) != width || FreeImage_GetHeight( dib ) != height )
	// 	{
	// 		std::cout << "ERROR: input image dimensions do not match specified dimensions" << std::endl;
	// 		return;
	// 	}

	// 	if ( bpp != 128 )
	// 	{
	// 		std::cout << "ERROR: input image bytes per pixel are " << bpp << " instead of 128" << std::endl;
	// 		return;
	// 	}

	// 	for ( int y = 0; y < height; y++ )
	// 	{
	// 		const auto srcLine = FreeImage_GetScanLine( dib, height - 1 - y );
	// 		auto destLine = buffer + y * width;
	// 		memcpy( destLine, srcLine, width * sizeof( float4 ) );
	// 	}
	// }

	// static void readImage( float3 *buffer, const char *filename, int width, int height )
	// {
	// 	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// 	fif = FreeImage_GetFileType( filename, 0 );
	// 	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( filename );
	// 	FIBITMAP *dib = FreeImage_Load( fif, filename );
	// 	uint pitch = FreeImage_GetPitch( dib );
	// 	uint bpp = FreeImage_GetBPP( dib );

	// 	if ( !dib )
	// 	{
	// 		std::cout << "ERROR: Couldn't load image." << std::endl;
	// 		return;
	// 	}

	// 	if ( FreeImage_GetWidth( dib ) != width || FreeImage_GetHeight( dib ) != height )
	// 	{
	// 		std::cout << "ERROR: input image dimensions do not match specified dimensions" << std::endl;
	// 		return;
	// 	}

	// 	if ( bpp == 96 )
	// 	{
	// 		for ( int y = 0; y < height; y++ )
	// 		{
	// 			const auto srcLine = FreeImage_GetScanLine( dib, height - 1 - y );
	// 			auto destLine = buffer + y * width;
	// 			memcpy( destLine, srcLine, width * sizeof( float3 ) );
	// 		}
	// 	}
	// 	else if ( bpp == 128 )
	// 	{
	// 		float4* tempBuffer = new float4[width];

	// 		for ( int y = 0; y < height; y++ )
	// 		{
	// 			const auto srcLine = FreeImage_GetScanLine( dib, height - 1 - y );
	// 			memcpy( tempBuffer, srcLine, width * sizeof( float4 ) );

	// 			for ( int x = 0; x < width; x++ )
	// 				buffer[x + y * width] = tempBuffer[x].rgb;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		std::cout << "ERROR: input image bytes per pixel are " << bpp << " instead of 96 or 128" << std::endl;
	// 		return;
	// 	}
	// }

	// // grayscale png
	// static void readImage( byte *buffer, const char *filename, int width, int height )
	// {
	// 	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// 	fif = FreeImage_GetFileType( filename, 0 );
	// 	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( filename );
	// 	FIBITMAP *dib = FreeImage_Load( fif, filename );
	// 	uint pitch = FreeImage_GetPitch( dib );
	// 	uint bpp = FreeImage_GetBPP( dib );

	// 	if ( !dib )
	// 	{
	// 		std::cout << "ERROR: Couldn't load image." << std::endl;
	// 		return;
	// 	}

	// 	if ( FreeImage_GetWidth( dib ) != width || FreeImage_GetHeight( dib ) != height )
	// 	{
	// 		std::cout << "ERROR: input image dimensions do not match specified dimensions" << std::endl;
	// 		return;
	// 	}

	// 	if ( bpp != 8 )
	// 	{
	// 		std::cout << "ERROR: input image bytes per pixel are " << bpp << " instead of 8" << std::endl;
	// 		return;
	// 	}

	// 	for ( int y = 0; y < height; y++ )
	// 	{
	// 		const auto srcLine = FreeImage_GetScanLine( dib, height - 1 - y );
	// 		auto destLine = buffer + y * width;
	// 		memcpy( destLine, srcLine, width * sizeof( byte ) );
	// 	}
	// }

	// static bool writeImage( float4 *buffer, float multiplier, string name, bool timecode )
	// {
	// 	string filename = name;

	// 	if ( timecode )
	// 	{
	// 		time_t now = time( 0 );
	// 		tm* gmtm = gmtime( &now );
	// 		char timeChars[100];
	// 		strftime( timeChars, sizeof( timeChars ), "%Y-%m-%dT%H%M%SUTC", gmtm );
	// 		filename = filename + "_" + string( timeChars );
	// 	}

	// 	filename = filename + ".exr";

	// 	// Write accumulator to 32-bit EXR image
	// 	FIBITMAP *floatmap = FreeImage_AllocateT( FIT_RGBAF, SCRWIDTH, SCRHEIGHT, 128 );

	// 	for ( int y = 0; y < SCRHEIGHT; y++ )
	// 	{
	// 		FIRGBAF *bytes = (FIRGBAF *)FreeImage_GetScanLine( floatmap, y );
	// 		for ( int x = 0; x < SCRWIDTH; x++ )
	// 		{
	// 			float4 pixel = buffer[SCRWIDTH * ( SCRHEIGHT - y - 1 ) + x];
	// 			bytes[x].red = pixel.r * multiplier;
	// 			bytes[x].green = pixel.g * multiplier;
	// 			bytes[x].blue = pixel.b * multiplier;
	// 			bytes[x].alpha = pixel.a * multiplier;
	// 		}
	// 	}

	// 	bool saved = FreeImage_Save( FIF_EXR, floatmap, filename.c_str(), EXR_ZIP );

	// 	if ( saved )
	// 		std::cout << "Image saved succesfully as " << filename << std::endl;
	// 	else
	// 		std::cout << "ERROR: Image save failed" << std::endl;

	// 	FreeImage_Unload( floatmap );

	// 	return saved;
	// }

	// static void writeImage( float3 *buffer, float multiplier, string name, int width, int height )
	// {
	// 	// construct filename using the current timestamp
	// 	time_t now = time( 0 );
	// 	tm *gmtm = gmtime( &now );
	// 	char timeChars[100];
	// 	strftime( timeChars, sizeof( timeChars ), "%Y-%m-%dT%H%M%SUTC", gmtm );

	// 	string filename = name + "_" + string( timeChars ) + ".exr";

	// 	// Write buffer to 32-bit EXR image
	// 	FIBITMAP *floatmap = FreeImage_AllocateT( FIT_RGBAF, width, height, 128 );

	// 	for ( int y = 0; y < height; y++ )
	// 	{
	// 		FIRGBAF *bytes = (FIRGBAF *)FreeImage_GetScanLine( floatmap, y );
	// 		for ( int x = 0; x < width; x++ )
	// 		{
	// 			float3 pixel = buffer[width * ( height - y - 1 ) + x];
	// 			bytes[x].red = pixel.x * multiplier;
	// 			bytes[x].green = pixel.y * multiplier;
	// 			bytes[x].blue = pixel.z * multiplier;
	// 			bytes[x].alpha = 1;
	// 		}
	// 	}

	// 	if ( FreeImage_Save( FIF_EXR, floatmap, filename.c_str(), EXR_ZIP ) )
	// 		std::cout << "Image saved succesfully as " << filename << std::endl;
	// 	else
	// 		std::cout << "ERROR: Image save failed" << std::endl;

	// 	FreeImage_Unload( floatmap );
	// }

	// static bool writeImage( byte* buffer, string name, bool timecode, bool threeChannels = false )
	// {
	// 	string filename = name;

	// 	if ( timecode )
	// 	{
	// 		time_t now = time( 0 );
	// 		tm* gmtm = gmtime( &now );
	// 		char timeChars[100];
	// 		strftime( timeChars, sizeof( timeChars ), "%Y-%m-%dT%H%M%SUTC", gmtm );
	// 		filename = filename + "_" + string( timeChars );
	// 	}

	// 	filename = filename + ".png";

	// 	// Write accumulator to 32-bit EXR image
	// 	FIBITMAP* bitmap = FreeImage_Allocate( SCRWIDTH, SCRHEIGHT, 24 );
	// 	RGBQUAD color;

	// 	for ( int y = 0; y < SCRHEIGHT; y++ )
	// 	{
	// 		for ( int x = 0; x < SCRWIDTH; x++ )
	// 		{
	// 			if ( threeChannels )
	// 			{
	// 				color.rgbRed = buffer[3 * (y * SCRWIDTH + x) + 0];
	// 				color.rgbGreen = buffer[3 * ( y * SCRWIDTH + x ) + 1];
	// 				color.rgbBlue = buffer[3 * ( y * SCRWIDTH + x ) + 2];
	// 			}
	// 			else
	// 			{
	// 				color.rgbRed = buffer[y * SCRWIDTH + x];
	// 				color.rgbGreen = buffer[y * SCRWIDTH + x];
	// 				color.rgbBlue = buffer[y * SCRWIDTH + x];
	// 			}
	// 			FreeImage_SetPixelColor( bitmap, x, SCRHEIGHT - y - 1, &color );
	// 		}
	// 	}

	// 	bool saved = FreeImage_Save( FIF_PNG, bitmap, filename.c_str(), 0 );

	// 	if ( saved )
	// 		std::cout << "Image saved succesfully as " << filename << std::endl;
	// 	else
	// 		std::cout << "ERROR: Image save failed" << std::endl;

	// 	FreeImage_Unload( bitmap );

	// 	return saved;
	// }

	// static bool writeImage( byte* buffer, string name, bool timecode, int width, int height, bool threeChannels = false )
	// {
	// 	string filename = name;

	// 	if ( timecode )
	// 	{
	// 		time_t now = time( 0 );
	// 		tm* gmtm = gmtime( &now );
	// 		char timeChars[100];
	// 		strftime( timeChars, sizeof( timeChars ), "%Y-%m-%dT%H%M%SUTC", gmtm );
	// 		filename = filename + "_" + string( timeChars );
	// 	}

	// 	filename = filename + ".png";

	// 	// Write accumulator to 32-bit EXR image
	// 	FIBITMAP* bitmap = FreeImage_Allocate( width, height, 24 );
	// 	RGBQUAD color;

	// 	for ( int y = 0; y < height; y++ )
	// 	{
	// 		for ( int x = 0; x < width; x++ )
	// 		{
	// 			if ( threeChannels )
	// 			{
	// 				color.rgbRed = buffer[3 * ( y * width + x ) + 0];
	// 				color.rgbGreen = buffer[3 * ( y * width + x ) + 1];
	// 				color.rgbBlue = buffer[3 * ( y * width + x ) + 2];
	// 			}
	// 			else
	// 			{
	// 				color.rgbRed = buffer[y * width + x];
	// 				color.rgbGreen = buffer[y * width + x];
	// 				color.rgbBlue = buffer[y * width + x];
	// 			}
	// 			FreeImage_SetPixelColor( bitmap, x, height - y - 1, &color );
	// 		}
	// 	}

	// 	bool saved = FreeImage_Save( FIF_PNG, bitmap, filename.c_str(), 0 );

	// 	if ( saved )
	// 		std::cout << "Image saved succesfully as " << filename << std::endl;
	// 	else
	// 		std::cout << "ERROR: Image save failed" << std::endl;

	// 	FreeImage_Unload( bitmap );

	// 	return saved;
	// }

	// // https://en.wikipedia.org/wiki/SRGB
	// static float LinearToSRGB( float input )
	// {
	// 	if ( input <= 0.0031308f )
	// 		return 12.92f * input;
	// 	else
	// 		return 1.055f * powf( input, 1.0f / 2.4f ) - 0.055f;
	// }

	// // draw a circle
	// static void drawcircle( Surface *screen, int x0, int y0, int radius, int aperture, int min_x, int max_x, int color )
	// {
	// 	int x = radius;
	// 	int y = 0;
	// 	int err = 0;

	// 	while ( x >= y )
	// 	{
	// 		if ( x > min_x && x < max_x ) screen->Plot( x0 + x, ( y <= aperture ? y0 + y : +1000 ), color );
	// 		if ( y > min_x && y < max_x ) screen->Plot( x0 + y, ( x <= aperture ? y0 + x : +1000 ), color );
	// 		if ( -y > min_x && -y < max_x ) screen->Plot( x0 - y, ( x <= aperture ? y0 + x : +1000 ), color );
	// 		if ( -x > min_x && -x < max_x ) screen->Plot( x0 - x, ( y <= aperture ? y0 + y : +1000 ), color );
	// 		if ( -x > min_x && -x < max_x ) screen->Plot( x0 - x, ( y <= aperture ? y0 - y : -1000 ), color );
	// 		if ( -y > min_x && -y < max_x ) screen->Plot( x0 - y, ( x <= aperture ? y0 - x : -1000 ), color );
	// 		if ( y > min_x && y < max_x ) screen->Plot( x0 + y, ( x <= aperture ? y0 - x : -1000 ), color );
	// 		if ( x > min_x && x < max_x ) screen->Plot( x0 + x, ( y <= aperture ? y0 - y : -1000 ), color );

	// 		if ( err <= 0 )
	// 		{
	// 			y += 1;
	// 			err += 2 * y + 1;
	// 		}

	// 		if ( err > 0 )
	// 		{
	// 			x -= 1;
	// 			err -= 2 * x + 1;
	// 		}
	// 	}
	// }

	// static bool isInView( int2 xy )
	// {
	// 	return ( ( xy.x >= 0 ) && ( xy.y >= 0 ) && ( xy.x < SCRWIDTH ) && ( xy.y < SCRHEIGHT ) );
	// }

	// // return circle pixels
	// static vector<int2> getcirclepixels( Surface* screen, int x0, int y0, int radius, int aperture, int min_x, int max_x, int color )
	// {
	// 	int x = radius;
	// 	int y = 0;
	// 	int err = 0;

	// 	vector<int2> output;

	// 	int crap = 0;

	// 	while ( x >= y )
	// 	{
	// 		if ( x > min_x&& x < max_x )    isInView( int2( x0 + x, ( y <= aperture ? y0 + y : +1000 ) ) ) ? output.push_back( int2( x0 + x, ( y <= aperture ? y0 + y : +1000 ) ) ) : crap = 0;
	// 		if ( y > min_x&& y < max_x )    isInView( int2( x0 + y, ( x <= aperture ? y0 + x : +1000 ) ) ) ? output.push_back( int2( x0 + y, ( x <= aperture ? y0 + x : +1000 ) ) ) : crap = 0;
	// 		if ( -y > min_x && -y < max_x ) isInView( int2( x0 - y, ( x <= aperture ? y0 + x : +1000 ) ) ) ? output.push_back( int2( x0 - y, ( x <= aperture ? y0 + x : +1000 ) ) ) : crap = 0;
	// 		if ( -x > min_x && -x < max_x ) isInView( int2( x0 - x, ( y <= aperture ? y0 + y : +1000 ) ) ) ? output.push_back( int2( x0 - x, ( y <= aperture ? y0 + y : +1000 ) ) ) : crap = 0;
	// 		if ( -x > min_x && -x < max_x ) isInView( int2( x0 - x, ( y <= aperture ? y0 - y : -1000 ) ) ) ? output.push_back( int2( x0 - x, ( y <= aperture ? y0 - y : -1000 ) ) ) : crap = 0;
	// 		if ( -y > min_x && -y < max_x ) isInView( int2( x0 - y, ( x <= aperture ? y0 - x : -1000 ) ) ) ? output.push_back( int2( x0 - y, ( x <= aperture ? y0 - x : -1000 ) ) ) : crap = 0;
	// 		if ( y > min_x&& y < max_x )    isInView( int2( x0 + y, ( x <= aperture ? y0 - x : -1000 ) ) ) ? output.push_back( int2( x0 + y, ( x <= aperture ? y0 - x : -1000 ) ) ) : crap = 0;
	// 		if ( x > min_x&& x < max_x )    isInView( int2( x0 + x, ( y <= aperture ? y0 - y : -1000 ) ) ) ? output.push_back( int2( x0 + x, ( y <= aperture ? y0 - y : -1000 ) ) ) : crap = 0;

	// 		if ( err <= 0 )
	// 		{
	// 			y += 1;
	// 			err += 2 * y + 1;
	// 		}

	// 		if ( err > 0 )
	// 		{
	// 			x -= 1;
	// 			err -= 2 * x + 1;
	// 		}
	// 	}

	// 	return output;
	// }
};