#include "Precompiled.h"
#include "VKFramebuffer.h"
#include "VKDevice.h"
#include "VKTexture.h"
#include "VKInitialisers.h"
#include "VKUtilities.h"
#include "VKRenderer.h"
#include "VKTexture.h"

namespace Lumos
{
    namespace Graphics
    {
        VKFramebuffer::VKFramebuffer(const FramebufferDesc& frameBufferInfo)
            : m_Framebuffer(nullptr)
        {
            m_Width           = frameBufferInfo.width;
            m_Height          = frameBufferInfo.height;
            m_AttachmentCount = frameBufferInfo.attachmentCount;

            TDArray<VkImageView> attachments;

            for(uint32_t i = 0; i < m_AttachmentCount; i++)
            {
                switch(frameBufferInfo.attachmentTypes[i])
                {
                case TextureType::COLOUR:
                    attachments.PushBack(frameBufferInfo.mipIndex >= 0 ? static_cast<VKTexture2D*>(frameBufferInfo.attachments[i])->GetMipImageView(frameBufferInfo.mipIndex) : static_cast<VKTexture2D*>(frameBufferInfo.attachments[i])->GetImageView());
                    break;
                case TextureType::DEPTH:
                    attachments.PushBack(static_cast<VKTextureDepth*>(frameBufferInfo.attachments[i])->GetImageView());
                    break;
                case TextureType::DEPTHARRAY:
                    attachments.PushBack(static_cast<VKTextureDepthArray*>(frameBufferInfo.attachments[i])->GetImageView(frameBufferInfo.layer));
                    break;
                case TextureType::OTHER:
                    attachments.PushBack(static_cast<VKTexture2D*>(frameBufferInfo.attachments[i])->GetImageView());
                    break;
                case TextureType::CUBE:
                    attachments.PushBack(frameBufferInfo.mipIndex >= 0 ? static_cast<VKTextureCube*>(frameBufferInfo.attachments[i])->GetImageView(frameBufferInfo.layer, frameBufferInfo.mipIndex) : static_cast<VKTextureCube*>(frameBufferInfo.attachments[i])->GetImageView(frameBufferInfo.layer));
                    break;
                }
            }

            VkFramebufferCreateInfo framebufferCreateInfo = VKInitialisers::FramebufferCreateInfo();
            framebufferCreateInfo.renderPass              = static_cast<VKRenderPass*>(frameBufferInfo.renderPass)->GetHandle();
            framebufferCreateInfo.attachmentCount         = m_AttachmentCount;
            framebufferCreateInfo.pAttachments            = attachments.Data();
            framebufferCreateInfo.width                   = m_Width;
            framebufferCreateInfo.height                  = m_Height;
            framebufferCreateInfo.layers                  = 1;
            framebufferCreateInfo.pNext                   = nullptr;
            framebufferCreateInfo.flags                   = 0;

            VK_CHECK_RESULT(vkCreateFramebuffer(VKDevice::Get().GetDevice(), &framebufferCreateInfo, VK_NULL_HANDLE, &m_Framebuffer));
        }

        VKFramebuffer::~VKFramebuffer()
        {
            DeletionQueue& deletionQueue = VKRenderer::GetCurrentDeletionQueue();

            auto framebuffer = m_Framebuffer;

            deletionQueue.PushFunction([framebuffer]
                                       { vkDestroyFramebuffer(VKDevice::Get().GetDevice(), framebuffer, VK_NULL_HANDLE); });
        }

        void VKFramebuffer::MakeDefault()
        {
            CreateFunc = CreateFuncVulkan;
        }

        Framebuffer* VKFramebuffer::CreateFuncVulkan(const FramebufferDesc& info)
        {
            return new VKFramebuffer(info);
        }
    }
}
