/**
 \file
 \brief Routines to do LU decomposition and Simplex method.
 \ingroup atassess

 File:           atsolve.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to do LU decomposition and Simplex method.

 LU decomposition of a matrix makes its
 possible to solve simultaneous equations
 using X = A.inv(B). If A.X = B and A is square (nxn)
 and B is a column vector with n components, then
 X = A\B = Ainv(B) is the solution to the equation
 A.X = B computed by LU decomposition

 Simplex method = amoeba (= minimisation or model fitting tool)

 Arguments:

 Returns:        void

 Revisions:      8/8/2004 Created from southeast solve.c

 15/9/2004 Added Amoeba code and amotry

 20/9/2004 Added AmoebaL() so can have cut down Amoeba call.

 12-11-2010 Bec Gorton
 Commented out the LU functions. These have been replaced with some open source code in doolittle.c

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

#define SWAP(a,b) swap=(a);(a)=(b);(b)=swap;


/* VPA specific routines */
static void Back_Calc(int ndim, int nchrt, int YrMax, double prm_sp);
static void VPA_Funk_Val(int ndim, int nchrt, int YrMax, double *SS);
static void Solve_Back(int Yr, int age, int ndim, double M_sp);
static void Solve_Plus(int Yr, int nchrt, int ndim, double M_sp);

/* Production model routines */
static void Prod_Calc(MSEBoxModel *bm,int ndim, int sp, int nchrt, int YrMax, double prm_sp, FILE *ofp);
static void Prod_Funk_Val(int ndim, int sp, int nchrt, int YrMax, double *SS);

/* Linear regression routines */
static void Linear_Regression(MSEBoxModel *bm, int funkflag, int sp, double *X, int ndim, double **P, double *Y, int YrMax, int ESTQ, FILE *ofp);
static void Regression_CPUE_Funk_Val(int ndim, int nid, int YrMax, double *X, double *SS);
static void Regression_Funk_Val(int ndim, int YrMax, double *X, double *SS);

/**
 * static function definitions
 */
static double Amotry(MSEBoxModel *bm, double **p, double *y, double *psum, int ndim, int funkflag, int ihi,
		int *nfunk, double fac, int nchrt, int YrMax, double prm_sp, int sp, FILE *ofp, double *xpar);

void AmoebaL(MSEBoxModel *bm, double **p, double *y, int ndim, double ftol, int itmax, int funkflag, int nchrt, int YrMax, double prm_sp,
		int sp, int *nfunk, int *ilow, FILE *ofp, double *xpar) {
	/* Not all calls to Amoeba will have the extra info needed by the Assessment model calls
	 so reflect this in the two step call */

	char *dummyname = "blank";

	Amoeba(bm, 0, sp, 1.0, dummyname, p, y, ndim, ftol, itmax, funkflag, nchrt, YrMax, prm_sp, nfunk, ilow, ofp, NULL);

	return;
}

void Amoeba(MSEBoxModel *bm, int assessing, int sp, double dayt, char* speciesname, double **p, double *y, int ndim, double ftol,
		int itmax, int funkflag, int nchrt, int YrMax, double prm_sp, int *nfunk, int *ilow, FILE *ofp, double *xpar)
