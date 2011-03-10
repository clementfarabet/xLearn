// proposition


#ifndef LMCUBE_H_
#define LMCUBE_H_


#ifdef __cplusplus
extern "C" {
#endif


void InitLUT();

void lmcube_point( int32_t x, int32_t y, int32_t z, uint8_t *cube, int32_t point, 
                   double x_offset, double y_offset, double z_offset,
                   double *xp, double *yp, double *zp);



void pointcst( int32_t x, int32_t y, int32_t z, uint8_t *cube, int32_t point, 
               double x_offset, double y_offset, double z_offset,
               double x_factor, double y_factor, double z_factor,
               double *xp, double *yp, double *zp);



int32_t estfixe(uint8_t *cube, int32_t point);



int32_t lmarchingcubes(struct xvimage * f, uint8_t v, 
                       int32_t nregul, int32_t obj_id, FILE *fileout, 
                       int32_t format);



int32_t lmarchingcubes2(struct xvimage * f,
                        int32_t nregul, int32_t obj_id, FILE *fileout, 
                        int32_t format);




#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LMCUBE_H_ */


















// LuM end of file
