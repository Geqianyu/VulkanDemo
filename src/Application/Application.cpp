#include "Application.h"

bool QueueFamilyIndices::isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

#ifndef NDEBUG
    const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
#endif

const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

const int MAX_FRAMES_IN_FLIGHT = 2;

Application::Application(const int _width, const int _height, const std::string& _name)
{
    std::cout << setFontColor("Application is created", FontColor::Green) << std::endl;
    initWindow(_width, _height, _name);
}

Application::~Application()
{
    std::cout << setFontColor("Application is released", FontColor::Indigo) << std::endl;
}

void Application::run()
{
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::initWindow(const int _width, const int _height, const std::string& _name)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(_width, _height, _name.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
    {
        std::runtime_error(setFontColor("Failed to create a window", FontColor::Red));
    }
    std::cout << setFontColor("Success to create a window", FontColor::Green) << std::endl;

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

void Application::initVulkan()
{
    createInstance();

    #ifndef NDEBUG
        setupDebugMessenger();
    #endif

    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createVertexIndicesBuffer();
    createCommandBuffers();
    createSyncObjects();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(m_device);
}

void Application::cleanup()
{
    cleanupSwapchain();

    vkDestroyBuffer(m_device, m_vertexIndicesBuffer, nullptr);
    std::cout << setFontColor("Destroy the vertex indices buffer", FontColor::Indigo) << std::endl;

    vkFreeMemory(m_device, m_vertexIndicesBufferMemory, nullptr);
    std::cout << setFontColor("Destroy the vertex indices buffer memory", FontColor::Indigo) << std::endl;

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    std::cout << setFontColor("Destroy the vertex buffer", FontColor::Indigo) << std::endl;

    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
    std::cout << setFontColor("Destroy the vertex buffer memory", FontColor::Indigo) << std::endl;

    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    std::cout << setFontColor("Destroy the graphics pipeline", FontColor::Indigo) << std::endl;

    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    std::cout << setFontColor("Destroy the pipeline layout", FontColor::Indigo) << std::endl;

    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    std::cout << setFontColor("Destroy the render pass", FontColor::Indigo) << std::endl;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_flightFences[i], nullptr);
        std::cout << setFontColor("Destroy synchronization objects " + std::to_string(i), FontColor::Indigo) << std::endl;
    }

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    std::cout << setFontColor("Destroy the command pool", FontColor::Indigo) << std::endl;

    vkDestroyDevice(m_device, nullptr);
    std::cout << setFontColor("Destroy the logical device", FontColor::Indigo) << std::endl;

    #ifndef NDEBUG
        destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        std::cout << setFontColor("Destroy the debug messenger", FontColor::Indigo) << std::endl;
    #endif

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    std::cout << setFontColor("Destroy the surface", FontColor::Indigo) << std::endl;

    vkDestroyInstance(m_instance, nullptr);
    std::cout << setFontColor("Destroy the vulkan instance", FontColor::Indigo) << std::endl;

    glfwDestroyWindow(m_window);
    std::cout << setFontColor("Destroy the window", FontColor::Indigo) << std::endl;

    glfwTerminate();
    std::cout << setFontColor("Terminate glfw", FontColor::Indigo) << std::endl;
}

void Application::createInstance()
{
    #ifndef NDEBUG
        if (!checkValidationLayerProperties())
        {
            throw std::runtime_error(setFontColor("Validation layers requested, but not available", FontColor::Red));
        }
    #endif

    VkApplicationInfo appInfo
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,     // sType
        nullptr,                                // pNext
        "Vulkan Demo",                          // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),               // applicationVersion
        "No Engine",                            // pEngineName
        VK_MAKE_VERSION(1, 0, 0),               // engineVersion
        VK_API_VERSION_1_0                      // apiVersion
    };

    std::vector<const char*>&& extensions = getRequiredExtensions();
    #ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();
        VkInstanceCreateInfo createInfo
        {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,                                              // sType
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCreateInfo,                 // pNext
            VK_FALSE,                                                                            // flags
            &appInfo,                                                                            // pApplicationInfo
            static_cast<uint32_t>(validationLayers.size()),                                      // enabledLayerCount
            validationLayers.data(),                                                             // ppEnabledLayerNames
            static_cast<uint32_t>(extensions.size()),                                            // enabledExtensionCount
            extensions.data()                                                                    // ppEnabledExtensionNames
        };
    #else
        VkInstanceCreateInfo createInfo
        {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // sType
            nullptr,                                        // pNext
            VK_FALSE,                                       // flags
            &appInfo,                                       // pApplicationInfo
            0,                                              // enabledLayerCount
            nullptr,                                        // ppEnabledLayerNames
            static_cast<uint32_t>(extensions.size()),       // enabledExtensionCount
            extensions.data()                               // ppEnabledExtensionNames
        };
    #endif

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create vulkan instance", FontColor::Red));
    }
    else
    {
        std::cout << setFontColor("Success to create vulkan instance", FontColor::Green) << std::endl;

        uint32_t instanceExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
        std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data());
        std::cout << "\tAvailable extensions:\n";
        for (const VkExtensionProperties& extension : instanceExtensionProperties)
        {
            std::cout << "\t\t" << extension.extensionName << "\n";
        }
    }
}

#ifndef NDEBUG
bool Application::checkValidationLayerProperties()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFount = false;
        for (const auto& layer : availableLayers)
        {
            if (strcmp(layerName, layer.layerName) == 0)
            {
                layerFount = true;
                break;
            }
        }

        if (!layerFount)
        {
            return false;
        }
    }

    return true;
}

void Application::setupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = getDebugUtilsMessengerCreateInfo();
    if (createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to set up debug messenger", FontColor::Red));
    }
    std::cout << setFontColor("Success to set up debug messenger", FontColor::Green) << std::endl;
}

VkResult Application::createDebugUtilsMessengerEXT(VkInstance _instance, const VkDebugUtilsMessengerCreateInfoEXT* _pCreateInfo, const VkAllocationCallbacks* _pAllocator, VkDebugUtilsMessengerEXT* _pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(_instance, _pCreateInfo, _pAllocator, _pDebugMessenger);
}

VkDebugUtilsMessengerCreateInfoEXT Application::getDebugUtilsMessengerCreateInfo()
{
    return VkDebugUtilsMessengerCreateInfoEXT
    {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,    // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,            // messageSeverity
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,          // messageType
        debugCallback,                                              // pfnUserCallback
        nullptr                                                     // pUserData
    };
}

void Application::destroyDebugUtilsMessengerEXT(VkInstance _instance, VkDebugUtilsMessengerEXT _debugMessenger, const VkAllocationCallbacks* _pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(_instance, _debugMessenger, _pAllocator);
    }
}
#endif

std::vector<const char*> Application::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    #ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    return extensions;
}

