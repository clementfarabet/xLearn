/*
Copyright ESIEE (2010) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

/*
This code was originally written by Jacques-Olivier Lachaud for the open-source library ImaGene. It has been translated into C and adapted by Michel Couprie.
October 2010
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <mcutil.h>
#include <mcsegment.h>

void mcsegment_print(mcsegment * ms)
{
  printf("a, b, mu = %d %d %d\n", ms->m_a, ms->m_b, ms->m_mu);
  printf("U' = %d %d, L' = %d %d, ", ms->m_up.x, ms->m_up.y, ms->m_lp.x, ms->m_lp.y);
  printf("U = %d %d, L = %d %d\n", ms->m_u.x, ms->m_u.y, ms->m_l.x, ms->m_l.y);
  printf("C'_n = %d %d, C_n = %d %d\n", ms->m_cp_n.x, ms->m_cp_n.y, ms->m_c_n.x, ms->m_c_n.y);
  printf("state_cp_n = %d, state_cp_n = %d\n", ms->m_state_cp_n, ms->m_state_c_n);
} // mcsegment_print()

/**
 * (Re-)Initializes the line segment as an horizontal edge with
 * first point C_n in (0,0) and second point in (1,0). The
 * "interior" of the contour is to the right (clockwise) or
 * negatively along y axis.
 */
void mcsegment_init(mcsegment * ms)
{
  // initial digital line coefficients.
  ms->m_a = 0;
  ms->m_b = 1;
  ms->m_mu = 0;
  ms->m_up.x = 1;
  ms->m_up.y = 0;          // U'
  ms->m_lp.x = 1;
  ms->m_lp.y = 0;          // L'
  ms->m_u.x = 0;
  ms->m_u.y = 0;           // U
  ms->m_l.x = 0;
  ms->m_l.y = 0;           // L
  ms->m_cp_n.x = 1;
  ms->m_cp_n.y = 0;        // C'_n
  ms->m_c_n.x = 0;
  ms->m_c_n.y = 0;         // C_n
  ms->m_state_cp_n = 0;    // 0 : tangent, 4: turned_int, 8: turned_ext
  ms->m_state_c_n = 0;     // 0 : tangent, 4: turned_int, 8: turned_ext
} // mcsegment_init()

/**
 * Tries to extend the discrete line segment to the "front"
 * (positively along the contour).
 *
 * @param code 0: no move, 1: turn right (toward interior), 2: move
 * straight ahead (no turn), 3: turn left (toward exterior).
 *
 * @return '0' if this extension was valid and the discrete line
 * segment has been updated, '1' if the extension was not valid
 * because of code incompatibility, '2' if the extension was not
 * valid because it cannot be a digital straight line. In case '!=0'
 * the discrete line segment is unchanged.
 */
