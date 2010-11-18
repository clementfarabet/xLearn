/*
 * Torch5 mpeg2 module....
 * hacked from mpeg2dec by Iain Melvin <iainmelvin@gmail.com>

 * mpeg2dec.c
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include "mpeg2decoder.h"

tFrame *frame_new(int _width, int _height ){
  tFrame *p=(tFrame*)malloc(sizeof(tFrame));
  p->width=_width; 
  p->height=_height;
  p->pnext=NULL;
  p->rgb32=(uint8_t*)malloc(p->width*p->height*4*sizeof(uint8_t));
  if (p->rgb32==NULL)
    exit(1);
  return p;
}
void frame_delete(tFrame *p){
  free(p->rgb32);
  free(p);
}

tOutput *output_new(){
  tOutput *p=(tOutput*)malloc(sizeof(tOutput));
  p->frame_number=0;
  p->current_frame=NULL;
  p->convert = mpeg2convert_rgb32;
  return p;
}

void output_delete(tOutput *p){
  while(p->current_frame){
    output_nextframe(p); // clear stored frames
  };
  free(p);
}

void output_nextframe(tOutput *p){
  if (p->current_frame!=NULL){
    tFrame* pold = p->current_frame;
    p->current_frame=p->current_frame->pnext;
    frame_delete(pold);
  }
}

void output_draw( tOutput *p,uint8_t * const * buf, void * id)
{
  // make new frame
  tFrame *pnew=frame_new(p->width,p->height);

  // add to decoded frames list
  if (p->current_frame==NULL){
    p->current_frame = pnew;
  }else{
    tFrame *pf = p->current_frame;
    while (pf->pnext!=NULL) 
      pf=pf->pnext;
    pf->pnext=pnew;
  }
  memcpy (pnew->rgb32, buf[0], p->width*p->height*4*sizeof(uint8_t));

static int test_save_out_first_frame_r=0;
  if (test_save_out_first_frame_r==1){
    test_save_out_first_frame_r=0;
    FILE *fp=fopen("test_frame.txt","w+");
    int x=0;int y=0;int rgba=0;
    for (y=0;y<p->height;y++){
      for (x=0;x<p->width;x++){
        for (rgba=0;rgba<1;rgba++){
          fprintf(fp,"%d\t",buf[0][(y*p->width*4)+(x*4+rgba)]);
      } }
      fprintf(fp,"\n");
    } fclose(fp);
  }
}

int output_setup ( tOutput *p, 
                   unsigned int _width,
                   unsigned int _height, 
                   unsigned int chroma_width,
                   unsigned int chroma_height )
{
  p->width=_width;
  p->height=_height;
  return 0;
}

void decoder_decode_mpeg2 ( tDecoder *p, uint8_t * current, uint8_t * end)
{
    const mpeg2_info_t * info;
    mpeg2_state_t state;

    mpeg2_buffer (p->mpeg2dec, current, end);
    p->total_offset += end - current;

    info = mpeg2_info (p->mpeg2dec);
    while (1) {
      state = mpeg2_parse (p->mpeg2dec);
      switch (state) {
        case STATE_BUFFER:
        return;
      case STATE_SEQUENCE:
        /* might set nb fbuf, convert format, stride */
        /* might set fbufs */
        if (output_setup (p->output,
            info->sequence->width,
            info->sequence->height,
            info->sequence->chroma_width,
            info->sequence->chroma_height)) {
          fprintf (stderr, "display setup failed\n");
          exit (1);
        }
        if (p->output->convert ){
          //printf("converting\n");
          if (mpeg2_convert (p->mpeg2dec, p->output->convert, NULL)) {
            fprintf (stderr, "color conversion setup failed\n");
            exit (1);
          }
        }
        mpeg2_skip (p->mpeg2dec, 0 ); //(output->draw == NULL));
        break;
      case STATE_PICTURE:
        /* might skip */
        /* might set fbuf */
      break;
      case STATE_SLICE:
      case STATE_END:
      case STATE_INVALID_END:
        /* draw current picture */
        /* might free frame buffer */
        if (info->display_fbuf) {
          output_draw(p->output,
	             info->display_fbuf->buf,
                     info->display_fbuf->id);
         //print_fps (0);
        }
      break;
      default:
      break;
    }
  }
}



