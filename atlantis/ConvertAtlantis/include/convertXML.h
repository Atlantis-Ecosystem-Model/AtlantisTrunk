/*******************************************************************//**
 \file
 \brief  Definitions of utility XML functions.



 Revisions:

 28-10-2009 Bec Gorton
 Added code to convert the assessment input file into XML.

 15-11-2010 Bec Gorton
 Removed the references to the hash table code.

 21-06-2012 Bec Gorton
 Added Convert_Fisheries_To_XML function.

 *********************************************************************/

//#include <regex.h>



void Convert_Biol_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Harvest_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Management_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Assess_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Implementation_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Economic_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Run_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Groups_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Convert_Fisheries_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void xmlSaveFormatFileDestFolder(char *destFolder, char *fileName, xmlDocPtr doc, int number);

/**
 * These define the type of error checking done when parameters are read in.
 */
typedef enum {
	GENERAL_VERT_MOVEMENT = 0
} AtlantisGeneralParamTypes;

/* Useful functions in the atBiolConvert code that are used by the other files */
xmlNodePtr Parse_File_Single_Line(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, char *tag, char *regEx, char *comment, char *units, int type,
		char *recommendedValue, int required, void(*initFP)(MSEBoxModel *bm,  xmlNodePtr parent), void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str,
				char *valueStr));

xmlNodePtr Parse_File(MSEBoxModel *bm, FILE *inputFP,char *fileName,  xmlNodePtr cur, char *tag, char *regEx, char *comment, char *units, int type, int length, int required, void(*initFP)(MSEBoxModel *bm, xmlNodePtr parent), void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr));

void Init_Age_Structured_SpeciesXML(MSEBoxModel *bm, xmlNodePtr parent);
void Init_Vert_Species_Zero_ValuesXML(MSEBoxModel *bm,  xmlNodePtr parent);


void Species_Last_XMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);

void Init_Species_Zero_ValuesXML(MSEBoxModel *bm, xmlNodePtr parent);

/* useful functions in the harvest file */
void Species_NodeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr, char *paramStr);
void Create_Fished_Species_XMLNodes(MSEBoxModel *bm, xmlNodePtr parent);
void Create_Impacted_Species_XMLNodes(MSEBoxModel *bm,  xmlNodePtr parent);

void Fishery_Last_XMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
void Fisheries_NodeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr, char *paramStr);
void Create_Fishery_XMLNodes(MSEBoxModel *bm, xmlNodePtr parent);
xmlNodePtr Create_Harvest_Fishery_Group_ParamXML(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, int paramID, char *comment,
		char *units, int type);
void Create_Fished_Groups_ChangeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr parent, char *paramName, char *comment);

void createImpactedSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
void createFishedSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
//void createVertebrateSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
void createFisheriesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
void createSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr);
void Create_Fishery_ChangeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr parent, char *paramName, char *comment, char *multParam);

xmlNodePtr Create_Fishery_ParamXML(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, int paramID, char *comment, char *units, int type,
		char *recommendedValue);

xmlNodePtr Create_Species_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlNodePtr parent,
		int paramID, char *comment, char *units, ATL_XML_ATTRIBUTE_TYPES type,
		char *recommendedValue);

xmlNodePtr Create_RBC_Species_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlNodePtr parent,
		int paramID, char *comment, char *units, ATL_XML_ATTRIBUTE_TYPES type,
		char *recommendedValue);
xmlNodePtr Create_RBC_Species_ParamXML_Optional(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlNodePtr parent,
		int paramID, char *comment, char *units, ATL_XML_ATTRIBUTE_TYPES type,
		char *recommendedValue);
