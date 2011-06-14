//==============================================================================
// File: opencv
//
// Description: A wrapper for a couple of OpenCV functions.
//
// Created: February 12, 2010, 1:22AM
//
// Install on ubuntu : 
//  http://www.samontab.com/web/2010/04/installing-opencv-2-1-in-ubuntu/
//
// Author: Clement Farabet // clement.farabet@gmail.com
//==============================================================================

#include <luaT.h>
#include <TH.h>

#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>

#include <cv.h>
#include <highgui.h>

#define CV_NO_BACKWARD_COMPATIBILITY

//============================================================
// To load this lib in LUA:
// require 'libopencv'

//============================================================
// Conversion helpers
// these functions just create an IPL header to describe 
// the tensor given (no image allocation is done here)
//
static IplImage * doubleImage(THTensor *source) {
  // Get size and channels
  int channels = source->size[2];
  CvSize size = cvSize(source->size[0], source->size[1]);
  
  // Create ipl header
  IplImage * ipl = cvCreateImageHeader(size, IPL_DEPTH_64F, channels);

  // Point to tensor data
  ipl->imageData = (char *)source->storage->data;
  
  // in Torch, channels are separate
  if (channels != 1) ipl->dataOrder = 1;

  return ipl;
}

static IplImage * floatImage(THFloatTensor *source) {
  // Get size and channels
  int channels = source->size[2];
  CvSize size = cvSize(source->size[0], source->size[1]);
  
  // Create ipl header
  IplImage * ipl = cvCreateImageHeader(size, IPL_DEPTH_32F, channels);

  // Point to tensor data
  ipl->imageData = (char *)source->storage->data;
  
  // in Torch, channels are separate
  if (channels != 1) ipl->dataOrder = 1;

  return ipl;
}

static IplImage * charImage(THCharTensor *source) {
  // Get size and channels
  int channels = source->size[2];
  CvSize size = cvSize(source->size[0], source->size[1]);

  // Create ipl header
  IplImage * ipl = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);

  // Point to tensor data
  ipl->imageData = source->storage->data;
    
  // in Torch, channels are separate
  if (channels != 1) ipl->dataOrder = 1;

  return ipl;
}

static IplImage * shortImage(THShortTensor *source) {
  // Get size and channels
  int channels = source->size[2];
  CvSize size = cvSize(source->size[0], source->size[1]);

  // Create ipl header
  IplImage * ipl = cvCreateImageHeader(size, IPL_DEPTH_16S, channels);

  // Point to tensor data
  ipl->imageData = (char *)source->storage->data;
    
  // in Torch, channels are separate
  if (channels != 1) ipl->dataOrder = 1;

  return ipl;
}

//============================================================
// Conversion helpers
// These functions create an IplImage, from a torch Tensor, 
// and the other way around
//
static IplImage * torch2opencv_8U(THTensor *source) {
  // Pointers
  uchar * dest_data;

  // Get size and channels
  int channels = source->size[2];
  int dest_step;
  CvSize dest_size = cvSize(source->size[0], source->size[1]);

  // Create ipl image
  IplImage * dest = cvCreateImage(dest_size, IPL_DEPTH_8U, channels);

  // get pointer to raw data
  cvGetRawData(dest, (uchar**)&dest_data, &dest_step, &dest_size);

  // copy
  int x, y, k;
  for (y=0; y<source->size[1]; y++)
    for (x=0; x<source->size[0]; x++)
      for (k=0; k<source->size[2]; k++) {
        dest_data[ y*dest_step + x*dest->nChannels + (dest->nChannels-1)-k ]
          = (uchar)(THTensor_get3d(source, x, y, k) * 255.0);
      }

  // return freshly created IPL image
  return dest;
}

