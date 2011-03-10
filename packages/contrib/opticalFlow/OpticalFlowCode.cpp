#include "OpticalFlow.h"
#include "ImageProcessing.h"
#include "GaussianPyramid.h"
#include <cstdlib> 
#include <iostream>

using namespace std;

bool OpticalFlow::IsDisplay=false;

OpticalFlow::OpticalFlow(void)
{
}

OpticalFlow::~OpticalFlow(void)
{
}

//--------------------------------------------------------------------------------------------------------
//  function to compute dx, dy and dt for motion estimation
//--------------------------------------------------------------------------------------------------------
void OpticalFlow::getDxs(DImage &imdx, DImage &imdy, DImage &imdt, const DImage &im1, const DImage &im2)
{
	// Im1 and Im2 are the smoothed version of im1 and im2
	DImage Im1,Im2;
	double gfilter[5]={0.05,0.2,0.5,0.2,0.05};
	im1.imfilter_hv(Im1,gfilter,2,gfilter,2);
	im2.imfilter_hv(Im2,gfilter,2,gfilter,2);

    //Im1.copyData(im1);
    //Im2.copyData(im2);
    
	Im2.dx(imdx,true);
	Im2.dy(imdy,true);
	imdt.Subtract(Im2,Im1);
	imdx.setDerivative();
	imdy.setDerivative();
	imdt.setDerivative();
}

//--------------------------------------------------------------------------------------------------------
// function to do sanity check: imdx*du+imdy*dy+imdt=0
//--------------------------------------------------------------------------------------------------------
void OpticalFlow::SanityCheck(const DImage &imdx, const DImage &imdy, const DImage &imdt, double du, double dv)
{
	if(imdx.matchDimension(imdy)==false || imdx.matchDimension(imdt)==false)
	{
		cout<<"The dimensions of the derivatives don't match!"<<endl;
		return;
	}
	const double* pImDx,*pImDy,*pImDt;
	pImDx=imdx.data();
	pImDy=imdy.data();
	pImDt=imdt.data();
	double error=0;
	for(int i=0;i<imdx.height();i++)
		for(int j=0;j<imdx.width();j++)
			for(int k=0;k<imdx.nchannels();k++)
			{
				int offset=(i*imdx.width()+j)*imdx.nchannels()+k;
				double temp=pImDx[offset]*du+pImDy[offset]*dv+pImDt[offset];
				error+=fabs(temp);
			}
	error/=imdx.nelements();
	cout<<"The mean error of |dx*u+dy*v+dt| is "<<error<<endl;
}

//--------------------------------------------------------------------------------------------------------
// function to warp image based on the flow field
//--------------------------------------------------------------------------------------------------------
void OpticalFlow::warpFL(DImage &warpIm2, const DImage &Im1, const DImage &Im2, const DImage &vx, const DImage &vy)
{
	if(warpIm2.matchDimension(Im2)==false)
		warpIm2.allocate(Im2.width(),Im2.height(),Im2.nchannels());
	ImageProcessing::warpImage(warpIm2.data(),Im1.data(),Im2.data(),vx.data(),vy.data(),Im2.width(),Im2.height(),Im2.nchannels());
}

//--------------------------------------------------------------------------------------------------------
// function to generate mask of the pixels that move inside the image boundary
//--------------------------------------------------------------------------------------------------------
void OpticalFlow::genInImageMask(DImage &mask, const DImage &vx, const DImage &vy)
{
	int imWidth,imHeight;
	imWidth=vx.width();
	imHeight=vx.height();
	if(mask.matchDimension(vx)==false)
		mask.allocate(imWidth,imHeight);
	const double *pVx,*pVy;
	double *pMask;
	pVx=vx.data();
	pVy=vy.data();
	mask.reset();
	pMask=mask.data();
	double x,y;
	for(int i=0;i<imHeight;i++)
		for(int j=0;j<imWidth;j++)
		{
			int offset=i*imWidth+j;
			y=i+pVx[offset];
			x=j+pVy[offset];
			if(x<0  || x>imWidth-1 || y<0 || y>imHeight-1)
				continue;
			pMask[offset]=1;
		}
}

