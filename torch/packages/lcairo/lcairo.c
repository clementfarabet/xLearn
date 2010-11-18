#include "TH.h"
#include "luaT.h"
 
#include <string.h>

#include "cairo.h"
#ifdef CAIRO_HAS_PDF_SURFACE
#include "cairo-pdf.h"
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#include "cairo-ps.h"
#endif

#include "window.h"
#include "lcairo.h"

static const void* torch_Tensor_id;

static lua_State *globalL=NULL;

static const void *tSurface_id = NULL;
static const void *tSurfaceData_id = NULL;
static const void *tImage_id   = NULL;
static const void *tWindow_id  = NULL;
static const void *tCairo_id   = NULL;
static const void *tPattern_id   = NULL;

#ifdef LCAIRO_USE_READLINE
#include "readline/readline.h"
int readline_callback(){
  /* printf("lcairo readline_callback\n"); */
  if (globalL){ 
    lua_State* L=globalL;
    lua_pushstring(L, "readline_callback");
    lua_rawget(L, LUA_GLOBALSINDEX);
    if (lua_isfunction(L,-1)) {
      if (0!=lua_pcall(L,0,0,0)) {
        printf("error:in readline callback: %s\n", lua_tostring(L, -1));
        lua_pop(L,1);
      }
    }
    else{
      lua_pop(L, 1);
    }
  }
  return 0;
}
#endif/* LCAIRO_USE_READLINE */

/* Cairo */

static int lcairo_cairo_create(lua_State *L){
 cairo_surface_t *psurf=luaT_checkudata(L,1,tSurface_id);
 cairo_t* p = cairo_create(psurf);
 cairo_select_font_face(p, "sans-serif", 
                        CAIRO_FONT_SLANT_NORMAL, 
                        CAIRO_FONT_WEIGHT_NORMAL);
 cairo_set_font_size(p, 10);
 luaT_pushudata(L, p, tCairo_id); 
 return 1;
}
static int lcairo_cairo_destroy(lua_State *L){
 cairo_t *p=luaT_checkudata(L,1,tCairo_id);
 cairo_destroy(p);
 lua_settop(L,1); 
 return 1;
}

#define LCAIRO_FUNC(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 cairo_##F(cr);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_1D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 cairo_##F(cr,d1);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_2D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 double d2=luaL_checknumber(L,3); \
 cairo_##F(cr,d1,d2);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_3D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 double d2=luaL_checknumber(L,3); \
 double d3=luaL_checknumber(L,4); \
 cairo_##F(cr,d1,d2,d3);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_4D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 double d2=luaL_checknumber(L,3); \
 double d3=luaL_checknumber(L,4); \
 double d4=luaL_checknumber(L,5); \
 cairo_##F(cr,d1,d2,d3,d4);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_5D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 double d2=luaL_checknumber(L,3); \
 double d3=luaL_checknumber(L,4); \
 double d4=luaL_checknumber(L,5); \
 double d5=luaL_checknumber(L,6); \
 cairo_##F(cr,d1,d2,d3,d4,d5);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_6D(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 double d1=luaL_checknumber(L,2); \
 double d2=luaL_checknumber(L,3); \
 double d3=luaL_checknumber(L,4); \
 double d4=luaL_checknumber(L,5); \
 double d5=luaL_checknumber(L,6); \
 double d6=luaL_checknumber(L,7); \
 cairo_##F(cr,d1,d2,d3,d4,d5,d6);  \
 lua_settop(L,1); \
 return 1;\
}
#define LCAIRO_FUNC_TXT(F) \
static int lcairo_cairo_##F(lua_State *L){ \
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); \
 const char *p=luaL_checkstring(L,2); \
 cairo_##F(cr,p);  \
 lua_settop(L,1); \
 return 1;\
}

LCAIRO_FUNC(stroke)
LCAIRO_FUNC(stroke_preserve)
LCAIRO_FUNC(save)
LCAIRO_FUNC(restore)
LCAIRO_FUNC(show_page)
LCAIRO_FUNC(fill)
LCAIRO_FUNC(paint)
LCAIRO_FUNC(fill_preserve)
LCAIRO_FUNC(clip)
LCAIRO_FUNC(reset_clip)
LCAIRO_FUNC(identity_matrix)

