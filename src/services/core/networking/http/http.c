#include "curl.h"

typedef struct {
	
	uint32_t http_code;
	uint32_t http_buffer[MAX_BUFFER_HTTP];
	
	
	
} Http_handle;

static Http_handle 
http_init (void) {
	
	
	
	return SUCCESS; 
}
