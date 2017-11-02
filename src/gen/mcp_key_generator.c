/*
 * key generator
 * 2017 Tong Zhang<ztong@vt.edu>
 */
#include <mcp_core.h>

static int
item_key_ticker(struct context *ctx, void *arg)
{
    struct dist_info *di = arg;

    di->next(di);

    return 0;
}

static void
trigger(struct context *ctx, event_type_t type, void *rarg, void *carg)
{
    struct gen *g = &ctx->key_gen;
    struct dist_info *key_di = &ctx->key_dist;

    ASSERT(type == EVENT_GEN_KEY_TRIGGER);

    gen_start(g, ctx, key_di, item_key_ticker, key_di, EVENT_GEN_KEY_FIRE);
}

static void
init(struct context *ctx, void *arg)
{
    ecb_register(ctx, EVENT_GEN_KEY_TRIGGER, trigger, NULL);
}

static void
no_op(struct context *ctx, void *arg)
{
    /* do nothing */
}

struct load_generator key_generator = {
    "generate item keys",
    init,
    no_op,
    no_op,
    no_op
};
