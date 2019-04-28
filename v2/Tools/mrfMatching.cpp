#include "mrfMatching.h"
#include "../core/ViewPair.h"
#include "../core/Stereo.h"

float round(float x)
{
	return floor(x + 0.5);
}

MrfMatching::MrfMatching(ViewPair *viewPair, int radius, int maxDisparity, int step, int nParts)
{
	m_initialOrder = true;
	m_viewPair = viewPair;
	m_leftId = 0;
	m_rightId = 1;
	m_radius = radius;
	m_nDisparities = 1;//maxDisparity + 1;
	m_nParts = nParts;
	m_leftParts.assign(nParts);
	m_rightParts.assign(nParts);
	m_outputParts.assign(nParts);
	m_maxIters = 100;
	m_step = step;
}

void MrfMatching::includeKnownMatches(CImgList<float> &pairs)
{

	int xCr = 0;//(int) round((m_viewPair->getRectView(m_rightId)->getK())(2, 0));
	int xCl = 0;//(int) round((m_viewPair->getRectView(m_leftId)->getK())(2, 0));
	saveMatrix(CImg<float>(2, 1, 1, 1, xCr, xCl));

	int ptsInInitialOrder = 0;
	for (int i = 0; i < pairs.size; i += 2)
	{
		if (pairs[i](0, 0) - xCl > pairs[i + 1](0, 0) - xCr)
		{
			ptsInInitialOrder++;
		}
	}
	m_initialOrder = ptsInInitialOrder >= pairs.size / 4;
	m_leftId = 1 - (int) (m_initialOrder);
	m_rightId = (int) (m_initialOrder);

	int W = m_viewPair->getRectView(m_rightId)->dimx();
	int H = m_viewPair->getRectView(m_rightId)->dimy();

	m_knownPointsMap.assign(W, H, 1, 2, 0);
	m_knownPointsProbas.assign(W * H);

	int outRangeDisp;
	do 
	{
		outRangeDisp = 0;
		m_knownPointsMap.fill(0);
		m_knownPointsProbas = CImgList<float>(W * H);
		for (int i = 0; i < pairs.size; i += 2)
		{
			int Xr = (int) round(pairs[i + 1](0, 0));
			int Xl = (int) round(pairs[i](0, 0));
			int Yr = (int) round(pairs[i + 1](0, 1));
			int Yl = (int) round(pairs[i](0, 1));

			if ((Xl - xCl) - (Xr - xCr) > m_nDisparities - 1)
			{
				outRangeDisp = (Xl - xCl) - (Xr - xCr);
			}
			else if ((Xl - xCl) - (Xr - xCr) >= 0)
			{
				m_knownPointsMap(Xr, Yr, 0, 0) = 1;
				m_knownPointsMap(Xr, Yr, 0, 1) = Yl - Yr;
				m_knownPointsProbas[Xr + (Yr * W)] = CImg<float>(m_nDisparities, 1, 1, 1, 0);
				m_knownPointsProbas[Xr + (Yr * W)]((Xl - xCl) - (Xr - xCr)) = 1;
			}
		}
		m_nDisparities = regmax(outRangeDisp + 1, m_nDisparities);
	} while (outRangeDisp > 0);


	m_errorMap = interpLinear2d(m_knownPointsMap);
	m_errorMap.get_normalize(0, 254).save("error.bmp");

}

