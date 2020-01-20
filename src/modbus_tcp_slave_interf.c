#include <sys/time.h> /* for the time timeval */  
#include <modbus.h>
#include <stdlib.h>
#include <errno.h>

#include "ethread.h" // used for threading 

#include "modbus_tcp_slave_interf.h"
#include "helpers/logging.h"
#include "master_slave.h"// config slave internals
#include "string_utils.h"

/* check for the context */
#ifdef TCP
#define SLAVE_TCP_PORT 1502 /* the port 502 used in TCP context, some issues have been encontered on this port, use the port 1502 instead */
#define MASTER_IP "xxx.xxx.xxx.xxx" /* additional IP address used by the master */
#define SLAVE_TCP "yyy.yyy.yyy.yyy" /* this is the IP address used by the slave to connect to the master */
#elif TCP_IP
#define SLAVE_TCP_PORT 1502 
//#define MASTER_IP "::::::" /* additional IP address used by the master */
#define SLAVE_TCP_IP "::" /* this is the IP address used by the slave to connect to the master */
#endif

#define NB_CONNECTION 10 /* the slave will attempt 10 connections before quitting ... */

#define MAX_ACCEPTED_TIMEOUT_MS 5000

/*#define ENABLE_LIBMODBUS_DEBUG 1*/
#define MANAGE_RESP_TIMEOUT 0 /* change to 1 to enable sync timeouts */
#define MANAGE_BYTE_RESP_TIMEOUT 0 /* inter byte timeouts */


/* @brief:
 * 	these values should get constants convenient to the starting adresses of the different
 * 	 registers and their number respectively. This is the case when the registers are located on continuous blocks
 * 	 Otherwise, I will create customized table for that.
 * @TODO: 
 *  	fill in with the suitable required data 
 * */
#define REG_DISCRETE_OUTPUT_ADDRESS "@discrete_output"
#define REG_DISCRETE_OUTUT_NB "nb_discrete_output"
#define REG_HOLDING_ADDRESS "@holding_register"
#define REG_HOLDING_NB "nb_holding_reg"
#define REG_DISCRETE_INPUT_ADDRESS "@discrete_input"
#define REG_DISCRETE_INPUT_NB "nb_discrete_input" 
#define REG_INPUT_ADDRESS "@input_registers"
#define REG_INPUT_NB "nb_input_register" 

/* here goes the definition of the registers adresses to use */ 
//const unsigned int registers[] = {}; 

/* a complete definition of where to find a piece of data requires both the address (or register number) and function code (or register type). 
 *
 *
 *   Function Code	Register Type				comments	
   _________________________________________________________________________	
  	1 		Read Coil				NA* 
  	2 		Read Discrete Input			NA* 
	3 		Read Holding Registers			defined
	4 		Read Input Registers			defined
	5 		Write Single Coil			NA*
	6 		Write Single Holding Register		defined
	15 		Write Multiple Coils			NA
	16 		Write Multiple Holding Registers	?
   _________________________________________________________________________

   NA*: I am not sure but these registers seem not used in our situation ...

 * */

/* these are the known registers  used in the modbus protocol
 * They are also decalared in the modbus
 *
 * TODO: check what to be ignored
 * */

#define READ_COIL 0x01
#define READ_DISCRETE_INPUT 0x02
#define READ_HOLDING_REGISTER 0x03
#define READ_INPUT_REGISTER 0x04
#define WRITE_SINGLE_COIL 0x05
#define WRITE_SINGLE_HOLDING_REGISTER 0x06
#define WRITE_MULTIPLE_COILS 0x0F
#define WRITE_MULTIPLE_HOLDING_REGISTER 0x10