/* Downhill Simplex (amoeba) routine from Numerical Recipes second edition pg 308 */
{
	/* Multidimensional minimisation  of the function funk(x) where x is an ndim
	 dimensional vector by the downhill simplex method of Nelder and Mead 1965. The matrix
	 P[1..ndim+1][1..ndim] is input, the ndim+1 rows are ndim vectors defining the
	 vertices of the starting simplex. Also input is the ndim+1 vector Y, whose
	 components must be pre-initialised to the values of funk() evaluated at the ndim+1
	 vetrices of P; and ftol the fractional convergence tolerance to be achieved in the
	 function value. On output p and y will have been reset to ndim+1 new points all
	 within ftol of a minimum function value, and nfunk gives the number of function
	 evaluations taken.
	 */

	int i, j, ilo, ihi, inhi;
	//int mpts=ndim+1;
	int mpts = ndim;
	double ytry, ysave, sum, rtol, swap;
	double *psum = alloc1d(ndim); /* Temporary storage */

	*nfunk = 0;

	for (j = 0; j < ndim; j++) {
		for (sum = 0.0, i = 0; i < mpts; i++) {   // Note added the sum to be in line with Numerical Recipes
			sum += p[i][j];
		}
        psum[j] = sum;
	}

	for (;;) {
		/* First determine which point is highest (worst), next highest, and lowest (best)
		 by looping over the points in the simplex */
		ilo = 0;
		ihi = 1;
		ihi = y[0] > y[1] ? (inhi = 1, 0) : (inhi = 0, 1);
		for (i = 0; i < mpts; i++) {
			if (y[i] < y[ilo])
				ilo = i;
			if (y[i] > y[ihi]) {
				inhi = ihi;
				ihi = i;
			} else if (y[i] > y[inhi]) {
				if (i != ihi)
					inhi = i;
			}
		}
		rtol = 2.0 * fabs(y[ihi] - y[ilo]) / (fabs(y[ihi]) + fabs(y[ilo]) + small_num);

		/* Compute the fractional range from the highest to lowest
		 and return if satisfactory */
		if (rtol < ftol) {
			// First version just had break here, but the book has additional content so adding that again here
			// to be in line with Numerical Recipes. Maybe add an if (!assessing) test here so consistent with old approach?

			// If returning, put best point and value in slot 1
			SWAP(y[0],y[ilo]); 
			for(i=0; i<ndim; i++){
				SWAP(p[1][i],p[ilo][i]);
			}
			break;
		}
		if (*nfunk >= itmax) {
			*ilow = ilo;
			if (assessing)
				fprintf(ofp, "Time: %e assessment of %s has failed to converge\n ", dayt, speciesname);
			else
				fprintf(ofp, "Time %e downhill_simplex (amoeba) failed to converge\n", dayt);
			return;
		}
		*nfunk += 2;				// Added this line to be in line with Numerical Recipes

		/* Begin a new iteration */
		/* First extrapolate by a factor alpha through the face of the simplex
		 across from the high point (i.e. reflect the simplex from the high point) */
		ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, -alpha, nchrt, YrMax, prm_sp, sp, ofp, xpar);
		if (ytry <= y[ilo]) {
			/* Gives a result better than the best point, so try an additional extrapolation by a factor gamma */
			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, gamma, nchrt, YrMax, prm_sp, sp, ofp, xpar);
		} else if (ytry >= y[inhi]) {
			/* The reflected point is worse then the second highest, so look for
			 an intermediate lower point (i.e. do a 1-dimensional contraction). */
			ysave = y[ihi];
			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, beta, nchrt, YrMax, prm_sp, sp, ofp, xpar);
			if (ytry >= ysave) {
				/* Can't seem to get rid of that high point. Better contract
				 around the lowest (best) point */
				for (i = 0; i < mpts; i++) {
					if (i != ilo) {
						for (j = 0; j < ndim; j++) {
							psum[j] = 0.5 * (p[i][j] + p[ilo][j]);
							p[i][j] = psum[j];
						}
						y[i] = Funk(bm, funkflag, sp, nchrt, ndim, YrMax, prm_sp, psum, ofp, xpar);
					}
				}
				/* Keep track of function evaluations */
				(*nfunk) += ndim;

				/* Recompute psum */
				for (j = 0; j < ndim; j++) {
					for (sum = 0.0, i = 0; i < mpts; i++) {  // Note added the sum to be in line with Numerical Recipes
						sum += p[i][j];
						psum[j] = sum;
					}
				}
			}
		} else -- (*nfunk);   // Correct the evaluation count       // Added this line to be in line with Numerical Recipes

		/* Go back for the test of doneness and the next iteration */
	}

	*ilow = ilo;

	/* Free local array */
	free1d(psum);

	return;
}

