/*

    File:           time.c

    Created:        1991

    Author:         Stephen Walker
		    CSIRO Division of Oceanography

    Purpose:        Routines to handle time.

    Arguments:      See each routine below

    Revisions:      none

    $Id: time.c 2761 2011-07-01 04:35:25Z gor171 $

*/

/*
 * These next two routines are adapted from Numerical Recipes in C
 * by Press et. al.
 * They convert between date and integer julian days.
 * Note that a julian day starts at midday (for convenience
 * of astronomers!)
 */

#define IGREG (15+31L*(10+12L*1582))

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sjwlib.h>

/** Convert from calendar date to julian days.
  * Adapted from Numerical Recipes in C by Press et. al.
  * Note that a julian day starts at midday (for convenience
  * of astronomers!)
  *
  * @param mm month.
  * @param id day.
  * @param iyyy year.
  * @return julian day.
  */
long julday(int mm, int id, int iyyy)
{
	long jul;
	int ja,jy,jm;

	if (iyyy == 0)
		quit("julday: there is no year zero.\n");
	if (iyyy < 0) ++iyyy;
	if (mm > 2) {
		jy = iyyy;
		jm = mm+1;
	} else {
		jy = iyyy-1;
		jm  =mm+13;
	}
	jul = (long) (floor(365.25*jy)+floor(30.6001*jm)+id+1720995);
	if (id+31L*(mm+12L*iyyy) >= IGREG) {
		ja = (int)(0.01*jy);
		jul += 2-ja+(int) (0.25*ja);
	}
	return jul;
}

#undef IGREG

#define IGREG 2299161

/** Convert to julian days given the calendar date.
  * Adapted from Numerical Recipes in C by Press et. al.
  * Note that a julian day starts at midday (for convenience
  * of astronomers!)
  *
  * @param julian julian days.
  * @param mm pointer to returned month.
  * @param id pointer to returned day.
  * @param iyyy pointer to returned year.
  */
void
caldat(long int julian, int *mm, int *id, int *iyyy)
{
	long ja,jalpha,jb,jc,jd,je;

	if (julian >= IGREG) {
		jalpha = (long)(((float) (julian-1867216)-0.25)/36524.25);
		ja = julian+1+jalpha-(long) (0.25*(double)jalpha);
	} else
		ja = julian;
	jb = ja+1524;
	jc = (long)(6680.0+((float) (jb-2439870)-122.1)/365.25);
	jd = (long)(365*(double)jc+(0.25*(double)jc));
	je = (long)((double)(jb-jd)/30.6001);
	*id = (int)(jb-jd)-(int) (30.6001*(double)je);
	*mm = (int)je-1;
	if (*mm > 12) *mm -= 12;
	*iyyy = (int)jc-4715;
	if (*mm > 2) --(*iyyy);
	if (*iyyy <= 0) --(*iyyy);
}

#undef IGREG

/** Convert to julian days given date/time.
  *
  * @param y year of gregorian calendar.
  * @param mo month of year.
  * @param d day of month.
  * @param h hour of day.
  * @param mi minute of hour.
  * @param s seconds of minute.
  */
double tojul(int y, int mo, int d, int h, int mi, int s)
{
	double j;
	double frac;

	j = (double)julday(mo,d,y);
	frac = ((h*60.0 + mi)*60.0 + s)/86400.0;
	return(j+frac-0.5);
}

/** Convert to julian seonds given date/time.
  *
  * @param j julian seconds.
  * @param y pointer to returned year of gregorian calendar.
  * @param mo pointer to returned month of year.
  * @param d pointer to returned day of month.
  * @param h pointer to returned hour of day.
  * @param mi pointer to returned minute of hour.
  * @param s pointer to returned seconds of minute.
  */
void todat(double j, int *y, int *mo, int *d, int *h, int *mi, int *s)
{
	long ij;
	double sec;

	ij = (long)(floor(j+0.5));
	caldat(ij,mo,d,y);

	sec = (j + 0.5 - (double)ij) * 86400.0;
	*h = (int)floor(sec/3600.0);
	sec -= *h * 3600.0;
	*mi = (int)floor(sec/60.0);
	sec -= *mi*60.0;
	*s =  (int)floor(sec+0.5);

	if( *s >= 60 ) {
	    *s -= 60;
	    *mi += 1;
	}
	if( *mi >= 60 ) {
	    *mi -= 60;
	    *h += 1;
	}
	if( *h >= 24 ) {
	    double j = tojul(*y,*mo,*d,0,0,0) + 1;
	    caldat((long)j,mo,d,y);
	    *h -= 24;
	}
}


/* Units and scaling factors */

struct {
    char *unit;
    double mult;
} sec_conv[] = {
    { "us", 0.000001 },
    { "usec", 0.000001 },
    { "ms", 0.001 },
    { "msec", 0.001 },
    { "s", 1.0 },
    { "sec", 1.0 },
    { "second", 1.0 },
    { "min", 60.0 },
    { "minute", 60.0 },
    { "h", 3600.0 },
    { "hr", 3600.0 },
    { "hour", 3600.0 },
    { "d", 86400.0 },
    { "day", 86400.0 },
    { "week", 604800.0 },
    { NULL, 0.0 }
};

/** Converts a strings of the form 2 days, or 1 week,
  * or 3.6 hr, etc. into a value in seconds.
  *
  * @param str formated character string.
  * @param sec seconds.
  * @return non-zero if successful.
  */
