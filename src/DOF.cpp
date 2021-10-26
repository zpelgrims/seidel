#include "precomp.h"

//
// Applies DOF using a Seidel aberrations, calculates the sensor coordinates
//
float2 DOF::ApplySeidel( bool* valid, LensSystem* lensSystem, float focus_distance, float wavelength, LensData lensData, float2 Ps, float z, float2 Pprime0, float2 Pprime1, float theta, float rho )
{
	float D0 = z + lensData.entrancePupil;
	float M = -lensData.focalLength / ( z + lensData.principalPlaneFront - lensData.focalLength );
	// https://ocw.mit.edu/courses/mechanical-engineering/2-71-optics-spring-2009/video-lectures/lecture-5-thick-lenses-the-composite-lens-the-eye/MIT2_71S09_lec05.pdf
	// slide 9, M_T = - f / x_o

	float Mprime = lensData.exitPupilRadius / lensData.entrancePupilRadius;

#ifdef ENABLE_OPTICAL_VIGNETTING
	//
	// Calculate optical vignetting by checking if the ray passes through the first lens element
	//
#if 0
	float3 O = float3( Ps.x, Ps.y, -z );
	float3 D = ( float3( Pprime0.x, Pprime0.y, lensData.entrancePupil ) - O ).normalized();

	*valid = lensSystem->TraceRay3D( &O, &D, wavelength, 0, lensSystem->num_elements - 1, true, false );
	if ( !*valid ) return float2();
#else
	float element0 = lensSystem->centers[0] + lensSystem->radii[0];
	float2 dir = ( Pprime0 - Ps ) / D0; // direction vector if we move a distance of 1 on the z axis
	float2 Popening = Ps + dir * ( z + element0 );
	if ( Popening.sqrLength() > lensSystem->apertures[0] * lensSystem->apertures[0] ) // check if the ray can pass through the lens element
	{
		*valid = false;
		return float2();
	}
#endif
#endif

	//
	// Object plane coordinates
	//
	float2 P0 = Ps;

	//
	// p0, which is used to calculate the Seidel aberrations
	//
	float2 p0 = P0 / D0;

	//
	// Some preliminary calculations for the Seidel aberrations.
	// Calculate the axial and radial normalized vectors of p0.
	//
	float p0_size = p0.length();
	float2 p0_axial = p0_size != 0 ? p0 / p0_size : float2( 0, 1 );
	float2 p0_radial = float2( p0_axial.y, -p0_axial.x );

	// calculate the angle between the positive y-axis and p0_axial
	float angle = acosf( float2( 0, 1 ).dot( p0_axial ) );
	if ( p0.x < 0 ) angle = 2 * PI - angle;

	// do expensive calculations only once up front
	float sinTheta = sinf( theta - angle );
	float cosTheta = cosf( theta - angle );

	float rho2 = rho * rho;
	float rho3 = rho2 * rho;
	float rho4 = rho2 * rho2;

	float y0 = p0_size;
	float y0_2 = y0 * y0;
	float y0_3 = y0_2 * y0;

	//
	// SEIDEL ABERRATIONS
	//

	float2 delta_p0 = float2( 0, 0 ); // delta p0, so in the ('normalized') image plane, where this point would be in focus
	float phi = 0.0f;

#ifdef ENABLE_ABERRATIONS
	// spherical aberration ( B != 0 )
	delta_p0.x += lensData.B * rho3 * sinTheta;
	delta_p0.y += lensData.B * rho3 * cosTheta;
	phi += -0.25f * lensData.B * rho4;

	// coma ( F != 0 )
	delta_p0.x += -2.0f * lensData.F * y0 * rho2 * sinTheta * cosTheta;
	delta_p0.y += -lensData.F * y0 * rho2 * ( 1.0f + 2.0f * cosTheta * cosTheta );
	phi += lensData.F * y0 * rho3 * cosTheta;

	// astigmatism ( C != 0) and curvature of field ( D != 0 )
	delta_p0.x += lensData.D * rho * y0_2 * sinTheta;
	delta_p0.y += ( 2 * lensData.C + lensData.D ) * rho * y0_2 * cosTheta;
	phi += -lensData.C * y0_2 * rho2 * cosTheta * cosTheta - 0.5f * lensData.D * y0_2 * rho2;

	// distortion ( E != 0 )
	delta_p0.y += -lensData.E * y0_3;
	phi += lensData.E * y0_3 * rho * cosTheta;
#endif

	//
	// Calculate P1 (image plane coordinates)
	//
	float2 deltap = ( p0_axial * delta_p0.y + p0_radial * delta_p0.x );
	float2 p1 = p0 + deltap;

	float D1 = D0 * M * Mprime;
	float2 P1 = p1 * D0 * M;

	//
	// Calculate the sensor distance based on the average focal length of the lens and the focus distance, such that this is
	// fixed no matter the wavelength. Also calculate the focal distance, or the distance at which the lens forms an image of
	// light of the current wavelength.
	//

	float M_sensor = -meanLensData.focalLength / ( focus_distance + meanLensData.principalPlaneFront - meanLensData.focalLength );
	float Mprime_sensor = meanLensData.exitPupilRadius / meanLensData.entrancePupilRadius;
	float D1_sensor = ( focus_distance + meanLensData.entrancePupil ) * M_sensor * Mprime_sensor;
	float z_image = lensData.exitPupil - D1;
	float z_sensor = meanLensData.exitPupil - D1_sensor;

	//
	// Calculate Qstripe, the point on the Gaussian reference sphere we are going to interpolate between
	//
	float3 P1star = float3( P0.x * M, P0.y * M, z_image );
	float3 P1_3 = float3( P1.x, P1.y, z_image );
	float3 Pprime1_3 = float3( Pprime1.x, Pprime1.y, lensData.exitPupil );
	float gaussianReferenceSphereRadius = ( P1star - float3( 0, 0, lensData.exitPupil ) ).length(); // equation 5 from 5.1 in Principles of Optics, p. 231
	float3 Qstripe = P1star + ( Pprime1_3 - P1star ).normalized() * ( gaussianReferenceSphereRadius - phi );

	//
	// Calculate the coordinates on the image sensor. As P1 are the coordinates at the focal distance, we need to interpolate
	// between Q and focal distance coordinates to obtain the image sensor coordinates.
	//
	float image_to_sensor_dist = z_sensor - z_image;
	float3 ray = ( P1_3 - Qstripe );
	float3 Psensor_3 = P1_3 + ray / ray.z * image_to_sensor_dist;

	*valid = true;
	return float2( Psensor_3.x, Psensor_3.y );
}

