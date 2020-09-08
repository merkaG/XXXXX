#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#include <modbus/modbus.h>

#define FAILURE -1

int
main (int argc, char ** argv)
{

	modbus_t *ctx = NULL;
	int server_socket = 0;
	fd_set refset;
	int size = 0;
	uint8_t *req = NULL;
	int ret = -1;

	modbus_mapping_t *mb_mapping = NULL;

	           /* To listen any addresses on port 502 */
           ctx = modbus_new_tcp(NULL, 5002);

           /* Handle until 10 established connections */
           server_socket = modbus_tcp_listen(ctx, 10);
	   modbus_tcp_accept(ctx, &server_socket);

	   size = modbus_receive(ctx, req);
	   if (size >= 0) {
	   	printf ("\n request: %s, size \n", req);
		// modbus_reply(modbus_t *ctx, const uint8_t *req, int req_length, modbus_mapping_t *mb_mapping)
		ret = modbus_reply(ctx, req, size, NULL);
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