static IplImage * torch2opencv_32F(THTensor *source) {
  // Pointers
  float * dest_data;

  // Get size and channels
  int channels = source->size[2];
  int dest_step;
  CvSize dest_size = cvSize(source->size[0], source->size[1]);

  // Create ipl image
  IplImage * dest = cvCreateImage(dest_size, IPL_DEPTH_32F, channels);

  // get pointer to raw data
  cvGetRawData(dest, (uchar**)&dest_data, &dest_step, &dest_size);
  dest_step /= sizeof(float);

  // copy
  int x, y, k;
  for (y=0; y<source->size[1]; y++)
    for (x=0; x<source->size[0]; x++)
      for (k=0; k<source->size[2]; k++) {
        dest_data[ y*dest_step + x*dest->nChannels + (dest->nChannels-1)-k ]
          = (float)(THTensor_get3d(source, x, y, k));
      }

  // return freshly created IPL image
  return dest;
}

static THTensor * opencv2torch_8U(IplImage *source, THTensor *dest) {
  // Pointers
  uchar * source_data; 

  // Get pointers / info
  int source_step;
  CvSize source_size;
  cvGetRawData(source, (uchar**)&source_data, &source_step, &source_size);

  // Resize target
  THTensor_resize3d(dest, source->width, source->height, source->nChannels);

  // copy
  int x, y, k;
  for (y=0; y<source->height; y++)
    for (x=0; x<source->width; x++)
      for (k=0; k<source->nChannels; k++) {
        THTensor_set3d(dest, x, y, k, 
                       (double)source_data[ y*source_step + x*source->nChannels + (source->nChannels-1)-k ]);
      }

  // return freshly created IPL image
  return dest;
}

static THTensor * opencv2torch_32F(IplImage *source, THTensor *dest) {
  // Pointers
  float * source_data; 

  // Get pointers / info
  int source_step;
  CvSize source_size;
  cvGetRawData(source, (uchar**)&source_data, &source_step, &source_size);
  source_step /= sizeof(float);

  // Resize target
  THTensor_resize3d(dest, source->width, source->height, source->nChannels);

  // copy
  int x, y, k;
  for (y=0; y<source->height; y++)
    for (x=0; x<source->width; x++)
      for (k=0; k<source->nChannels; k++) {
        THTensor_set3d(dest, x, y, k, 
                       (double)source_data[ y*source_step + x*source->nChannels + (source->nChannels-1)-k ]);
      }

  // return freshly created IPL image
  return dest;
}

static THTensor * opencvPoints2torch_32F(CvPoint2D32f * points, int npoints, THTensor *dest) {

  // Resize target
  THTensor_resize2d(dest, npoints, 2);

  // copy
  int p;
  for (p=0; p<npoints; p++){
    THTensor_set2d(dest, p, 0, points[p].x);
    THTensor_set2d(dest, p, 1, points[p].y);
  }

  // return freshly created IPL image
  return dest;
}

static CvPoint2D32f * torch_32F2opencvPoints(THTensor *src) {

  int count = src->size[0];
  // create output
  CvPoint2D32f * points_cv = 0;
  points_cv = (CvPoint2D32f*)cvAlloc(count*sizeof(points_cv[0]));

  // copy
  int p;
  for (p=0; p<count; p++){
    points_cv[p].x = THTensor_get2d(src, p, 0);
    points_cv[p].y = THTensor_get2d(src, p, 1);
  }

  // return freshly created IPL image
  return points_cv;
}


//============================================================
// Wrapper around simple OpenCV functions
// All these functions work on the Lua stack
// Input and output tensors must be provided, usually in the
// correct format (char/float/double...)
//
static int l_cvCanny (lua_State *L) {
  // Get Tensor's Info
  THCharTensor * source = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.CharTensor"));
  THCharTensor * dest = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.CharTensor"));

  // Generate IPL headers
  IplImage * source_ipl = charImage(source);
  IplImage * dest_ipl = charImage(dest);

  // Thresholds with default values
  double low_threshold = 0;
  double high_threshold = 1;
  int aperture_size = 3;
  if (lua_isnumber(L, 3)) low_threshold = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) high_threshold = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) aperture_size = lua_tonumber(L, 5);

  // Simple call to CV function
  cvCanny(source_ipl, dest_ipl, low_threshold, high_threshold, aperture_size );
  cvScale(dest_ipl, dest_ipl, 0.25, 0);

  // Deallocate headers
  cvReleaseImageHeader(&source_ipl);
  cvReleaseImageHeader(&dest_ipl);

  return 0;
}

