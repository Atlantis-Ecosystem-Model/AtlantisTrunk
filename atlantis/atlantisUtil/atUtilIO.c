/*******************************************************************//**
 \file
 \brief Utility functions to assist with input file IO.
 \brief \ingroup atUtil

 File:			atIOUtil.c

 Purpose:        Utility functions to assist with input file IO.


 Revisions:      20/05/2008 Bec Gorton

 Changed the buflen from 500 to 1200 to allow for Issacs input file to be read.


 22-04-2009 Bec Gorton
 Added the Util_XML_Read_Value function to read in XML values from the new XML input file.
 Also moved all the the function that are IO related into here from atlantisUtil.c.

 14-07-2009 Bec Gorton
 Fixed a bug in at_buildArrayStruct that was generating a warning.

 28-10-2009 Bec Gorton
 Removed the readXMLArrayChecked function.

 17-09-2009 Bec Gorton
 Fixed a bug in the readkeyprm_iarray_checked function that was not checking the array size correctly.

 05-07-2012 Bec Gorton
 Added the Util_Get_Detritus_Index function so we can grab the group index of a detritus index.

 *********************************************************************/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atTracer.h>
#include <atUtilLib.h>
#include <atManageLib.h>
#include "atEcologyLib.h"
#include "atHarvestLib.h"
#include <atXMLUtil.h>

#include <atEconomicLib.h>

/**
 *	Parameter reading
 *	This routine reads a parameter value of ecological models and
 *	modelling process according to its name, by searching from
 *	the begining of the file.
 *
 *	Inputs: FILE  *infile:  handle of input file,
 *	    char  *valueName: parameter name.
 *	Output: returned value of the parameter, or 0.0 if it's not found.
 *
 */
double Util_XML_Read_Value(char *fileName, int type, int localecotest, int isRequired, xmlNodePtr parent, int entry_type, char *valueName) {

	double value;
	xmlNodePtr attributeNode;
	attributeNode = Util_XML_Get_Node(type, parent, valueName);

	if (attributeNode == NULL) {
		if (isRequired == TRUE)
			quit("Util_XML_Read_Value. Error: Cannot find atlantis parameter '%s' in input file %s.\n", valueName, fileName);
		return 0.0;
	}

	/* Sucessfully found parameter - convert to double, check and return.*/
	if(!Util_XML_Get_Node_Value_Double(ATLANTIS_ATTRIBUTE, attributeNode, &value)){
		if (isRequired == TRUE)
			quit("Util_XML_Read_Value. Error: Cannot find atlantis parameter '%s' in input file %s.\n", valueName, fileName);
		return 0.0;
	}
	if (verbose > 2)
		printf("value = %e\n", value);
	Check_Value(valueName, value, entry_type, valueName);
	
	printf("In atUtil printing ecosystem parameters");
	
	if (localecotest > 2)
		printf("Ecosystem parameter: %s = %f\n", valueName, value);
	printf("In atUtil localecotest2 printing ecosystem parameters");
	
	return value;
}

int Util_XML_Get_Value_Time(char *fileName, int type, int localecotest, int isRequired, xmlNodePtr parent, int entry_type, char *valueName, double *val) {

	char value[STRLEN];

	Util_XML_Get_Value_String(fileName, type, isRequired, parent, valueName, value);


	if(!strtosecs(value,val) )
		quit("Util_XML_Read_Time: Can't read %s\n",valueName);

	return TRUE;
}

/**
 *	\brief This function will return the species id that matches the given speciesStr.
 */

int Util_Get_FG_Index(MSEBoxModel *bm, char *speciesName){
	int index;

	for (index = 0; index < bm->K_num_tot_sp; index++) {
		if (strcmp(FunctGroupArray[index].groupCode, speciesName) == 0) {
			return index;
		}
	}

	return -1;
}

/**
 *	\brief This function will return the species id that matches the given speciesStr.
 */

int Util_Get_FG_Index_From_Token(MSEBoxModel *bm, char *speciesName, char *fileName, char *token, int quitOnFail) {
	int index;

	for (index = 0; index < bm->K_num_tot_sp; index++) {
		if (strcmp(FunctGroupArray[index].groupCode, speciesName) == 0) {
			return index;
		}
	}
	if (quitOnFail) {
		quit("Error. Unrecognised species string %s in parameter %s in input file %s\n", speciesName, token, fileName);
	}

	return -1;
}


/**
 *	\brief This function will return the group index of the given
 *	detIndex.
 */

int Util_Get_Detritus_Index(MSEBoxModel *bm, int detIndex) {
	int index;

	/* Work out which detritus group we are looking at*/
	for(index = 0; index < bm->K_num_tot_sp; index++){
		if (FunctGroupArray[index].isDetritus == TRUE) {
			if(FunctGroupArray[index].detIndex == detIndex){
				return index;
			}
		}
	}

	quit("Util_Get_Detritus_Index: Detritus index %d not found\n", detIndex);

	return -1;
}


int Check_Value(char *paramString, double value, int checkType, char *typeName) {
	double base, diff;

	switch (checkType) {
	case no_checking:
		/* No check necessary */
		break;
	case binary_check:
		/* flag value so either 1 or 0 */
		if (value != 0.0 && value != 1.0)
			quit("Error: flag %s - %s must be either 1 or 0. Value is %f\n", typeName, paramString, value);
		break;
	case proportion_check:
		/* value must be between 0 and 1 */
		if (value < 0.0 && value > 1.0)
			quit("Error: flag %s - %s must be between 0 and 1 (currently %e)\n", typeName, paramString, value);
		break;
	case integer_check:
		/* value must be integer */
		base = floor(value);
		diff = fabs(base - value);
		if (diff > 0)
			quit("Error: value of %s - %s must be an integer (currently %e)\n", typeName, paramString, value);
		break;
	default:
		quit(
				"Error: invalid entry_value must be set to 0, 1, 2, or 4, but current set to %d for %s (3 would indicate a sum which is of no use for single value entries)\n",
				value, paramString);
		break;
	}
	return 0;
}

