#include "SiftSearch.h"

#include <FLOAT.H>
#include <QList>


bool cmpKeypoints(Keypoints::value_type const&a, Keypoints::value_type const&b) 
{
	return a.first.sigma < b.first.sigma;
}


void insertDescriptor(CImgList<float> &outList, VL::float_t x, VL::float_t y, VL::float_t sig, VL::float_t ang, VL::float_t const * descr_pt)
{
	CImg<float> descList(132);

	descList(0) = x;
	descList(1) = y;
	descList(2) = sig;
	descList(3) = ang;  

	for(int i = 0; i < 128; i++)
	{
		descList(i + 4) = descr_pt[i];
	}

	outList.push_back(descList);

}


void imageToBuffer(CImg<float> &input, VL::PgmBuffer *buffer)
{
	int width = input.dimx();
	int height = input.dimy();
	int dimsSize = width*height;

	float maxval = input.max();

	buffer->width = width;
	buffer->height = height;

	buffer->data = new VL::pixel_t [dimsSize];

	int i = 0;
	float r, g, b;
	cimg_forXY(input, x, y)
	{
			i = x + y * width;
			r = input(x, y, 0, 0) / maxval;
			g = input(x, y, 0, 1) / maxval;
			b = input(x, y, 0, 2) / maxval;
			buffer->data[i] = ((VL::pixel_t) 0.3 * r + 0.59 * g + 0.11 * b);
	}
}


void siftPointsSearch(CImg<float> &input, CImgList<float> &outList)
{
	VL::PgmBuffer buffer;
	buffer.data = NULL;
	imageToBuffer(input, &buffer);

	keyVector keypointList;

	int levels = 3;
	float threshold = 0.04f / levels / 2.0f;
	float edgeThreshold = 10.0f;
	float magnif = 3.0;
	int const S = levels;
	int const omin = -1;
	float const sigman = .5;
	float const sigma0 = 1.6 * powf(2.0f, 1.0f / S);
	int O = std::max(int(std::floor(log((double) std::min(buffer.width, buffer.height)) / log(2.)) - omin - 3), 1);

	VL::Sift sift(buffer.data, buffer.width, buffer.height, sigman, sigma0, O, S, omin, -1, S+1);

	sift.detectKeypoints(threshold, edgeThreshold);
	sift.setNormalizeDescriptor(true);
	sift.setMagnification(magnif);

	for(VL::Sift::KeypointsConstIter iter = sift.keypointsBegin(); iter != sift.keypointsEnd(); ++iter) 
	{
		VL::float_t angles [4];
		int nangles;
		nangles = sift.computeKeypointOrientations(angles, *iter);

		for(int a = 0 ; a < nangles ; ++a) 
		{

			VL::float_t descr_pt [128];
			sift.computeKeypointDescriptor(descr_pt, *iter, angles[a]);
      
			insertDescriptor(outList, iter->x, iter->y, iter->sigma, angles[a], descr_pt);
		} 
	} 

	if(buffer.data != NULL)
	{
		delete[] buffer.data;
	}
}

void siftPairsSearch(CImgList<float> &siftPoints0, CImgList<float> &siftPoints1, CImgList<float> &outList, float maxSimilarity, int *nOper, int *status)
{

	if (nOper != NULL)
	{
		(*nOper) += siftPoints0.size * siftPoints1.size;
	}
	//float correl, maxCorrel, secMaxCorrel;
	//int argmax;
	//CImg<float> desc0, desc1;
	//for (int i = 0; i < siftPoints0.size; i++)
	//{
	//	desc0 = siftPoints0[i].get_columns(4, 131);

	//	maxCorrel = 0;
	//	secMaxCorrel = 0;
	//	argmax = 0;
	//	for (int j = 0; j < siftPoints1.size; j++)
	//	{
	//		desc1 = siftPoints1[j].get_columns(4, 131);
	//		correl = crossCorrel(desc0, desc1);

	//		if (correl > maxCorrel)
	//		{
	//			secMaxCorrel = maxCorrel;
	//			maxCorrel = correl;
	//			argmax = j;
	//		}
	//		else if (correl > secMaxCorrel)
	//		{
	//			secMaxCorrel = correl;
	//		}
	//		if (status != NULL)
	//		{
	//			(*status)++;
	//		}
	//	}
	//	if (maxCorrel * (1 - minSharpness) > secMaxCorrel  && maxCorrel > minCorrel)
	//	{
	//		outList << toHomog(siftPoints0[i].get_columns(0, 1).get_transpose());
	//		outList << toHomog(siftPoints1[argmax].get_columns(0, 1).get_transpose());
	//	}
	//}

	CImg<float> desc0;
	QList<int> matchedPts;
	QList<int> matchedPtsInd;
	for (int i = 0; i < siftPoints0.size; i++)
	{
		desc0 = siftPoints0[i].get_columns(4, 131);
		CImg<float> desc1;
		float dist;
		float minDist = FLT_MAX;
		float secMinDist = FLT_MAX;
		int argMin = 0;

		for (int j = 0; j < siftPoints1.size; j++)
		{
			desc1 = siftPoints1[j].get_columns(4, 131);
			dist = (desc0 - desc1).norm(2);

			if (dist < minDist)
			{
				secMinDist = minDist;
				minDist = dist;
				argMin = j;
			}
			else if (dist < secMinDist)
			{
				secMinDist = dist;
			}
			if (status != NULL)
			{
				(*status)++;
			}
		}
		if (minDist < maxSimilarity * secMinDist)
		{
			int index = matchedPts.indexOf(argMin);
			if (index >= 0)
			{
				outList.remove(matchedPtsInd[index], matchedPtsInd[index] + 1);
			}
			else
			{
				matchedPts << argMin;
				matchedPtsInd << outList.size;
				outList << toHomog(siftPoints0[i].get_columns(0, 1).get_transpose());
				outList << toHomog(siftPoints1[argMin].get_columns(0, 1).get_transpose());
			}
		}
	}

}


