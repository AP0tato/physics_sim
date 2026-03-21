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

// Initializes the Metal device, command queue, and compute pipeline.
bool init();

// Runs the add_arrays compute kernel for n elements.
// Returns false if GPU resources are unavailable or dispatch fails.
bool add_arrays(const float* in_a, const float* in_b, float* out, std::size_t n);

// Releases all Metal resources owned by the bridge.
void shutdown();

} // namespace gpu_compute

#endif // GPU_COMPUTE_HPP
