/**
 \file
 \brief C file for process subroutines for south east ecological model
 \ingroup atEcology

 File:	attime.c
 Author:	Beth Fulton
 Created:	21/9/2004
 Purpose:	C file for process subroutines for south east ecological model
 Revisions:

 11/8/2004 Created msetime.c from time routines in mseecomproc.c

 21/9/2004 Created attime.c from msetime.c

 16/8/2005 Put in default case for all switch statements

 29/11/2008 Beth Fulton
 Allowed for a wider range of timesteps (when defining night and day)

 08-04-2009 Bec Gorton
 Finished off code that Beth wrote to calculate the proportion of the current day
 that is in sunlight for each box. This means the primary producer light limitation
 calculations should be more accurate. The timeODay function has also been rewritten.

 04-06-2009 Bec Gorton
 Added debugging code to print out the lat and lon of the boxes so that
 i can check the projection is correct.

 16-02-2010 Bec Gorton
 Added the Bio_Active function to this file.

 10-05-2010 Bec Gorton
 Changed the Calculate_Prop_Light_Time function to use PI instead of an approx values for pi.

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atecology.h"
#include <atHarvestLib.h>

static int Time_ODay(MSEBoxModel *bm, double modeldt, double startday, double nowday);
static void Calculate_Prop_Light_Time(MSEBoxModel *bm, double timeleft, double modeldt);
static void Bio_Active(MSEBoxModel *bm, FILE *llogfp);
static void Time_OYr(MSEBoxModel *bm, double thisday);
static void Calculate_Move_Date_Index(MSEBoxModel *bm);

/*************************************************************************//**
 *	This routine is the main time procedure in the ecological module.
 *	It is called by the main() function.
 *
 *	Inputs :
 *	double	t     :  current time  (seconds)
 *	double  dt    :  time range
 *
 *  Outputs :
 *  time information for the MSEBoxModel (Time of year etc)
 */
void Ecology_Time_Check(MSEBoxModel *bm, double t, double dt, FILE *llogfp) {
	double ht = bm->t; /* Was ht=bm->hd.t; but one now used gives peak	in summer when wanted */
	double thisday = bm->dayt;
	double lastday = bm->predayt;
	int newflagday = 0;
	int totnz = bm->wcnz + bm->sednz;
	int i, j;
	double PointInYear;

	if (verbose > 1)
		printf("checking the time\n");

	/* Calculate time of day, time of year and temperature sensitive parameters of models */
	thisday = ht / 86400.0;

	if (thisday >= (lastday + 1.0)) {
		lastday = thisday;
		Time_OYr(bm, thisday);

		/* Reset the checked array */
		for (i = 0; i < bm->nbox; i++) {
			for (j = 0; j < totnz; j++) {
				bm->checkedalready[i][j] = 0;
			}
		}
	}

	PointInYear = (thisday / 365.0 - floor(thisday / 365.0)) * 365.0;

	newflagday = Time_ODay(bm, dt, lastday, thisday);

	/* Reset MSEBoxModel values */
	bm->dayt = thisday;
	bm->predayt = lastday;
	bm->flagday = newflagday;
	bm->DayInYear = PointInYear;

	/* Determine activitiy states of dynamic components */
	Bio_Active(bm, llogfp);
	Harvest_Set_Fishery_Active(bm, llogfp);

	return;
}

/**
 * \brief Computing time of year
 *
 *	Converts current elapsed time into day of the year equivalent for the
 *	purposes of seasonal activties (like spawning and migration)
 *
 */
