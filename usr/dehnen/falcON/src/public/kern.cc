// -*- C++ -*-                                                                 |
//-----------------------------------------------------------------------------+
//                                                                             |
// kern.cc                                                                     |
//                                                                             |
// C++ code                                                                    |
//                                                                             |
// Copyright Walter Dehnen, 2000-2002                                          |
// e-mail:   wdehnen@aip.de                                                    |
// address:  Astrophysikalisches Institut Potsdam,                             |
//           An der Sternwarte 16, D-14482 Potsdam, Germany                    |
//                                                                             |
//-----------------------------------------------------------------------------+
//                                                                             |
// implementing internal/kern.h                                                |
// or           proprietary/kern_sse.h                                         |
//                                                                             |
// Note that in case of the non-public SSE_CODE, most contents of this file    |
// are commented out and are replaced by the private version in priv.h         |
//                                                                             |
//-----------------------------------------------------------------------------+
//                                                                             |
// macro steered structure:                                                    |
//                                                                             |
//     general macros & code required for all further code below               |
// #ifdef SSE_CODE                                                             |
// # include <nbdy/priv/kern_sse>   code using SSE instructions                |
// #else                                                                       |
//     macros needed for non-SSE stuff                                         |
// # ifdef ALLOW_INDI                                                          |
// #  include <nbdy/priv/kern_ind>  non-SSE code for adaptive softening        |
// # else                                                                      |
//     non-SSE code for non-adaptive softening                                 |
// # endif                                                                     |
// #endif                                                                      |
// #ifndef(ALLOW_INDI)                                                         |
//     code for non-adaptive softening, not implemented using SSE              |
// #endif                                                                      |
//                                                                             |
//-----------------------------------------------------------------------------+
#include <public/auxx.h>

#if defined(REAL_IS_FLOAT) && defined(USE_SSE)
#  define  SSE_CODE
#  include <proper/kern_sse.h>
#else
#  undef   SSE_CODE
#  include <public/kern.h>
#endif

////////////////////////////////////////////////////////////////////////////////

#define ALLOW_FULL_POT       // enable fully consistent computation of potential
#undef  ALLOW_FULL_POT       // never compute the potential fully consistently  

using namespace nbdy;
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// macros also used in both the public and proprietary part of the code must be 
// defined here.                                                                
//                                                                              
////////////////////////////////////////////////////////////////////////////////
// single Body-Body interaction                                                 
////////////////////////////////////////////////////////////////////////////////
#define NEWTON(MUM)							\
  x  = one/Rq;								\
  D0 = MUM*sqrt(x);							\
  R *= D0*x;
//------------------------------------------------------------------------------
#define P0(MUM)								\
  x  = one/(Rq+EPQ);							\
  D0 = MUM*sqrt(x);							\
  R *= D0*x;
//------------------------------------------------------------------------------
#define P1(MUM)								\
  x  = one/(Rq+EPQ);							\
  D0 = MUM*sqrt(x);							\
  register real D1 = D0*x;						\
  register real heq= half*EPQ;						\
  D0+= heq*D1;								\
  D1+= heq*3*D1*x;							\
  R *= D1;
//------------------------------------------------------------------------------
#define P2(MUM)								\
  x  = one/(Rq+EPQ);							\
  D0 = MUM*sqrt(x);							\
  register real D1 = D0*x, D2= 3*D1*x, D3= 5*D2*x;			\
  register real heq= half*EPQ;						\
  D0+= heq*(D1+heq*D2);							\
  D1+= heq*(D2+heq*D3);							\
  R *= D1;
//------------------------------------------------------------------------------
#define P3(MUM)								\
  x  = one/(Rq+EPQ);							\
  D0 = MUM*sqrt(x);							\
  register real D1 = D0*x, D2= 3*D1*x, D3= 5*D2*x, D4= 7*D3*x;		\
  register real heq= half*EPQ, qeq=half*heq;				\
  D0+= ((heq*D3+D2)*qeq+D1)*heq;					\
  D1+= ((heq*D4+D3)*qeq+D2)*heq;					\
  R *= D1;
