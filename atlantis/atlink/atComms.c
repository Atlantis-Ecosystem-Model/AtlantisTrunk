/**
\file
\brief The code that establishes the SOAP communication with EwE.
\ingroup atLink

    File:		atComms.c
	Created:	4/9/2008
	Author:		Bec Gorton,   CSIRO Marine Laboratories,  2008
	Purpose:	Establish
    Note:

    Revisions:

*************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

#include <libcsoap/soap-ctx.h>
#include <libcsoap/soap-xml.h>
#include <libcsoap/soap-router.h>
#include <libcsoap/soap-client.h>
#include <nanohttp/nanohttp-common.h>

#define SIZE 20000


//static char *url = "http://140.79.22.193/WebService1/Service1.asmx";
//static char *urn = "http://tempuri.org/WebService1/Service1";
//static char *method = "HelloWorld";

//static char *SoapAction = "http://tempuri.org/WebService1/Service1/HelloWorld";

/* The EwE Settings */
//static char *EwEServerUrl = "http://140.79.22.154:10001/EwESoapserver";
//static const char const *EwEUrn = "urn:EwE";

//static char *EwEServerUrl = "http://150.229.227.92:80/EwEWebService/Service.asmx";
//static const char *EwEUrn = "http://150.229.227.92:80/EwE";

static char *EwEServerUrl = "http://profiler.ecopath.org/EwEWebService/Service1.asmx";
static const char *EwEUrn = "http://tempuri.org/";
static char *EwEMethod = "Init";
static char *EwESoapAction = "http://tempuri.org/Init";


/* The set methods */
static const char *EwESetBiomassMethod = 		"SetBiomass";
static const char *EwESetProductionMethod = 	"SetProduction";
static const char *EwESetMortalityMethod = 		"SetMortality";
static const char *EwESetNutrientMethod = 		"SetNutrients";
static const char *EwESetPropertiesMethod = 	"SetProperties";

/* The get methods */
static const char *EwEGetBiomassMethod = 		"GetBiomass";
static const char *EwEGetProductionMethod = 	"GetProduction";
static const char *EwEGetMortalityMethod = 		"GetMortality";
static const char *EwEGetNutrientsMethod = 		"GetNutrients";
static const char *EwEGetPropertiesMethod = 	"GetProperties";


static const char *EwEInitMethod = 				"Init";
static const char *EwELoadModelMethod = 		"LoadModel";
static const char *EwERunEcospaceMethod = 		"RunEcospace";
static const char *EwERunNextTSMethod = 		"RunNextEcospaceTS";


static const char *LocalTestMethod = 		"runModel";


//static char *EwEServerUrl = "http://150.229.227.92/ewewebservice/";
//static const char const *EwEUrn = "/EwE";
//static const char const *EwESetBiomassMethod = "BiteWorld";
//static const char const *EwEGetBiomassMethod = "getBiomass";
//static const char const *EwEMethod = "RunModel";

herror_t err;

/**
 * \brief Start the client.
 *
 *
 */
int startClient(int argc, char **argv)
 {
    herror_t err;
    if ((err = soap_client_init_args(argc, argv)) != H_OK)
    {
     	printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
     	herror_release(err);
     	exit(1);
   	}
   	return 0;
 }

void endClient(SoapCtx *request, SoapCtx *response)
 {
 	//Request cleanup
   	soap_ctx_free(response);
   	soap_ctx_free(request);

	//Client cleanup
   	soap_client_destroy();
 }
