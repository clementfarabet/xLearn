/*
Copyright ESIEE (2009) 

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
This code was originally written by Jacques-Olivier Lachaud for the open-source library ImaGene. It has been translated into C by Michel Couprie.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int32_t x, y;
} Vector2i;

typedef struct {
  /**
   * Coefficient a in (a,b,mu) characterizing the line segment.
   */
  int32_t m_a;
  /**
   * Coefficient b in (a,b,mu) characterizing the line segment.
   */
  int32_t m_b;
  /**
   * Coefficient mu in (a,b,mu) characterizing the line segment.
   */
  int32_t m_mu;
  /**
   * Positive upper leaning point U'.
   */
  Vector2i m_up;
  /**
   * Positive lower leaning point L'.
   */
  Vector2i m_lp;
  /**
   * Negative upper leaning point U.
   */
  Vector2i m_u;
  /**
   * Positive lower leaning point L.
   */
  Vector2i m_l;
  /**
   * Positive extremity C'_n.
   */
  Vector2i m_cp_n;
  /**
   * Negative extremity C_n.
   */
  Vector2i m_c_n;
  /**
   * State describing the segment [C'_{n-1} <-> C'_n]. 0 : tangent, 4:
   * turned_int, 8: turned_ext
   */
  uint8_t m_state_cp_n; 
  /**
   * State describing the segment [C_{n-1} <-> C_n]. 0 : tangent, 4:
   * turned_int, 8: turned_ext
   */
  uint8_t m_state_c_n;

} mcsegment;

extern void mcsegment_print(mcsegment * ms);
extern void mcsegment_init(mcsegment * ms);
extern uint32_t mcsegment_extendsFront(mcsegment * ms, uint32_t code);
extern uint32_t mcsegment_extendsBack(mcsegment * ms, uint32_t code);
extern uint32_t mcsegment_retractsBack(mcsegment * ms, uint32_t code);

#define _mcsegment_h
#ifdef __cplusplus
}
#endif
