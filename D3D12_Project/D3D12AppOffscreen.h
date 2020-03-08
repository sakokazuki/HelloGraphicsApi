#pragma once

#include "D3D12AppBase.h"
#include <DirectXMath.h>


class D3D12AppOffscreen : public D3D12AppBase{

public:
	D3D12AppOffscreen() : D3D12AppBase(){}

	virtual void Prepare() override;
	virtual void Cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
		DirectX::XMFLOAT2 UV;
	};

	struct ShaderParameters
	{
		DirectX::XMFLOAT4X4 mtxWorld;
		DirectX::XMFLOAT4X4 mtxView;
		DirectX::XMFLOAT4X4 mtxProj;
	};

	enum
	{
		TextureSrvDescriptorBase = 0,
		ConstantBufferDescriptorBase = 1,
		// サンプラーは別ヒープなので先頭を使用
		SamplerDescriptorBase = 0,
	};

private:
	ComPtr<ID3D12Resource1> CreateBuffer(UINT bufferSize, const void* initialData);
	ComPtr<ID3D12Resource1> m_triangleVertexBuffer, m_cubeVertexBuffer;
	ComPtr<ID3D12Resource1> m_triangleIndexBuffer, m_cubeIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW  m_triangleVertexBufferView, m_cubeVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW   m_triangleIndexBufferView, m_cubeIndexBufferView;
	UINT  m_triangleIndexCount, m_cubeIndexCount;

	ComPtr<ID3DBlob>  m_offscreenVs, m_offscreenPs;
	ComPtr<ID3DBlob>  m_drawVs, m_drawPs;
	ComPtr<ID3D12RootSignature> m_offscreenRootSignature, m_drawRootSignature;
	ComPtr<ID3D12PipelineState> m_offscreenPipeline, m_drawPipeline;

	ComPtr<ID3D12Resource1> m_offsecreenBuffer;
	ComPtr<ID3D12DescriptorHeap> m_offscreenRtv;

	// キューブ描画用
	ComPtr<ID3D12DescriptorHeap> m_heapSrvCbv;
	ComPtr<ID3D12DescriptorHeap> m_heapSampler;
	UINT  m_samplerDescriptorSize;
	std::vector<ComPtr<ID3D12Resource1>> m_constantBuffers;
	D3D12_GPU_DESCRIPTOR_HANDLE m_sampler;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_cbViews;
};