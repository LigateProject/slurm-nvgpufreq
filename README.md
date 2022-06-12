# NVGPUFREQ Slurm Plugin
This plugin unrestrict the Nvidia application clock command when an _exclusive job_ run on a node tagged with gres _nvgpufreq_.

The nvgpufreq plugin call the _nvmlDeviceSetAPIRestriction()_ to restrict/unrestrict the GPU frequency clock at user level. When the application clock commands have been unrestricted a standard users can chenge the GPU frequency using the nvidia-smi tool or the NVML APIs.

## Plugin checks
The nvgpufreq plugin intercepts the prolog and epilog of each job submitted in the cluster (_slurm_spank_job_prolog()_ and _slurm_spank_job_epilog()_).

### Prolog
In the prolog procedure, the plugin does the following checks:

1. Retrieve the node info from slurmctld. If the plugin cannot contact the slurmctld the plugin terminates its execution.
2. Check if the node is tagged with the gres _nvgpufreq_. If the node is not tagged the plugin terminates its execution.
3. Retrieve the job info from slurmctld. If the plugin cannot contact the slurmctld the plugin terminates its execution.
4. Check if the job requests the nvgpufreq gres. If the job does not specify the gres _nvgpufreq_ the plugin terminates its execution.
5. Check if the job run exclusive on the node. If the node can be shared among multiple jobs the plugin terminates its execution.
6. The plugin call the _nvmlDeviceSetAPIRestriction()_ to unrestrict the GPU frequency clock for regular users.

### Epilog
In the epilog procedure, the plugin does the following checks:

1. Retrieve the node info from slurmctld. If the plugin cannot contact the slurmctld the plugin terminates its execution.
2. Check if the node is tagged with the gres _nvgpufreq_. If the node is not tagged the plugin terminates its execution.
3. Check if the node has been configured from a nvgpufreq job and restore it. After that, the plugin deletes _/var/run/nvgpufreq.run_ and concludes the epilog procedure.

## Evaluation
To evaluate if the plugin concludes with the configuration of the node, the users/administrators can check the existence of the file _/var/run/nvgpufreq.run_, which contains the information if something when wrong or the plugin correctly terminated. This file should always be removed from the plugin in the epilog procedure after the restoration of the node.

## Logs
The plugin implements three types of logs:
* _[SLURM-NVGPUFREQ]_: for general information.
* _[SLURM-NVGPUFREQ][WARN]_: for warning information. This includes misconfigurations that do not affect the execution of the plugin.
* _[SLURM-NVGPUFREQ][ERR]_: for error information. This includes problems that terminate the execution of the plugin.

## Getting started
### Compiling
To compile the code:
1. Clone this repo to a node where is deployed SLURM daemon
    ```bash
    git clone https://gitlab.hpc.cineca.it/dcesari1/slurm-nvgpufreq.git
    ```
2. Create a build directory
    ```bash
    mkdir build-nvgpufreq
    ```
3. Enter in the build directory
    ```bash
    cd build-nvgpufreq
    ```
4. Run CMAKE and specify an install directory
    ```bash
    cmake -DCMAKE_INSTALL_PREFIX=../install-nvgpufreq ../slurm-nvgpufreq
    ```
5. Run makefile to start the compilation and install the plugin
    ```bash
    make && make install
    ```
### Configurations
#### gres.conf
Before to deploy the plugin must be defined a gres called _nvgpufreq_. The gres allows the system administrators to identify only a subset of the nodes where the plugin can be used from the users.
```
NodeName=... Name=nvgpufreq Count=1
```
#### slurm.conf
Add the gres configurations to the _slurm.conf_:
```
GresTypes=nvgpufreq
PlugStackConfig=/run/slurm/conf/plugstack.conf
NodeName=... Gres=nvgpufreq:1 ...
```
#### plugstack.conf
Add the plugin configuration to the _plugstack.conf_:
```
optional   /path/to/nvgpufreq.so
```
### Run
When a user wants to use the plugin must submit a job specify the _nvgpufreq_ gres and the exclusivity of the job.
```
sbatch $SLURM_CONF --gres=nvgpufreq --exclusive $BIN
```
### SLURM Bugs
For SLURM version between 20.0 and 20.02.7 see the following: https://bugs.schedmd.com/show_bug.cgi?id=9081
