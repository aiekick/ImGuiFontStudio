#include "TextureHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <MainFrame.h>

bool TextureHelper::sNeedToSkipRendering = false;

#if VULKAN
static void TextureHelper_check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

VkCommandBuffer TextureHelper::beginSingleTimeCommands(VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(MainFrame::sVulkanInitInfo.Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void TextureHelper::endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(MainFrame::sVulkanInitInfo.Queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(MainFrame::sVulkanInitInfo.Queue);

    vkFreeCommandBuffers(MainFrame::sVulkanInitInfo.Device, commandPool, 1, &commandBuffer);
}


std::shared_ptr<TextureObject> TextureHelper::CreateTextureFromBuffer(VkCommandPool vCommandPool, uint8_t* buffer, int w, int h, int n, TextureFilteringEnum vFiltering)
{
    std::shared_ptr<TextureObject> res = nullptr;

    // Use any command queue
    auto cmd = TextureHelper::beginSingleTimeCommands(vCommandPool);
    if (cmd)
    {
        res = TextureHelper::CreateTextureFromBuffer(cmd, buffer, w, h, n, vFiltering);

        TextureHelper::endSingleTimeCommands(vCommandPool, cmd);
    }

    return res;
}

std::shared_ptr<TextureObject> TextureHelper::CreateTextureFromBuffer(VkCommandBuffer command_buffer, uint8_t* buffer, int w, int h, int n, TextureFilteringEnum vFiltering)
{
    std::shared_ptr<TextureObject> res = std::shared_ptr<TextureObject>(new TextureObject,
        [](TextureObject* obj)
        {
            TextureHelper::DestroyTexture(obj);
            delete obj;
        }
    );

    res->w = w;
    res->h = h;
    res->n = n;

    VkResult err;

    size_t buffer_size = sizeof(char) * n * w * h;

    // Create the Image:
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.extent.width = w;
        info.extent.height = h;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        err = vkCreateImage(MainFrame::sVulkanInitInfo.Device, &info, MainFrame::sVulkanInitInfo.Allocator, &res->img);
        TextureHelper_check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(MainFrame::sVulkanInitInfo.Device, res->img, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = ImGui_ImplVulkanH_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
        err = vkAllocateMemory(MainFrame::sVulkanInitInfo.Device, &alloc_info, MainFrame::sVulkanInitInfo.Allocator, &res->imgMem);
        TextureHelper_check_vk_result(err);
        err = vkBindImageMemory(MainFrame::sVulkanInitInfo.Device, res->img, res->imgMem, 0);
        TextureHelper_check_vk_result(err);
    }

    // Create the Image Sampler :
    {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = (VkFilter)vFiltering;
        info.minFilter = (VkFilter)vFiltering;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        if (vFiltering == TextureFilteringEnum::TEX_FILTER_LINEAR)
        {
            info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            info.minLod = -1000;
            info.maxLod = 1000;
        }
        else
        {
            info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            info.minLod = VK_LOD_CLAMP_NONE;
            info.maxLod = VK_LOD_CLAMP_NONE;
        }
        err = vkCreateSampler(MainFrame::sVulkanInitInfo.Device, &info, MainFrame::sVulkanInitInfo.Allocator, &res->sam);
        TextureHelper_check_vk_result(err);
    }

    // Create the Image View:
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = res->img;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        err = vkCreateImageView(MainFrame::sVulkanInitInfo.Device, &info, MainFrame::sVulkanInitInfo.Allocator, &res->view);
        TextureHelper_check_vk_result(err);
    }

    // create the descriptor. will be put in ImTextureID
    res->descriptor = ImGui_ImplVulkanH_Create_UserTexture_Descriptor(res->sam, res->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Create the Upload Buffer:
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = buffer_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        err = vkCreateBuffer(MainFrame::sVulkanInitInfo.Device, &buffer_info, MainFrame::sVulkanInitInfo.Allocator, &res->buf);
        TextureHelper_check_vk_result(err);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(MainFrame::sVulkanInitInfo.Device, res->buf, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = ImGui_ImplVulkanH_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
        err = vkAllocateMemory(MainFrame::sVulkanInitInfo.Device, &alloc_info, MainFrame::sVulkanInitInfo.Allocator, &res->bufMem);
        TextureHelper_check_vk_result(err);
        err = vkBindBufferMemory(MainFrame::sVulkanInitInfo.Device, res->buf, res->bufMem, 0);
        TextureHelper_check_vk_result(err);
    }

    // Upload to Buffer:
    {
        char* map = NULL;
        err = vkMapMemory(MainFrame::sVulkanInitInfo.Device, res->bufMem, 0, buffer_size, 0, (void**)(&map));
        TextureHelper_check_vk_result(err);
        memcpy(map, buffer, buffer_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = res->bufMem;
        range[0].size = buffer_size;
        err = vkFlushMappedMemoryRanges(MainFrame::sVulkanInitInfo.Device, 1, range);
        if (err != VK_SUCCESS)
            printf("vkFlushMappedMemoryRanges issue");
        TextureHelper_check_vk_result(err);
        vkUnmapMemory(MainFrame::sVulkanInitInfo.Device, res->bufMem);
    }

    // Copy to Image:
    {
        VkImageMemoryBarrier copy_barrier[1] = {};
        copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].image = res->img;
        copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier[0].subresourceRange.levelCount = 1;
        copy_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = w;
        region.imageExtent.height = h;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(command_buffer, res->buf, res->img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier use_barrier[1] = {};
        use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].image = res->img;
        use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier[0].subresourceRange.levelCount = 1;
        use_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
    }

    err = vkDeviceWaitIdle(MainFrame::sVulkanInitInfo.Device);
    TextureHelper_check_vk_result(err);

    return res;
}

std::shared_ptr<TextureObject> TextureHelper::CreateTextureFromFile(VkCommandBuffer command_buffer, const char* inFile, TextureFilteringEnum vFiltering)
{
    std::shared_ptr<TextureObject> res = nullptr;

    printf("file to load : %s\n", inFile);

    int w, h, chans;
    unsigned char* imgDatas = stbi_load(inFile, &w, &h, &chans, STBI_rgb_alpha);
    if (imgDatas && w && h)
    {
        res = CreateTextureFromBuffer(command_buffer, imgDatas, w, h, 4, vFiltering);

        stbi_image_free(imgDatas);
    }

    return res;
}

// adapted from https://github.com/SaschaWillems/Vulkan/blob/master/examples/screenshot/screenshot.cpp
bool TextureHelper::SaveTextureToPng(VkCommandPool vCommandPool, GLFWwindow* vWin, const char* vFilePathName, std::shared_ptr<TextureObject> vTextureObject)
{
    bool res = false;

    if (vWin && strlen(vFilePathName) && vTextureObject)
    {
        VkResult err;
        
        VkImage srcImage = vTextureObject->img;

        // memory host image 
        VkImageCreateInfo imgCreateInfo = {};
        imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imgCreateInfo.extent.width = vTextureObject->w;
        imgCreateInfo.extent.height = vTextureObject->h;
        imgCreateInfo.extent.depth = 1;
        imgCreateInfo.arrayLayers = 1;
        imgCreateInfo.mipLevels = 1;
        imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkImage dstImage;
        err = vkCreateImage(MainFrame::sVulkanInitInfo.Device, &imgCreateInfo, MainFrame::sVulkanInitInfo.Allocator, &dstImage);
        TextureHelper_check_vk_result(err);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(MainFrame::sVulkanInitInfo.Device, dstImage, &req);
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = ImGui_ImplVulkanH_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, req.memoryTypeBits);
        if (alloc_info.memoryTypeIndex != 0xFFFFFFFF)
        {
            VkDeviceMemory dstImageMemory;
            err = vkAllocateMemory(MainFrame::sVulkanInitInfo.Device, &alloc_info, MainFrame::sVulkanInitInfo.Allocator, &dstImageMemory);
            TextureHelper_check_vk_result(err);
            err = vkBindImageMemory(MainFrame::sVulkanInitInfo.Device, dstImage, dstImageMemory, 0);
            TextureHelper_check_vk_result(err);

            VkImageMemoryBarrier imageMemoryBarrier = {};

            // Use any command queue
            auto copyCmd = TextureHelper::beginSingleTimeCommands(vCommandPool);
            if (copyCmd)
            {
                // dst image
                imageMemoryBarrier = VkImageMemoryBarrier();
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcAccessMask = 0;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.image = dstImage;
                imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

                // src image
                imageMemoryBarrier = VkImageMemoryBarrier();
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.image = srcImage;
                imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

                const bool supportsBlit = false;

                // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
                if (supportsBlit)
                {
                    // Define the region to blit (we will blit the whole swapchain image)
                    VkOffset3D blitSize;
                    blitSize.x = vTextureObject->w;
                    blitSize.y = vTextureObject->h;
                    blitSize.z = 1;
                    VkImageBlit imageBlitRegion{};
                    imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageBlitRegion.srcSubresource.layerCount = 1;
                    imageBlitRegion.srcOffsets[1] = blitSize;
                    imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageBlitRegion.dstSubresource.layerCount = 1;
                    imageBlitRegion.dstOffsets[1] = blitSize;

                    // Issue the blit command
                    vkCmdBlitImage(
                        copyCmd,
                        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &imageBlitRegion,
                        VK_FILTER_NEAREST);
                }
                else
                {
                    // Otherwise use image copy (requires us to manually flip components)
                    VkImageCopy imageCopyRegion{};
                    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.srcSubresource.layerCount = 1;
                    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.dstSubresource.layerCount = 1;
                    imageCopyRegion.extent.width = vTextureObject->w;
                    imageCopyRegion.extent.height = vTextureObject->h;
                    imageCopyRegion.extent.depth = 1;

                    // Issue the copy command
                    vkCmdCopyImage(
                        copyCmd,
                        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &imageCopyRegion);
                }

                // dst
                imageMemoryBarrier = VkImageMemoryBarrier();
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageMemoryBarrier.image = dstImage;
                imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

                // src
                imageMemoryBarrier = VkImageMemoryBarrier();
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.image = srcImage;
                imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

                TextureHelper::endSingleTimeCommands(vCommandPool, copyCmd);
            }

            // Get layout of the image (including row pitch)
            VkImageSubresource subResource{};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            VkSubresourceLayout subResourceLayout;
            vkGetImageSubresourceLayout(MainFrame::sVulkanInitInfo.Device, dstImage, &subResource, &subResourceLayout);

            // Map image memory so we can start copying from it
            const char* imagedata;
            vkMapMemory(MainFrame::sVulkanInitInfo.Device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
            imagedata += subResourceLayout.offset;

            int32_t resWrite = stbi_write_png(
                vFilePathName,
                vTextureObject->w,
                vTextureObject->h,
                vTextureObject->n,
                imagedata,
                subResourceLayout.rowPitch);

            res = (resWrite > 0);

            vkUnmapMemory(MainFrame::sVulkanInitInfo.Device, dstImageMemory);
            vkFreeMemory(MainFrame::sVulkanInitInfo.Device, dstImageMemory, nullptr);
        }
        vkDestroyImage(MainFrame::sVulkanInitInfo.Device, dstImage, nullptr);
    }

    return res;
}

#else

std::shared_ptr<TextureObject> TextureHelper::CreateTextureFromBuffer(uint8_t* buffer, int w, int h, int n, TextureFilteringEnum vFiltering)
{
    std::shared_ptr<TextureObject> res = std::shared_ptr<TextureObject>(new TextureObject,
        [](TextureObject* obj)
        {
            TextureHelper::DestroyTexture(obj);
            delete obj;
        }
    );

    res->w = w;
    res->h = h;
    res->n = n;

    size_t buffer_size = sizeof(char) * n * w * h;

    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

    GLuint id = 0;
    glGenTextures(1, &res->textureId);
    glBindTexture(GL_TEXTURE_2D, res->textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)vFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)vFiltering);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return res;
}

