#include "precomp.h" // include (only) this in every .cpp file



#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"


const float* read_exr_layer(const char* input, const char* layer_name) {
	float* out; // width * height * RGBA
	int width;
	int height;
	const char* err = NULL; // or nullptr in C++11

	// will read `diffuse.R`, `diffuse.G`, `diffuse.B`, (`diffuse.A`) channels
	int ret = LoadEXRWithLayer(&out, &width, &height, input, layer_name, &err);

	if (ret != TINYEXR_SUCCESS) {
		if (err) {
		fprintf(stderr, "ERR : %s\n", err);
		FreeEXRErrorMessage(err); // release memory of error message.
		}
	} else {
		// return out;
		// free(out); // release memory of image data
		std::cout << "success" << std::endl;

		return out;
	}
}

const float* read_exr_beauty(const char* input) {
	float* out; // width * height * RGBA
	int width;
	int height;
	const char* err = NULL; // or nullptr in C++11

	// will read `diffuse.R`, `diffuse.G`, `diffuse.B`, (`diffuse.A`) channels
	int ret = LoadEXR(&out, &width, &height, input, &err);

	if (ret != TINYEXR_SUCCESS) {
		if (err) {
		fprintf(stderr, "ERR : %s\n", err);
		FreeEXRErrorMessage(err); // release memory of error message.
		}
	} else {
		// return out;
		// free(out); // release memory of image data
		std::cout << "success" << std::endl;

		return out;
	}
}

void save_to_exr(std::vector<float> img, std::string filename, unsigned xres, unsigned yres) {
	EXRHeader header;
	InitEXRHeader(&header);

	EXRImage image;
	InitEXRImage(&image);
	image.num_channels = 4;
	image.width = xres;
	image.height = yres;

	std::vector<float> images[4];
	images[0].resize(xres * yres);
	images[1].resize(xres * yres);
	images[2].resize(xres * yres);
	images[3].resize(xres * yres);

	for (unsigned int i = 0; i < xres * yres; i++) {
		images[0][i] = img[4*i+0];
		images[1][i] = img[4*i+1];
		images[2][i] = img[4*i+2];
		images[3][i] = img[4*i+3];
	}

	float* image_ptr[4];
	image_ptr[0] = &(images[3].at(0)); // A
	image_ptr[1] = &(images[2].at(0)); // B
	image_ptr[2] = &(images[1].at(0)); // G
	image_ptr[3] = &(images[0].at(0)); // R

	image.images = (unsigned char**)image_ptr;
	header.num_channels = 4;
	header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
	strncpy(header.channels[0].name, "A", 255); header.channels[0].name[strlen("A")] = '\0';
	strncpy(header.channels[1].name, "B", 255); header.channels[1].name[strlen("B")] = '\0';
	strncpy(header.channels[2].name, "G", 255); header.channels[2].name[strlen("G")] = '\0';
	strncpy(header.channels[3].name, "R", 255); header.channels[3].name[strlen("R")] = '\0';

	header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels); 
	header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
	for (int i = 0; i < header.num_channels; i++) {
		header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
		header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
	}
	
	header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

	const char* err;
	int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
	if (ret != TINYEXR_SUCCESS) {
		std::cout << "[LENTIL BIDIRECTIONAL TL] Error when saving exr: " << err << std::endl;
	}
}

inline int fastrand() { 
  static unsigned int g_seed = (214013*g_seed+2531011); 
  return (g_seed>>16)&0x7FFF; 
} 


// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Application::Init()
{
	// read cmd arguments, ordering:
	// lens.zmx image.exr samplesperframe framecount filename




	lensFileName = "/home/cactus/seidel/assets/lensdesigns/doublegauss.zmx";
	char* imageFileName = "/home/cactus/seidel/assets/shanghai.exr";
	samplesPerFrame = 10000;
	frameCountSave = 0;
	outputFileName = "/home/cactus/seidel/assets/shanghai_out.exr";
	focus = 0.6;


	// Random::seed = Timer::CurrentTime();
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
	// ls.screen = screen;
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
	const float* input_exr = read_exr_beauty(imageFileName);
	inputImage = new float4[SCRWIDTH * SCRHEIGHT];
	
	// ZENO: need to convert to float4 format
	for (int i=0; i<SCRWIDTH*SCRHEIGHT; i++) {
		inputImage[i].r = input_exr[(i*4)];
		inputImage[i].g = input_exr[(i*4)+1];
		inputImage[i].b = input_exr[(i*4)+2];
		inputImage[i].a = input_exr[(i*4)+3];
	}
	// HelperFunctions::readImage( inputImage, imageFileName, SCRWIDTH, SCRHEIGHT );

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





	
	bool clearAccumulator = false;
	bool recalculateLens = false;

	aperture = clamp( aperture, 0.0f, 1.0f );

	std::cout << "starting computation" << std::endl;
	
	for ( int y = 0; y < SCRHEIGHT; y++ )
	{
		for ( int x = 0; x < SCRWIDTH; x++ )
		{
			// for (int n=0; n<samplesPerFrame; n++){

				float _samples = contributions[y * SCRWIDTH + x] / contributionPerSample;
				int samples = (int)_samples; // base number of samples, floor
				if ( _samples - samples > Random::rnd() ) samples++;
				float multiplier = 1.0f / samples * ( 1.0f / ( std::min( 1.0f, _samples ) ) );

				for ( int sample = 0; sample < samples; sample++ )
					dof.Apply( inputImage, accumulator, cocMap, x, y, &ls, multiplier, false );
				totalSamplesTaken += samples;
			// }
		}
	}

	std::cout << "starting copying to buffer" << std::endl;

	__m128 gamma = _mm_set1_ps( 0.454545f );
	float multiplier = 1.0f;
	std::vector<float> img(SCRHEIGHT*SCRWIDTH*4);

	for ( int y = 0; y < SCRHEIGHT; y++ )
	{
		for ( int x = 0; x < SCRWIDTH; x++ )
		{
			float4 pixel = accumulator[y * SCRWIDTH + x];
			img[(y * SCRWIDTH + x)] = pixel.r;
			img[(y * SCRWIDTH + x)+1] = pixel.g;
			img[(y * SCRWIDTH + x)+2] = pixel.b;
			img[(y * SCRWIDTH + x)+3] = pixel.a;
// #ifdef NORMALIZE_PIXELS
// 			pixel.rgb = HelperFunctions::ToneMap( pixel.rgb * ( exposure / pixel.a ), gamma );
// #else
// 			pixel.rgb = HelperFunctions::ToneMap( pixel.rgb * exposure * multiplier, gamma );
// #endif
			
			// screen->GetBuffer()[y * SCRWIDTH + x] = HelperFunctions::float4ToUint( pixel );
		}
	}

	save_to_exr(img, outputFileName, SCRWIDTH, SCRHEIGHT);
	std::cout << "img saved" << std::endl;
}


int main () {
	Application app;
	app.Init();
}