int strtosecs(char *str, double *sec)
{
    int n;
    double v;
    char u[MAXLINELEN];

    n = sscanf(str,"%lf %s",&v,u);

    if( n < 1 )
	/* Can't read anything */
	return(0);

    if( n == 1 || u[0] == '#' ) {
	/* No units after value - assume already in seconds */
	*sec = v;
	return(1);
    }

    if( n == 2 ) {
	unsigned int i;

	/* Convert units to lower case */
	for(i=0; i<strlen(u); i++)
	    u[i] = (char)tolower(u[i]);
	/* Strip plural trailing s */
	if( (i=(unsigned int)strlen(u)) > 2 && u[i-1] == 's' )
	    u[i-1] = 0;
	/* Search through list for matching units */
	for(i=0; sec_conv[i].unit; i++)
	    if( strcmp(sec_conv[i].unit,u) == 0 ) {
		*sec = v * sec_conv[i].mult;
		return(1);
	    }
    }
    return(0);
}


/** Convert an array of times from one epoch to another.
  *
  * @param oepoch Original epoch.
  * @param nepoch New epoch.
  * @param times New times in units of new epoch.
  * @param n Number of times.
  * @return non-zero if successful.
  */
void ChangeTimeUnits(char *oepoch, char *nepoch, double *times, int n)
{
    char buf[MAXLINELEN];
    double oscale = 0.0;
    double ooffset = 0.0;
    double nscale = 0.0;
    double noffset = 0.0;
    double scale = 0.0;
    double offset = 0.0;
    int i;

    /* Compute the scaling and offset (in julian seconds) for the old epoch. */
    ooffset = DateToJulian(oepoch)*86400.0;
    snprintf(buf, sizeof(buf), "1 %s",oepoch);
    if(!strtosecs(buf,&oscale) )
        quit("ChangeTimeEpoch: Can't convert original epoch %s to seconds\n",buf);

    /* Compute the scaling and offset (in julian seconds) for the new epoch. */
    noffset = DateToJulian(nepoch)*86400.0;
    snprintf(buf, sizeof(buf), "1 %s",nepoch);
    if(!strtosecs(buf,&nscale) )
        quit("ChangeTimeEpoch: Can't convert new epoch %s to seconds\n",buf);

    scale = oscale/nscale;
    offset = (ooffset - noffset)/nscale;
    for (i=0; i<n; ++i)
        times[i] = times[i]*scale + offset;
}



/* static aresult array */
char datestring[] = "1995-12-29 10:01:00";
/** Converts a time value expressed with given units
  * into a date string.
  *
  * Time units are assumed to have the form:
  * units since YYYY-MM-DD [HH:MM:SS] [TZ]
  *
  * @param t time in seconds.
  * @param u units.
  * @return ISO formated date/time string.
  */
char *TimeToDate(double t, char *u)
{
    char units[] = "1 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    double mult;
    int y, mo, d;
    int h, mi, s;
    double j = DateToJulian(u);

    /* Read units, convert to seconds */
    if( sscanf(u,"%10s since %*d-%*d-%*d",&units[2]) != 1 )
	quit("TimeToDate: Can't understand %s\n",u);
    if( !strtosecs(units,&mult) )
	quit("TimeToDate: Can't convert %s to seconds\n",units);

    /* Add time value to epoch */
    j += t*mult/86400.0;

    /* Convert back to date */
    todat(j,&y,&mo,&d,&h,&mi,&s);

    /* Store the resulting string */
    snprintf(datestring, sizeof(datestring), "%4d-%02d-%02d %02d:%02d:%02d",y,mo,d,h,mi,s);

    return(datestring);
}


/** Converts a date and time expressed as YYYY-MM-DD HH:MM:SS
  * into a time value with given units.
  *
  * Time units are assumed to have the form:
  * units since YYYY-MM-DD [HH:MM:SS] [TZ]
  *
  * @param d date and time in ISO date/time format (YYYY-MM-DD HH:MM:SS).
  * @param u units in ISO date/time format (YYYY-MM-DD HH:MM:SS).
  * @return julian seconds.
  */
double DateToTime(char *d, char *u)
{
    double e;
    double j;
    double t;
    double mult;
    char buf[MAXLINELEN];

    /* Convert date and units to respective Julian days */
    e = DateToJulian(u);
    j = DateToJulian(d);
    /* Calculate time in seconds */
    t = 86400.0*(j-e);

    /* Convert to required units */
    snprintf(buf, sizeof(buf), "1 %s",u);
    if( !strtosecs(buf,&mult) )
	quit("DateToTime: Can't convert %s to seconds\n",buf);
    t /= mult;

    return(t);
}

/** Converts a date and time expressed as
  * YYYY-MM-DD HH:MM:SS or
  * units since YYYY-MM-DD HH:MM:SS into Julian days.
  *
  * @param d date and time in ISO date/time format (YYYY-MM-DD HH:MM:SS).
  * @return julian seconds.
  */
double DateToJulian(char *d)
{
    int y, mo, day;
    int h, mi, s;
    double j;
    char *p;

    /* Strip "units since", if present */
    if( (p=strstr(d,"since")) != NULL )
	p += 5;
    else
	p = d;

    /* Read year, month and day */
    if( sscanf(p,"%d-%d-%d",&y,&mo,&day) != 3 )
	quit("DateToJulian: Can't understand %s\n",p);

    /* Read hours, minutes and seconds if present */
    if( sscanf(p,"%*d-%*d-%*d %d:%d:%d",&h,&mi,&s) != 3 ) {
	h = 0;
	mi = 0;
	s = 0;
    }

    /* Convert to Julian days */
    j = tojul(y,mo,day,h,mi,s);
    return(j);
}