uint32_t mcsegment_extendsFront(mcsegment * ms, uint32_t code)
{
#undef F_NAME
#define F_NAME "mcsegment_extendsFront"

  // Memorizes C'_n and state.
  Vector2i old_cp_n;
  uint8_t state = ms->m_state_cp_n;
  
  old_cp_n.x = ms->m_cp_n.x;
  old_cp_n.y = ms->m_cp_n.y;

  // [code] indicates where is C'_{n+1}.
  // Depends on the current state and on the move.
  // Some moves are incompatible. Others are compatible but there is no
  // discrete line that includes all these points.
  switch ( ms->m_state_cp_n + code ) {
  case 0: break;
  case 1: 
    if ( ms->m_a <= 0 ) // test compatibility
      { ms->m_cp_n.y--; ms->m_state_cp_n = 4; } 
    else code = 0;
    break;
  case 2: ms->m_cp_n.x++; ms->m_state_cp_n = 0; break;
  case 3: 
    if ( ms->m_a >= 0 ) // test compatibility
      { ms->m_cp_n.y++; ms->m_state_cp_n = 8; }
    else code = 0;
    break;
  case 4: code = 0; break;
  case 5: code = 0; break;
  case 6: ms->m_cp_n.y--; break;
  case 7: 
    assert( ms->m_a <= 0 ); // test compatibility
    ms->m_cp_n.x++; ms->m_state_cp_n = 0;
    break;
  case 8: code = 0; break;
  case 9: 
    assert( ms->m_a >= 0 ); // test compatibility
    ms->m_cp_n.x++; ms->m_state_cp_n = 0;
    break;
  case 10: ms->m_cp_n.y++; break;
  case 11: code = 0; break;
  default: code = 0; break;
  }

  // In this case, the discrete line segment is not updated.
  if ( code == 0 ) return 1;

  // compute new ax-by.
  int c = ms->m_a * ms->m_cp_n.x - ms->m_b * ms->m_cp_n.y;
  int lower_bound = ms->m_mu - 1;
  int upper_bound = ms->m_mu + ( ms->m_a >= 0 ? ms->m_a : -ms->m_a ) + ms->m_b;
  // start recognition.
  if ( ( c < lower_bound ) || ( c > upper_bound ) )
    { // not a digital line
      ms->m_cp_n = old_cp_n;
      ms->m_state_cp_n = state;
      return 2;
    }
  else if ( c == lower_bound )
    { // new upper critical point.
      ms->m_up = ms->m_cp_n; // U'<- C'_{n+1}
      ms->m_l = ms->m_lp;    // L <- L'
      ms->m_a = ms->m_up.y - ms->m_u.y;
      ms->m_b = ms->m_up.x - ms->m_u.x;
      ms->m_mu = ms->m_a * ms->m_up.x - ms->m_b * ms->m_up.y;
    }
  else if ( c == upper_bound )
    { // new lower critical point.
      ms->m_u = ms->m_up;    // U <- U'
      ms->m_lp = ms->m_cp_n; // L'<- C'_{n+1}
      ms->m_a = ms->m_lp.y - ms->m_l.y;
      ms->m_b = ms->m_lp.x - ms->m_l.x;
      ms->m_mu = ms->m_a * ms->m_lp.x - ms->m_b * ms->m_lp.y 
	- ( ms->m_a >= 0 ? ms->m_a : -ms->m_a ) 
	- ms->m_b + 1;
    }
  else
    { // still the same digital line.
      // The upper and lower critical point may change.
      if ( c == ms->m_mu ) 
	ms->m_up = ms->m_cp_n; // U' <- C'_{n+1}
      if ( c == ( upper_bound - 1 ) )
	ms->m_lp = ms->m_cp_n; // L' <- C'_{n+1}
    }

  // The discrete line segment has been updated.
  return 0;
} // mcsegment_extendsFront()


/**
 * Tries to extend the discrete line segment to the "back"
 * (negatively along the contour).
 *
 * @param code 0: no move, 1: turn left (toward interior), 2: move
 * straight ahead (no turn), 3: turn right (toward exterior).
 *
 * @return '0' if this extension was valid and the discrete line
 * segment has been updated, '1' if the extension was not valid
 * because of code incompatibility, '2' if the extension was not
 * valid because it cannot be a digital straight line. In case '!=0'
 * the discrete line segment is unchanged.
 */
