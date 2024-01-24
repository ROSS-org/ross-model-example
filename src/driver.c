#include "driver.h"
#include <stdbool.h>
#include <ross.h>
#include <stdio.h>

// ================================= Global variables ================================

static int total_iterations = 0;

void driver_config(int total_iter) {
    total_iterations = total_iter;
}


// ========================== Helper functions =========================
void producer_send_self_next(struct tw_lp *lp) {
  uint64_t const self = lp->gid;
  assert(self == 0);
  double const offset = 1;
  tw_event *e = tw_event_new(self, offset, lp);
  struct Message *msg = tw_event_data(e);
  msg->lp_type = MESSAGE_LP_TYPE_producer;
  msg->prod_type = MESSAGE_PRODUCER_next;
  assert_valid_Message(msg);
  tw_event_send(e);
}


void producer_send_add_to_cruncher(struct ProducerState *s, struct tw_lp *lp, int value) {
  double const offset = 0.5;
  uint64_t const other = 1;
  tw_event *e = tw_event_new(other, offset, lp);
  struct Message *msg = tw_event_data(e);
  msg->lp_type = MESSAGE_LP_TYPE_cruncher;
  msg->crunch_type = MESSAGE_CRUNCHER_add;
  msg->to_add = value;
  assert_valid_Message(msg);
  tw_event_send(e);
}


// ========================== Model functions =========================
// These functions are called directly by ROSS

// LP initialization. Called once for each LP
void producer_init(struct ProducerState *s, struct tw_lp *lp) {
  uint64_t const self = lp->gid;
  uint64_t const other = 1;
  assert(self == 0);

  if (total_iterations <= 0) {
      s->number_left = 0;
      return;
  }

  // Initialize Producer iter count
  s->number_left = total_iterations;
  // Send initialization to itself
  producer_send_self_next(lp);

  // Send initialize to cruncher
  double const offset = 0.5;
  tw_event *e = tw_event_new(other, offset, lp);
  struct Message *msg = tw_event_data(e);
  msg->lp_type = MESSAGE_LP_TYPE_cruncher;
  msg->crunch_type = MESSAGE_CRUNCHER_initialize;
  assert_valid_Message(msg);
  tw_event_send(e);

  assert_valid_ProducerState(s);
}


// LP pre_run. Called once for each LP. Used to schedule events to other LPs
// at init (because init cannot schedule events to other events)
void producer_pre_run(struct ProducerState *s, struct tw_lp *lp) {
  uint64_t const self = lp->gid;
  uint64_t const other = 1;
  assert(self == 0);

  if (s->number_left > 0) {
      // Send msg to Cruncher
      double const offset = 1;

      tw_event *e = tw_event_new(other, offset, lp);
      struct Message *msg = tw_event_data(e);
      msg->lp_type = MESSAGE_LP_TYPE_cruncher;
      msg->crunch_type = MESSAGE_CRUNCHER_initialize;
      assert_valid_Message(msg);
      tw_event_send(e);
  }

  assert_valid_ProducerState(s);
}


// Forward event handler
void producer_event(
        struct ProducerState *s,
        struct tw_bf *bitfield,
        struct Message *in_msg,
        struct tw_lp *lp) {
  // initialize the bit field
  (void)bitfield;
  assert(in_msg->lp_type == MESSAGE_LP_TYPE_producer);
  uint64_t const self = lp->gid;
  uint64_t const other = 1;
  assert(self == 0);

  // handle the message
  switch (in_msg->prod_type) {
  case MESSAGE_PRODUCER_next:
    {
      // Message to cruncher
      int const value = s->number_left--;
      producer_send_add_to_cruncher(s, lp, value);

      if (s->number_left > 0) {
        // Message to itself
        producer_send_self_next(lp);
      } else {
        // most offset values don't matter, this does, it has to be processed AFTER the add operation
        // Another option to force an order on message processing (when tied) is user defined priorities
        double const offset = 0.75;
        tw_event *e = tw_event_new(other, offset, lp);
        struct Message *msg = tw_event_data(e);
        msg->lp_type = MESSAGE_LP_TYPE_cruncher;
        msg->crunch_type = MESSAGE_CRUNCHER_return;
        assert_valid_Message(msg);
        tw_event_send(e);
      }
    }
    break;
  case MESSAGE_PRODUCER_total:
    // We print to screen the value that we receive for total, but we only do
    // it at commit, otherwise (in optimistic mode) the message could be
    // printed multiple times if it is rolledback and processed again
    break;
  }
  assert_valid_Message(in_msg);
}


