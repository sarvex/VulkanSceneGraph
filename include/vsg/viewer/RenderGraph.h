#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

#include <vsg/viewer/Camera.h>
#include <vsg/viewer/Window.h>
#include <vsg/viewer/WindowResizeHandler.h>

namespace vsg
{

    class VSG_DECLSPEC RenderGraph : public Inherit<Group, RenderGraph>
    {
    public:
        RenderGraph();

        /// Construct RenderGraph assigning window and setting up clearValues with the appropriate settings for the Window's attachments and color.
        RenderGraph(ref_ptr<Window> in_window, ref_ptr<View> view = {});

        using Group::accept;

        /// execute vkCmdBeginRenderPass and then traverse the RenderGraph subgraph
        void accept(RecordTraversal& recordTraversal) const override;

        /// either window or framebuffer must be assigned. If framebuffer is set then it takes precedence, if not sense the appropriate window's framebuffer is used.
        ref_ptr<Framebuffer> framebuffer;
        ref_ptr<Window> window;

        /// RenderPass tp use passed to the vkCmdBeginRenderPass, either obtained from which of the framebuffer or window are active
        RenderPass* getRenderPass();

        /// Get the Exten2D of the attached Framebuffer or Widnow.
        VkExtent2D getExtent() const;


        /// ReandingArea settings for VkRenderPassBeginInfo.renderArea passed to the vkCmdBeginRenderPass, usually maps the ViewportState's scissor
        VkRect2D renderArea;

        /// Buffer clearing settings for vkRrenderPassInfo.clearValueCount & vkRenderPassInfo.pClearValues passed to the vkCmdBeginRenderPass
        using ClearValues = std::vector<VkClearValue>;
        ClearValues clearValues; // initialize window colour and depth/stencil

        /// Subpass contents stetting passed to vkCmdBeginRenderPass
        VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE;

        /// Callback used to automatically update viewports, scissors, renderArea and clears when the window is resized.
        /// By default is null so no resize handling is done.
        ref_ptr<WindowResizeHandler> windowResizeHandler;

        /// invoke the WindowResizeHandler, called automatically when window dimension change is detected.
        void resized();

        /// window extent at previous frame, used to track window resizes
        const uint32_t invalid_dimension = std::numeric_limits<uint32_t>::max();
        mutable VkExtent2D previous_extent = VkExtent2D{invalid_dimension, invalid_dimension};
    };
    VSG_type_name(vsg::RenderGraph);

    /// Convenience function that sets up RenderGraph and associated View to render the specified scene graph from the specified camera view.
    /// Assigns the WindowResizeHandler to provide basic window resize handling.
    extern VSG_DECLSPEC ref_ptr<RenderGraph> createRenderGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

} // namespace vsg