static int l_cvSobel (lua_State *L) {
  // Get Tensor's Info
  THCharTensor * source = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.CharTensor"));
  THShortTensor * dest = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.ShortTensor"));

  // Generate IPL headers
  IplImage * source_ipl = charImage(source);
  IplImage * dest_ipl = shortImage(dest);

  // Thresholds with default values
  int dx = 0;
  int dy = 1;
  int aperture_size = 3;
  if (lua_isnumber(L, 3)) dx = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) dy = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) aperture_size = lua_tonumber(L, 5);

  // Simple call to CV function
  cvSobel(source_ipl, dest_ipl, dx, dy, aperture_size);

  // Deallocate headers
  cvReleaseImageHeader(&source_ipl);
  cvReleaseImageHeader(&dest_ipl);

  return 0;
}

//============================================================
static int l_cvCornerHarris(lua_State *L) {
  // Get Tensor's Info
  THTensor * image = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * harris = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));

  IplImage * image_ipl = torch2opencv_8U(image);

  CvSize dest_size = cvSize(image->size[0], image->size[1]);

  // Create ipl image
  IplImage * harris_ipl = cvCreateImage(dest_size, IPL_DEPTH_32F, 1);

  int blockSize = 5;
  int aperture_size = 3;
  double k = 0.04;

  // User values:
  if (lua_isnumber(L, 3)) {
    blockSize = lua_tonumber(L, 3);
  }
  if (lua_isnumber(L, 4)) {
    aperture_size = lua_tonumber(L, 4);
  }
  if (lua_isnumber(L, 5)) {
    k = lua_tonumber(L, 5);
  }

  cvCornerHarris(image_ipl, harris_ipl, blockSize, aperture_size, k);

  // return results
  opencv2torch_32F(harris_ipl, harris);

  // Deallocate IPL images
  cvReleaseImage(&harris_ipl);
  cvReleaseImage(&image_ipl);

  return 0;
}


//============================================================
static int l_cvGoodFeaturesToTrack(lua_State *L) {
  // Get Tensor's Info
  THTensor * image = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * points = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * image_out = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  
  CvSize dest_size = cvSize(image->size[0], image->size[1]);
  IplImage * image_ipl = torch2opencv_8U(image);
  IplImage * image_out_ipl = torch2opencv_8U(image_out);


  IplImage * grey = cvCreateImage( dest_size, 8, 1 );

  cvCvtColor( image_ipl, grey, CV_BGR2GRAY );
  CvPoint2D32f* points_cv = 0;


  IplImage* eig = cvCreateImage( dest_size, 32, 1 );
  IplImage* temp = cvCreateImage( dest_size, 32, 1 );

  int count = 500;
  double quality = 0.01;
  double min_distance = 10;
  int win_size = 10;  

  // User values:
  if (lua_isnumber(L, 4)) {
    count = lua_tonumber(L, 4);
  }
  if (lua_isnumber(L, 5)) {
    quality = lua_tonumber(L, 5);
  }
  if (lua_isnumber(L, 6)) {
    min_distance = lua_tonumber(L, 6);
  }
  if (lua_isnumber(L, 7)) {
    win_size = lua_tonumber(L, 7);
  }

  points_cv = (CvPoint2D32f*)cvAlloc(count*sizeof(points_cv[0]));

  cvGoodFeaturesToTrack( grey, eig, temp, points_cv, &count,
			 quality, min_distance, 0, 3, 0, 0.04 );
  
  cvFindCornerSubPix( grey, points_cv, count,
		      cvSize(win_size,win_size), 
		      cvSize(-1,-1),
		      cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,
				     20,0.03));
  int i;
  for( i = 0; i < count; i++ ) {
    cvCircle( image_out_ipl, cvPointFrom32f(points_cv[i]), 25, 
	      CV_RGB(0,255,0), 1, 8,0);
  }
  
  // return results
  points = opencvPoints2torch_32F(points_cv, count, points);
  opencv2torch_8U(image_out_ipl, image_out);

  // Deallocate points_cv
  cvFree(&points_cv);
  cvReleaseImage( &eig );
  cvReleaseImage( &temp );
  cvReleaseImage( &grey );
  cvReleaseImage( &image_ipl );
  cvReleaseImage( &image_out_ipl );

  return 0;
}