////////////////////////////////////////////////////////////////////////////////
// Cell-Body interaction with approximated gravity                              
////////////////////////////////////////////////////////////////////////////////
#define CELL_BODY(_A,_B,_D0,_D1,_D2,_D3,_R)                                    \
  if(is_sink(_A)) {                                /* IF(interaction B->A) >*/ \
    c0(_A)+= _D0;                                  /*   add to C0 of A      */ \
    c1(_A).sub_mul(_R, _D1);                       /*   add to C1 of A      */ \
    c2(_A).add_quadrup(_R,-_D1,_D2);               /*   add to C2 of A      */ \
    c3(_A).add_quadrup(_R,_D2,-_D3);               /*   add to C3 of A      */ \
  }                                                /* <                     */ \
  if(is_sink(_B)) {                                /* IF(interaction B->A) >*/ \
    register vect RQ;                              /*   auxiliary vector    */ \
    quad(_A).inner_prod(_R,RQ);                    /*   R*Q_A               */ \
    register real tQ=trace(quad(_A)),RQR=_R*RQ;    /*   tr(Q_A), R*Q*R      */ \
    _B->pot()-=          _D0-_D1*tQ+_D2*RQR;       /*   pot A->B            */ \
    _B->acc().add_mul(_R,_D1-_D2*tQ+_D3*RQR);      /*   acc A->B (in R)     */ \
    RQ       *= _D2;                               /*   R*Q_A*D2            */ \
    _B->acc().sub_twice(RQ);                       /*   acc A->B (in R*Q_A) */ \
  }                                                /* <                     */
//------------------------------------------------------------------------------
#define CELL_BODY_FULL_POT(_A,_B,_D0,_D1,_D2,_D3,_R)                           \
  SYM3(C3); C3.quadrup(_R,_D2,_D3);                /* pre-compute C3        */ \
  if(is_sink(_A)) {                                /* IF(interaction B->A) >*/ \
    c0(_A)+= _D0;                                  /*   add to C0 of A      */ \
    c1(_A).add_mul(_R, _D1);                       /*   add to C1 of A      */ \
    c2(_A).add_quadrup(_R,_D1,_D2);                /*   add to C2 of A      */ \
    c3(_A) +=C3;                                   /*   add to C3 of A      */ \
  }                                                /* <                     */ \
  if(is_sink(_B)) {                                /* IF(interaction B->A) >*/ \
    register vect RQ;                              /*   auxiliary vector    */ \
    quad(_A).inner_prod(_R,_RQ);                   /*   R*Q_A               */ \
    register real tQ=trace(quad(_A)),RQR=_R*RQ;    /*   tr(Q_A), R*Q*R      */ \
    _B->pot()-= C3.inner_prod(octo(_A))            /*   add to pot of B     */ \
                      +  _D0+_D1*tQ+_D2*RQR;       /*   pot A->B            */ \
    _B->acc().sub_mul(_R,_D1+_D2*tQ+_D3*RQR);      /*   acc A->B (in R)     */ \
    RQ      *= _D2;                                /*   R*Q_A*D2            */ \
    _B->acc().sub_twice(RQ);                       /*   acc A->B (in R*Q_A) */ \
  }                                                /* <                     */
