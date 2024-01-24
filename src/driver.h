#ifndef MODEL_DRIVER_H
#define MODEL_DRIVER_H

/** @file
 * Functions implementing Model as PDES in ROSS.
 */

#include "state.h"

/** Setting global variables to by the simulation. */
void driver_config(int total_iterations);

// ============== PRODUCER ==============
/** Initialization. */
void producer_init(struct ProducerState *s, struct tw_lp *lp);

/** Scheduling events to other LPs. */
void producer_pre_run(struct ProducerState *s, struct tw_lp *lp);

/** Forward event handler. */
void producer_event(
        struct ProducerState *s,
        struct tw_bf *bf,
        struct Message *in_msg,
        struct tw_lp *lp);

/** Reverse event handler. */
void producer_event_reverse(
        struct ProducerState *s,
        struct tw_bf *bf,
        struct Message *in_msg,
        struct tw_lp *lp);

/** Commit event handler. */
void producer_event_commit(
        struct ProducerState *s,
        struct tw_bf *bf,
        struct Message *in_msg,
        struct tw_lp *lp);


// ============== CRUNCHER ==============
/** Initialization. */
void cruncher_init(struct CruncherState *s, struct tw_lp *lp);

/** Forward event handler. */
void cruncher_event(
        struct CruncherState *s,
        struct tw_bf *bf,
        struct Message *in_msg,
        struct tw_lp *lp);

/** Reverse event handler. */
void cruncher_event_reverse(
        struct CruncherState *s,
        struct tw_bf *bf,
        struct Message *in_msg,
        struct tw_lp *lp);

/** Cleaning and printing info before shut down. */
void cruncher_final(struct CruncherState *s, struct tw_lp *lp);

#endif /* end of include guard */
