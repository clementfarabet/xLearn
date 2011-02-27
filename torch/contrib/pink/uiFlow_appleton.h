#ifndef UIFLOW_APPLETON_H_
#define UIFLOW_APPLETON_H_
/*
This software is ment to be free.
(C) UjoImro, 2007
ujoimro@gmail.com
*/
uiScalarField * uiSearchFlow_ben(
		uiScalarField * Potencial, /* measure field picture */
		uiScalarField * S, /* image of the source and of the sink (not the original image) */
		uiScalarField * gg, /* Boundaries */
		int iteration,     /* number of iterations */
		uiVal_type tau		 /* timestep */
																);

#endif /*UIFLOW_APPLETON_H_*/
//LuM end of file