void Time_OYr(MSEBoxModel *bm, double thisday) {
	int DaysInQ = 0;
	int PointInYear = 0;
	int DayInQuarter = 0;
	int QuarterOfYear = 0;
	int NextQuarterOfYear = 0;
	int DayInMonth = 0;
	int ThisMonth = 0;
	int NewMonth = 0;
	int DayofWeek = bm->DofW;
	int NewWeek = 0;

	if ((thisday / 365.0) < 1.0)
		PointInYear = (int) (floor(thisday + 0.5));
	else
		PointInYear = (int) (floor((thisday / 365.0 - floor(thisday / 365.0)) * 365.0 + 0.5));

	/* Work out day of the week */
	DayofWeek++;
	if (DayofWeek > 6) {
		DayofWeek = 0;
		NewWeek = 1;
	} else
		NewWeek = 0;

	/* Work out the month of the year 0 - 11 (NOT 1 - 12) */
	ThisMonth = (int) (floor(PointInYear / 30.0));
	if (ThisMonth > 11)
		ThisMonth = 11;

	DayInMonth = PointInYear - 30 * ThisMonth;

	if (DayInMonth > 30)
		DayInMonth = 30;

	if ((!DayInMonth) || (bm->MofY < ThisMonth))
		NewMonth = 1;

	if (NewMonth) {
		if (bm->dayt < 1)
			bm->lastMofY = 11;
		else
			bm->lastMofY = bm->MofY;

		/* Work out BiMonth (i.e. which bimonthly period of the year currently in) */
		if (bm->dayt < 1)
			bm->LastBiM = 5;
		else
			bm->LastBiM = bm->BiM;
	}

	switch (ThisMonth) {
	case 0:
	case 1:
		bm->BiM = 0;
		break;
	case 2:
	case 3:
		bm->BiM = 1;
		break;
	case 4:
	case 5:
		bm->BiM = 2;
		break;
	case 6:
	case 7:
		bm->BiM = 3;
		break;
	case 8:
	case 9:
		bm->BiM = 4;
		break;
	case 10:
	case 11:
		bm->BiM = 5;
		break;
	}

	/* Work out the quarter of the year etc */
	if (PointInYear <= 90) {
		QuarterOfYear = 0;
		DayInQuarter = PointInYear;
	} else if ((PointInYear > 90) && (PointInYear <= 181)) {
		QuarterOfYear = 1;
		DayInQuarter = PointInYear - 90;
	} else if ((PointInYear > 181) && (PointInYear <= 272)) {
		QuarterOfYear = 2;
		DayInQuarter = PointInYear - 181;
	} else {
		QuarterOfYear = 3;
		DayInQuarter = PointInYear - 272;
	}
	if (QuarterOfYear < 3)
		DaysInQ = 91;
	else
		DaysInQ = 92;
	bm->HowFar = ((double)(DayInQuarter)) / ((double)(DaysInQ));
	if (QuarterOfYear < 3)
		NextQuarterOfYear = QuarterOfYear + 1;
	else
		NextQuarterOfYear = 0;

	/* Update values to return to the MSEBoxModel */
	bm->TofY = PointInYear;
	bm->MofY = ThisMonth;
	bm->DofM = DayInMonth;
	bm->DofQ = DayInQuarter;
	bm->QofY = QuarterOfYear;
	bm->NextQofY = NextQuarterOfYear;
	bm->newmonth = NewMonth;
	bm->DofW = DayofWeek;
	bm->newweek = NewWeek;

	/* Update the group movement index to make sure we are in the right season */
	Calculate_Move_Date_Index(bm);

	return;
}

/**
 *\brief Time of Day
 *	Sets the diel flag by determining from elapsed time in the day
 *	whether it is day or night (based on the season), this allows for diel
 *	activities (such as feeding). Takes model timestep into account at
 *	present to ensure some daylight hours. Also sets proportion of the time
 *  step when it will be light (for those who worry about such things)
 *
 **/

int Time_ODay(MSEBoxModel *bm, double modeldt, double startday, double nowday) {

	int daytime, dayswitch;

	daytime = 1;

	/* As nowday and startday passed after lastday updated on new days you start with
	 startday == nowday and then they get out of step after that so need the "1 - ()"
	 to get actual time left for the purposes of varying daylength calcs (doesn't
	 matter for fixed daylengths as do get day/night eventually anyway)
	 */

	bm->timeleft = 1 - (nowday - startday);
    
	if (modeldt == 86400.0)
		dayswitch = 0;
	else if (modeldt == (86400.0 / 2.0))
		dayswitch = 1;
	else
		dayswitch = 2;
    
    
    //printf("nowday: %e, startday: %e, timeleft: %e dayswitch: %d modeldt: %e\n", nowday, startday, bm->timeleft, dayswitch, modeldt);

    
	switch (dayswitch) {
	case one_dt: /* One timestep per day, assume always day (noon) */
		daytime = 1;
		bm->timeleft = 0.5;
		break;
	case two_dt: /* Two timesteps per day, assume 12 hours each
	 of day and night, starting with day */
		if (bm->timeleft > 0.5) {
			daytime = 1;
			bm->timeleft = 0.5;
		} else {
			daytime = 0;
			bm->timeleft = 0.0;
		}
		break;
	case other_dt:
		/* For any other timestep size assume first 6 hours is night,
		 then 12 hours day and then 6 hours of night
		 */
        if ((bm->timeleft >= 0.75) || (bm->timeleft <= 0.25)) {
			daytime = 0;
		} else {
			daytime = 1;
		}
		break;
	default:
		quit("No such dayswitch defined (%d) - value must be between 0 and 2 currently\n", dayswitch);
		break;
	}

	/* If worrying about exact daylength then set prop_ligh_time
	 to proportion of the day length that the sky is light
	 */
	if (bm->lim_sun_hours)
		Calculate_Prop_Light_Time(bm, bm->timeleft, modeldt);

    //fprintf(bm->logFile, "Time: %e modeldt: %f nowday: %f startday: %f dayswitch: %d daytime: %d, timeleft: %f\n", bm->dayt, modeldt, nowday, startday, dayswitch, daytime, bm->timeleft);

	return daytime;
}