/**
 * \brief Extrapolates by a factor fac through the face of the simplex across from the high point,
 * tries it, and replaces the high point if the new point is better
 */
double Amotry(MSEBoxModel *bm, double **p, double *y, double *psum, int ndim, int funkflag, int ihi,
		int *nfunk, double fac, int nchrt, int YrMax, double prm_sp, int sp, FILE *ofp, double *xpar)
{
	int j;
	double fac1, fac2, ytry;
	double *ptry = alloc1d(ndim); /* Temporary storage */

	fac1 = (1.0 - fac) / ndim;
	fac2 = fac1 - fac;
	for (j = 0; j < ndim; j++) {
		ptry[j] = psum[j] * fac1 - p[ihi][j] * fac2;
	}

	/* Evaluate the function at the trial point */
	ytry = Funk(bm, funkflag, sp, nchrt, ndim, YrMax, prm_sp, ptry, ofp, xpar);

	(*nfunk)++;

	/* If its better than the highest, the replace the highest */
	if (ytry < y[ihi]) {
		y[ihi] = ytry;
		for (j = 0; j < ndim; j++) {
			psum[j] += ptry[j] - p[ihi][j];
			p[ihi][j] = ptry[j];
		}
	}

	/* Free local array */
	free1d(ptry);

	/* Return the answer */
	return ytry;

}


/**
 * \brief Powell routine - Numerical recipes
 */
void powell(MSEBoxModel *bm, int sp, double *xf, double **xunit, int npar, double ftol, int *iter, double *ss, int funkflag) {

	return;
}


/********************** Model fitting function (funk) **************************/
double Funk(MSEBoxModel *bm, int funkflag, int sp, int nchrt, int ndim, int YrMax, double prm_sp, double *X, FILE *ofp, double *xpar) {
	int age, i;
	double SS = 0;
	double ans = 0;

	switch (funkflag) {
	case no_assess: /* No assessment */
		break;
	case schafer_model: /* Schafer production model assessment function */
		for (i = 0; i < est_B0_id + 1; i++) {
			NEst[YrMax][i] = exp(X[i]);
		}
		Prod_Calc(bm, ndim, sp, nchrt, YrMax, prm_sp, ofp);
		Prod_Funk_Val(ndim, sp, nchrt, YrMax, &SS);

		if (SS < SSmin)
			SSmin = SS;
		ans = SS;

		break;
	case VPA_model: /* ADAPT VPA assessment functions */
		for (age = 0; age < nchrt; age++) {
			NEst[YrMax][age] = exp(X[age]);
		}
		Back_Calc(ndim, nchrt, YrMax, prm_sp);
		VPA_Funk_Val(ndim, nchrt, YrMax, &SS);

		if (SS < SSmin)
			SSmin = SS;

		ans = SS;
		break;
	case MSVPA_model: /* MSVPA assessment functions */
	case CAB_model: /* Integrated assessment functions */
	case CAB_mimic: /* Integrated assessment mimic */
    case SS3_model: /* SS3 assessment called directly */
		break;
	case qlinear_regress: /* Linear regression */
		Regression_Funk_Val(ndim, YrMax, X, &SS);
		ans = SS;
		break;
	case CPUE_linear_regress: /* Linear regression */
		Regression_CPUE_Funk_Val(ndim, sp, YrMax, X, &SS);
		ans = SS;
		break;
	case EquilF_Funk_model:
		EquilF_Funk(bm, sp, xpar, ofp);
		break;
	case SurplusProduction_model:
		SurplusProduction(bm, sp, xpar, ofp);
		break;
	default:
		quit("No such function flag defined - how did it get here?\n");
		break;
	}

	return ans;
}

/** Supporting routines required to calculate stock estimates during VPA model fitting **/
/**
 * \brief For VPA assessment, back project for all ages and years
 */
