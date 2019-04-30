#ifndef ODYSSEY_READAHEAD_H
#define ODYSSEY_READAHEAD_H

#include <signal.h>

/*
 * Odyssey.
 *
 * Scalable PostgreSQL connection pooler.
*/

typedef struct od_readahead od_readahead_t;

struct od_readahead
{
	machine_msg_t *buf;
	int            size;
	int            pos;
	int            pos_read;
};

static inline void
assert_readahead_valid(const od_readahead_t *readahead) {
	assert(readahead->buf);
	if (readahead->pos_read > readahead->pos ||
	    readahead->pos > readahead->size )
		raise(SIGABRT);
}

static inline void
od_readahead_init(od_readahead_t *readahead)
{
	readahead->buf      = NULL;
	readahead->size     = 0;
	readahead->pos      = 0;
	readahead->pos_read = 0;
}

static inline void
od_readahead_free(od_readahead_t *readahead)
{
	if (readahead->buf)
		machine_msg_free(readahead->buf);
}

static inline int
od_readahead_prepare(od_readahead_t *readahead, int size)
{
	readahead->size = size;
	readahead->buf = machine_msg_create(size);
	if (readahead->buf == NULL)
		return -1;
	return 0;
}

static inline int
od_readahead_left(od_readahead_t *readahead)
{
	assert_readahead_valid(readahead);
	return readahead->size - readahead->pos;
}

static inline int
od_readahead_unread(od_readahead_t *readahead)
{
	assert_readahead_valid(readahead);
	return readahead->pos - readahead->pos_read;
}

static inline char*
od_readahead_pos(od_readahead_t *readahead)
{
	assert_readahead_valid(readahead);
	return (char*)machine_msg_data(readahead->buf) + readahead->pos;
}

static inline char*
od_readahead_pos_read(od_readahead_t *readahead)
{
	assert_readahead_valid(readahead);
	return (char*)machine_msg_data(readahead->buf) + readahead->pos_read;
}

static inline void
od_readahead_pos_advance(od_readahead_t *readahead, int value)
{
	readahead->pos += value;
	assert_readahead_valid(readahead);
}

static inline void
od_readahead_pos_read_advance(od_readahead_t *readahead, int value)
{
	readahead->pos_read += value;
	assert_readahead_valid(readahead);
}

static inline void
od_readahead_reuse(od_readahead_t *readahead)
{
	size_t unread = od_readahead_unread(readahead);
	if (unread > sizeof(sizeof(kiwi_header_t)))
		return;
	if (unread == 0) {
		readahead->pos      = 0;
		readahead->pos_read = 0;
		return;
	}
	/* save next packet header */
	char *data = machine_msg_data(readahead->buf);
	memmove(data, data + readahead->pos_read, unread);
	readahead->pos      = unread;
	readahead->pos_read = 0;

	assert_readahead_valid(readahead);
}

#endif /* ODYSSEY_READAHEAD_H */
