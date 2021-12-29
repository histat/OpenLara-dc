#include "primitive.h"

static unsigned int *buffer_base[5] = {(unsigned int *)0xe0000000, 0, 0, 0, 0};
static unsigned int *buffer_offset[5] = {0, 0, 0, 0, 0};
static unsigned int *buffer_bottom[5] = {0, 0, 0, 0, 0};

static int current_type = 0;
static int direct_type = 0;

static int ta_lists = 0;

int primitive_buffer_init(int type, void *buffer, int size)
{
	int addr_mod = (int)((unsigned int)buffer & 0x0000001f);

	if (size > 0)
	{
		/* Need byte align and multiple */
		if ((type & 0x01) && (64 > (size - addr_mod)))
		{
			/* 64 byte */
			return -1;
		}
		else if (32 > (size - addr_mod))
		{
			/* 32 byte */
			return -1;
		}

		/* Set primitive buffer */
		buffer_base[type] = (unsigned int *)((unsigned int)buffer & 0xffffffe0);
		buffer_offset[type] = buffer_base[type];
		buffer_bottom[type] = buffer_base[type] + (size >> 2);

		if (type == direct_type)
			direct_type = -1;
	}
	else
	{
		/* No buffer */
		buffer_base[type] = (unsigned int *)0xe0000000;
		buffer_offset[type] = buffer_base[type];
		buffer_bottom[type] = 0;

		direct_type = type;
	}

	if(size > 0)
	  ta_lists |= (1<<type);

	return 0;
}

void primitive_buffer_begin(void)
{
	int i;

	/* Buffer offset clear */
	for (i = 0; i < 5; i++)
		buffer_offset[i] = buffer_base[i];

	/* Direct list start */
	pvr_list_begin(direct_type);
}

void primitive_buffer_flush(void)
{
	unsigned int *d = (unsigned int *)0xe0000000;
	int i;

	/* QACR0 QACR1 */
	volatile int *qacr = (volatile int *)0xff000038;
	qacr[0] = qacr[1] = 0x10;

	/* Direct list end */
	pvr_list_finish();

	/* Flush data */
	for (i = 0; i < 5; i++)
	{
		unsigned int *s = buffer_base[i];
		int cnt = (int)(buffer_offset[i] - buffer_base[i]);

		/* Direct list */
		if (i == direct_type)
			continue;

		if (ta_lists & (1<<i)) {
			/* List start */
			pvr_list_begin(i);

		} else {
			continue;
		}

		/* List disable */
		if (buffer_base[i] == buffer_bottom[i])
			continue;

		/* List start */
		//pvr_list_begin(i);

		while (cnt)
		{
			asm("pref @%0"
				:
				: "r"(s + 8));
			d[0] = *s++;
			d[1] = *s++;
			d[2] = *s++;
			d[3] = *s++;
			d[4] = *s++;
			d[5] = *s++;
			d[6] = *s++;
			d[7] = *s++;
			asm("pref @%0"
				:
				: "r"(d));

			d += 8;
			cnt -= 8;
		}

		/* List end */
		pvr_list_finish();
	}
}

int primitive_header(void *header, int size)
{
	unsigned int *d;
	unsigned int *s = (unsigned int *)header;

	/* Get list type */
	current_type = (s[0] >> 24) & 0x07;

	if (current_type == direct_type)
	{
		/* QACR0 QACR1 */
		volatile int *qacr = (volatile int *)0xff000038;
		qacr[0] = qacr[1] = 0x10;
	}
	else
	{
		/* Check size */
		if (buffer_bottom[current_type] < (buffer_offset[current_type] + (size >> 2)))
			return -1;
	}

	/* Set buffer addr */
	d = buffer_offset[current_type];
	buffer_offset[current_type] += size >> 2;

	/* Write data */
	size >>= 5;

	while (size--)
	{
		asm("pref @%0"
			:
			: "r"(s + 8));
		d[0] = *s++;
		d[1] = *s++;
		d[2] = *s++;
		d[3] = *s++;
		d[4] = *s++;
		d[5] = *s++;
		d[6] = *s++;
		d[7] = *s++;

		if (current_type == direct_type)
		{
			asm("pref @%0"
				:
				: "r"(d));
		}

		d += 8;
	}

	return 0;
}

int prim_commit_vert_ready(int size)
{

	if (current_type == direct_type)
	{
		/* QACR0 QACR1 */
	  	volatile int *qacr = (volatile int *)0xff000038;
		qacr[0] = qacr[1] = 0x10;
	}
	else
	{
		/* Check size */
		if (buffer_bottom[current_type] < (buffer_offset[current_type] + (size >> 2)))
			return -1;
	}

	return 0;
}