#define DEMUX_PAYLOAD_START 1
int decoder_demux (tDecoder *p, uint8_t * buf, uint8_t * end, int flags)
{
    static int mpeg1_skip_table[16] = {
	0, 0, 4, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    ///
     // the demuxer keeps some state between calls:
     // if "state" = DEMUX_HEADER, then "head_buf" contains the first
     //     "bytes" bytes from some header.
     // if "state" == DEMUX_DATA, then we need to copy "bytes" bytes
     //     of ES data before the next header.
     // if "state" == DEMUX_SKIP, then we need to skip "bytes" bytes
     //     of data before the next header.
     //
     // NEEDBYTES makes sure we have the requested number of bytes for a
     // header. If we dont, it copies what we have into head_buf and returns,
     // so that when we come back with more data we finish decoding this header.
     //
     // DONEBYTES updates "buf" to point after the header we just parsed.
     ///

#define DEMUX_HEADER 0
#define DEMUX_DATA 1
#define DEMUX_SKIP 2
    p->demux_state = DEMUX_SKIP;
    p->demux_state_bytes = 0;
   

    uint8_t * header;
    int bytes;
    int len;

#define NEEDBYTES(x)						\
    do {							\
	int missing;						\
								\
	missing = (x) - bytes;					\
	if (missing > 0) {					\
	    if (header == p->head_buf) {				\
		if (missing <= end - buf) {			\
		    memcpy (header + bytes, buf, missing);	\
		    buf += missing;				\
		    bytes = (x);				\
		} else {					\
		    memcpy (header + bytes, buf, end - buf);	\
		    p->demux_state_bytes = bytes + end - buf;		\
		    return 0;					\
		}						\
	    } else {						\
		memcpy (p->head_buf, header, bytes);		\
		p->demux_state = DEMUX_HEADER;				\
		p->demux_state_bytes = bytes;				\
		return 0;					\
	    }							\
	}							\
    } while (0)

#define DONEBYTES(x)		\
    do {			\
	if (header != p->head_buf)	\
	    buf = header + (x);	\
    } while (0)

    if (flags & DEMUX_PAYLOAD_START)
	goto payload_start;
    switch (p->demux_state) {
    case DEMUX_HEADER:
	if (p->demux_state_bytes > 0) {
	    header = p->head_buf;
	    bytes = p->demux_state_bytes;
	    goto continue_header;
	}
	break;
    case DEMUX_DATA:
	if (p->demux_pid || (p->demux_state_bytes > end - buf)) {
	    decoder_decode_mpeg2 (p,buf, end);
	    p->demux_state_bytes -= end - buf;
	    return 0;
	}
	decoder_decode_mpeg2 (p,buf, buf + p->demux_state_bytes);
	buf += p->demux_state_bytes;
	break;
    case DEMUX_SKIP:
	if (p->demux_pid || (p->demux_state_bytes > end - buf)) {
	    p->demux_state_bytes -= end - buf;
	    return 0;
	}
	buf += p->demux_state_bytes;
	break;
    }

    while (1) {
	if (p->demux_pid) {
	    p->demux_state = DEMUX_SKIP;
	    return 0;
	}
    payload_start:
	header = buf;
	bytes = end - buf;
    continue_header:
	NEEDBYTES (4);
	if (header[0] || header[1] || (header[2] != 1)) {
	    if (p->demux_pid) {
		p->demux_state = DEMUX_SKIP;
		return 0;
	    } else if (header != p->head_buf) {
		buf++;
		goto payload_start;
	    } else {
		header[0] = header[1];
		header[1] = header[2];
		header[2] = header[3];
		bytes = 3;
		goto continue_header;
	    }
	}
	if (p->demux_pid) {
	    if ((header[3] >= 0xe0) && (header[3] <= 0xef))
		goto pes;
	    fprintf (stderr, "bad stream id %x\n", header[3]);
	    exit (1);
	}
	switch (header[3]) {
	case 0xb9:	// program end code 
	    // DONEBYTES (4); 
	    // break;         
	    return 1;
	case 0xba:	// pack header 
	    NEEDBYTES (5);
	    if ((header[4] & 0xc0) == 0x40) {	// mpeg2 
		NEEDBYTES (14);
		len = 14 + (header[13] & 7);
		NEEDBYTES (len);
		DONEBYTES (len);
		// header points to the mpeg2 pack header 
	    } else if ((header[4] & 0xf0) == 0x20) {	// mpeg1 
		NEEDBYTES (12);
		DONEBYTES (12);
		// header points to the mpeg1 pack header 
	    } else {
		fprintf (stderr, "weird pack header\n");
		DONEBYTES (5);
	    }
	    break;
	default:
	    if (header[3] == p->demux_track) {
	    pes:
		NEEDBYTES (7);
		if ((header[6] & 0xc0) == 0x80) {	// mpeg2 
		    NEEDBYTES (9);
		    len = 9 + header[8];
		    NEEDBYTES (len);
		    // header points to the mpeg2 pes header 
		    if (header[7] & 0x80) {
			uint32_t pts, dts;

			pts = (((header[9] >> 1) << 30) |
			       (header[10] << 22) | ((header[11] >> 1) << 15) |
			       (header[12] << 7) | (header[13] >> 1));
			dts = (!(header[7] & 0x40) ? pts :
			       (((header[14] >> 1) << 30) |
				(header[15] << 22) |
				((header[16] >> 1) << 15) |
				(header[17] << 7) | (header[18] >> 1)));
			mpeg2_tag_picture (p->mpeg2dec, pts, dts);
		    }
		} else {	// mpeg1 
		    int len_skip;
		    uint8_t * ptsbuf;

		    len = 7;
		    while (header[len - 1] == 0xff) {
			len++;
			NEEDBYTES (len);
			if (len > 23) {
			    fprintf (stderr, "too much stuffing\n");
			    break;
			}
		    }
		    if ((header[len - 1] & 0xc0) == 0x40) {
			len += 2;
			NEEDBYTES (len);
		    }
		    len_skip = len;
		    len += mpeg1_skip_table[header[len - 1] >> 4];
		    NEEDBYTES (len);
		    // header points to the mpeg1 pes header 
		    ptsbuf = header + len_skip;
		    if ((ptsbuf[-1] & 0xe0) == 0x20) {
			uint32_t pts, dts;

			pts = (((ptsbuf[-1] >> 1) << 30) |
			       (ptsbuf[0] << 22) | ((ptsbuf[1] >> 1) << 15) |
			       (ptsbuf[2] << 7) | (ptsbuf[3] >> 1));
			dts = (((ptsbuf[-1] & 0xf0) != 0x30) ? pts :
			       (((ptsbuf[4] >> 1) << 30) |
				(ptsbuf[5] << 22) | ((ptsbuf[6] >> 1) << 15) |
				(ptsbuf[7] << 7) | (ptsbuf[18] >> 1)));
			mpeg2_tag_picture (p->mpeg2dec, pts, dts);
		    }
		}
		DONEBYTES (len);
		bytes = 6 + (header[4] << 8) + header[5] - len;
		if (p->demux_pid || (bytes > end - buf)) {
		    decoder_decode_mpeg2 (p,buf, end);
		    p->demux_state = DEMUX_DATA;
		    p->demux_state_bytes = bytes - (end - buf);
		    return 0;
		} else if (bytes > 0) {
		    decoder_decode_mpeg2 (p,buf, buf + bytes);
		    buf += bytes;
		}
	    } else if (header[3] < 0xb9) {
		fprintf (stderr,
			 "looks like a video stream, not system stream\n");
		DONEBYTES (4);
	    } else {
		NEEDBYTES (6);
		DONEBYTES (6);
		bytes = (header[4] << 8) + header[5];
		if (bytes > end - buf) {
		    p->demux_state = DEMUX_SKIP;
		    p->demux_state_bytes = bytes - (end - buf);
		    return 0;
		}
		buf += bytes;
	    }
	}
    }
}


tFrame* decoder_getframe(tDecoder *p)
{
    //do we have a frame already?
    if (p->output->current_frame){
	output_nextframe(p->output);
	if (p->output->current_frame)
	  return p->output->current_frame;
    }

    // need to decode some more..
    uint8_t * end;
    do {
	end = p->buffer + fread (p->buffer, 1, p->buffer_size, p->in_file);
	if (p->demux_track!=0){
		if (decoder_demux (p,p->buffer, end, 0))
	    		break;	// hit program_end_code 
 	}else{
		//video only file
		decoder_decode_mpeg2 (p,p->buffer, end);
	}
   } while (end == p->buffer + p->buffer_size && !p->sigint && p->output->current_frame==NULL );

   return p->output->current_frame; // NULL signifies end
}

tDecoder *decoder_new(const char *filename){
  return decoder_new2( filename, 's', -1 );
}
tDecoder* decoder_new2(const char *filename, char _type, int optarg){
  // default _type = s _optarg=-1

  tDecoder *p=(tDecoder*)malloc(sizeof(tDecoder));

  p->type=_type;
  p->mpeg2dec = mpeg2_init();
  if (p->mpeg2dec ==NULL)
    exit(1);

  p->buffer_size = 4096;
  p->buffer = (uint8_t *) malloc (p->buffer_size);
  if (p->buffer==NULL)
    exit(1);
  
  p->demux_track=0;
  p->demux_pid=0;
  p->total_offset=0;
  p->sigint=0;
  //verbose=0;
      
  p->output = output_new();

  switch (p->type){
    case 's':
      if (optarg != -1) {
        p->demux_track = optarg;
        if (p->demux_track < 0xe0)
            p->demux_track += 0xe0;
        if (p->demux_track < 0xe0 || p->demux_track > 0xef ) {
            printf("Invalid track number: %d\n", optarg);
            exit(1);
        }
      }
    break;
    case 't': // TRANSPORT (digital TV)
      printf("Transport streams not supported - Programs and pure video only\n");
      exit(1);
    break;

    case 'p': //pva - rare
      printf("PVA format not supported - Programs and pure video only\n");
      exit(1);
    break;
  };

  p->in_file = fopen (filename, "rb");
  if (!p->in_file) {
      printf("loading file could not open file:%s errno:%d",filename,errno);
      exit(1);
  }
  return p; 
}

int decoder_newFrame(tDecoder *p, THTensor *x){

  //get frame
  tFrame* frame = decoder_getframe(p);
  if (frame==NULL)
    return 0;

  if (x->size[1]!=frame->width 
   || x->size[2]!=frame->height
   || x->size[3]!=3){
     THTensor_resize3d(x,frame->width,frame->height,3);
  }

  uint8_t *src=frame->rgb32;
  int src_stride0= 4;
  int src_stride1= frame->width*4;
  int src_stride2= 1;

  double *dst= x->storage->data;
  int dst_stride0= x->stride[0];
  int dst_stride1= x->stride[1]; 
  int dst_stride2= x->stride[2]; 

  int i,j;
  for(j = 0; j < frame->height; j++){
    uint8_t *psrc=&src[j*src_stride1];
    double  *pdst=&dst[j*dst_stride1];
    for(i = 0; i < frame->width; i++){
      double *pdst_blk=&pdst[i*dst_stride0];
      pdst_blk[2*dst_stride2] = (*psrc++)/255.0;
      pdst_blk[1*dst_stride2] = (*psrc++)/255.0;
      pdst_blk[0*dst_stride2] = (*psrc++)/255.0;
      psrc++; // 32bit
    } 
  }  
  return 1;
}

void decoder_delete(tDecoder *p){
    mpeg2_close (p->mpeg2dec);
    fclose (p->in_file);
    output_delete(p->output);
    free (p->buffer);
    free(p);
}

/*
DLL_EXPORT int luaopen_libmpeg2(lua_State *L)
{
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_GLOBALSINDEX, "mpeg2");

  sparselab_stat_init(L);

  return 1;
}
*/

