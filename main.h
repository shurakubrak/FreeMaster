#pragma once

#include <stdio.h>
#include "socket.h"
#include <stdbool.h>

#define DEV_COUNT	200

void* thrd_sock_read(void* arg);
void* thrd_master_send(void* arg);
bool get_par();
void master_msg(devs_t* dev);
void blink(in_addr_t addr);
void reverse(char s[]);
void itoa(int n, char s[]);