/*
 * $Id: readpnm.hxx,v 4.2 2008/01/22 13:54:22 hut66au Exp $
 *
 * Imview, the portable image analysis application
 * http://www.cmis.csiro.au/Hugues.Talbot/imview
 * ----------------------------------------------------------
 *
 *  Imview is an attempt to provide an image display application
 *  suitable for professional image analysis. It was started in
 *  1997 and is mostly the result of the efforts of Hugues Talbot,
 *  Image Analysis Project, CSIRO Mathematical and Information
 *  Sciences, with help from others (see the CREDITS files for
 *  more information)
 *
 *  Imview is Copyrighted (C) 1997-2001 by Hugues Talbot and was
 *  supported in parts by the Australian Commonwealth Science and 
 *  Industry Research Organisation. Please see the COPYRIGHT file 
 *  for full details. Imview also includes the contributions of 
 *  many others. Please see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 * */

/*------------------------------------------------------------------------
 *
 * Header information for PNM input
 *
 * Hugues Talbot	29 Jul 1998
 * edited by UjoImro    25 Feb 2009
 *      
 *-----------------------------------------------------------------------*/

#ifndef READPNM_H
#define READPNM_H

typedef enum {
    IV_MANDATORY =  0,
    IV_OPTIONAL ,
    IV_FINAL
} toberead;



PTR<pink::ujoi <float> > readPNMImage(const string & filename);
//int pnmnbsubfiles(const char *);

#endif
