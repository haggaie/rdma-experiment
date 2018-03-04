/* Common definitions for RDMA example client and server */

/* Only include this header file once in a given compilation */
#pragma once

#include <sys/types.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>

/* Types of messages the client and server exchange */
enum message_type
{
	/* End communication */
	MSG_DISCONNECT = 0,
	/* Set a key-value pair in the database */
	MSG_SET = 1,
	/* Acknowledge a key-value pair was received and set */
	MSG_SET_RESP = 2,
	/* Query a value of a given key from the database */
	MSG_QUERY = 3,
	/* A valid value was found in the database. Returns value == -1 if the
	 * key is missing */
	MSG_QUERY_RESP = 4,
};

/* Structure of a message between the client and the server */
struct message
{
	/* Enum classifying this message */
	enum message_type type;
	/* Key by which values are indexed */
	int key;
	/* Value to associate with the key */
	int value;
};

int post_recv_all(struct rdma_cm_id *id, struct ibv_mr *mr,
		  struct message *messages, size_t num_messages);