//============================================================
static int l_cvTrackPyrLK(lua_State *L) {
  // Get Tensor's Info
  THTensor * image1 = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * image2 = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * points1 = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * points2 = luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * ff = 0;
  THTensor * fe = 0;

  int count = points1->size[0];
  int win_size = 10;  

  // User values:
  if (lua_isnumber(L, 5)) {
    win_size = lua_tonumber(L, 5);
  }

  if (!lua_isnil(L,6)) {
    ff = luaT_checkudata(L,6,luaT_checktypename2id(L, "torch.Tensor"));
    THTensor_resize1d(ff,count);
  }
  if (!lua_isnil(L,7)) {
    fe = luaT_checkudata(L,7,luaT_checktypename2id(L, "torch.Tensor"));
    THTensor_resize1d(fe,count);
  }

  CvSize dest_size = cvSize(image1->size[0], image1->size[1]);
  IplImage * image1_ipl = torch2opencv_8U(image1);
  IplImage * image2_ipl = torch2opencv_8U(image2);
  

  IplImage * grey1 = cvCreateImage( dest_size, 8, 1 );
  IplImage * grey2 = cvCreateImage( dest_size, 8, 1 );

  cvCvtColor( image1_ipl, grey1, CV_BGR2GRAY );
  cvCvtColor( image2_ipl, grey2, CV_BGR2GRAY );
  CvPoint2D32f* points1_cv = torch_32F2opencvPoints(points1);
  CvPoint2D32f* points2_cv = 0;
  points2_cv = (CvPoint2D32f*)cvAlloc(count*sizeof(points2_cv[0]));


  // Call Lucas Kanade algorithm
  char features_found[ count ];
  float feature_errors[ count ];
  CvSize pyr_sz = cvSize( image1_ipl->width+8, image1_ipl->height/3 );

  IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  
  cvCalcOpticalFlowPyrLK( grey1, grey2, 
			  pyrA, pyrB, 
			  points1_cv, points2_cv, 
			  count, 
			  cvSize( win_size, win_size ), 
			  5, features_found, feature_errors,
			  cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.3 ), 0 );
  
  // return results
  opencvPoints2torch_32F(points2_cv, count, points2);
  int i;
  if (ff != 0){
    for(i=0;i<count;i++){
      THTensor_set1d(ff,i,features_found[i]);
    }
  }
  if (fe != 0){
    for(i=0;i<count;i++){
      THTensor_set1d(fe,i,feature_errors[i]);
    }
  }
  // Deallocate points_cv
  cvFree(&points1_cv);
  cvFree(&points2_cv);
  cvReleaseImage( &pyrA );
  cvReleaseImage( &pyrB );
  cvReleaseImage( &grey1 );
  cvReleaseImage( &grey2);
  cvReleaseImage( &image1_ipl );
  cvReleaseImage( &image2_ipl );

  return 0;
}

