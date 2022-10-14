#include "main.h"
#include <string.h>


bool is_master = false;
int id = FAIL;
in_addr_t beg_addr = 0;

/*Потоки*/
void* thread_SockUDP_read(void* arg)
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
				is_master = true;
				st = get_time_ms();
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

int main()
{
	get_par();;
	Connect();
	//StartExchange();
	/*поток чтения socket*/
	pthread_t thread_sock_read;
	if (!pthread_create(&thread_sock_read, NULL, thread_SockUDP_read, NULL))
		pthread_detach(thread_sock_read);

	in_addr_t addr = beg_addr;
	char buf[sizeof(msg_t)];
	msg_t msg;
	printf("is slave\n");
	while (1) {
		if (is_master) {
			Master(buf, &msg, htonl(addr));
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
			id =  atoi(buf);
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

void Packed(msg_t* msg, char* bf)
{
	bf[0] = ((char*)(&msg->id))[0];
	bf[1] = ((char*)(&msg->id))[1];
	bf[2] = ((char*)(&msg->id))[2];
	bf[3] = ((char*)(&msg->id))[3];
	bf[4] = ((char*)(&msg->tx))[0];
	bf[5] = ((char*)(&msg->tx))[1];
	bf[6] = ((char*)(&msg->tx))[2];
	bf[7] = ((char*)(&msg->tx))[3];
	bf[8] = ((char*)(&msg->tx))[4];
	bf[9] = ((char*)(&msg->tx))[5];
	bf[10] = ((char*)(&msg->tx))[6];
	bf[11] = ((char*)(&msg->tx))[7];
	bf[12] = ((char*)(&msg->tx))[8];
	bf[13] = ((char*)(&msg->tx))[9];
	bf[14] = ((char*)(&msg->temp))[0];
	bf[15] = ((char*)(&msg->temp))[1];
	bf[16] = ((char*)(&msg->temp))[2];
	bf[17] = ((char*)(&msg->temp))[3];
	bf[18] = ((char*)(&msg->light))[0];
	bf[19] = ((char*)(&msg->light))[1];
	bf[20] = ((char*)(&msg->light))[2];
	bf[21] = ((char*)(&msg->light))[3];
}
//-------------------------------------------

void Master(char* buf, msg_t* msg, in_addr_t addr)
{
	msg->id = id;
	memcpy(msg->tx, "I am", 4);
	msg->temp = 24;
	msg->light = 5000;
	Packed(msg, buf);
	struct in_addr st;
	st.s_addr = addr;
	//printf("send to addr: %s\n", inet_ntoa(st));
	Send(buf, sizeof(msg_t), inet_ntoa(st), 6030);
}