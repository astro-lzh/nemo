// -*- C++ -*-                                                                  
////////////////////////////////////////////////////////////////////////////////
///                                                                             
/// \file   src/public/manip/randomize_azimuth.cc                               
///                                                                             
/// \author Walter Dehnen                                                       
/// \date   2004-2006                                                           
///                                                                             
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// Copyright (C) 2004-2006 Walter Dehnen                                        
//                                                                              
// This program is free software; you can redistribute it and/or modify         
// it under the terms of the GNU General Public License as published by         
// the Free Software Foundation; either version 2 of the License, or (at        
// your option) any later version.                                              
//                                                                              
// This program is distributed in the hope that it will be useful, but          
// WITHOUT ANY WARRANTY; without even the implied warranty of                   
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            
// General Public License for more details.                                     
//                                                                              
// You should have received a copy of the GNU General Public License            
// along with this program; if not, write to the Free Software                  
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                    
//                                                                              
////////////////////////////////////////////////////////////////////////////////
#include <public/defman.h>
#include <public/random.h>
#include <ctime>
#include <cmath>

namespace falcON { namespace Manipulate {
  // ///////////////////////////////////////////////////////////////////////////
  //                                                                            
  // class randomize_azimuth                                                    
  //                                                                            
  /// manipulator: rotates vectors by random azimuth for bodies in_subset()     
  ///                                                                           
  /// This manipulator rotates all vector properties (position, velocity,       
  /// acceleration, jerk, auxiliary vector, predicted velocity) for each body   
  /// in_subset() (default: all, see set_subset) by a random angle around the   
  /// z-axis.\n                                                                 
  /// Useful to suppress the growth of non-axisymmetric modes in disk galaxies. 
  ///                                                                           
  /// Meaning of the parameters:\n                                              
  /// par[0]: seed for RNG (default: seconds since 1st January 1970)            
  ///                                                                           
  /// Usage of pointers: none\n                                                 
  /// Usage of flags:    uses in_subset()\n                                     
  ///                                                                           
  // ///////////////////////////////////////////////////////////////////////////
  class randomize_azimuth : public manipulator {
  private:
    const Random3 Ran;
    //--------------------------------------------------------------------------
    static void rotate(vect&x, real const&c, real const&s) {
      register real 
	t0 = c*x[0] + s*x[1];
      x[1] = c*x[1] - s*x[0];
      x[0] = t0;
    }
    //--------------------------------------------------------------------------
    static void rotate(vect&x, vect const&o, real const&c, real const&s) {
      register real 
	t0 = o[0] + c*(x[0]-o[0]) + s*(x[1]-o[1]);
      x[1] = o[1] + c*(x[1]-o[1]) - s*(x[0]-o[0]);
      x[0] = t0;
    }
    //--------------------------------------------------------------------------
  public:
    const char* name    () const { return "randomize_azimuth"; }
    const char* describe() const {
      return 
	"randomize azimuth of vectors for bodies in_subset() (default: all)";
    }
    //--------------------------------------------------------------------------
    fieldset need   () const { return fieldset::o; }
    fieldset provide() const { return fieldset::o; }
    fieldset change () const { return fieldset::vectors; }
    //--------------------------------------------------------------------------
    bool manipulate(const snapshot*) const;
    //--------------------------------------------------------------------------
    randomize_azimuth(const double*pars,
		      int          npar,
		      const char  *file) :
      Ran (npar>0? long(pars[0]) : long(time(0)) )
    {
      if(npar<1 && debug(1) || debug(2))
	std::cerr<<
	  " Manipulator \""<<name()<<"\":\n"
	  " randomizes azimuth of vectors for all bodies in"
	  " 'subset' (default: all)\n"
	  " parameter: seed for RNG (default: secs since 1970)\n";
      if(npar>1 && debug(1))
	warning(" Manipulator \"%s\": skipping parameters beyond 1\n",name());
    }
    //--------------------------------------------------------------------------
    ~randomize_azimuth() {}
  };
  //////////////////////////////////////////////////////////////////////////////
  bool randomize_azimuth::manipulate(const snapshot*S) const {
    if(S->have_some(fieldset::vectors)) 
      LoopSubsetBodies(S,b) {
	double phi = TPi * Ran();
	register real
	  c = std::cos(phi),
	  s = std::sin(phi);
	if(S->have(fieldbit::x)) rotate(b.pos (), c,s);
	if(S->have(fieldbit::v)) rotate(b.vel (), c,s);
	if(S->have(fieldbit::a)) rotate(b.acc (), c,s);
	if(S->have(fieldbit::j)) rotate(b.jerk(), c,s);
	if(S->have(fieldbit::w)) rotate(b.vprd(), c,s);
	if(S->have(fieldbit::z)) rotate(b.zet (), c,s);
      }
    return false;
  }
  //////////////////////////////////////////////////////////////////////////////
} }

__DEF__MAN(falcON::Manipulate::randomize_azimuth)