LCAIRO_FUNC_1D(set_line_width)
LCAIRO_FUNC_1D(set_font_size)
LCAIRO_FUNC_1D(rotate)
LCAIRO_FUNC_2D(move_to)
LCAIRO_FUNC_2D(line_to)
LCAIRO_FUNC_2D(translate)
LCAIRO_FUNC_2D(scale)
LCAIRO_FUNC_3D(set_source_rgb)
LCAIRO_FUNC_4D(set_source_rgba)
LCAIRO_FUNC_4D(rectangle)
LCAIRO_FUNC_5D(arc)
LCAIRO_FUNC_6D(curve_to)

LCAIRO_FUNC_TXT(show_text)

static int lcairo_cairo_set_source(lua_State *L){
 cairo_t         *cr=luaT_checkudata(L,1,tCairo_id);
 cairo_pattern_t *p =luaT_checkudata(L,2,tPattern_id);
 cairo_set_source(cr,p);
 lua_settop(L,1);
 return 1;
}

static int lcairo_cairo_get_source(lua_State *L){
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id);
 cairo_pattern_t* p=cairo_get_source(cr);
 luaT_pushudata(L, p, tPattern_id); 
 return 1;
}

static int lcairo_cairo_set_source_surface(lua_State *L){
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id);
 cairo_surface_t *surf=luaT_checkudata(L,2,tSurface_id);
 cairo_set_source_surface(cr,surf,0,0); 
 lua_settop(L,1);
 return 1;
}
static int lcairo_cairo_set_source_surface_xy(lua_State *L){
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id);
 cairo_surface_t *surf=luaT_checkudata(L,2,tSurface_id);
 double x=luaL_checknumber(L,3); \
 double y=luaL_checknumber(L,4); \
 cairo_set_source_surface(cr,surf,x,y); 
/* not required - maybe another day...
 printf("xy:%g,%g\n",x,y);
 cairo_matrix_t matrix;
 //cairo_pattern_t* pattern=cairo_get_source(cr);
 cairo_matrix_init_translate (&matrix, x,y);
 //cairo_matrix_init_scale (&matrix, 0.2,0.2);
 cairo_pattern_set_matrix (pattern, &matrix);
 cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
 cairo_set_source(cr,pattern);
 printf("%d\n",cairo_pattern_status(pattern));
 printf("pattern type:%d\n",cairo_pattern_get_type(cr));
*/
 lua_settop(L,1);
 return 1;
}

/* non - standard below... */

static int lcairo_cairo_set_dash(lua_State *L){
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id);
 double sep   =luaL_checknumber(L,2); 
 double offset=luaL_checknumber(L,3); 
 cairo_set_dash(cr,&sep,1,offset); 
 /* cairo_destroy(cr); what? */
 lua_settop(L,1);
 return 1;
}
static int lcairo_cairo_set_not_dash(lua_State *L){
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id);
 cairo_set_dash(cr,NULL,0,0); 
 /* cairo_destroy(cr); what? */
 lua_settop(L,1);
 return 1;
}


static int lcairo_cairo_set_matrix(lua_State *L){ 
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); 
 cairo_matrix_t m;
 m.xx=luaL_checknumber(L,2); 
 m.xy==luaL_checknumber(L,3); 
 m.yx=luaL_checknumber(L,4); 
 m.yy=luaL_checknumber(L,5); 
 m.x0=luaL_checknumber(L,6); 
 m.y0=luaL_checknumber(L,7); 
 cairo_set_matrix(cr,&m);  
 lua_settop(L,1); 
 return 1;
}

static int lcairo_cairo_get_matrix(lua_State *L){ 
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); 
 cairo_matrix_t m;
 cairo_get_matrix(cr,&m);
 lua_settop(L,1); 
 lua_pushnumber(L,m.xx);
 lua_pushnumber(L,m.yx);
 lua_pushnumber(L,m.xy);
 lua_pushnumber(L,m.yy);
 lua_pushnumber(L,m.x0);
 lua_pushnumber(L,m.y0);
 return 6;
}


static int lcairo_cairo_get_text_width(lua_State *L){ 
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); 
 const char *p=luaL_checkstring(L,2); 
 cairo_text_extents_t t;
 cairo_text_extents(cr,p,&t);
 lua_settop(L,1); 
 lua_pushnumber(L,t.x_advance);
 return 1;
}
static int lcairo_cairo_get_text_height(lua_State *L){ 
 cairo_t *cr=luaT_checkudata(L,1,tCairo_id); 
 const char *p=luaL_checkstring(L,2); 
 cairo_text_extents_t t;
 cairo_text_extents(cr,p,&t);
 lua_settop(L,1); 
 lua_pushnumber(L,t.height);
 return 1;
}

/* cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE); */



/* cairo pattern */
static int lcairo_pattern_set_filter(lua_State *L){
 cairo_pattern_t *p=luaT_checkudata(L,1,tPattern_id);

 /* enum... */
 cairo_pattern_set_filter(p, CAIRO_FILTER_NEAREST);

 lua_settop(L,1);
 return 1;
}


/* cairo surface */
static int lcairo_image_surface_get_width(lua_State *L){
 cairo_surface_t *p=luaT_checkudata(L,1,tSurface_id);
 int v=cairo_image_surface_get_width(p); 
 lua_settop(L,1); 
 lua_pushnumber(L,v);
 return 1;
}
static int lcairo_image_surface_get_height(lua_State *L){
 cairo_surface_t *p=luaT_checkudata(L,1,tSurface_id);
 int v=cairo_image_surface_get_height(p); 
 lua_settop(L,1); 
 lua_pushnumber(L,v);
 return 1;
}

static int lcairo_surface_write_to_png(lua_State *L){
 cairo_surface_t *p=luaT_checkudata(L,1,tSurface_id);
 const char *filename=luaL_checkstring(L,2);
 cairo_status_t status=cairo_surface_write_to_png(p,filename); 
 if (status!=CAIRO_STATUS_SUCCESS)
  luaL_error(L,"couldn't save file: %s\n",filename);
 lua_settop(L,1);
 return 1;
}

static int lcairo_surface_destroy(lua_State *L){
 cairo_surface_t *p=luaT_checkudata(L,1,tSurface_id);
 cairo_surface_destroy(p); 
 lua_settop(L,1);
 return 1;
}

/* Window */

static int lcairo_create_window(lua_State *L){
  int w=luaL_checkint(L,1);
  int h=luaL_checkint(L,2);
  const char *name=luaL_optstring(L,3,"Torch5.1");
  tWindow* p=tWindow_create(L,w,h,name);//"Torch5.1");
  luaT_pushudata(L, p, tWindow_id); 
  return 1;
}

static int lcairo_window_set_surface(lua_State *L){
  tWindow            *p=luaT_checkudata(L,1,tWindow_id);
  cairo_surface_t *surf=luaT_checkudata(L,2,tSurface_id);
  p->surf = surf;
  tWindow_set_surface(p,surf);
  lua_settop(L,1);
  return 1;
}


static int lcairo_window_refresh(lua_State *L){
 tWindow *p=luaT_checkudata(L,1,tWindow_id);
 tWindow_refresh(p); 
 lua_settop(L,1);
 return 1;
}

static int lcairo_window_valid(lua_State *L){
 tWindow *p=luaT_checkudata(L,1,tWindow_id);
 lua_settop(L,1);
 lua_pushnumber(L,p->valid?1:0);
 return 1;
}
/*
static int lcairo_window_resize(lua_State *L){
 tWindow *p=luaT_checkudata(L,1,tWindow_id);
 int w=luaL_checkint(L,2);
 int h=luaL_checkint(L,3);
 tWindow_resize_display(p,w,h);
 lua_settop(L,1);
 return 1;
}
*/
static int lcairo_window_messages_next(lua_State *L){
 tWindow *p=luaT_checkudata(L,1,tWindow_id);
 tMsg m;
 static char cmsg[2000];

 /* work out how to send nice table back */
 lua_settop(L,1);
 m = tWindow_message_next(p);

 switch(m.id){
   case TMSG_NULL:       sprintf(cmsg,"null"); break;
   case TMSG_RESIZE:     sprintf(cmsg,"resize\t%d\t%d",m.width,m.height); break;
   case TMSG_PAINT:      sprintf(cmsg,"paint"); break;
   case TMSG_MOUSE_MOVE: sprintf(cmsg,"mouse_move\t%d\t%d",m.x,m.y); break;
   default:
     /* printf("unhandled window message id:%d\n",m.id); */
     sprintf(cmsg,"ignore"); 
   break;
 };
 lua_pushstring(L,cmsg);
 return 1;
}

static int lcairo_window_destroy(lua_State *L){
 tWindow *p=luaT_checkudata(L,1,tWindow_id);
 tWindow_destroy(p);
 lua_settop(L,1);
 return 1;
}