void Back_Calc(int ndim, int nchrt, int YrMax, double prm_sp) {
	int Yr, age;

	for (Yr = YrMax; Yr > 0; Yr--) {
		for (age = 1; age < nchrt - 1; age++) {
			Solve_Back(Yr, age, ndim, prm_sp);
		}
		Solve_Plus(Yr - 1, nchrt, ndim, prm_sp);
	}

	return;

}

void Solve_Plus(int Yr, int nchrt, int ndim, double M_sp) {
	double Fmin, Fmax, Nmax1, Nmax2, Nproj, Nhit, FF, ZZ;
	int II;

	Fmin = 0;
	Fmax = 3;
	Nhit = NEst[Yr + 1][nchrt - 1];

	for (II = 0; II < ndim; II++) {
		FF = (Fmin + Fmax) / 2.0;
		ZZ = FF + M_sp;
		Nmax1 = (CData[Yr][nchrt - 2] * ZZ / FF) / (1.0 - exp(-ZZ));
		Nmax2 = (CData[Yr][nchrt - 1] * ZZ / FF) / (1.0 - exp(-ZZ));
		Nproj = (Nmax1 + Nmax2) * exp(-ZZ);
		if (fabs(Nproj - Nhit) < 0.01) {
			F[Yr][nchrt - 2] = FF;
			NEst[Yr][nchrt - 2] = Nmax1;
			F[Yr][nchrt - 1] = FF;
			NEst[Yr][nchrt - 1] = Nmax2;
		}
		if (Nproj > Nhit)
			Fmin = FF;
		else
			Fmax = FF;
	}

	return;
}

/**
 * \brief Back projection
 */
void Solve_Back(int Yr, int age, int ndim, double M_sp) {
	double ZZ, FF, Fmin, Fmax, Nback, CTarg, CProj;
	int II;

	Fmin = 0;
	Fmax = 3;
	CTarg = CData[Yr - 1][age - 1];
	for (II = 0; II < ndim; II++) {
		FF = (Fmin + Fmax) / 2.0;
		ZZ = M_sp + FF;
		Nback = NEst[Yr][age] * exp(ZZ);
		CProj = (FF / ZZ) * Nback * (1.0 - exp(-ZZ));
		if (fabs(CProj - CTarg) < 0.001) {
			F[Yr - 1][age - 1] = FF;
			NEst[Yr - 1][age - 1] = Nback;
		}
		if (CProj > CTarg)
			Fmax = FF;
		else
			Fmin = FF;
	}

	return;
}

/**
 * \brief This routine calculates sum of squares for VPA model fit
 */
void VPA_Funk_Val(int ndim, int nchrt, int YrMax, double *SS) {
	int age, Yr;
	double Qval, Nval, Error;

	*SS = 0;

	for (age = 0; age < nchrt; age++) {
		/* Estimate Q */
		Nval = 0;
		Qval = 0;
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			if (IData[Yr][age] > 0) {
				Nval++;
				Qval += log(IData[Yr][age] / (NEst[Yr][age] + small_num));
			}
		}
		Qval = exp(Qval / (Nval + small_num));

		/* Estimate contribution to SS */
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			if (IData[Yr][age] > 0) {
				/* Calculate residuals */
				Error = log(IData[Yr][age]) - log(NEst[Yr][age] * Qval);
				Resu[Yr][age] = Error;
				IDatahat[Yr][age] = NEst[Yr][age] * Qval;
				(*SS) += Error * Error;
			}
		}
	}

	return;
}

/** Supporting routines required to calculate stock estimates during VPA model fitting **/
/**
 * \brief Calculate all aspects of production model, estimate biomass and cpue so can calculate SS
 */
