#ifndef _OpticalFlow_h
#define _OpticalFlow_h

#include "Image.h"

class OpticalFlow
{
private:
	static bool IsDisplay;
public:
	OpticalFlow(void);
	~OpticalFlow(void);
public:
	static void getDxs(DImage& imdx,DImage& imdy,DImage& imdt,const DImage& im1,const DImage& im2);
	static void SanityCheck(const DImage& imdx,const DImage& imdy,const DImage& imdt,double du,double dv);
	static void warpFL(DImage& warpIm2,const DImage& Im1,const DImage& Im2,const DImage& vx,const DImage& vy);
	static void genConstFlow(DImage& flow,double value,int width,int height);
	static void genInImageMask(DImage& mask,const DImage& vx,const DImage& vy);
	static void SmoothFlowPDE(const DImage& Im1,const DImage& Im2, DImage& warpIm2,DImage& vx,DImage& vy,
														 double alpha,int nOuterFPIterations,int nInnerFPIterations,int nCGIterations);
	static void Laplacian(DImage& output,const DImage& input,const DImage& weight);
	static void testLaplacian(int dim=3);

	// function of coarse to fine optical flow
	static void Coarse2FineFlow(DImage& vx,DImage& vy,DImage &warpI2,const DImage& Im1,const DImage& Im2,double alpha,double ratio,int minWidth,
															int nOuterFPIterations,int nInnerFPIterations,int nCGIterations);
	// function to convert image to features
	static void im2feature(DImage& imfeature,const DImage& im);
};

#endif