bool TextureHelper::SaveTextureToPng(GLFWwindow* vWin, const char* vFilePathName, std::shared_ptr<TextureObject> vTextureObject)
{
    bool res = false;

    if (vWin && strlen(vFilePathName) && vTextureObject)
    {
        glfwMakeContextCurrent(vWin);

        GLenum format = GL_RGBA;
        if (vTextureObject->n == 1) format = GL_RED;
        if (vTextureObject->n == 2) format = GL_RG;
        if (vTextureObject->n == 3) format = GL_RGB;
        if (vTextureObject->n == 4) format = GL_RGBA;

        // Upload texture to graphics system
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)vTextureObject->textureId);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        
        std::vector<uint8_t> imagedata;
        imagedata.resize((size_t)vTextureObject->w * (size_t)vTextureObject->h * (size_t)vTextureObject->n); // 1 channel only
        glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, imagedata.data());
        glBindTexture(GL_TEXTURE_2D, last_texture);

        const size_t stride = vTextureObject->w * vTextureObject->n;
        int32_t resWrite = stbi_write_png(
            vFilePathName,
            vTextureObject->w,
            vTextureObject->h,
            vTextureObject->n,
            imagedata.data(),
            stride);

        res = (resWrite > 0);
    }

    return res;
}

#endif


