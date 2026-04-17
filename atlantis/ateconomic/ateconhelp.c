/**
 \file
 \brief C file for helpful math routines for economic models for Atlantis
 \ingroup ateconomic

 File:	ateconhelp.c
 Author:	Beth Fulton
 Created:	7/10/2005
 Purpose:	C file for helpful math routines for economic models for Atlantis
 Revisions: 25/10/2005 Created the file

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>

#include <atEconomic.h>

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define I_LIM 7	  /* The size limit for arrays to be sorted by straight insertion
					(= M in numerical recipes code) */
#define NSTACK 50 /* The required auxiliary storage */

/**
 * \brief Sorting algorithm to sort quota holders based on profitability
 *	- from numerical recipes for C
 *
 *	Modified from quicksort in numerical recipes so its base 0
 *	(not 1 like in the book) and replaced l=1 with simple referral to zero without
 *
 *	@param x is the array to sort
 *  @param bbx and
 *	@param ccx are the arrays to simultaneously rearrage based on the sort
 *	@param n is the dimension
 *	@ param ascendflag is a binary flag defining kind of sort to perform
 *	(ascending or descending)
 *
 */
void Quicksort_Dir(double *x, double *bbx, double *ccx, double *ddx, double *eex, int n, int ascendflag) {
	double a, b, c, d, e, temp;
	int i, j, k;
	int ir = n - 1;
	int jstack = 0;
	int l = 0; /* Modified from l=1 in the original numerical recipes code */
	int nmid = (int) (n / 2); /* If even n all get swapped, if odd then middle one needn't be touched */
	int *istack;

	istack = (int *) i_alloc1d(NSTACK);

	/* Ascending sort - taken from page 333 of Numerical Recipes in C
	 Sorts an array x[0....n-1] into ascending numerical order
	 using the Quicksort algorithm, while making the corresponding
	 rearrangement of the arrays bbx[0....n-1] and ccx[0....n-1].
	 n is input
	 x is replaced on output by its sorted rearrangement
	 */
	for (;;) { /* Insertion sort when subarray small enough */
		if (ir - l < I_LIM) {
			for (j = l + 1; j <= ir; j++) {
				a = x[j];
				b = bbx[j];
				c = ccx[j];
				d = ddx[j];
				e = eex[j];
				for (i = j - 1; i >= l; i--) {
					if (x[i] <= a)
						break;
					x[i + 1] = x[i];
					bbx[i + 1] = bbx[i];
					ccx[i + 1] = ccx[i];
					ddx[i + 1] = ddx[i];
					eex[i + 1] = eex[i];
				}
				x[i + 1] = a;
				bbx[i + 1] = b;
				ccx[i + 1] = c;
				ddx[i + 1] = d;
				eex[i + 1] = e;
			}
			if (!jstack) {
				break;
			}
			ir = istack[jstack]; /* Pop stack and begin a new round of partitioning */
			l = istack[jstack - 1];
			jstack -= 2;
		} else {
			/* Choose median of left, centre, and right elements as partioining element a.
			 Also rearrange so that a[l] <= a[l+1] <= a[ir]
			 */
			k = (l + ir) >> 1;
			SWAP(x[k],x[l+1]);
			SWAP(bbx[k],bbx[l+1]);
			SWAP(ccx[k],ccx[l+1]);
			SWAP(ddx[k],ddx[l+1]);
			SWAP(eex[k],eex[l+1]);
			if (x[l] > x[ir]) {
				SWAP(x[l],x[ir]);
				SWAP(bbx[l],bbx[ir]);
				SWAP(ccx[l],ccx[ir]);
				SWAP(ddx[l],ddx[ir]);
				SWAP(eex[l],eex[ir]);
			}
			if (x[l + 1] > x[ir]) {
				SWAP(x[l+1],x[ir]);
				SWAP(bbx[l+1],bbx[ir]);
				SWAP(ccx[l+1],ccx[ir]);
				SWAP(ddx[l+1],ddx[ir]);
				SWAP(eex[l+1],eex[ir]);
			}
			if (x[l] > x[l + 1]) {
				SWAP(x[l],x[l+1]);
				SWAP(bbx[l],bbx[l+1]);
				SWAP(ccx[l],ccx[l+1]);
				SWAP(ddx[l],ddx[l+1]);
				SWAP(eex[l],eex[l+1]);
			}

			/* Initialise pointers for partitioning */
			i = l + 1;
			j = ir;
			a = x[l + 1]; /* Partitioning element */
			b = bbx[l + 1];
			c = ccx[l + 1];
			d = ddx[l + 1];
			e = eex[l + 1];
			for (;;) {
				/* Beginning of innermost loop */
				do
					i++;
				while (x[i] < a); /* Scan up for element > a */
				do
					j--;
				while (x[j] > a); /* Scan down for element < a */
				if (j < i)
					break; /* Pointers crossed. Partitioning complete. */
				SWAP(x[i],x[j]); /* Exchange elements */
				SWAP(bbx[i],bbx[j]);
				SWAP(ccx[i],ccx[j]);
				SWAP(ddx[i],ddx[j]);
				SWAP(eex[i],eex[j]);
				/* End of innermost loop */
			}
			x[l + 1] = x[j]; /* Insert partitioning element */
			bbx[l + 1] = bbx[j];
			ccx[l + 1] = ccx[j];
			ddx[l + 1] = ddx[j];
			eex[l + 1] = eex[j];
			x[j] = a;
			bbx[j] = b;
			ccx[j] = c;
			ddx[j] = d;
			eex[j] = e;
			jstack += 2;

			/* Push pointers to larger subarray on stack, process smaller subarray immediately */
			if (jstack > NSTACK)
				quit("NSTACK too small in Quicksort_Dir() in economics library. Recode define statement.\n");
			if (ir - 1 + i >= j - l) {
				istack[jstack] = ir;
				istack[jstack - 1] = i;
				ir = j - 1;
			} else {
				istack[jstack] = j - 1;
				istack[jstack - 1] = l;
				l = i;
			}
		}
	}

	if (!ascendflag) {
		/* Descending sort - just reverse the order from the ascending order case */
		/* Make sure ir reset to vector length */
		ir = n - 1;
		for (i = 0; i < nmid; i++) {
			SWAP(x[i],x[ir-i]);
			SWAP(bbx[i],bbx[ir-i]);
			SWAP(ccx[i],ccx[ir-i]);
			SWAP(ddx[i],ddx[ir-i]);
			SWAP(eex[i],eex[ir-i]);
		}
	}

	/* Free local array */
	i_free1d(istack);

	return;
}
