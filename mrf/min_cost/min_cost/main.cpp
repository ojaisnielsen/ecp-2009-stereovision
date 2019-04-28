#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "Fast_PD.h"
#include "CImg.h"

using namespace cimg_library;
using namespace std;

typedef CV_Fast_PD::Real Real;

int main(int argc, char *argv[])
{
	char *inputFileName = argv[1];
	FILE *inputFile = fopen(inputFileName, "rb");
	assert(inputFile);

	int w, h;
	int nPoints; 
	int nDisparities; 
	int nPairs;
	int maxIters;
	cout << "Maximum number of iterations: ";
	cin >> maxIters;

	fread(&w, sizeof(int), 1, inputFile);
	fread(&h, sizeof(int), 1, inputFile);
	fread(&nPoints, sizeof(int), 1, inputFile);
	fread(&nPairs , sizeof(int), 1, inputFile);
	fread(&nDisparities, sizeof(int), 1, inputFile);

	Real *disparityCosts = new Real[nPoints * nDisparities];
	int *pairs  = new int [nPairs * 2];
	Real *disparityDists = new Real[nDisparities * nDisparities];
	Real *pairWeights = new Real[nPairs];
	fread(disparityCosts, sizeof(Real), nPoints * nDisparities, inputFile);
	fread(pairs , sizeof(int ), nPairs * 2 , inputFile);
	fread(disparityDists , sizeof(Real), nDisparities * nDisparities, inputFile);
	fread(pairWeights, sizeof(Real), nPairs, inputFile);	

	fclose(inputFile);

	CV_Fast_PD pd(nPoints, nDisparities, disparityCosts, nPairs, pairs, disparityDists, maxIters, pairWeights);
	pd.run();

	delete[] disparityCosts;
	delete[] disparityDists;
	delete[] pairWeights;
	delete[] pairs;

	CImg<float> disparities(w, h);

	int x, y;
	for (int i = 0; i < nPoints; i++)
	{
		y = (i / w);
		x = i - (w * y);
		disparities(x, y) = pd._pinfo[i].label;
	}

	disparities.normalize(0, 254);
	stringstream outputFileName;
	outputFileName << inputFileName << ".bmp";
	disparities.save(outputFileName.str().c_str());
		
	return 0;
}