/* these ExceptionCode values are already available in modbus.h ...
#define ILLEGAL_FUNCTION 0x01
#define ILLEGAL_DATA_ADDRESS 0x02
#define ILLEGAL_DATA_VALUE 0x03
#define SERVER_DEVICE_FAILURE 0x04
#define ACKNOWLEDGE 0x05
#define SERVER_DEVICE_BUSY 0x06
#define MEMORY_PARITY_ERROR 0x08
#define GATEWAY_PATH_UNAVAILABLE 0x0A 
*/

static modbus_t *ctx;

typedef struct {
	int addr;
	int length;
	enum {
		HOLDING = 0, INPUT = 1 
	}type; /* they are 16-bits registers, there can be 2 other types Discrete input/output, but apparently not used here ... */
}register_t;

/* define the list of register to be used 
 * TODO: ask for what registers to use */
const uint16_t HOLDING_REGISTERS_TAB[] = {};
const uint16_t INPUT_REGISTERS_TAB[] = {};
const uint8_t INPUT_BITS_TAB[] = {};
const uint8_t BITS_TAB[] = {}; 


static modbus_mapping_t *mb_mapping;
static uint8_t *request;
static int header_length = 0;


typedef unsigned int uint; 

/*
 * Init the modbus_tcp all needed requirements
 *
 * */
void init_modbus_tcp_pi(modbus_t *ctx){

	struct timeval *byte_resp_timeout;
	struct timeval *get_byte_timeout;

	struct timeval *resp_timeout;
	struct timeval *prev_timeout;

	int i = 0; 

	struct ethread_info eti_listener = ETHREAD_ETI_INITIALIZER;
	
	byte_resp_timeout = malloc(sizeof(struct timeval));
	get_byte_timeout = malloc(sizeof(struct timeval));

	resp_timeout = malloc(sizeof(struct timeval));
	prev_timeout = malloc(sizeof(struct timeval));

#if ENABLE_LIBMODBUS_DEBUG
                /* TRUE defined as 1 in modbus.h */
                modbus_set_debug(ctx, TRUE);
#endif

#if MODBUS_TCP_IP
	ctx = modbus_new_tcp_pi(SLAVE_TCP_IP, port);
#elif MODBUS_TCP
	ctx = modbus_new_tcp(SLAVE_TCP, port);
#endif 

	//configure_controller_for_slave_role();
	/* check for successful return of the modbus_new_tcp_pi  */
	if (ctx == NULL) {
		USER_ERR("Unable to allocate libmodbus context: %s", modbus_strerror(errno));
		goto terminate_modbus;
	}

	if (modbus_connect(ctx) == -1) {
		USER_ERR("Connection failed: %s\n", modbus_strerror(errno));
		goto terminate_modbus;
	}

	if (modbus_set_slave(ctx, MODBUS_TCP_SLAVE) == -1){
		USER_ERR("modbus_set_slave() failed: %s", modbus_strerror(errno));
		goto terminate_modbus;
	}

	/* it is recommended to disable the recovery mode in a server/slave mode **** TODO: review, is this right */
	if (modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_PROTOCOL | MODBUS_ERROR_RECOVERY_LINK)) {
		modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_NONE);
		USER_ERR("modbus_set_error_recovery() failed: %s", modbus_strerror(errno));
	}

	request = malloc(MODBUS_TCP_MAX_ADU_LENGTH);

	header_length = modbus_get_header_length(ctx);

#if TCP	
	server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
	modbus_tcp_accept(ctx, &server_socket);
#elif TCP_IP
	server_socket = modbus_tcp_pi_listen(ctx, NB_CONNECTION);
	modbus_tcp_pi_accept(ctx, &server_socket);
#endif

#if MANAGE_RESP_TIMEOUT
	/* @note: 
	 * 	It is desirable to manage a timeout in order not to definitely wait for an answer which will
	 * 	perhaps never arrive
	 * */
	modbus_get_response_timeout(ctx, prev_timeout);
	if(prev_timeout->tv_sec > MAX_ACCEPTED_TIMEOUT){
		resp_timeout->tv_sec = MAX_ACCEPTED_TIMEOUT;
		resp_timeout->tv_usec = 0;
		modbus_set_response_timeout(ctx, resp_timeout);
	}
