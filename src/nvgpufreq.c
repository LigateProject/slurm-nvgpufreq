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

HIDDEN int conf_nvgpufreq(spank_t spank_ctx, int argc, char **argv, int conf)
{
  nvmlReturn_t result;
  unsigned int device_count, i;
  int ret = OK_RET;

  void *hl = dlopen(NVML_SHARED_OBJ, RTLD_LAZY);
  if(hl == NULL)
  {
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to open the NVML shared object: " NVML_SHARED_OBJ);
    return ERROR_RET;
  }

  nvmlReturn_t (*DLnvmlInit)(void) = dlsym(hl, "nvmlInit");
  nvmlReturn_t (*DLnvmlDeviceGetCount)(unsigned int*) = dlsym(hl, "nvmlDeviceGetCount");
  nvmlReturn_t (*DLnvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*) = dlsym(hl, "nvmlDeviceGetHandleByIndex");
  nvmlReturn_t (*DLnvmlDeviceGetName)(nvmlDevice_t, char*, unsigned int) = dlsym(hl, "nvmlDeviceGetName");
  nvmlReturn_t (*DLnvmlDeviceGetPciInfo)(nvmlDevice_t, nvmlPciInfo_t*) = dlsym(hl, "nvmlDeviceGetPciInfo");
  nvmlReturn_t (*DLnvmlDeviceResetApplicationsClocks)(nvmlDevice_t) = dlsym(hl, "nvmlDeviceResetApplicationsClocks");
  nvmlReturn_t (*DLnvmlDeviceSetAPIRestriction)(nvmlDevice_t, nvmlRestrictedAPI_t, nvmlEnableState_t) = dlsym(hl, "nvmlDeviceSetAPIRestriction");
  nvmlReturn_t (*DLnvmlShutdown)(void) = dlsym(hl, "nvmlShutdown");
  char* (*DLnvmlErrorString)(nvmlReturn_t) = dlsym(hl, "nvmlErrorString");
  if(DLnvmlInit == NULL ||
     DLnvmlDeviceGetCount == NULL ||
     DLnvmlDeviceGetHandleByIndex == NULL ||
     DLnvmlDeviceGetName == NULL ||
     DLnvmlDeviceGetPciInfo == NULL ||
     DLnvmlDeviceResetApplicationsClocks == NULL ||
     DLnvmlDeviceSetAPIRestriction == NULL ||
     DLnvmlShutdown == NULL)
  {
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to open the symbols of NVML shared object: " NVML_SHARED_OBJ);
    return ERROR_RET;
  }

  result = DLnvmlInit();
  if(NVML_SUCCESS != result)
  { 
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to initialize NVML: %s\n", DLnvmlErrorString(result));
    return ERROR_RET;
  }

  result = DLnvmlDeviceGetCount(&device_count);
  if(NVML_SUCCESS != result)
  { 
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to query device count: %s\n", DLnvmlErrorString(result));
    result = DLnvmlShutdown();
    if(NVML_SUCCESS != result)
      slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to shutdown NVML: %s\n", DLnvmlErrorString(result));
    return ERROR_RET;
  }

  for(i = 0; i < device_count; i++)
  {
    nvmlDevice_t device;
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    nvmlPciInfo_t pci;
	  nvmlRestrictedAPI_t apiType = NVML_RESTRICTED_API_SET_APPLICATION_CLOCKS;

    result = DLnvmlDeviceGetHandleByIndex(i, &device);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to get handle for device %u: %s\n", i, DLnvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }

    result = DLnvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to get name of device %u: %s\n", i, DLnvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }
    
    result = DLnvmlDeviceGetPciInfo(device, &pci);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to get pci info for device %u: %s\n", i, DLnvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }

    if(conf == SET)
    {
      nvmlEnableState_t isRestricted = NVML_FEATURE_DISABLED;

      result = DLnvmlDeviceResetApplicationsClocks(device);
      if(NVML_SUCCESS != result)
      {
        slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to set API restriction for device %u: %s\n", i, DLnvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
  	
  	  result = DLnvmlDeviceSetAPIRestriction(device, apiType, isRestricted);
      if(NVML_SUCCESS != result)
      { 
        slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to set API restriction for device %u: %s\n", i, DLnvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks commands have been set to UNRESTRICTED for GPU: %u. %s [%s]\n", i, name, pci.busId);
    }
    else if(conf == RESET)
    {
      nvmlEnableState_t isRestricted = NVML_FEATURE_ENABLED;

      result = DLnvmlDeviceSetAPIRestriction(device, apiType, isRestricted);
      if(NVML_SUCCESS != result)
      { 
        slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to set API restriction for device %u: %s\n", i, DLnvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks commands have been set to RESTRICTED for GPU: %u. %s [%s]\n", i, name, pci.busId);

      result = DLnvmlDeviceResetApplicationsClocks(device);
      if(NVML_SUCCESS != result)
      { 
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks have been reset for GPU: %u. %s [%s]\n", i, name, pci.busId);
    }
  }

  result = DLnvmlShutdown();
  if(NVML_SUCCESS != result)
  {
    slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to shutdown NVML: %s\n", DLnvmlErrorString(result));
    ret = WARNING_RET;
  }

  dlclose(hl);

  if(conf == SET)
  {
    // Create the the nvgpufreq status file
    FILE *fd_run_config = fopen(PLUGIN_RUN_CONF, "w");
    if(fd_run_config == NULL){
      slurm_info("[SLURM-NVGPUFREQ][ERR] Failed to create the nvgpufreq status file: %s", PLUGIN_RUN_CONF);
      ret = WARNING_RET;
    }
    else{
      if(ret == WARNING_RET)
        fprintf(fd_run_config, "WARNING");
      else
        fprintf(fd_run_config, "COMPLETED");

      fclose(fd_run_config);
    }
  }
  else if(conf == RESET)
    remove(PLUGIN_RUN_CONF);

  return ret;
}