uint32_t mcsegment_extendsBack(mcsegment * ms, uint32_t code)
{
#undef F_NAME
#define F_NAME "mcsegment_extendsBack"
  // Memorizes C_n and state.
  Vector2i old_c_n;
  uint8_t state = ms->m_state_c_n;

  old_c_n.x = ms->m_c_n.x;
  old_c_n.y = ms->m_c_n.y;

  // [code] indicates where is C'_{n+1}.
  // Depends on the current state and on the move.
  // Some moves are incompatible. Others are compatible but there is no
  // discrete line that includes all these points.
  switch ( ms->m_state_c_n + code ) {
  case 0: break;
  case 1: 	
    if ( ms->m_a >= 0 ) // test compatibility
      { ms->m_c_n.y--; ms->m_state_c_n = 4; }
    else code = 0;
    break;
  case 2: ms->m_c_n.x--; ms->m_state_c_n = 0; break;
  case 3: 
    if ( ms->m_a <= 0 ) // test compatibility
      { ms->m_c_n.y++; ms->m_state_c_n = 8; }
    else code = 0;
    break;
  case 4: code = 0; break;
  case 5: code = 0; break;
  case 6: ms->m_c_n.y--; break;
  case 7: 
    assert( ms->m_a >= 0 ); // test compatibility
    ms->m_c_n.x--; ms->m_state_c_n = 0; break;
  case 8: code = 0; break;
  case 9: 
    assert( ms->m_a <= 0 ); // test compatibility
    ms->m_c_n.x--; ms->m_state_c_n = 0; break;
  case 10: ms->m_c_n.y++; break;
  case 11: code = 0; break;
  default: code = 0; break;
  }

  // In this case, the discrete line segment is not updated.
  if ( code == 0 ) return 1;

  // compute new ax-by.
  int c = ms->m_a * ms->m_c_n.x - ms->m_b * ms->m_c_n.y;
  int lower_bound = ms->m_mu - 1;
  int upper_bound = ms->m_mu + ( ms->m_a >= 0 ? ms->m_a : -ms->m_a ) + ms->m_b;
  // start recognition.
  if ( ( c < lower_bound ) || ( c > upper_bound ) )
    { // not a digital line
      ms->m_c_n = old_c_n;
      ms->m_state_c_n = state;
      return 2;
    }
  else if ( c == lower_bound )
    { // new upper critical point.
      ms->m_u = ms->m_c_n;   // U <- C_{n+1}
      ms->m_lp = ms->m_l;    // L'<- L
      ms->m_a = ms->m_up.y - ms->m_u.y;
      ms->m_b = ms->m_up.x - ms->m_u.x;
      ms->m_mu = ms->m_a * ms->m_u.x - ms->m_b * ms->m_u.y;
    }
  else if ( c == upper_bound )
    { // new lower critical point.
      ms->m_up = ms->m_u;    // U'<- U
      ms->m_l = ms->m_c_n;   // L <- C_{n+1}
      ms->m_a = ms->m_lp.y - ms->m_l.y;
      ms->m_b = ms->m_lp.x - ms->m_l.x;
      ms->m_mu = ms->m_a * ms->m_l.x - ms->m_b * ms->m_l.y 
	- ( ms->m_a >= 0 ? ms->m_a : -ms->m_a )
	- ms->m_b + 1;
    }
  else
    { // still the same digital line.
      // The upper and lower critical point may change.
      if ( c == ms->m_mu ) 
	ms->m_u = ms->m_c_n; // U <- C_{n+1}
      if ( c == ( upper_bound - 1 ) )
	ms->m_l = ms->m_c_n; // L <- C_{n+1}
    }

  // The discrete line segment has been updated.
  return 0;
} // mcsegment_extendsBack()

/**
 * Tries to retract the discrete line segment to the "back"
 * (negatively along the contour).
 *
 * @param code the shape of the retraction going from the "back"
 * extremity: 0: no move, 1: turn left (toward interior), 2: move
 * straight ahead (no turn), 3: turn right (toward exterior).
 *
 * @return 'true' if this retraction was valid and the discrete line
 * segment has been updated, otherwise 'false' and the discrete line
 * segment is unchanged.
 *
 * @pre ms->m_c_n != (0,0)
 */

