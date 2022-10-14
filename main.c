#include "main.h"
#include <string.h>


bool is_master = false;
int32_t ID = FAIL;
in_addr_t beg_addr = 0;

msg_t msg;
char buf_blink[sizeof(int32_t) + 1] = {BLINK,0,0,0,0};
char buf_msg[sizeof(msg_t) + 1] = { MSG,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

/*Потоки*/
void* thrd_sock_read(void* arg)
{
	printf("Start thread for socket");
	while (!Connect())
		ssleep(3);

	uint64_t st = get_time_ms();
	int res = 0;
	while (1) {
		res = Recive();
		switch (res) {
		case FAIL:
			while (!Connect())
				ssleep(3);
			break;
		case SOCK_BUF_EMPTY:
			if ((get_time_ms() - st) > 6000) {
				st = get_time_ms();
				is_master = true;
				printf("is master\n");
			}
			break;
		case OK:
			st = get_time_ms();
			break;
		}
		pthread_yield();
	}
	Close();
	printf("Stop thread for connection socket\n");
	return NULL;
}
/****************************************************/

void* thrd_master_send(void* arg)
{
	return NULL;
}
/****************************************************/

int main()
{
	get_par();;
	Connect();
	
	/*поток чтения socket*/
	pthread_t thread_sock_read;
	if (!pthread_create(&thread_sock_read, NULL, thrd_sock_read, NULL))
		pthread_detach(thread_sock_read);
	
	pthread_t thread_master_send;
	if (!pthread_create(&thread_master_send, NULL, thrd_master_send, NULL))
		pthread_detach(thread_master_send);

	in_addr_t addr = beg_addr;
	
	
	printf("is slave\n");
	while (1) {
		if (is_master) {
			//Master(htonl(addr));
			Blink(htonl(addr));
			if (addr < beg_addr + DEV_COUNT - 1)
				addr++;
			else
				addr = beg_addr;
			msleep(23);
		}
		else
			ssleep(1);
	}
	return 0;
}
//--------------------------------------

bool get_par()
{
	FILE* fd;
	char buf[15];

	fd = fopen("idd", "r");
	if (fd != NULL) {
		if (fgets(buf, sizeof(buf), fd) != NULL) {
			ID =  atoi(buf);
			if (fgets(buf, sizeof(buf), fd) != NULL) {
				in_addr_t addr = inet_addr(buf);
				if (addr != FAIL) {
					beg_addr = ntohl(addr);
					if (beg_addr != FAIL)
						return true;
				}
			}
		}
	}
	fclose(fd);
	return false;
}
//--------------------------------------

void Master(in_addr_t addr)
{
	msg.id = ID;
	memcpy(msg.tx, "I am", 4);
	msg.temp = 24;
	msg.light = 5000;

	buf_msg[1] = ((char*)(&msg.id))[0];
	buf_msg[2] = ((char*)(&msg.id))[1];
	buf_msg[3] = ((char*)(&msg.id))[2];
	buf_msg[4] = ((char*)(&msg.id))[3];
	buf_msg[5] = ((char*)(&msg.tx))[0];
	buf_msg[6] = ((char*)(&msg.tx))[1];
	buf_msg[7] = ((char*)(&msg.tx))[2];
	buf_msg[8] = ((char*)(&msg.tx))[3];
	buf_msg[9] = ((char*)(&msg.tx))[4];
	buf_msg[10] = ((char*)(&msg.tx))[5];
	buf_msg[11] = ((char*)(&msg.tx))[6];
	buf_msg[12] = ((char*)(&msg.tx))[7];
	buf_msg[13] = ((char*)(&msg.tx))[8];
	buf_msg[14] = ((char*)(&msg.tx))[9];
	buf_msg[15] = ((char*)(&msg.temp))[0];
	buf_msg[16] = ((char*)(&msg.temp))[1];
	buf_msg[17] = ((char*)(&msg.temp))[2];
	buf_msg[18] = ((char*)(&msg.temp))[3];
	buf_msg[19] = ((char*)(&msg.light))[0];
	buf_msg[20] = ((char*)(&msg.light))[1];
	buf_msg[21] = ((char*)(&msg.light))[2];
	buf_msg[22] = ((char*)(&msg.light))[3];

	Send(buf_msg, 1 + sizeof(msg_t), addr, UDP_PORT);
}
//---------------------------------------------------

void Blink(in_addr_t addr)
{
	buf_blink[1] = ((char*)(&ID))[0];
	buf_blink[2] = ((char*)(&ID))[1];
	buf_blink[3] = ((char*)(&ID))[2];
	buf_blink[4] = ((char*)(&ID))[3];
	
	//st.s_addr = addr;
	//printf("send to addr: %s\n", inet_ntoa(st));
	//in_addr_t a_n = htonl(addr);
	//in_addr_t a_s = htons(addr);
	Send(buf_blink, 1 + sizeof(int32_t), addr, UDP_PORT);
}
