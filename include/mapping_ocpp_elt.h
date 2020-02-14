/**
 * @brief:
 * 	
 * 	mapping the elements found here: http://192.168.123.123/manufacturer
 * 	to be read/write ...
 *
 */

#ifndef MAPPING_OCCP_ELT_H
#define MAPPING_OCCP_ELT_H

typedef unsigned int uint;

typedef struct{
	enum{
		Master,
		Both,
		Indiv,
	}scope;

	union {
		char *string;
		int num;
		uint unsigned_num;
		enum enumeration;
	}value;

	char *right;
}element_t;

/**
 * 	Compilation of the table found here: http://192.168.123.123/doc/doc_1
 *
 * */
const element_t elements[] = {
	{.scope = "Master", .name = "OCPPSlaveState", .right = "R"},
	{.scope = "Mater", .name = "SSHClientKey", .right = "R"},
	{.scope = "Master", .name = "INVOKECUSTOMSCRIPT1", .right = "RW"},
	{.scope = "Master", .name = "INVOKECUSTOMSCRIPT2", .right = "RW"},
	{.scope = "Master", .name = "INVOKECUSTOMSCRIPT3", .right = "RW"},
	{.scope = "Indiv", .name = "RCMB_STATE", .right = "R"},
	{.scope = "Indiv", .name = "ChargeBoxSerialNumber", .right = "RW"},
	{.scope = "Master", .name = "ChargePointSerialNumber", .right = "RW"},
	{.scope = "Indiv", .name = "ErrorsList", .right = "R"},
	{.scope = "Both", .name = "OPKGInstallExtraOption", .right = "RW"},
	{.scope = "Indiv", .name = "CPUFrequency", .right = "R"},
	{.scope = "Both", .name = "StopTxnSampledData", .right = "RW"},
	{.scope = "Both", .name = "StopTxnAlignedData", .right = "RW"},
	{.scope = "Both", .name = "MeterValuesAlignedData", .right = "RW"},
	{.scope = "Both", .name = "MeterValuesSampledData", .right = "RW"},
	{.scope = "Indiv", .name = "MeterPublicKey", .right = "R"},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
	{.scope = "", .value = "", .right = ""},
}

#endif
