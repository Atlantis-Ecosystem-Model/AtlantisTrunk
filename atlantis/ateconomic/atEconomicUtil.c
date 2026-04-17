/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

double Get_Deemed_Value_Scale(MSEBoxModel *bm, int sp, int nf) {
	int fdv;
	double DVscale = 0.0;

	fdv = (int) (bm->FISHERYprms[nf][flagDV_id]);
	switch (fdv) {
	case noDV_id: /* No use of deemed values so do nothing */
		DVscale = 0.0;
		break;
	case simpleDV_id: /* Deemed value is simple proportion of price, using constant proportions per species per fishery */
		DVscale = bm->SP_FISHERYprms[sp][nf][deemedvalue_id];
		break;
	case complexDV_id: /* Deemed value is a proportion estimated explicitly
	 from costs of landing, price of stock, crew share.
	 Trying to give an additional incentive to land rather
	 than discard, so trying to represent a small return
	 to the quota owner

	 Base the crew share on the first subfleet - FIX may want to generalise this in future
	 */
		DVscale = (1.0 - bm->SUBFLEET_ECONprms[nf][0][crewshare_id] - bm->SP_FISHERYprms[sp][nf][deemedvalue_id]);
		if (DVscale < 0.0)
			DVscale = 0.0;
		break;
	default:
		quit("No such deemed value option, they must be between (none = %d and compound = %d - reset for %s)\n", noDV_id, complexDV_id,
				FisheryArray[nf].fisheryCode);
		break;
	}
	return DVscale;
}

double Get_Expected_Profit(MSEBoxModel *bm, int nf, int ns, int sp, int month, int ntarg, double ExpEffort, char *checkName, int do_debug, FILE *llogfp) {
	double ExpCatch, ExpExposure, ans, calcTax = 0, DVscale, DVprice = 0;
	double TripLength = bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id];
	double TripCost = bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id];

	/* the expected catch is set to the historic catch read in from the input file initially then calculated later */
	ExpCatch = bm->BlackBook[nf][ns][sp][month][expect_id];

	if (bm->TemporalBycatchAvoid) {
		/* If taking temporal exposure to bycatch species (and discards) into account when scheduling effort */
		ExpExposure = FunctGroupArray[sp].speciesParams[sp_concern_id] * bm->BlackBook[nf][ns][sp][month][bycatch_id];
	} else
		ExpExposure = 0;

	/*
	 if(do_debug){
	 fprintf(llogfp,"Time: %e %s-%d on %s TemporalBycatchAvoid: %d, ExpExposure: %e, spconcern: %e, month: %d, Blackbook_bycatch: %e\n",
	 bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->TemporalBycatchAvoid, ExpExposure, FunctGroupArray[sp].speciesParams[sp_concern_id], month, bm->BlackBook[nf][ns][sp][month][bycatch_id]);
	 }
	 */

	if (ExpEffort < small_num)
		ans = 0;
	else {
		calcTax = max(bm->SP_FISHERYprms[sp][nf][FixedMinTax_id], bm->SP_FISHERYprms[sp][nf][tax_id] * bm->SP_FISHERYprms[sp][nf][saleprice_id]);

		/* If we are using deemed values then calculate that */
		if (bm->dayt > bm->DVstart) {
			/* Get the deemed value scale */
			DVscale = Get_Deemed_Value_Scale(bm, sp, nf);
			DVprice = bm->SP_FISHERYprms[sp][nf][saleprice_id] * DVscale;
		} else
			DVprice = 0.0;

		if (bm->OrigEconCalc) {
			/* CPUE based and put ntargets in as trip cost not born by one species individually, but have tax at
			 species specific level
			 */
			ans = (bm->SP_FISHERYprms[sp][nf][saleprice_id] * ExpCatch / (ExpEffort + small_num)) - TripCost / ntarg - calcTax * ExpExposure / (ExpEffort + small_num);
		} else {
			/* Maybe more sensible to sum up total expected value here (minus tax) and then take trip cost off for the entire trip below after done the sum */
			ans = bm->SP_FISHERYprms[sp][nf][saleprice_id] * ExpCatch + ExpExposure * (DVprice - calcTax);
		}
	}

	if (do_debug) {
		//if(bm->dayt > bm->checkstart){
		fprintf(llogfp,
				"Time: %e %s %s %s ans: %e, ExpCatch: %e, TripLength: %e price: %e, ExpEffort: %e, TripCost: %e, DVprice: %e, tax: %e (FixedMinTax_id: %e, tax_id: %e), ExpExposure: %e\n",
				bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, checkName, ans, ExpCatch, TripLength,
				bm->SP_FISHERYprms[sp][nf][saleprice_id], ExpEffort, TripCost, DVprice, calcTax, bm->SP_FISHERYprms[sp][nf][FixedMinTax_id],
				bm->SP_FISHERYprms[sp][nf][tax_id], ExpExposure);
	}

	if (ans < 0)
		ans = 0;

	return ans;
}