/* PDF,PS */
static int lcairo_create_pdf_surface(lua_State *L){
#ifdef CAIRO_HAS_PDF_SURFACE
  const char *filename=luaL_checkstring(L,1);
  double w=luaL_checknumber(L,2);
  double h=luaL_checknumber(L,3);
  cairo_surface_t *p = (cairo_surface_t*)cairo_pdf_surface_create(filename,w,h);
  luaT_pushudata(L, p, tSurface_id); 
  return 1;
#else
  luaL_error(L,"Installed Cairo does not support PDF");
  return 0;
#endif
}
static int lcairo_create_ps_surface(lua_State *L){
#ifdef CAIRO_HAS_PS_SURFACE
  const char *filename=luaL_checkstring(L,1);
  double w=luaL_checknumber(L,2);
  double h=luaL_checknumber(L,3);
  cairo_surface_t *p = (cairo_surface_t*)cairo_ps_surface_create(filename,w,h);
  luaT_pushudata(L, p, tSurface_id); 
  return 1;
#else
  luaL_error(L,"Installed Cairo does not support PS");
  return 0;
#endif
}

#ifdef CAIRO_HAS_SVG_SURFACE
#endif

/* Image */
static int lcairo_create_image_from_png(lua_State *L){
  const char *filename=luaL_checkstring(L,1);
  tImage *p=(tImage*)luaT_alloc(L, sizeof(tImage));
  p->data = NULL; /* won't have this */
  p->surf = cairo_image_surface_create_from_png(filename);
  if (cairo_surface_status(p->surf)!=CAIRO_STATUS_SUCCESS){
    printf("image_create_from_png: ERROR: couldn't load %s\n",filename);
    cairo_surface_destroy(p->surf);
    return 0;
 //   lua_settop(L,1);
 //   return -1;
  }
  luaT_pushudata(L, p, tImage_id); 
  return 1;
}

static int lcairo_create_image(lua_State *L){
  int w=luaL_checkint(L,1);
  int h=luaL_checkint(L,2);
  tImage *p=(tImage*)luaT_alloc(L, sizeof(tImage));
  p->data = luaT_alloc(L,w*h*4);
  memset(p->data,0xff,w*h*4);
  p->surf = cairo_image_surface_create_for_data(p->data,CAIRO_FORMAT_ARGB32,w,h,4*w);
  luaT_pushudata(L, p, tImage_id); 
  return 1;
}
static int lcairo_image_get_surface(lua_State *L){
  tImage *p=luaT_checkudata(L,1,tImage_id);
  luaT_pushudata(L, p->surf, tSurface_id); 
  return 1;
}
static int lcairo_image_get_surface_data(lua_State *L){
  tImage *p=luaT_checkudata(L,1,tImage_id);
  luaT_pushudata(L, p->data, tSurfaceData_id); 
  return 1;
}
static int lcairo_image_destroy(lua_State *L){
  tImage *p=luaT_checkudata(L,1,tImage_id);
  luaT_free(L,p->data);
  cairo_surface_destroy(p->surf);
  luaT_free(L,p);
  lua_settop(L,1);
  return 1;
}



/* Pixel Surface */

void _T2_to_surface( double *src,     unsigned char *pdst,
                     int src_stride0, int src_stride1,
                     int width,   int height  ){
  int i,j;
  unsigned int num_pixels=0;
  for(j=0;j<height;j++) {
    unsigned char *p = pdst + (j*4*width); 
    for(i=0;i<width;i++) {
      unsigned char c;
      c = (int)(src[i*src_stride0+j*src_stride1]*255.0);
      *p++= c; *p++= c; *p++= c;
      *p++= 255;
      num_pixels++;
    }
  }  
}

void _surface_to_T2( unsigned char *src, double *dst, 
                     int dst_stride0, int dst_stride1,
                     int width,   int height  ){
  int i,j;
  for(j=0;j<height;j++) {
    unsigned char *p = src + (j*4*width); /* +dx*4 later */
    for(i=0;i<width;i++) {
      double val = 0.0;
      val += *p++;
      val += *p++;
      val += *p++;
              p++;
      val /= (255.0*3);
      dst[i*dst_stride0+j*dst_stride1] = val;
    }
  }


}