//============================================================
static int l_cvCalcOpticalFlowPyrLK(lua_State *L) {
  // Get Tensor's Info
  THTensor * image1 = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * image2 = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * flow_x = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * flow_y = luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * points = luaT_checkudata(L, 5, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * image_out = luaT_checkudata(L, 6, luaT_checktypename2id(L, "torch.Tensor"));
  

  int count = 500;
  double quality = 0.01;
  double min_distance = 10;
  int win_size = 10;  

  // User values:
  if (lua_isnumber(L, 7)) {
    count = lua_tonumber(L, 7);
  }
  if (lua_isnumber(L, 8)) {
    quality = lua_tonumber(L, 8);
  }
  if (lua_isnumber(L, 9)) {
    min_distance = lua_tonumber(L, 9);
  }
  if (lua_isnumber(L, 10)) {
    win_size = lua_tonumber(L, 10);
  }

  CvSize dest_size = cvSize(image1->size[0], image1->size[1]);
  IplImage * image1_ipl = torch2opencv_8U(image1);
  IplImage * image2_ipl = torch2opencv_8U(image2);
  IplImage * image_out_ipl = torch2opencv_8U(image_out);


  IplImage * grey1 = cvCreateImage( dest_size, 8, 1 );
  IplImage * grey2 = cvCreateImage( dest_size, 8, 1 );

  cvCvtColor( image1_ipl, grey1, CV_BGR2GRAY );
  cvCvtColor( image2_ipl, grey2, CV_BGR2GRAY );
  CvPoint2D32f* points1_cv = 0;
  CvPoint2D32f* points2_cv = 0;


  IplImage* eig = cvCreateImage( dest_size, 32, 1 );
  IplImage* temp = cvCreateImage( dest_size, 32, 1 );

  // FIXME reuse points
  points1_cv = (CvPoint2D32f*)cvAlloc(count*sizeof(points1_cv[0]));
  points2_cv = (CvPoint2D32f*)cvAlloc(count*sizeof(points2_cv[0]));

  cvGoodFeaturesToTrack( grey1, eig, temp, points1_cv, &count,
			 quality, min_distance, 0, 3, 0, 0.04 );
  
  cvFindCornerSubPix( grey1, points1_cv, count,
		      cvSize(win_size,win_size), 
		      cvSize(-1,-1),
		      cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,
				     20,0.03));
  // Call Lucas Kanade algorithm
  char features_found[ count ];
  float feature_errors[ count ];
  CvSize pyr_sz = cvSize( image1_ipl->width+8, image1_ipl->height/3 );

  IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  
  cvCalcOpticalFlowPyrLK( grey1, grey2, 
			  pyrA, pyrB, 
			  points1_cv, points2_cv, 
			  count, 
			  cvSize( win_size, win_size ), 
			  5, features_found, feature_errors,
			  cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.3 ), 0 );
  // make image
  int i;
  for( i = 0; i < count; i++ ) {
    if (features_found[i] >0){
      CvPoint p0 = cvPoint( cvRound( points1_cv[i].x), 
			    cvRound( points1_cv[i].y));
      CvPoint p1 = cvPoint( cvRound( points2_cv[i].x), 
			    cvRound( points2_cv[i].y));
      cvLine( image_out_ipl, p0, p1, CV_RGB(255,0,0), 1, CV_AA, 0);
      //create the flow vectors to be compatible with the other
      //opticalFlows
      if (((p1.x > 0) && (p1.x < flow_x->size[0])) &&
	  ((p1.y > 0) && (p1.y < flow_x->size[1]))) {
	THTensor_set2d(flow_x,p1.x,p1.y,points1_cv[i].x - points2_cv[i].x);
	THTensor_set2d(flow_y,p1.x,p1.y,points1_cv[i].y - points2_cv[i].y);
      }
    }
  }
  
  // return results
  opencvPoints2torch_32F(points2_cv, count, points);
  opencv2torch_8U(image_out_ipl, image_out);

  // Deallocate points_cv
  cvFree(&points1_cv);
  cvFree(&points2_cv);
  cvReleaseImage( &eig );
  cvReleaseImage( &temp );
  cvReleaseImage( &pyrA );
  cvReleaseImage( &pyrB );
  cvReleaseImage( &grey1 );
  cvReleaseImage( &grey2);
  cvReleaseImage( &image1_ipl );
  cvReleaseImage( &image2_ipl );
  cvReleaseImage( &image_out_ipl );

  return 0;
}

