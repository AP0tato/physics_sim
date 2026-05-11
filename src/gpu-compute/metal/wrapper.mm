#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstddef>
#include <limits.h>
#include <vector>
#include <mach-o/dyld.h>
#include <string>

#include "wrapper.hpp"

namespace metal {
namespace {

id<MTLDevice> g_device = nil;
id<MTLCommandQueue> g_queue = nil;
id<MTLComputePipelineState> g_pipeline = nil;

std::vector<std::string> functions;

NSString* metallib_path()
{
	uint32_t executable_path_size = PATH_MAX;
	char executable_path[PATH_MAX];
	if (_NSGetExecutablePath(executable_path, &executable_path_size) != 0)
	{
		return nil;
	}

	char resolved_executable_path[PATH_MAX];
	if (realpath(executable_path, resolved_executable_path) == nullptr)
	{
		return nil;
	}

	std::string path_string(resolved_executable_path);
	const std::size_t slash = path_string.find_last_of('/');
	if (slash == std::string::npos)
	{
		return nil;
	}

	const std::string metallib = path_string.substr(0, slash + 1) + "functions.metallib";
	return [NSString stringWithUTF8String:metallib.c_str()];
}

bool create_pipeline()
{
	NSError* error = nil;
	NSString* library_path = metallib_path();
	if (!library_path)
	{
		return false;
	}

	NSURL* library_url = [NSURL fileURLWithPath:library_path];
	id<MTLLibrary> library = [g_device newLibraryWithURL:library_url error:&error];

	if (!library || error)
	{
		return false;
	}

	id<MTLFunction> function = [library newFunctionWithName:@"add_arrays"];
	if (!function)
	{
		return false;
	}

	g_pipeline = [g_device newComputePipelineStateWithFunction:function error:&error];
	if (!g_pipeline || error)
	{
		return false;
	}

	return true;
}

} // namespace

bool init()
{
	if (g_pipeline)
	{
		return true;
	}

	g_device = MTLCreateSystemDefaultDevice();
	if (!g_device)
	{
		return false;
	}

	g_queue = [g_device newCommandQueue];
	if (!g_queue)
	{
		g_device = nil;
		return false;
	}

	if (!create_pipeline())
	{
		shutdown();
		return false;
	}

	return true;
}

bool add_arrays(const float* in_a, const float* in_b, float* out, std::size_t n)
{
	if (!in_a || !in_b || !out || n == 0)
	{
		return false;
	}

	if (!init())
	{
		return false;
	}

	const NSUInteger bytes = static_cast<NSUInteger>(n * sizeof(float));
	id<MTLBuffer> a = [g_device newBufferWithBytes:in_a length:bytes options:MTLResourceStorageModeShared];
	id<MTLBuffer> b = [g_device newBufferWithBytes:in_b length:bytes options:MTLResourceStorageModeShared];
	id<MTLBuffer> result = [g_device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
	if (!a || !b || !result)
	{
		return false;
	}

	id<MTLCommandBuffer> command_buffer = [g_queue commandBuffer];
	id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
	[encoder setComputePipelineState:g_pipeline];
	[encoder setBuffer:a offset:0 atIndex:0];
	[encoder setBuffer:b offset:0 atIndex:1];
	[encoder setBuffer:result offset:0 atIndex:2];

	const NSUInteger width = std::min<NSUInteger>(g_pipeline.maxTotalThreadsPerThreadgroup, static_cast<NSUInteger>(256));
	MTLSize threads_per_group = MTLSizeMake(width, 1, 1);
	MTLSize thread_count = MTLSizeMake(static_cast<NSUInteger>(n), 1, 1);
	[encoder dispatchThreads:thread_count threadsPerThreadgroup:threads_per_group];
	[encoder endEncoding];

	[command_buffer commit];
	[command_buffer waitUntilCompleted];
	if (command_buffer.status != MTLCommandBufferStatusCompleted)
	{
		return false;
	}

	std::copy_n(static_cast<const float*>(result.contents), n, out);
	return true;
}

void shutdown()
{
	g_pipeline = nil;
	g_queue = nil;
	g_device = nil;
}

} // namespace gpu_compute
