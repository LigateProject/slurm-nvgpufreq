/*
BSD 3-Clause License

Copyright (c) 2022, CINECA
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Daniele Cesarini, CINECA
*/

#include "nvgpufreq.h"

SPANK_PLUGIN(nvgpufreq, 1);

// Callback of the job prolog. 
int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv)
{
  char hostname[BUFF_SIZE];

  // Plugin loaded
  slurm_info("[SLURM-NVGPUFREQ] The spank nvgpufreq plugin has been loaded!");

  // Retrive node info
  gethostname(hostname, sizeof(hostname));
  node_info_msg_t *node_msg;
  if(slurm_load_node_single(&node_msg, hostname, 0) < 0){
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to retrive information of node '%s' from slurmctld! Exit!", hostname);
    return ERROR_RET;
  }
  
  // Check if the node is tagged with the nvgpufreq gres
  node_info_t *node = &node_msg->node_array[0];
  if(!node_has_gres(node)){
    slurm_info("[SLURM-NVGPUFREQ] The node is not tagged with the nvgpufreq gres, exit!");
    slurm_free_node_info_msg(node_msg);
    return OK_RET;
  }

  // Retrive job info
  int job_id;
  if(spank_get_item(spank_ctx, S_JOB_ID, &job_id) != ESPANK_SUCCESS){
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to retrive the job ID from slurmctld! Exit!");
    return ERROR_RET;
  }
  job_info_msg_t *job_msg;
  if(slurm_load_job(&job_msg, job_id, 0) < 0){
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to load the information of job %d from slurmctld! Exit!", job_id);
    return ERROR_RET;
  }

  // Check if the job requests the nvgpufreq gres
  job_info_t *job = &job_msg->job_array[0];
  if(!job_req_gres(job)){
    slurm_info("[SLURM-NVGPUFREQ] The job does not request the nvgpufreq gres, exit!");
    slurm_free_job_info_msg(job_msg);
    return OK_RET;
  }
  else
    slurm_info("[SLURM-NVGPUFREQ] The job %d requests the nvgpufreq gres, try to unrestricted the applications clocks commands!", job_id);

  // Set the node
  int ret_conf_nvgpufreq = conf_nvgpufreq(spank_ctx, argc, argv, SET);
  if(ret_conf_nvgpufreq == ERROR_RET){
    slurm_info("[SLURM-NVGPUFREQ][ERR] It is not possible to unrestricted the applications clocks commands! Exit!");
    return ERROR_RET;
  }
  else if(ret_conf_nvgpufreq == WARNING_RET){
    slurm_info("[SLURM-NVGPUFREQ][WARN] The applications clocks commands have been partially unrestricted!");
    return WARNING_RET;
  }
  else{
    return OK_RET;
  }
}

// Callback of the job epilog. Called at the same time as the job epilog.
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv)
{
  char hostname[BUFF_SIZE];

  // Retrive node info
  gethostname(hostname, sizeof(hostname)); 
  node_info_msg_t *node_msg;
  if(slurm_load_node_single(&node_msg, hostname, 0) < 0){
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to retrive information of node '%s' from slurmctld! Exit!", hostname);
    return ERROR_RET;
  }

  // Check if the node is tagged with the nvgpufreq gres
  node_info_t *node = &node_msg->node_array[0];
  if(!node_has_gres(node)){
    slurm_info("[SLURM-NVGPUFREQ] The node is not tagged with the nvgpufreq gres, exit!");
    slurm_free_node_info_msg(node_msg);
    return OK_RET;
  }
  slurm_free_node_info_msg(node_msg);

  // Reset the node
  int ret_conf_nvgpufreq = conf_nvgpufreq(spank_ctx, argc, argv, RESET);
  if(ret_conf_nvgpufreq == ERROR_RET){
    slurm_info("[SLURM-NVGPUFREQ][ERR] It is not possible to restricted the applications clocks commands! Exit!");
    return ERROR_RET;
  }
  else if(ret_conf_nvgpufreq == WARNING_RET)
    slurm_info("[SLURM-NVGPUFREQ][WARN] The applications clocks commands have been partially restricted!");

  return OK_RET;
}
