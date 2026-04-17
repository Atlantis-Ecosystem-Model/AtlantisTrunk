/*

    File:           param.c

    Created:        Tue Sep 24 14:30:26 EST 1991

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Routines to read parameter values
		    from an ascii file. The file consists
		    of a number of lines, each in the format

		    label value

		    where label is a string (containing no blanks)
		    identifying the parameter which
		    is assigned value. Different types of values
		    are handled by different routines -

		    dparam() - read a double value
		    fparam() - read a float value
		    lparam() - read a long value
		    iparam() - read an int value
		    sparam() - read a short value
		    cparam() - read a character value
		    strparam() - read a string value
		    dparamarray() - read an array of double values

		    For cparam(), the value is taken to
		    be the first non-space character after the label.
		    For strparam, the value must be enclosed in double
		    quotes, eg:

		    label	"A silly value"

		    The qoutes are not included in the returned parameter
		    value

		    Each routine except strparam() performs some suitable
		    type of range checking.

    Arguments:      All routines have the same first three arguments:

		    fp        File from which to read

		    label     String to match in file. For match
			      to occur, the string must appear at
			      the beginning of the line in the file.

		    p         Pointer to stored parameter value
			      (or in the case of the array routines,
			      pointer to the pointer which if not NULL
			      will be allocated to the size
			      specified on the parameter label
			      line)

		    dparam(), fparam(), lparam() and sparam() have
		    forth and fifth arguments:

		    min       minimum allowable value

		    max       maximum allowable value

		    cparam() has only a forth argument:

		    chset     string containing allowable characters

		    strparam also only has a forth argument:

		    maxlen	Maximum string length

		    dparamarray() has forth and fifth parameters which
		    are the minimum and maximum values for each of
		    the array elements. There is also a sixth argument
		    which is a pointer to the size of the array.

    Returns:        If sucessful, each routine returns
		    the position in the file where the parameter
		    was found (as given by ftell()).
                    If something goes wrong each calls param_error()
		    which prints a message and then either
		    exits (so that the routine never returns)
		    or returns (after which the routine should
		    return -1 to the calling program).
		    The behaviour of param_error() is controlled
		    by param_fatal().

    Revisions:      11 Nov 92:  dparamarray only allocates
		    memory if *p == NULL

		    28 Oct 93: Added param_error() and
		    param_fatal() code.

     $Id: txt_param.c 2761 2011-07-01 04:35:25Z gor171 $

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sjwlib.h"

void	param_error( char *msg);
void	param_fatal(int flag);


/** Read a double value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned double value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @return position in file where label was found, else -1.
  */
