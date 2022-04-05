################################################################################
#
# Copyright 1993-2019 NVIDIA Corporation.  All rights reserved.
#
# NOTICE TO USER:   
#
# This source code is subject to NVIDIA ownership rights under U.S. and 
# international Copyright laws.  
#
# NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
# CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
# IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
# REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.   
# IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
# OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
# OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE 
# OR PERFORMANCE OF THIS SOURCE CODE.  
#
# U.S. Government End Users.  This source code is a "commercial item" as 
# that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of 
# "commercial computer software" and "commercial computer software 
# documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995) 
# and is provided to the U.S. Government only as a commercial end item.  
# Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
# 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
# source code with only those rights set forth herein.
#
################################################################################
#
# Build script for project
#
################################################################################
# Altera OCL Flags and Install Directories
#NKFLGS := -lalteracl -lalterahalmmd -lalterammdpcie -lelf -lrt  -lstdc++
#INKFLGS := -lalteracl -lelf -lrt  -lstdc++
#LINKFLGS := -lalteracl -lalterahalmmd -lelf -lrt  -lstdc++
AOCL_COMPILE_CONFIG=$(shell aocl compile-config)
LINKFLGS=$(shell aocl link-config)
OCLINSTALL := /opt/altera16.1/hld

# INTEL and NVIDIA OCL Flags and Install Directories
LINKFLGS := -lOpenCL -lstdc++ -L/opt/altera16.1/hld/board/terasic/de5net/linux64/lib -L/opt/altera16.1/hld/host/linux64/lib -Wl,--no-as-needed -lalteracl -lacl_emulator_kernel_rt  -lalterahalmmd -lelf -lrt -ldl 
#LINKFLGS := -lOpenCL -lstdc++ -L/opt/altera18.0/hld/board/nalla_pcie/linux64/lib -L/opt/altera18.0/hld/host/linux64/lib -Wl,--no-as-needed -lalteracl -lacl_emulator_kernel_rt  -lalterahalmmd -lnalla_pcie_mmd -lelf -lrt -ldl

# Add source files here
EXECUTABLE      := sha256
# C/C++ source files (compiled with gcc / c++)
CCFILES         := sha256.cpp

################################################################################
# Rules and targets
include /usr/local/OpenCL/OpenCL_altera/common/common_opencl.mk