void MrfMatching::startPart(int nPart, int *nOper, int *status)
{
	int Y0 = nPart * m_viewPair->getRectView(m_rightId)->dimy() / m_nParts;
	int Y1 = ((nPart + 1) * m_viewPair->getRectView(m_rightId)->dimy() / m_nParts) - 1;
	m_rightParts[nPart] = m_viewPair->getRectView(m_rightId)->get_lines(Y0, Y1);
	m_leftParts[nPart] = m_viewPair->getRectView(m_leftId)->get_lines(Y0, Y1);

	CImg<float> smoothed = anisotropicSmoothing(m_rightParts[nPart], 10, 10);
	CImg<float> horGradientMap  = sobelFilter(smoothed, 'x');
	CImg<float> verGradientMap  = sobelFilter(smoothed, 'y');
	//CImg<float> updiagGradientMap = horGradientMap + verGradientMap;
	//CImg<float> downdiagGradientMap = horGradientMap - verGradientMap;
	horGradientMap.abs();
	verGradientMap.abs();
	//updiagGradientMap.abs();
	//downdiagGradientMap.abs();
	//float maxGradient = regmax(regmax(horGradientMap.max(), verGradientMap.max()), regmax(updiagGradientMap.max(), downdiagGradientMap.max()));
	float maxGradient = regmax(horGradientMap.max(), verGradientMap.max());


	int W = m_rightParts[nPart].dimx();
	int H = m_rightParts[nPart].dimy();
	int XC0 = 0;//128;
	int XC1 = W - 1;//187;
	int YC0 = 0; //126;
	int YC1 = H - 1; //235;
	int w = (XC1 - XC0 + 1) / m_step;
	int h = (YC1 - YC0 + 1) / m_step;
	int nPoints = w * h;
	int xCr = 0;//(int) (m_viewPair->getRectView(m_rightId)->getK())(2, 0);
	int xCl = 0;//(int) (m_viewPair->getRectView(m_leftId)->getK())(2, 0);

	if (nOper != NULL)
	{
		(*nOper) += 4 * nPoints / 3;
	}


	CImg<float> disparityProbas(m_nDisparities);
	Real *disparityCosts = new Real[nPoints * m_nDisparities];

	CImg<float> rightPatch(2 * m_radius + 1, 2 * m_radius + 1);
	CImg<float> leftPatch(2 * m_radius + 1, 2 * m_radius + 1);

	for (int Y = YC0; Y <= YC1 - m_step + 1; Y+=m_step)
	{
		int y = (Y - YC0) / m_step;

		for (int Xr = XC0; Xr <= XC1 - m_step + 1; Xr+=m_step)
		{
			int xR = (Xr - XC0) / m_step;

			CImg<float> P(1, 3, 1, 1, 1);
			P(0, 0) = Xr;
			P(0, 1) = Y + Y0;
			if (isOutOfFrame(m_viewPair->getRectFilledRegion(m_rightId), P))
			{
				disparityProbas = CImg<float>(m_nDisparities, 1, 1, 1, 0);
				disparityProbas(0) = 1;
			}
			else if (m_knownPointsMap(Xr, Y + Y0, 0, 0))
			{
				disparityProbas = m_knownPointsProbas[Xr + ((Y + Y0) * W)];
			}
			else
			{
				rightPatch = m_rightParts[nPart].get_crop(Xr - m_radius, Y - m_radius, Xr + m_radius, Y + m_radius);

				float averageCorrel = 0;
				for (int Xl = Xr - xCr + xCl; Xl < regmin(Xr - xCr + xCl + m_nDisparities, W); Xl++)
				{
					leftPatch = m_leftParts[nPart].get_crop(Xl - m_radius, Y + m_errorMap(Xr, Y) - m_radius, Xl + m_radius, Y + m_errorMap(Xr, Y) + m_radius);
					disparityProbas((Xl - xCl) - (Xr - xCr)) = crossCorrel(leftPatch, rightPatch);
					averageCorrel += disparityProbas((Xl - xCl) - (Xr - xCr));
						
				}
				averageCorrel /= regmin(Xr - xCr + xCl + m_nDisparities, W) - (Xr - xCr + xCl);

				if (Xr - xCr + xCl + m_nDisparities > W)
				{
					for (int Xl = W; Xl < Xr - xCr + xCl + m_nDisparities; Xl++)
					{
						disparityProbas((Xl - xCl) - (Xr - xCr)) = averageCorrel;
					}
				}

			}

			if (disparityProbas.norm(1) > 0)
			{
				float alpha = (1 - (m_nDisparities * FLT_EPSILON)) / disparityProbas.norm(1);
				disparityProbas *= alpha;
			}
			disparityProbas += FLT_EPSILON;
			disparityProbas.apply(probaEnergy);

			float maxCost = disparityProbas.max();
			for (int d = 0; d < m_nDisparities; d++)
			{
				disparityCosts[(y * w) + xR + (d * nPoints)] = (Real)(INT_MAX * disparityProbas(d) / maxCost);
			}

			if (status != NULL)
			{
				(*status)++;
			}
		}
	}

	int nPairs = 2 * ((2 * w * h) - w - h);// + 4 * (w - 1) * (h - 1);
	int *pairs = new int[2 * nPairs];
	Real *pairWeights = new Real[nPairs];

	float intMax = (float)(INT_MAX);
	int i = 0;
	for (int X = XC0; X <= XC1 - m_step + 1; X+=m_step)
	{
		int x = (X - XC0) / m_step;

		for (int Y = YC0; Y <= YC1 - m_step + 1; Y+=m_step)
		{
			int y = (Y - YC0) / m_step;

			if (y < h - 1)
			{
				pairWeights[i] = (Real)(sqrt(intMax) * (1 - (verGradientMap(X, Y) / maxGradient)));
				//pairWeights[i] = (Real)(sqrt(intMax));
				pairs[2 * i] = (y * w) + x;
				pairs[(2 * i) + 1] = ((y + 1) * w) + x;
				i++;
				pairWeights[i] = (Real)(sqrt(intMax) * (1 - (verGradientMap(X, Y) / maxGradient)));
				//pairWeights[i] = (Real)(sqrt(intMax));
				pairs[2 * i] = ((y + 1) * w) + x;
				pairs[(2 * i) + 1] = (y * w) + x;
				i++;
			}

			if (x < w - 1)
			{
				pairWeights[i] = (Real)(sqrt(intMax) * (1 - (horGradientMap(X, Y) / maxGradient)));
				//pairWeights[i] = (Real)(sqrt(intMax));
				pairs[2 * i] = (y * w) + x;
				pairs[(2 * i) + 1] = (y * w) + x + 1;
				i++;
				pairWeights[i] = (Real)(sqrt(intMax) * (1 - (horGradientMap(X, Y) / maxGradient)));
				//pairWeights[i] = (Real)(sqrt(intMax));
				pairs[2 * i] = (y * w) + x + 1;
				pairs[(2 * i) + 1] = (y * w) + x;
				i++;
			}

			//if (y < h - 1 && x < w - 1)
			//{
			////	pairWeights[i] = (Real)(sqrt(intMax) * (1 - (downdiagGradientMap(X, Y) / maxGradient)));
			//	pairWeights[i] = (Real)(sqrt(intMax));
			//	pairs[2 * i] = (y * w) + x;
			//	pairs[(2 * i) + 1] = ((y + 1) * w) + x + 1;
			//	i++;
			//	//pairWeights[i] = (Real)(sqrt(intMax) * (1 - (horGradientMap(X, Y) / maxGradient)));
			//	pairWeights[i] = (Real)(sqrt(intMax));
			//	pairs[2 * i] = ((y + 1) * w) + x + 1;
			//	pairs[(2 * i) + 1] = (y * w) + x;
			//	i++;

			//	//pairWeights[i] = (Real)(sqrt(intMax) * (1 - (updiagGradientMap(X, Y) / maxGradient)));
			//	pairWeights[i] = (Real)(sqrt(intMax));
			//	pairs[2 * i] = ((y + 1) * w) + x;
			//	pairs[(2 * i) + 1] = (y * w) + x + 1;
			//	i++;

			//	//pairWeights[i] = (Real)(sqrt(intMax) * (1 - (updiagGradientMap(X, Y) / maxGradient)));
			//	pairWeights[i] = (Real)(sqrt(intMax));
			//	pairs[2 * i] = (y * w) + x + 1;
			//	pairs[(2 * i) + 1] = ((y + 1) * w) + x;
			//	i++;
			//}
		}
	}

	Real *disparityDists = new Real[m_nDisparities * m_nDisparities];
	for (int i = 0; i < m_nDisparities; i++)
	{
		for (int j = 0; j < m_nDisparities; j++)
		{
			disparityDists[(i * m_nDisparities) + j] = (Real)(sqrt(intMax) * pow(abs((float) (j - i)) / (m_nDisparities - 1), (float) 1));
		}
	}

	//FILE *outputFile = fopen("graph.bin", "wb" );
	//fwrite(&w, sizeof(int), 1, outputFile);
	//fwrite(&h, sizeof(int), 1, outputFile);
	//fwrite(&nPoints, sizeof(int), 1, outputFile);
	//fwrite(&nPairs, sizeof(int), 1, outputFile);
	//fwrite(&m_nDisparities, sizeof(int), 1, outputFile);
	//fwrite(disparityCosts, sizeof(Real), nPoints * m_nDisparities, outputFile);
	//fwrite(pairs, sizeof(int), 2 * nPairs, outputFile);
	//fwrite(disparityDists, sizeof(Real), m_nDisparities * m_nDisparities, outputFile);
	//fwrite(pairWeights, sizeof(Real), nPairs, outputFile);
	//fclose(outputFile);



	CV_Fast_PD pd(nPoints, m_nDisparities, disparityCosts, nPairs, pairs, disparityDists, m_maxIters, pairWeights);
	pd.run();

	if (status != NULL)
	{
		(*status) += nPoints / 3;
	}

	delete[] disparityCosts;
	delete[] disparityDists;
	delete[] pairWeights;
	delete[] pairs;

	m_outputParts[nPart].assign(w, h, 1, 1, 0);
	int x, y;
	for (int i = 0; i < nPoints; i++)
	{
		y = i / w;
		x = i % w;
		m_outputParts[nPart](x, y) = pd._pinfo[i].label;// + xCl - xCr;
	}
	m_outputParts[nPart].resize(XC1 - XC0 + 1, YC1 - YC0 + 1, 1, 1, 5);

}

bool MrfMatching::initialOrder()
{
	return m_initialOrder;
}

CImg<float> MrfMatching::disparityMap()
{
	CImg<float> output;
	for (int i = 0; i < m_nParts; i++)
	{
		output.append(m_outputParts[i], 'y');
	}
	return output;
}


float MrfMatching::probaEnergy(float proba)
{
	return -1 * log(proba);
}