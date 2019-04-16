/*
 * Copyright (c) 2010 Intel Corporation.  All rights reserved.
 *
 * This software is available to you under the OpenIB.org BSD license
 * below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AWV
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>


static unsigned msg_length = 16;

static const char *server = "127.0.0.1";
static const char *port = "7471";

static struct rdma_cm_id *id;
static struct ibv_mr  *send_mr;//, *mr; 
static int send_flags;
static uint8_t *send_msg;
static uint8_t *recv_msg;

static int run(void)
{
	struct rdma_addrinfo hints, *res;
	struct ibv_qp_init_attr attr;
	struct ibv_wc wc;
	int ret;
    printf("start of run\n");

	// Without diving to further details, this section sets the destination address and port
	memset(&hints, 0, sizeof hints);
	hints.ai_port_space = RDMA_PS_TCP;
	ret = rdma_getaddrinfo(server, port, &hints, &res);
	if (ret) {
		printf("rdma_getaddrinfo: %s\n", gai_strerror(ret));
		goto out;
	}

	// Prepare some attributes for the QP that we are soon going to create.
	memset(&attr, 0, sizeof attr);
	attr.cap.max_send_wr = attr.cap.max_recv_wr = 1; //Max number of send/recv requests allowed in the queue
	attr.cap.max_send_sge = attr.cap.max_recv_sge = 1;
	attr.qp_context = id;
	attr.sq_sig_all = 1;
	
	/* 
		rdma_create_ep - creates an identifier and optional QP used to track communication information.
		id - A reference where the allocated communication identifier will be returned 
	*/
	ret = rdma_create_ep(&id, res, NULL, &attr);

	if (ret) {
		perror("rdma_create_ep");
		goto out_free_addrinfo;
	}

	/* Allocate memory for transmitted messages */
    send_msg = malloc(msg_length);
	if (!send_msg) {
		perror("send msg allocation failed");
		goto out_destroy_ep;
	}
	
	/*
		rdma_reg_msgs - Register memory for transmitting messages
		id - The id for the QP that was defined in the create_ep command
		recv_msg - The address of the memory buffer(s) to register
		msg_length - the amount of bytes to register
	*/
	send_msg[0] = 'R';
	send_msg[1] = 'o';
	send_msg[2] = 'y';
	for(int i=3;i<msg_length - 1;i++)
		send_msg[i] = i+64;
	send_msg[msg_length - 1] = 0;
	
	send_mr = rdma_reg_msgs(id, send_msg, msg_length); // Register memory buffer for send message
	if (!send_mr) {
		perror("rdma_reg_msgs for send_msg");
		ret = -1;
		goto out_free_send;
	}
	
	/*
	rdma_connect - Initiate an active connection request. Similar to establishing TCP connection.
	The command will succeed only if the server called rdma_listen
	*/
    printf("Connecting...\n");
	ret = rdma_connect(id, NULL);
	if (ret) {
		perror("rdma_connect");
		goto out_dereg_send;
	}
    printf("sending message\n");
	
	/*
	rdma_post_send - Post a work request to the send queue .The contents of the posted buffer,  
					 send_msg, will be sent to the remote peer of the connection.
	*/
	printf("sending %s \n", send_msg);
	ret = rdma_post_send(id, NULL, send_msg, msg_length, send_mr, send_flags);
	if (ret) {
		perror("rdma_post_send");
		goto out_disconnect;
	}

    printf("waiting for send_comp...\n");
	// Loop until wc with send complete arrives.
	while ((ret = rdma_get_send_comp(id, &wc)) == 0);
	if (ret < 0) {
		perror("rdma_get_send_comp");
		goto out_disconnect;
	}
	if (wc.status != 0) {
		printf("got completion with error: %s\n", ibv_wc_status_str(wc.status));
		goto out_disconnect;
	}

out_disconnect:
	rdma_disconnect(id);
out_dereg_send:
	rdma_dereg_mr(send_mr);
out_free_send:
	free(send_mr);
	send_mr = NULL;
out_destroy_ep:
	rdma_destroy_ep(id);
out_free_addrinfo:
	rdma_freeaddrinfo(res);
out:
	return ret;
}

int main(int argc, char **argv)
{
	int op, ret;

	while ((op = getopt(argc, argv, "s:p:l:")) != -1) {
		switch (op) {
		case 's':
			server = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'l':
			msg_length = atoi(optarg);
			break;
		default:
			printf("usage: %s\n", argv[0]);
			printf("\t[-s server_address]\n");
			printf("\t[-p port_number]\n");
			exit(1);
		}
	}

	printf("rdma_client: starttTTTT\n");
	printf("Another print\n");
	ret = run();
	printf("rdma_client: end %d\n", ret);
	return ret;
}