/**
 *\brief Get latitude off y coordinate
 *	Calculates the latitude from the y coordinate and projection information
 *  included in the bgm file
 *
 **/
double Projection_GetLatitude(MSEBoxModel *bm, double x_coord, double y_coord) {
	double lon = 0.0, lat = 0.0;

	/* Use the proj4 or sjwlib code to do the tranformation */
	mpInverse(bm->projection, x_coord, y_coord, &lat, &lon);

	//printf("x_coord = %e, y_coord = %e, lat - %e, lon = %e\n", x_coord, y_coord, lat, lon);

	return lat;
}

/**
 *\brief Get longitude off y coordinate
 *	Calculates the longitude from the y coordinate and projection information
 *  included in the bgm file
 *
 **/
double Projection_GetLongitude(MSEBoxModel *bm, double x_coord, double y_coord) {
	double lon = 0.0, lat = 0.0;

	/* Use the proj4 or sjwlib code to do the tranformation */
	mpInverse(bm->projection, x_coord, y_coord, &lat, &lon);

	//printf("x_coord = %e, y_coord = %e, lat - %e, lon = %e\n", x_coord, y_coord, lat, lon);

	return lon;
}


/**
 *\brief Calculate Proportion of Timestep that Sun is Up
 *	Calculates the proportion of the time step that has daylight. This is
 *  for the purposes of correcting any primary production if people
 *  want to resolve daylight hours more than 12 hours light, 12 hours dark.
 *
 * Equations used to get P and D are off internet site on calculating daylength given latitude - http://mathforum.org/library/drmath/view/56478.html
 *
 *
 *
 * timeleft is prop of day remaining. In most cases this will be 0.5.
 *
 * This is called once per full timestep not each adaptive timestep.
 *
 *
 *
 **/
void Calculate_Prop_Light_Time(MSEBoxModel *bm, double timeleft, double modeldt) {
	double prop_done = 1 - timeleft;
	double step_prop = modeldt/86400.0;
	double latitude, calendar_day, P, D, D_in_sec, prop_day_light;//, min_prop_day, max_prop_day;//, diff_to_max, diff_to_min;
	int b;
	double x;
	double hours_in_timestep = step_prop * 24.0;
	double total_hours, sunrise, sunset, start_hour, end_hour, sun_hours;

	/* First calculate P */
	calendar_day = (double) (bm->TofY);
	P = asin(0.39795 * cos(0.2163108 + 2 * atan(0.9671396 * tan(0.00860 * (calendar_day - 186.0)))));

	/* Now use that and current latitude to calculate day length */
	for (b = 0; b < bm->nbox; b++) {
		latitude = Projection_GetLatitude(bm, bm->boxes[b].inside.x, bm->boxes[b].inside.y);


		D = 24.0 - (24.0 / PI) * acos((sin(0.8333 * PI / 180.0) + (sin(latitude * PI / 180.0) * sin(P))) / (cos(latitude * PI / 180.0) * cos(P)));
		x = (sin(0.8333 * PI / 180.0) + (sin(latitude * PI / 180.0) * sin(P))) / (cos(latitude * PI / 180.0) * cos(P));

		/* Check to see if we have complete or no daylight */
        if(fabs(x) > 1.0){
            if(x > 1.0){
                D = 24.0;
                if(!bm->flagIsPolar) {
                    warn("24 hours of daylight box %d in day %d, latitude = %e\n", b, bm->TofY, latitude);
                }
            } else {
                D = 0.0;
                if(!bm->flagIsPolar) {
                    warn("No daylight in box %d in day %d, latitude = %e\n", b, bm->TofY, latitude);
                }
			}
		}

		D_in_sec = D * 3600.0;

		/* Calculate total proportion of day that is in sunlight */
		prop_day_light = D_in_sec / 86400.0;

		/* Calculate number of hours of daylight in day */
		total_hours = 24.0 * prop_day_light;

		if(hours_in_timestep == 24.0){
			sun_hours = total_hours;
		}else{

			/* Assume half of dark hours are from midnight */
			sunrise = (24.0 - total_hours)/2.0;
			sunset = sunrise + total_hours;

			/* Get the start and end time of the timestep */
			end_hour = prop_done * 24.0;
			start_hour = end_hour - hours_in_timestep;

			/* Now calculate the number of sun hours in this timestep */
			if(start_hour > sunset || end_hour < sunrise){
				sun_hours = 0;
			}else{
				if(start_hour < sunrise){
					if(end_hour > sunset){
						sun_hours = sunset - sunrise;
					}else{
						sun_hours = end_hour - sunrise;
					}

				}else{
					if(end_hour > sunset){
						sun_hours = sunset - start_hour;
					}else{
						sun_hours = end_hour - start_hour;
					}
				}
			}
		}

		bm->boxes[b].prop_light_time = sun_hours / hours_in_timestep;

	}
	return;
}