void Application::pickPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error(setFontColor("Failed to find GPUs whit vulkan support", FontColor::Red));
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
        if (isSuitableDevice(physicalDevice))
        {
            m_physicalDevice = physicalDevice;
            break;
        }
    }

    if (m_physicalDevice == nullptr)
    {
        throw std::runtime_error(setFontColor("Failed to find a suitable GPU", FontColor::Red));
    }
    std::cout << setFontColor("Success to find a suitable GPU", FontColor::Purple) << std::endl;
    printPhysicalDeviceFeature(m_physicalDevice);
    printPhysicalDeviceProperties(m_physicalDevice);
}

bool Application::isSuitableDevice(const VkPhysicalDevice _physicalDevice)
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    bool extensionsSupport = checkDeviceExtensionSupport(_physicalDevice);

    bool swapchainAdequate = false;
    if (extensionsSupport)
    {
        SwapChainSupportDetails swapchainSupportDetails = querySwapchainSupport(_physicalDevice);
        swapchainAdequate = !swapchainSupportDetails.formats.empty() && !swapchainSupportDetails.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupport && swapchainAdequate;
}

long long int Application::rateDeviceSuitability(const VkPhysicalDevice _physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(_physicalDevice, &physicalDeviceFeatures);

    if (!physicalDeviceFeatures.geometryShader)
    {
        return 0;
    }

    long long int score = 0;
    if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }
    score += physicalDeviceProperties.limits.maxImageDimension2D;

    return score;
}

void Application::printPhysicalDeviceFeature(const VkPhysicalDevice _physicalDevice)
{
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(_physicalDevice, &physicalDeviceFeatures);
    std::cout
        << "\tPhysical Device Features:\n"
        << "\t\trobust buffer access: " << std::to_string(physicalDeviceFeatures.robustBufferAccess) << "\n"
        << "\t\tfull draw index uint32: " << std::to_string(physicalDeviceFeatures.fullDrawIndexUint32) << "\n"
        << "\t\timage cube array: " << std::to_string(physicalDeviceFeatures.imageCubeArray) << "\n"
        << "\t\tindependent blend: " << std::to_string(physicalDeviceFeatures.independentBlend) << "\n"
        << "\t\tgeometry shader: " << std::to_string(physicalDeviceFeatures.geometryShader) << "\n"
        << "\t\ttessellation shader: " << std::to_string(physicalDeviceFeatures.tessellationShader) << "\n"
        << "\t\tsample rate shading: " << std::to_string(physicalDeviceFeatures.sampleRateShading) << "\n"
        << "\t\tdual src blend: " << std::to_string(physicalDeviceFeatures.dualSrcBlend) << "\n"
        << "\t\tlogic op: " << std::to_string(physicalDeviceFeatures.logicOp) << "\n"
        << "\t\tmulti draw indirect: " << std::to_string(physicalDeviceFeatures.multiDrawIndirect) << "\n"
        << "\t\tdraw indirect first instance: " << std::to_string(physicalDeviceFeatures.drawIndirectFirstInstance) << "\n"
        << "\t\tdepth clamp: " << std::to_string(physicalDeviceFeatures.depthClamp) << "\n"
        << "\t\tdepth bias clamp: " << std::to_string(physicalDeviceFeatures.depthBiasClamp) << "\n"
        << "\t\tfill mode non solid: " << std::to_string(physicalDeviceFeatures.fillModeNonSolid) << "\n"
        << "\t\tdepth bounds: " << std::to_string(physicalDeviceFeatures.depthBounds) << "\n"
        << "\t\twide lines: " << std::to_string(physicalDeviceFeatures.wideLines) << "\n"
        << "\t\tlarge points: " << std::to_string(physicalDeviceFeatures.largePoints) << "\n"
        << "\t\talpha to one: " << std::to_string(physicalDeviceFeatures.alphaToOne) << "\n"
        << "\t\tmulti viewport: " << std::to_string(physicalDeviceFeatures.multiViewport) << "\n"
        << "\t\tsampler anisotropy: " << std::to_string(physicalDeviceFeatures.samplerAnisotropy) << "\n"
        << "\t\ttexture compression ETC2: " << std::to_string(physicalDeviceFeatures.textureCompressionETC2) << "\n"
        << "\t\ttexture compression ASTC_LDR: " << std::to_string(physicalDeviceFeatures.textureCompressionASTC_LDR) << "\n"
        << "\t\ttexture compression BC: " << std::to_string(physicalDeviceFeatures.textureCompressionBC) << "\n"
        << "\t\tocclusion query precise: " << std::to_string(physicalDeviceFeatures.occlusionQueryPrecise) << "\n"
        << "\t\tpipeline statistics query: " << std::to_string(physicalDeviceFeatures.pipelineStatisticsQuery) << "\n"
        << "\t\tvertex pipeline stores and atomics: " << std::to_string(physicalDeviceFeatures.vertexPipelineStoresAndAtomics) << "\n"
        << "\t\tfragment stores and atomics: " << std::to_string(physicalDeviceFeatures.fragmentStoresAndAtomics) << "\n"
        << "\t\tshader tessellation and geometry point size: " << std::to_string(physicalDeviceFeatures.shaderTessellationAndGeometryPointSize) << "\n"
        << "\t\tshader image gather extended: " << std::to_string(physicalDeviceFeatures.shaderImageGatherExtended) << "\n"
        << "\t\tshader storage image extended formats: " << std::to_string(physicalDeviceFeatures.shaderStorageImageExtendedFormats) << "\n"
        << "\t\tshader storage image multisample: " << std::to_string(physicalDeviceFeatures.shaderStorageImageMultisample) << "\n"
        << "\t\tshader storage image read without format: " << std::to_string(physicalDeviceFeatures.shaderStorageImageReadWithoutFormat) << "\n"
        << "\t\tshader storage image write without format: " << std::to_string(physicalDeviceFeatures.shaderStorageImageWriteWithoutFormat) << "\n"
        << "\t\tshader uniform buffer array dynamic indexing: " << std::to_string(physicalDeviceFeatures.shaderUniformBufferArrayDynamicIndexing) << "\n"
        << "\t\tshader sampled image array dynamic indexing: " << std::to_string(physicalDeviceFeatures.shaderSampledImageArrayDynamicIndexing) << "\n"
        << "\t\tshader storage buffer array dynamic indexing: " << std::to_string(physicalDeviceFeatures.shaderStorageBufferArrayDynamicIndexing) << "\n"
        << "\t\tshader storage image array dynamic indexing: " << std::to_string(physicalDeviceFeatures.shaderStorageImageArrayDynamicIndexing) << "\n"
        << "\t\tshader clip distance: " << std::to_string(physicalDeviceFeatures.shaderClipDistance) << "\n"
        << "\t\tshader cull distance: " << std::to_string(physicalDeviceFeatures.shaderCullDistance) << "\n"
        << "\t\tshader float64: " << std::to_string(physicalDeviceFeatures.shaderFloat64) << "\n"
        << "\t\tshader int64: " << std::to_string(physicalDeviceFeatures.shaderInt64) << "\n"
        << "\t\tshader int16: " << std::to_string(physicalDeviceFeatures.shaderInt16) << "\n"
        << "\t\tshader resource residency: " << std::to_string(physicalDeviceFeatures.shaderResourceResidency) << "\n"
        << "\t\tshader resource min lod: " << std::to_string(physicalDeviceFeatures.shaderResourceMinLod) << "\n"
        << "\t\tsparse binding: " << std::to_string(physicalDeviceFeatures.sparseBinding) << "\n"
        << "\t\tsparse residency buffer: " << std::to_string(physicalDeviceFeatures.sparseResidencyBuffer) << "\n"
        << "\t\tsparse residency image2D: " << std::to_string(physicalDeviceFeatures.sparseResidencyImage2D) << "\n"
        << "\t\tsparse residency image3D: " << std::to_string(physicalDeviceFeatures.sparseResidencyImage3D) << "\n"
        << "\t\tsparse residency 2 samples: " << std::to_string(physicalDeviceFeatures.sparseResidency2Samples) << "\n"
        << "\t\tsparse residency 4 samples: " << std::to_string(physicalDeviceFeatures.sparseResidency4Samples) << "\n"
        << "\t\tsparse residency 8 samples: " << std::to_string(physicalDeviceFeatures.sparseResidency8Samples) << "\n"
        << "\t\tsparse residency 16 samples: " << std::to_string(physicalDeviceFeatures.sparseResidency16Samples) << "\n"
        << "\t\tsparse residency aliased: " << std::to_string(physicalDeviceFeatures.sparseResidencyAliased) << "\n"
        << "\t\tvariable multisample rate: " << std::to_string(physicalDeviceFeatures.variableMultisampleRate) << "\n"
        << "\t\tinherited queries: " << std::to_string(physicalDeviceFeatures.inheritedQueries) << std::endl;
}

