#include "primitive.h"



#define INTER_POINT(R, P, Q)                                \
	pw = 1.0f / P.z;                                        \
	qw = 1.0f / Q.z;                                        \
	inter = (pw - NEAR_Z) / (pw - qw);                      \
	R.z = 1.0f / (pw + (qw - pw) * inter);                  \
	R.x = (P.x * pw + (Q.x * qw - P.x * pw) * inter) * R.z; \
	R.y = (P.y * pw + (Q.y * qw - P.y * pw) * inter) * R.z;

int primitive_nclip_modifier(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size)
{
	modifier_header_t *header = (modifier_header_t *)eol_header;
	vertex_3f_t *p = (vertex_3f_t *)vertex_list;
	vertex_3f_t c[2];
	vertex_3f_t cover; /* Cover vertex */
	float inter, pw, qw;
	int cover_flag = 0;
	int commit_faces = 0;

	for (; index_size; index_size -= 3)
	{
		if ((1.0f / p[*index_list++].z) >= NEAR_Z)
		{
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
			{
				if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				{
					/* all inside */
					prim_commit_modi_vert(header, &p[index_list[-3]], &p[index_list[-2]], &p[index_list[-1]]);
					commit_faces++;
					continue;
				}
				else
				{
					/* 0 and 1 inside, 2 outside */
					INTER_POINT(c[0], p[index_list[-2]], p[index_list[-1]])
					INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
					prim_commit_modi_vert(header, &p[index_list[-3]], &p[index_list[-2]], &c[0]);
					prim_commit_modi_vert(header, &c[0], &c[1], &p[index_list[-3]]);
					commit_faces += 2;
				}
			}
			else
			{
				if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				{
					/* 0 inside, 1 outside 2 inside */
					INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
					INTER_POINT(c[1], p[index_list[-2]], p[index_list[-1]])
					prim_commit_modi_vert(header, &p[index_list[-3]], &c[0], &c[1]);
					prim_commit_modi_vert(header, &c[1], &p[index_list[-1]], &p[index_list[-3]]);
					commit_faces += 2;
				}
				else
				{
					/* 0 inside, 1 and 2 outside */
					INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
					INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
					prim_commit_modi_vert(header, &p[index_list[-3]], &c[0], &c[1]);
					commit_faces++;
				}
			}
		}
		else
		{
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
			{
				if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				{
					/* 0 outside, 1 and 2 inside */
					INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
					INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
					prim_commit_modi_vert(header, &c[0], &p[index_list[-2]], &p[index_list[-1]]);
					prim_commit_modi_vert(header, &p[index_list[-1]], &c[1], &c[0]);
					commit_faces += 2;
				}
				else
				{
					/* 0 outside, 1 inside, 2 outside */
					INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
					INTER_POINT(c[1], p[index_list[-2]], p[index_list[-1]])
					prim_commit_modi_vert(header, &c[0], &p[index_list[-2]], &c[1]);
					commit_faces++;
				}
			}
			else
			{
				if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				{
					/* 0 and 1 outside, 2 inside */
					INTER_POINT(c[0], p[index_list[-2]], p[index_list[-1]])
					INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
					prim_commit_modi_vert(header, &c[0], &p[index_list[-1]], &c[1]);
					commit_faces++;
				}
				else
				{
					continue;
				}
			}
		}

		/* Make cover polygon. */

		if (cover_flag)
		{
			prim_commit_modi_vert(header, &cover, &c[0], &c[1]);
			commit_faces++;
		}
		else
		{
			cover = c[0];
			cover_flag = 1;
		}
	}

	/* Send EOL header */
	prim_commit_modi_vert(header, 0, 0, 0);

	return commit_faces;
}