// Reverse Event Handler
// Notice that all operations are reversed using the data stored in either the reverse
// message or the bit field
void producer_event_reverse(
        struct ProducerState *s,
        struct tw_bf *bitfield,
        struct Message *in_msg,
        struct tw_lp *lp) {
  (void)bitfield;
  (void)lp;

  // handle the message
  switch (in_msg->prod_type) {
  case MESSAGE_PRODUCER_next:
    s->number_left++;
    break;
  case MESSAGE_PRODUCER_total:
    break;
  }

  assert_valid_Message(in_msg);
}


// Commit event handler
// This function is only called when it can be make sure that the message won't be
// roll back. Either the commit or reverse handler will be called, not both
void producer_event_commit(
        struct ProducerState *s,
        struct tw_bf *bitfield,
        struct Message *in_msg,
        struct tw_lp *lp) {
  (void)s;
  (void)bitfield;
  (void)lp;

  // handle the message
  if (in_msg->prod_type == MESSAGE_PRODUCER_total) {
      printf("The total is: %d\n", in_msg->total);
  }
  assert_valid_Message(in_msg);
}


void cruncher_init(struct CruncherState *s, struct tw_lp *lp) {
    (void)s;
    (void)lp;
    s->times_added = 0;
    assert_valid_CruncherState(s);
}


// Forward event handler
void cruncher_event(
        struct CruncherState *s,
        struct tw_bf *bitfield,
        struct Message *in_msg,
        struct tw_lp *lp) {
  // initialize the bit field
  (void)bitfield;
  assert(in_msg->lp_type == MESSAGE_LP_TYPE_cruncher);

  uint64_t const self = lp->gid;
  uint64_t const other = 0;
  assert(self == 1);

  switch (in_msg->crunch_type) {
  case MESSAGE_CRUNCHER_initialize:
    s->total = 0;
    break;
  case MESSAGE_CRUNCHER_add:
    //For debugging purposes only, not parallel optimistic safe
    //printf("Received %d with (prev) total of %d", in_msg->to_add, s->total);
    s->total += in_msg->to_add;
    s->times_added++;
    //For debugging purposes only, not parallel optimistic safe
    //printf(" and now total of %d\n", s->total);
    break;
  case MESSAGE_CRUNCHER_return:
    {
      double const offset = 0.75;
      tw_event *e = tw_event_new(other, offset, lp);
      struct Message *msg = tw_event_data(e);
      msg->lp_type = MESSAGE_LP_TYPE_producer;
      msg->prod_type = MESSAGE_PRODUCER_total;
      msg->total = s->total;
      assert_valid_Message(msg);
      tw_event_send(e);
    }
    break;
  }
}


// Reverse Event Handler
void cruncher_event_reverse(
        struct CruncherState *s,
        struct tw_bf *bitfield,
        struct Message *in_msg,
        struct tw_lp *lp) {
  (void)bitfield;
  (void)lp;

  switch (in_msg->crunch_type) {
  case MESSAGE_CRUNCHER_initialize:
    // TODO: This is a potential bug! We are not rollbacking to the actual
    // previous state of the system, but we didn't save it anywhere. Exercise
    // for the reader: FIX
    // s->total = ?
    break;
  case MESSAGE_CRUNCHER_add:
    s->total -= in_msg->to_add;
    s->times_added--;
    break;
  case MESSAGE_CRUNCHER_return:
    break;
  }
}


// Reporting any final statistics for this LP in the file previously opened
void cruncher_final(struct CruncherState *s, struct tw_lp *lp) {
    (void)lp;
    printf("Cruncher processed a total of %d messages\n", s->times_added);
}
