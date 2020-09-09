#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h> // memset
#include <errno.h>
#include <stdlib.h>
#include <modbus/modbus.h>

#define FAILURE -1

const uint16_t UT_BITS_ADDRESS = 0x130;
const uint16_t UT_BITS_NB = 0x25;
const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };

const uint16_t UT_INPUT_BITS_ADDRESS = 0x1C4;
const uint16_t UT_INPUT_BITS_NB = 0x16;
const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_REGISTERS_ADDRESS = 0x160;
const uint16_t UT_REGISTERS_NB = 0x3;
const uint16_t UT_REGISTERS_NB_MAX = 0x20;
const uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x108;
const uint16_t UT_INPUT_REGISTERS_NB = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

int
main (int argc, char ** argv)
{

	modbus_t *ctx = NULL;
	int server_socket = 0;
	fd_set refset;
	int size = 0;
	uint8_t *req = NULL;
	int ret = -1;

	req = malloc(6*sizeof(uint8_t));
	if (req == NULL) {
	  printf ("\n failed to malloc \n");
	  return FAILURE;
	}

	memset(req, 0, 6);
	modbus_mapping_t *mb_mapping = NULL;



	mb_mapping = modbus_mapping_new(2, 2, 2, 2);

	           /* To listen any addresses on port 502 */
           ctx = modbus_new_tcp(NULL, 5002);

           /* Handle until 10 established connections */
           server_socket = modbus_tcp_listen(ctx, 10);
	   modbus_tcp_accept(ctx, &server_socket);

	   size = modbus_receive(ctx, req);
	   if (size >= 0) {
	   	printf ("\n request: %s, size %i \n", req, size);
		// modbus_reply(modbus_t *ctx, const uint8_t *req, int req_length, modbus_mapping_t *mb_mapping)
		ret = modbus_reply(ctx, req, size, mb_mapping);
		if (ret == -1) {
		  printf ("\n  failed to reply: %s  \n", modbus_strerror(errno));
		}

	   }
	   else
		   printf("\n failed to receive: %s \n", modbus_strerror(errno));

           /* Clear the reference set of socket */
           FD_ZERO(&refset);

           /* Add the server socket */
           FD_SET(server_socket, &refset);

           if (select(server_socket + 1, &refset, NULL, NULL, NULL) == -1) {
           }


  return FAILURE;
}
