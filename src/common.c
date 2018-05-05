/***************************************************************************
 *            common.c
 *
 *  Copyright  2016  mc
 *  <mcarmen@<host>>
 ****************************************************************************/

#include "common.h"

rule setRule(char* src_dst_address, char* ip_address, int netmask, char* src_dst_port, int port)
{
    rule r;

    if(strcmp(src_dst_address, "src") == 0)
        r.src_dst_addr = SRC;
    else
        if(strcmp(src_dst_address, "dst") == 0)
            r.src_dst_addr = DST;

    inet_aton(ip_address, &r.addr);

    r.mask = netmask;

    if(strcmp(src_dst_port, "dport") == 0)
        r.src_dst_port = DST;
    else
        r.src_dst_port = SRC;

    r.port = port;

    return r;
}

void print(rule r)
{
    char src_dst_addr[MAX_BUFF_SIZE], src_dst_port[MAX_BUFF_SIZE];

    if(r.src_dst_addr == SRC)
        strncpy(src_dst_addr, "src", strlen("src"));
    else if (r.src_dst_addr == DST)
        strncpy(src_dst_addr, "dst", strlen("dst"));

    if(r.src_dst_port == SRC)
        strncpy(src_dst_port, "sport", strlen("sport"));
    else if(r.src_dst_port == DST)
        strncpy(src_dst_port, "dport", strlen("dport"));

    printf("%s %s %d %s %d\n", src_dst_addr, inet_ntoa(r.addr), r.mask, src_dst_port, r.port);
}