//--------------------------------------------------------------------------------------------------------
// function to compute optical flow field using two fixed point iterations
// Input arguments:
//     Im1, Im2:						frame 1 and frame 2
//	warpIm2:						the warped frame 2 according to the current flow field u and v
//	u,v:									the current flow field, NOTICE that they are also output arguments
//	
//--------------------------------------------------------------------------------------------------------
void OpticalFlow::SmoothFlowPDE(const DImage &Im1, const DImage &Im2, DImage &warpIm2, DImage &u, DImage &v, 
																    double alpha, int nOuterFPIterations, int nInnerFPIterations, int nCGIterations)
{
	DImage mask,imdx,imdy,imdt;
	int imWidth,imHeight,nChannels,nPixels;
	imWidth=Im1.width();
	imHeight=Im1.height();
	nChannels=Im1.nchannels();
	nPixels=imWidth*imHeight;

	DImage du(imWidth,imHeight),dv(imWidth,imHeight);
	DImage uu(imWidth,imHeight),vv(imWidth,imHeight);
	DImage ux(imWidth,imHeight),uy(imWidth,imHeight);
	DImage vx(imWidth,imHeight),vy(imWidth,imHeight);
	DImage Phi_1st(imWidth,imHeight);
	DImage Psi_1st(imWidth,imHeight,nChannels);

	DImage imdxy,imdx2,imdy2,imdtdx,imdtdy;
	DImage ImDxy,ImDx2,ImDy2,ImDtDx,ImDtDy;
	DImage A11,A12,A22,b1,b2;
	DImage foo1,foo2;

	// variables for conjugate gradient
	DImage r1,r2,p1,p2,q1,q2;
	double* rou;
	rou=new double[nCGIterations];

	double varepsilon_phi=pow(0.001,2);
	double varepsilon_psi=pow(0.001,2);

	//--------------------------------------------------------------------------
	// the outer fixed point iteration
	//--------------------------------------------------------------------------
	for(int count=0;count<nOuterFPIterations;count++)
	{
		// compute the gradient
		getDxs(imdx,imdy,imdt,Im1,warpIm2);

		// generate the mask to set the weight of the pxiels moving outside of the image boundary to be zero
		genInImageMask(mask,vx,vy);

		// set the derivative of the flow field to be zero
		du.reset();
		dv.reset();

		//--------------------------------------------------------------------------
		// the inner fixed point iteration
		//--------------------------------------------------------------------------
		for(int hh=0;hh<nInnerFPIterations;hh++)
		{
			// compute the derivatives of the current flow field
			if(hh==0)
			{
				uu.copyData(u);
				vv.copyData(v);
			}
			else
			{
				uu.Add(u,du);
				vv.Add(v,dv);
			}
			uu.dx(ux);
			uu.dy(uy);
			vv.dx(vx);
			vv.dy(vy);

			// compute the weight of phi
			Phi_1st.reset();
			double* phiData=Phi_1st.data();
			double temp;
			const double *uxData,*uyData,*vxData,*vyData;
			uxData=ux.data();
			uyData=uy.data();
			vxData=vx.data();
			vyData=vy.data();
			for(int i=0;i<nPixels;i++)
			{
				temp=uxData[i]*uxData[i]+uyData[i]*uyData[i]+vxData[i]*vxData[i]+vyData[i]*vyData[i];
				phiData[i]=1/(2*sqrt(temp+varepsilon_phi));
			}

			// compute the nonlinear term of psi
			Psi_1st.reset();
			double* psiData=Psi_1st.data();
			const double *imdxData,*imdyData,*imdtData;
			const double *duData,*dvData;
			imdxData=imdx.data();
			imdyData=imdy.data();
			imdtData=imdt.data();
			duData=du.data();
			dvData=dv.data();
		
			double _a  = 10000, _b = 0.1;
			if(nChannels==1)
			{
				for(int i=0;i<nPixels;i++)
				{
					temp=imdtData[i]+imdxData[i]*duData[i]+imdyData[i]*dvData[i];
					//if(temp*temp<0.04)
					psiData[i]=1/(2*sqrt(temp*temp+varepsilon_psi));
					//psiData[i] = _a*_b/(1+_a*temp*temp);
				}
			}
			else
			{
				for(int i=0;i<nPixels;i++)
					for(int k=0;k<nChannels;k++)
					{
						int offset=i*nChannels+k;
						temp=imdtData[offset]+imdxData[offset]*duData[i]+imdyData[offset]*dvData[i];
						//if(temp*temp<0.04)
						psiData[offset]=1/(2*sqrt(temp*temp+varepsilon_psi));
						//psiData[offset] =  _a*_b/(1+_a*temp*temp);
					}
			}

			// prepare the components of the large linear system
			ImDxy.Multiply(Psi_1st,imdx,imdy);
			ImDx2.Multiply(Psi_1st,imdx,imdx);
			ImDy2.Multiply(Psi_1st,imdy,imdy);
			ImDtDx.Multiply(Psi_1st,imdx,imdt);
			ImDtDy.Multiply(Psi_1st,imdy,imdt);

			if(nChannels>1)
			{
				ImDxy.collapse(imdxy);
				ImDx2.collapse(imdx2);
				ImDy2.collapse(imdy2);
				ImDtDx.collapse(imdtdx);
				ImDtDy.collapse(imdtdy);
			}
			else
			{
				imdxy.copyData(ImDxy);
				imdx2.copyData(ImDx2);
				imdy2.copyData(ImDy2);
				imdtdx.copyData(ImDtDx);
				imdtdy.copyData(ImDtDy);
			}

			// filtering
			imdx2.smoothing(A11,3);
			imdxy.smoothing(A12,3);
			imdy2.smoothing(A22,3);

			// add epsilon to A11 and A22
			A11.Add(alpha*0.1);
			A22.Add(alpha*0.1);

			// form b
			imdtdx.smoothing(b1,3);
			imdtdy.smoothing(b2,3);
			// laplacian filtering of the current flow field
		    Laplacian(foo1,u,Phi_1st);
			Laplacian(foo2,v,Phi_1st);
			double *b1Data,*b2Data;
			const double *foo1Data,*foo2Data;
			b1Data=b1.data();
			b2Data=b2.data();
			foo1Data=foo1.data();
			foo2Data=foo2.data();

			for(int i=0;i<nPixels;i++)
			{
				b1Data[i]=-b1Data[i]-alpha*foo1Data[i];
				b2Data[i]=-b2Data[i]-alpha*foo2Data[i];
			}

			// for debug only, displaying the matrix coefficients
			//A11.imwrite("A11.bmp",ImageIO::normalized);
			//A12.imwrite("A12.bmp",ImageIO::normalized);
			//A22.imwrite("A22.bmp",ImageIO::normalized);
			//b1.imwrite("b1.bmp",ImageIO::normalized);
			//b2.imwrite("b2.bmp",ImageIO::normalized);

			//-----------------------------------------------------------------------
			// conjugate gradient algorithm
			//-----------------------------------------------------------------------
			r1.copyData(b1);
			r2.copyData(b2);
			du.reset();
			dv.reset();

			for(int k=0;k<nCGIterations;k++)
			{
				rou[k]=r1.norm2()+r2.norm2();
				//cout<<rou[k]<<endl;
				if(rou[k]<1E-10)
					break;
				if(k==0)
				{
					p1.copyData(r1);
					p2.copyData(r2);
				}
				else
				{
					double ratio=rou[k]/rou[k-1];
					p1.Add(r1,p1,ratio);
					p2.Add(r2,p2,ratio);
				}
				// go through the large linear system
				foo1.Multiply(A11,p1);
				foo2.Multiply(A12,p2);
				q1.Add(foo1,foo2);
				Laplacian(foo1,p1,Phi_1st);
				q1.Add(foo1,alpha);

				foo1.Multiply(A12,p1);
				foo2.Multiply(A22,p2);
				q2.Add(foo1,foo2);
				Laplacian(foo2,p2,Phi_1st);
				q2.Add(foo2,alpha);

				double beta;
				beta=rou[k]/(p1.innerproduct(q1)+p2.innerproduct(q2));
				
				du.Add(p1,beta);
				dv.Add(p2,beta);

				r1.Add(q1,-beta);
				r2.Add(q2,-beta);
			}
			//-----------------------------------------------------------------------
			// end of conjugate gradient algorithm
			//-----------------------------------------------------------------------
		}// end of inner fixed point iteration
		
		// the following procedure is merely for debugging
		//cout<<"du "<<du.norm2()<<" dv "<<dv.norm2()<<endl;
		// update the flow field
		u.Add(du,1);
		v.Add(dv,1);
		warpFL(warpIm2,Im1,Im2,u,v);
	}// end of outer fixed point iteration
	
	
	delete rou;
}

