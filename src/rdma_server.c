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

#include "common.h"
#include "database.h"

static char *port = "7471";

#define NUM_MESSAGES 16

static struct rdma_cm_id *listen_id, *id;
static struct ibv_mr *mr, *database_mr;
static int send_flags = 0;
static struct message messages[NUM_MESSAGES];

int handle_message(struct ibv_wc* wc)
{
	struct message *msg = &messages[wc->wr_id];
	int ret;

	switch (msg->type)
	{
	case MSG_DISCONNECT:
		return -1; /* Tell main loop to disconnect */
	case MSG_SET:
		LOG("Got SET request with key %d and value %d\n",
		       msg->key, msg->value);
		msg->type = MSG_SET_RESP;
		if (set_key_value(msg->key, msg->value))
			printf("Invalid key: %d\n", msg->key);
		break;
	case MSG_QUERY:
		LOG("Got QUERY request with key %d\n",
		       msg->key);
		msg->type = MSG_QUERY_RESP;
		if (query_key(msg->key, &msg->value))
			printf("Invalid key: %d\n", msg->key);
		LOG("Returning query response with value: %d\n", msg->value);
		break;
	case MSG_EXCHANGE_DATABASE_INFO:
		msg->db_info.rkey = database_mr->rkey;
		msg->db_info.num_keys = NUM_KEYS;
		msg->db_info.address = (uintptr_t)table;
		break;
	deafult:
		printf("Got invalid request type: %d\n", msg->type);
		return -1;
	}

	ret = rdma_post_send(id, (void *)(uintptr_t)wc->wr_id, msg, sizeof(*msg), mr, send_flags);
	if (ret) {
		perror("rdma_post_send");
		return -1;
	}
}

void main_loop()
{
	struct ibv_wc wc;
	int ret;

	while (1) {
		/* Check for newly received messages */
		ret = rdma_get_recv_comp(id, &wc);
		if (ret < 0) {
			perror("rdma_get_recv_comp");
			return;
		} else if (ret > 0) {
			if (wc.status != IBV_WC_SUCCESS) {
				printf("error: got recieve completion with status code: %d\n", wc.status);
				return;
			}
			if (wc.byte_len != sizeof(struct message)) {
				printf("error: got recieve completion length not matching the expected message length.\n");
				return;
			}
			/* Got a message */
			ret = handle_message(&wc);
			if (ret)
				/* Got disconnect message */
				return;
		}

		/* Check for completed send operations */
		ret = rdma_get_send_comp(id, &wc);
		if (ret < 0)
			perror("rdma_get_send_comp");
		else if (ret > 0) {
			if (wc.status != IBV_WC_SUCCESS) {
				printf("error: got send completion with status code: %d\n", wc.status);
				return;
			}

			/* Reuse the buffer for the next receive operation */
			ret = rdma_post_recv(id, (void *)(uintptr_t)wc.wr_id, &messages[wc.wr_id], sizeof(struct message), mr);
			if (ret) {
				perror("rdma_post_recv");
				return;
			}
		}
	}
}

static int run(void)
{
	struct rdma_addrinfo hints, *res;
	struct ibv_qp_init_attr init_attr;
	struct ibv_qp_attr qp_attr;
	int ret, i;

	memset(&hints, 0, sizeof hints);
	hints.ai_flags = RAI_PASSIVE;
	hints.ai_port_space = RDMA_PS_TCP;
	ret = rdma_getaddrinfo(NULL, port, &hints, &res);
	if (ret) {
		printf("rdma_getaddrinfo: %s\n", gai_strerror(ret));
		return ret;
	}

	memset(&init_attr, 0, sizeof init_attr);
	init_attr.cap.max_send_wr = init_attr.cap.max_recv_wr = NUM_MESSAGES;
	init_attr.cap.max_send_sge = init_attr.cap.max_recv_sge = 1;
	init_attr.cap.max_inline_data = 16;
	init_attr.sq_sig_all = 1;
	ret = rdma_create_ep(&listen_id, res, NULL, &init_attr);
	if (ret) {
		perror("rdma_create_ep");
		goto out_free_addrinfo;
	}

	ret = rdma_listen(listen_id, 0);
	if (ret) {
		perror("rdma_listen");
		goto out_destroy_listen_ep;
	}

	ret = rdma_get_request(listen_id, &id);
	if (ret) {
		perror("rdma_get_request");
		goto out_destroy_listen_ep;
	}

	memset(&qp_attr, 0, sizeof qp_attr);
	memset(&init_attr, 0, sizeof init_attr);
	ret = ibv_query_qp(id->qp, &qp_attr, IBV_QP_CAP,
			   &init_attr);
	if (ret) {
		perror("ibv_query_qp");
		goto out_destroy_accept_ep;
	}

	mr = rdma_reg_msgs(id, messages, sizeof(messages));
	if (!mr) {
		ret = -1;
		perror("rdma_reg_msgs");
		goto out_destroy_accept_ep;
	}

	ret = post_recv_all(id, mr, messages, NUM_MESSAGES);
	if (ret)
		goto out_dereg;

	database_mr = ibv_reg_mr(id->pd, table, sizeof(table),
				 IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE |
				 IBV_ACCESS_LOCAL_WRITE);
	if (!database_mr) {
		ret = -1;
		perror("ibv_reg_mr");
		goto out_dereg;
	}

	ret = rdma_accept(id, NULL);
	if (ret) {
		perror("rdma_accept");
		goto out_dereg;
	}

	main_loop();

out_disconnect:
	rdma_disconnect(id);
out_dereg:
	if (database_mr)
		rdma_dereg_mr(database_mr);
	rdma_dereg_mr(mr);
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

	while ((op = getopt(argc, argv, "p:i")) != -1) {
		switch (op) {
		case 'p':
			port = optarg;
			break;
		case 'i':
			send_flags = IBV_SEND_INLINE;
			break;
		default:
			printf("usage: %s\n", argv[0]);
			printf("\t[-p port_number]\n");
			printf("\t[-i inline transmission]\n");
			exit(1);
		}
	}

	printf("rdma_server: start\n");
	ret = run();
	printf("rdma_server: end %d\n", ret);
	return ret;
}
