#include "precomp.h"

//
// Returns lensData for a given wavelength and distance. Linearly interpolates between the closest two lensData values.
//
LensData LensSystem::GetLensData( float wavelength, float dist )
{
	float w = ( ( wavelength - 0.360f ) / 0.470f ) * ( LOOKUP_SIZE - 1 );
	int w1 = (int)w;
	int w2 = std::min( w1 + 1, LOOKUP_SIZE - 1 );
	float wpart = ( w - w1 );

	float v = std::min( LOOKUP_SIZE - 1.0f, ( dist - 0.2f ) / 15.0f * LOOKUP_SIZE );
	int v1 = (int)v;
	int v2 = std::min( v1 + 1, LOOKUP_SIZE - 1 );
	float vpart = ( v - v1 );

	LensData ld1 = lensData[w1 * LOOKUP_SIZE + v1];
	LensData ld2 = lensData[w2 * LOOKUP_SIZE + v1];
	LensData ld3 = lensData[w1 * LOOKUP_SIZE + v2];
	LensData ld4 = lensData[w2 * LOOKUP_SIZE + v2];

	return HelperFunctions::bilinear( wpart, vpart, ld1, ld2, ld3, ld4 );
}

//
// Intersect ray with lens element (circle)
//
bool LensSystem::IntersectRay( float2 O, float2 D, float* t, float2 center, float radius, bool useGaussianOptics = false )
{
	if ( useGaussianOptics )
	{
		float x = center.x + radius; // x coordinate of the lens plane
		*t = ( x - O.x ) / D.x;
		return true;
	}

	float2 C = center - O;
	float Csize2 = C.dot( C );
	float radius2 = radius * radius;

	// From inside the sphere
	if ( Csize2 <= radius2 )
	{
		float a = D.dot( D );
		float b = -2.0f * D.dot( C );
		float c = Csize2 - radius2;

		*t = ( -b + sqrtf( b * b - 4 * a * c ) ) / ( 2 * a );
		return true;
	}

	// From outside the sphere
	*t = C.dot( D );
	float2 Q = C - D * *t;
	float p2 = Q.dot( Q );
	if ( p2 > radius2 )
		return false;
	*t -= sqrtf( radius2 - p2 );
	return true;
}
bool LensSystem::IntersectRay3D( float3 O, float3 D, float* t, float3 center, float radius, bool useGaussianOptics = false )
{
	if ( useGaussianOptics )
	{
		float z = center.z + radius; // x coordinate of the lens plane
		*t = ( z - O.z ) / D.z;
		return true;
	}

	float3 C = center - O;
	float Csize2 = C.dot( C );
	float radius2 = radius * radius;

	// From inside the sphere
	if ( Csize2 <= radius2 )
	{
		float a = D.dot( D );
		float b = -2.0f * D.dot( C );
		float c = Csize2 - radius2;

		*t = ( -b + sqrtf( b * b - 4 * a * c ) ) / ( 2 * a );
		return true;
	}

	// From outside the sphere
	*t = C.dot( D );
	float3 Q = C - D * *t;
	float p2 = Q.dot( Q );
	if ( p2 > radius2 )
		return false;
	*t -= sqrtf( radius2 - p2 );
	return true;
}

//
// Returns the normal that a specified ray encounters when it hits a circle (so O must already be on the surface)
//
float2 LensSystem::GetNormal( float2 O, float2 D, float2 center )
{
	float2 diff_n = ( O - center ).normalized();
	if ( D.dot( diff_n ) > 0 ) diff_n *= -1.0f;

	return diff_n;
}
float3 LensSystem::GetNormal3D( float3 O, float3 D, float3 center )
{
	float3 diff_n = ( O - center ).normalized();
	if ( D.dot( diff_n ) > 0 ) diff_n *= -1.0f;

	return diff_n;
}

