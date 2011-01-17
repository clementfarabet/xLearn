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
  {NULL, NULL}  /* sentinel */
};

int luaopen_libopencv (lua_State *L) {
  luaL_openlib(L, "libopencv", opencv, 0);
  return 1; 
}