void Application::printPhysicalDeviceProperties(const VkPhysicalDevice _physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

    std::cout
        << "\tPhysical Device Properties:\n"
        << "\t\tapi version: " << std::to_string(physicalDeviceProperties.apiVersion) << "\n"
        << "\t\tdriver version: " << std::to_string(physicalDeviceProperties.driverVersion) << "\n"
        << "\t\tvendor ID: " << std::to_string(physicalDeviceProperties.vendorID) << "\n"
        << "\t\tdevice ID: " << std::to_string(physicalDeviceProperties.deviceID) << "\n"
        << "\t\tdevice type: ";
    switch (physicalDeviceProperties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        std::cout << "other" << "\n";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        std::cout << "integrated GPU" << "\n";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        std::cout << "discrete GPU" << "\n";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        std::cout << "virtual GPU" << "\n";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        std::cout << "CPU" << "\n";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        std::cout << "unknow" << "\n";
        break;
    default:
        std::cout << "other" << "\n";
        break;
    }
    std::cout
        << "\t\tdevice name: " << std::string(physicalDeviceProperties.deviceName, physicalDeviceProperties.deviceName + VK_MAX_PHYSICAL_DEVICE_NAME_SIZE) << "\n"
        // << "\t\tpipeline cache UUID: " << std::string((char*)(physicalDeviceProperties.pipelineCacheUUID)) << "\n"
        << "\t\tlimits:\n"
        << "\t\t\tmax image dimension 1D: " << std::to_string(physicalDeviceProperties.limits.maxImageDimension1D) << "\n"
        << "\t\t\tmax image dimension 2D: " << std::to_string(physicalDeviceProperties.limits.maxImageDimension2D) << "\n"
        << "\t\t\tmax image dimension 3D: " << std::to_string(physicalDeviceProperties.limits.maxImageDimension3D) << "\n"
        << "\t\t\tmax image dimension cube: " << std::to_string(physicalDeviceProperties.limits.maxImageDimensionCube) << "\n"
        << "\t\t\tmax image array layers: " << std::to_string(physicalDeviceProperties.limits.maxImageArrayLayers) << "\n"
        << "\t\t\tmax texel buffer elements: " << std::to_string(physicalDeviceProperties.limits.maxTexelBufferElements) << "\n"
        << "\t\t\tmax uniform buffer range: " << std::to_string(physicalDeviceProperties.limits.maxUniformBufferRange) << "\n"
        << "\t\t\tmax storage buffer range: " << std::to_string(physicalDeviceProperties.limits.maxStorageBufferRange) << "\n"
        << "\t\t\tmax push constants size: " << std::to_string(physicalDeviceProperties.limits.maxPushConstantsSize) << "\n"
        << "\t\t\tmax memory allocation count: " << std::to_string(physicalDeviceProperties.limits.maxMemoryAllocationCount) << "\n"
        << "\t\t\tmax sampler allocation count: " << std::to_string(physicalDeviceProperties.limits.maxSamplerAllocationCount) << "\n"
        << "\t\t\tbuffer image granularity: " << std::to_string(physicalDeviceProperties.limits.bufferImageGranularity) << "\n"
        << "\t\t\tsparse address space size: " << std::to_string(physicalDeviceProperties.limits.sparseAddressSpaceSize) << "\n"
        << "\t\t\tmax bound descriptor sets: " << std::to_string(physicalDeviceProperties.limits.maxBoundDescriptorSets) << "\n"
        << "\t\t\tmax per stage descriptor samplers: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorSamplers) << "\n"
        << "\t\t\tmax per stage descriptor uniform buffers: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorUniformBuffers) << "\n"
        << "\t\t\tmax per stage descriptor storage buffers: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorStorageBuffers) << "\n"
        << "\t\t\tmax per stage descriptor sampled images: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorSampledImages) << "\n"
        << "\t\t\tmax per stage descriptor storage images: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorStorageImages) << "\n"
        << "\t\t\tmax per stage descriptor input attachments: " << std::to_string(physicalDeviceProperties.limits.maxPerStageDescriptorInputAttachments) << "\n"
        << "\t\t\tmax per stage resources: " << std::to_string(physicalDeviceProperties.limits.maxPerStageResources) << "\n"
        << "\t\t\tmax descriptor set samplers: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetSamplers) << "\n"
        << "\t\t\tmax descriptor set uniform buffers: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetUniformBuffers) << "\n"
        << "\t\t\tmax descriptor set uniform buffers dynamic: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetUniformBuffersDynamic) << "\n"
        << "\t\t\tmax descriptor set storage buffers: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetStorageBuffers) << "\n"
        << "\t\t\tmax descriptor set storage buffers dynamic: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetStorageBuffersDynamic) << "\n"
        << "\t\t\tmax descriptor set sampled images: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetSampledImages) << "\n"
        << "\t\t\tmax descriptor set storage images: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetStorageImages) << "\n"
        << "\t\t\tmax descriptor set input attachments: " << std::to_string(physicalDeviceProperties.limits.maxDescriptorSetInputAttachments) << "\n"
        << "\t\t\tmax vrtex input attributes: " << std::to_string(physicalDeviceProperties.limits.maxVertexInputAttributes) << "\n"
        << "\t\t\tmax vertex input bindings: " << std::to_string(physicalDeviceProperties.limits.maxVertexInputBindings) << "\n"
        << "\t\t\tmax vertex input attribute offset: " << std::to_string(physicalDeviceProperties.limits.maxVertexInputAttributeOffset) << "\n"
        << "\t\t\tmax vertex input binding stride: " << std::to_string(physicalDeviceProperties.limits.maxVertexInputBindingStride) << "\n"
        << "\t\t\tmax vertex output components: " << std::to_string(physicalDeviceProperties.limits.maxVertexOutputComponents) << "\n"
        << "\t\t\tmax tessellation generation level: " << std::to_string(physicalDeviceProperties.limits.maxTessellationGenerationLevel) << "\n"
        << "\t\t\tmax tessellation patch size: " << std::to_string(physicalDeviceProperties.limits.maxTessellationPatchSize) << "\n"
        << "\t\t\tmax tessellation control per vertex input components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationControlPerVertexInputComponents) << "\n"
        << "\t\t\tmax tessellation control per vertex output components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationControlPerVertexOutputComponents) << "\n"
        << "\t\t\tmax tessellation cntrol per patch output components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationControlPerPatchOutputComponents) << "\n"
        << "\t\t\tmax tessellation control total output components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationControlTotalOutputComponents) << "\n"
        << "\t\t\tmax tessellation evaluation input components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationEvaluationInputComponents) << "\n"
        << "\t\t\tmax tessellation evaluation output components: " << std::to_string(physicalDeviceProperties.limits.maxTessellationEvaluationOutputComponents) << "\n"
        << "\t\t\tmax geometry shader invocations: " << std::to_string(physicalDeviceProperties.limits.maxGeometryShaderInvocations) << "\n"
        << "\t\t\tmax geometry input components: " << std::to_string(physicalDeviceProperties.limits.maxGeometryInputComponents) << "\n"
        << "\t\t\tmax geometry output components: " << std::to_string(physicalDeviceProperties.limits.maxGeometryOutputComponents) << "\n"
        << "\t\t\tmax geometry output vertices: " << std::to_string(physicalDeviceProperties.limits.maxGeometryOutputVertices) << "\n"
        << "\t\t\tmax geometry total output components: " << std::to_string(physicalDeviceProperties.limits.maxGeometryTotalOutputComponents) << "\n"
        << "\t\t\tmax fragment input components: " << std::to_string(physicalDeviceProperties.limits.maxFragmentInputComponents) << "\n"
        << "\t\t\tmax fragment output attachments: " << std::to_string(physicalDeviceProperties.limits.maxFragmentOutputAttachments) << "\n"
        << "\t\t\tmax fragment dual src attachments: " << std::to_string(physicalDeviceProperties.limits.maxFragmentDualSrcAttachments) << "\n"
        << "\t\t\tmax fragment combined output resources: " << std::to_string(physicalDeviceProperties.limits.maxFragmentCombinedOutputResources) << "\n"
        << "\t\t\tmax compute shared memory size: " << std::to_string(physicalDeviceProperties.limits.maxComputeSharedMemorySize) << "\n"
        << "\t\t\tmax compute work group count: " << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupCount[0]) << "--" << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupCount[1]) << "--" << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupCount[2]) << "\n"
        << "\t\t\tmax compute work group invocations: " << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupInvocations) << "\n"
        << "\t\t\tmax compute work group size: " << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupSize[0]) << "--" << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupSize[1]) << "--" << std::to_string(physicalDeviceProperties.limits.maxComputeWorkGroupSize[2]) << "\n"
        << "\t\t\tsub pixel precision bits: " << std::to_string(physicalDeviceProperties.limits.subPixelPrecisionBits) << "\n"
        << "\t\t\tsub texel precision bits: " << std::to_string(physicalDeviceProperties.limits.subTexelPrecisionBits) << "\n"
        << "\t\t\tmipmap precision bits: " << std::to_string(physicalDeviceProperties.limits.mipmapPrecisionBits) << "\n"
        << "\t\t\tmax draw indexed index value: " << std::to_string(physicalDeviceProperties.limits.maxDrawIndexedIndexValue) << "\n"
        << "\t\t\tmax draw indirect count: " << std::to_string(physicalDeviceProperties.limits.maxDrawIndirectCount) << "\n"
        << "\t\t\tmax sampler lod bias: " << std::to_string(physicalDeviceProperties.limits.maxSamplerLodBias) << "\n"
        << "\t\t\tmax sampler anisotropy: " << std::to_string(physicalDeviceProperties.limits.maxSamplerAnisotropy) << "\n"
        << "\t\t\tmax viewports: " << std::to_string(physicalDeviceProperties.limits.maxViewports) << "\n"
        << "\t\t\tmax viewport dmensions: " << std::to_string(physicalDeviceProperties.limits.maxViewportDimensions[0]) << "----" << std::to_string(physicalDeviceProperties.limits.maxViewportDimensions[1]) << "\n"
        << "\t\t\tviewport bounds range: " << std::to_string(physicalDeviceProperties.limits.viewportBoundsRange[0]) << "----" << std::to_string(physicalDeviceProperties.limits.viewportBoundsRange[1]) << "\n"
        << "\t\t\tviewport sub pixel bits: " << std::to_string(physicalDeviceProperties.limits.viewportSubPixelBits) << "\n"
        << "\t\t\tmin memory map alignment: " << std::to_string(physicalDeviceProperties.limits.minMemoryMapAlignment) << "\n"
        << "\t\t\tmin texel buffer offset alignment: " << std::to_string(physicalDeviceProperties.limits.minTexelBufferOffsetAlignment) << "\n"
        << "\t\t\tmin uniform buffer offset alignment: " << std::to_string(physicalDeviceProperties.limits.minUniformBufferOffsetAlignment) << "\n"
        << "\t\t\tmin storage buffer offset alignment: " << std::to_string(physicalDeviceProperties.limits.minStorageBufferOffsetAlignment) << "\n"
        << "\t\t\tmin texel offset: " << std::to_string(physicalDeviceProperties.limits.minTexelOffset) << "\n"
        << "\t\t\tmax texel offset: " << std::to_string(physicalDeviceProperties.limits.maxTexelOffset) << "\n"
        << "\t\t\tmint exel gather offset: " << std::to_string(physicalDeviceProperties.limits.minTexelGatherOffset) << "\n"
        << "\t\t\tmax texel gather offset: " << std::to_string(physicalDeviceProperties.limits.maxTexelGatherOffset) << "\n"
        << "\t\t\tmin interpolation offset: " << std::to_string(physicalDeviceProperties.limits.minInterpolationOffset) << "\n"
        << "\t\t\tmax interpolation offset: " << std::to_string(physicalDeviceProperties.limits.maxInterpolationOffset) << "\n"
        << "\t\t\tsub pixel interpolation offset bits: " << std::to_string(physicalDeviceProperties.limits.subPixelInterpolationOffsetBits) << "\n"
        << "\t\t\tmax framebuffer width: " << std::to_string(physicalDeviceProperties.limits.maxFramebufferWidth) << "\n"
        << "\t\t\tmax framebuffer height: " << std::to_string(physicalDeviceProperties.limits.maxFramebufferHeight) << "\n"
        << "\t\t\tmax framebuffer layers: " << std::to_string(physicalDeviceProperties.limits.maxFramebufferLayers) << "\n"
        << "\t\t\tframebuffer color sample counts: " << std::to_string(physicalDeviceProperties.limits.framebufferColorSampleCounts) << "\n"
        << "\t\t\tframebuffer depth sample counts: " << std::to_string(physicalDeviceProperties.limits.framebufferDepthSampleCounts) << "\n"
        << "\t\t\tframebuffer stencil sample counts: " << std::to_string(physicalDeviceProperties.limits.framebufferStencilSampleCounts) << "\n"
        << "\t\t\tframebuffer no attachments sample counts: " << std::to_string(physicalDeviceProperties.limits.framebufferNoAttachmentsSampleCounts) << "\n"
        << "\t\t\tmax color attachments: " << std::to_string(physicalDeviceProperties.limits.maxColorAttachments) << "\n"
        << "\t\t\tsampled image color sample counts: " << std::to_string(physicalDeviceProperties.limits.sampledImageColorSampleCounts) << "\n"
        << "\t\t\tsampled image integer sample counts: " << std::to_string(physicalDeviceProperties.limits.sampledImageIntegerSampleCounts) << "\n"
        << "\t\t\tsampled image depth sample counts: " << std::to_string(physicalDeviceProperties.limits.sampledImageDepthSampleCounts) << "\n"
        << "\t\t\tsampled image stencil sample counts: " << std::to_string(physicalDeviceProperties.limits.sampledImageStencilSampleCounts) << "\n"
        << "\t\t\tstorage image sample counts: " << std::to_string(physicalDeviceProperties.limits.storageImageSampleCounts) << "\n"
        << "\t\t\tmax sample mask words: " << std::to_string(physicalDeviceProperties.limits.maxSampleMaskWords) << "\n"
        << "\t\t\ttimestamp compute and graphics: " << std::to_string(physicalDeviceProperties.limits.timestampComputeAndGraphics) << "\n"
        << "\t\t\ttimestamp period: " << std::to_string(physicalDeviceProperties.limits.timestampPeriod) << "\n"
        << "\t\t\tmax clip distances: " << std::to_string(physicalDeviceProperties.limits.maxClipDistances) << "\n"
        << "\t\t\tmax cull distances: " << std::to_string(physicalDeviceProperties.limits.maxCullDistances) << "\n"
        << "\t\t\tmax combined clip and cull distances: " << std::to_string(physicalDeviceProperties.limits.maxCombinedClipAndCullDistances) << "\n"
        << "\t\t\tdiscretequeuepriorities: " << std::to_string(physicalDeviceProperties.limits.discreteQueuePriorities) << "\n"
        << "\t\t\tpoint size range: " << std::to_string(physicalDeviceProperties.limits.pointSizeRange[1]) << "----" << std::to_string(physicalDeviceProperties.limits.pointSizeRange[1]) << "\n"
        << "\t\t\tline width range: " << std::to_string(physicalDeviceProperties.limits.lineWidthRange[0]) << "----" << std::to_string(physicalDeviceProperties.limits.lineWidthRange[1]) << "\n"
        << "\t\t\tpoint size granularity: " << std::to_string(physicalDeviceProperties.limits.pointSizeGranularity) << "\n"
        << "\t\t\tline width granularity: " << std::to_string(physicalDeviceProperties.limits.lineWidthGranularity) << "\n"
        << "\t\t\tstrict lines: " << std::to_string(physicalDeviceProperties.limits.strictLines) << "\n"
        << "\t\t\tstandard sample locations: " << std::to_string(physicalDeviceProperties.limits.standardSampleLocations) << "\n"
        << "\t\t\toptimal buffer copy offset alignment: " << std::to_string(physicalDeviceProperties.limits.optimalBufferCopyOffsetAlignment) << "\n"
        << "\t\t\toptimal buffer copy row pitch alignment: " << std::to_string(physicalDeviceProperties.limits.optimalBufferCopyRowPitchAlignment) << "\n"
        << "\t\t\tnon coherent atom ize: " << std::to_string(physicalDeviceProperties.limits.nonCoherentAtomSize) << "\n"
        << "\t\tsparse properties:\n"
        << "\t\t\tresidency standard 2D block shape: " << std::to_string(physicalDeviceProperties.sparseProperties.residencyStandard2DBlockShape) << "\n"
        << "\t\t\tresidency standard 2D multisample block shape: " << std::to_string(physicalDeviceProperties.sparseProperties.residencyStandard2DMultisampleBlockShape) << "\n"
        << "\t\t\tresidency standard 3D block shape: " << std::to_string(physicalDeviceProperties.sparseProperties.residencyStandard3DBlockShape) << "\n"
        << "\t\t\tresidency aligned mip size: " << std::to_string(physicalDeviceProperties.sparseProperties.residencyAlignedMipSize) << "\n"
        << "\t\t\tresidency non resident strict: " << std::to_string(physicalDeviceProperties.sparseProperties.residencyNonResidentStrict) << std::endl;
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice _physicalDevice)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    int i = 0;
    for (const VkQueueFamilyProperties& queueFamilyProperty : queueFamilyProperties)
    {
        if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        ++i;
    }

    return indices;
}

