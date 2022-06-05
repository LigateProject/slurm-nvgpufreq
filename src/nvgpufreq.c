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

  result = nvmlInit();
  if(NVML_SUCCESS != result)
  { 
    slurm_info("[SLURM-SYSFS][ERR] Failed to initialize NVML: %s\n", nvmlErrorString(result));
    return ERROR_RET;
  }

  result = nvmlDeviceGetCount(&device_count);
  if(NVML_SUCCESS != result)
  { 
    slurm_info("[SLURM-SYSFS][ERR] Failed to query device count: %s\n", nvmlErrorString(result));
    result = nvmlShutdown();
    if(NVML_SUCCESS != result)
      slurm_info("[SLURM-SYSFS][ERR] Failed to shutdown NVML: %s\n", nvmlErrorString(result));
    return ERROR_RET;
  }

  for(i = 0; i < device_count; i++)
  {
    nvmlDevice_t device;
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    nvmlPciInfo_t pci;
	  nvmlRestrictedAPI_t apiType = NVML_RESTRICTED_API_SET_APPLICATION_CLOCKS;

    result = nvmlDeviceGetHandleByIndex(i, &device);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-SYSFS][ERR] Failed to get handle for device %u: %s\n", i, nvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }

    result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-SYSFS][ERR] Failed to get name of device %u: %s\n", i, nvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }
    
    result = nvmlDeviceGetPciInfo(device, &pci);
    if(NVML_SUCCESS != result)
    { 
      slurm_info("[SLURM-SYSFS][ERR] Failed to get pci info for device %u: %s\n", i, nvmlErrorString(result));
      ret = WARNING_RET;
      continue;
    }

    if(conf == SET)
    {
      nvmlEnableState_t isRestricted = NVML_FEATURE_DISABLED;

      result = nvmlDeviceResetApplicationsClocks(device);
      if(NVML_SUCCESS != result)
      {
        slurm_info("[SLURM-SYSFS][ERR] Failed to set API restriction for device %u: %s\n", i, nvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
  	
  	  result = nvmlDeviceSetAPIRestriction(device, apiType, isRestricted);
      if(NVML_SUCCESS != result)
      { 
        slurm_info("[SLURM-SYSFS][ERR] Failed to set API restriction for device %u: %s\n", i, nvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks commands have been set to UNRESTRICTED for GPU: %u. %s [%s]\n", i, name, pci.busId);
    }
    else if(conf == RESET)
    {
      nvmlEnableState_t isRestricted = NVML_FEATURE_ENABLED;

      result = nvmlDeviceSetAPIRestriction(device, apiType, isRestricted);
      if(NVML_SUCCESS != result)
      { 
        slurm_info("[SLURM-SYSFS][ERR] Failed to set API restriction for device %u: %s\n", i, nvmlErrorString(result));
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks commands have been set to RESTRICTED for GPU: %u. %s [%s]\n", i, name, pci.busId);

      result = nvmlDeviceResetApplicationsClocks(device);
      if(NVML_SUCCESS != result)
      { 
        ret = WARNING_RET;
        continue;
      }
      slurm_info("[SLURM-NVGPUFREQ] Applications clocks have been reset for GPU: %u. %s [%s]\n", i, name, pci.busId);
    }
  }

  result = nvmlShutdown();
  if(NVML_SUCCESS != result)
  {
    slurm_info("[SLURM-SYSFS][ERR] Failed to shutdown NVML: %s\n", nvmlErrorString(result));
    return WARNING_RET;
  }
  else
    return ret;
}
