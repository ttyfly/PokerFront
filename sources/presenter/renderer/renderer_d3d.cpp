#include "renderer_d3d.h"

#include <d3dcompiler.h>

#include "OptMacros.h"
#include "Component/Camera.h"

// Constant buffer
struct Constants
{
    DirectX::XMFLOAT4X4 modelViewProj;
};

Renderer_DX11::Renderer_DX11(HWND hWnd)
{
    m_ShouldResizeBuffers = false;
    Initialize(hWnd);
}

Renderer_DX11::~Renderer_DX11()
{
    if (m_DeviceContext)
        m_DeviceContext->ClearState();
}

void Renderer_DX11::Prepare()
{
    float clientWidth = (float)(m_ClientRect.right - m_ClientRect.left);
    float clientHeight = (float)(m_ClientRect.bottom - m_ClientRect.top);

    if (m_ShouldResizeBuffers)
    {
        m_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        m_RenderTargetView.Reset();
        m_DepthStencilView.Reset();

        HRESULT hr = m_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        assert(SUCCEEDED(hr));

        CreateRenderTargets();

        m_ShouldResizeBuffers = false;
    }

    if (m_Camera.expired())
    {
        CreateDefaultViewProjectionMatrix();
    }
    else
    {
        std::shared_ptr<Camera> camera = m_Camera.lock();
        assert(camera != nullptr);

        DirectX::XMMATRIX viewMat = camera->GetViewMatrix();
        DirectX::XMMATRIX projMat = camera->GetProjectionMatrix();
        DirectX::XMStoreFloat4x4(&m_CurrentProjectionMatrix, projMat);
        DirectX::XMStoreFloat4x4(&m_CurrentViewProjectionMatrix, viewMat * projMat);
    }

    float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), backgroundColor);
    m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, clientWidth, clientHeight, 0.0f, 1.0f };
    m_DeviceContext->RSSetViewports(1, &viewport);

    m_DeviceContext->RSSetState(m_RasterizerState.Get());
    m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);
    m_DeviceContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());

    m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
}

