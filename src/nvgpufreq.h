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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <nvml.h>

#include "slurm/slurm.h"
#include "slurm/spank.h"


#ifndef __SPANK_NVGPUFREQ__
#define	__SPANK_NVGPUFREQ__

#define PLUGIN_RUN_CONF "/var/run/nvgpufreq.run"
#define NVML_SHARED_OBJ "/install/linux-centos7-sandybridge/gcc-4.8.5/cuda-11.7.0-nvmudm4bbsnap6z7gu7cespqpdtdtl7v/targets/x86_64-linux/lib/stubs/libnvidia-ml.so"

#define ERROR "<ERROR>"
#define IGNORE "<IGNORE>"
#define CPU_ID "<CPU_ID>"

#define BUFF_SIZE 1024

#define OK_RET 0
#define ERROR_RET -1
#define WARNING_RET -2

#define TRUE 1
#define FALSE 0

#define SET 1
#define RESET 0

// Hide symbols for external linking
#define HIDDEN  __attribute__((visibility("hidden")))

// main.c
int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv);

// check.c
int node_has_gres(node_info_t *job);
int job_req_gres(job_info_t *job);
int is_job_exclusive(job_info_t *job);
int is_node_consistent();

// nvgpufreq.c
int conf_nvgpufreq(spank_t spank_ctx, int argc, char **argv, int conf);

#endif // __SPANK_NVGPUFREQ__