////////////////////////////////////////////////////////////////////////////////
// Cell-Cell interaction with approximated gravity                              
////////////////////////////////////////////////////////////////////////////////
#define CELL_CELL(_A,_B,_D0,_D1,_D2,_D3,_R)                                    \
  SYM2(C2); C2.quadrup(_R,-_D1,_D2);               /* pre-compute C2        */ \
  SYM3(C3); C3.quadrup(_R,_D2,-_D3);               /* pre-compute C3        */ \
  register vect RQ;                                /* to hold R*Q           */ \
  register real tQ, RQR;                           /* to hold tr(Q), R*Q*R  */ \
  if(is_sink(_A)) {                                /* IF(interaction B->A) >*/ \
    quad(_B).inner_prod(_R,RQ);                    /*   R*Q_B               */ \
    tQ    = trace(quad(_B));                       /*   tr(Q_B)             */ \
    RQR   = _R*RQ;                                 /*   R*Q_B*R             */ \
    c0(_A)+=           _D0 - _D1*tQ + _D2*RQR;     /*   add to C0 of A      */ \
    c1(_A).sub_mul(_R, _D1 - _D2*tQ + _D3*RQR);    /*   add to C1 of A (R)  */ \
    RQ   *= _D2;                                   /*   R*Q_B*D2            */ \
    c1(_A).add_twice(RQ);                          /*   add to C1 of A(R*QB)*/ \
    c2(_A)+= C2;                                   /*   add to C2 of A      */ \
    c3(_A)+= C3;                                   /*   add to C3 of A      */ \
  }                                                /* <                     */ \
  if(is_sink(_B)) {                                /* IF(interaction B->A) >*/ \
    _R.negate();                                   /*   set R to -R         */ \
    quad(_A).inner_prod(_R,RQ);                    /*   R*Q_B               */ \
    tQ    = trace(quad(_A));                       /*   tr(Q_A)             */ \
    RQR   = _R*RQ;                                 /*   R*Q_A*R             */ \
    c0(_B)+=           _D0 - _D1*tQ + _D2*RQR;     /*   add to C0 of B      */ \
    c1(_B).sub_mul(_R, _D1 - _D2*tQ + _D3*RQR);    /*   add to C1 of B (R)  */ \
    RQ   *= _D2;                                   /*   R*Q_A*D2            */ \
    c1(_B).add_twice(RQ);                          /*   add to C1 of B(R*QA)*/ \
    c2(_B)+= C2;                                   /*   add to C2 of B      */ \
    c3(_B)-= C3;                                   /*   add to C3 of B      */ \
  }                                                /* <                     */
//------------------------------------------------------------------------------
#define CELL_CELL_FULL_POT(_A,_B,_D0,_D1,_D2,_D3,_R)                           \
  SYM2(C2); C2.quadrup(_R,_D1,_D2);                /* pre-compute C2        */ \
  SYM3(C3); C3.quadrup(_R,_D2,_D3);                /* pre-compute C3        */ \
  register vect RQ;                                /* to hold R*Q           */ \
  register real tQ, RQR;                           /* to hold tr(Q), R*Q*R  */ \
  if(is_sink(_A)) {                                /* IF(interaction B->A) >*/ \
    quad(_B).inner_prod(_R,RQ);                    /*   R*Q_B               */ \
    tQ    = trace(quad(_B));                       /*   tr(Q_B)             */ \
    RQR   = _R*RQ;                                 /*   R*Q_B*R             */ \
    c0(_A)+=           _D0 + _D1*tQ + _D2*RQR      /*   add to C0 of A      */ \
                     - C3.inner_prod(octo(_B));    /*   add to C0 of A      */ \
    c1(_A).add_mul(_R, _D1 + _D2*tQ + _D3*RQR);    /*   add to C1 of A (R)  */ \
    RQ   *= _D2;                                   /*   R*Q_B*D2            */ \
    c1(_A).add_twice(RQ);                          /*   add to C1 of A(R*QB)*/ \
    c2(_A)+=_ C2;                                  /*   add to C2 of A      */ \
    c3(_A)+= C3;                                   /*   add to C3 of A      */ \
  }                                                /* <                     */ \
  if(is_sink(_B)) {                                /* IF(interaction B->A) >*/ \
    _R.negate();                                   /*   set R to -R         */ \
    quad(_A).inner_prod(_R,RQ);                    /*   R*Q_B               */ \
    tQ    = trace(quad(_A));                       /*   tr(Q_A)             */ \
    RQR   = _R*RQ;                                 /*   R*Q_A*R             */ \
    c0(_B)+=           _D0 + _D1*tQ + _D2*RQR;     /*   add to C0 of B      */ \
                     + C3.inner_prod(octo(_A));    /*   add to C0 of B      */ \
    c1(_B).add_mul(_R, _D1 + _D2*tQ + _D3*RQR);    /*   add to C1 of B (R)  */ \
    RQ   *= _D2;                                   /*   R*Q_A*D2            */ \
    c1(_B).add_twice(RQ);                          /*   add to C1 of B(R*QA)*/ \
    c2(_B)+= C2;                                   /*   add to C2 of B      */ \
    c3(_B)-= C3;                                   /*   add to C3 of B      */ \
  }                                                /* <                     */
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// Auxiliary stuff                                                              
//                                                                              
////////////////////////////////////////////////////////////////////////////////
namespace nbdy {
  typedef grav_tree::cell_iterator cell_iter;
  typedef grav_tree::soul_iterator soul_iter;
  //////////////////////////////////////////////////////////////////////////////
  // what subsouls of a cell are sinks?                                         
  //////////////////////////////////////////////////////////////////////////////
  enum who {                                       // what subsouls are sink    
    none = 0,                                      //   none                    
    some = 1,                                      //   some, but not all       
    all  = 2                                       //   all                     
  };
  //----------------------------------------------------------------------------
  inline who who_is_sink(cell_iter const&C)
  {
    if(!is_sink(C)) return none;
    if( al_sink(C)) return all;
                    return some;
  }
}
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// Below, we only give the non-SSE version of the code.                         
//                                                                              
////////////////////////////////////////////////////////////////////////////////