uint32_t mcsegment_retractsBack(mcsegment * ms, uint32_t code )
{
#undef F_NAME
#define F_NAME "mcsegment_retractsBack"
  // Cannot retract if reference frame is in the retraction.
  assert( ( ms->m_c_n.x != 0 ) || ( ms->m_c_n.y != 0 ) );
  
  Vector2i m;
  m.x = ms->m_c_n.x;
  m.y = ms->m_c_n.y;

  // Takes care of state machine.
  switch ( ms->m_state_c_n + code ) {
  case 0: break;
  case 1: assert( ms->m_a < 0 );
    ms->m_state_c_n = 8; 
    ++ms->m_c_n.x;
    break;
  case 2: // state unchanged
    ++ms->m_c_n.x;
    break;
  case 3: assert( ms->m_a > 0 );
    ms->m_state_c_n = 4;
    ++ms->m_c_n.x;
    break;
  case 4: break;
  case 5: assert( ms->m_a > 0 );
    ms->m_state_c_n = 0;
    ++ms->m_c_n.y;
    break;
  case 6: assert( ms->m_a > 0 );
    // state unchanged
    ++ms->m_c_n.y;
    break;
  case 7: assert( 0 ); // Should not occur
    code = 0;
    break;
  case 8: break;
  case 9: assert( 0 ); // Should not occur
    code = 0;
    break;
  case 10: assert( ms->m_a < 0 );
    // state unchanged
    --ms->m_c_n.y;
    break;
  case 11: assert( ms->m_a < 0 );
    ms->m_state_c_n = 0;
    --ms->m_c_n.y;
    break;
  }

  if ( code == 0 ) return 0;
  
  // Updates line segment and upper and lower leaning lines depending on the
  // retracted point.

  // Case 0: horizontal line.
  if ( ( ms->m_a == 0 ) && ( ms->m_b == 1 ) )
    {
      assert( ( m.x == ms->m_u.x ) && ( m.x == ms->m_l.x ) );
      assert( ( m.y == ms->m_u.y ) && ( m.y == ms->m_l.y ) );
      ++ms->m_u.x;
      ++ms->m_l.x;
      assert( (ms->m_u.x != ms->m_up.x) || (ms->m_u.y != ms->m_up.y) );
    }
  // Case 1: M == U
  else if (( m.x == ms->m_u.x ) && ( m.y == ms->m_u.y ))
    { // Two subcases depending on L' == L
      if (( ms->m_lp.x == ms->m_l.x ) && ( ms->m_lp.y == ms->m_l.y ))
	{ // Case 1.a: L' == L
	  // => M Changed the slope of the line.
	  
	  // P "is" a lower leaning point.
	  // a > 0 => P = M + (1,-1)
	  // a < 0 => P = M + (-1,-1)
	  Vector2i p;
	  p.x = m.x;
	  p.y = m.y;
	  if ( ms->m_a > 0 )
	    { 
	      p.x += 1;
	      p.y -= 1;
	    }
	  else // ( ms->m_a < 0 )
	    { 
	      p.x -= 1;
	      p.y -= 1;
	    }
	  // Since P and L defines the slope of the line, we get:
	  ms->m_b = ms->m_l.x - p.x;
	  ms->m_a = ms->m_l.y - p.y;
	  // Since U' belongs to the upper leaning line, we get:
	  ms->m_mu = ms->m_a * ms->m_up.x - ms->m_b * ms->m_up.y;

	  // L and U' are not changed.

	  // JOL 2004/09/07
	  if ( ms->m_a == 0 ) 
	    {
	      ms->m_u = ms->m_l;
	      ms->m_lp = ms->m_up;
	      assert((ms->m_u.x == ms->m_c_n.x) && (ms->m_lp.x == ms->m_cp_n.x));
	      assert((ms->m_u.y == ms->m_c_n.y) && (ms->m_lp.y == ms->m_cp_n.y));
	    }
	  else if ( ms->m_a > 0 )
	    {
	      // We recompute U from U' and the length of the discrete segment.
	      int k = ( ms->m_up.x - ms->m_c_n.x ) / ms->m_b;
	      ms->m_u.x = ms->m_up.x - k * ms->m_b;
	      ms->m_u.y = ms->m_up.y - k * ms->m_a;
	      
	      // We recompute L' from L and the length of the discrete segment.
	      // JOL 2004/09/07 : int l = ( ms->m_cp_n.x - p.x ) / ms->m_b;
	      int l = ( ms->m_cp_n.x - ms->m_c_n.x ) / ms->m_b;
	      ms->m_lp.x = ms->m_l.x + ( l - 1 ) * ms->m_b;
	      ms->m_lp.y = ms->m_l.y + ( l - 1 ) * ms->m_a;
	      
	      assert( ( k >= 0 ) && ( l >= 1 ) );
	    }
	  else // case ( ms->m_a < 0 )
	    {
	      // We recompute U from U' and the length of the discrete segment.
	      int k = ( ms->m_up.y - ms->m_c_n.y ) / ms->m_a;
	      ms->m_u.x = ms->m_up.x - k * ms->m_b;
	      ms->m_u.y = ms->m_up.y - k * ms->m_a;
	      
	      // We recompute L' from L and the length of the discrete segment.
	      // JOL 2004/09/07 : int l = ( ms->m_cp_n.x - p.x ) / ms->m_b;
	      int l = ( ms->m_cp_n.y - ms->m_c_n.y ) / ms->m_a;
	      ms->m_lp.x = ms->m_l.x + ( l - 1 ) * ms->m_b;
	      ms->m_lp.y = ms->m_l.y + ( l - 1 ) * ms->m_a;
	      
	      assert( ( k >= 0 ) && ( l >= 1 ) );
	    }
	}
      else
	{ // Case 1.b: L' != L
	  // => M did not change the slope of the line, was only on the upper
	  // leaning line.
	  // U <- U + (b,a)
	  ms->m_u.x += ms->m_b;
	  ms->m_u.y += ms->m_a;
	}
    } // if ( m == ms->m_u )
  // Case 2: M == L
  else if (( m.x == ms->m_l.x ) && ( m.y == ms->m_l.y ))
    { // Two subcases depending on U' == U
      if (( ms->m_up.x == ms->m_u.x ) && ( ms->m_up.y == ms->m_u.y ))
	{ // Case 2.a: U' == U 
	  // => M Changed the slope of the line.
	  
	  // P "is" an upper leaning point.
	  // a > 0 => P = M + (-1,1)
	  // a < 0 => P = M + (1,1)
	  Vector2i p;
	  p.x = m.x;
	  p.y = m.y;

	  if ( ms->m_a > 0 )
	    { 
	      p.x -= 1;
	      p.y += 1;
	    }
	  else // ( ms->m_a < 0 )
	    { 
	      p.x += 1;
	      p.y += 1;
	    }
	  // Since P and U defines the slope of the line, we get:
	  ms->m_b = ms->m_u.x - p.x;
	  ms->m_a = ms->m_u.y - p.y;
	  // Since U belongs to the upper leaning line, we get:
	  ms->m_mu = ms->m_a * ms->m_u.x - ms->m_b * ms->m_u.y;
	  // L' and U are not changed.

	  // JOL 2004/09/07
	  if ( ms->m_a == 0 ) 
	    {
	      ms->m_l = ms->m_u;
	      ms->m_up = ms->m_lp;
	      assert((ms->m_l.x == ms->m_c_n.x) && (ms->m_up.x == ms->m_cp_n.x));
	      assert((ms->m_l.y == ms->m_c_n.y) && (ms->m_up.y == ms->m_cp_n.y));
	    }
	  else if ( ms->m_a > 0 )
	    {
	      // We recompute L from L' and the length of the discrete segment.
	      int k = ( ms->m_lp.y - ms->m_c_n.y ) / ms->m_a;
	      ms->m_l.x = ms->m_lp.x - k * ms->m_b;
	      ms->m_l.y = ms->m_lp.y - k * ms->m_a;
	      
	      // We recompute U' from U and the length of the discrete segment.
	      // JOL 2004/09/07 : int l = ( ms->m_cp_n.x - p.x ) / ms->m_b;
	      int l = ( ms->m_cp_n.y - ms->m_c_n.y ) / ms->m_a;
	      ms->m_up.x = ms->m_u.x + ( l - 1 ) * ms->m_b;
	      ms->m_up.y = ms->m_u.y + ( l - 1 ) * ms->m_a;
	      
	      assert( ( k >= 0 ) && ( l >= 1 ) );
	    }
	  else // case ( ms->m_a < 0 )
	    {
	      // We recompute L from L' and the length of the discrete segment.
	      int k = ( ms->m_lp.x - ms->m_c_n.x ) / ms->m_b;
	      ms->m_l.x = ms->m_lp.x - k * ms->m_b;
	      ms->m_l.y = ms->m_lp.y - k * ms->m_a;
	      
	      // We recompute U' from U and the length of the discrete segment.
	      // JOL 2004/09/07 : int l = ( ms->m_cp_n.x - p.x ) / ms->m_b;
	      int l = ( ms->m_cp_n.x - ms->m_c_n.x ) / ms->m_b;
	      ms->m_up.x = ms->m_u.x + ( l - 1 ) * ms->m_b;
	      ms->m_up.y = ms->m_u.y + ( l - 1 ) * ms->m_a;
	      
	      assert( ( k >= 0 ) && ( l >= 1 ) );
	    }
	}
      else
	{ // Case 2.b: U' != U
	  // => M did not change the slope of the line, was only on the lower
	  // leaning line.
	  // L <- L + (b,a)
	  ms->m_l.x += ms->m_b;
	  ms->m_l.y += ms->m_a;
	}
    } // if ( m == ms->m_l )

//   cerr << "retractsBack: (" << ms->m_a << "," << ms->m_b << "," << ms->m_mu << ")"
//        << " Cn=" << ms->m_c_n.x << "," << ms->m_c_n.y
//        << " C'n=" << ms->m_cp_n.x << "," << ms->m_cp_n.y
//        << endl
//        << " U=" << ms->m_u.x << "," << ms->m_u.y
//        << " U'=" << ms->m_up.x << "," << ms->m_up.y
//        << " L=" << ms->m_l.x << "," << ms->m_l.y
//        << " L'=" << ms->m_lp.x << "," << ms->m_lp.y 
//        << endl;
  

  // Otherwise, nothing to do.
  return 1;
} // mcsegment_retractsBack()

