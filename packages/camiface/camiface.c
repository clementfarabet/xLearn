//==============================================================================
// File: sys
//
// Description: A couple of useful functions accesible from Lua as sys.*
//              This code is hard or impossible to code in Lua... and nice to 
//              have there !!
//
// Created: February 10, 2010, 12:31AM
//
// Author: Clement Farabet // clement.farabet@gmail.com
//==============================================================================

#include <dirent.h>
#include <errno.h>

#include <luaT.h>
#include <TH.h>

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

// To load this lib in LUA:
// require 'sys'

// Data struct used to represent the shared segment
struct video_buffer {
  char request_frame;
  char frame_ready;
  char bytes_per_pixel;
  char dump_to_file;
  int width;
  int height;
  unsigned char data[];
};

static volatile struct video_buffer * buffer = NULL; 
static int shared_mem_connected = 0;
static key_t shmem_key;
static int shmem_id;  

// To exec in lua: sys.connectSharedMemory('path')
static int l_getSharedFrame (lua_State *L) {
  // Shared Mem Info
  const char *path = luaL_checkstring(L, 1);
  int dump_to_file = 0;

  // Dump ?
  if (lua_isboolean(L, 3)) dump_to_file = lua_toboolean(L, 3);

  // Get Tensor's Info
  THCharTensor * tensor8 = NULL; 
  THTensor * tensor64 = NULL;
  int tensor_bits;
  if (luaT_isudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"))) {
    tensor64 = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.Tensor"));
    tensor_bits = 64;
  } else if (luaT_isudata(L, 2, luaT_checktypename2id(L, "torch.CharTensor"))) {
    tensor8 = luaT_toudata(L, 2, luaT_checktypename2id(L, "torch.CharTensor"));
    tensor_bits = 8;
  } else {
    perror("getFrame: tensor should be double or char");
    lua_pushnil(L);  /* return nil and ... */
    return 1;    
  }

  // Shared Mem Size + dim check
  int buffer_size;
  if (tensor_bits == 8) {
    buffer_size = tensor8->size[0] * tensor8->size[1] * 4; // RGB
    // Check nb of dims
    if (tensor8->nDimension != 2 && tensor8->nDimension != 3) {
      printf("getFrame T8: tensor must have 2 or 3 dimensions\n");
      lua_pushnil(L);
      return 1;
    } 
  } else {
    buffer_size = tensor64->size[0] * tensor64->size[1] * 4; // RGB    
    // Check nb of dims
    if (tensor64->nDimension != 2 && tensor64->nDimension != 3) {
      printf("getFrame T64: tensor must have 2 or 3 dimensions\n");
      lua_pushnil(L);
      return 1;
    } 
  }

  // Do that only once
  if (shared_mem_connected == 0) {
    // We use a shared memory segment to make the pixels available to external software
    if ((shmem_key = ftok(path, 'A')) == -1) {
      perror("ftok couldnt get the shared mem descriptor");
      lua_pushnil(L);  /* return nil and ... */
      return 1;
    }
    
    // Sync to the segment
    if((shmem_id = shmget(shmem_key, buffer_size + 16, 0644 | IPC_CREAT)) == -1) {
      perror("shmget couldnt sync the shared mem segment");
      lua_pushnil(L);  /* return nil and ... */
      return 1;
    }

    // and link data to the segment
    buffer = (struct video_buffer *)shmat(shmem_id, (void *)0, 0);

    // Only once !
    printf("buffer connected to shared mem\n");
    shared_mem_connected = 1;
  }
  
  // make a frame request
  buffer->dump_to_file = dump_to_file;
  buffer->request_frame = 1;
  while (buffer->request_frame); // request received
  while (!buffer->frame_ready) sleep(0.01); // wait for frame to be ready

  // Fill tensor's storage with shared mem data
  long i0, i1, i2; 
  if (tensor_bits == 64) {
    if (tensor64->nDimension == 2) {  // GREEN Chanel
      for (i1=0; i1<tensor64->size[1]; i1++) {  
        for (i0=0; i0<tensor64->size[0]; i0++) {
          double pix = ((double)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+1]) / 256;
          THTensor_set2d(tensor64, i0, i1, pix);
        }
      }
    } else if (tensor64->nDimension == 3) {  // GREEN Chanel
      if (tensor64->size[2] == 1) {
        for (i1=0; i1<tensor64->size[1]; i1++) {
          for (i0=0; i0<tensor64->size[0]; i0++) {
            double pix = ((double)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+1]) / 256; 
            THTensor_set3d(tensor64, i0, i1, 0, pix);
          }
        }
      } else if (tensor64->size[2] == 3) {   // RGB Chanels
        for (i1=0; i1<tensor64->size[1]; i1++) {
          for (i0=0; i0<tensor64->size[0]; i0++) {
            for (i2=0; i2<tensor64->size[2]; i2++) {
              double pix = ((double)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+i2]) / 256; 
              THTensor_set3d(tensor64, i0, i1, i2, pix);
            }
          }      
        }
      }
    }
  } else if (tensor_bits == 8) {
    if (tensor8->nDimension == 2) {  // GREEN Chanel
      for (i1=0; i1<tensor8->size[1]; i1++) {  
        for (i0=0; i0<tensor8->size[0]; i0++) {
          unsigned char pix = (unsigned char)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+1]/2;
          THCharTensor_set2d(tensor8, i0, i1, pix);
        }
      }
    } else if (tensor8->nDimension == 3) {  // GREEN Chanel
      if (tensor8->size[2] == 1) {
        for (i1=0; i1<tensor8->size[1]; i1++) {
          for (i0=0; i0<tensor8->size[0]; i0++) {
            unsigned char pix = (unsigned char)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+1]/2; 
            THCharTensor_set3d(tensor8, i0, i1, 0, pix);
          }
        }
      } else if (tensor8->size[2] == 3) {   // RGB Chanels
        for (i1=0; i1<tensor8->size[1]; i1++) {
          for (i0=0; i0<tensor8->size[0]; i0++) {
            for (i2=0; i2<tensor8->size[2]; i2++) {
              unsigned char pix = (unsigned char)buffer->data[(i1*buffer->width+i0)*buffer->bytes_per_pixel+i2]/2; 
              THCharTensor_set3d(tensor8, i0, i1, i2, pix);
            }
          }      
        }
      }
    }
  }
  
  // Reset that guy for next time (not really useful)
  buffer->dump_to_file = 0;
  return 0; 
}