void Application::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };
    float queuePriorities = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // sType
            nullptr,                                        // pNext
            VK_FALSE,                                       // flags
            queueFamily,                                    // queueFamilyIndex
            1,                                              // queueCount
            &queuePriorities                                // pQueuePriorities
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{ };

    #ifndef NDEBUG
        VkDeviceCreateInfo createInfo
        {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // sType
            nullptr,                                        // pNext
            VK_FALSE,                                       // flags
            static_cast<uint32_t>(queueCreateInfos.size()), // queueCreateInfoCount
            queueCreateInfos.data(),                        // pQueueCreateInfos
            static_cast<uint32_t>(validationLayers.size()), // enabledLayerCount
            validationLayers.data(),                        // ppEnabledLayerNames
            static_cast<uint32_t>(deviceExtensions.size()), // enabledExtensionCount
            deviceExtensions.data(),                        // ppEnabledExtensionNames
            &deviceFeatures                                 // pEnabledFeatures
        };
    #else
        VkDeviceCreateInfo createInfo
        {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // sType
            nullptr,                                        // pNext
            VK_FALSE,                                       // flags
            static_cast<uint32_t>(queueCreateInfos.size()), // queueCreateInfoCount
            queueCreateInfos.data(),                        // pQueueCreateInfos
            0,                                              // enabledLayerCount
            nullptr,                                        // ppEnabledLayerNames
            static_cast<uint32_t>(deviceExtensions.size()), // enabledExtensionCount
            deviceExtensions.data(),                        // ppEnabledExtensionNames
            &deviceFeatures                                 // pEnabledFeatures
        };
    #endif

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create logical device", FontColor::Red));
    }
    std::cout << setFontColor("Success to create logical device", FontColor::Green) << std::endl;

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Application::createSurface()
{
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create window surface", FontColor::Red));
    }
    std::cout << setFontColor("Success to create window surface", FontColor::Green) << std::endl;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice _physicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails Application::querySwapchainSupport(VkPhysicalDevice _physicalDevice)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, m_surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, m_surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, m_surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
    for (const VkSurfaceFormatKHR& availableFormat : _availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return _availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes)
{
    for (const VkPresentModeKHR& availablePresentMode : _availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities)
{
    if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return _capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    VkExtent2D actualExtent
    {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    actualExtent.width = std::clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);
    return actualExtent;
}

void Application::createSwapchain()
{
    SwapChainSupportDetails swapchainSupport = querySwapchainSupport(m_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };
    VkSwapchainCreateInfoKHR createInfo
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,                                                                // sType
        nullptr,                                                                                                    // pNext
        VK_FALSE,                                                                                                   // flags
        m_surface,                                                                                                  // surface
        imageCount,                                                                                                 // minImageCount
        surfaceFormat.format,                                                                                       // imageFormat
        surfaceFormat.colorSpace,                                                                                   // imageColorSpace
        extent,                                                                                                     // imageExtent
        1,                                                                                                          // imageArrayLayers
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                                                                        // imageUsage
        indices.graphicsFamily == indices.presentFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,   // imageSharingMode
        static_cast<uint32_t>(indices.graphicsFamily == indices.presentFamily ? 0 : 2),                             // queueFamilyIndexCount
        queueFamilyIndices,                                                                                         // pQueueFamilyIndices
        swapchainSupport.capabilities.currentTransform,                                                             // preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,                                                                          // compositeAlpha
        presentMode,                                                                                                // presentMode
        VK_TRUE,                                                                                                    // clipped
        VK_NULL_HANDLE                                                                                              // oldSwapchain
    };

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create swap chain", FontColor::Red));
    }
    std::cout << setFontColor("Success to create swap chain", FontColor::Green) << std::endl;

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
}