void _surface_to_T3( unsigned char *src, double *dst, 
                     long dst_stride0, long dst_stride1, long dst_stride2,
                     int width ,int height, int depth ){
  int i,j;
  for(j=0;j<height;j++) {
    unsigned char *p = src + (j*4*width); /* +dx*4 later */
    for(i=0;i<width;i++) {
       dst[i*dst_stride0+j*dst_stride1+2*dst_stride2] = (*p++)/255.0;
       dst[i*dst_stride0+j*dst_stride1+1*dst_stride2] = (*p++)/255.0;
       dst[i*dst_stride0+j*dst_stride1+0*dst_stride2] = (*p++)/255.0;
       if (depth==4) {
        dst[i*dst_stride0+j*dst_stride1+3*dst_stride2] = (*p++)/255.0;
       }
       else
         p++; /* ignore alpha channel */
    }
  }
}

void _T3_to_surface( double *src, unsigned char *dst,
                     long src_stride0, long src_stride1, long src_stride2,
                     int width ,int height, int depth  ){
  int i,j;
  for(j=0;j<height;j++) {
    unsigned char *p = dst + (j*4*width); /* +dx*4 later */
    for(i=0;i<width;i++) {
      *p++ = (int)(src[i*src_stride0+j*src_stride1+2*src_stride2]*255.0);
      *p++ = (int)(src[i*src_stride0+j*src_stride1+1*src_stride2]*255.0);
      *p++ = (int)(src[i*src_stride0+j*src_stride1+0*src_stride2]*255.0);
      if (depth==4) {
        *p++ = (int)(src[i*src_stride0+j*src_stride1+3*src_stride2]*255.0);
      }
      else
        *p++ = 255;
    } 
  } 
}

static int lcairo_THTensor_to_surface( lua_State *L){
  THTensor     *Tsrc = luaT_checkudata(L,1,torch_Tensor_id);
  unsigned char *dst = luaT_checkudata(L,2,tSurfaceData_id);
  if ( Tsrc->nDimension==3 ){
    _T3_to_surface( THTensor_dataPtr(Tsrc), dst,
                     Tsrc->stride[0], Tsrc->stride[1], Tsrc->stride[2],
                       Tsrc->size[0],   Tsrc->size[1],   Tsrc->size[2]  );

  }else if (Tsrc->nDimension==2 ){
    _T2_to_surface( THTensor_dataPtr(Tsrc), dst,
                     Tsrc->stride[0], Tsrc->stride[1],
                       Tsrc->size[0],   Tsrc->size[1]  );
  }else{
    luaL_error(L,"nDimension not 2 or 3");
  }
  return 1;
}
static int lcairo_surface_to_THTensor( lua_State *L ){

  unsigned char *src = luaT_checkudata(L,1,tSurfaceData_id);
  THTensor     *Tdst = luaT_checkudata(L,2,torch_Tensor_id);
  if ( Tdst->nDimension==3 ){
    _surface_to_T3( src,THTensor_dataPtr(Tdst), 
                     Tdst->stride[0], Tdst->stride[1], Tdst->stride[2],
                       Tdst->size[0],   Tdst->size[1],   Tdst->size[2]  );

  }else if (Tdst->nDimension==2 ){
    _surface_to_T2( src, THTensor_dataPtr(Tdst),
                     Tdst->stride[0], Tdst->stride[1],
                       Tdst->size[0],   Tdst->size[1]  );
  }else{
    luaL_error(L,"nDimension not 2 or 3");
  }

  return 1;
}


