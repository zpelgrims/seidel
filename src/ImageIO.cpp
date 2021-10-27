#include "precomp.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"


const float* ImageIO::read_exr_layer(const char* input, const char* layer_name) {
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

		return out;
	}
}

const float* ImageIO::read_exr_beauty(const char* input) {
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

		return out;
	}
}

void ImageIO::save_to_exr(std::vector<float> img, std::string filename, unsigned xres, unsigned yres) {
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