//
// Applies DOF using a Screen Space Ray Tracing, calculates the sensor coordinates
//
float2 DOF::ApplySSRT( bool* valid, LensSystem* lensSystem, float focus_distance, float wavelength, LensData lensData, float2 Ps, float z, float2 Pprime0 )
{
	//
	// Use 3D (!) ray tracing to trace the ray from the light source to the imaging sensor
	//
	float3 O = float3( Ps.x, Ps.y, -z );
	float3 D = ( float3( Pprime0.x, Pprime0.y, lensData.entrancePupil ) - O ).normalized();

	// move the ray origin forward to reduce banding artifacts. assumes the lens starts at x = 0.
	if ( z > 0.1f )
		O += D * ( z - 0.05f );

	*valid = lensSystem->TraceRay3D( &O, &D, wavelength, 0, lensSystem->num_elements - 1, true, false );
	if ( !*valid ) return float2();

	//
	// Intersect the ray and the imaging sensor
	//

	float t = ( lensSystem->sensorPosition - O.z ) / D.z;
	float3 Psensor = O + t * D;

	return float2( Psensor.x, Psensor.y );
}

void DOF::Apply( float4* inputImage, float4* accumulator, float* cocMap, int x, int y, LensSystem* lensSystem, float brightness, bool fillCocMap )
{
#ifdef ZOOM
	if ( x > 0.625f * SCRWIDTH || x < 0.375f * SCRWIDTH || y > 0.625f * SCRHEIGHT || y < 0.375f * SCRHEIGHT ) return;
#endif

	//
	// preliminaries
	//
	float wavelength = Random::rnd() * 0.470f + 0.360f;
	float3 color_rgb = CIE1931::WavelengthXYZ( wavelength );

	if ( fillCocMap ) wavelength = 0.550f;

#if !defined ENABLE_CHROMATICS || defined UseSprite || defined UsePencilMap 
	color_rgb = float3( 1.0f, 1.0f, 1.0f );
	wavelength = 0.550f;
#endif

	float4 pixel = inputImage[y * SCRWIDTH + x];

	//
	// Camera space coordinates of the light source (P_s)
	//
#ifdef TESTING
	float2 pixelOffset = float2( 0.0f, 0.0f );
#else
	float2 pixelOffset = float2( Random::rnd() - 0.5f, Random::rnd() - 0.5f );
#endif

	float FOVsize = SENSOR_SIZE * pixel.a / ( lensSystem->sensorPosition - meanLensData.principalPlaneRear );
	float2 Ps = ( ( float2( x, y ) + pixelOffset - float2( SCRWIDTH, SCRHEIGHT ) * 0.5f ) / SCRWIDTH ) * FOVsize; // met "Basics of lens optics in all of these equations (similar triangles on both sides of the lens):" https://www.scantips.com/lights/fieldofviewmath.html

	float z = sqrtf( pixel.a * pixel.a - Ps.sqrLength() ); // distance of the light source plane

#if defined UseSimpleDOF || defined UseSprite || defined UsePencilMap || defined UseSeidelDistortion
	LensData lensData = meanLensData;
#else
	LensData lensData = lensSystem->GetLensData( wavelength, z );
#endif


	//
	// Entrance (P'_0) and exit (P'_1) pupil coordinates
	//
	float _theta = fillCocMap ? 0.0f : Random::rnd();
#if defined UseSprite || defined UsePencilMap
	float _rho = ( Random::rnd() );
	color_rgb *= 2;
#else
	float _rho = fillCocMap ? 0.5f : sqrtf( Random::rnd() );
#endif
	float theta = _theta * 2.0f * PI;
	float M_prime = lensData.exitPupilRadius / lensData.entrancePupilRadius;
	float rho = _rho * lensData.exitPupilRadius / M_prime;

	float2 Pprime1 = float2( _rho * sinf( theta ), _rho * cosf( theta ) ) * lensData.exitPupilRadius;
	float2 Pprime0 = Pprime1 / M_prime;

	//
	// Calculate the sensor plane coordinates, either by using screen space ray tracing or applying Seidel aberrations.
	//
	bool valid = true;

	float2 Psensor;

#ifdef UseSeidel
	Psensor = ApplySeidel( &valid, lensSystem, lensSystem->seidelFocus, wavelength, lensData, Ps, z, Pprime0, Pprime1, theta, rho );
#elif defined UseSSRT
	Psensor = ApplySSRT( &valid, lensSystem, lensSystem->FOCUS, wavelength, lensData, Ps, z, Pprime0 );
#endif

	Psensor /= SENSOR_SIZE; // normalize
	Psensor *= -1;			// flip the image

	if ( !valid )
		return;

#ifdef ZOOM
	Psensor *= 4;
	color_rgb *= 16;
#endif

	//
	// Pixel coordinates
	//
	int x_render = (int)( Psensor.x * SCRWIDTH + SCRWIDTH / 2 + 1.0f ) - 1;
	int y_render = (int)( Psensor.y * SCRWIDTH + SCRHEIGHT / 2 + 1.0f ) - 1;

	if ( fillCocMap )
	{
		cocMap[y * SCRWIDTH + x] = ( ( x - x_render ) * ( x - x_render ) + ( y - y_render ) * ( y - y_render ) );
		return;
	}

#ifdef USE_APERTURE_SPRITE
	color_rgb *= lensSystem->spriteMultiplier * lensSystem->apertureSprite[256 * (int)( 256 * _rho ) + (int)( 256 * _theta )] / 256.0f;
#endif

	pixel.r *= color_rgb.x;
	pixel.g *= color_rgb.y;
	pixel.b *= color_rgb.z;

	if ( x_render >= 0 && y_render >= 0 && x_render < SCRWIDTH && y_render < SCRHEIGHT )
	{
		accumulator[y_render * SCRWIDTH + x_render].rgb += pixel.rgb * brightness;
		accumulator[y_render * SCRWIDTH + x_render].a += brightness;
	}
}
