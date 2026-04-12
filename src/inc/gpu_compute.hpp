#ifndef GPU_COMPUTE_HPP

#define GPU_COMPUTE_HPP

#include <cstddef>

enum class GpuVendor {
    Unknown,
    Nvidia,
    Amd,
    Intel,
    Apple
};

namespace gpu_compute {

// Detects the active/default GPU vendor for runtime backend decisions.
GpuVendor detect_vendor();

// Initializes the gpu device, command queue, and compute pipeline.
bool init();

// Releases all gpu resources owned by the bridge.
void shutdown();

} // namespace gpu_compute

#endif // GPU_COMPUTE_HPP