void Application::createImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,       // sType
            nullptr,                                        // pNext
            VK_FALSE,                                       // flags
            m_swapchainImages[i],                           // image
            VK_IMAGE_VIEW_TYPE_2D,                          // viewType
            m_swapchainImageFormat,                         // format
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
            },                                              // components
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            },                                              // subresourceRange
        };
        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error(setFontColor("Failed to create image views", FontColor::Red));
        }
        std::cout << setFontColor("Success to create image view " + std::to_string(i), FontColor::Green) << std::endl;
    }
}

void Application::createRenderPass()
{
    // 图像缓存描述
    VkAttachmentDescription colorAttachment
    {
        VK_FALSE,                                       // flags
        m_swapchainImageFormat,                         // format
        VK_SAMPLE_COUNT_1_BIT,                          // samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,                    // loadOp
        VK_ATTACHMENT_STORE_OP_STORE,                   // storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,                // stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,               // stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,                      // initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                 // finalLayout
    };

    // 附着引用
    VkAttachmentReference colorAttachmentRef
    {
        0,                                              // attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        // layout
    };

    // 子流程
    VkSubpassDescription subpass
    {
        VK_FALSE,                                       // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,                // pipelineBindPoint
        0,                                              // inputAttachmentCount
        nullptr,                                        // pInputAttachments
        1,                                              // colorAttachmentCount
        &colorAttachmentRef,                            // pColorAttachments
        nullptr,                                        // pResolveAttachments
        nullptr,                                        // pDepthStencilAttachment
        0,                                              // preserveAttachmentCount
        nullptr                                         // pPreserveAttachments
    };
    VkSubpassDependency subpassDependency
    {
        VK_SUBPASS_EXTERNAL,                            // srcSubpass
        0,                                              // dstSubpass
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // dstStageMask
        0,                                              // srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // dstAccessMask
        VK_FALSE                                        // dependencyFlags
    };
    VkRenderPassCreateInfo renderPassCreateInfo
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,          // sType
        nullptr,                                            // pNext
        VK_FALSE,                                           // flags
        1,                                                  // attachmentCount
        &colorAttachment,                                   // pAttachments
        1,                                                  // subpassCount
        &subpass,                                           // pSubpasses
        1,                                                  // dependencyCount
        &subpassDependency                                  // pDependencies
    };
    if (vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create render pass", FontColor::Red));
    }
    std::cout << setFontColor("Success to create render pass", FontColor::Green) << std::endl;
}