void Prod_Calc(MSEBoxModel *bm,int ndim, int sp, int nchrt, int YrMax, double prm_sp, FILE *ofp) {
	int Yr, model_state, p_dynamic = 0, q_dynamic = 0;
	double r, K, B0, step1, step2, step3, Biom_estimate, p, avgq, q, qinc;

	/* Determine production model assumptions being used
	 prm_sp = 0: Constant q and p
	 prm_sp = 1: Constant q, assymetric p
	 prm_sp = 2: Changing q, constant p
	 prm_sp = 3: changing q, assymetric p
	 */
	model_state = (int) (prm_sp);

	switch (model_state) {
	case 0:
		p_dynamic = 0;
		q_dynamic = 0;
		break;
	case 1:
		p_dynamic = 1;
		q_dynamic = 0;
		break;
	case 2:
		p_dynamic = 0;
		q_dynamic = 1;
		break;
	case 3:
		p_dynamic = 1;
		q_dynamic = 1;
		break;
	default:
		quit("No such VPA model_state defined - how did it get here?\n");
		break;
	}

	/* Assumes latest estimates of r, K, B0 are stored in NEst[YrMax][i] */
	r = NEst[YrMax][est_r_id] / 100.0;
	K = NEst[YrMax][est_K_id] * 1000.0;
	B0 = NEst[YrMax][est_B0_id] * 1000.0;

	if (p_dynamic) {
		/* Allow for assymetric production, but constrain so that p>0 always */
		step1 = max(0.1, NEst[YrMax][est_p_id]);
		p = step1 / 1000000.0;
	} else
		p = 1.0 / 1000000.0;

	/* Calculate estimates of biomass */
	NEst[0][est_B_id] = B0;
	for (Yr = 1; Yr < YrMax + 1; Yr++) {
		step1 = NEst[Yr - 1][est_B_id] / (K + small_num);
		step2 = pow(step1, p);
		Biom_estimate = NEst[Yr - 1][est_B_id] + NEst[Yr - 1][est_B_id] * (r / p) * (1.0 - step2) - CTData[Yr];
		/* Put in constraint to prevent negative biomasses */
		NEst[Yr][est_B_id] = max(1.0, Biom_estimate);
	}

	/* Calculate estimates of q */
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		NEst[Yr][est_q_id] = log(CTData[Yr] / (NEst[Yr][est_B_id] + small_num));
	}

	if (q_dynamic) {
		/* Changing catchabillity so perform a linear regression */

		/* Set starting points for minimisation */
		XX[0] = 1.0;
		XX[1] = 1.0;

		Linear_Regression(bm, qlinear_regress, sp, XX, ndim, PP, YY, YrMax, 1, ofp);

		/* Get results of regression - need to take exponent as linear
		 regression dealt with logged ratios */
		q = exp(XX[0]);
		qinc = exp(XX[1]);
	} else {
		/* Constant (estimated) q */

		avgq = 0;
        for (Yr = 0; Yr < YrMax + 1; Yr++) {
            avgq += NEst[Yr][est_q_id];
        }

		avgq /= ((double)(YrMax + 1.0));

		q = exp(avgq); /* Take the exponent as NEst[Yr][est_q_id] are logged ratios */
		qinc = 1.0;

	}

	/* Update q predictions */
	NEst[0][est_q_id] = q;
	for (Yr = 1; Yr < YrMax + 1; Yr++) {
		NEst[Yr][est_q_id] = NEst[Yr - 1][est_q_id] * qinc;
	}

	/* Calculate predicted CPUE (IData) */
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		NEst[Yr][est_I_id] = NEst[Yr][est_B_id] * NEst[Yr][est_q_id];
	}

	/* Calculate MSY */
	step1 = (p + 1.0);
	step2 = ((p + 1.0) / (p + small_num));
	step3 = pow(step1, step2);
	NEst[YrMax][est_msy_id] = (r * K) / (step3 + small_num);

	return;
}

/**
 * \brief This routine calculates minisattion criteria for Schaefer Production model fit.
 * Not using sum of squares, bur using log likelihood - as suggested in Chapter 10
 * of Modelling and Quantitative Methods in Fisheries by Malcolm Haddon
 * */
