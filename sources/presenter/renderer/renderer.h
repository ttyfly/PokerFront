#include <memory>

#include "common/math.h"

namespace presenter
{
    class MaterialData {};
    class RenderResourceData {};
    class RenderResource {};

    class Renderer
    {
    public:
        Renderer();
        virtual ~Renderer() {};

        /* Prepare for rendering. Should be called each frame before all Render() calling. */
        virtual void prepare() = 0;

        /* Render. */
        virtual void render(const RenderResource& renderResource, const pf_math::Transform& transform) = 0;

        /* Present buffer. Should be called each frame after all Render() calling. */
        virtual void present() = 0;

        /* Create a D3D11 render resource. */
        virtual std::unique_ptr<RenderResource> create_render_resource(const RenderResourceData& data) = 0;

        /* Set the main camera. */
        // void UseCamera(std::weak_ptr<Camera> camera);

        /* Resize buffers. Should be called when resizing the window. */
        virtual void resize_buffers(const pf_math::Rect& client_rect) = 0;
    };
}