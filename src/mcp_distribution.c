/*
 *  twemperf - a tool for measuring memcached server performance.
 *  Copyright (C) 2011 Twitter, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <mcp_core.h>

static void
dist_next_deterministic(struct dist_info *di)
{
    di->next_val = 0.5 * (di->min + di->max);
}

static void
dist_next_uniform(struct dist_info *di)
{
    double lower = di->min, upper = di->max;

    di->next_val = lower + (upper - lower) * erand48(di->xsubi);
}

static void
dist_next_exponential(struct dist_info *di)
{
    double mean = 0.5 * (di->min + di->max);

    di->next_val = -mean * log(1.0 - erand48(di->xsubi));
}

static void
dist_next_sequential(struct dist_info *di)
{
    di->next_val = di->min++;
}

//zipfian, a=0.99
#define ZIPF_THETA 0.99
static double
zipf_zeta(double N, double theta)
{
    double ans = 0.0;
    for(int i=1;i<=N;i++)
    {
        ans = ans + pow(1.0/N, theta);
    }
    return ans;
}

static void
dist_next_zipf(struct dist_info *di)
{
    double theta = ZIPF_THETA;
    double N = di->max - di->min;
    double alpha = 1.0 / (1.0 - theta);
    double zetan = di->zeta;//load pre-computed zeta
    double eta
        = (1.0 - pow(2.0/N, 1.0-theta)) /
          (1.0 - zipf_zeta(2.0,theta)/zetan);
    double u = erand48(di->xsubi);
    double uz = u * zetan;
    if (uz<1.0)
    {
        di->next_val = di->min + 1.0;
    }else if (uz<1+pow(0.5,theta))
    {
        di->next_val = di->min + 2.0;
    }else
    {
        di->next_val
            = di->min + 1.0 + (N * pow(eta*u - eta + 1.0, alpha));
    }
}

void
dist_init(struct dist_info *di, dist_type_t type, double min, double max,
          uint32_t id)
{
    di->type = type;

    di->xsubi[0] = (uint16_t)(0x1234 ^ id);
    di->xsubi[1] = (uint16_t)(0x5678 ^ (id << 8));
    di->xsubi[2] = (uint16_t)(0x9abc ^ ~id);

    di->min = min;
    di->max = max;

    switch (di->type) {
    case DIST_NONE:
        di->next = NULL;
        break;

    case DIST_DETERMINISTIC:
        di->next = dist_next_deterministic;
        break;

    case DIST_UNIFORM:
        di->next = dist_next_uniform;
        break;

    case DIST_EXPONENTIAL:
        di->next = dist_next_exponential;
        break;

    case DIST_SEQUENTIAL:
        di->next = dist_next_sequential;
        break;

    case DIST_ZIPF:
        di->zeta = zipf_zeta(max-min+1, ZIPF_THETA);
        di->next = dist_next_zipf;
        break;

    default:
        NOT_REACHED();
    }
    di->next_val = 0.0;
}
