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

// To load this lib in LUA:
// require 'libopencv'

// Conversion helper
// this function just creates an IPL header to describe the tensor
// (no image allocation is done here)
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


static int l_cvCalcOpticalFlowBM (lua_State *L) {
  // Get Tensor's Info
  THCharTensor * current = luaT_checkudata(L, 1, luaT_checktypename2id(L, "torch.CharTensor"));
  THCharTensor * previous = luaT_checkudata(L, 2, luaT_checktypename2id(L, "torch.CharTensor"));
  THFloatTensor * velx = luaT_checkudata(L, 3, luaT_checktypename2id(L, "torch.FloatTensor"));
  THFloatTensor * vely = luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.FloatTensor"));

  // Generate IPL headers
  IplImage * current_ipl = charImage(current);
  IplImage * previous_ipl = charImage(previous);
  IplImage * velx_ipl = floatImage(velx);
  IplImage * vely_ipl = floatImage(vely);
  
  // Thresholds with default values
  CvSize blockSize = cvSize(7, 7);
  CvSize shiftSize = cvSize(20, 20);
  CvSize max_range = cvSize(20, 20);
  int usePrevious = 0;
  if (lua_isnumber(L, 5) && lua_isnumber(L, 6)) { 
    blockSize.width = lua_tonumber(L, 3); 
    blockSize.width = lua_tonumber(L, 4);
  }
  if (lua_isnumber(L, 7) && lua_isnumber(L, 8)) { 
    shiftSize.width = lua_tonumber(L, 5); 
    shiftSize.width = lua_tonumber(L, 6);
  }
  if (lua_isnumber(L, 9) && lua_isnumber(L, 10)) { 
    shiftSize.width = lua_tonumber(L, 7); 
    shiftSize.width = lua_tonumber(L, 8);
  }
  if (lua_isnumber(L, 11)) { 
    usePrevious = lua_tonumber(L, 9); 
  }  

  // Simple call to CV function
  cvCalcOpticalFlowBM(previous_ipl, current_ipl, blockSize, shiftSize, 
                      max_range, usePrevious, velx_ipl, vely_ipl);

  // Deallocate headers
  cvReleaseImageHeader(&previous_ipl);
  cvReleaseImageHeader(&current_ipl);
  cvReleaseImageHeader(&vely_ipl);
  cvReleaseImageHeader(&velx_ipl);

  return 0;
}



/* Merge channels from input, to generate an interleaved image. */
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


/* unmerge channels from input, to generate an torch image. */
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


/* takes image filename and cascade path from the command line */
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

  //printf("dumping frame\n");
  //cvSaveImage("1_image_dump.png", small_image, NULL);

  /* use the fastest variant */
  CvSeq* faces;
  CvMemStorage* storage = cvCreateMemStorage(0);
  faces = cvHaarDetectObjects( small_image, cascade, storage, 1.2, 3, 0, cvSize(30,30) );
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


// Register functions in LUA
static const struct luaL_reg opencv [] = {
  {"canny", l_cvCanny},
  {"sobel", l_cvSobel},
  {"captureFromCam", l_cvCaptureFromCAM},
  {"releaseCam", l_cvReleaseCAM},
  {"calcOpticalFlow", l_cvCalcOpticalFlowBM},
  {"haarDetectObjects", l_cvHaarDetectObjects},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libopencv (lua_State *L) {
  luaL_openlib(L, "libopencv", opencv, 0);
  return 1; 
}
