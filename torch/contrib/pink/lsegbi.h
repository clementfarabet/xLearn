/* $Id: lsegbi.h,v 1.2 2009/01/07 12:46:33 mcouprie Exp $ */
#ifdef __cplusplus
extern "C" {
#endif
/* ============== */
/* prototype for lsegbi.c */
/* ============== */

  int32_t lsegbinonhomotopique(
    struct xvimage *image,
    int32_t connex,
    double cmin,
    double cmax
    );

#ifdef __cplusplus
}
#endif