void Renderer_DX11::Render(const D3DRenderResource& renderResource, DirectX::FXMMATRIX modelTransform)
{
    DirectX::XMMATRIX viewProjectionMatrix = DirectX::XMLoadFloat4x4(&m_CurrentViewProjectionMatrix);
    DirectX::XMMATRIX modelViewProjMatrix = modelTransform * viewProjectionMatrix;

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    m_DeviceContext->Map(m_DefaultConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    Constants* constants = (Constants*)(mappedSubresource.pData);
    DirectX::XMStoreFloat4x4(&constants->modelViewProj, DirectX::XMMatrixTranspose(modelViewProjMatrix));
    m_DeviceContext->Unmap(m_DefaultConstantBuffer.Get(), 0);

    m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_DeviceContext->IASetInputLayout(m_DefaultInputLayout.Get());

    m_DeviceContext->VSSetShader(m_DefaultVertexShader.Get(), NULL, 0);
    m_DeviceContext->PSSetShader(m_DefaultPixelShader.Get(), NULL, 0);

    m_DeviceContext->VSSetConstantBuffers(0, 1, m_DefaultConstantBuffer.GetAddressOf());

    UINT vertexStride = sizeof(VertexData);
    UINT vertexOffset = 0;

    m_DeviceContext->IASetVertexBuffers(0, 1, renderResource.VertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);

    if (renderResource.IndexBuffer != nullptr)
    {
        m_DeviceContext->IASetIndexBuffer(renderResource.IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

        for (const FaceInfo& faceInfo : renderResource.FaceInfoArray)
        {
            if (!renderResource.TextureViews.empty() && faceInfo.Textured)
            {
                m_DeviceContext->PSSetShaderResources(0, 1, renderResource.TextureViews[faceInfo.TextureIndex].GetAddressOf());
            }

            m_DeviceContext->DrawIndexed(faceInfo.IndexCount, faceInfo.IndexOffset, 0);
        }
    }
    else
    {
        for (const FaceInfo& faceInfo : renderResource.FaceInfoArray)
        {
            if (!renderResource.TextureViews.empty() && faceInfo.Textured)
            {
                m_DeviceContext->PSSetShaderResources(0, 1, renderResource.TextureViews[faceInfo.TextureIndex].GetAddressOf());
            }

            m_DeviceContext->Draw(faceInfo.IndexCount, faceInfo.IndexOffset);
        }
    }
}

void Renderer_DX11::Present()
{
    m_SwapChain->Present(1, 0);
}

std::unique_ptr<D3DRenderResource> Renderer_DX11::CreateRenderResource(const RenderResourceData& renderResourceData)
{
    assert(!renderResourceData.VertexDataArray.empty());

    std::unique_ptr<D3DRenderResource> renderResource = std::make_unique<D3DRenderResource>();

    /* Create vertex buffer */

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = renderResourceData.VertexDataArray.size() * sizeof(VertexData);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = { 0 };
    vertexSubresourceData.pSysMem = renderResourceData.VertexDataArray.data();

    HRESULT hr = m_Device->CreateBuffer(
        &vertexBufferDesc,
        &vertexSubresourceData,
        renderResource->VertexBuffer.GetAddressOf());
    assert(SUCCEEDED(hr));

    /* Create index buffer */

    if (!renderResourceData.IndexArray.empty())
    {
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = renderResourceData.IndexArray.size() * sizeof(UINT16);
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { 0 };
        indexSubresourceData.pSysMem = renderResourceData.IndexArray.data();

        hr = m_Device->CreateBuffer(
            &indexBufferDesc,
            &indexSubresourceData,
            renderResource->IndexBuffer.GetAddressOf());
        assert(SUCCEEDED(hr));
    }

    /* Create texture views */

    for (const TextureData& textureData : renderResourceData.TextureDataArray)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = textureData.Width;
        textureDesc.Height = textureData.Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = textureData.Data;
        textureSubresourceData.SysMemPitch = 4 * textureData.Width;

        ComPtr<ID3D11Texture2D> texture;
        m_Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        ComPtr<ID3D11ShaderResourceView> textureView;
        m_Device->CreateShaderResourceView(texture.Get(), nullptr, &textureView);

        renderResource->TextureViews.push_back(textureView);
    }

    /* Copy face info */
    renderResource->FaceInfoArray = renderResourceData.FaceInfoArray;

    return renderResource;
}

void Renderer_DX11::UseCamera(std::weak_ptr<Camera> camera)
{
    m_Camera = camera;
}

void Renderer_DX11::ResizeBuffers(RECT clientRect)
{
    m_ShouldResizeBuffers = true;
    m_ClientRect = clientRect;
}

void Renderer_DX11::Initialize(HWND hWnd)
{
    GetClientRect(hWnd, &m_ClientRect);
    CreateSwapChain(hWnd);
    CreateRenderTargets();
    CreateDefaultShaders();
    CreateDefaultConstantBuffer();
    CreateRenderStates();
    CreateDefaultViewProjectionMatrix();
}

void Renderer_DX11::CreateSwapChain(HWND hWnd)
{
    /* Create D3D11 Device and Context */
    {
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> deviceContext;

        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
            0, creationFlags,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, device.GetAddressOf(),
            0, deviceContext.GetAddressOf());
        assert(SUCCEEDED(hr));

        hr = device.As(&m_Device);
        assert(SUCCEEDED(hr));

        hr = deviceContext.As(&m_DeviceContext);
        assert(SUCCEEDED(hr));
    }

    /* Create Swap Chain */
    {
        ComPtr<IDXGIDevice1> dxgiDevice;
        HRESULT hr = m_Device.As(&dxgiDevice);
        assert(SUCCEEDED(hr));

        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
        assert(SUCCEEDED(hr));

        //DXGI_ADAPTER_DESC adapterDesc;
        //dxgiAdapter->GetDesc(&adapterDesc);

        //OutputDebugStringA("Graphics Device: ");
        //OutputDebugStringW(adapterDesc.Description);

        ComPtr<IDXGIFactory2> dxgiFactory;
        hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)dxgiFactory.GetAddressOf());
        assert(SUCCEEDED(hr));

        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
        d3d11SwapChainDesc.Width = 0;
        d3d11SwapChainDesc.Height = 0;
        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        d3d11SwapChainDesc.SampleDesc.Count = 1;
        d3d11SwapChainDesc.SampleDesc.Quality = 0;
        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d3d11SwapChainDesc.BufferCount = 2;
        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        d3d11SwapChainDesc.Flags = 0;

        hr = dxgiFactory->CreateSwapChainForHwnd(
            m_Device.Get(),
            hWnd,
            &d3d11SwapChainDesc,
            0, 0,
            m_SwapChain.GetAddressOf());
        assert(SUCCEEDED(hr));
    }