static int l_forkProcess (lua_State *L) {
  const char *exec_cmd = luaL_checkstring(L, 1);
  
  // forget children
  signal(SIGCHLD, SIG_IGN);

  // Fork and exec external process
  pid_t pID = fork();
  if (pID == 0) {
    execlp (exec_cmd, exec_cmd, (char *)0);
  } else if (pID > 0) {
    printf("Started child process with pID %d\n", pID);
  }

  // Return the ret code
  lua_pushnumber(L, (int)pID);
  return 1;
}

static int l_killProcess (lua_State *L) {
  pid_t pID = (pid_t)luaL_checknumber(L, 1);
  printf("Killing process with pID = %d\n", pID);
  kill(pID, SIGINT);
  return 0;
}

static int l_nonBlockingRead (lua_State *L) {
  int flags, flags_saved;
  flags_saved = fcntl(0, F_GETFL, 0);
  flags = flags_saved | O_NONBLOCK;
  fcntl(0, F_SETFL, flags);
  char c = getchar();
  fcntl(0, F_SETFL, flags_saved);
  // return it
  lua_pushnumber(L, c);
  return 1;
}


// Register functions
static const struct luaL_reg camiface [] = {
  {"getSharedFrame", l_getSharedFrame},
  {"forkProcess", l_forkProcess},
  {"killProcess", l_killProcess},
  {"read", l_nonBlockingRead},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libcamiface (lua_State *L) {
  luaL_openlib(L, "libcamiface", camiface, 0);
  return 1; 
}