void OpticalFlow::Laplacian(DImage &output, const DImage &input, const DImage& weight)
{
	if(output.matchDimension(input)==false)
		output.allocate(input);
	output.reset();

	if(input.matchDimension(weight)==false)
	{
		cout<<"Error in image dimension matching OpticalFlow::Laplacian()!"<<endl;
		return;
	}
	
	const double *inputData=input.data(),*weightData=weight.data();
	int width=input.width(),height=input.height();
	DImage foo(width,height);
	double *fooData=foo.data(),*outputData=output.data();

	// horizontal filtering
	for(int i=0;i<height;i++)
		for(int j=0;j<width-1;j++)
		{
			int offset=i*width+j;
			fooData[offset]=(inputData[offset+1]-inputData[offset])*weightData[offset];
		}
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
			int offset=i*width+j;
			if(j<width-1)
				outputData[offset]-=fooData[offset];
			if(j>0)
				outputData[offset]+=fooData[offset-1];
		}
	foo.reset();
	// vertical filtering
	for(int i=0;i<height-1;i++)
		for(int j=0;j<width;j++)
		{
			int offset=i*width+j;
			fooData[offset]=(inputData[offset+width]-inputData[offset])*weightData[offset];
		}
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
			int offset=i*width+j;
			if(i<height-1)
				outputData[offset]-=fooData[offset];
			if(i>0)
				outputData[offset]+=fooData[offset-width];
		}
}

