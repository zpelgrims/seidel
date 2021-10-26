#pragma once

namespace PrimeFocusCPU
{
	class Application
	{
	public:
		// void SetTarget( Surface* surface ) { screen = surface; }
		void Init();
		// void Shutdown();
		// void Compute();

	private:
		// Surface* screen;
		float4* inputImage;
		float4* accumulator;

		vector<int> randomizedPixelOrder;
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

		bool save_frame = false;

		int framecount = 0;
		float contributions[SCRWIDTH * SCRHEIGHT];

		Timer timer;
		Timer timerRenders;
		float rendertime = 0.0f;
	};

};