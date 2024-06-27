#pragma once

#include <memory>
#include <vector>
#include <string>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include "renderer.h"

namespace presenter
{
    class Camera;

    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    struct VertexData
    {
        DirectX::XMFLOAT3 Vertex;
        DirectX::XMFLOAT2 TexCoord;
    };

    struct TextureData
    {
        UINT8* Data;
        UINT Width;
        UINT Height;
    };

    struct FaceInfo
    {
        UINT IndexOffset;
        UINT IndexCount;
        bool Textured;
        UINT TextureIndex;
    };

    struct RenderResourceData
    {
        std::vector<VertexData> VertexDataArray;
        std::vector<UINT16> IndexArray;
        std::vector<TextureData> TextureDataArray;
        std::vector<FaceInfo> FaceInfoArray;
    };

    /**
     * Render resource for D3D11. SceneObject takes the ownership of it and should
     * keep it alive as long as rendering is needed.
     */
    struct D3DRenderResource
    {
        ComPtr<ID3D11Buffer> VertexBuffer;
        ComPtr<ID3D11Buffer> IndexBuffer;
        std::vector<ComPtr<ID3D11ShaderResourceView>> TextureViews;
        std::vector<FaceInfo> FaceInfoArray;
    };

    /* Direct3D 11 renderer */
    class Renderer_DX11 : Renderer
    {
    public:
        Renderer_DX11(HWND hWnd);
        ~Renderer_DX11();

        /* Prepare for rendering. Should be called each frame before all Render() calling. */
        virtual void Prepare();

        /* Render. */
        virtual void Render(const D3DRenderResource& renderResource, DirectX::FXMMATRIX modelTransform);

        /* Present buffer. Should be called each frame after all Render() calling. */
        virtual void Present();

        /* Create a D3D11 render resource. */
        virtual std::unique_ptr<D3DRenderResource> CreateRenderResource(const RenderResourceData& renderResourceData);

        /* Set the main camera. */
        // void UseCamera(std::weak_ptr<Camera> camera);

        /* Resize buffers. Should be called when resizing the window. */
        virtual void ResizeBuffers(RECT clientRect);

    protected:
        void Initialize(HWND hWnd);
        void CreateSwapChain(HWND hWnd);
        void CreateRenderTargets();
        void CreateDefaultShaders();
        void CreateDefaultConstantBuffer();
        void CreateRenderStates();
        void CreateDefaultViewProjectionMatrix();

        ComPtr<ID3D11Device1> m_Device;
        ComPtr<ID3D11DeviceContext1> m_DeviceContext;
        ComPtr<IDXGISwapChain1> m_SwapChain;

        ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
        ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

        ComPtr<ID3D11VertexShader> m_DefaultVertexShader;
        ComPtr<ID3D11PixelShader> m_DefaultPixelShader;
        ComPtr<ID3D11InputLayout> m_DefaultInputLayout;
        ComPtr<ID3D11Buffer> m_DefaultConstantBuffer;

        ComPtr<ID3D11RasterizerState> m_RasterizerState;
        ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
        ComPtr<ID3D11SamplerState> m_SamplerState;

        std::weak_ptr<Camera> m_Camera;
        DirectX::XMFLOAT4X4 m_CurrentViewProjectionMatrix;
        DirectX::XMFLOAT4X4 m_CurrentProjectionMatrix;

        bool m_ShouldResizeBuffers;
        RECT m_ClientRect;
    };
};