#ifdef SSE_CODE
                                                   // implementing              
                                                   // inc/proprietary/kern_sse.h
#  include <proper/kern_sse.cc>                    // non-public: SSE code      

#else                                              // implementing              
                                                   // inc/internal/kern.h       

////////////////////////////////////////////////////////////////////////////////
//                                                                              
// macros for computing the gravity                                             
//                                                                              
// DSINGL       called after loading a soul-soul interaction                    
// ASINGL       called after loading a cell-node interaction                    
//                                                                              
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// for direct summation code, we assume:                                        
// - real  D0          contains Mi*Mj   on input                                
// - real  D1          contains R^2+e^2 on input                                
// - real  HEQ,QEQ     are set to e^2, e^2/2 and e^2/4 for global softening     
//                                                                              
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// for approximate gravity code, we assume:                                     
// - real  D0          contains Mi*Mj   on input                                
// - real  D1          contains R^2+e^2 on input                                
// - real  D2,D3       to be computed as well                                   
// - real  HEQ,QEQ     are set to e^2/2 and e^2/4      for global softening     
//                                                                              
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// NOTE that we changed the definition of the D_n by a sign:                    
//                                                                              
// D_n = (-1/r d/dr)^n g(r) at r=|R|                                            
//                                                                              
////////////////////////////////////////////////////////////////////////////////
#if P_ORDER != 3
#  error  "currently we only support P_ORDER==3 in kern.cc"
#endif
//------------------------------------------------------------------------------
// P0: Plummer and Newtonian                                                    
//------------------------------------------------------------------------------
# define DSINGL_P0_G				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  = XX * D0;
# define DSINGL_P0_I				\
  DSINGL_P0_G
//------------------------------------------------------------------------------
# define ASINGL_P0_G				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  =      XX * D0;				\
  D2  =  3 * XX * D1;				\
  D3  =  5 * XX * D2;
//------------------------------------------------------------------------------
// P1                                                                           
//------------------------------------------------------------------------------
# define DSINGL_P1(HQ)				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  = XX * D0;				\
  XX *= 3  * D1;     /* XX == T2 */		\
  D0 += HQ * D1;				\
  D1 += HQ * XX;
# define DSINGL_P1_G				\
  DSINGL_P1(HEQ)
//------------------------------------------------------------------------------
# define ASINGL_P1(HQ)				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  =      XX * D0;				\
  D2  =  3 * XX * D1;				\
  D3  =  5 * XX * D2;				\
  XX *=  7 * D3;      /* XX == T4 */		\
  D0 += HQ * D1;				\
  D1 += HQ * D2;				\
  D2 += HQ * D3;				\
  D3 += HQ * XX;