//
// Refract a ray and returns whether it was successful
//
bool LensSystem::Refract( float2* D, float n1, float n2, float2 normal, bool useGaussianOptics = false )
{
	float n1n2 = n1 / n2;
	float cosTheta = 1;
	if ( !useGaussianOptics )
		cosTheta = normal.dot( *D * -1.0f );

	float k = 1.0f - n1n2 * n1n2 * ( 1.0f - cosTheta * cosTheta );
	if ( k < 0 ) return false;

	*D = *D * n1n2 + normal * ( n1n2 * cosTheta - sqrtf( k ) );
	return true;
}
bool LensSystem::Refract3D( float3* D, float n1, float n2, float3 normal, bool useGaussianOptics = false )
{
	float n1n2 = n1 / n2;
	float cosTheta = 1;
	if ( !useGaussianOptics )
		cosTheta = normal.dot( *D * -1.0f );

	float k = 1.0f - n1n2 * n1n2 * ( 1.0f - cosTheta * cosTheta );
	if ( k < 0 ) return false;

	*D = *D * n1n2 + normal * ( n1n2 * cosTheta - sqrtf( k ) );
	return true;
}

//
// Trace ray through the lens and update origin and direction and returns whether if was succesful
// Forwards is from low -> high, or object plane -> image plane. So by default (forwards = true) we trace from object plane to image plane.
//
bool LensSystem::TraceRay( float2* O, float2* D, float wavelength, int lowest_element, int highest_element, bool forwards = true, bool useGaussianOptics = false, int* hit = NULL, bool registerHit = false )
{
	float t = 1.0f;
	bool valid = true;

	int i = forwards ? lowest_element : highest_element;
	while ( true )
	{
		if ( !( forwards ? i <= highest_element : i >= lowest_element ) ) break;

		// intersect, update rayOrigin
		if ( !IntersectRay( *O, *D, &t, float2( centers[i], 0 ), radii[i], useGaussianOptics ) )
			valid = false;
		*O += *D * t;

		// check if we pass through the aperture
		if ( abs( O->y ) > apertures[i] )
			valid = false;

		// refract ray
		int prev = forwards ? i : i + 1;
		int next = forwards ? i + 1 : i;
		float n1 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[prev * 6] );
		float n2 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[next * 6] );
		if ( !Refract( D, n1, n2, GetNormal( *O, *D, float2( centers[i], 0 ) ), useGaussianOptics ) )
			valid = false;

		if ( !valid )
			break; //early out

		forwards ? i++ : i--;
	}

	if ( registerHit && !valid ) *hit = i;

	return valid;
}
bool LensSystem::TraceRay3D( float3* O, float3* D, float wavelength, int lowest_element, int highest_element, bool forwards = true, bool useGaussianOptics = false )
{
	float t = 1.0f;
	bool valid = true;

	int i = forwards ? lowest_element : highest_element;
	while ( true )
	{
		if ( !( forwards ? i <= highest_element : i >= lowest_element ) ) break;

		// intersect, update rayOrigin
		if ( !IntersectRay3D( *O, *D, &t, float3( 0, 0, centers[i] ), radii[i], useGaussianOptics ) )
			valid = false;
		*O += *D * t;

		// check if we pass through the aperture
		if ( float2( O->x, O->y ).length() > apertures[i] )
			valid = false;

		// refract ray
		int prev = forwards ? i : i + 1;
		int next = forwards ? i + 1 : i;
		float n1 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[prev * 6] );
		float n2 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[next * 6] );
		if ( !Refract3D( D, n1, n2, GetNormal3D( *O, *D, float3( 0, 0, centers[i] ) ), useGaussianOptics ) )
			valid = false;

		if ( !valid )
			break; //early out

		forwards ? i++ : i--;
	}

	return valid;
}

