#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "LIMITS.H"
#include "FLOAT.H"

#include "CImg.h"
#include "Fast_PD.h"


using namespace cimg_library;
using namespace std;

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


typedef CV_Fast_PD::Real Real;

float probaEnergy(float proba)
{
	return -1 * log(proba);
}

float crossCorrel(CImg<float> &vect0, CImg<float> &vect1)
{
	CImg<float> newVect0(vect0);
	CImg<float> newVect1(vect1);
	newVect0 -= vect0.mean();
	newVect0 /= newVect0.norm();
	newVect1 -= vect1.mean();
	newVect1 /= newVect1.norm();
	//return newVect0.dot(newVect1);
	float correl = newVect0.dot(newVect1);
	if (correl < 0)
	{
		correl = 0;
	}
	return correl;
}

CImg<float> sobelGradientMask(const char axe)
{
	char *values;
	if (axe == 'x')
	{
		values = "-1,0,1,-2,0,2,-1,0,1";
	}
	else
	{
		values = "1,2,1,0,0,0,-1,-2,-1";
	}

	CImg<float> mask(3, 3, 1, 1, values, 0);


	return mask;
}

CImg<float> gradientMask(const char axe)
{
	char *values = "-1,0,1";
	CImg<float> mask;
	if (axe == 'x')
	{
		mask.assign(1, 3, 1, 1);
		mask.fill(values, 0);
	}
	else
	{
		mask.assign(3, 1, 1, 1);
		mask.fill(values, 0);
	}

	return mask;
}

float paramK;
float g(float _s)
{
	return exp(-_s / paramK);
}

void anisotropicSmoothing(CImg<float> &_in, int _nbIt)
{
	CImg<float> horMask = gradientMask('x');
	CImg<float> verMask = gradientMask('y');
	CImg<float> img_horGrad;
	CImg<float> img_verGrad;
	CImg<float> img_gradMagn;
	CImg<float> currentChannel;
	CImg<float> out;
	for (int v = 0; v < _in.dimv(); v++)
	{
		currentChannel = _in.get_channel(v);
		for(int i = 0; i < _nbIt; i++)
		{
			img_horGrad = currentChannel.get_convolve(horMask);
			img_verGrad = currentChannel.get_convolve(verMask);
			img_gradMagn = img_horGrad.get_sqr() + img_verGrad.get_sqr();
			img_gradMagn.sqrt();
			img_gradMagn.apply(g);
			img_horGrad.mul(img_gradMagn);
			img_verGrad.mul(img_gradMagn);
			img_horGrad.convolve(horMask);
			img_verGrad.convolve(verMask);
			currentChannel += 0.2 * (img_horGrad + img_verGrad);
		}
		out.append(currentChannel, 'v');
	}
	_in = out;
}