void Application::createGraphicsPipeline()
{
    std::string vertexShaderFilePath(ASSET_INCLUDE_PATH + std::string("shaders/shader.vert.spv"));
    std::vector<char> vertexShaderCode = readFile(vertexShaderFilePath);
    std::string fragmentShaderFilePath(ASSET_INCLUDE_PATH + std::string("shaders/shader.frag.spv"));
    std::vector<char> fragmentShaderCode = readFile(fragmentShaderFilePath);

    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    std::cout << setFontColor("Success to create vertex shader module", FontColor::Green) << std::endl;
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);
    std::cout << setFontColor("Success to create fragment shader module", FontColor::Green) << std::endl;

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        VK_SHADER_STAGE_VERTEX_BIT,                                 // stage
        vertexShaderModule,                                         // module
        "main",                                                     // pName
        nullptr                                                     // pSpecializationInfo
    };
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        VK_SHADER_STAGE_FRAGMENT_BIT,                               // stage
        fragmentShaderModule,                                       // module
        "main",                                                     // pName
        nullptr                                                     // pSpecializationInfo
    };

    VkPipelineShaderStageCreateInfo shaderStages[]{ vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    // 顶点输入
    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // sType
        nullptr,                                                        // pNext
        VK_FALSE,                                                       // flags
        1,                                                              // vertexBindingDescriptionCount
        & bindingDescription,                                           // pVertexBindingDescriptions
        static_cast<uint32_t>(attributeDescriptions.size()),            // vertexAttributeDescriptionCount
        attributeDescriptions.data()                                    // pVertexAttributeDescriptions
    };

    // 输入装配
    VkPipelineInputAssemblyStateCreateInfo inputAssembly
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // sType
        nullptr,                                                        // pNext
        VK_FALSE,                                                       // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                            // topology
        VK_FALSE                                                        // primitiveRestartEnable
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,          // sType
        nullptr,                                                        // pNext
        VK_FALSE,                                                       // flags
        1,                                                              // viewportCount
        nullptr,                                                        // pViewports
        1,                                                              // scissorCount
        nullptr                                                         // pScissors
    };

    // 光栅化
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,     // sType
        nullptr,                                                        // pNext
        VK_FALSE,                                                       // flags
        VK_FALSE,                                                       // depthClampEnable
        VK_FALSE,                                                       // rasterizerDiscardEnable
        VK_POLYGON_MODE_FILL,                                           // polygonMode
        VK_CULL_MODE_BACK_BIT,                                          // cullMode
        VK_FRONT_FACE_CLOCKWISE,                                        // frontFace
        VK_FALSE,                                                       // depthBiasEnable
        0.0f,                                                           // depthBiasConstantFactor
        0.0f,                                                           // depthBiasClamp
        0.0f,                                                           // depthBiasSlopeFactor
        1.0f                                                            // lineWidth
    };

    // 多重采样
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,       // sType
        nullptr,                                                        // pNext
        VK_FALSE,                                                       // flags
        VK_SAMPLE_COUNT_1_BIT,                                          // rasterizationSamples
        VK_FALSE,                                                       // sampleShadingEnable
        1.0f,                                                           // minSampleShading
        nullptr,                                                        // pSampleMask
        VK_FALSE,                                                       // alphaToCoverageEnable
        VK_FALSE                                                        // alphaToOneEnable
    };

    // 深度和模板测试
    // VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{ };

    // 颜色混合
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState
    {
        VK_FALSE,                                                   // blendEnable
        VK_BLEND_FACTOR_ONE,                                        // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                       // dstColorBlendFactor
        VK_BLEND_OP_ADD,                                            // colorBlendOp
        VK_BLEND_FACTOR_ONE,                                        // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                       // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                            // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT                                  // colorWriteMask
    };
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        VK_FALSE,                                                   // logicOpEnable
        VK_LOGIC_OP_COPY,                                           // logicOp
        1,                                                          // attachmentCount
        &colorBlendAttachmentState,                                 // pAttachments
        { 0.0f, 0.0f, 0.0f, 0.0f }                                  // blendConstants[4]
    };

    // 动态状态
    std::vector<VkDynamicState> dynamicStates
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,       // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        static_cast<uint32_t>(dynamicStates.size()),                // dynamicStateCount
        dynamicStates.data()                                        // pDynamicStates
    };

    // 管线布局
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,              // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        0,                                                          // setLayoutCount
        nullptr,                                                    // pSetLayouts
        0,                                                          // pushConstantRangeCount
        nullptr                                                     // pPushConstantRanges
    };
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create pipeline layout", FontColor::Red));
    }
    std::cout << setFontColor("Success to create pipeline layout", FontColor::Green) << std::endl;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // sType
        nullptr,                                                    // pNext
        VK_FALSE,                                                   // flags
        2,                                                          // stageCount
        shaderStages,                                               // pStages
        &vertexInputInfo,                                           // pVertexInputState
        &inputAssembly,                                             // pInputAssemblyState
        nullptr,                                                    // pTessellationState
        &viewportStateCreateInfo,                                   // pViewportState
        &rasterizationStateCreateInfo,                              // pRasterizationState
        &multisampleStateCreateInfo,                                // pMultisampleState
        nullptr,                                                    // pDepthStencilState
        &colorBlendStateCreateInfo,                                 // pColorBlendState
        &dynamicStateCreateInfo,                                    // pDynamicState
        m_pipelineLayout,                                           // layout
        m_renderPass,                                               // renderPass
        0,                                                          // subpass
        nullptr,                                                    // basePipelineHandle
        0                                                           // basePipelineIndex
    };
    if (vkCreateGraphicsPipelines(m_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create graphics pipeline", FontColor::Red));
    }
    std::cout << setFontColor("Success to create graphics pipeline", FontColor::Green) << std::endl;

    vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
    std::cout << setFontColor("Destroy the vertex shader module", FontColor::Indigo) << std::endl;
    vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);
    std::cout << setFontColor("Destroy the fragment shader module", FontColor::Indigo) << std::endl;
}

