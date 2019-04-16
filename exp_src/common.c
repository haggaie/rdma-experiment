#include <stdio.h>

#include "common.h"

int post_recv_all(struct rdma_cm_id *id, struct ibv_mr *mr,
		  struct message *messages, size_t num_messages)
{
	size_t i;
	int ret;

	for (i = 0; i < num_messages; ++i) {
		ret = rdma_post_recv(id, (void *)(uintptr_t)i, &messages[i], sizeof(struct message), mr);
		if (ret) {
			perror("rdma_post_recv");
			return ret;
		}
	}

	return 0;
}

bool enable_prints = true;