//
//
//void sayHello(int argc, char **argv)
//{
//	SoapCtx *request;
//	SoapCtx *response;
//	herror_t err;
//	//char *str;
//	//int i;
//
//	if ((err = soap_client_init_args(argc, argv)) != H_OK)
//	{
//		printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
//		herror_release(err);
//		exit(1);
//	}
//
//	//Envelope creation
//	if ((err = soap_ctx_new_with_method(urn, method, &request)) != H_OK)
//	{
//		printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
//		herror_release(err);
//		soap_client_destroy();
//		exit(1);
//	}
//
//	soap_env_add_item(request->env, "xsd:string", "name", "bec");
//	xmlDocFormatDump(stdout, request->env->root->doc, 1);
//
//	//Invocation
//	printf("Sending request \n");
//	if ((err = soap_client_invoke(request, &response, url, SoapAction)) != H_OK)
//	{
//		printf("[%d] %s(): %s\n", herror_code(err), herror_func(err), herror_message(err));
//		herror_release(err);
//		soap_ctx_free(request);
//		soap_client_destroy();
//		exit(1);
//	}
//
//	//Printout result
//	printf("**** received ****\n");
//	xmlDocFormatDump(stdout, response->env->root->doc, 1);
//
//	//Request cleanup
//
//	soap_ctx_free(response);
//	soap_ctx_free(request);
//
//	//Client cleanup
//	soap_client_destroy();
//}
//

void sayEwEHello(int argc, char **argv)
{
	SoapCtx *request;
	SoapCtx *response;
	herror_t err;
	//char *str;
	//int i;

	if ((err = soap_client_init_args(argc, argv)) != H_OK)
	{
		printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
		herror_release(err);
		exit(1);
	}

	//Envelope creation
	if ((err = soap_ctx_new_with_method(EwEUrn, EwEMethod, &request)) != H_OK)
	{
		printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
		herror_release(err);
		soap_client_destroy();
		exit(1);
	}

	//soap_env_add_item(request->env, "xsd:string", "name", "bec");
	xmlDocFormatDump(stdout, request->env->root->doc, 1);


	//Invocation
	printf("Sending request \n");
	if ((err = soap_client_invoke(request, &response, EwEServerUrl, EwESoapAction)) != H_OK)
	{
		printf("[%d] %s(): %s\n", herror_code(err), herror_func(err), herror_message(err));
		herror_release(err);
		soap_ctx_free(request);
		soap_client_destroy();
		exit(1);
	}

	//Printout result
	printf("**** received ****\n");
	xmlDocFormatDump(stdout, response->env->root->doc, 1);

	//Request cleanup

	soap_ctx_free(response);
	soap_ctx_free(request);

	//Client cleanup
	soap_client_destroy();
}