#ifdef DEBUG
int32_t main()
{
  mcsegment ms;  
  uint32_t ret;
  char r[80];

  mcsegment_init(&ms);
  
  do
  {
    printf("commande (qUIT, iNIT, pRINT, 0 (no move), 1 (turn left), 2 (forward), 3 (turn right))\n");
    scanf("%s", r);
    switch (r[0])
    {
      case 'i':
	mcsegment_init(&ms);
        break;
      case 'p':
	mcsegment_print(&ms);
        break;
      case '0':
	ret = mcsegment_extendsFront(&ms, 0);
	printf("ret = %d\n", ret);
	mcsegment_print(&ms);
        break;
      case '1':
	ret = mcsegment_extendsFront(&ms, 1);
	printf("ret = %d\n", ret);
	mcsegment_print(&ms);
        break;
      case '2':
	ret = mcsegment_extendsFront(&ms, 2);
	printf("ret = %d\n", ret);
	mcsegment_print(&ms);
        break;
      case '3':
	ret = mcsegment_extendsFront(&ms, 3);
	printf("ret = %d\n", ret);
	mcsegment_print(&ms);
        break;
      case 'q': break;
      default: 
	printf("ERREUR : mauvaise commande\n");
    }
  } while (r[0] != 'q');
}
#endif
