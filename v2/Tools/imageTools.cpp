#include "imageTools.h"


anisotropicSmoothing::funcG::funcG(float K)
{
	paramK = K;
}
float anisotropicSmoothing::funcG::operator()(float x)
{
	return std::exp(-x / paramK);
}

anisotropicSmoothing::anisotropicSmoothing(CImg<float> &input, int nIter, float K) : CImg<float>()
{
	g = funcG(K);
	CImg<float> horMask(1, 3, 1, 1, "-1,0,1", 0);
	CImg<float> verMask(3, 1, 1, 1, "-1,0,1", 0);
	CImg<float> horGradMap;
	CImg<float> verGradMap;
	CImg<float> gradMagnMap;
	CImg<float> currentChannel;
	CImg<float> output;

	for (int v = 0; v < input.dimv(); v++)
	{
		currentChannel = input.get_channel(v);
		for(int i = 0; i < nIter; i++)
		{
			horGradMap = currentChannel.get_convolve(horMask);
			verGradMap = currentChannel.get_convolve(verMask);

			gradMagnMap = horGradMap.get_sqr() + verGradMap.get_sqr();
			gradMagnMap.sqrt();
			gradMagnMap.apply(g);

			horGradMap.mul(gradMagnMap);
			verGradMap.mul(gradMagnMap);

			horGradMap.convolve(horMask);
			verGradMap.convolve(verMask);

			currentChannel += 0.2 * (horGradMap + verGradMap);
		}
		append(currentChannel, 'v');
	}
}



CImg<float> sobelFilter(CImg<float> &input, char dir)
{
	CImg<float> mask(3, 3, 1, 1);
	if (dir == 'x')
	{
		mask.fill(-1, 0, 1, -2, 0, 2, -1, 0, 1);
	}
	else
	{
		mask.fill(1, 2, 1, 0, 0, 0, -1, -2, -1);
	}
	CImg<float> output = input.get_resize(input.dimx(), input.dimy(), 1, 1).get_convolve(mask);
	//output.abs();
	//output.normalize(0, 255);
	return output;
}


