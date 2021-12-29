#include "primitive.h"


int primitive_nclip_strip(pvr_vertex_t *vertex_list, int *index_list, int index_size)
{
	polygon_vertex_t *p = (polygon_vertex_t *)vertex_list;
	int commit_vertex = 0;

	/* Check size */
	if (prim_commit_vert_ready(index_size))
		return -1;

	while (index_size)
	{
		int clip = 0;
		int strip_num = 0;
		int i = 0;

		/* Get strip size. */
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
				prim_commit_poly_vert(&p[index_list[-2]], 0);
				prim_commit_poly_vert(&p[index_list[-1]], 0);
				commit_vertex += 2;

				clip = 3;
			}
			else
			{
				/* 0 inside, 1 outside */
				prim_commit_poly_vert(&p[index_list[-2]], 0);
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], 0);
				commit_vertex += 2;

				clip = 1;
			}
		}
		else
		{
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
			{
				/* 0 outside and 1 inside */
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], 0);
				prim_commit_poly_vert(&p[index_list[-1]], 0);
				commit_vertex += 2;

				clip = 2;
			}
		}

		/* Third point and more. */
		for (i = 3; i <= strip_num; i++)
		{
			int eos = 0;

			/* End of strip. */
			if (i == strip_num)
				eos = 1;

			/* Inside check. */
			if ((1.0f / p[*index_list++].z) >= NEAR_Z)
				clip |= (1 << 2);

			switch (clip)
			{
			case 1: /* 0 inside, 1 and 2 outside */
				/* Pause strip */
				prim_commit_poly_inter(&p[index_list[-1]], &p[index_list[-3]], 1);
				commit_vertex++;
				break;
			case 2: /* 0 outside, 1 inside, 2 outside */
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], eos);
				commit_vertex++;
				break;
			case 3: /* 0 and 1 inside, 2 outside */
				prim_commit_poly_inter(&p[index_list[-1]], &p[index_list[-3]], 0);
				prim_commit_poly_vert(&p[index_list[-2]], 0);
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], eos);
				commit_vertex += 3;
				break;
			case 4: /* 0 and 1 outside, 2 inside */
				prim_commit_poly_inter(&p[index_list[-1]], &p[index_list[-3]], 0);
				if (!(i & 0x01))
				{ /* Turn over */
					prim_commit_poly_inter(&p[index_list[-1]], &p[index_list[-3]], 0);
					commit_vertex++;
				}
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], 0);
				prim_commit_poly_vert(&p[index_list[-1]], eos);
				commit_vertex += 3;
				break;
			case 5: /* 0 inside, 1 outside and 2 inside */
				prim_commit_poly_vert(&p[index_list[-1]], 0);
				prim_commit_poly_inter(&p[index_list[-2]], &p[index_list[-1]], 0);
				prim_commit_poly_vert(&p[index_list[-1]], eos);
				commit_vertex += 3;
				break;
			case 6: /* 0 outside, 1 and 2 inside */
				prim_commit_poly_inter(&p[index_list[-1]], &p[index_list[-3]], 0);
				prim_commit_poly_vert(&p[index_list[-2]], 0);
				prim_commit_poly_vert(&p[index_list[-1]], eos);
				commit_vertex += 3;
				break;
			case 7: /* all inside */
				prim_commit_poly_vert(&p[index_list[-1]], eos);
				commit_vertex++;
				break;
			default:
				break;
			}

			/* Next clip code. */
			clip >>= 1;
		}
	}

	return commit_vertex;
}

int primitive_strip(pvr_vertex_t *vertex_list, int *index_list, int index_size)
{
	polygon_vertex_t *p = (polygon_vertex_t *)vertex_list;
	int commit_vertex = 0;

	/* Check size */
	if (prim_commit_vert_ready(index_size))
		return -1;

	while (index_size)
	{
		int strip_num = 0;

		/* Get strip size. */
		strip_num = *index_list++;
		index_size -= strip_num + 1;
		if (strip_num < 2)
		{
			index_list += strip_num;
			continue;
		}

		for (; strip_num; strip_num--)
		{
			int eos = 0;

			if (strip_num == 1)
				eos = 1;

			prim_commit_poly_vert(&p[*index_list++], eos);

			commit_vertex++;
		}
	}

	return commit_vertex;
}