//    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
//    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
//    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
//    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
//    swapChainDesc.SampleDesc.Count = 1;
//    swapChainDesc.SampleDesc.Quality = 0;
//    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//    swapChainDesc.BufferCount = 2;
//    swapChainDesc.OutputWindow = hWnd;
//    swapChainDesc.Windowed = true;
//
//    D3D_FEATURE_LEVEL featureLevel;
//    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
//#ifdef _DEBUG
//    flags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//    HRESULT hr = D3D11CreateDeviceAndSwapChain(
//        NULL,
//        D3D_DRIVER_TYPE_HARDWARE,
//        NULL,
//        flags,
//        NULL,
//        0,
//        D3D11_SDK_VERSION,
//        &swapChainDesc,
//        m_SwapChain.GetAddressOf(),
//        m_Device.GetAddressOf(),
//        &featureLevel,
//        m_DeviceContext.GetAddressOf()
//    );
//
//    assert(S_OK == hr && m_SwapChain.Get() && m_Device.Get() && m_DeviceContext.Get());
}

void Renderer_DX11::CreateRenderTargets()
{
    /* Create render target view */

    ComPtr<ID3D11Texture2D> framebuffer;

    HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)framebuffer.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = m_Device->CreateRenderTargetView(framebuffer.Get(), 0, m_RenderTargetView.GetAddressOf());
    assert(SUCCEEDED(hr));

    /* Create depth stencil view */

    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    framebuffer->GetDesc(&depthStencilBufferDesc);

    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthBuffer;

    hr = m_Device->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthBuffer.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = m_Device->CreateDepthStencilView(depthBuffer.Get(), nullptr, m_DepthStencilView.GetAddressOf());
    assert(SUCCEEDED(hr));
}

void Renderer_DX11::CreateDefaultShaders()
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // Compile vertex shader
    HRESULT hr = D3DCompileFromFile(
        DEFAULT_VERTEX_SHADER,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        flags,
        0,
        vsBlob.GetAddressOf(),
        errorBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errorBlob.Get())
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }

    // Compile pixel shader
    hr = D3DCompileFromFile(
        DEFAULT_PIXEL_SHADER,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        flags,
        0,
        psBlob.GetAddressOf(),
        errorBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errorBlob.Get())
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }

    hr = m_Device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        NULL,
        m_DefaultVertexShader.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = m_Device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        NULL,
        m_DefaultPixelShader.GetAddressOf());
    assert(SUCCEEDED(hr));

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        /*
        { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        */
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = m_Device->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_DefaultInputLayout.GetAddressOf());
    assert(SUCCEEDED(hr));
}

void Renderer_DX11::CreateDefaultConstantBuffer()
{
    D3D11_BUFFER_DESC constantBufferDesc = {};
    // ByteWidth must be a multiple of 16, per the docs
    constantBufferDesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hResult = m_Device->CreateBuffer(&constantBufferDesc, nullptr, &m_DefaultConstantBuffer);
    assert(SUCCEEDED(hResult));
}

void Renderer_DX11::CreateRenderStates()
{
    /* Create rasterizer state */

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    //rasterizerDesc.FrontCounterClockwise = TRUE;

    HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerState);
    assert(SUCCEEDED(hr));

    /* Create depth stencil state */

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    assert(SUCCEEDED(hr));

    /* Create Sampler State */

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    hr = m_Device->CreateSamplerState(&samplerDesc, m_SamplerState.GetAddressOf());
    assert(SUCCEEDED(hr));
}

void Renderer_DX11::CreateDefaultViewProjectionMatrix()
{
    using namespace DirectX;

    float fovAngleY = XM_PIDIV4;
    float nearZ = 0.1f;
    float farZ = 10000.0f;

    float clientWidth = (float)(m_ClientRect.right - m_ClientRect.left);
    float clientHeight = (float)(m_ClientRect.bottom - m_ClientRect.top);

    XMVECTOR upVector = XMVectorSet(UP_VECTOR, 0.0f);
    XMVECTOR frontVector = XMVectorSet(FRONT_VECTOR, 0.0f);

    XMMATRIX projMat = XMMatrixPerspectiveFovLH(fovAngleY, clientWidth / clientHeight, nearZ, farZ);
    XMMATRIX viewMat = XMMatrixLookToLH(XMVectorZero(), frontVector, upVector);

    XMStoreFloat4x4(&m_CurrentProjectionMatrix, projMat);
    XMStoreFloat4x4(&m_CurrentViewProjectionMatrix, viewMat * projMat);
}
