/*
  This software is licensed under 
  CeCILL FREE SOFTWARE LICENSE AGREEMENT

  This software comes in hope that it will be useful but 
  without any warranty to the extent permitted by aplicable law.
  
  (C) UjoImro, 2006-2010
  Université Paris-Est, Laboratoire d'Informatique Gaspard-Monge, Equipe A3SI, ESIEE Paris, 93162, Noisy le Grand CEDEX
  ujoimro@gmail.com
*/

#ifndef UIIMAGE_H_
#define UIIMAGE_H_

namespace pink
{
  
  const int longestline = 100;
  
  void uiWriteImage ( const pink::ujoi<float> & image, const string & filename );
  void uiWriteFloatImage3D( const pink::ujoi<float> & image, const string & filename );
  
//// in fact next two functions should not be exported
  void uiWriteImage2D ( const pink::ujoi<float> & image, const string & filename );
  void uiWriteImage3D ( const pink::ujoi<float> & image, const string & filename );


  template <class image_type>
  void print_image( const image_type & image )
  {
    stringstream result;
        
    
    if ((image.get_size().prod()>1000) or (image.get_size().size()>2))
    {
      cout << "the image is too big\n";      
    }
    else /* NOT (image.get_size().prod()>1000) or (image.get_size().size()>2) */
    {
      vint curr;
      int tmp; // for converting the references. Apparently cout does not like them.      
      result << "[\n";      
      FOR(q, image.get_size()[0])
      {        
        FOR(w, image.get_size()[1])
        {
          tmp = image[curr << q,w];          
          cout << tmp << ", ";          
        } /* FOR w */
        cout << "\n";        
      } /* FOR q */
      cout << "]\n";      
    } /* NOT (image.get_size().prod()>1000) or (image.get_size().size()>2) */

  } /* print_image */
  
  
} /* namespace pink*/




#endif /*UIIMAGE_H_*/
/* LuM end of file */

