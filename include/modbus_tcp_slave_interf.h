/* @file: modbus_tcp_slave.h
 *
 * The work that I have done is flexible for tcp both forms: the tcp v 4 and the tcp ip 
 *
 * "The TCP PI (Protocol Indepedent) backend implements a Modbus variant used for communications over TCP IPv4 and IPv6
 *         networks. It does not require a checksum calculation as lower layer takes care of the same.
 *
 *          Contrary to the TCP IPv4 only backend, the TCP PI backend offers hostname resolution but it consumes about 1Kb of
 *          additional memory."
 *
 * */

/* @brief:
 * 
 * Implementing a slave interface:
 * 	'Slave' controller connects to a master as its backend.
 * 	'Master' controller handles the backend connection and may start and stop transaction for its slaves. 
 * */

#ifndef MODBUS_TCP_SLAVE_INTERF_H_
#define MODBUS_TCP_SLAVE_INTERF_H_

#include <modbus.h>


#ifndef MODBUS_TCP_SLAVE
#define MODBUS_TCP_SLAVE 0xFF /* used to default to the slave number value */
#endif

typedef unsigned int uint;


/* @brief:
 *
 * */
static void init_modbus_tcp_pi(modbus_t *ctx);

/* @brief:
 * 	This will do all the cleanup stuff
 * */
static void terminate_modbus_tcp_pi(modbus_t *ctx);

/* @brief:
 *  	A given contoller can play the role of slave or master. It needs to be configured for the end role. 
 *  	This will configure the controller for the slave role.
 *  	TODO: review 
 * */
static void configure_controller_for_slave_role(void);

/* @brief:
 *	  
 *	  listens to master for requests ...
 *
 * */
void listen_for_master_request(void);

/* @brief:
 *	returns the current value of the configuration name.
 *	we do not need a function code as it is already about reading one register - INPUT or HOLDING
 *	TODO: review
 * */
static uint_t get_slave_configuration(char *query);

/* @brief:
 *	In case of write right, the config can be changed with the new_value
 *  TODO: review
 * */ 
static void set_slave_configuration(uint8_t *query, uint_t action);

/*
 *	@endif
 * */
#endif
