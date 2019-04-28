#ifndef DEF_IMAGETOOLS
#define DEF_IMAGETOOLS

#include <CImg.h>

using namespace cimg_library;


class anisotropicSmoothing : public CImg<float>
{
private:
	class funcG
	{
	private:
		float paramK;
	public:
		funcG(float K = 1);
		float operator()(float x);
	};
	funcG g;

public:
	anisotropicSmoothing(CImg<float> &input, int nIter, float K);
};

CImg<float> sobelFilter(CImg<float> &input, char dir);

#endif
