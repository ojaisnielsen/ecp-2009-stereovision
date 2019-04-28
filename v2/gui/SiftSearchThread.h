#ifndef DEF_SIFTSEARCHTHREAD
#define DEF_SIFTSEARCHTHREAD

#include "../core/Stereo.h"
#include "../tools/siftSearch.h"
#include "../tools/regminmax.h"

class SiftSearchThread : public QThread
{
public:
	SiftSearchThread(View *input0, View *input1, float maxSimilarity);
	void run();
	int getStatus();
	CImgList<float> &getSiftPairs();

private:
	class SiftPointsThread : public QThread
	{
	public:
		SiftPointsThread(View *input, CImgList<float> *output);
		void run();
	private:
		View *m_input;
		CImgList<float> *m_output;
	};
	View *m_input0;
	View *m_input1;
	float m_maxSimilarity;
	int m_status;
	int m_nOper;
	CImgList<float> m_siftPoints0;
	CImgList<float> m_siftPoints1;
	CImgList<float> m_siftPairs;
};

#endif