/*
BSD 3-Clause License

Copyright (c) 2020, CINECA
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

HIDDEN int node_has_gres(node_info_t *node)
{
  if(node == NULL || node->gres == NULL || strcasestr(node->gres, "nvgpufreq") == NULL)
    return FALSE;
  else
    return TRUE;
}

HIDDEN int job_req_gres(job_info_t *job)
{
  if(job == NULL || job->tres_per_node == NULL || strcasestr(job->tres_per_node, "nvgpufreq") == NULL)
    return FALSE;
  else
    return TRUE;
}

HIDDEN int is_job_exclusive(job_info_t *job)
{
  if(job == NULL || job->shared)
    return FALSE;
  else
    return TRUE;
}

HIDDEN int is_node_consistent()
{
  if(access(PLUGIN_RUN_CONF, F_OK) != -1)
    return FALSE;
  else
    return TRUE;
}