void TextureHelper::DestroyTexture(TextureObject* image_object)
{
    if (image_object)
    {
        TextureHelper::sNeedToSkipRendering = true;

#if VULKAN
        vkDeviceWaitIdle(MainFrame::sVulkanInitInfo.Device);

        if (image_object->buf)
        {
            vkDestroyBuffer(MainFrame::sVulkanInitInfo.Device, image_object->buf, MainFrame::sVulkanInitInfo.Allocator);
            image_object->buf = VK_NULL_HANDLE;
        }
        if (image_object->bufMem)
        {
            vkFreeMemory(MainFrame::sVulkanInitInfo.Device, image_object->bufMem, MainFrame::sVulkanInitInfo.Allocator);
            image_object->bufMem = VK_NULL_HANDLE;
        }

        if (image_object->view)
        {
            vkDestroyImageView(MainFrame::sVulkanInitInfo.Device, image_object->view, MainFrame::sVulkanInitInfo.Allocator);
            image_object->view = VK_NULL_HANDLE;
        }
        if (image_object->img)
        {
            vkDestroyImage(MainFrame::sVulkanInitInfo.Device, image_object->img, MainFrame::sVulkanInitInfo.Allocator);
            image_object->img = VK_NULL_HANDLE;
        }
        if (image_object->imgMem)
        {
            vkFreeMemory(MainFrame::sVulkanInitInfo.Device, image_object->imgMem, MainFrame::sVulkanInitInfo.Allocator);
            image_object->imgMem = VK_NULL_HANDLE;
        }
        if (image_object->sam)
        {
            vkDestroySampler(MainFrame::sVulkanInitInfo.Device, image_object->sam, MainFrame::sVulkanInitInfo.Allocator);
            image_object->sam = VK_NULL_HANDLE;
        }

        if (image_object->descriptor)
        {
            ImGui_ImplVulkanH_Destroy_UserTexture_Descriptor(&image_object->descriptor);
            image_object->descriptor = VK_NULL_HANDLE;
        }
#else
        // size_t is 4 bytes sized for x32 and 8 bytes sizes for x64.
        // TexID is ImTextureID is a void so same size as size_t
        // id is a uint so 4 bytes on x32 and x64
        // so conversion first on size_t (uint32/64) and after on GLuint give no warnings
        if (image_object->textureId)
        {
            glDeleteTextures(1, &image_object->textureId);
            image_object->textureId = 0U;
        }
#endif    
    }
}