//============================================================
// draws red flow lines on an image (for visualizing the flow)
static int l_cvDrawFlowlinesOnImage (lua_State *L) {
  THTensor * points1 = luaT_checkudata(L,1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * points2 = luaT_checkudata(L,2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * image   = luaT_checkudata(L,3, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * color   = luaT_checkudata(L,4, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * mask    = 0;
  int usemask = 0;
  if (!lua_isnil(L,5)){
    usemask = 1;
    mask = luaT_checkudata(L,5, luaT_checktypename2id(L, "torch.Tensor"));
  }
  IplImage * image_ipl = torch2opencv_8U(image);
  CvScalar color_cv = CV_RGB(THTensor_get1d(color,0),
			     THTensor_get1d(color,1),
			     THTensor_get1d(color,2));
  int count = points1->size[0];
  int i;
  for( i = 0; i < count; i++ ) {
    if ( !usemask || (THTensor_get1d(mask,i) > 0)){
      CvPoint p0 = cvPoint( cvRound( THTensor_get2d(points1,i,0)),
			    cvRound( THTensor_get2d(points1,i,1)));
      CvPoint p1 = cvPoint( cvRound( THTensor_get2d(points2,i,0)),
			    cvRound( THTensor_get2d(points2,i,1)));
      cvLine( image_ipl, p0, p1, color_cv, 2, CV_AA, 0);
    }
  }
  // return results
  opencv2torch_8U(image_ipl, image);
  cvReleaseImage( &image_ipl );
  return 0;
}


//============================================================
// OpticalFlow
// Works on torch.Tensors (double). All the conversions are
// done in C.
//
static int l_cvCalcOpticalFlow(lua_State *L) {
  // Get Tensor's Info
  THTensor * current = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * previous = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * velx = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.Tensor"));
  THTensor * vely = luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.Tensor"));

  // Generate IPL images
  IplImage * current_ipl = torch2opencv_8U(current);
  IplImage * previous_ipl = torch2opencv_8U(previous);
  IplImage * velx_ipl;
  IplImage * vely_ipl;

  // Default values
  int method = 1;
  int lagrangian = 1;
  int iterations = 5;
  CvSize blockSize = cvSize(7, 7);
  CvSize shiftSize = cvSize(20, 20);
  CvSize max_range = cvSize(20, 20);
  int usePrevious = 0;

  // User values:
  if (lua_isnumber(L, 5)) {
    method = lua_tonumber(L, 5);
  }

  // HS only:
  if (lua_isnumber(L, 6)) {
    lagrangian = lua_tonumber(L, 6);
  }
  if (lua_isnumber(L, 7)) {
    iterations = lua_tonumber(L, 7);
  }

  // BM+LK only:
  if (lua_isnumber(L, 6) && lua_isnumber(L, 7)) { 
    blockSize.width = lua_tonumber(L, 6); 
    blockSize.height = lua_tonumber(L, 7);
  }
  if (lua_isnumber(L, 8) && lua_isnumber(L, 9)) { 
    shiftSize.width = lua_tonumber(L, 8); 
    shiftSize.height = lua_tonumber(L, 9);
  }
  if (lua_isnumber(L, 10) && lua_isnumber(L, 11)) { 
    max_range.width = lua_tonumber(L, 10); 
    max_range.height = lua_tonumber(L, 11);
  }
  if (lua_isnumber(L, 12)) { 
    usePrevious = lua_tonumber(L, 12); 
  }  

  // Compute flow
  if (method == 1) 
    {
      // Alloc outputs
      CvSize osize = cvSize((previous_ipl->width-blockSize.width)/shiftSize.width,
                            (previous_ipl->height-blockSize.height)/shiftSize.height);

      // Use previous results
      if (usePrevious == 1) {
        velx_ipl = torch2opencv_32F(velx);
        vely_ipl = torch2opencv_32F(vely);
      } else {
        velx_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);
        vely_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);
      }

      // Cv Call
      cvCalcOpticalFlowBM(previous_ipl, current_ipl, blockSize, shiftSize, 
                          max_range, usePrevious, velx_ipl, vely_ipl);
    }
  else if (method == 2) 
    {
      // Alloc outputs
      CvSize osize = cvSize(previous_ipl->width, previous_ipl->height);

      velx_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);
      vely_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);

      // Cv Call
      cvCalcOpticalFlowLK(previous_ipl, current_ipl, blockSize, velx_ipl, vely_ipl);
    }
  else if (method == 3) 
    {
      // Alloc outputs
      CvSize osize = cvSize(previous_ipl->width, previous_ipl->height);

      // Use previous results
      if (usePrevious == 1) {
        velx_ipl = torch2opencv_32F(velx);
        vely_ipl = torch2opencv_32F(vely);
      } else {
        velx_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);
        vely_ipl = cvCreateImage(osize, IPL_DEPTH_32F, 1);
      }

      // Iteration criterion
      CvTermCriteria term = cvTermCriteria(CV_TERMCRIT_ITER, iterations, 0);

      // Cv Call
      cvCalcOpticalFlowHS(previous_ipl, current_ipl, usePrevious, velx_ipl, vely_ipl, 
                          lagrangian, term);
    }

  // return results
  opencv2torch_32F(velx_ipl, velx);
  opencv2torch_32F(vely_ipl, vely);

  // Deallocate IPL images
  cvReleaseImage(&previous_ipl);
  cvReleaseImage(&current_ipl);
  cvReleaseImage(&vely_ipl);
  cvReleaseImage(&velx_ipl);

  return 0;
}