int primitive_nclip_modifier_strip(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size)
{
	modifier_header_t *header = (modifier_header_t *)eol_header;
	vertex_3f_t *p = (vertex_3f_t *)vertex_list;
	vertex_3f_t c[2];
	vertex_3f_t cover; /* Cover vertex */
	float inter, pw, qw;
	int cover_flag = 0;
	int commit_faces = 0;

	while (index_size)
	{
		int clip = 0;
		int strip_num = 0;
		int i;

		/* Get strip size */
		strip_num = *index_list++;
		index_size -= strip_num + 1;
		if (strip_num < 2)
		{
			index_list += strip_num;
			continue;
		}

		/* First and Second point. */
		if ((1.0f / p[*index_list++].z) >= NEAR_Z)
		{
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
			{
				/* 0, 1 inside */
				clip = 3;
			}
			else
			{
				/* 0 inside, 1 outside */
				clip = 1;
			}
		}
		else
		{
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
			{
				/* 0 outside and 1 inside */
				clip = 2;
			}
		}

		/* Third point and more. */
		for (i = 3; i <= strip_num; i++)
		{

			/* Inside check. */
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				clip |= (1 << 2);

			switch (clip)
			{
			case 1: /* 0 inside, 1 and 2 outside */
				INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
				INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
				prim_commit_modi_vert(header, &p[index_list[-3]], &c[0], &c[1]);
				commit_faces++;
				break;
			case 2: /* 0 outside, 1 inside, 2 outside */
				INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
				INTER_POINT(c[1], p[index_list[-2]], p[index_list[-1]])
				prim_commit_modi_vert(header, &c[0], &p[index_list[-2]], &c[1]);
				commit_faces++;
				break;
			case 3: /* 0 and 1 inside, 2 outside */
				INTER_POINT(c[0], p[index_list[-2]], p[index_list[-1]])
				INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
				prim_commit_modi_vert(header, &p[index_list[-3]], &p[index_list[-2]], &c[0]);
				prim_commit_modi_vert(header, &c[0], &c[1], &p[index_list[-3]]);
				commit_faces += 2;
				break;
			case 4: /* 0 and 1 outside, 2 inside */
				INTER_POINT(c[0], p[index_list[-2]], p[index_list[-1]])
				INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
				prim_commit_modi_vert(header, &c[0], &p[index_list[-1]], &c[1]);
				commit_faces++;
				break;
			case 5: /* 0 inside, 1 outside 2 inside */
				INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
				INTER_POINT(c[1], p[index_list[-2]], p[index_list[-1]])
				prim_commit_modi_vert(header, &p[index_list[-3]], &c[0], &c[1]);
				prim_commit_modi_vert(header, &c[1], &p[index_list[-1]], &p[index_list[-3]]);
				commit_faces += 2;
				break;
			case 6: /* 0 outside, 1 and 2 inside */
				INTER_POINT(c[0], p[index_list[-3]], p[index_list[-2]])
				INTER_POINT(c[1], p[index_list[-1]], p[index_list[-3]])
				prim_commit_modi_vert(header, &c[0], &p[index_list[-2]], &p[index_list[-1]]);
				prim_commit_modi_vert(header, &p[index_list[-1]], &c[1], &c[0]);
				commit_faces += 2;
				break;
			case 7: /* all inside */
				prim_commit_modi_vert(header, &p[index_list[-3]], &p[index_list[-2]], &p[index_list[-1]]);
				commit_faces++;
				break;
			default:
				break;
			}

			/* Make cover polygon. */
			if (clip && clip != 7)
			{
				if (cover_flag)
				{
					prim_commit_modi_vert(header, &cover, &c[0], &c[1]);
					commit_faces++;
				}
				else
				{
					cover = c[0];
					cover_flag = 1;
				}
			}

			/* Next clip code. */
			clip >>= 1;
		}
	}

	/* Send EOL header */
	prim_commit_modi_vert(header, 0, 0, 0);

	return commit_faces;
}

#undef INTER_POINT

int primitive_modifier(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size)
{
	modifier_header_t *header = (modifier_header_t *)eol_header;
	vertex_3f_t *p = (vertex_3f_t *)vertex_list;
	int i;

	for (i = 0; i < index_size; i += 3)
	{
		prim_commit_modi_vert(header, &p[index_list[0]], &p[index_list[1]], &p[index_list[2]]);
		index_list += 3;
	}

	/* Send EOL header */
	prim_commit_modi_vert(header, 0, 0, 0);

	return index_size / 3 * 2;
}

int primitive_modifier_strip(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size)
{
	modifier_header_t *header = (modifier_header_t *)eol_header;
	vertex_3f_t *p = (vertex_3f_t *)vertex_list;
	int commit_face = 0;

	while (index_size)
	{
		int strip_num = 0;

		/* Get strip size */
		strip_num = *index_list++;
		index_size -= strip_num + 1;
		if (strip_num < 2)
		{
			index_list += strip_num;
			continue;
		}

		for (strip_num -= 2; strip_num ; strip_num--)
		{
			prim_commit_modi_vert(header, &p[index_list[0]], &p[index_list[1]], &p[index_list[2]]);
			index_list++;
			commit_face++;
		}
	}

	/* Send EOL header */
	prim_commit_modi_vert(header, 0, 0, 0);

	return commit_face;
}
