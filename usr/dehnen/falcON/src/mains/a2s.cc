// -*- C++ -*-                                                                  
////////////////////////////////////////////////////////////////////////////////
///                                                                             
/// \file   src/mains/a2s.cc                                                    
///                                                                             
/// \author Walter Dehnen                                                       
/// \date   2002-2006                                                           
///                                                                             
////////////////////////////////////////////////////////////////////////////////
//                                                                              
// Copyright (C) 2002-2006 Walter Dehnen                                        
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
//                                                                              
// history:                                                                     
//                                                                              
// v 0.0   24/11/2002  WD created.                                              
// v 0.1   20/06/2003  WD updated & debugged                                    
// v 0.2   29/10/2003  WD automated version, compiler in version etc            
// v 0.3   25/02/2004  WD added SPH items, option Ns                            
// v 0.3.1 04/03/2005  WD updated list in defv[] for read; use falcON::input    
// v 1.0   12/04/2005  WD fixed bug related to SPH quantities                   
// v 1.1   20/05/2005  WD several minor updates                                 
// v 2.0   16/06/2005  WD new falcON                                            
// v 2.1   27/06/2005  WD deBUGged                                              
// v 2.2   04/07/2006  WD made public (in the hope that it's bug free ... )     
////////////////////////////////////////////////////////////////////////////////
#define falcON_VERSION   "2.2"
#define falcON_VERSION_D "04-jul-2006 Walter Dehnen                          "
//-----------------------------------------------------------------------------+
#ifndef falcON_NEMO                                // this is a NEMO program    
#  error You need NEMO to compile "a2s"
#endif
#define falcON_RepAction 0                         // no action reporting       
#include <body.h>                                  // bodies etc..              
#include <public/io.h>                             // WDs C++ NEMO I/O          
#include <main.h>                                  // NEMO basics & main        
//------------------------------------------------------------------------------
string defv[] = {
  "in=???\n           input ascii file                                   ",
  "out=???\n          output snapshot file                               ",
  "N=???\n            number of lines (ignore lines starting with '#'    ",
  "Ns=0\n             first Ns lines to contain SPH data                 ",
  "read=???\n         items to read (in given order). Recognizing:\n"
  "                    m: mass\n"
  "                    x: position\n"
  "                    v: velocity\n"
  "                    a: acceleration\n"
  "                    j: jerk (not used in falcON)\n"
  "                    p: N-body potential\n"
  "                    q: external potential\n"
  "                    e: individual eps_i\n"
  "                    l: time-step level\n"
  "                    k: integer key\n"
  "                    f: body flag\n"
  "                    r: density estimate\n"
  "                    y: auxiliary scalar\n"
  "                    z: auxiliary vector\n"
  "                    H: SPH smoothing length\n"
  "                    N: SPH number of neighbours\n"
  "                    U: SPH internal energy\n"
  "                    I: SPH (dU/dt)_total\n"
  "                    E: SPH (dU/dt)_external\n"
  "                    S: SPH entropy or entropy function\n"
  "                    R: SPH gas density\n"
  "                    C: SPH sound speed\n"
  "                    D: SPH drho/dt\n"
  "                    J: SPH dh/dt or dlnh/dt\n"
  "                    F: SPH factor f_i                                 ",
  "write=\n           items to write [default: read]                     ",
  "time=0\n           time for snapshot                                  ",
  falcON_DEFV, NULL };
//------------------------------------------------------------------------------
string usage = "a2s -- Walter's simple ascii to snapshot converter";
//------------------------------------------------------------------------------
void falcON::main() falcON_THROWING
{
  input in(getparam("in"));
  if(!in) falcON_THROW("cannot open ascii input file\"%s\"",getparam("in"));
  fieldbit item[32] = { fieldbit::invalid };
  int   K=0;
  const char*get = getparam("read");
  for(; K!=32 && get[K]; ++K)
    item[K] = fieldbit(get[K]);
  snapshot shot (getdparam("time"));
  shot.read_simple_ascii(in, item, K, getiparam("N"), getiparam("Ns"));
  nemo_out out  (getparam("out"));
  fieldset read (getioparam("read"));
  fieldset write(hasvalue("write")?  getioparam("write") : read);
  if(!read.contain(write))
    warning("cannot write '%s' data (only read '%s')",
	    word(write & ~read), word(read));
  write &= read;
  shot.write_nemo(out,write);
}
