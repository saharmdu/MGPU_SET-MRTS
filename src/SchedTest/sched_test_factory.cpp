// Copyright [2017] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <lp_rta_gfp_fmlp.h>
#include <lp_rta_gfp_pip.h>
#include <lp_rta_pfp_dpcp.h>
#include <lp_rta_pfp_mpcp.h>
#include <nc_lp_ee_vpr.h>
#include <rta_gdc_native.h>
#include <rta_gfp_bc.h>
#include <rta_gfp_native.h>
#include <rta_gfp_ng.h>
#include <rta_pfp_ff.h>
#include <rta_pfp_gs.h>
#include <rta_pfp_ro_np.h>
#include <rta_pfp_rop.h>
#include <rta_pfp_wf.h>
#include <rta_pfp_wf_spinlock.h>
#include <sched_test_factory.h>

SchedTestBase* SchedTestFactory::createSchedTest(string test_name,
                                                 TaskSet tasks,
                                                 ProcessorSet processors,
                                                 ResourceSet resources) {
  if (0 == strcmp(test_name.data(), "RTA-GFP-native")) {
    tasks.RM_Order();
    return new RTA_GFP_native(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-GFP-BC")) {
    tasks.RM_Order();
    return new RTA_GFP_BC(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-GFP-NG")) {
    tasks.RM_Order();
    return new RTA_GFP_NG(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-PFP-WF")) {
    tasks.RM_Order();
    return new RTA_PFP_WF(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-PFP-FF")) {
    tasks.RM_Order();
    return new RTA_PFP_FF(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-PFP-WF-spinlock")) {
    tasks.RM_Order();
    return new RTA_PFP_WF_spinlock(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "LP-RTA-GFP-PIP")) {
    return new LP_RTA_GFP_PIP(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "LP-RTA-GFP-FMLP")) {
    return new LP_RTA_GFP_FMLP(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "LP-RTA-PFP-DPCP")) {
    return new LP_RTA_PFP_DPCP(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "LP-RTA-PFP-MPCP")) {
    return new LP_RTA_PFP_MPCP(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-PFP-ROP")) {
    tasks.RM_Order();
    return new RTA_PFP_ROP(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "RTA-PFP-GS")) {
    return new RTA_PFP_GS(tasks, processors, resources);
  } else if (0 == strcmp(test_name.data(), "NC-LP-EE-VPR")) {
    return new NC_LP_EE_VPR(tasks, processors, resources);
  } else {
    return NULL;
  }
}
