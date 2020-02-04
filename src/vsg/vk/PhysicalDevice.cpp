/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/PhysicalDevice.h>

#include <iostream>

using namespace vsg;

PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice device, int graphicsFamily, int presentFamily, int computeFamily) :
    _device(device),
    _graphicsFamily(graphicsFamily),
    _presentFamily(presentFamily),
    _computeFamily(computeFamily),
    _instance(instance)
{
    vkGetPhysicalDeviceProperties(_device, &_properties);

    // get ray tracing properies
    _rayTracingProperties = getProperties<VkPhysicalDeviceRayTracingPropertiesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV>();

#if 0
    std::cout << "shaderGroupHandleSize " << _rayTracingProperties.shaderGroupHandleSize << std::endl;
    std::cout << "maxRecursionDepth " << _rayTracingProperties.maxRecursionDepth << std::endl;
    std::cout << "maxShaderGroupStride " << _rayTracingProperties.maxShaderGroupStride << std::endl;
    std::cout << "shaderGroupBaseAlignment " << _rayTracingProperties.shaderGroupBaseAlignment << std::endl;
    std::cout << "maxGeometryCount " << _rayTracingProperties.maxGeometryCount << std::endl;
    std::cout << "maxInstanceCount " << _rayTracingProperties.maxInstanceCount << std::endl;
    std::cout << "maxTriangleCount " << _rayTracingProperties.maxTriangleCount << std::endl;
    std::cout << "maxDescriptorSetAccelerationStructures " << _rayTracingProperties.maxDescriptorSetAccelerationStructures << std::endl;
#endif
}

PhysicalDevice::~PhysicalDevice()
{
}

PhysicalDevice::Result PhysicalDevice::create(Instance* instance, VkQueueFlags queueFlags, Surface* surface)
{
    if (!instance)
    {
        return Result("Error: vsg::PhysicalDevice::create(...) failed to create physical device, undefined Instance.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        // Checked the DeviceQueueFamilyProperties for support for graphics
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

        int graphicsFamily = -1;
        int presentFamily = -1;
        int computeFamily = -1;

        VkQueueFlags matchedQueues = 0;

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const auto& queueFamily = queueFamiles[i];
#if 1
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                graphicsFamily = i;
                matchedQueues |= VK_QUEUE_GRAPHICS_BIT;
            }

            if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
            {
                computeFamily = i;
                matchedQueues |= VK_QUEUE_COMPUTE_BIT;
            }
#else
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) //RAYTRACING HACK
            {
                graphicsFamily = i;
                computeFamily = i;
                matchedQueues |= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
            }
#endif
            if (surface)
            {
                VkBool32 presentSupported = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupported);
                if (queueFamily.queueCount > 0 && presentSupported)
                {
                    presentFamily = i;
                }
            }
        }

        if (((matchedQueues & queueFlags) == queueFlags) && (surface == nullptr || presentFamily >= 0))
        {
            return Result(new PhysicalDevice(instance, device, graphicsFamily, presentFamily, computeFamily));
        }
    }

    return Result("Error: vsg::Device::create(...) failed to create physical device.", VK_INCOMPLETE);
}
