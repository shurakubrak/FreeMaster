#include "main.h"
#include <string.h>

bool is_master = false;
int32_t ID = FAIL;
in_addr_t beg_addr = 0;
devs_t devs;
msg_t msg;
char buf_blink[sizeof(int32_t) + 1] = {BLINK,0,0,0,0};
char buf_msg[sizeof(msg_t) + 1] = { MSG,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

/*Потоки*/
void* thrd_sock_read(void* arg)
{
	printf("Start thread for socket\n");
	while (!s_connect())
		ssleep(3);

	uint64_t st = get_time_ms();
	int res = 0;
	while (1) {
		res = s_recive();
		switch (res) {
		case FAIL:
			while (!s_connect())
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
	s_close();
	printf("Stop thread for connection socket\n");
	return NULL;
}
/****************************************************/

void* thrd_master_send(void* arg)
{
	devs_t dev;
	while (1) {
		for (size_t i = 0; i < DEVS_COUNT; i++) {
			dev.id = i;/*просто через id удобнее передать индекс*/
			if (devs_access(&dev, READ_MIDL))
				master_msg(&dev);
			msleep(50);
		}
	}
	return NULL;
}
/****************************************************/

int main()
{
	if (!get_par()) {
		printf("Settings error\n");
		return -1;
	}
	s_connect();
	devs_init(beg_addr);

	/*поток чтения socket*/
	pthread_t thread_sock_read;
	if (!pthread_create(&thread_sock_read, NULL, thrd_sock_read, NULL))
		pthread_detach(thread_sock_read);
	
	pthread_t thread_master_send;
	if (!pthread_create(&thread_master_send, NULL, thrd_master_send, NULL))
		pthread_detach(thread_master_send);

	printf("is slave\n");
	devs_t dev;
	while (1) {
		for (size_t i = 0; i < DEVS_COUNT; i++) {
			if (is_master) {
				blink(htonl(beg_addr + i));
				dev.id = i;
				devs_access(&dev, CLEAR);
				msleep(20);
			}
			else
				ssleep(1);
		}
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

void master_msg(devs_t* dev)
{
	msg.id = ID;
	char t[] = {0,0,0,0,0,0,0,0};
	itoa(dev->temp[0], t);
	memcpy(msg.tx, "t.", 2);
	memcpy(&msg.tx[2], t, 8);
	msg.temp = dev->temp[0];
	msg.light = dev->light[0];

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

	s_send(buf_msg, 1 + sizeof(msg_t), dev->addr, UDP_PORT);
}
//---------------------------------------------------

void blink(in_addr_t addr)
{
	buf_blink[1] = ((char*)(&ID))[0];
	buf_blink[2] = ((char*)(&ID))[1];
	buf_blink[3] = ((char*)(&ID))[2];
	buf_blink[4] = ((char*)(&ID))[3];
	
	s_send(buf_blink, 1 + sizeof(int32_t), addr, UDP_PORT);
}
//----------------------------------------------------------

/* reverse:  переворачиваем строку s на месте */
void reverse(char s[])
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
void itoa(int n, char s[])
{
	int i, sign;
	if ((sign = n) < 0) n = -n;
	i = 0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0) s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}
//-------------------------------------------------------------------------