# define ASINGL_P1_G				\
  ASINGL_P1(HEQ)
//------------------------------------------------------------------------------
// P2                                                                           
//------------------------------------------------------------------------------
# define DSINGL_P2(HQ)				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  = XX * D0;				\
  register real					\
  D2  = 3 * XX * D1;				\
  XX *= 5 * D2;           /* XX == T3 */	\
  D0 += HQ*(D1+HQ*D2);				\
  D1 += HQ*(D2+HQ*XX);
# define DSINGL_P2_G				\
  DSINGL_P2(HEQ)
//------------------------------------------------------------------------------
# define ASINGL_P2(HQ)				\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  =      XX * D0;				\
  D2  =  3 * XX * D1;				\
  D3  =  5 * XX * D2;				\
  register real					\
  D4  =  7 * XX * D3;				\
  XX *=  9 *      D4   ;  /* XX == T5 */	\
  D0 += HQ *(D1+HQ*D2);				\
  D1 += HQ *(D2+HQ*D3);				\
  D2 += HQ *(D3+HQ*D4);				\
  D3 += HQ *(D4+HQ*XX);
# define ASINGL_P2_G				\
  ASINGL_P2(HEQ)
//------------------------------------------------------------------------------
// P3                                                                           
//------------------------------------------------------------------------------
# define DSINGL_P3(HQ,QQ)			\
  register real					\
  XX  = one/D1;					\
  D0 *= sqrt(XX);				\
  D1  =     XX * D0;				\
  register real					\
  D2  = 3 * XX * D1;				\
  register real					\
  D3  = 5 * XX * D2;				\
  XX *= 7 * D3;           /* XX == T4 */	\
  D0 += HQ*(D1+QQ*(D2+HQ*D3));			\
  D1 += HQ*(D2+QQ*(D3+HQ*XX));
# define DSINGL_P3_G				\
  DSINGL_P3(HEQ,QEQ)
//------------------------------------------------------------------------------
# define ASINGL_P3(HQ,QQ)			\
  register real 				\
  XX  = one/D1;				\
  D0 *= sqrt(XX);				\
  D1  =      XX * D0;			\
  D2  =  3 * XX * D1;			\
  D3  =  5 * XX * D2;			\
  register real 				\
  D4  =  7 * XX * D3;			\
  register real 				\
  D5  =  9 * XX * D4   ;			\
  XX *= 11 *      D5   ;  /* XX == T6 */	\
  D0 += HQ*(D1+QQ*(D2+HQ*D3));	\
  D1 += HQ*(D2+QQ*(D3+HQ*D4));	\
  D2 += HQ*(D3+QQ*(D4+HQ*D5));	\
  D3 += HQ*(D4+QQ*(D5+HQ*XX));
# define ASINGL_P3_G				\
  ASINGL_P3(HEQ,QEQ)
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// 1. direct summation of many-body interactions                                
//                                                                              
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// macros for organizing the gravity computation via direct summation.          
//                                                                              
// we have to take care for sinks and non-sinks                                 
//                                                                              
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// These macros assume:                                                         
//                                                                              
// - vect    X0     position of left soul                                       
// - real    M0     mass of left soul                                           
// - vect    F0     force for left soul (if used)                               
// - real    P0     potential for left soul (if used)                           
// - vect    dR     to be filled with  X0-X_j                                   
// - real    D0     to be filled with  M0*M_j                                   
// - real    D1     to be filled with  norm(dR[J])+eps^2  on loading            
//                                                                              
////////////////////////////////////////////////////////////////////////////////
#define LOAD_G(RGHT)				\
  dR = X0 - cofm(RGHT);				\
  D1 = norm(dR) + EPQ;				\
  D0 = M0 * mass(RGHT)
//------------------------------------------------------------------------------
#define PUT_LEFT(RGHT)				\
  dR *= D1;					\
  P0 -= D0;					\
  F0 -= dR;
