#include "driver.h"
#include "mapping.h"
#include "utils.h"
#include <model_config.h>


/** Defining LP types.
 * - These are the functions called by ROSS for each LP
 * - Multiple sets can be defined (for multiple LP types)
 */
tw_lptype model_lps[] = {
    {(init_f)    producer_init,
     (pre_run_f) producer_pre_run,
     (event_f)   producer_event,
     (revent_f)  producer_event_reverse,
     (commit_f)  producer_event_commit,
     (final_f)   NULL,
     (map_f)     model_map,
     sizeof(struct ProducerState)
    },
    {(init_f)    cruncher_init,
     (pre_run_f) NULL,
     (event_f)   cruncher_event,
     (revent_f)  cruncher_event_reverse,
     (commit_f)  NULL,
     (final_f)   cruncher_final,
     (map_f)     model_map,
     sizeof(struct CruncherState)
    },
    {0},
};

/** Define command line arguments default values. */
static int total_iter = 10;

/** Custom to Model command line options. */
static tw_optdef const model_opts[] = {
    TWOPT_GROUP("Model"),
    TWOPT_UINT("total-iter", total_iter, "number of numbers to send to secondary LP"),
    TWOPT_END(),
};


int main(int argc, char *argv[]) {
  tw_opt_add(model_opts);
  tw_init(&argc, &argv);

  // Do some error checking?
  //if (g_tw_mynode == 0) {
  //  check_folder("output");
  //}
  // Setting the driver configuration should be done before running anything
  driver_config(total_iter);
  // Print out some settings?
  if (g_tw_mynode == 0) {
    printf("Model git version: " MODEL_VERSION "\n");
  }

  // Custom Mapping
  /*
  g_tw_mapping = CUSTOM;
  g_tw_custom_initial_mapping = &model_custom_mapping;
  g_tw_custom_lp_global_to_local_map = &model_mapping_to_lp;
  */

  // Useful ROSS variables and functions
  // tw_nnodes() : number of nodes/processors defined
  // g_tw_mynode : my node/processor id (mpi rank)

  // Useful ROSS variables (set from command line)
  // g_tw_events_per_pe
  // g_tw_lookahead
  // g_tw_nlp
  // g_tw_nkp
  // g_tw_synchronization_protocol
  // g_tw_total_lps

  if (tw_nnodes() > 2) {
      tw_error(TW_LOC, "This model can be run in 1 or 2 PEs only! Attempting to run on %d nodes", tw_nnodes());
  }

  // There are only two LPs running in this simulation
  int const num_lps_in_pe = tw_nnodes() == 1 ? 2 : 1;

  // set up LPs within ROSS
  tw_define_lps(num_lps_in_pe, sizeof(struct Message));
  // note that g_tw_nlp gets set here by tw_define_lps

  // IF there are multiple LP types
  //    you should define the mapping of GID -> lptype index
  g_tw_lp_typemap = &model_typemap;

  // set the global variable and initialize each LP's type
  g_tw_lp_types = model_lps;
  tw_lp_setup_types();

  // Do some file I/O here? on a per-node (not per-LP) basis

  tw_run();

  tw_end();

  return 0;
}
