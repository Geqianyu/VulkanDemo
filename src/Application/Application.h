#ifndef GQY_APPLICATION_H
#define GQY_APPLICATION_H

#include <exception>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <limits>
#include <fstream>

#include "common.h"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{ };
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Application
{
public:
    Application(const int _width, const int _height, const std::string& _name);
    Application(const Application& _application) = delete;
    ~Application();

    Application& operator = (const Application& _application) = delete;

    void run();

private:
    void initWindow(const int _width, const int _height, const std::string& _name);
    void initVulkan();
    void mainLoop();
    void cleanup();

    /*****************************************initVulkan******************************************/
    void createInstance();

    #ifndef NDEBUG
        bool checkValidationLayerProperties();
        void setupDebugMessenger();
        VkResult createDebugUtilsMessengerEXT(VkInstance _instance, const VkDebugUtilsMessengerCreateInfoEXT* _pCreateInfo, const VkAllocationCallbacks* _pAllocator, VkDebugUtilsMessengerEXT* _pDebugMessenger);
        VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();
        void destroyDebugUtilsMessengerEXT(VkInstance _instance, VkDebugUtilsMessengerEXT _debugMessenger, const VkAllocationCallbacks* _pAllocator);
    #endif

    std::vector<const char*> getRequiredExtensions();

    void pickPhysicalDevice();
    bool isSuitableDevice(const VkPhysicalDevice _physicalDevice);
    long long int rateDeviceSuitability(const VkPhysicalDevice _physicalDevice);
    void printPhysicalDeviceFeature(const VkPhysicalDevice _physicalDevice);
    void printPhysicalDeviceProperties(const VkPhysicalDevice _physicalDevice);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice _physicalDevice);
    void createLogicalDevice();
    void createSurface();
    bool checkDeviceExtensionSupport(VkPhysicalDevice _physicalDevice);
    SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice _physicalDevice);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities);
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char>& _code);
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    /*********************************************************************************************/

    /******************************************mainLoop*******************************************/
    void drawFrame();
    void recordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);
    void recreateSwapchain();
    void cleanupSwapchain();
    /*********************************************************************************************/

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT _messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
        void* _pUserData
    );

    static std::vector<char> readFile(const std::string& _filename);
    static void framebufferResizeCallback(GLFWwindow* _window, int _width, int _height);

private:
    GLFWwindow* m_window = nullptr;

    VkInstance m_instance = nullptr;

    #ifndef NDEBUG
        VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
    #endif

    VkSurfaceKHR m_surface = nullptr;

    VkPhysicalDevice m_physicalDevice = nullptr;
    VkDevice m_device = nullptr;

    VkQueue m_graphicsQueue = nullptr;
    VkQueue m_presentQueue = nullptr;

    VkSwapchainKHR m_swapchain = nullptr;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat = VkFormat::VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainExtent{ };
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;

    VkRenderPass m_renderPass = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_graphicsPipeline = nullptr;

    VkCommandPool m_commandPool = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_flightFences;
    bool m_framebufferResized = false;
    uint32_t m_currentFrame = 0;
};

#endif