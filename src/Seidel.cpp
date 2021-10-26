#include "precomp.h"

void Seidel::GenerateCoefficients( float wavelength, int num_elements, std::vector<float> dispconstants, std::vector<float> radii, std::vector<float> centers, std::vector<float> thicknesses, LensData *lensData, float dist )
{
	std::vector<float> r;
	std::vector<float> s;
	std::vector<float> s_prime;
	std::vector<float> t;
	std::vector<float> t_prime;
	std::vector<float> n;
	std::vector<float> k;
	std::vector<float> h;

	//
	// calculate constants
	//

	n.push_back( HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[0] ) );
	for ( int i = 0; i < num_elements; i++ )
	{
		// n
		n.push_back( HelperFunctions::CalculateRefractiveIndex( wavelength, &dispconstants[( i + 1 ) * 6] ) );

		// r
		r.push_back( -radii[i] );

		// t, negative as in image 5.9
		if ( i == 0 )
			t.push_back( -( centers[0] + radii[0] - lensData->entrancePupil ) );
		else
			t.push_back( t_prime[i - 1] - thicknesses[i - 1] );

		// s, negative as in image 5.9
		if ( i == 0 )
			s.push_back( -( centers[0] + radii[0] + dist ) ); // distance from object plane to first lens element, negative
		else
			s.push_back( s_prime[i - 1] - thicknesses[i - 1] );

		// s'
		s_prime.push_back( ( r[i] * s[i] * n[i + 1] ) / ( r[i] * n[i] + s[i] * ( n[i + 1] - n[i] ) ) );

		// t'
		t_prime.push_back( ( r[i] * t[i] * n[i + 1] ) / ( r[i] * n[i] + t[i] * ( n[i + 1] - n[i] ) ) );

		// h
		if ( i == 0 )
			h.push_back( s[0] / ( t[0] - s[0] ) );
		else
			h.push_back( ( s[i] / s_prime[i - 1] ) * h[i - 1] );

		// k
		if ( i == 0 )
			k.push_back( ( t[0] * ( t[0] - s[0] ) ) / ( n[0] * s[0] ) );
		else
		{
			float d = thicknesses[i - 1];
			float delta = d / ( n[i] * h[i - 1] * h[i] );

			k.push_back( k[i - 1] + delta );
		}
	}
	
	lensData->s_prime = s_prime[num_elements - 1];

	lensData->B = 0;
	lensData->C = 0;
	lensData->D = 0;
	lensData->E = 0;
	lensData->F = 0;

	for ( int i = 0; i < num_elements; i++ )
	{
		// "Each primary aberration coefficient of a centred system is the sum of the corresponding
		// aberration coefficients associated with the individual surfaces of the system"

		std::vector<float> c = Seidel::CalculateCoefficients( r[i], n[i + 1], n[i], s[i], s_prime[i], t[i], k[i], h[i] );

		lensData->B += c[0];
		lensData->C += c[1];
		lensData->D += c[2];
		lensData->E += c[3];
		lensData->F += c[4];
	}
}