//
// Precalculate lensData values for LOOKUP_SIZE different wavelength values and distances
//
void LensSystem::Precalculate( float aperture )
{
	apertures[num_aperturestop] = originalAperture * aperture;

	float step = 1.0f / ( LOOKUP_SIZE - 1 );

	meanFocalLength = 0.0f;
	meanFstop = 0.0f;
	float min_focalLength = 1E35f;
	float min_focalLength_wavelength = 0.0f;
	float max_focalLength = -1E35f;
	float max_focalLength_wavelength = 0.0f;

	//
	// for each wavelength step
	//

	for ( int i = 0; i < LOOKUP_SIZE; i++ )
	{
		float wavelength = ( i * step ) * 0.470f + 0.360f;

		//
		// entrance and exit pupils
		//

		// first construct a marginal ray
		float angle_min = 0.0f;
		float angle_max = atanf( apertures[0] / 1.0f );
		float criterium_diff = 1E-7f * angle_max;
		int steps = 0;

		// iteratively converge the min and max angle of the ray in order to find the angle between the marginal ray and the x axis
		while ( true )
		{
			float angle_step = ( angle_max - angle_min ) * 0.01f;
			float angle = angle_min;

			for ( int step = 0; step < 100; step++ )
			{
				steps++;
				angle += angle_step;

				float2 O = float2( centers[0] + radii[0] - 1.0f, 0.0f );
				float2 D = float2( cosf( angle ), sinf( angle ) );
				if ( !TraceRay( &O, &D, wavelength, 0, num_elements - 1 ) )
				{
					angle_min = angle - angle_step;
					angle_max = angle;
					break;
				}
			}

			if ( angle_max - angle_min < criterium_diff || steps > 1000 )
				break;
		}

		// identify the aperture stop by hitting it with a ray
		float2 O = float2( centers[0] + radii[0] - 1.0f, 0.0f );
		float2 D = float2( cosf( angle_max ), sinf( angle_max ) );
		int num_stop = 0;
		TraceRay( &O, &D, wavelength, 0, num_elements - 1, true, false, &num_stop, true );
		num_stop = num_aperturestop; // nvm

		// find the x coordinates of the pupils
		float entrance_pupil = 0;
		float exit_pupil = 0;
		for ( int attempt = 0; attempt < 10; attempt++ ) // multiple attempts where the ray becomes more and more close to the x-axis, in case its too far away and doesn't hit some surfaces
		{
			// entrance pupil
			float2 O1 = float2( centers[num_stop] + radii[num_stop], 0.0f );
			float2 D1 = float2( -1.0f, -0.1f / powf( 2.0f, attempt ) ).normalized();

			if ( !TraceRay( &O1, &D1, wavelength, 0, num_stop, false ) ) continue;

			// exit pupil
			float2 O2 = float2( centers[num_stop] + radii[num_stop], 0.0f );
			float2 D2 = float2( 1.0f, 0.1f / powf( 2.0f, attempt ) ).normalized();

			if ( !TraceRay( &O2, &D2, wavelength, num_stop, num_elements - 1, true ) ) continue;

			float t1 = -O1.y / D1.y;
			float t2 = -O2.y / D2.y;

			entrance_pupil = O1.x + t1 * D1.x;
			exit_pupil = O2.x + t2 * D2.x;

			break;
		}

		for ( int j = 0; j < LOOKUP_SIZE; j++ )
		{
			lensData[i * LOOKUP_SIZE + j].entrancePupil = entrance_pupil;
			lensData[i * LOOKUP_SIZE + j].exitPupil = exit_pupil;
		}

		// Find the radii by using marginal rays. Use angle_min as that is the largest angle of which we are sure that it passes through the pupils.

		// construct marginal ray for the entrance pupil
		float2 O1 = float2( centers[0] + radii[0] - 1.0f, 0.0f );
		float2 D1 = float2( cosf( angle_min ), sinf( angle_min ) );

		// construct marginal ray for the exit pupil
		float2 O2 = float2( centers[0] + radii[0] - 1.0f, 0.0f );
		float2 D2 = float2( cosf( angle_min ), sinf( angle_min ) );
		if ( !TraceRay( &O2, &D2, wavelength, 0, num_elements - 1 ) )
			std::cout << "ERROR: Couldn't trace ray to determine exit pupil radius" << std::endl;

		// find y-coordinate of marginal rays at pupil coordinates to find pupil radii
		float t1 = ( entrance_pupil - O1.x ) / D1.x;
		float t2 = ( exit_pupil - O2.x ) / D2.x;

		float entrance_pupil_radius = abs( ( O1 + D1 * t1 ).y );
		float exit_pupil_radius = abs( ( O2 + D2 * t2 ).y );

		for ( int j = 0; j < LOOKUP_SIZE; j++ )
		{
			lensData[i * LOOKUP_SIZE + j].entrancePupilRadius = entrance_pupil_radius;
			lensData[i * LOOKUP_SIZE + j].exitPupilRadius = exit_pupil_radius;
		}

		//
		// Calculate focal lengths and principal planes using matrices (Optics, page 247 - 250)
		//
		float4 A = float4( 1.0f, 0.0f, 0.0f, 1.0f ); // 2x2 matrix, identity

		for ( int k = 0; k < num_elements; k++ )
		{
			int j = k;
			int jn = k + 1;

			float n1 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[6 * j] );
			float n2 = HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[6 * jn] );

			float d = ( n2 - n1 ) / -radii[j];
			float4 R = float4( 1.0f, -d, 0.0f, 1.0f ); // 2x2 matrix, refraction matrix

			// A = R * A
			A = float4(
				R.x * A.x + R.y * A.z,
				R.x * A.y + R.y * A.w,
				R.z * A.x + R.w * A.z,
				R.z * A.y + R.w * A.w );

			if ( j == num_elements - 1 )
				break;

			float dist = ( centers[jn] + radii[jn] ) - ( centers[j] + radii[j] );
			float4 T = float4( 1.0f, 0.0f, dist / n2, 1.0f ); // 2x2 matrix, transfer matrix

			// A = T * A
			A = float4(
				T.x * A.x + T.y * A.z,
				T.x * A.y + T.y * A.w,
				T.z * A.x + T.w * A.z,
				T.z * A.y + T.w * A.w );
		}

		for ( int j = 0; j < LOOKUP_SIZE; j++ )
		{
			lensData[i * LOOKUP_SIZE + j].focalLength = -1.0f / A.y;
			lensData[i * LOOKUP_SIZE + j].principalPlaneFront = centers[0] + radii[0] + ( 1.0f - A.x ) / ( -A.y );
			lensData[i * LOOKUP_SIZE + j].principalPlaneRear = centers[num_elements - 1] + radii[num_elements - 1] + ( A.w - 1.0f ) / ( -A.y );
		}

		if ( lensData[i * LOOKUP_SIZE].focalLength < min_focalLength )
		{
			min_focalLength = lensData[i * LOOKUP_SIZE].focalLength;
			min_focalLength_wavelength = wavelength;
		}
		if ( lensData[i * LOOKUP_SIZE].focalLength > max_focalLength )
		{
			max_focalLength = lensData[i * LOOKUP_SIZE].focalLength;
			max_focalLength_wavelength = wavelength;
		}

		meanFocalLength += lensData[i * LOOKUP_SIZE].focalLength;
		meanFstop += lensData[i * LOOKUP_SIZE].focalLength / ( 2.0f * lensData[i * LOOKUP_SIZE].entrancePupilRadius );
	}

	meanFocalLength /= ( float( LOOKUP_SIZE ) );
	meanFstop /= ( float( LOOKUP_SIZE ) );
	std::cout << "min  focal length: " << ( 1000 * min_focalLength ) << "mm (at " << (int)( 1000 * min_focalLength_wavelength ) << "nm)" << std::endl;
	std::cout << "mean focal length: " << ( 1000 * meanFocalLength ) << "mm" << std::endl;
	std::cout << "max  focal length: " << ( 1000 * max_focalLength ) << "mm (at " << (int)( 1000 * max_focalLength_wavelength ) << "nm)" << std::endl;
	std::cout << "mean f-stop: f/" << meanFstop << "" << std::endl;

	std::cout << "FOV at mean focal length for an image sensor of width " << ( SENSOR_SIZE * 1000 ) << "mm: " << ( 2.0f * atanf( SENSOR_SIZE / ( 2 * meanFocalLength ) ) * 180.0f / PI ) << " degrees" << std::endl;

	//
	// Seidel coefficients
	//

	float diststep = 15.0f / LOOKUP_SIZE;
	for ( int i = 0; i < LOOKUP_SIZE; i++ )
	{
		float wavelength = ( i * step ) * 0.470f + 0.360f;
		for ( int j = 0; j < LOOKUP_SIZE; j++ )
		{
			float dist = j * diststep + 0.2f;
			Seidel::GenerateCoefficients( wavelength, num_elements, dispconstants, radii, centers, thicknesses, &lensData[i * LOOKUP_SIZE + j], dist );

		}
	}

	LensData meanLensData = GetLensData( 0.550f, FOCUS );

	std::cout << std::endl;
	std::cout << "Mean Seidel coefficients:" << std::endl;
	std::cout << "B: " << std::to_string( meanLensData.B ) << std::endl;
	std::cout << "C: " << std::to_string( meanLensData.C ) << std::endl;
	std::cout << "D: " << std::to_string( meanLensData.D ) << std::endl;
	std::cout << "E: " << std::to_string( meanLensData.E ) << std::endl;
	std::cout << "F: " << std::to_string( meanLensData.F ) << std::endl;
	std::cout << std::endl;

	//
	// Calculate the sensor distance for SSRT. Uses a weighted average (weight = luminance) over the whole spectrum and entrance pupil.
	//

	float totalWeight = 0;
	float tempSensorPosition = 0.0f;
	for ( float wavelength = 0.360f; wavelength < 0.830f; wavelength += 0.05f )
	{
		for ( float part = 0.01f; part < 1.0f; part += 0.05f )
		{
			LensData ld = GetLensData( wavelength, FOCUS );

			float2 O1s = float2( -FOCUS, 0.0f );
			float2 D1s = ( float2( ld.entrancePupil, ld.entrancePupilRadius * part ) - O1s ).normalized();

			float2 O2s = float2( -FOCUS, 0.0f );
			float2 D2s = ( float2( ld.entrancePupil, -ld.entrancePupilRadius * part ) - O2s ).normalized();

			if ( !TraceRay( &O1s, &D1s, wavelength, 0, num_elements - 1, true, false ) || !TraceRay( &O2s, &D2s, wavelength, 0, num_elements - 1, true, false ) )
				std::cout << "ERROR: Couldn't ray trace lens to obtain sensor position";
			else
			{
				float weight = HelperFunctions::Luminance( CIE1931::WavelengthRGB( wavelength ) );
				tempSensorPosition += weight * HelperFunctions::IntersectRays( O1s, D1s, O2s, D2s ).x;
				totalWeight += weight;
			}
		}
	}

	sensorPosition = tempSensorPosition / totalWeight;
	std::cout << "Sensor position: " << sensorPosition << std::endl;

	//
	// Calculate the focus distance to be used with Seidel, such that it matches the SSRT as closely as possible.
	// Using paraxial rays (as close to the axis as possible) to approximate Gaussian optics.
	//

	totalWeight = 0;
	float tempFocus = 0.0f;
	for ( float wavelength = 0.360f; wavelength < 0.830f; wavelength += 0.05f )
	{
		LensData ld = GetLensData( wavelength, FOCUS );

		float2 O1s = float2( sensorPosition, 0.0f );
		float2 D1s = ( float2( ld.exitPupil, ld.exitPupilRadius * 0.001f ) - O1s ).normalized();

		float2 O2s = float2( sensorPosition, 0.0f );
		float2 D2s = ( float2( ld.exitPupil, -ld.exitPupilRadius * 0.001f ) - O2s ).normalized();

		if ( ld.exitPupil > sensorPosition )
		{
			D1s *= -1;
			D2s *= -1;
		}

		if ( !TraceRay( &O1s, &D1s, wavelength, 0, num_elements - 1, false, false ) || !TraceRay( &O2s, &D2s, wavelength, 0, num_elements - 1, false, false ) )
			std::cout << "ERROR: Couldn't ray trace lens to obtain Seidel focus distance";
		else
		{
			float weight = HelperFunctions::Luminance( CIE1931::WavelengthRGB( wavelength ) );
			tempFocus += weight * HelperFunctions::IntersectRays( O1s, D1s, O2s, D2s ).x;
			totalWeight += weight;
		}
	}

	seidelFocus = -tempFocus / totalWeight;
	std::cout << "Focus: " << FOCUS << std::endl;
	std::cout << "Seidel focus: " << seidelFocus << std::endl;
}