//------------------------------------------------------------------------------
#define PUT_RGHT(RGHT)				\
  dR          *= D1;				\
  RGHT->pot() -= D0;				\
  RGHT->acc() += dR;
//------------------------------------------------------------------------------
#define PUT_BOTH(RGHT)				\
  dR          *= D1;				\
  P0          -= D0;				\
  F0          -= dR;				\
  RGHT->pot() -= D0;				\
  RGHT->acc() += dR;
//------------------------------------------------------------------------------
#define PUT_SOME(RGHT)				\
  dR   *= D1;					\
  P0   -= D0;					\
  F0   -= dR;					\
  if(is_sink(RGHT)) {				\
    RGHT->pot() -= D0;				\
    RGHT->acc() += dR;				\
  }
////////////////////////////////////////////////////////////////////////////////
// below we only give code for global softening                                 
////////////////////////////////////////////////////////////////////////////////
#ifdef ALLOW_INDI
#  include <proprietary/kern_ind.cc>               // non-public: individual eps
#else
////////////////////////////////////////////////////////////////////////////////
#define GRAV_ALL(DSINGL,PUT)			\
for(register soul_iter B=B0; B!=BN; ++B) {	\
  LOAD_G(B);					\
  DSINGL					\
  PUT(B)					\
} 
//------------------------------------------------------------------------------
#define GRAV_FEW(DSINGL)			\
for(register soul_iter B=B0; B!=BN; ++B)	\
  if(is_sink(B)) {				\
    LOAD_G(B);					\
    DSINGL					\
    PUT_RGHT(B)					\
  } 
//------------------------------------------------------------------------------
// now defining auxiliary inline functions for the computation of  N            
// interactions. There are the following 10 cases:                              
// - each for the cases YA, YS, YN, NA, NS                                      
// - each for global and individual softening                                   
//------------------------------------------------------------------------------
#define ARGS					\
  kern_type const&KERN,				\
  soul_iter const&A,				\
  soul_iter const&B0,				\
  soul_iter const&BN,				\
  real      const&EPQ,				\
  real      const&HEQ,				\
  real      const&QEQ
//------------------------------------------------------------------------------
#define START					\
  const    real      M0=mass(A);		\
  const    vect      X0=cofm(A);		\
  register vect      dR;			\
  register real      D0,D1;