#if MANAGE_BYTE_RESP_TIMEOUT
	/*	Sometimes if the connection is not stable, may be we need to reset the established connection 
	 *	Fixing an acceptable timeout between read bytes could be a good habit 
	 * */
	modbus_get_byte_timeout(ctx, get_byte_timeout);
	if(get_byte_timeout->tv_sec > MAX_ACCEPTED_TIMEOUT){
		byte_resp_timeout->tv_sec = MAX_ACCEPTED_TIMEOUT;
		byte_resp_timeout->tv_usec = 0;
		modbus_set_response_timeout(ctx, byte_resp_timeout);
	}


#endif
#endif
	/* TODO: review this, what is mapping? */
	mb_mapping = modbus_mapping_new(REG_DISCRETE_OUTPUT_ADDRESS + REG_DISCRETE_OUTPUT_NB,
			REG_HOLDING_ADDRESS + REG_HOLDING_NB,
			REG_DISCRETE_INPUT_ADDRESS + REG_DISCRETE_INPUT_NB,
			REG_INPUT_ADDRESS + REG_INPUT_NB);
	if (mb_mapping == NULL) {
		USER_ERR("Failed to allocate the mapping: %s", modbus_strerror(errno));
		goto terminate_modbus;
	}

	/* Initialize input values that's can be only done server side. 
	 * TODO: review needed, this is can be omitted, no coils in our product?
	 *
	 * TODO: define the input_bits_nb variable
	 * */
	modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, input_bits_nb,
			INPUT_BITS_TAB);

	/* Initialize values of INPUT and HOLDING EGISTERS. 
	 * TODO: define the values of the input_register_nb and register_nb respectively  */
	for (i=0; i < input_register_nb; i++) {
		mb_mapping->tab_input_registers[i] = INPUT_REGISTERS_TAB[i];;
	}

	/* loop for request from the master and or responses to master */
	ethread_start(eti_listener, "listen_for_master_request", listen_for_master_request, NULL);
} /* end of the init_modbus function */


/* @Note: 
 * 	When a Modbus slave recognizes a packet, but determines that there is an error in the request, it will return an exception code reply instead of a data reply. The exception reply consists of the slave address or unit number, a copy of the function code with the high bit set, and an exception code. If the function code was 3, for example, the function code in the exception reply will be 0x83.
 * */
void listen_for_master_request(void){

	uint8_t *request; 
	int offset = 0;

	int read_value = 0;

	while(1){
		do {
			offset = modbus_receive(ctx, request);
		} while (offset == 0);

		if (offset == -1 && errno != EMBBADCRC) {
			LOG_ERR("Error receiving request from master: %s", modbus_strerror(errno));
			/* TODO: goto to terminate? */
			break;
		}

		/* @Note: Analyse the received request:
		 *
		 * The idea is to check what element is being requested access for and then check the access rights
		 * */
		switch(request[offset]){
			case READ_COIL:/* these are special registers */
			case READ_DISCRETE_INPUT:
			case WRITE_SINGLE_COIL:
			case WRITE_MULTIPLE_COILS:
				/* TODO: an exception response should be done */
				USER_ERR("These are special registers, exiting ...");
				break;
			case READ_HOLDING_REGISTER: 
				ethread_lock_thread(eti_listener);
				read_value = get_slave_configuration(READ_HOLDING_REGISTER);
				modbus_tcp_send(ctx, request);
				ethread_unlock(eti_listener);
				break;
			case READ_INPUT_REGISTER: // case READ_INPUT_REGISTER
				ethread_lock(eti_listener);
				read_value = get_slave_configuration(READ_INPUT_REGISTER);
				modbus_tcp_send(ctx, request);
				ethread_unlock(eti_listener);
				break;
			case WRITE_SINGLE_HOLDING_REGISTER: // it is a write single holding register, but how to check it out? TODO
				set_slave_configuration(WRITE_HOLDING_REGISTER);	
				modbus_tcp_send(ctx, request);
				break;
			case WRITE_MULTIPLE_HOLDING_REGISTER: // it is a write multiple holding register
				set_slave_configuration(WRITE_MULTIPLE_HOLDING_REGISTER);
				modbus_tcp_send(ctx, request);
				break;
			default:
				USER_ERR("Unknown function code ...");
				/* for function codes not supported send an ExceptionCode = 1, as mentioned in the protocol 
				 * spec
				 * */
				goto terminate_modbus;
		}	
		goto terminate_modbus;
	}

	resp_length = modbus_reply(ctx, request, offset, mb_mapping);
	if(resp_length == -1)
		USER_ERR("Error while sending a response to master: %s\n", modbus_strerror(errno));
}
}

