#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "lib_mem.h"
#include "task_udpserver.h"
#include "queue.h"
#include "w5500_socket.h"

extern OS_EVENT* sem_vs1053async;
extern OS_EVENT* sem_vs1053_play_async;

struct sockaddr_in g_remote_sin;

u8 is_line_established = 0;		//ͨѶ�����Ƿ���

Queue *s_rcv_queue = NULL;
static rev_buffer_t s_rcv_buffer[RCV_BUFFER_NUM];

void rcv_queue_enqueue(void *data)
{
	if(queue_size(s_rcv_queue) == RCV_BUFFER_NUM)
		queue_poll(s_rcv_queue, NULL);
	
	queue_enqueue(s_rcv_queue, data);
}

void* rcv_queue_dequeue()
{
	void *ret = NULL;

	queue_poll(s_rcv_queue, &ret);

	return ret;
}

static void rev_buffer_init()
{
	int i = 0; 
	
	queue_new(&s_rcv_queue);
	
	for (i = 0; i < RCV_BUFFER_NUM; i++)
	{
		s_rcv_buffer[i].data = pvPortMalloc(RCV_BUFFER_SIZE);
		if(s_rcv_buffer[i].data == NULL)
			return;
		rcv_queue_enqueue(&s_rcv_buffer[i]);
		s_rcv_buffer[i].len = 0;
	}
}

static u32 time_stamp = 0, time_elapse = 0;
void task_udpserver(void *p_arg)
{
	
	int err;	
	struct sockaddr_in sin;
	socklen_t sin_len; 
	int sock_fd;                
	int ret = 0;
	rev_buffer_t *pbuff = NULL;
	u32 now = 0;
	rev_buffer_init();
	
	
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) 
	{  
        return ;
    }

	sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr=INADDR_ANY;  
    sin.sin_port=SRC_RCV_PORT;
	sin_len = sizeof(sin);
	
	err = bind(sock_fd, &sin, sizeof(sin));  
    if (err < 0) 
	{  
        return ;  
    }
	
	while(1)
	{
		pbuff = rcv_queue_dequeue();
		
		if(pbuff == NULL)
			while(1);
			
		pbuff->len = recvfrom(sock_fd, pbuff->data, RCV_BUFFER_SIZE, 0, &g_remote_sin, &sin_len);

		now = OSTimeGet();
		time_elapse = now - time_stamp;
		time_stamp = now;
		
		ret = parse_AT(pbuff->data, RCV_BUFFER_SIZE);
		if(ret)
		{
			pbuff->len = 0;
			if(is_line_established)
			{
				OSSemPost(sem_vs1053async);
				OSSemPost(sem_vs1053_play_async);
			}
		}
		rcv_queue_enqueue(pbuff);
		
	}
	
}

//return 0 : NOT AT  1: AT
int parse_AT(u8* buffer, int len)
{
	u8 cmd = 0;
	if(buffer[0] == 'A' && buffer[1] == 'T')
	{
		cmd = buffer[2];

		switch(cmd)
		{
		case AT_LINE_EATABLISH:
			is_line_established = 1;
			break;
		case AT_LINE_SHUTDOWN:
			is_line_established = 0;
			break;
		}
		return 1;
	}
	return 0;
}


