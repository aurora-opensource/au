// Copyright 2026 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// Make the version macros (`AU_VERSION`, etc.) available anywhere `config.hh` reaches --- which is
// effectively the entire library, since the core machinery includes this header.
#include "au/version.hh"

//
// Device/GPU support (CUDA, HIP)
//
// AU_DEVICE_FUNC: marks functions as callable from both host and device.
// AU_DEVICE_VAR: marks constexpr variables as accessible from device code.
//
// Note: AU_DEVICE_FUNC uses __CUDACC__ / __HIPCC__ (compiler detection) because functions need
// the annotation during both host and device compilation passes.
//
// AU_DEVICE_VAR uses __CUDA_ARCH__ / __HIP_DEVICE_COMPILE__ (device pass detection) because
// __device__ on a variable makes it device-only, which would break host code. By only applying
// __device__ during the device compilation pass, the same variable is visible to both host and
// device code.
//

#if defined(__CUDACC__) || defined(__HIPCC__)
#define AU_DEVICE_FUNC __host__ __device__
#else
#define AU_DEVICE_FUNC
#endif

#if defined(__CUDA_ARCH__) || defined(__HIP_DEVICE_COMPILE__)
#define AU_DEVICE_VAR __device__
#else
#define AU_DEVICE_VAR
#endif
