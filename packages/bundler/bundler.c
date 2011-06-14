/*
  + This is a wrapper for bundler -- the structure from motion code
  used in the Rome in a day project - */

// To load this lib in LUA:
// require 'libbundler'

#include <luaT.h>
#include <TH.h>

#include "matrix.h"
#include "sfm.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


/** ------------------------------------------------------------------
 ** Lua wrapper: sfm
 **/

static int sfm_l(lua_State *L) {

  /* -----------------------------------------------------------------
   * check arguments

   * For 6 degree of freedom cameras and 3D points we have N = 6m +
   * 3n unknowns

   */
  // from the stack
  int num_cameras = 1 ;
  if (lua_isnumber(L,1)) { 
    num_cameras = lua_tonumber(L,1); 
  } else { 
    THError("num_cameras should be an int"); 
  }
  int num_pts = 1;
  if (lua_isnumber(L,2)) { 
    num_pts = lua_tonumber(L,2); 
  } else { 
    THError("num_pts should be an int"); 
  }
  int num_projections = 1;
  if (lua_isnumber(L,3)) { 
    num_projections = lua_tonumber(L,3); 
  } else { 
    THError("num_projections should be an int"); 
  }
  THCharTensor *vmask_th = (THCharTensor *)luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.CharTensor"));
  THTensor *proj_th = (THTensor *)luaT_checkudata(L, 5, luaT_checktypename2id(L, "torch.Tensor"));

  /* number of images (starting from the 1st) whose parameters should
   * not be modified.  All A_ij (see below) with j<mcon are assumed to
   * be zero
   */
  int ncons =  0; 

 /* visibility mask: vmask[i, j]=1 if point i visible in image j, 0
    otherwise. nxm */
  char *vmask = (char *)vmask_th->storage->data;

   int i,j;
   /*
     for (i = 0; i < num_pts; i++){
     for (j = 0; j < num_cameras; j++){
       printf("%d ",vmask[(j * num_pts) + i]);
     }
     printf("\n");
   }
   */
     
  /* projections are the points in 2d image space.  Each point in 3d
   * can have multiple projections in different cameras.  Need to
   * figure out how to correctly index this.
  */
  double *projections = (double *)proj_th->storage->data; 
  /*
  for (i = 0; i < num_projections; i++){
    printf("[%d,%d](%f,%f)\n",2*i,2*i+1,
	   projections[2*i],projections[(2*i)+1]);
  }
  */

  /* Should the focal length be estimated?  if no use 6 euclidean
     contraints for the cameras, if yes use 7 contraints 
  */
  int est_focal_length = 1;  

  /* Is the focal length constant for all cameras? */
  int const_focal_length = 0; 
  /* Apply undistortion? */
  int undistort = 0; 
  /* Are the camera centers explicit? */
  int explicit_camera_centers = 0; 

  /* initial parameter of the cameras p0: (a1, ...,).  aj are the
   *  image j parameters, size m*cnp
   */
  camera_params_t init_camera_params[num_cameras];

  /* initial estimate of position of points in 3D (size n*pnp).  I
   * need to figure out how to do the initial placement of the points
   * into 3D.
   */
  v3_t init_pts[num_pts];   
  for (i=0;i<num_pts;i++){
    Vx(init_pts[i]) = 0.0;
    Vy(init_pts[i]) = 0.0;
    Vz(init_pts[i]) = 0.0;
  }
  int use_constraints = 1;
  int use_point_constraints = 1;
  
  /* n*3 vector of constraints */
  v3_t pt_constraints[num_pts]; 
  /* single weight for all pt_constraints */
  double pt_constraint_weight = 1; 
  /* if fix_points = 1 only solve for camera (motion) */
  int fix_points = 0; 
  int optimize_for_fisheye = 0;
  /* one of the opts[2] // 0.0; // 1.0e-10; // 1.0e-15; I:
   * minim. options [\mu, \epsilon1, \epsilon2, \epsilon3,
   * \epsilon4]. Respectively the scale factor for initial \mu, stoping
   * thresholds for ||J^T e||_inf, ||dp||_2, ||e||_2 and
   * (||e||_2-||e_new||_2)/||e||_2 
   */
  double eps2 = 0.0; 
  /* contain the output */
  double *Vout = NULL;
  double *Sout = NULL; 
  double *Uout = NULL;
  double *Wout = NULL;

  for (i = 0; i < 10 ; i ++){
    printf("(%f, %f, %f)\n",
	  Vx(init_pts[i]),Vy(init_pts[i]),Vz(init_pts[i]));
  }
  /* run_sfm is a wrapper written by Noah Snavely in the sfm-driver
   * util.  It makes the call to sba-1.5s sba_motstr_levmar() function
   * a little easier.  I need to figure out what the Vout,Sout,Uout
   * and Wout are for -- they could just be scratch space 
   */ 
  run_sfm(num_pts, num_cameras, ncons, vmask, projections, 
	  est_focal_length, const_focal_length, undistort, 
	  explicit_camera_centers, init_camera_params, 
	  init_pts, 
	  use_constraints, use_point_constraints, 
	  pt_constraints, pt_constraint_weight, 
	  fix_points, optimize_for_fisheye, 
	  eps2, Vout, Sout, Uout,  Wout);
  for (i = 0 ; i < 10 ; i ++){
    printf("(%f, %f, %f)\n",
	  Vx(init_pts[i]),Vy(init_pts[i]),Vz(init_pts[i]));
  }
  return 0;
}

// Register functions in LUA
static const struct luaL_reg bundler [] = {
  {"sfm", sfm_l},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libbundler (lua_State *L) {
  luaL_openlib(L, "libbundler", bundler, 0);
  return 1;
}