void prim_commit_poly_vert(polygon_vertex_t *p, int eos)
{
	polygon_vertex_t *d = (polygon_vertex_t *)buffer_offset[current_type];

	if (eos)
		d->flags = 0xf0000000; /* PVR_CMD_VERTEX_EOL */
	else
		d->flags = 0xe0000000; /* PVR_CMD_VERTEX */

	d->x = p->x;
	d->y = p->y;
	d->z = p->z;
	d->u = p->u;
	d->v = p->v;
	d->base.color = p->base.color;
	d->offset.color = p->offset.color;

	if (current_type == direct_type)
		asm("pref @%0"
			:
			: "r"(d));

	/* Update */
	buffer_offset[current_type] += 8;
}

void prim_commit_poly_inter(polygon_vertex_t *p, polygon_vertex_t *q, int eos)
{
	polygon_vertex_t *d = (polygon_vertex_t *)buffer_offset[current_type];
	packed_color_t c;
	float pw = 1.0f / p->z;
	float qw = 1.0f / q->z;
	float inter = (pw - NEAR_Z) / (pw - qw);

	if (eos)
		d->flags = 0xf0000000; /* PVR_CMD_VERTEX_EOL */
	else
		d->flags = 0xe0000000; /* PVR_CMD_VERTEX */

	d->z = 1.0f / (pw + (qw - pw) * inter);
	d->x = (p->x * pw + (q->x * qw - p->x * pw) * inter) * d->z;
	d->y = (p->y * pw + (q->y * qw - p->y * pw) * inter) * d->z;
	d->u = p->u + (q->u - p->u) * inter;
	d->v = p->v + (q->v - p->v) * inter;

	c.argb[0] = p->base.argb[0] + (q->base.argb[0] - p->base.argb[0]) * inter;
	c.argb[1] = p->base.argb[1] + (q->base.argb[1] - p->base.argb[1]) * inter;
	c.argb[2] = p->base.argb[2] + (q->base.argb[2] - p->base.argb[2]) * inter;
	c.argb[3] = p->base.argb[3] + (q->base.argb[3] - p->base.argb[3]) * inter;
	d->base.color = c.color;

	c.argb[0] = p->offset.argb[0] + (q->offset.argb[0] - p->offset.argb[0]) * inter;
	c.argb[1] = p->offset.argb[1] + (q->offset.argb[1] - p->offset.argb[1]) * inter;
	c.argb[2] = p->offset.argb[2] + (q->offset.argb[2] - p->offset.argb[2]) * inter;
	c.argb[3] = p->offset.argb[3] + (q->offset.argb[3] - p->offset.argb[3]) * inter;
	d->offset.color = c.color;

	if (current_type == direct_type)
		asm("pref @%0"
			:
			: "r"(d));

	/* Update */
	buffer_offset[current_type] += 8;
}

void prim_commit_modi_vert(modifier_header_t *eol_header, vertex_3f_t *a, vertex_3f_t *b, vertex_3f_t *c)
{
	modifier_vertex_t *d;
	static modifier_vertex_t buf __attribute__((aligned(32))) = {
		0xf0000000, /* PVR_CMD_VERTEX_EOL */
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0, 0, 0, 0, 0, 0}};
	static int first = 1;

	if (first)
	{
		unsigned int cmd = eol_header->cmd;
		unsigned int mode1 = eol_header->mode1;

		if (!a)
			return;

		first = 0;

		/* Send first header */
		eol_header->cmd &= 0xffffffbf;
		eol_header->mode1 &= 0x9fffffff;
		primitive_header(eol_header, 32);
		eol_header->mode1 = mode1;
		eol_header->cmd = cmd;

		/* Set next vertex */
		buf.a = *a;
		buf.b = *b;
		buf.c = *c;

		return;
	}
	else if (!a)
	{
		modifier_header_t *h = (modifier_header_t *)buffer_offset[current_type];

		first = 1;

		/* Set header */
		*h = *eol_header;

		/* Update */
		buffer_offset[current_type] += 8;

		d = (modifier_vertex_t *)buffer_offset[current_type];

		/* Last vertex */
		*d = buf;

		if (current_type == direct_type)
			asm("pref @%0"
				:
				: "r"(d));

		/* Update */
		buffer_offset[current_type] += 16;

		return;
	}

	/* Check size */
	if (buffer_bottom[current_type] < (buffer_offset[current_type] + (64 >> 2)))
		return;

	d = (modifier_vertex_t *)buffer_offset[current_type];

	/* Send vertex */
	*d = buf;

	if (current_type == direct_type)
		asm("pref @%0"
			:
			: "r"(d));

	/* Update */
	buffer_offset[current_type] += 16;

	/* Set next vertex */
	buf.a = *a;
	buf.b = *b;
	buf.c = *c;
}

void prim_commit_spri_vert(sprite_vertex_t *p)
{
	sprite_vertex_t *d = (sprite_vertex_t *)buffer_offset[current_type];

	*d = *p;

	if (current_type == direct_type)
		asm("pref @%0"
			:
			: "r"(d));

	/* Update */
	buffer_offset[current_type] += 16;
}
