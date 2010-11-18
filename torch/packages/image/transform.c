#undef TAPI
#define TAPI __declspec(dllimport)

#include "luaT.h"
#include "TH.h"

static const void* torch_Tensor_id = NULL;

static void _op_validate( lua_State *L,  THTensor *Tsrc, THTensor *Tdst){

	long src_depth = 1;
	long dst_depth = 1;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

	if(Tdst->nDimension == 3) dst_depth =  Tdst->size[2];
        if(Tsrc->nDimension == 3) src_depth =  Tsrc->size[2];

	if( (Tdst->nDimension==3 && ( src_depth!=dst_depth)) ||
		(Tdst->nDimension!=Tsrc->nDimension)
		)
		luaL_error(L, "image.scale: src and dst depths do not match"); 	


	if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
		luaL_error(L, "image.scale: src and dst depths do not match"); 	
} 	

static long _op_stride( THTensor *T,int i){
	if (i<2) return T->stride[i];
	if(T->nDimension == 3)
		return T->stride[2];
	return 0;
}

static long _op_depth( THTensor *T){
	if(T->nDimension == 3) return T->size[2]; /* rgb or rgba */
	return 1; /* greyscale */
}

static void scale_rowcol( 	THTensor *Tsrc,
			THTensor *Tdst, 
			long src_start, 
			long dst_start, 
			long src_stride,
			long dst_stride,
			long src_len,
			long dst_len ){ 

	double *src= THTensor_dataPtr(Tsrc);
	double *dst= THTensor_dataPtr(Tdst);

	if ( dst_len > src_len ){
	  long di;
    double si_f;
    long si_i;
    double scale = (double)(src_len - 1) / (dst_len - 1);
    for( di = 0; di < dst_len - 1; di++ )
    {
      long dst_pos = dst_start + di*dst_stride;
      si_f = di * scale; si_i = (long)si_f; si_f -= si_i;
      
      dst[dst_pos] = (1 - si_f) * src[ src_start + si_i       * src_stride ] +
        si_f * src[ src_start + (si_i + 1) * src_stride ];
	  }
    
	  dst[ dst_start + (dst_len - 1) * dst_stride ] = 
      src[ src_start + (src_len - 1) * src_stride ];
    
	} 
	else if ( dst_len < src_len )
	{
	  long di;
	  long si0_i = 0; double si0_f = 0;
	  long si1_i; double si1_f;
	  long si;
	  double scale = (double)src_len / dst_len;
	  double acc, n;
	  for( di = 0; di < dst_len; di++ )
	  {
	     si1_f = (di + 1) * scale; si1_i = (long)si1_f; si1_f -= si1_i;
	     acc = (1 - si0_f) * src[ src_start + si0_i * src_stride ];
	     n = 1 - si0_f;
	     for( si = si0_i + 1; si < si1_i; si++ )
	     {
	       acc += src[ src_start + si * src_stride ];
	       n += 1;
	     }
	     if( si1_i < src_len )
	     {
	       acc += si1_f * src[ src_start + si1_i*src_stride ];
	       n += si1_f;
	     }
	     dst[ dst_start + di*dst_stride ] = acc / n;
	     si0_i = si1_i; si0_f = si1_f;
	   }
 	}
 	else
 	{
 	  long i;
 	  for( i = 0; i < dst_len; i++ )
 	    dst[ dst_start + i*dst_stride ] = src[ src_start + i*src_stride ];
 	}

}
	

