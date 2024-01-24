#ifndef MODEL_STATE_H
#define MODEL_STATE_H

/** @file
 * Definition of messages and LP states for this simulation.
 */

#include <ross.h>
#include <stdbool.h>
#include <assert.h>

// ================================ State struct ===============================

struct ProducerState {
  int number_left;
};

static inline bool is_valid_ProducerState(struct ProducerState * hs) {
    return hs->number_left >= 0;
}

static inline void assert_valid_ProducerState(struct ProducerState * hs) {
#ifndef NDEBUG
    assert(hs->number_left >= 0);
#endif // NDEBUG
}

struct CruncherState {
  int total;
  int times_added;  // counts the number of times `MESSAGE_CRUNCHER_add` was processed
};

static inline bool is_valid_CruncherState(struct CruncherState * hs) {
    return hs->times_added >= 0;
}

static inline void assert_valid_CruncherState(struct CruncherState * hs) {
#ifndef NDEBUG
#endif // NDEBUG
}

// ========================= Message enums and structs =========================

/** There are two types of LPs. */
enum MESSAGE_LP_TYPE {
  MESSAGE_LP_TYPE_producer,
  MESSAGE_LP_TYPE_cruncher,
};

enum MESSAGE_PRODUCER {
  MESSAGE_PRODUCER_next,
  MESSAGE_PRODUCER_total,
};

enum MESSAGE_CRUNCHER {
  MESSAGE_CRUNCHER_initialize,
  MESSAGE_CRUNCHER_add,
  MESSAGE_CRUNCHER_return,
};

// There is only one message structure definition
struct Message {
  enum MESSAGE_LP_TYPE lp_type;

  union {
    struct { // message lp type = producer
        enum MESSAGE_PRODUCER prod_type;
        int total;
    };
    struct { // message type = cruncher
        enum MESSAGE_CRUNCHER crunch_type;
        int to_add;
    };
  };
};

static inline bool is_valid_Message(struct Message * msg) {
    switch (msg->lp_type) {
        case MESSAGE_LP_TYPE_producer:
            if (msg->prod_type > MESSAGE_PRODUCER_total) {
                return false;
            }
            break;
        case MESSAGE_LP_TYPE_cruncher:
            if (msg->crunch_type > MESSAGE_CRUNCHER_return) {
                return false;
            }
            break;
    }
    return true;
}

// This function might fail if `msg->rev_state` doesn't point to a
// valid memory region
static inline void assert_valid_Message(struct Message * msg) {
#ifndef NDEBUG
#endif // NDEBUG
}

#endif /* end of include guard */