/**
 * \brief This routine makes one last call to textfiles before shutdown
 */
void Textfile_Dump(MSEBoxModel *bm, FILE *llogfp) {
	if (do_biology)
		Ecology_Calculate_Total_Abundance(bm, bm->dt, -2, llogfp);
	if (do_manage && bm->flag_fisheries_on) {
		Harvest_Report_Fisheries_Stats(bm, llogfp);
		Harvest_Report_Annual_Stats(bm, llogfp);

		if (bm->flagecon_on) {
			Economic_Output_Indices(bm, llogfp);
			Economic_Output_Effort_Schedule(bm);
			Economic_Output_Meas(bm);
			Economic_Output_Port_Meas(bm);
			Economic_Output_Trades(bm);
			Manage_Output_Indices(bm);
		}
	}

	return;
}

/**
 * \brief Routine to error check size of array as its read in
 *
 *	entry_type indicates what parameter assertion checks have to be made
 *	0 = none
 *	1 = values are 1 or 0 only (as is a flag)
 *	2 = values are [0,1]
 *	3 = values are [0,1] and sum of values is 1.0
 */
int readkeyprm_darray_checked(FILE *fp, int entry_type, char *key, double **p, int *size, int verifsize) {
	int i, sized;
	double sum, base, diff;
	int ans = 1;

	ans = readkeyprm_darray(fp, key, p, size);
	sized = *size;

	if (verifsize > 0 && sized != verifsize) {
		quit("Array %s is wrong size (expecting %d and got %d as defined size from param file)\n", key, verifsize, sized);
		ans = 0;
	} else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) {
		fprintf(stderr, "\n");
		warn("No verification of size possible for %s\n", key);
	}

	if (!ans)
		quit("readkeyprm_darray_checked: Answer is false.");

	switch (entry_type) {
	case no_checking:
		/* No checks needed */
		break;
	case binary_check:
		/* Flag values only (i.e. either 1 or 0 only) */
		for (i = 0; i < sized; i++) {
			if ((*p)[i] != 0.0 && (*p)[i] != 1.0)
				quit("Entries in %s are supposed to be flag values (i.e. either 1 or 0), but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	case proportion_check:
		/* Values are between 0 and 1 */
		for (i = 0; i < sized; i++) {
			if ((*p)[i] < 0.0 || (*p)[i] > 1.0)
				quit("Entries in %s are supposed to be proportions (i.e. between 0 and 1), but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	case probability_check:
		/* Values are between 0 and 1, and sum is 1.0 */
		if (verbose > 2)
			printf("Checking %s\n", key);
		sum = 0.0;
		for (i = 0; i < sized; i++) {
			if ((*p)[i] < 0.0 && (*p)[i] > 1.0)
				quit("Entries in %s are supposed to be proportions (i.e. between 0 and 1), but %d entry is %e\n", key, i, (*p)[i]);
			else {
				sum += (*p)[i];
				if (verbose > 2)
					printf("%e (sum %e)\n", (*p)[i], sum);
			}
		}
		diff = fabs(sum - 1.0);
		if (diff > buffer_rounding)
			quit("Entries in %s are supposed to sum to 1.0, but current sum is %e (diff %e)\n", key, sum, diff);
		break;
	case integer_check:
		/* Check that integer values are integer */
		for (i = 0; i < sized; i++) {
			base = floor((*p)[i]);
			diff = fabs((*p)[i] - base);
			if (diff > 0)
				quit("Entries in %s are supposed to be integer values, but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	default:
		quit("Check id for array check must be 0, 1, 2 or 3. You have set %d, so recode\n", entry_type);
		break;
	}

	return ans;
}

/**
 * \brief Routine to error check size of array as its read in
 *
 *	entry_type indicates what parameter assertion checks have to be made
 *	0 = none
 *	1 = values are 1 or 0 only (as is a flag)
 *	2 = values are [0,1]
 *	3 = values are [0,1] and sum of values is 1.0
 */
int readkeyprm_iarray_checked(FILE *fp, int entry_type, char *key, int **p, int *size, int verifsize) {
	int i, sized;
	int ans = 1;

	ans = readkeyprm_iarray(fp, key, p, size);
	sized = *size;

	if (sized != verifsize) {
		quit("Array %s is wrong size (expecting %d and got %d as defined size from param file)\n", key, verifsize, sized);
		ans = 0;
	} else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) {
		fprintf(stderr, "\n");
		warn("No verification of size possible for %s. Expected size is %d but an array of size %d was found.\n", key, verifsize, sized);
	}

	if (!ans)
		quit("readkeyprm_iarray_checked: Answer is false.");

	switch (entry_type) {
	case no_checking:
		/* No checks needed */
		break;
	case binary_check:
		/* Flag values only (i.e. either 1 or 0 only) */
		for (i = 0; i < sized; i++) {
			if ((*p)[i] != 0 && (*p)[i] != 1)
				quit("Entries in %s are supposed to be flag values (i.e. either 1 or 0), but %d entry is %d\n", key, i, (*p)[i]);
		}
		break;
	case proportion_check:
	case probability_check:
	case integer_check:
		/* Do nothing */
		break;
	default:
		quit("Check id for array check must be 0, 1, 2 or 3. You have set %d, so recode\n", entry_type);
		break;
	}

	return ans;
}

