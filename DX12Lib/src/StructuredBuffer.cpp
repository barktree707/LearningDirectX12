#include <DX12LibPCH.h>

#include <StructuredBuffer.h>

#include <Application.h>
#include <ResourceStateTracker.h>

#include <d3dx12.h>

StructuredBuffer::StructuredBuffer( const std::wstring& name )
    : Buffer(name)
    , m_NumElements(0)
    , m_ElementSize(0)
{
    m_SRV = Application::Get().AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    m_UAV = Application::Get().AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    // Create a byte address buffer for the counter.
    auto device = Application::Get().GetDevice();

    ComPtr<ID3D12Resource> counterResource;
    ThrowIfFailed( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( 4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS( &counterResource ) ) );

    ResourceStateTracker::AddGlobalResourceState( counterResource.Get(), D3D12_RESOURCE_STATE_COMMON );
    m_CounterBuffer.SetD3D12Resource( counterResource );
    m_CounterBuffer.CreateViews( 1, 4 );
}

void StructuredBuffer::CreateViews( size_t numElements, size_t elementSize )
{
    auto device = Application::Get().GetDevice();

    m_NumElements = numElements;
    m_ElementSize = elementSize;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = static_cast<UINT>( m_NumElements );
    srvDesc.Buffer.StructureByteStride = static_cast<UINT>( m_ElementSize );
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device->CreateShaderResourceView( m_d3d12Resource.Get(), &srvDesc, m_SRV );

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.NumElements = static_cast<UINT>( m_NumElements );
    uavDesc.Buffer.StructureByteStride = static_cast<UINT>( m_ElementSize );
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    device->CreateUnorderedAccessView( m_d3d12Resource.Get(),
                                       m_CounterBuffer.GetD3D12Resource().Get(),
                                       &uavDesc, m_UAV );
}