static int scaleBilinear(lua_State *L){

  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor_id);
  THTensor *Ttmp;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height;
  long src_stride0, src_stride1, src_stride2, src_width, src_height;
  long tmp_stride0, tmp_stride1, tmp_stride2, tmp_width, tmp_height;
  long i, j, k;

	_op_validate(L, Tsrc,Tdst);


  Ttmp = THTensor_newWithSize2d(Tdst->size[0], Tsrc->size[1]);/* , 0); _op_depth(Tsrc)); */

	dst_stride0= _op_stride(Tdst,0);
	dst_stride1= _op_stride(Tdst,1);
	dst_stride2= _op_stride(Tdst,2);
	src_stride0= _op_stride(Tsrc,0);
	src_stride1= _op_stride(Tsrc,1);
	src_stride2= _op_stride(Tsrc,2);
	tmp_stride0= _op_stride(Ttmp,0);
	tmp_stride1= _op_stride(Ttmp,1);
	tmp_stride2= _op_stride(Ttmp,2);
	dst_width=   Tdst->size[0];
	dst_height=  Tdst->size[1];
	src_width=   Tsrc->size[0];
	src_height=  Tsrc->size[1];
	tmp_width=   Ttmp->size[0];
	tmp_height=  Ttmp->size[1];

for(k=0;k<_op_depth(Tsrc);k++)
{
	/* compress/expand rows first */
	for(j = 0; j < src_height; j++) {
		scale_rowcol(	Tsrc,
				Ttmp,
				0*src_stride0+j*src_stride1+k*src_stride2,
				0*tmp_stride0+j*tmp_stride1+k*tmp_stride2,
				src_stride0,
				tmp_stride0,
				src_width,
				tmp_width );

	}

	/* then columns */
	for(i = 0; i < dst_width; i++) {
		scale_rowcol(	Ttmp,
				Tdst,
				i*tmp_stride0+0*tmp_stride1+k*tmp_stride2,
				i*dst_stride0+0*dst_stride1+k*dst_stride2,
				tmp_stride1,
				dst_stride1,
				tmp_height,
				dst_height );
	}

}
	THTensor_free(Ttmp);

/*
	double *src= Tsrc->storage->data+ Tsrc->storageOffset;
	double *dst= THTensor_dataPtr(Tdst);

	
	for(int j = 0; j < dst_height; j++) {
	for(int i = 0; i < dst_width; i++) {
		double val = 0.0;
		int ii=(int) (0.5+((double)i)*scx);
		int jj=(int) (0.5+((double)j)*scy);
		if(ii>src_width-1) ii=src_width-1;
		if(jj>src_height-1) jj=src_height-1;
		for(int k=0;k<_op_depth(Tsrc);k++)
		{
			val=src[ii*src_stride0+jj*src_stride1+k*src_stride2];
	 	  	dst[i*dst_stride0+j*dst_stride1+k*dst_stride2] = val;
		}
	}} 
*/

  return 0;
}

static int scaleSimple(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor_id);
  double *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  double scx, scy;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

	src= THTensor_dataPtr(Tsrc);
	dst= THTensor_dataPtr(Tdst);
 	
	dst_stride0= Tdst->stride[0];
	dst_stride1= Tdst->stride[1]; 
	dst_stride2 = 0;
	dst_width=   Tdst->size[0];
	dst_height=  Tdst->size[1];
	dst_depth =  0;
	if(Tdst->nDimension == 3)
  	{
    	   dst_stride2 = Tdst->stride[2]; 
	   dst_depth = Tdst->size[2];
	}

	src_stride0= Tsrc->stride[0];
	src_stride1= Tsrc->stride[1]; 
	src_stride2 = 0;
	src_width=   Tsrc->size[0];
	src_height=  Tsrc->size[1];
	src_depth = 0;

        if(Tsrc->nDimension == 3)
        {
    	  src_stride2 = Tsrc->stride[2];
    	  src_depth =  Tsrc->size[2];
	}

	if( (Tdst->nDimension==3 && ( src_depth!=dst_depth)) ||
		(Tdst->nDimension!=Tsrc->nDimension)
		)
	{
	printf("image.scale:%d,%d,%ld,%ld\n",Tsrc->nDimension,Tdst->nDimension,src_depth,dst_depth);
		luaL_error(L, "image.scale: src and dst depths do not match"); 	
        }         


	if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
		luaL_error(L, "image.scale: src and dst depths do not match"); 	


	/* printf("%d,%d -> %d,%d\n",src_width,src_height,dst_width,dst_height); */
	scx=((double)src_width)/((double)dst_width);
	scy=((double)src_height)/((double)dst_height);


	for(j = 0; j < dst_height; j++) {

		for(i = 0; i < dst_width; i++) {
			double val = 0.0;
						
			long ii=(long) (0.5+((double)i)*scx);
			long jj=(long) (0.5+((double)j)*scy);
			if(ii>src_width-1) ii=src_width-1;
			if(jj>src_height-1) jj=src_height-1;

			if(Tsrc->nDimension==2)
			{
		 	 val=src[ii*src_stride0+jj*src_stride1];
		 	 dst[i*dst_stride0+j*dst_stride1] = val;
			}
			else	
			{ 
			 for(k=0;k<src_depth;k++)
			 {
			  val=src[ii*src_stride0+jj*src_stride1+k*src_stride2];
		 	  dst[i*dst_stride0+j*dst_stride1+k*dst_stride2] = val;
			 }
			}
		}
	} 

  return 0;
}