VkShaderModule Application::createShaderModule(const std::vector<char>& _code)
{
    VkShaderModuleCreateInfo createInfo
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,            // sType
        nullptr,                                                // pNext
        VK_FALSE,                                               // flags
        _code.size(),                                           // codeSize
        reinterpret_cast<const uint32_t*>(_code.data())         // pCode
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create shader module", FontColor::Red));
    }

    return shaderModule;
}

void Application::createFramebuffers()
{
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); ++i)
    {
        VkImageView attachments[]
        {
            m_swapchainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferCreateInfo
        {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,              // sType
            nullptr,                                                // pNext
            VK_FALSE,                                               // flags
            m_renderPass,                                           // renderPass
            1,                                                      // attachmentCount
            attachments,                                            // pAttachments
            m_swapchainExtent.width,                                // width
            m_swapchainExtent.height,                               // height
            1                                                       // layers
        };
        if (vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(setFontColor("Failed to create framebuffer " + std::to_string(i), FontColor::Red));
        }
        std::cout << setFontColor("Success to create framebuffer " + std::to_string(i), FontColor::Green) << std::endl;
    }
}

void Application::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,         // sType
        nullptr,                                            // pNext
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,    // flags
        queueFamilyIndices.graphicsFamily.value()           // queueFamilyIndex
    };
    if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create command pool", FontColor::Red));
    }
    std::cout << setFontColor("Success to create command pool", FontColor::Green) << std::endl;
}

void Application::createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkBuffer& _buffer, VkDeviceMemory& _deviceMemory)
{
    VkBufferCreateInfo bufferCreateInfo
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,               // sType
        nullptr,                                            // pNext
        VK_FALSE,                                           // flags
        _size,                                              // _size
        _usageFlags,                                        // _usageFlags
        VK_SHARING_MODE_EXCLUSIVE,                          // sharingMode
        0,                                                  // queueFamilyIndexCount
        nullptr                                             // pQueueFamilyIndices
    };
    if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to create a buffer", FontColor::Red));
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, _buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                                 // sType
        nullptr,                                                                // pNext
        memoryRequirements.size,                                                // allocationSize
        findMemoryType(memoryRequirements.memoryTypeBits, _propertyFlags)       // memoryTypeIndex
    };
    if (vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &_deviceMemory) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to allocate buffer memory", FontColor::Red));
    }

    vkBindBufferMemory(m_device, _buffer, _deviceMemory, 0);
}

void Application::copyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,                 // sType
        nullptr,                                                        // pNext
        m_commandPool,                                                  // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                                // level
        1                                                               // commandBufferCount
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,                // sType
        nullptr,                                                    // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,                // flags
        nullptr                                                     // pInheritanceInfo
    };
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkBufferCopy copyRegion
    {
        0,                          // srcOffset
        0,                          // dstOffset
        _size                       // size
    };
    vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,              // sType
        nullptr,                                    // pNext
        0,                                          // waitSemaphoreCount
        nullptr,                                    // pWaitSemaphores
        nullptr,                                    // pWaitDstStageMask
        1,                                          // commandBufferCount
        &commandBuffer,                             // pCommandBuffers
        0,                                          // signalSemaphoreCount
        nullptr                                     // pSignalSemaphores
    };
    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Application::createVertexBuffer()
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(m_device, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<size_t>(vertexBufferSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
    copyBuffer(stagingBuffer, m_vertexBuffer, vertexBufferSize);
    std::cout << setFontColor("Success to create vertex buffer", FontColor::Green) << std::endl;

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Application::createVertexIndicesBuffer()
{
    VkDeviceSize vertexIndicesBufferSize = sizeof(vertexIndices[0]) * vertexIndices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(vertexIndicesBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, vertexIndicesBufferSize, 0, &data);
    std::memcpy(data, vertexIndices.data(), static_cast<size_t>(vertexIndicesBufferSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(vertexIndicesBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexIndicesBuffer, m_vertexIndicesBufferMemory);
    copyBuffer(stagingBuffer, m_vertexIndicesBuffer, vertexIndicesBufferSize);
    std::cout << setFontColor("Success to create vertex indices buffer", FontColor::Green) << std::endl;

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

uint32_t Application::findMemoryType(uint32_t _typeFilter, VkMemoryPropertyFlags _properties)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &physicalDeviceMemoryProperties);
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
        if ((_typeFilter & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & _properties) == _properties))
        {
            return i;
        }
    }

    throw std::runtime_error(setFontColor("Failed to find suitable memory type", FontColor::Red));
}

void Application::createCommandBuffers()
{
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,         // sType
        nullptr,                                                // pNext
        m_commandPool,                                          // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                        // level
        static_cast<uint32_t>(m_commandBuffers.size())          // commandBufferCount
    };
    if (vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to allocate command buffers", FontColor::Red));
    }
    std::cout << setFontColor("Success to allocate command buffers", FontColor::Purple) << std::endl;
}

