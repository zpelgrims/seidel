#pragma once

class ImageIO
{
  public:
	
	static const float* read_exr_layer(const char* input, const char* layer_name);
  static const float* read_exr_beauty(const char* input);
	static void save_to_exr(std::vector<float> img, std::string filename, unsigned xres, unsigned yres);
};
