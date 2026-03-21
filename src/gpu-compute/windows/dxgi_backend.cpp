#if defined(_WIN64) || defined(_WIN32)

#include "gpu_compute.hpp"

#include <dxgi.h>
#include <wrl/client.h>

namespace gpu_compute {
namespace {

GpuVendor map_vendor_id(unsigned int vendor_id)
{
    switch (vendor_id)
    {
        case 0x10DE:
            return GpuVendor::Nvidia;
        case 0x1002:
            return GpuVendor::Amd;
        case 0x8086:
            return GpuVendor::Intel;
        default:
            return GpuVendor::Unknown;
    }
}

} // namespace

GpuVendor detect_vendor()
{
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(factory.GetAddressOf()))))
    {
        return GpuVendor::Unknown;
    }

    for (unsigned int i = 0;; ++i)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(i, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        DXGI_ADAPTER_DESC1 desc{};
        if (FAILED(adapter->GetDesc1(&desc)))
        {
            continue;
        }

        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        {
            continue;
        }

        return map_vendor_id(desc.VendorId);
    }

    return GpuVendor::Unknown;
}

bool init()
{
    return detect_vendor() != GpuVendor::Unknown;
}

bool add_arrays(const float* in_a, const float* in_b, float* out, std::size_t n)
{
    if (!in_a || !in_b || !out)
    {
        return false;
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        out[i] = in_a[i] + in_b[i];
    }

    return true;
}

void shutdown() {}

} // namespace gpu_compute

#endif