void Application::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_flightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,        // sType
        nullptr,                                        // pNext
        VK_FALSE                                        // flags
    };
    VkFenceCreateInfo fenceCreateInfo
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,            // sType
        nullptr,                                        // pNext
        VK_FENCE_CREATE_SIGNALED_BIT                    // flags
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_flightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(setFontColor("Failed to create synchronization objects " + std::to_string(i) + " for a frame", FontColor::Red));
        }
        std::cout << setFontColor("Success to create synchronization objects " + std::to_string(i) + " for a frame", FontColor::Green) << std::endl;
    }
}

void Application::drawFrame()
{
    vkWaitForFences(m_device, 1, &m_flightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], nullptr, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_framebufferResized = false;
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error(setFontColor("Failed to acquire swap chain image", FontColor::Red));
    }

    vkResetFences(m_device, 1, &m_flightFences[m_currentFrame]);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
    recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

    VkSemaphore waitSemaphores[]{ m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[]{ m_renderFinishedSemaphores[m_currentFrame]};
    VkSubmitInfo submitInfo
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                  // sType
        nullptr,                                        // pNext
        1,                                              // waitSemaphoreCount
        waitSemaphores,                                 // pWaitSemaphores
        waitStages,                                     // pWaitDstStageMask
        1,                                              // commandBufferCount
        &m_commandBuffers[m_currentFrame],              // pCommandBuffers
        1,                                              // signalSemaphoreCount
        signalSemaphores                                // pSignalSemaphores
    };
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_flightFences[m_currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to submit draw command buffer", FontColor::Red));
    }
    std::cout << setFontColor("Submit draw command buffer", FontColor::Purple) << std::endl;

    VkSwapchainKHR swapchains[]{ m_swapchain };
    VkPresentInfoKHR presentInfoKHR
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,         // sType
        nullptr,                                    // pNext
        1,                                          // waitSemaphoreCount
        signalSemaphores,                           // pWaitSemaphores
        1,                                          // swapchainCount
        swapchains,                                 // pSwapchains
        &imageIndex,                                // pImageIndices
        nullptr                                     // pResults
    };
    result = vkQueuePresentKHR(m_presentQueue, &presentInfoKHR);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to present swap chain image", FontColor::Red));
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Application::recordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // sType
        nullptr,                                            // pNext
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,       // flags
        nullptr                                             // pInheritanceInfo
    };
    if (vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to begin recording command buffer " + std::to_string(_imageIndex), FontColor::Red));
    }
    std::cout << setFontColor("Begin to record command buffer " + std::to_string(_imageIndex), FontColor::Purple) << std::endl;

    VkClearValue clearValues{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    VkRenderPassBeginInfo renderPassBeginInfo
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,           // sType
        nullptr,                                            // pNext
        m_renderPass,                                       // renderPass
        m_swapchainFramebuffers[_imageIndex],               // framebuffer
        {
            { 0, 0 },
            m_swapchainExtent
        },                                                  // renderArea
        1,                                                  // clearValueCount
        &clearValues                                        // pClearValues
    };
    vkCmdBeginRenderPass(_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    // 视口和裁剪
    VkViewport viewport
    {
        0.0f,                                                           // x
        0.0f,                                                           // y
        static_cast<float>(m_swapchainExtent.width),                    // width
        static_cast<float>(m_swapchainExtent.height),                   // height
        0.0f,                                                           // minDepth
        1.0f                                                            // maxDepth
    };
    vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

    // 裁剪范围
    VkRect2D scissor
    {
        {0, 0},
        m_swapchainExtent
    };
    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[]{ m_vertexBuffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(_commandBuffer, m_vertexIndicesBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(_commandBuffer);
    if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error(setFontColor("Failed to record command buffer " + std::to_string(_imageIndex), FontColor::Red));
    }
    std::cout << setFontColor("Success to record command buffer " + std::to_string(_imageIndex), FontColor::Purple) << std::endl;
}

void Application::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createFramebuffers();
}

void Application::cleanupSwapchain()
{
    for (size_t i = 0; i < m_swapchainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(m_device, m_swapchainFramebuffers[i], nullptr);
        std::cout << setFontColor("Destroy framebuffer " + std::to_string(i), FontColor::Indigo) << std::endl;
    }
    for (size_t i = 0; i < m_swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
        std::cout << setFontColor("Destroy the swap chain image view " + std::to_string(i), FontColor::Indigo) << std::endl;
    }
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    std::cout << setFontColor("Destroy the swapchain", FontColor::Indigo) << std::endl;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, VkDebugUtilsMessageTypeFlagsEXT _messageType, const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData, void* _pUserData)
{
    if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT & _messageSeverity)
    {
        std::cout << setFontColor("Validation layers error: " + std::string(_pCallbackData->pMessage), FontColor::Red) << std::endl;
    }
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT & _messageSeverity)
    {
        std::cout << setFontColor("Validation layers warning: " + std::string(_pCallbackData->pMessage), FontColor::Yellow) << std::endl;
    }
    else
    {
        std::cout << setFontColor("Validation layers verbose: " + std::string(_pCallbackData->pMessage), FontColor::White) << std::endl;
    }
    return VK_FALSE;
}

std::vector<char> Application::readFile(const std::string& _filename)
{
    std::ifstream file(_filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(setFontColor("Failed to open file", FontColor::Red));
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void Application::framebufferResizeCallback(GLFWwindow* _window, int _width, int _height)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
    app->m_framebufferResized = true;
}

VkVertexInputBindingDescription Application::getBindingDescription()
{
    VkVertexInputBindingDescription vertexInputBindingDescription
    {
        0,                                      // binding
        sizeof(Vertex),                         // stride
        VK_VERTEX_INPUT_RATE_VERTEX             // inputRate
    };
    return vertexInputBindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Application::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescriptions;
    vertexInputAttributeDescriptions[0].location = 0;
    vertexInputAttributeDescriptions[0].binding = 0;
    vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, positionOS);
    vertexInputAttributeDescriptions[1].location = 1;
    vertexInputAttributeDescriptions[1].binding = 0;
    vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

    return vertexInputAttributeDescriptions;
}