long
dparam(FILE *fp, char *label, double *p, double min, double max)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	double val;
	short ll;
	short assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("dparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (short)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			if( sscanf(line+ll,"%lf",&val) != 1 ) {
				snprintf(msg, sizeof(msg), "Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			/* range check */
			if( val < min || val > max ) {
				snprintf(msg, sizeof(msg), "Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = val;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg), "Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg), "Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}

/** Read a float value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned float value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @return position in file where label was found, else -1.
  */
long
fparam(FILE *fp, char *label, float *p, double min, double max)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	double val;
	short ll;
	short assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("fparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (short)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			if( sscanf(line+ll,"%lf",&val) != 1 ) {
				snprintf(msg,sizeof(msg), "Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			if( val < min || val > max ) {
				snprintf(msg,sizeof(msg), "Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = (float)val;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg,sizeof(msg), "Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg,sizeof(msg), "Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}


/** Read a long value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned long value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @return position in file where label was found, else -1.
  */
long
lparam(FILE *fp, char *label, long int *p, long int min, long int max)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	long val;
	short ll;
	short assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("lparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (short)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			if( sscanf(line+ll,"%ld",&val) != 1 ) {
				snprintf(msg, sizeof(msg),"Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			if( val < min || val > max ) {
				snprintf(msg, sizeof(msg),"Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = val;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg),"Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg),"Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}


/** Read an integer value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned integer value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @return position in file where label was found, else -1.
  */
long
iparam(FILE *fp, char *label, int *p, int min, int max)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	int val;
	int ll;
	int assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("iparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (int)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			if( sscanf(line+ll,"%d",&val) != 1 ) {
				snprintf(msg, sizeof(msg),"Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			if( val < min || val > max ) {
				snprintf(msg, sizeof(msg),"Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = val;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg),"Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg),"Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}

/** Read a short value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned short value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @return position in file where label was found, else -1.
  */
long
sparam(FILE *fp, char *label, short int *p, int min, int max)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	int val;
	int ll;
	int assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("sparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll =(int) strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			if( sscanf(line+ll,"%d",&val) != 1 ) {
				snprintf(msg, sizeof(msg),"Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			if( val < min || val > max ) {
				snprintf(msg, sizeof(msg),"Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = (short)val;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg),"Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg),"Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}

/** Read a character value associated with a label from a stream.
  * The value is validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned character value.
  * @param chset string containing set of allowable characters.
  * @return position in file where label was found, else -1.
  */
long
cparam(FILE *fp, char *label, char *p, char *chset)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	char *s;
	int ll;
	int assigned = 0;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("cparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (int)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			/* skip empty space on line after label */
			s = line+ll;
			while( *s && isspace(*s) ) s++;
			if( *s == 0 ) {
				snprintf(msg, sizeof(msg),"Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			/* check if character is in allowable set */
			if( strchr(chset,*s) == NULL ) {
				snprintf(msg, sizeof(msg),"Parameter %s outside valid range\n",label);
				param_error(msg);
				return(-1);
			}
			*p = *s;
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg),"Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg),"Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}

/** Read a string associated with a label from a stream.
  * The string must be quoted.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned string.
  * @param maxlen maximum length of the string.
  * @return position in file where label was found, else -1.
  */
long
strparam(FILE *fp, char *label, char *p, long int maxlen)
{
	char line[MAXLINELEN];
	char msg[MAXLINELEN*2];
	char *s;
	int ll;
	int assigned = 0;
	long i;
	long temppos;
	long fpos = -1L;

	/* seek to start of file */
	if( fseek(fp,0L,0) != 0 ) {
		param_error("strparam: Can't seek to start of parameter file\n");
		return(-1);
	}

	ll = (int)strlen(label);
	/* loop to read each line */
	for(;;) {
		temppos = ftell(fp);
		if( fgets(line,MAXLINELEN,fp) == NULL )
			break;
		/* check for label match, plus following space */
		if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
			/* skip to beginning of string */
			s = line+ll;
			while( *s && isspace(*s) ) s++;
			if( !(*s) ) {
				snprintf(msg, sizeof(msg),"Parameter %s format bad on line>> %s",label,line);
				param_error(msg);
				return(-1);
			}
			if( *s == '"' ) {
			    /* copy string, omitting leading and trailing " */
			    s++;
			    i = 0;
			    while( *s && *s != '"' && ++i < maxlen )
				    *p++ = *s++;
			    *p = 0;
			}
			else {
			    /* copy string up to next space */
			    i = 0;
			    while( *s && !isspace(*s) && ++i < maxlen )
				    *p++ = *s++;
			    *p = 0;
			}
			assigned++;
			fpos = temppos;
		}
	}
	if( assigned == 0 ) {
		snprintf(msg, sizeof(msg),"Parameter %s not found\n",label);
		param_error(msg);
		return(-1);
	}

	if( assigned > 1 ) {
		snprintf(msg, sizeof(msg),"Warning - parameter %s found %d times - last value used\n",label,assigned);
		warn(msg);
	}
	return(fpos);
}

/** Read an array of double values associated with a label from a stream.
  * If the array is incomplete, it will be filled with the last value.
  * The values are validated to ensure it within a permissible range.
  * @param fp pointer to a file stream.
  * @param label string label to search for.
  * @param p pointer to returned double value.
  * @param min minimum permissible value.
  * @param max maximum permissible value.
  * @param size pointer to returned size of the array.
  * @return position in file where label was found, else -1.
  */
long
dparamarray(FILE *fp, char *label, double **p, double min, double max, long int *size)
{
    char line[MAXLINELEN];
    char msg[MAXLINELEN*2];
    double val;
    double lastval = 0;
    int ll;
    int found = 0;
    long temppos;
    long fpos = -1L;
    long i;

    /* seek to start of file */
    if( fseek(fp,0L,0) != 0 ) {
	param_error("dparamarray: Can't seek to start of parameter file\n");
	return(-1);
    }

    ll = (int)strlen(label);
    /* loop to read each line and find last occurrence of parameter */
    for(;;) {
	temppos = ftell(fp);
	if( fgets(line,MAXLINELEN,fp) == NULL )
	    break;
	/* check for label match, plus following space */
	if( strncmp(label,line,(size_t)ll)==0 && isspace(line[ll]) ) {
	    if( sscanf(line+ll,"%ld",size) != 1 || *size < 0L ) {
		snprintf(msg, sizeof(msg),"dparamarray: Parameter %s array size format bad on line>> %s",label,line);
		param_error(msg);
		return(-1);
	    }
	    found++;
	    fpos = temppos;
	}
    }
    if( !found ) {
	snprintf(msg, sizeof(msg),"dparamarray: Parameter %s not found\n",label);
	param_error(msg);
	return(-1);
    }

    if( found > 1 ) {
	snprintf(msg, sizeof(msg),"dparamarray:  parameter %s found %d times - last values used\n",label,found);
	warn(msg);
    }

    if( (*size) > 0L ) {
    /* allocate memory for array if required */
	if( *p==NULL && (*p=(double *)malloc((size_t)(*size)*sizeof(double))) == NULL ) {
	    snprintf(msg, sizeof(msg),"dparamarray: %s: Can't read array value\n",label);
	    param_error(msg);
	    return(-1);
	}
	fseek(fp,fpos,0);
	/* throw away parameter label line */
	fgets(line,MAXLINELEN,fp);
	/* loop to read array elements */
	for(i=0; i<(*size); i++) {
	    if(fscanf(fp,"%lf",&val) != 1 ) {
		/* we've run out of values */
		if( i == 0 ) {
		    /* no values at all */
		    snprintf(msg, sizeof(msg),"dparamarray: %s: Can't read any array value\n",label);
		    param_error(msg);
		    return(-1);
		}
		else {
		    /* not enough values - fill remainder with last value */
		    snprintf(msg, sizeof(msg),"dparamarray: %s: Filling to end of array with last value, %e\n",label,lastval);
		    warn(msg);
                    for(; i<(*size); i++)
			(*p)[i] = lastval;
		    break;
		}
	    }
	    /* range check */
	    if( val < min || val > max ) {
		snprintf(msg, sizeof(msg),"dparamarray: %s: array element outside valid range\n",label);
		param_error(msg);
		return(-1);
	    }
	    (*p)[i] = val;
	    lastval = val;
	}
    }

    return(fpos);
}

static int prm_fatal = 1;

void
param_fatal(int flag)
{
    prm_fatal = flag;
}

void
param_error(char *msg)
{
    if( prm_fatal )
	quit(msg);
    else
	warn(msg);
}