//
// Import lens data from ZEMAX file and precalculate lensData values
//
void LensSystem::ImportFile( std::string filepath )
{
	std::ifstream infile( filepath );

	float scale = 1.0f;

	bool set_radius = false;
	bool set_material = false;
	bool set_aperture = false;
	bool set_thickness = false;
	float aperture = 0;
	bool ignore_element = false;

	std::string line;
	bool started = false;
	while ( std::getline( infile, line ) )
	{
		std::istringstream iss( line );

		std::vector<std::string> split( ( std::istream_iterator<std::string>( iss ) ),
			std::istream_iterator<std::string>() );

		if ( split.size() == 0 ) continue;

		std::string command = split[0];

		if ( command == "UNIT" )
		{
			if ( split[1] == "MM" )
				scale = 0.001f;
			else if ( split[1] == "CM" )
				scale = 0.01f;
			else
				std::cout << "ERROR: Couldn't set lens scale, unknown identifier " + split[1] << std::endl;
		}

		if ( !started && command == "SURF" ) started = true;
		if ( !started ) continue;

		if ( command == "TYPE" )
		{
			if ( split[1] == "EVENASPH" )
				std::cout << "ERROR: lens file contains aspherical elements, which are not supported!" << std::endl;
		}
		if ( command == "CURV" )
		{
			float curvature = std::stof( split[1] );
			if ( curvature == 0 ) curvature = 0.001f;
			radii.push_back( -scale / curvature );
			set_radius = true;
		}

		if ( command == "DISZ" )
		{
			thicknesses.push_back( scale * std::stof( split[1] ) );
			set_thickness = true;
		}

		if ( command == "GLAS" )
		{
			materials.push_back( split[1] );
			set_material = true;
		}

		if ( command == "FLAP" )
		{
			aperture = scale * std::stof( split[2] );
		}

		if ( command == "DIAM" )
		{
			if ( aperture == 0 ) aperture = scale * std::stof( split[1] );
		}

		if ( command == "STOP" )
		{
			// lens stop
			num_aperturestop = num_elements;
		}

		if ( command == "SURF" )
		{
			//num_elements++;
			if ( aperture > 1E-4f )
				num_elements++;

			apertures.push_back( aperture );
			if ( !set_radius ) radii.push_back( 1000 );
			if ( !set_material ) materials.push_back( "AIR" );
			if ( !set_thickness ) thicknesses.push_back( 0 );

			aperture = 0;
			set_radius = false;
			set_material = false;
			set_thickness = false;
		}
	}

	// make sure no thickness is too much
	for ( int i = 0; i < thicknesses.size(); i++ )
	{
		if ( thicknesses[i] > 1000 ) thicknesses[i] = 0;
	}

	// generate center coordinates
	float center = 0;
	for ( int i = 0; i < apertures.size(); i++ )
	{
		center -= radii[i];
		centers.push_back( center );
		center += radii[i] + thicknesses[i];
	}

	materials.erase( materials.begin() );
	materials.erase( materials.begin() );

	centers.erase( centers.begin() );
	centers.erase( centers.begin() );

	radii.erase( radii.begin() );
	radii.erase( radii.begin() );

	apertures.erase( apertures.begin() );
	apertures.erase( apertures.begin() );

	thicknesses.erase( thicknesses.begin() );
	thicknesses.erase( thicknesses.begin() );

	// make sure we begin with air
	if ( materials.at( 0 ) != "AIR" )
		materials.insert( materials.begin(), "AIR" );

	// materials
	for ( int i = 0; i < materials.size(); i++ )
	{
		std::vector<float> dat = Glass::GetDispersionConstants( materials[i] );
		for ( int j = 0; j < 6; j++ )
		{
			dispconstants.push_back( dat[j] );
		}
	}

	std::cout << "Imported lens containing " + std::to_string( centers.size() ) + " surfaces from file " + filepath << std::endl;

	originalAperture = apertures[num_aperturestop];

	// precalculate values
	Precalculate( APERTURE );
}