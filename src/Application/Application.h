#ifndef GQY_APPLICATION_H
#define GQY_APPLICATION_H

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <exception>
#include <vector>
#include <cstring>
#include <map>
#include <unordered_map>
#include <optional>
#include <set>
#include <limits>
#include <fstream>
#include <array>
#include <chrono>
#include <algorithm>

#include "common.h"

struct Vertex
{
    glm::vec3 positionOS;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator == (const Vertex& _vertex) const;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

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
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char>& _code);
    void createFramebuffers();
    void createCommandPool();
    void createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkBuffer& _buffer, VkDeviceMemory& _deviceMemory);
    void copyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
    void createDepthResource();
    void createColorResource();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _imageTiling, VkFormatFeatureFlags _formatFeatureFlags);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat _format);
    void generateMipmaps(VkImage _image, VkFormat _format, int32_t _textureWidth, int32_t _textureHeight, uint32_t _mipLevels);
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    VkImageView createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _imageAspectFlags, uint32_t _mipLevels);
    void createImage(uint32_t _width, uint32_t _height, uint32_t _mipLevels, VkSampleCountFlagBits _sampleCountFlagBits, VkFormat _format, VkImageTiling _imageTiling, VkImageUsageFlags _imageUsageFlags, VkMemoryPropertyFlags _memoryPropertyFlags, VkImage& _image, VkDeviceMemory& _imageMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer _commandBuffer);
    void transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldImageLayout, VkImageLayout _newImageLayout, uint32_t _mipLevels);
    void copyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);
    void loadModel();
    void createVertexBuffer();
    void createVertexIndicesBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    uint32_t findMemoryType(uint32_t _typeFilter, VkMemoryPropertyFlags _properties);
    void createCommandBuffers();
    void createSyncObjects();
    VkSampleCountFlagBits getMaxUsableSampleCount();
    /*********************************************************************************************/

    /******************************************mainLoop*******************************************/
    void drawFrame();
    void updateUniformBuffer(uint32_t _currentFrame);
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
    VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_graphicsPipeline = nullptr;

    VkCommandPool m_commandPool = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;

    uint32_t m_mipLevels = 0;
    VkImage m_textureImage = nullptr;
    VkDeviceMemory m_textureImageMemory = nullptr;
    VkImageView m_textureImageView = nullptr;
    VkSampler m_textureSampler = nullptr;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_vertexIndices;
    VkBuffer m_vertexBuffer = nullptr;
    VkDeviceMemory m_vertexBufferMemory = nullptr;
    VkBuffer m_vertexIndicesBuffer = nullptr;
    VkDeviceMemory m_vertexIndicesBufferMemory = nullptr;

    VkImage m_depthImage = nullptr;
    VkDeviceMemory m_depthImageMemory = nullptr;
    VkImageView m_depthImageView = nullptr;

    VkSampleCountFlagBits m_massSamples = VK_SAMPLE_COUNT_1_BIT;
    VkImage m_colorImage = nullptr;
    VkDeviceMemory m_colorImageMemory = nullptr;
    VkImageView m_colorImageView = nullptr;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_flightFences;
    bool m_framebufferResized = false;

    uint32_t m_currentFrame = 0;
};

#endif