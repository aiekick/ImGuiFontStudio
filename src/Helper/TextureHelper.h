#pragma once

#include <globals.h>
#include <memory>

struct TextureObject
{
#if VULKAN
    VkDeviceMemory      imgMem = VK_NULL_HANDLE;
    VkImage             img = VK_NULL_HANDLE;
    VkDeviceMemory      bufMem = VK_NULL_HANDLE;
    VkBuffer            buf = VK_NULL_HANDLE;
    VkSampler           sam = VK_NULL_HANDLE;
    VkImageView         view = VK_NULL_HANDLE;
    VkDescriptorSet     descriptor = VK_NULL_HANDLE;
#else
    GLuint              textureId = 0U;
#endif
};

#if VULKAN
enum class TextureFilteringEnum
{
    TEX_FILTER_LINEAR = VK_FILTER_LINEAR,
    TEX_FILTER_NEAREST = VK_FILTER_NEAREST
};
#else
enum class TextureFilteringEnum
{
    TEX_FILTER_LINEAR = GL_LINEAR,
    TEX_FILTER_NEAREST = GL_NEAREST
};
#endif

class TextureHelper
{
public:
    static bool sNeedToSkipRendering;

public:
#if VULKAN
    static std::shared_ptr<TextureObject> CreateTextureFromBuffer(VkCommandBuffer command_buffer, uint8_t* buffer, int w, int h, int n, TextureFilteringEnum vFiltering);
    static std::shared_ptr<TextureObject> CreateTextureFromFile(VkCommandBuffer command_buffer, const char* inFile, TextureFilteringEnum vFiltering, VkDescriptorSet* vOriginal = nullptr);
    static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool);
    static void endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer);
#else
    static std::shared_ptr<TextureObject> CreateTextureFromBuffer(uint8_t* buffer, int w, int h, int n, TextureFilteringEnum vFiltering);
#endif

    static void DestroyTexture(TextureObject* image_object);
};