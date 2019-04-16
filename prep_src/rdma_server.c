/*
 * Copyright (c) 2005-2009 Intel Corporation.  All rights reserved.
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
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#define CHAR_NUM 32 


static unsigned msg_length = 16;
static const char *server = "0.0.0.0";
static const char *port = "7471";

static struct rdma_cm_id *listen_id, *id;
static struct ibv_mr *mr;
static uint8_t *recv_msg;

static int run(void)
{
	struct rdma_addrinfo hints, *res;
	struct ibv_qp_init_attr init_attr;
	struct ibv_wc wc;
	int ret;
	
	// Without diving to further details, this section sets the listening port and other params
	memset(&res, 0, sizeof res);
	memset(&hints, 0, sizeof hints);
	hints.ai_flags = RAI_PASSIVE;
	hints.ai_port_space = RDMA_PS_TCP;
	ret = rdma_getaddrinfo(server, port, &hints, &res);
	if (ret) {
		printf("rdma_getaddrinfo: %s\n", gai_strerror(ret));
		return ret;
	}

	// Prepare some attributes for the QP that we are soon going to create.
	memset(&init_attr, 0, sizeof init_attr);
	init_attr.cap.max_send_wr = init_attr.cap.max_recv_wr = 1;
	init_attr.cap.max_send_sge = init_attr.cap.max_recv_sge = 1;
	init_attr.sq_sig_all = 1;

	/* 
		rdma_create_ep - creates an identifier and optional QP used to track communication information.
		id - A reference where the allocated communication identifier will be returned 
	*/
	ret = rdma_create_ep(&listen_id, res, NULL, &init_attr);
	if (ret) {
		perror("rdma_create_ep");
		goto out_free_addrinfo;
	}

	/*
		rdma_listen - Initiate a listen for incoming connection requests or datagram service lookup. 
					  The listen is restricted to the locally bound source address.
	*/
	ret = rdma_listen(listen_id, 0);
	if (ret) {
		perror("rdma_listen");
		goto out_destroy_listen_ep;
	}

	/*
		rdma_get_request - Wait for a connect request from any client. 
						   This command blocks until a connect request arrives.
		id - Representing the connection request that will be returned to the user.
	*/
	printf("rdma_get_request: waiting for request...\n");
	ret = rdma_get_request(listen_id, &id);
	if (ret) {
		perror("rdma_get_request");
		goto out_destroy_listen_ep;
	}

	/* Allocate memory for received messages */
    recv_msg = malloc(msg_length);
	if (!recv_msg) {
		perror("recv msg allocation failed");
		goto out_destroy_accept_ep;
	}

    /* 
		rdma_reg_msgs - Register memory for receiving messages
		id - The id for the QP that was defined in the create_ep command
		recv_msg - The address of the memory buffer(s) to register
		msg_length - the amount of bytes to register
	*/
	mr = rdma_reg_msgs(id, recv_msg, msg_length);
	if (!mr) {
		ret = -1;
		perror("rdma_reg_msgs for recv_msg");
		goto out_free_recv_msg;
	}

	/*
	rdma_post_recv - Post a work request to the receive queue of the queue pair associated with the
					 rdma_cm_id, id. The posted buffer will be queued to receive an incoming message 
					 sent by the remote peer.
	*/
	ret = rdma_post_recv(id, NULL, recv_msg, msg_length, mr);
	if (ret) {
		perror("rdma_post_recv");
		goto out_dereg_recv;
	}

	/*
	rdma_accept - Called from the listening side to accept a connection or datagram service lookup request.
	*/
	ret = rdma_accept(id, NULL);
	if (ret) {
		perror("rdma_accept");
		goto out_dereg_recv;
	}

	// Loop until wc sends a receive completion notification. Once this happens, we
	// know that the recv_msg buffer contains the received message.
		while ((ret = rdma_get_recv_comp(id, &wc)) == 0);
		if (ret < 0) {
			perror("rdma_get_recv_comp");
			goto out_disconnect;
		}
		if (wc.status != 0) {
			printf("got completion with error: %s\n", ibv_wc_status_str(wc.status));
			goto out_disconnect;
		}
		printf("receive completed!! %s \n", recv_msg);

out_disconnect:
	rdma_disconnect(id);
out_dereg_recv:
	rdma_dereg_mr(mr);
out_free_recv_msg:
	free(recv_msg);
	recv_msg = NULL;
out_destroy_accept_ep:
	rdma_destroy_ep(id);
out_destroy_listen_ep:
	rdma_destroy_ep(listen_id);
out_free_addrinfo:
	rdma_freeaddrinfo(res);
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

	printf("rdma_server: start\n");
	ret = run();
	printf("rdma_server: end %d\n", ret);
	return ret;
}