static int rotate(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor_id);
  double theta = luaL_checknumber(L, 3);
  double *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  double xc, yc;
	double id,jd; 
	long ii,jj;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");


	src= THTensor_dataPtr(Tsrc);
	dst= THTensor_dataPtr(Tdst);

	dst_stride0= Tdst->stride[0];
	dst_stride1= Tdst->stride[1]; 
	dst_stride2 = 0;
	dst_width=   Tdst->size[0];
	dst_height=  Tdst->size[1];
	dst_depth = 0;
	if(Tdst->nDimension == 3)
  	{
    	   dst_stride2 = Tdst->stride[2]; 
	   dst_depth = Tdst->size[2];
	}

	src_stride0= Tsrc->stride[0];
	src_stride1= Tsrc->stride[1]; 
	src_stride2 = 0;
	src_width=   Tsrc->size[0];
	src_height=  Tsrc->size[1];
	src_depth = 0;
        if(Tsrc->nDimension == 3)
        {
    	  src_stride2 = Tsrc->stride[2];
    	  src_depth =  Tsrc->size[2];
	}

	if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) )
		luaL_error(L, "image.rotate: src and dst depths do not match");

	if( (Tsrc->nDimension!=Tdst->nDimension) )
		luaL_error(L, "image.rotate: src and dst depths do not match"); 

	xc=src_width/2.0; 
	yc=src_height/2.0;

	for(j = 0; j < dst_height; j++) {
		jd=j;
		for(i = 0; i < dst_width; i++) {
			double val = -1;
			id= i;
			
			ii=(long)( cos(theta)*(id-xc)-sin(theta)*(jd-yc) );
			jj=(long)( cos(theta)*(jd-yc)+sin(theta)*(id-xc) ); 
			ii+=(long) xc; jj+=(long) yc;
	
			/* rotated corners are blank */
			if(ii>src_width-1) val=0;
			if(jj>src_height-1) val=0;
			if(ii<0) val=0;
			if(jj<0) val=0;

			if(Tsrc->nDimension==2)
			{
			  if(val==-1) 
			    val=src[ii*src_stride0+jj*src_stride1];
		 	  dst[i*dst_stride0+j*dst_stride1] = val;
			}
			else	
			{ 
			 int do_copy=0; if(val==-1) do_copy=1;
			 for(k=0;k<src_depth;k++)
			 {
			   if(do_copy)
			    val=src[ii*src_stride0+jj*src_stride1+k*src_stride2];
		 	   dst[i*dst_stride0+j*dst_stride1+k*dst_stride2] = val;
			 }
		 	}


		
		}
	}
  
  return 0;
}