/* If two or more slave devices are connected, each Slave must identify itself to the master with its respective Connector ID added as a suffix. E.g. #2 or #005 should be appended to the ChargePointID. Suffixes #, #0 or #1 are reserved and cannot be used.
 * */
static void *slave_identify(void){
	/* in rtu context it is required to know the slave ID (aka slave address, unit number, unit ID) 
	 *
	 *  In the TCP it is not required but still need the unit ID in some cases */
	int *controller_ID;

	controller_ID = configure_controller_for_slave_role();
	if(*controller_ID == -1){
		LOG_ERR("error getting the slave controller ID ...");
		return -1
	}

	return controller_ID;
}

/* TODO: terminate this routine
 * */
static char* configure_controller_for_slave_role(void){

	/* first we need to chack whether it is possible to config the slave */
	if(configure_slave_internal()==-1){
		USER_ERR("Could configure the slave internals...");
		return -1;
	}

	/* we need to configure the chargepoint ID, as mentioned above some IDs cannot be considered for that */
	if(strcmp("#",connectorID)!=0 || ("#",connectorID)!=0 || ("#",connectorID)!=0){
		USER_ERR("These are reserved IDs");
		return -1;
	}

	return strcat_s(ChargePointID, strlen(ChargePointID)+strlen(connectorID)+1, connectorID);
}

/* @ slave_set_configuration:
 *    configuration names will be got from the register values 
 * */
static void *set_slave_configuration(uint *query, uint_t action){

	int *request = query;

	/* check for write access to data with read only access rights
	 * Here is the validation is done:
	 * 1- validate the data address --> otherwise fire ExceptionCode = 2
	 * 2- validate data value --> otherwise fire ExceptionCode = 3
	 * 3- execute the modbus function --> otherwise fire ExceptionCode = 4, 5, 6
	 * */
	for (i=0; i < nb_read_register; i++){
		if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== read_register[i]) {
			LOG_ERR("Trying to write to read only data, exception sent ... \n");
			modbus_reply_exception(ctx, request, MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
			break;
		}
	}
	/* check for the size? 2/4 bytes?
	 * */
	config_index = *new_value;
}

/* this is a routine to read element data
 * The registers concerned are the read HOLDING and INPUT
 * */

static uint_t get_slave_configuration(uint_t *configuration_name){

	int i = 0;

	for (i=0; i < nb_read_register; i++){
		if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== read_register[i]) {

		}

		uint *config_address = configuration_name;
	}

	/* Close the connection and free memory
	 * */
	void terminate_modbus_tcp(modbus_t *ctx){

		/* Stop the listening thread, and giving it a 100 ms for cleanups
		 * */
		ethread_stop(eti_listener, 100);

		/* Terminate the modbus communication
		 * */
terminate_modbus:
		modbus_mapping_free(mb_mapping);
		free(request);
		/* TODO: related only to rtu context? */
		modbus_close(ctx);
		modbus_free(ctx);
	}