int main(int argc, char *argv[])
{
	char *leftImagePath = argv[1];
	char *rightImagePath = argv[2];

	CImg<float> leftImage(leftImagePath);
	CImg<float> rightImage(rightImagePath);
	int radius;
	int maxDisparity;
	char smoothing;
	int smoothIters;

	cout << "Anisotropic smoothing (y/n): ";
	cin >> smoothing;
	if (smoothing == 'y')
	{
		cout << "	Parameter: ";
		cin >> paramK;
		cout << "	Iterations: ";
		cin >> smoothIters;
		anisotropicSmoothing(leftImage, smoothIters);
		anisotropicSmoothing(rightImage, smoothIters);
		leftImage.save("temp.bmp");
	} 
	cout << "Cross correlation radius: ";
	cin >> radius;
	cout << "Maximum disparity: ";
	cin >> maxDisparity;


	int W = rightImage.dimx();
	int H = rightImage.dimy();
	int dv = rightImage.dimv();

	CImg<float> horGradientMap(rightImage);
	horGradientMap.resize(W, H, 1, 1);
	CImg<float> verGradientMap(horGradientMap);
	horGradientMap.convolve(sobelGradientMask('x'));
	horGradientMap.abs();
	horGradientMap.normalize(0, 254);
	verGradientMap.convolve(sobelGradientMask('y'));
	verGradientMap.abs();
	verGradientMap.normalize(0, 254);

	int w = W - (2 * radius);
	int h = H - (2 * radius);


	int nPoints = w * h;
	int nDisparities = maxDisparity + 1;

	Real *disparityCosts = new Real[nPoints * nDisparities];

	CImg<float> rightPatch(2 * radius + 1, 2 * radius + 1);
	CImg<float> leftPatch(2 * radius + 1, 2 * radius + 1);

	CImg<float> disparityProbas(nDisparities);
	//CImg<float> energyMapR(w, h);
	//CImg<float> energyMapG(w, h);
	//CImg<float> energyMapB(w, h);
	//CImg<float> energyMap(w, h);

	for (int y = radius; y < H - radius; y++)
	{
		for (int xR = radius; xR < W - radius; xR++)
		{
			rightPatch = rightImage.get_crop(xR - radius, y - radius, 0, 0, xR + radius, y + radius, 0, 0);
			rightPatch.append(rightImage.get_crop(xR - radius, y - radius, 0, 1, xR + radius, y + radius, 0, 1), 'x');
			rightPatch.append(rightImage.get_crop(xR - radius, y - radius, 0, 2, xR + radius, y + radius, 0, 2), 'x');

			for (int xL = xR; xL < min(xR + nDisparities, W - radius); xL++)
			{
				leftPatch = leftImage.get_crop(xL - radius, y - radius, 0, 0, xL + radius, y + radius, 0, 0);
				leftPatch.append(leftImage.get_crop(xL - radius, y - radius, 0, 1, xL + radius, y + radius, 0, 1), 'x');
				leftPatch.append(leftImage.get_crop(xL - radius, y - radius, 0, 2, xL + radius, y + radius, 0, 2), 'x');
					
				disparityProbas(xL - xR) = crossCorrel(leftPatch, rightPatch);
					
				//cout << -1 * log(correl) << endl;
				//disparityCosts[(((y - radius) * w) + xR - radius) + ((xL - xR) * nPoints)] = -1 * log(correl);

				//energyMapR = leftPatchR - rightPatchR;
				//energyMapR.sqr();
				//energyMapG = leftPatchG - rightPatchG;
				//energyMapG.sqr();
				//energyMapB = leftPatchB - rightPatchB;
			 
			}
			if (xR + nDisparities > W - radius)
			{
				for (int xL = W - radius; xL < xR + nDisparities; xL++)
				{
					disparityProbas(xL - xR) = 0;
					//disparityCosts[(((y - radius) * w) + xR - radius) + ((xL - xR) * nPoints)] = 1000000000000;
				}
			}

			float alpha = (1 - (nDisparities * FLT_EPSILON)) / disparityProbas.norm(1);
			disparityProbas *= alpha;
			disparityProbas += FLT_EPSILON;


			//disparityProbas /= disparityProbas.norm(1);
			//disparityProbas += FLT_EPSILON;

			disparityProbas.apply(probaEnergy);
			float maxCost = disparityProbas.max();
			for (int d = 0; d < nDisparities; d++)
			{
				disparityCosts[(((y - radius) * w) + (xR - radius)) + (d * nPoints)] = static_cast<Real>(INT_MAX * disparityProbas(d) / maxCost);
			}
		}
		cout << 100 * y / h << "%" << endl;
	}

	int nPairs = (2 * w * h) - w - h;
	int *pairs = new int[2 * nPairs];
	Real *pairWeights = new Real[nPairs];

	float intMax = static_cast<float>(INT_MAX);
	int i = 0;
	for (int x = radius; x < W - radius; x++)
	{
		for (int y = radius; y < H - radius; y++)
		{
			if (y < H - radius - 1)
			{
				//pairWeights[i] = static_cast<Real>(sqrt(intMax) * exp(-abs(verGradientMap(x, y))));

				pairWeights[i] = static_cast<Real>(sqrt(intMax) * (1 - (verGradientMap(x, y) / 255)));
				pairs[2 * i] = ((y - radius) * w) + x - radius;
				pairs[(2 * i) + 1] = ((y + 1 - radius) * w) + x - radius;
				i++;
			}

			if (x < W - radius - 1)
			{
				//pairWeights[i] = static_cast<Real>(sqrt(intMax) * exp(-abs(horGradientMap(x, y))));
				pairWeights[i] = static_cast<Real>(sqrt(intMax) * (1 - (horGradientMap(x, y) / 255)));
				pairs[2 * i] = ((y - radius) * w) + x - radius;
				pairs[(2 * i) + 1] = ((y - radius) * w) + x + 1 - radius;
				i++;
			}
		}
	}


	Real *disparityDists = new Real[nDisparities * nDisparities];
	for (int i = 0; i < nDisparities; i++)
	{
		for (int j = 0; j < nDisparities; j++)
		{
			disparityDists[(i * nDisparities) + j] = static_cast<Real>(sqrt(intMax) * abs(j - i) / maxDisparity);
		}
	}

	stringstream outputFileName;
	outputFileName << "Smoothing_" << smoothing;
	if (smoothing == 'y')
	{
		outputFileName << "_Smooth_param_" << paramK << "_Smooth_iters_" << smoothIters;
	}
	outputFileName << "_Max_disparity_" << maxDisparity << "_Correl_radius_" << radius << ".dat";

	FILE *outputFile = fopen(outputFileName.str().c_str(), "wb" );
	assert(outputFile);
	fwrite(&w, sizeof(int), 1, outputFile);
	fwrite(&h, sizeof(int), 1, outputFile);
	fwrite(&nPoints, sizeof(int), 1, outputFile);
	fwrite(&nPairs, sizeof(int), 1, outputFile);
	fwrite(&nDisparities, sizeof(int), 1, outputFile);
	fwrite(disparityCosts, sizeof(Real), nPoints * nDisparities, outputFile);
	fwrite(pairs, sizeof(int), 2 * nPairs, outputFile);
	fwrite(disparityDists, sizeof(Real), nDisparities * nDisparities, outputFile);
	fwrite(pairWeights, sizeof(Real), nPairs, outputFile);
	fclose(outputFile);


	delete[] disparityCosts;
	delete[] disparityDists;
	delete[] pairWeights;
	delete[] pairs;
		
	return 0;
}
