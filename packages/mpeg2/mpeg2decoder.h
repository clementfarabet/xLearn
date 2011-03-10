#ifndef _MPEG2_DECODER_HDEF
#define _MPEG2_DECODER_HDEF

#include <inttypes.h>
#include <stdio.h>
#include "mpeg2.h"
#include "mpeg2convert.h"

#include "THTensor.h"


typedef struct _tFrame{
  int     width;
  int     height;
  uint8_t *rgb32;	
  struct _tFrame *pnext;
} tFrame;

tFrame* frame_new(int _width,int _height);
void    frame_delete(tFrame* p);

typedef struct {
  // decoded frames..
  int width;
  int height;
  int frame_number;
  tFrame* 	current_frame;
int (* convert)(int stage, void * id,
		const struct mpeg2_sequence_s * sequence,
	     	int stride, uint32_t accel, void * arg,
	     	struct mpeg2_convert_init_s * result);
} tOutput;


typedef struct {
  char 	type;
  FILE*	in_file;

  int      buffer_size;
  uint8_t* buffer;
  int 	 total_offset;
  int 	 sigint;

  int demux_state;
  int demux_state_bytes;
  uint8_t head_buf[264];

  int 	demux_track;
  int 	demux_pid;
  int 	demux_pva;

  tOutput*	output; 
  mpeg2dec_t* 	mpeg2dec;

} tDecoder;

tOutput *output_new();
void output_delete(tOutput *p);
void output_nextframe(tOutput *p);
void output_draw (tOutput *p, uint8_t * const * buf, void * id);
int  output_setup(tOutput *p, unsigned int _width, unsigned int _height, unsigned int chroma_width, unsigned int chroma_height );


tDecoder *decoder_new      (const char *filename);
tDecoder *decoder_new2     (const char *filename, char type, int optarg);
void decoder_delete        (tDecoder *p);
void decoder_decode_mpeg2  (tDecoder *p, uint8_t * current, uint8_t * end);
int  decoder_demux         (tDecoder *p, uint8_t * buf, uint8_t * end, int flags);
//	void decoder_no_accel(); {mpeg2_accel (0);}
tFrame* decoder_getFrame(tDecoder *p);
int    decoder_newFrame(tDecoder *p, THTensor *x);


#endif// _MPEG2_DECODER_HDEF
