#include "zs.h"
#include "btree_sync_mbox.h"

struct Scavenge_Arg {
        int             type;
        ZS_cguid_t     cguid;
        uint64_t        snap_seq;
        uint64_t        active_seqno;
        int             btree_index;
	int		throttle_value;
        struct btree_raw        *btree;
        struct btree *bt;
        struct ZS_state        *zs_state;
};
typedef struct Scavenge_Arg Scavenge_Arg_t;

void scavenger_init(int nthreads);
void scavenger_stop();
void astats_init(int nthreads);
void astats_stop();

ZS_status_t btree_scavenge(struct ZS_state  *zs_state, Scavenge_Arg_t S);

typedef struct astats_arg_ {
    ZS_cguid_t     cguid;
    int             btree_index;
    struct btree_raw        *btree;
    struct btree *bt;
    struct ZS_state        *zs_state;
    int suspend_duration;
    int suspend_after_node_count;
} astats_arg_t;

ZS_status_t btree_start_astats(struct ZS_state  *zs_state, astats_arg_t S);