void OpticalFlow::testLaplacian(int dim)
{
	// generate the random weight
	DImage weight(dim,dim);
	for(int i=0;i<dim;i++)
		for(int j=0;j<dim;j++)
			//weight.data()[i*dim+j]=(double)rand()/RAND_MAX+1;
			weight.data()[i*dim+j]=1;
	// go through the linear system;
	DImage sysMatrix(dim*dim,dim*dim);
	DImage u(dim,dim),du(dim,dim);
	for(int i=0;i<dim*dim;i++)
	{
		u.reset();
		u.data()[i]=1;
		Laplacian(du,u,weight);
		for(int j=0;j<dim*dim;j++)
			sysMatrix.data()[j*dim*dim+i]=du.data()[j];
	}
	// test whether the matrix is symmetric
	for(int i=0;i<dim*dim;i++)
	{
		for(int j=0;j<dim*dim;j++)
		{
			if(sysMatrix.data()[i*dim*dim+j]>=0)
				printf(" ");
			printf(" %1.0f ",sysMatrix.data()[i*dim*dim+j]);
		}
		printf("\n");
	}
}

//--------------------------------------------------------------------------------------
// function to perfomr coarse to fine optical flow estimation
//--------------------------------------------------------------------------------------
void OpticalFlow::Coarse2FineFlow(DImage &vx, DImage &vy, DImage &warpI2,const DImage &Im1, const DImage &Im2, double alpha, double ratio, int minWidth, 
																	 int nOuterFPIterations, int nInnerFPIterations, int nCGIterations)
{
	// first build the pyramid of the two images
	GaussianPyramid GPyramid1;
	GaussianPyramid GPyramid2;
	if(IsDisplay)
		cout<<"Constructing pyramid...";
	GPyramid1.ConstructPyramid(Im1,ratio,minWidth);
	GPyramid2.ConstructPyramid(Im2,ratio,minWidth);
	if(IsDisplay)
		cout<<"done!"<<endl;
	
	// now iterate from the top level to the bottom
	DImage Image1,Image2,WarpImage2;

	for(int k=GPyramid1.nlevels()-1;k>=0;k--)
	{
		if(IsDisplay)
			cout<<"Pyramid level "<<k;
		int width=GPyramid1.Image(k).width();
		int height=GPyramid1.Image(k).height();
		im2feature(Image1,GPyramid1.Image(k));
		im2feature(Image2,GPyramid2.Image(k));

		if(k==GPyramid1.nlevels()-1) // if at the top level
		{
			vx.allocate(width,height);
			vy.allocate(width,height);
			//warpI2.copyData(Image2);
			WarpImage2.copyData(Image2);
		}
		else
		{

			vx.imresize(width,height);
			vx.Multiplywith(1/ratio);
			vy.imresize(width,height);
			vy.Multiplywith(1/ratio);
			//warpFL(warpI2,GPyramid1.Image(k),GPyramid2.Image(k),vx,vy);
			warpFL(WarpImage2,Image1,Image2,vx,vy);
		}
		//SmoothFlowPDE(GPyramid1.Image(k),GPyramid2.Image(k),warpI2,vx,vy,alpha,nOuterFPIterations,nInnerFPIterations,nCGIterations);
		//SmoothFlowPDE(Image1,Image2,WarpImage2,vx,vy,alpha*pow((1/ratio),k),nOuterFPIterations,nInnerFPIterations,nCGIterations);
		SmoothFlowPDE(Image1,Image2,WarpImage2,vx,vy,alpha,nOuterFPIterations,nInnerFPIterations,nCGIterations);
		if(IsDisplay)
			cout<<endl;
	}
	warpFL(warpI2,Im1,Im2,vx,vy);
}

