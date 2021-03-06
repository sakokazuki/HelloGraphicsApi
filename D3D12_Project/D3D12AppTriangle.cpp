
#include "D3D12AppTriangle.h"
#include <stdexcept>

void D3D12AppTriangle::Prepare()
{
	Vertex triangleVertices[] = {
		{ { 0.0f, 0.25f, 0.5f },{ 1.0f, 0.0f,0.0f,1.0f } },
	{ { 0.25f,-0.25f, 0.5f },{ 0.0f, 1.0f,0.0f,1.0f } },
	{ { -0.25f,-0.25f, 0.5f },{ 0.0f, 0.0f,1.0f,1.0f } },
	};
	uint32_t indices[] = { 0, 1, 2 };

	// 頂点バッファとインデックスバッファの生成.
	m_vertexBuffer = CreateBuffer(sizeof(triangleVertices), triangleVertices);
	m_indexBuffer = CreateBuffer(sizeof(indices), indices);
	m_indexCount = _countof(indices);

	// vertex/index bufferviewの生成.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// シェーダーをコンパイル.
	HRESULT hr;
	ComPtr<ID3DBlob> errBlob;
	hr = CompileShaderFromFile(L"triangleVs.hlsl", L"vs_6_0", m_vs, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
	}
	hr = CompileShaderFromFile(L"trianglePs.hlsl", L"ps_6_0", m_ps, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
	}

	// ルートシグネチャの構築
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	ComPtr<ID3D10Blob> signature;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
	if (FAILED(hr))
	{
		throw std::runtime_error("D3D12SerializeRootSignature failed.");
	}

	// RootSignature の生成
	hr = m_device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
	
	if (FAILED(hr))
	{
		throw std::runtime_error("CrateRootSignature failed.");
	}

	

	

	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, offsetof(Vertex,Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
	};


	// パイプラインステートオブジェクトの生成.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	// シェーダーのセット
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());
	// ブレンドステート設定
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	// ラスタライザーステート
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// 出力先は1ターゲット
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	// デプスバッファのフォーマットを設定
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// ルートシグネチャのセット
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// マルチサンプル設定
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.SampleMask = UINT_MAX;

	hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateGraphicsPipelineState failed");
	}


}

void D3D12AppTriangle::Cleanup()
{
	auto index = m_swapchain->GetCurrentBackBufferIndex();
	auto fence = m_frameFences[index];
	auto value = ++m_frameFenceValues[index];
	m_commandQueue->Signal(fence.Get(), value);
	fence->SetEventOnCompletion(value, m_fenceWaitEvent);
	WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void D3D12AppTriangle::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
	// スワップチェイン表示可能からレンダーターゲット描画可能へ
	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	command->ResourceBarrier(1, &barrierToRT);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		m_heapRtv->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(
		m_heapDsv->GetCPUDescriptorHandleForHeapStart()
	);

	// カラーバッファ(レンダーターゲットビュー)のクリア
	const float clearColor[] = { 0.1f,0.25f,0.5f,0.0f }; // クリア色
	command->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

	// デプスバッファ(デプスステンシルビュー)のクリア
	command->ClearDepthStencilView(
		dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	command->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	// パイプラインステートのセット
	command->SetPipelineState(m_pipeline.Get());
	// ルートシグネチャのセット
	command->SetGraphicsRootSignature(m_rootSignature.Get());
	// ビューポートとシザーのセット
	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissorRect);

	// プリミティブタイプ、頂点・インデックスバッファのセット
	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	command->IASetIndexBuffer(&m_indexBufferView);

	// 描画命令の発行
	command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);

	// レンダーターゲットからスワップチェイン表示可能へ
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	command->ResourceBarrier(1, &barrierToPresent);
}

D3D12AppTriangle::ComPtr<ID3D12Resource1> D3D12AppTriangle::CreateBuffer(UINT bufferSize, const void* initialData)
{
	HRESULT hr;
	ComPtr<ID3D12Resource1> buffer;
	
	hr = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	if (SUCCEEDED(hr) && initialData != nullptr)
	{
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		hr = buffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, initialData, bufferSize);
			buffer->Unmap(0, nullptr);
		}
	}
	return buffer;
}