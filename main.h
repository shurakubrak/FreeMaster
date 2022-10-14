#pragma once

#include <stdio.h>
#include "socket.h"
#include <stdbool.h>

#define DEV_COUNT	200

void* thrd_sock_read(void* arg);
void* thrd_master_send(void* arg);
bool get_par();
void Master(in_addr_t addr);
void Blink(in_addr_t addr);