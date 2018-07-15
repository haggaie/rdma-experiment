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

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>

static char *server = "127.0.0.1";
static char *port = "7471";

#define NUM_MESSAGES 1

static struct rdma_cm_id *id;
static struct ibv_mr *mr, *send_mr;
static int send_flags = 0;
static struct message send_msg[NUM_MESSAGES];
static struct message recv_msg[NUM_MESSAGES];
static struct database_info db_info;

int transmit_message(struct message *msg)
{
	struct ibv_wc wc;
	int ret;

	ret = rdma_post_send(id, 0, msg, sizeof(*msg), send_mr, send_flags);
	if (ret) {
		perror("rdma_post_send");
		return -1;
	}

	while ((ret = rdma_get_send_comp(id, &wc)) == 0);
	if (ret < 0 || wc.status != IBV_WC_SUCCESS) {
		perror("rdma_get_send_comp");
		return -1;
	}

	return 0;
}

int generate_request()
{
	struct ibv_wc wc;
	int ret;

	printf("Enter next command:\n"
	       "  %d - disconnect and quit\n"
	       "  %d - set key value pair\n"
	       "  %d - query the value of a given key\n",
	       MSG_DISCONNECT, MSG_SET, MSG_QUERY);

	struct message *msg = &send_msg[0];
	scanf("%d", &msg->type);
	switch (msg->type) {
	case MSG_DISCONNECT:
		break;
	case MSG_SET:
	case MSG_QUERY:
		printf("Enter key:\n");
		scanf("%d", &msg->key);
		if (msg->type == MSG_SET) {
			printf("Enter value:\n");
			scanf("%d", &msg->value);
		}
		break;
	default:
		printf("Unknown type: %d\n", msg->type);
		return -1;
	}

	if (transmit_message(msg))
		return -1;

	/* Terminate the program after sending the disconnect message. */
	if (msg->type == MSG_DISCONNECT)
		return -1;
	return 0;
}

int handle_response()
{
	struct ibv_wc wc;
	int ret;

	while ((ret = rdma_get_recv_comp(id, &wc)) == 0);
	if (ret < 0 || wc.status != IBV_WC_SUCCESS) {
		perror("rdma_get_recv_comp");
		return -1;
	}

	struct message *msg = &recv_msg[wc.wr_id];
	switch (msg->type) {
	case MSG_QUERY_RESP:
		LOG("Got query response: key=%d, value=%d\n", msg->key,
		       msg->value);
		break;
	case MSG_SET_RESP:
		LOG("Got set response: key=%d, value=%d\n", msg->key,
		       msg->value);
		break;
	case MSG_EXCHANGE_DATABASE_INFO:
		db_info = msg->db_info;
		break;
	default:
		printf("Got unknown type: %d\n", msg->type);
		return -1;
	}

	ret = rdma_post_recv(id, (void *)(uintptr_t)wc.wr_id, &recv_msg[wc.wr_id], sizeof(struct message), mr);
	if (ret) {
		perror("rdma_post_recv");
		return -1;
	}

	return 0;
}

void main_loop()
{
	while (1) {
		if (generate_request())
			return;
		if (handle_response())
			return;
	}
}

int get_db_info()
{
	struct message *msg = &send_msg[0];
	msg->type = MSG_EXCHANGE_DATABASE_INFO;

	if (transmit_message(msg))
		return -1;

	if (handle_response())
		return -1;

	return 0;
}
static int run(void)
{
	struct rdma_addrinfo hints, *res;
	struct ibv_qp_init_attr attr;
	struct ibv_wc wc;
	int ret;

	memset(&hints, 0, sizeof hints);
	hints.ai_port_space = RDMA_PS_TCP;
	ret = rdma_getaddrinfo(server, port, &hints, &res);
	if (ret) {
		printf("rdma_getaddrinfo: %s\n", gai_strerror(ret));
		goto out;
	}

	memset(&attr, 0, sizeof attr);
	attr.cap.max_send_wr = attr.cap.max_recv_wr = NUM_MESSAGES;
	attr.cap.max_send_sge = attr.cap.max_recv_sge = 1;
	attr.cap.max_inline_data = 16;
	attr.qp_context = id;
	attr.sq_sig_all = 1;
	ret = rdma_create_ep(&id, res, NULL, &attr);
	if (ret) {
		perror("rdma_create_ep");
		goto out_free_addrinfo;
	}

	mr = rdma_reg_msgs(id, recv_msg, sizeof(recv_msg));
	if (!mr) {
		perror("rdma_reg_msgs for recv_msg");
		ret = -1;
		goto out_destroy_ep;
	}
	if ((send_flags & IBV_SEND_INLINE) == 0) {
		send_mr = rdma_reg_msgs(id, send_msg, sizeof(send_msg));
		if (!send_mr) {
			perror("rdma_reg_msgs for send_msg");
			ret = -1;
			goto out_dereg_recv;
		}
	}

	ret = post_recv_all(id, mr, recv_msg, NUM_MESSAGES);
	if (ret)
		goto out_dereg_send;

	ret = rdma_connect(id, NULL);
	if (ret) {
		perror("rdma_connect");
		goto out_dereg_send;
	}

	ret = get_db_info();
	if (ret)
		goto out_dereg_send;

	main_loop();

out_disconnect:
	rdma_disconnect(id);
out_dereg_send:
	if ((send_flags & IBV_SEND_INLINE) == 0)
		rdma_dereg_mr(send_mr);
out_dereg_recv:
	rdma_dereg_mr(mr);
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

	while ((op = getopt(argc, argv, "s:p:iq")) != -1) {
		switch (op) {
		case 's':
			server = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'i':
			send_flags = IBV_SEND_INLINE;
			break;
		case 'q':
			enable_prints = false;
			break;
		default:
			printf("usage: %s\n", argv[0]);
			printf("\t[-s server_address]\n");
			printf("\t[-p port_number]\n");
			printf("\t[-i inline transmission]\n");
			printf("\t[-q]\tQuiet mode - suppress response prints\n");
			exit(1);
		}
	}

	printf("rdma_client: start\n");
	ret = run();
	printf("rdma_client: end %d\n", ret);
	return ret;
}
