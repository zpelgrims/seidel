#pragma once

namespace PrimeFocusCPU
{
	class Application
	{
	public:
		void Init();

	private:
		float4* inputImage;
		float4* accumulator;

		std::vector<int> randomizedPixelOrder;
		float* cocMap;
		float contributionPerSample;

		LensSystem ls;
		DOF dof;

		int samplesPerFrame = 1;
		int frameCountSave = 1;
		char* outputFileName = "image";
		char* lensFileName = "";
		int totalSamplesTaken = 0;

		float exposure = 1.0f;
		float aperture = 1.0f;
		float focus = 1.0f;

		int framecount = 0;
		float contributions[SCRWIDTH * SCRHEIGHT];
	};

};