//============================================================
// Other converters
// a bit redundant now, these two functions swap from RGB
// to BGR
//
IplImage * torchRGBtoOpenCVBGR(IplImage * source) {
  uchar * source_data; 
  uchar * dest_data;
  int source_step, dest_step;
  CvSize source_size, dest_size;
  
  // create destination image
  IplImage * dest = cvCreateImage(cvSize(source->width, source->height),
                                  IPL_DEPTH_8U, source->nChannels);

  // get pointer to raw data
  cvGetRawData(source, (uchar**)&source_data, &source_step, &source_size);
  cvGetRawData(dest, (uchar**)&dest_data, &dest_step, &dest_size);

  // copy
  int x, y, k;
  for (y=0; y<source->height; y++)
    for (x=0; x<source->width; x++)
      for (k=0; k<source->nChannels; k++) {
        dest_data[ y*dest_step + x*dest->nChannels + 2-k ]
          = source_data[ k*source->width*source->height + y*source->width + x ];
      }
  
  return dest;
}

void openCVBGRtoTorchRGB(IplImage * source, IplImage * dest) {
  uchar * source_data; 
  double * dest_data;
  int source_step, dest_step;
  CvSize source_size, dest_size;

  // get pointer to raw data
  cvGetRawData(source, (uchar**)&source_data, &source_step, &source_size);
  cvGetRawData(dest, (uchar**)&dest_data, &dest_step, &dest_size);

  // copy
  int x, y, k;
  for (y=0; y<source->height; y++)
    for (x=0; x<source->width; x++)
      for (k=0; k<source->nChannels; k++) {
        dest_data[ k*dest->width*dest->height + y*dest->width + x ]
          = (double)source_data[ y*source_step + x*source->nChannels + 2-k ] / 255.0;
      }

  // set this flag
  dest->dataOrder = 1;
}

