#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "w5500_socket.h"


static u8 data_buffer[256];

void task_tcpserver(void *p_arg)
{
	int err;
	struct sockaddr_in server_addr;  
    struct sockaddr_in conn_addr;
	int sock_fd;                /* server socked */
	
	socklen_t addr_len;
	int length;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) 
	{  
        return ;
    }

	server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr =htonl(INADDR_ANY);  
    server_addr.sin_port = 80;

	err = bind(sock_fd,  &server_addr, sizeof(server_addr));  
    if (err < 0) 
	{  
        return ;  
    }

	err = listen(sock_fd, 1);  
    if (err < 0) {  
        return ;  
    }

	//sock_conn = accept(sock_fd, (struct sockaddr *)&conn_addr, &addr_len);
	
	while(1)
	{
		
		length = recvfrom(sock_fd, (unsigned int *)data_buffer, 256, 0, &conn_addr, &addr_len);
		send(sock_fd, (unsigned int *)data_buffer, length, 0);
	}
}