static const struct luaL_Reg lcairo__ [] = {

  /* cairo */

  {"cairo_create",         lcairo_cairo_create },
  /* no args */
  {"cairo_stroke",         lcairo_cairo_stroke },
  {"cairo_stroke_preserve",lcairo_cairo_stroke_preserve },
  {"cairo_save",           lcairo_cairo_save },
  {"cairo_restore",        lcairo_cairo_restore },
  {"cairo_show_page",      lcairo_cairo_show_page },
  {"cairo_fill",           lcairo_cairo_fill},
  {"cairo_paint",          lcairo_cairo_paint},
  {"cairo_fill_preserve",  lcairo_cairo_fill_preserve},
  {"cairo_clip",           lcairo_cairo_clip},
  {"cairo_reset_clip",     lcairo_cairo_reset_clip},
  {"cairo_identity_matrix",lcairo_cairo_identity_matrix},
  /* 1 double */
  {"cairo_set_line_width", lcairo_cairo_set_line_width },
  {"cairo_set_font_size",  lcairo_cairo_set_font_size },
  {"cairo_rotate",         lcairo_cairo_rotate },
  /* 2 doubles */
  {"cairo_move_to",        lcairo_cairo_move_to },
  {"cairo_line_to",        lcairo_cairo_line_to },
  {"cairo_translate",      lcairo_cairo_translate },
  {"cairo_scale",          lcairo_cairo_scale  },
  /* 3 doubles */
  {"cairo_set_source_rgb", lcairo_cairo_set_source_rgb },
  /* 4 doubles */
  {"cairo_set_source_rgba",lcairo_cairo_set_source_rgba },
  {"cairo_rectangle",      lcairo_cairo_rectangle },
  /* 5 doubles */
  {"cairo_arc",            lcairo_cairo_arc},
  /* 6 doubles! */
  {"cairo_curve_to",       lcairo_cairo_curve_to },
  /* 1 char * */
  {"cairo_show_text",      lcairo_cairo_show_text },
  /* special args.. */
  {"cairo_get_source"        ,lcairo_cairo_get_source },
  {"cairo_set_source",        lcairo_cairo_set_source },
  {"cairo_set_source_surface",lcairo_cairo_set_source_surface },
  {"cairo_set_source_surface_xy",lcairo_cairo_set_source_surface_xy },

  /* cairo custom  */
  /* these should be replace with better multiple args + table ret */
  {"cairo_set_dash",       lcairo_cairo_set_dash },
  {"cairo_set_not_dash",   lcairo_cairo_set_not_dash },
  {"cairo_set_matrix",       lcairo_cairo_set_matrix },
  {"cairo_get_matrix",       lcairo_cairo_get_matrix },
  {"cairo_get_text_width", lcairo_cairo_get_text_width },
  {"cairo_get_text_height", lcairo_cairo_get_text_height },

  /* pattern  */
  {"pattern_set_filter",    lcairo_pattern_set_filter },


  /* surface */

  {"surface_destroy",     lcairo_surface_destroy },
  {"surface_write_to_png",lcairo_surface_write_to_png },
  {"image_surface_get_width",   lcairo_image_surface_get_width },
  {"image_surface_get_height",  lcairo_image_surface_get_height },
  {"create_pdf_surface",  lcairo_create_pdf_surface },
  {"create_ps_surface",   lcairo_create_ps_surface },
  {"create_image",	  lcairo_create_image },
  {"create_image_from_png",lcairo_create_image_from_png },
  {"image_get_surface",	  lcairo_image_get_surface },
  {"image_get_surface_data",	  lcairo_image_get_surface_data },

  {"THTensor_to_surface", lcairo_THTensor_to_surface },
  {"surface_to_THTensor", lcairo_surface_to_THTensor },

  /* window */

  {"create_window",       lcairo_create_window },
  {"window_set_surface", lcairo_window_set_surface },
  {"window_refresh",      lcairo_window_refresh },
  {"window_valid",        lcairo_window_valid },
/*  {"window_resize",        lcairo_window_resize }, */
  {"window_messages_next", lcairo_window_messages_next },
  {"window_destroy",      lcairo_window_destroy },

  {NULL, NULL}
};



void lcairo_init(lua_State *L)
{  
  globalL=L;
  luaL_register(L, NULL, lcairo__);

  /* external classes used */

  torch_Tensor_id = luaT_checktypename2id(L, "torch.Tensor");

  /* cairo */

  tCairo_id  =luaT_newmetatable(L, "lcairo.tCairo",   NULL, NULL, lcairo_cairo_destroy, NULL);

  /* surfaces */

  /* tSurface and tSurfaceData are only ever refeneces - no garbage collection */
  tSurface_id    =luaT_newmetatable(L, "lcairo.tSurface", NULL, NULL, NULL, NULL);
  tSurfaceData_id=luaT_newmetatable(L, "lcairo.tSurfaceData", NULL, NULL, NULL, NULL);

  tImage_id  =luaT_newmetatable(L, "lcairo.tImage",   NULL, NULL, lcairo_image_destroy, NULL);
  tWindow_id =luaT_newmetatable(L, "lcairo.tWindow",  NULL, NULL, lcairo_window_destroy, NULL);

  /* patterns */

  /* May need tPattern and tPatternCreated:child tPattern to differentiate for garbage */
  tPattern_id =luaT_newmetatable(L, "lcairo.tPattern",  NULL, NULL, NULL, NULL);

#ifdef LCAIRO_USE_READLINE
  rl_event_hook=readline_callback;
#endif /* LCAIRO_USE_READLINE */
}