//------------------------------------------------------------------------------
namespace nbdy {
  //----------------------------------------------------------------------------
  // interactions 1 <==> N                                                      
  //----------------------------------------------------------------------------
  inline void many_YA_G(ARGS)
  {
    START;
    register real P0=zero;
    register vect F0=zero;
    switch(KERN) {
    case p1: GRAV_ALL(DSINGL_P1_G,PUT_BOTH) break;
    case p2: GRAV_ALL(DSINGL_P2_G,PUT_BOTH) break; 
    case p3: GRAV_ALL(DSINGL_P3_G,PUT_BOTH) break; 
    default: GRAV_ALL(DSINGL_P0_G,PUT_BOTH) break; 
    }
    A->pot() += P0;
    A->acc() += F0;
  }
  //----------------------------------------------------------------------------
  // interactions 1 <= [=> N ]                                                  
  //----------------------------------------------------------------------------
  inline void many_YS_G(ARGS)
  {
    START;
    register real P0=zero;
    register vect F0=zero;
    switch(KERN) {
    case p1: GRAV_ALL(DSINGL_P1_G,PUT_SOME) break;
    case p2: GRAV_ALL(DSINGL_P2_G,PUT_SOME) break;
    case p3: GRAV_ALL(DSINGL_P3_G,PUT_SOME) break;
    default: GRAV_ALL(DSINGL_P0_G,PUT_SOME) break;
    }									
    A->pot() += P0;
    A->acc() += F0;
  }
  //----------------------------------------------------------------------------
  // interactions 1 <=  N                                                       
  //----------------------------------------------------------------------------
  inline void many_YN_G(ARGS)
  {
    START;
    register real P0=zero;
    register vect F0=zero;
    switch(KERN) {
    case p1: GRAV_ALL(DSINGL_P1_G,PUT_LEFT) break;
    case p2: GRAV_ALL(DSINGL_P2_G,PUT_LEFT) break;
    case p3: GRAV_ALL(DSINGL_P3_G,PUT_LEFT) break;
    default: GRAV_ALL(DSINGL_P0_G,PUT_LEFT) break;
    }
    A->pot() += P0;
    A->acc() += F0;
  }
  //----------------------------------------------------------------------------
  // interactions 1 => N                                                        
  //----------------------------------------------------------------------------
  inline void many_NA_G(ARGS)
  {
    START;
    switch(KERN) {
    case p1: GRAV_ALL(DSINGL_P1_G,PUT_RGHT) break;
    case p2: GRAV_ALL(DSINGL_P2_G,PUT_RGHT) break;
    case p3: GRAV_ALL(DSINGL_P3_G,PUT_RGHT) break;
    default: GRAV_ALL(DSINGL_P0_G,PUT_RGHT) break;
    }
  }
  //----------------------------------------------------------------------------
  // interactions 1 [=> N ]                                                     
  //----------------------------------------------------------------------------
  inline void many_NS_G(ARGS)
  {
    START;
    switch(KERN) {
    case p1: GRAV_FEW(DSINGL_P1_G) break;
    case p2: GRAV_FEW(DSINGL_P2_G) break;
    case p3: GRAV_FEW(DSINGL_P3_G) break;
    default: GRAV_FEW(DSINGL_P0_G) break;
    }
  }
}                                                  // END: namespace nbdy       
#undef ARGS
#undef START
//------------------------------------------------------------------------------
// we now can define the cell-soul and cell-self interaction via direct sums    
//------------------------------------------------------------------------------
void grav_kern::many(cell_iter const&A, soul_iter const&B) const
{
  if(is_sink(B))
    switch(who_is_sink(A)) {
    case all: 
      many_YA_G(KERN,B,A.begin_souls(),A.end_soul_desc(),EPQ,HEQ,QEQ); break;
    case some:
      many_YS_G(KERN,B,A.begin_souls(),A.end_soul_desc(),EPQ,HEQ,QEQ); break;
    case none:
      many_YN_G(KERN,B,A.begin_souls(),A.end_soul_desc(),EPQ,HEQ,QEQ); break;
    }
  else
    switch(who_is_sink(A)) {
    case all:
      many_NA_G(KERN,B,A.begin_souls(),A.end_soul_desc(),EPQ,HEQ,QEQ); break;
    case some:
      many_NS_G(KERN,B,A.begin_souls(),A.end_soul_desc(),EPQ,HEQ,QEQ); break;
    case none: break;
    }
}
//------------------------------------------------------------------------------
void grav_kern::many(cell_iter const&C) const
{
  const    uint      N1 = number(C)-1;
  register uint      k, Nk;
  register soul_iter A = C.begin_souls();
  if(al_sink(C))
    for(k=0, Nk=N1; Nk!=0; --Nk, ++k, ++A)
      many_YA_G(KERN,A,A+1,A+1+Nk,EPQ,HEQ,QEQ);
  else
    for(k=0, Nk=N1; Nk!=0; --Nk, ++k, ++A)
      if(is_sink(A)) many_YS_G(KERN,A,A+1,A+1+Nk,EPQ,HEQ,QEQ);
      else           many_NS_G(KERN,A,A+1,A+1+Nk,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
// we now define non-inline functions for the computation of many direct        
// interactions between NA and NB souls                                         
// there are 8 cases, depending on whether all, some, or none of either A or    
// B are sinks (case none,none is trivial).                                     
//                                                                              
// these functions are called by grav_kern::many(cell,cell), which is inline in 
// kern.h, or by grav_kern::flush_scc() below.                                  
//------------------------------------------------------------------------------
void grav_kern::many_AA(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) many_YA_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_AS(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) many_YS_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_AN(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) many_YN_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_SA(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) 
    if(is_sink(A)) many_YA_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
    else           many_NA_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_SS(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) 
    if(is_sink(A)) many_YS_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
    else           many_NS_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_SN(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) 
    if(is_sink(A)) many_YN_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_NA(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) many_NA_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
//------------------------------------------------------------------------------
void grav_kern::many_NS(soul_iter const&A0, uint const&NA,
			soul_iter const&B0, uint const&NB) const
{
  const    soul_iter AN=A0+NA, BN=B0+NB;
  register soul_iter A,B;
  for(A=A0; A!=AN; ++A) many_NS_G(KERN,A,B0,BN,EPQ,HEQ,QEQ);
}
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// 2. approximate gravity                                                       
//                                                                              
////////////////////////////////////////////////////////////////////////////////
#if ALLOW_FULL_POT
# define CS_PUT					\
  if(FULL_POT) {				\
    CELL_BODY_FULL_POT(A,B,D0,D1,D2,D3,R)	\
  } else {					\
    CELL_BODY(A,B,D0,D1,D2,D3,R)		\
  }
#else
# define CS_PUT					\
  CELL_BODY(A,B,D0,D1,D2,D3,R)
#endif
//------------------------------------------------------------------------------
void grav_kern::grav(cell_iter const&A, soul_iter const&B,
		     vect           &R, real      const&Rq) const
{
  register real D0=mass(A)*mass(B), D1=Rq+EPQ, D2,D3;
  switch(KERN) {
  case p1: { ASINGL_P1_G CS_PUT } break;
  case p2: { ASINGL_P2_G CS_PUT } break;
  case p3: { ASINGL_P3_G CS_PUT } break;
  case p0: { ASINGL_P0_G CS_PUT } break;
  }
}
//------------------------------------------------------------------------------
#if ALLOW_FULL_POT
# define CC_PUT					\
  if(FULL_POT) {				\
    CELL_CELL_FULL_POT(A,B,D0,D1,D2,D3,R)	\
  } else {					\
    CELL_CELL(A,B,D0,D1,D2,D3,R)		\
  }
#else
# define CC_PUT					\
  CELL_CELL(A,B,D0,D1,D2,D3,R)
#endif
//------------------------------------------------------------------------------
void grav_kern::grav(cell_iter const&A, cell_iter const&B,
		     vect           &R, real      const&Rq) const
{
  register real D0=mass(A)*mass(B), D1=Rq+EPQ, D2,D3;
  switch(KERN) {
  case p1: { ASINGL_P1_G CC_PUT } break;
  case p2: { ASINGL_P2_G CC_PUT } break;
  case p3: { ASINGL_P3_G CC_PUT } break;
  case p0: { ASINGL_P0_G CC_PUT } break;
  }
}
////////////////////////////////////////////////////////////////////////////////
#endif // ALLOW_INDI                                                            
#endif // SSE_CODE                                                              
#ifndef ALLOW_INDI
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// 3. Single interaction                                                        
//                                                                              
////////////////////////////////////////////////////////////////////////////////
#ifdef SSE_CODE
void grav_kern_sse
#else
void grav_kern
#endif
::single(soul_iter const &A, soul_iter const&B) const
{
  register vect R  = cofm(A)-cofm(B);
  register real Rq = norm(R),x,D0;
  switch(KERN) {
  case p0: { P0(mass(A)*mass(B))     } break;
  case p1: { P1(mass(A)*mass(B))     } break;
  case p2: { P2(mass(A)*mass(B))     } break;
  case p3: { P3(mass(A)*mass(B))     } break;
  default: { NEWTON(mass(A)*mass(B)) } break;
  }
  if(is_sink(A)) { A->pot()-=D0; A->acc()-=R; }
  if(is_sink(B)) { B->pot()-=D0; B->acc()+=R; }
}
////////////////////////////////////////////////////////////////////////////////
#endif // ALLOW_INDI                                                            

