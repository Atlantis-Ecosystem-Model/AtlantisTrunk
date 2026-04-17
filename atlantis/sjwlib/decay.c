/*

    File:           decay.c
    
    Created:        Tue Dec 6 09:06:58 EST 1994
    
    Author:         Stephen Walker
                    CSIRO Division of Oceanography
    
    Purpose:        The decay equation is

			C = Co exp(-kt)

		    so that

			dC
			-- = -kC
			dt
		    
		    This file contains routines to implement one
		    time step of the above equation using forward,
		    centered or backward time steps, as well as an
		    exact method requiring a call to exp().
    
    Arguments:      c  - current tracer concentration
		    k  - decay rate
		    dt - time step

    Returns:        new concentration (double)
    
    Revisions:      none

    $Id: decay.c 2761 2011-07-01 04:35:25Z gor171 $

*/

#include <stdio.h>
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>

/** Decay using a forward time step. Numerically, this is as follows:
  *
\begin{verbatim}
C(t+dt) = C(t)*(1-k*dt).
\end{verbatim}
  *
  * @param c concentration
  * @param k decay rate
  * @param dt decay interval
  */
double decay_forward(double c, double k, double dt)
{
    return( c*(1.0-k*dt) );
}


/** Decay using a centered time step. Numerically, this is as follows:
  * 
\begin{verbatim}
           2-k*dt
 C(t+dt) = ------ C(t)
	   2+k*dt
\end{verbatim}
  * 
  */
double decay_centered(double c, double k, double dt)
{
    return( c*(2.0-k*dt)/(2.0+k*dt) );
}

/** Decay using a backward time step. Numerically, this is as follows:
  * 
\begin{verbatim}
            1
C(t+dt) = ------ C(t)
   	  1+k*dt
\end{verbatim}
  * 
  */
double decay_backward(double c, double k, double dt)
{
    return( c/(1.0+k*dt) );
}

/** Decay using an exact time step. Numerically, this is as follows:
  * 
\begin{verbatim}
C(t+dt) = C(t) exp(-k*dt)
\end{verbatim}
  * 
  */
double decay_exact(double c, double k, double dt)
{
    static double last_kdt = 0.0;
    static double v = 1.0;
    double kdt = -k*dt;
    double ans = 0.0;

    if( kdt != last_kdt ) {
	v = exp(kdt);
	last_kdt = kdt;
    }

    ans = c*v;
    if (!_finite(ans)){  // Sanity trap
        ans = 0.0;
    }
    
    return(ans);
}
