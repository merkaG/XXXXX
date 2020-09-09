/* Copyright (c) Me 
 *
 *  @date 07/09/2020
 *
 *
 *  @brief: check the modbus server role
 *
 * */

#include <stdio.h>
#include <modbus/modbus.h>
#include <errno.h>

#define FAILURE -1

#define LOG_ERR printf
#define LOG_DBG printf

#define IP "127.0.0.1" // we do local check, to be edited for further checks
#define PORT 5002

// I think this is already declared soewhere in the standard lib, but I dunno :(
#if !defined SUCCESS 
#define SUCCES 0
#endif


int 
main(int argc, char** argv)
{

  /* Read 5 holding registers from address 1 */
  uint8_t raw_req[] = { 0xFF, MODBUS_FC_READ_HOLDING_REGISTERS, 0x00, 0x01, 0x0, 0x05 };
  int req_size = 0;
  uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];
  int resp_size = 0;

  modbus_t *modbus_ctx = NULL;

  modbus_ctx = modbus_new_tcp(IP, PORT);	

  if (modbus_ctx == NULL)
  {
	LOG_ERR("failed to allocate for modbus context");
	return FAILURE;
  }

  // try connect and see result
  if (modbus_connect(modbus_ctx) == -1) {
  
    LOG_ERR("modbus connect fails: %s\n", modbus_strerror(errno));
    goto end;
  }
  
  req_size = modbus_send_raw_request(modbus_ctx, raw_req, 6 * sizeof(uint8_t));
  if (req_size == -1) {
	  LOG_ERR("failed to send request: %s", modbus_strerror(errno));
	  goto end;
  }

  resp_size = modbus_receive_confirmation(modbus_ctx, rsp);
  if (resp_size == -1) {
	  LOG_ERR("failed to receive confirmfirmation: %s", modbus_strerror(errno));
	  goto end;
  }

  LOG_DBG("got response : %s", rsp);

  // don't forget to modbus_close !!
  modbus_close(modbus_ctx);
  modbus_free(modbus_ctx);

	return SUCCES;

  // free the context
end:
  if (modbus_ctx)
	  modbus_free(modbus_ctx);

	return FAILURE;
}