int checkResponse(SoapCtx *response)
{
 	//char *str;
 	//xmlNodePtr node;

 	/* Check that the response was true */
//	node = soap_xml_get_children(response->env->root);
//
//	str = (char *) xmlNodeListGetString(node->doc, node->xmlChildrenNode, 1);
//
//	if(strcmp(str, "FALSE") == 0)
//		return FALSE;

	return TRUE;

 }

 int callBooleanServerFunction(char *method)
 {
 	SoapCtx *request;
	SoapCtx *response;

 	startClient(0, NULL);

	/* Create the XML Envelope */
   	if ((err = soap_ctx_new_with_method(EwEUrn, method, &request)) != H_OK)
   	{
     	printf("runModel - %s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
     	herror_release(err);
     	soap_client_destroy();
     	exit(1);
   	}

   	xmlDocFormatDump(stdout, request->env->root->doc, 1);

	/* Invoke the server function */
   	if ((err = soap_client_invoke(request, &response, EwEServerUrl, "http://tempuri.org/Init")) != H_OK)
   	{
     	printf("[%d] %s(): %s\n", herror_code(err), herror_func(err), herror_message(err));
     	herror_release(err);
     	soap_ctx_free(request);
     	soap_client_destroy();
     	exit(1);
   	}

   	/* TODO: Pass in box model so i can check for debug */
	xmlDocFormatDump(stdout, response->env->root->doc, 1);

	/* Check that the response was true */
	if(checkResponse(response) == FALSE){
		printf("EwE model setup failed \n");
		return FALSE;
	}

	endClient(request, response);
	return TRUE;

 }

 /**
  * \brief Setup the EwE model.
  *
  * This must be called before any set methods are called.
  *
  *
  */
int setupEwECommunication(void)
{
 	if(callBooleanServerFunction((char *)EwEInitMethod) == FALSE)
 		return 1;
 	if(callBooleanServerFunction((char *)EwELoadModelMethod) == FALSE)
 		return 2;
 	if(callBooleanServerFunction((char *)EwERunEcospaceMethod) == FALSE)
 		return 3;

 	return 0;
 }

 int TestLocal(void)
{
 	if(callBooleanServerFunction((char *)LocalTestMethod) == FALSE)
 		return 1;
 	return 0;
 }

/**
 * 	\brief Tell EwE to move to the next time step.
 *
 *
 */
int runModel(void)
{
	if(callBooleanServerFunction((char *)EwERunNextTSMethod) == FALSE)
		return FALSE;
	return TRUE;
}
/**
 * \brief Build the data string to send to EwE.
 *
 */
char *buildDataString(double **dataArray, int numRows, int numColumns)
{
	int rows, columns;
	char *str;

	str = (char *)malloc(sizeof(char) * numColumns * numRows * 50);

   	for(rows = 0; rows < numRows; rows++){
   		for(columns = 0; columns < numColumns; columns++){
   			if(rows == 0 && columns == 0)
   				snprintf(str, sizeof(str), "%e", dataArray[rows][columns]);
   			else
   				snprintf(str, sizeof(str), "%s,%e", str, dataArray[rows][columns]);
   		}
   	}
   	return str;
}

/**
 * \brief Get a parameter set back from EwE
 *
 */
int getParameter(char *method, char *species, double **dataArray, int numRows, int numColumns)
{
	SoapCtx *request;
	SoapCtx *response;
	startClient(0, NULL);

	/* Create the XML Envelope */
   	if ((err = soap_ctx_new_with_method(EwEUrn, EwEGetBiomassMethod, &request)) != H_OK)
   	{
     	printf("getBiomass - %s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
     	herror_release(err);
     	soap_client_destroy();
     	exit(1);
   	}

   	soap_env_add_item(request->env, "xsd:string", "name", "PL");

	/* Invoke the server function */
   	if ((err = soap_client_invoke(request, &response, EwEServerUrl, "")) != H_OK)
   	{
     	printf("[%d] %s(): %s\n", herror_code(err), herror_func(err), herror_message(err));
     	herror_release(err);
     	soap_ctx_free(request);
     	soap_client_destroy();
     	exit(1);
   	}

	xmlDocFormatDump(stdout, response->env->root->doc, 1);

	/* Get the parameter array output the returned XML document */
	endClient(request, response);

	return TRUE;
}

/**
 * \brief Set an EwE parameter
 *
 */
int setParameter(char *method, char *species, double **dataArray, int numRows, int numColumns)
{
	SoapCtx *request;
	SoapCtx *response;
	char *str;

	startClient(0, NULL);

	/* Build the parameter string */
    str = buildDataString(dataArray, numRows, numColumns);

	/* Create the soap envelope */
   	if ((err = soap_ctx_new_with_method(EwEUrn, method, &request)) != H_OK)
   	{
     	printf("%s - %s():%s [%d]\n", method, herror_func(err), herror_message(err), herror_code(err));
     	herror_release(err);
     	soap_client_destroy();
     	exit(1);
   	}

   	xmlDocFormatDump(stdout, request->env->root->doc, 1);

	/* Add the parameter string child */
   	soap_env_add_item(request->env, "xsd:string", "GroupName", species);
   	soap_env_add_item(request->env, "xsd:string", "Values", str);

	printf("Invoking %s service\n", method);
	/* Invoke the service */
   	if ((err = soap_client_invoke(request, &response, EwEServerUrl, "")) != H_OK)
   	{
     	printf("%s - [%d] %s(): %s\n", method, herror_code(err), herror_func(err), herror_message(err));
     	herror_release(err);
     	soap_ctx_free(request);
     	soap_client_destroy();
     	printf("calling exit\n");
     	exit(1);
   	}
   	printf("%s service complete\n", method);
   	xmlDocFormatDump(stdout, response->env->root->doc, 1);

	free(str);
	endClient(request, response);
	return TRUE;
}
/**
 * \brief Call the EwE setBiomass function.
 *
 * Will pass a set of biomass values via a string in the XML request.
 * The format of the string will be:
 * 	parameter, [row 1 values], [row 2 values]
 *
 * Where each row value is seperated by a comma.
 *
 */
int setBiomass(char *species, double **dataArray, int numRows, int numColumns)
{
	return setParameter((char *)EwESetBiomassMethod, species, dataArray, numRows, numColumns);
}

/**
 * \brief Call the EwE setProduction function.
 *
 * Will pass a set of biomass values via a string in the XML request.
 * The format of the string will be:
 * 	parameter, [row 1 values], [row 2 values]
 *
 * Where each row value is seperated by a comma.
 *
 */
int setProduction(char *species, double **dataArray, int numRows, int numColumns)
{
	return setParameter((char *)EwESetProductionMethod, species, dataArray, numRows, numColumns);
}

/**
 * \brief Call the EwE setMortality function.
 *
 * Will pass a set of biomass values via a string in the XML request.
 * The format of the string will be:
 * 	parameter, [row 1 values], [row 2 values]
 *
 * Where each row value is seperated by a comma.
 *
 */
int setMortality(char *species, double **dataArray, int numRows, int numColumns)
{
	return setParameter((char *)EwESetMortalityMethod, species, dataArray, numRows, numColumns);
}

/**
 * \brief Call the EwE setNutrient function.
 *
 * Will pass a set of biomass values via a string in the XML request.
 * The format of the string will be:
 * 	parameter, [row 1 values], [row 2 values]
 *
 * Where each row value is seperated by a comma.
 *
 */
int setNutrients(char *species, double **dataArray, int numRows, int numColumns)
{
	return setParameter((char *)EwESetNutrientMethod, species, dataArray, numRows, numColumns);
}

/**
 * \brief Call the EwE setProperties function.
 *
 * Will pass a set of biomass values via a string in the XML request.
 * The format of the string will be:
 * 	parameter, [row 1 values], [row 2 values]
 *
 * Where each row value is seperated by a comma.
 *
 */
int setProperties(char *species, double **dataArray, int numRows, int numColumns)
{
	return setParameter((char *)EwESetPropertiesMethod, species, dataArray, numRows, numColumns);
}
/**
 * \brief Call the EwE getBiomass function.
 *
 *	Will get the biomass data back from EwE.
 *
 */
int getBiomass(char *species, double **values, int numRows, int numColumns)
{
	return getParameter((char *)EwEGetBiomassMethod, species, values, numRows, numColumns);
}

/**
 * \brief Call the EwE getProduction function.
 *
 *	Will get the biomass data back from EwE.
 *
 */
int getProduction(char *species, double **values, int numRows, int numColumns)
{
	return getParameter((char *)EwEGetProductionMethod, species, values, numRows, numColumns);
}

/**
 * \brief Call the EwE getMortality function.
 *
 *	Will get the biomass data back from EwE.
 *
 */
int getMortality(char *species, double **values, int numRows, int numColumns)
{
	return getParameter((char *)EwEGetMortalityMethod, species, values, numRows, numColumns);
}

/**
 * \brief Call the EwE getNutrients function.
 *
 *	Will get the biomass data back from EwE.
 *
 */
int getNutrients(char *species, double **values, int numRows, int numColumns)
{
	return getParameter((char *)EwEGetNutrientsMethod, species, values, numRows, numColumns);
}
/**
 * \brief Call the EwE getProperties function.
 *
 *	Will get the biomass data back from EwE.
 *
 */
int getProperties(char *species, double **values, int numRows, int numColumns)
{
	return getParameter((char *)EwEGetPropertiesMethod, species, values, numRows, numColumns);
}