//============================================================
// HaarDetectObjects
// Simple object detector based on haar features
//
static int l_cvHaarDetectObjects (lua_State *L) {
  // Generate IPL header from tensor
  THCharTensor * input = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.CharTensor"));
  IplImage * image = charImage(input);

  // Invert channels 
  IplImage * image_interleaved = torchRGBtoOpenCVBGR(image);

  // Get cascade path
  const char * path_to_cascade = luaL_checkstring(L, 2);;
  CvHaarClassifierCascade * cascade 
    = (CvHaarClassifierCascade *) cvLoad(path_to_cascade, 0, 0, 0);
  if (cascade == NULL) {
    perror("file doesnt exist, exiting");
    lua_pushnil(L);
    return 1;
  }

  /* if the flag is specified, down-scale the input image to get a
     performance boost w/o loosing quality (perhaps) */
  int i, scale = 1;
  IplImage* small_image = image_interleaved;
  int do_pyramids = 1;
  if( do_pyramids ) {
    small_image = cvCreateImage( cvSize(image_interleaved->width/2,image_interleaved->height/2),
                                 IPL_DEPTH_8U, 3 );
    cvPyrDown( image_interleaved, small_image, CV_GAUSSIAN_5x5 );
    scale = 2;
  }

  /* use the fastest variant */
  CvSeq* faces;
  CvMemStorage* storage = cvCreateMemStorage(0);
#if (CV_MINOR_VERSION >= 2)
  faces = cvHaarDetectObjects( small_image, cascade, storage, 1.2, 3, 0,
                               cvSize(30,30), cvSize(small_image->width,small_image->height) );
#else
  faces = cvHaarDetectObjects( small_image, cascade, storage, 1.2, 3, 0, cvSize(30,30));
#endif
  return 0;

  /* extract all the rectangles, and add them on the stack */
  lua_newtable(L);
  for( i = 0; i < faces->total; i++ ) {
    /* extract the rectanlges only */
    CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i );
    
    printf("face at (%d,%d)-(%d,%d)", 
           face_rect.x*scale,
           face_rect.y*scale,
           face_rect.x*scale + face_rect.width*scale,
           face_rect.y*scale + face_rect.height*scale );
    
    // and push result on the stack
    lua_pushnumber(L,i*4+1);
    lua_pushnumber(L,face_rect.x*scale);
    lua_settable(L,-3);
    lua_pushnumber(L,i*4+2);
    lua_pushnumber(L,face_rect.y*scale);
    lua_settable(L,-3);
    lua_pushnumber(L,i*4+3);
    lua_pushnumber(L,face_rect.width*scale);
    lua_settable(L,-3);
    lua_pushnumber(L,i*4+4);
    lua_pushnumber(L,face_rect.height*scale);
    lua_settable(L,-3);
  }
  
  // Cleanup
  if( small_image != image_interleaved ) cvReleaseImage( &small_image );
  cvReleaseMemStorage( &storage );
  cvReleaseHaarClassifierCascade( &cascade );
  cvReleaseImageHeader( &image );
  cvReleaseImage( &image_interleaved );

  return 1; // the table contains the results
}

//============================================================
// CaptureFromCAM
// wrapper around the Cv camera interface
//
static CvCapture *camera = 0;
static int l_cvCaptureFromCAM (lua_State *L) {
  // Get Tensor's Info
  THTensor * image_byte = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));

  // idx
  int camidx = 0;
  if (lua_isnumber(L, 2)) camidx = lua_tonumber(L, 2);

  // get camera
  if( camera == 0 ) {
    printf("opencv: starting capture on device %d\n", camidx);
    camera = cvCaptureFromCAM(camidx);
    if (!camera) perror("Could not initialize capturing...\n");
  }

  // grab frame
  IplImage *frame = NULL;
  frame = cvQueryFrame(camera);
  if (!frame) perror("Could not get frame...\n");

  // resize given tensor
  if (image_byte->size[1]!=frame->width 
      || image_byte->size[2]!=frame->height || image_byte->size[3]!=3) {
     THTensor_resize3d(image_byte,frame->width,frame->height,3);
  }

  // Generate IPL headers
  IplImage * current_image = doubleImage(image_byte);

  // copy this frame to torch format
  openCVBGRtoTorchRGB(frame, current_image);

  // Deallocate headers
  cvReleaseImageHeader(&current_image);
  
  return 0;
}

static int l_cvReleaseCAM (lua_State *L) {
  cvReleaseCapture( &camera );
  return 0;
}

//============================================================
// Register functions in LUA
//
static const struct luaL_reg opencv [] = {
  {"canny", l_cvCanny},
  {"sobel", l_cvSobel},
  {"captureFromCam", l_cvCaptureFromCAM},
  {"releaseCam", l_cvReleaseCAM},
  {"calcOpticalFlow", l_cvCalcOpticalFlow},
  {"haarDetectObjects", l_cvHaarDetectObjects},
  {"CornerHarris", l_cvCornerHarris},
  {"GoodFeaturesToTrack", l_cvGoodFeaturesToTrack},
  {"TrackPyrLK", l_cvTrackPyrLK},
  {"calcOpticalFlowPyrLK", l_cvCalcOpticalFlowPyrLK},
  {"drawFlowlinesOnImage", l_cvDrawFlowlinesOnImage},
  {NULL, NULL}  /* sentinel */
  //  {"HoG", l_cvhog},
};

int luaopen_libopencv (lua_State *L) {
  luaL_openlib(L, "libopencv", opencv, 0);
  return 1; 
}
