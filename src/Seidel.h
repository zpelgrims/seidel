#pragma once

class Seidel
{
  private:
	static float pow2( float f ) { return f * f; }

  public:
	static std::vector<float> CalculateCoefficients( float r, float n, float n_prev, float s, float s_prime, float t, float k, float h )
	{
		// Equation 24 from page 225 from Principles of Optics.
		// We set b_i = 0, as we assume all surfaces to be spherical.

		float K = n_prev * ( 1.0f / r - 1.0f / s );
		//float K = n * ( 1.0f / r - 1.0f / s_prime ); // alternative

		float Ai = ( 1.0f / ( n * s_prime ) - 1.0f / ( n_prev * s ) );
		float Bi = ( 1.0f / pow2( n ) - 1.0f / pow2( n_prev ) ); 
		float h2 = h * h;
		float h4 = h2 * h2;

		float Ci = h2 * k * K;

		std::vector<float> output;
		output.reserve( 5 );
		output.push_back( 0.5f * ( h4 * pow2( K ) * Ai ) );													 // B
		output.push_back( 0.5f * ( pow2( 1.0f + Ci ) * Ai ) );												 // C
		output.push_back( 0.5f * ( Ci * ( 2.0f + Ci ) * Ai - K * Bi ) );									 // D
		output.push_back( 0.5f * ( k * ( 1.0f + Ci ) * ( 2.0f + Ci ) * Ai - ( ( 1.0f + Ci ) / h2 ) * Bi ) ); // E
		output.push_back( 0.5f * ( h2 * K * ( 1.0f + Ci ) * Ai ) );											 // F

		return output;
	}

	Seidel()
	{
	}

	static void GenerateCoefficients( float wavelength, int num_elements, std::vector<float> dispconstants, std::vector<float> radii, std::vector<float> centers, std::vector<float> thicknesses, LensData *lensData, float dist );
};