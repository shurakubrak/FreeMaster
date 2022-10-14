#pragma once

#include <stdio.h>
#include "socket.h"
#include <stdbool.h>

#define DEV_COUNT	200

void* thread_SockUDP_read(void* arg);
bool get_par();
void Packed(msg_t* msg, char* bf);
void Master(char* buf, msg_t* msg, in_addr_t addr);