//---------------------------------------------------------------------------------------
// function to convert image to feature image
//---------------------------------------------------------------------------------------
void OpticalFlow::im2feature(DImage &imfeature, const DImage &im)
{
	int width=im.width();
	int height=im.height();
	int nchannels=im.nchannels();
	if(nchannels==1)
	{
		imfeature.allocate(im.width(),im.height(),3);
		DImage imdx,imdy;
		im.dx(imdx,true);
		im.dy(imdy,true);
		double* data=imfeature.data();
		for(int i=0;i<height;i++)
			for(int j=0;j<width;j++)
			{
				int offset=i*width+j;
				data[offset*3]=im.data()[offset];
				data[offset*3+1]=imdx.data()[offset];
				data[offset*3+2]=imdy.data()[offset];
			}
	}
	else if(nchannels==3)
	{
		DImage grayImage;
		im.desaturate(grayImage);

		imfeature.allocate(im.width(),im.height(),5);
		DImage imdx,imdy;
		grayImage.dx(imdx,true);
		grayImage.dy(imdy,true);
		double* data=imfeature.data();
		for(int i=0;i<height;i++)
			for(int j=0;j<width;j++)
			{
				int offset=i*width+j;
				data[offset*5]=grayImage.data()[offset];
				data[offset*5+1]=imdx.data()[offset];
				data[offset*5+2]=imdy.data()[offset];
				data[offset*5+3]=im.data()[offset*3+1]-im.data()[offset*3];
				data[offset*5+4]=im.data()[offset*3+1]-im.data()[offset*3+2];
			}
	}
	else
		imfeature.copyData(im);
}