/**
 *
 *	\brief This subroutine determines which consumer groups and fisheries
 *	are active, light limitation prevents the primary producers from being
 *	active at night. A 2 indicates no preference, 1 = day and 0 = night.
 *
 */
static void Bio_Active(MSEBoxModel *bm, FILE *llogfp)

{
	int nsp, flag_sp_day, do_anyway;

	if (verbose > 1)
		printf("Activity patterns determined\n");

	do_anyway = 0;
	if (bm->dt == 86400.0)
		do_anyway = 1;

	for (nsp = 0; nsp < bm->K_num_tot_sp; nsp++) {
		flag_sp_day = (int) (FunctGroupArray[nsp].speciesParams[flagactive_id]);
		if ((flag_sp_day == 2) || (bm->flagday == flag_sp_day) || do_anyway)
			FunctGroupArray[nsp].speciesParams[active_id] = 1;
		else
			FunctGroupArray[nsp].speciesParams[active_id] = 0;

	}

	return;

}

/**
 * Caclulate the index of the 'season' for each group.
 *
 * The modelers can specify the number of time periods that each year is split into for prescribed movement.
 * We need to calculate the index of that period for the current day in year. This needs to be done for each individual group.
 *
 */

static void Calculate_Move_Date_Index(MSEBoxModel *bm){
	int sp, stage, index, next_index, orig_index;
	double delta;

	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].sp_geo_move){
            for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
                FunctGroupArray[sp].NeedMoveUpdate[stage] = 0;
            }
            orig_index = FunctGroupArray[sp].moveEntryIndex;
            
			/* find the length of each period */
			delta = 364.0 / (double)(FunctGroupArray[sp].numMoveEntries);
			
			/* now find the index of the current season. */
			index = floor(bm->TofY / delta);

			if (index >= FunctGroupArray[sp].numMoveEntries)
				index = FunctGroupArray[sp].numMoveEntries - 1;
            
            if (index < (FunctGroupArray[sp].numMoveEntries - 1))
                next_index = index + 1;
            else
                next_index = 0;
            
			FunctGroupArray[sp].moveEntryIndex = index;
            FunctGroupArray[sp].next_moveEntryIndex = next_index;

			//fprintf(bm->logFile, "Time: %e dayt, bm->TofY: %d, %s moveEntryIndex: %d orig_index: %d\n", bm->dayt, bm->TofY, FunctGroupArray[sp].groupCode, FunctGroupArray[sp].moveEntryIndex, orig_index);

            /* Check if need to read in new information */
            if (!bm->TofY || (index != orig_index)) { // This way get annual check for those who don't really move within a year but can shift through time driven by and SDM
                for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
                    FunctGroupArray[sp].NeedMoveUpdate[stage] = 1;
                    
                    //fprintf(bm->logFile, "Time: %e dayt, bm->TofY: %d, %s-%d set NeedMoveUpdate: %d\n", bm->dayt, bm->TofY, FunctGroupArray[sp].groupCode, stage, FunctGroupArray[sp].NeedMoveUpdate[stage]);
                }
            }
            
            /* Also got to get HowFar if number of entries isn't one per quarter */
            if (FunctGroupArray[sp].numMoveEntries != 4) {
                FunctGroupArray[sp].HowFar = (bm->TofY / delta) - floor(bm->TofY / delta);  // As only want the remainder for that movement period;
            } else {
                FunctGroupArray[sp].HowFar = bm->HowFar;
                FunctGroupArray[sp].moveEntryIndex = bm->QofY;
                FunctGroupArray[sp].next_moveEntryIndex = bm->NextQofY;
            }
        }
	}
}
