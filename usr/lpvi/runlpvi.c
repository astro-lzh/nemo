/*
 *  runlpvi:   NEMO frontend for LP-VIcode 
 *	
 *  29-dec-2019  V0.1  initial draft                  PJT
 *                0.3  multiple orbits in posvel=
 *
 *  See also http://lp-vicode.fcaglp.unlp.edu.ar/
 *
 */

#include <stdinc.h>
#include <getparam.h>
#include <history.h>
#include <filefn.h>
#include <run.h>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <vectmath.h>
#include <filestruct.h>


string defv[] = {
    "outdir=???\n        Output run directory (required)",
    "posvel=???\n        Initial conditions (4 or 6 values)",
    "prefix=log\n        Prefix for output files",
    "dt=0.01\n           Integration step",
    "tstop=1\n           Time of integration",
    "screen=1\n          Screen output?",
    "orbit=0\n           Orbit output?",
    "ci=0,0,0,0,0,0,0\n  CI indictors:  LIs, SALI, GALIs, SD & SSNs, RLI & LImax, MEGNO & SElLCE, FLI & OFLI",
    "nstep=1\n           Steps until Output",
    "dev=1\n             Initial dev. vectors (0 = at random, 1 = random orthonormal, 2=fixed)",
    "sali=0\n            SALI & potential(t): normal saturation (=0) or restart dev. vectors (=1)",

    "ndim=\n             Not used yet, but might be needed if posvel ambiguous",
    "exe=LP-VIcode\n     Name of the fortran executable",

    "VERSION=0.3\n       3-jan-2012 PJT",
    NULL,
};

string usage="NEMO front end for LP-VIcode";

#define MAXCI 7
#define MAXORB 1000

void nemo_main(void)
{
    int i, npv, ndim;
    string exefile = getparam("exe");
    string infile = "LP-VIcode.in";
    string parfile = "LP-VIcode.par";
    string datfile = "posvel.dat";
    string rundir = getparam("outdir");
    string fname;
    char dname[256], runcmd[256];
    real posvel[MAXORB*7];
    int nci, ci[MAXCI];
    stream ostr, datstr, histr;

    if (hasvalue("ndim"))
      ndim = getiparam("ndim");
    else
      ndim = -1;

    npv = nemoinpr(getparam("posvel"),posvel,MAXORB*7);
    if (npv==6 && ndim<0)
      ndim = 3;
    else if (npv==4 && ndim<0)
      ndim = 2;
    else if (ndim<0) 
      error("Need (multiple of) 4 or 6 values for posvel, or specifiy ndim=");

    nci = nemoinpi(getparam("ci"),ci,MAXCI);
    if (nci<0) error("Error ci=");
    for (i=nci; i<MAXCI; ++i) ci[i] = 0;

    run_mkdir(rundir);
    run_cd(rundir);

    // be nice and write a (NEMO) history file
    
    histr = stropen("history","w");
    put_history(histr);
    strclose(histr);


    // write the required .par file

    ostr = stropen(parfile,"w");
    fprintf(ostr,"* LP-VIcode.par : generated by NEMO's runlpvi\n");
    fprintf(ostr,"      INTEGER ndim\n");
    fprintf(ostr,"      PARAMETER (ndim=%d)\n",ndim);
    strclose(ostr);

    // write the initial conditions file
    // assume 1 orbit for now, no tstop per orbit

    ostr = stropen(datfile,"w");
    for (i=0; i<npv; ++i) {
      if (i>0 && i%(ndim*2)==0) fprintf(ostr,"\n");
      fprintf(ostr,"%f ",posvel[i]);
    }
    fprintf(ostr,"\n");
    strclose(ostr);

    // write the required .in file (comment lines are significant by the fact they need to exist)

    ostr = stropen(infile,"w");
    fprintf(ostr,"# LP-VIcode.in : generated by NEMO's runlpvi\n");
    fprintf(ostr,"# \n");
    fprintf(ostr,"%s\n",datfile);
    fprintf(ostr,"# \n");
    fprintf(ostr,"%s\n",getparam("prefix"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"%g\n",getrparam("dt"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"%g\n",getrparam("tstop"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"%d %d\n",getiparam("screen"),getiparam("orbit"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"# \n");
    fprintf(ostr,"%d %d %d %d %d %d %d\n",ci[0],ci[1],ci[2],ci[3],ci[4],ci[5],ci[6]);
    fprintf(ostr,"# \n");
    fprintf(ostr,"%d\n",getiparam("nstep"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"%d\n",getiparam("dev"));
    fprintf(ostr,"# \n");
    fprintf(ostr,"%d\n",getiparam("sali"));
    strclose(ostr);

    // run!
    
    sprintf(runcmd,"%s",exefile);
    run_sh(runcmd);
}
