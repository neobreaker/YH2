#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_socket.h"
#include "w5500_socket.h"
#include "lib_mem.h"

extern OS_EVENT* mbox_play_rcv;

rcv_pack_t g_pack;

void task_socket(void *p_arg)
{

	g_pack.data = pvPortMalloc(2048);
	
	if(!g_pack.data)
		return ;
	
	while(1)
	{
		g_pack.len = Read_SOCK_Data_Buffer(0, g_pack.data, (u8*)&g_pack.remote_ip, (u8*)&g_pack.remote_port);
		if(g_pack.len > 0)
			OSMboxPost(mbox_play_rcv, &g_pack);
		OSTimeDly(4);
	}
	
}