void Prod_Funk_Val(int ndim, int sp, int nchrt, int YrMax, double *SS) {
	int Yr;
	double Nval, Lval, SSQval, SSavg, Error;

	*SS = 0;

	/* Calculate SS */
	Nval = 0;
	Lval = 0;
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		Nval++;
		SSQval = log(ITData[Yr]) - log(NEst[Yr][est_I_id]);
		SSQval *= SSQval;
		Lval += SSQval;
	}

	SSavg = sqrt(Lval / (Nval + small_num));
	*SS = -(Nval / 2.0) * (log(2.0 * 3.1415926) + 2.0 * log(SSavg) + 1.0);

	/* Calculate residuals - as no age structure carried, store results in entry zero (0) */
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		Error = ITData[Yr] / (NEst[Yr][est_I_id] + small_num);
		Resu[Yr][0] = Error;
	}

	return;
}

/*****************/
/**
 *  Linear regression - least squares fitting of line y = b + ax
 *
 *	\brief This routine fits a linear regression model of the form Y = A1 + A2*X by doing least squares minimisation using amoeba
 *
 */
void Linear_Regression(MSEBoxModel *bm, int funkflag, int sp, double *X, int ndim, double **P, double *Y, int YrMax, int ESTQ, FILE *ofp) {
	int i, j, nfunk, Ilow;
	//double SS;
	double GRD = 1.2;

	/* Assumes X already initialised so set up tolerances and gridding */
	for (i = 0; i < ndim; i++) {
		for (j = 0; j < ndim; j++) {
			P[i][j] = X[j];
			if (i - 1 == j)
				P[i][j] *= GRD;
		}
	}

	for (i = 0; i < ndim; i++) {
		for (j = 0; j < ndim; j++) {
			X[j] = P[i][j];
		}
		Y[i] = Funk(bm, funkflag, sp, ndim, ndim, YrMax, 1.0, X, ofp, NULL);
	}

	AmoebaL(bm, P, Y, ndim, 0.001, 1000, funkflag, ndim, YrMax, 1.0, sp, &nfunk, &Ilow, ofp, NULL);

	if (nfunk <= 1000) {
		for (j = 0; j < ndim; j++) {
			X[j] = P[Ilow][j];
		}
		//SS = Funk(bm, funkflag, sp, ndim, ndim, YrMax, 1.0, X, ofp);
	} else if (ESTQ) {
		//SS = MAXDOUBLE;

		for (j = 0; j < ndim; j++) {
			X[j] = P[Ilow][j];
		}

		fprintf(
				ofp,
				"Failed to fit regression line to production model q estimates are q: %e and qinc: %e so making do or assuming constant q = 0.8 - see documentation\n",
				X[0], X[1]);

		/* Fitting failed so take best on offer or assume q = 0.8 with no increase through time */
		if (X[0] > 1 || X[0] < 0) {
			X[0] = 0.8;
			X[1] = 1.0;
		}

		if (X[1] < 0.7 || X[1] > 1.5) {
			X[1] = 1.0;
		}
	}

	return;

}
/**
 * \brief Routine calculating sums of squares for linear regression line  y = a + bx
 */
void Regression_Funk_Val(int ndim, int YrMax, double *X, double *SS) {
	int Yr;
	double a, b, y, Error;

	a = X[0];
	b = X[1];

	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		y = a + b * Yr;
		Error = NEst[Yr][est_q_id] - y;
		(*SS) += Error * Error;
	}

	return;

}
/**
 * \brief  Routine calculating sums of squares for linear regression line  y = a + bx
 */
void Regression_CPUE_Funk_Val(int ndim, int nid, int YrMax, double *X, double *SS) {
	int Yr;
	double a, b, y, Error;

	a = X[0];
	b = X[1];

	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		y = a + b * Yr;
		Error = CPUEtrend[nid][Yr] - y;
		*SS += Error * Error;
	}

	return;

}