static int cropNoScale(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor_id);
  long startx = luaL_checklong(L, 3);
  long starty = luaL_checklong(L, 4);
  double *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

	src= THTensor_dataPtr(Tsrc);
	dst= THTensor_dataPtr(Tdst);

	dst_stride0= Tdst->stride[0];
	dst_stride1= Tdst->stride[1]; 
	dst_stride2 = 0;
	dst_width=   Tdst->size[0];
	dst_height=  Tdst->size[1];
	dst_depth = 0;
	if(Tdst->nDimension == 3)
  	{
    	   dst_stride2 = Tdst->stride[2]; 
	   dst_depth = Tdst->size[2];
	}

	src_stride0= Tsrc->stride[0];
	src_stride1= Tsrc->stride[1]; 
	src_stride2 = 0;
	src_width=   Tsrc->size[0];
	src_height=  Tsrc->size[1];
	src_depth = 0;
        if(Tsrc->nDimension == 3)
        {
    	  src_stride2 = Tsrc->stride[2];
    	  src_depth =  Tsrc->size[2];
	}
 	

	if( startx<0 || starty<0 || (startx+dst_width>src_width) || (starty+dst_height>src_height))
		luaL_error(L, "image.crop: crop goes outside bounds of src"); 	

	if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
		luaL_error(L, "image.crop: src and dst depths do not match"); 	

	for(j = 0; j < dst_height; j++) {

		for(i = 0; i < dst_width; i++) {
			double val = 0.0;
						
			long ii=i+startx;
			long jj=j+starty;
	
			if(Tsrc->nDimension==2)
			{
		 	 val=src[ii*src_stride0+jj*src_stride1];
		 	 dst[i*dst_stride0+j*dst_stride1] = val;
			}
			else	
			{ 
			 for(k=0;k<src_depth;k++)
			 {
			  val=src[ii*src_stride0+jj*src_stride1+k*src_stride2];
		 	  dst[i*dst_stride0+j*dst_stride1+k*dst_stride2] = val;
			 }
			}
		}
	}  
  return 0;
}


static int translate(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor_id);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor_id);
  long shiftx = luaL_checklong(L, 3);
  long shifty = luaL_checklong(L, 4);
  double *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

	src= THTensor_dataPtr(Tsrc);
	dst= THTensor_dataPtr(Tdst);

	dst_stride0= Tdst->stride[0];
	dst_stride1= Tdst->stride[1]; 
	dst_stride2 = 0;
	dst_width=   Tdst->size[0];
	dst_height=  Tdst->size[1];
	dst_depth = 0;
	if(Tdst->nDimension == 3)
  	{
    	   dst_stride2 = Tdst->stride[2]; 
	   dst_depth = Tdst->size[2];
	}

	src_stride0= Tsrc->stride[0];
	src_stride1= Tsrc->stride[1]; 
	src_stride2 = 0;
	src_width=   Tsrc->size[0];
	src_height=  Tsrc->size[1];
	src_depth = 0;
        if(Tsrc->nDimension == 3)
        {
    	  src_stride2 = Tsrc->stride[2];
    	  src_depth =  Tsrc->size[2];
	}
 	
	if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
		luaL_error(L, "image.crop: src and dst depths do not match"); 	

	for(j = 0; j < src_height; j++) {

		for(i = 0; i < src_width; i++) {
			double val = 0.0;
						
			long ii=i+shiftx;
			long jj=j+shifty;

			if(ii<dst_width && jj<dst_height)  
      /* check it's within destination bounds, else crop */
			{	
			if(Tsrc->nDimension==2)
			{
		 	 val=src[i*src_stride0+j*src_stride1];
		 	 dst[ii*dst_stride0+jj*dst_stride1] = val;
			}
			else	
			{ 
			 for(k=0;k<src_depth;k++)
			 {
			  val=src[i*src_stride0+j*src_stride1+k*src_stride2];
		 	  dst[ii*dst_stride0+jj*dst_stride1+k*dst_stride2] = val;
			 }
			}
			}
		}
	}  
	return 0;
}


static const struct luaL_Reg image_transform__ [] = {
  {"scaleSimple", scaleSimple},
  {"scaleBilinear", scaleBilinear},
  {"rotate", rotate},
  {"translate", translate},
  {"cropNoScale", cropNoScale},
  {NULL, NULL}
};

void image_transform_init(lua_State *L)
{
  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");
  luaL_register(L, NULL, image_transform__);
}
