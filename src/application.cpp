#include "precomp.h" // include (only) this in every .cpp file



inline int fastrand() { 
  static unsigned int g_seed = (214013*g_seed+2531011); 
  return (g_seed>>16)&0x7FFF; 
} 


// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Application::Init()
{

	lensFileName = "/home/cactus/seidel/assets/lensdesigns/doublegauss.zmx";
	char* imageFileName = "/home/cactus/seidel/assets/shanghai.exr";
	samplesPerFrame = 1000000;
	frameCountSave = 0;
	outputFileName = "/home/cactus/seidel/assets/shanghai_out.exr";
	focus = 0.6;


	Random::seed = fastrand();
	std::cout << "Random seed set to " << Random::seed << std::endl;

	accumulator = new float4[SCRWIDTH * SCRHEIGHT];
	for ( int x = 0; x < SCRWIDTH * SCRHEIGHT; x++ )
		accumulator[x] = float4( 0, 0, 0, 0 );

	cocMap = new float[SCRWIDTH * SCRHEIGHT];

	//
	// Import lens system and calculate Seidel coefficients
	//

	ls = LensSystem();
	ls.FOCUS = focus;
	ls.ImportFile( lensFileName );

#ifdef USE_APERTURE_SPRITE

	//
	// Read aperture sprite from file
	//
	HelperFunctions::readImage( ls.apertureSprite, "assets/bokeh_sprite_polar_256.png", 256, 256 );

	//
	// normalize aperture sprite
	//
	float totalSprite = 0.0f;
	for ( int i = 0; i < 65536; i++ )
		totalSprite += ls.apertureSprite[i] / 256.0f;
	ls.spriteMultiplier = 65536.0f / totalSprite;

#endif
	 
	//
	// Read image from file
	//
	const float* input_exr = ImageIO::read_exr_beauty(imageFileName);
	inputImage = new float4[SCRWIDTH * SCRHEIGHT];
	
	// ZENO: need to convert to float4 format
	for (int i=0; i<SCRWIDTH*SCRHEIGHT; i++) {
		inputImage[i].r = input_exr[(i*4)];
		inputImage[i].g = input_exr[(i*4)+1];
		inputImage[i].b = input_exr[(i*4)+2];
		inputImage[i].a = input_exr[(i*4)+3];
	}

	// clamp values between 0 and 1000000 in order to prevent float overflow
	for ( int x = 0; x < SCRWIDTH; x++ )
	{
		for ( int y = 0; y < SCRHEIGHT; y++ )
		{
			inputImage[y * SCRWIDTH + x].r = clamp( inputImage[y * SCRWIDTH + x].r, 0.0f, 1000000.0f );
			inputImage[y * SCRWIDTH + x].g = clamp( inputImage[y * SCRWIDTH + x].g, 0.0f, 1000000.0f );
			inputImage[y * SCRWIDTH + x].b = clamp( inputImage[y * SCRWIDTH + x].b, 0.0f, 1000000.0f );
		}
	}

	dof.meanLensData = ls.GetLensData( 0.550f, focus );

	//
	// Set some values
	//
	aperture = APERTURE;
	exposure = EXPOSURE;

	//
	// Fill cocMap
	//
#ifdef SMART_SAMPLING
	for ( int x = 0; x < SCRWIDTH; x++ )
	{
		for ( int y = 0; y < SCRHEIGHT; y++ )
		{
			dof.Apply( inputImage, accumulator, cocMap, x, y, &ls, 1.0f, true );
		}
	}
#endif

	//
	// Calculate the contribution per pixel needed
	//

	float maxContribution = 0.0f;
	float totalContribution = 0.0f;
	for ( int n = 0; n < SCRWIDTH * SCRHEIGHT; n++ )
	{
		float4 pixel = inputImage[n];
		float luminance = HelperFunctions::Luminance( pixel.rgb );

		contributions[n] = std::max( 400.0f, cocMap[n] ) * luminance;
		maxContribution = std::max( maxContribution, contributions[n] );
		totalContribution += contributions[n];
	}
	contributionPerSample = totalContribution / samplesPerFrame;




int totalframes = 100;
#pragma omp parallel for
for (int framecount=0; framecount<totalframes; framecount++) {
	
	bool clearAccumulator = false;
	bool recalculateLens = false;

	aperture = clamp( aperture, 0.0f, 1.0f );
	
	for ( int y = 0; y < SCRHEIGHT; y++ )
	{
		for ( int x = 0; x < SCRWIDTH; x++ )
		{

				float _samples = contributions[y * SCRWIDTH + x] / contributionPerSample;
				int samples = (int)_samples; // base number of samples, floor
				if ( _samples - samples > Random::rnd() ) samples++;
				float multiplier = 1.0f / samples * ( 1.0f / ( std::min( 1.0f, _samples ) ) );

				for ( int sample = 0; sample < samples; sample++ )
					dof.Apply( inputImage, accumulator, cocMap, x, y, &ls, multiplier, false );
				totalSamplesTaken += samples;
			}
		}

}
	

	std::cout << "starting copying to buffer" << std::endl;

	__m128 gamma = _mm_set1_ps( 0.454545f );
	float multiplier = 1.0f/totalframes;
	std::vector<float> img(SCRHEIGHT*SCRWIDTH*4);

	for ( int y = 0; y < SCRHEIGHT; y++ )
	{
		for ( int x = 0; x < SCRWIDTH; x++ )
		{
			float4 pixel = accumulator[y * SCRWIDTH + x] * exposure * multiplier;
			img[((y * SCRWIDTH + x)*4)] = pixel.r;
			img[((y * SCRWIDTH + x)*4)+1] = pixel.g;
			img[((y * SCRWIDTH + x)*4)+2] = pixel.b;
			img[((y * SCRWIDTH + x)*4)+3] = pixel.a;			
		}
	}

	ImageIO::save_to_exr(img, outputFileName, SCRWIDTH, SCRHEIGHT);
	std::cout << "img saved" << std::endl;
}


int main () {
	Application app;
	app.Init();
}