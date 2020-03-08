
#include "D3D12AppOffscreen.h"
#include <stdexcept>

void D3D12AppOffscreen::Prepare()
{

	HRESULT hr;
	// レンダーターゲット用リソースを作成
	{
		auto offscreenBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_viewportWidth,
			m_viewportHeight,
			1, 0,
			1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

		hr = m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&offscreenBufferDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			nullptr,
			IID_PPV_ARGS(&m_offsecreenBuffer)
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed CreateCommittedResource(OffscreenBuffer)");
		}

	}

	// レンダーターゲット用のディスクリプタ(RTV),ディスクリプタヒープ作成
	{
		// ディスクリプタヒープ
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
		};
		hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_offscreenRtv));
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed CreateDescriptorHeap(RTV)");
		}

		// ディスクリプタヒープの先頭のディスクリプタとリソースを紐付ける
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_offscreenRtv->GetCPUDescriptorHandleForHeapStart());
		m_device->CreateRenderTargetView(m_offsecreenBuffer.Get(), nullptr, rtvHandle);
	}


	// Vertex/IndexバッファをTriangle/Cubeそれぞれ作成
	{
		Vertex triangleVertices[] = {
		{ { 0.0f, 0.25f, 0.5f },{ 1.0f, 0.0f,0.0f,1.0f }, {0.0f, 0.0f} },
	{ { 0.25f,-0.25f, 0.5f },{ 0.0f, 1.0f,0.0f,1.0f },  {0.0f, 0.0f} },
	{ { -0.25f,-0.25f, 0.5f },{ 0.0f, 0.0f,1.0f,1.0f },  {0.0f, 0.0f} },
		};
		uint32_t triangleIndices[] = { 0, 1, 2 };

		const float k = 1.0f;
		const DirectX::XMFLOAT4 red(1.0f, 0.0f, 0.0f, 1.0f);
		const DirectX::XMFLOAT4 green(0.0f, 1.0f, 0.0f, 1.0f);
		const DirectX::XMFLOAT4 blue(0.0f, 0.0f, 1.0f, 1.0);
		const DirectX::XMFLOAT4 white(1.0f, 1.0f, 1.0f, 1.0f);
		const DirectX::XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
		const DirectX::XMFLOAT4 yellow(1.0f, 1.0f, 0.0f, 1.0f);
		const DirectX::XMFLOAT4 magenta(1.0f, 0.0f, 1.0f, 1.0f);
		const DirectX::XMFLOAT4 cyan(0.0f, 1.0f, 1.0f, 1.0f);

		Vertex cubeVertices[] = {
			// 正面
			{ { -k,-k,-k }, red,{ 0.0f, 1.0f } },
			{ { -k, k,-k }, yellow,{ 0.0f, 0.0f } },
			{ { k, k,-k }, white,{ 1.0f, 0.0f } },
			{ { k,-k,-k }, magenta,{ 1.0f, 1.0f } },
			// 右
			{ { k,-k,-k }, magenta,{ 0.0f, 1.0f } },
			{ { k, k,-k }, white,{ 0.0f, 0.0f } },
			{ { k, k, k }, cyan,{ 1.0f, 0.0f } },
			{ { k,-k, k }, blue,{ 1.0f, 1.0f } },
			// 左
			{ { -k,-k, k }, black,{ 0.0f, 1.0f } },
			{ { -k, k, k }, green,{ 0.0f, 0.0f } },
			{ { -k, k,-k }, yellow,{ 1.0f, 0.0f } },
			{ { -k,-k,-k }, red,{ 1.0f, 1.0f } },
			// 裏
			{ { k,-k, k }, blue,{ 0.0f, 1.0f } },
			{ { k, k, k }, cyan,{ 0.0f, 0.0f } },
			{ { -k, k, k }, green,{ 1.0f, 0.0f } },
			{ { -k,-k, k }, black,{ 1.0f, 1.0f } },
			// 上
			{ { -k, k,-k }, yellow,{ 0.0f, 1.0f } },
			{ { -k, k, k }, green,{ 0.0f, 0.0f } },
			{ { k, k, k }, cyan,{ 1.0f, 0.0f } },
			{ { k, k,-k }, white,{ 1.0f, 1.0f } },
			// 底
			{ { -k,-k, k }, red,{ 0.0f, 1.0f } },
			{ { -k,-k,-k }, red,{ 0.0f, 0.0f } },
			{ { k,-k,-k }, magenta,{ 1.0f, 0.0f } },
			{ { k,-k, k }, blue,{ 1.0f, 1.0f } },
		};
		uint32_t cubeIndices[] = {
			0, 1, 2, 2, 3,0,
			4, 5, 6, 6, 7,4,
			8, 9, 10, 10, 11, 8,
			12,13,14, 14,15,12,
			16,17,18, 18,19,16,
			20,21,22, 22,23,20,
		};

		// 頂点バッファとインデックスバッファの生成.
		m_triangleVertexBuffer = CreateBuffer(sizeof(triangleVertices), triangleVertices);
		m_triangleIndexBuffer = CreateBuffer(sizeof(triangleIndices), triangleIndices);
		m_triangleIndexCount = _countof(triangleIndices);

		// vertex/index bufferviewの生成.
		m_triangleVertexBufferView.BufferLocation = m_triangleVertexBuffer->GetGPUVirtualAddress();
		m_triangleVertexBufferView.SizeInBytes = sizeof(triangleVertices);
		m_triangleVertexBufferView.StrideInBytes = sizeof(Vertex);

		m_triangleIndexBufferView.BufferLocation = m_triangleIndexBuffer->GetGPUVirtualAddress();
		m_triangleIndexBufferView.SizeInBytes = sizeof(triangleIndices);
		m_triangleIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		m_cubeVertexBuffer = CreateBuffer(sizeof(cubeVertices), cubeVertices);
		m_cubeIndexBuffer = CreateBuffer(sizeof(cubeIndices), cubeIndices);
		m_cubeIndexCount = _countof(cubeIndices);

		m_cubeVertexBufferView.BufferLocation = m_cubeVertexBuffer->GetGPUVirtualAddress();
		m_cubeVertexBufferView.SizeInBytes = sizeof(cubeVertices);
		m_cubeVertexBufferView.StrideInBytes = sizeof(Vertex);

		m_cubeIndexBufferView.BufferLocation = m_cubeIndexBuffer->GetGPUVirtualAddress();
		m_cubeIndexBufferView.SizeInBytes = sizeof(cubeIndices);
		m_cubeIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	}


	// シェーダーをコンパイル.
	{
		ComPtr<ID3DBlob> errBlob;
		hr = CompileShaderFromFile(L"triangleVs.hlsl", L"vs_6_0", m_offscreenVs, errBlob);
		if (FAILED(hr))
		{
			OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		}
		hr = CompileShaderFromFile(L"trianglePs.hlsl", L"ps_6_0", m_offscreenPs, errBlob);
		if (FAILED(hr))
		{
			OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		}
		hr = CompileShaderFromFile(L"textureVS.hlsl", L"vs_6_0", m_drawVs, errBlob);
		if (FAILED(hr))
		{
			OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		}
		hr = CompileShaderFromFile(L"texturePS.hlsl", L"ps_6_0", m_drawPs, errBlob);
		if (FAILED(hr))
		{
			OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		}
	}


	// ルートシグネチャの構築
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
		rootSigDesc.Init(
			0, nullptr,
			0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);
		ComPtr<ID3D10Blob> signature;
		ComPtr<ID3DBlob> errBlob;
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
			IID_PPV_ARGS(&m_offscreenRootSignature)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("CrateRootSignature failed.");
		}
	}


	// オフスクリーン描画のパイプラインステートを作成
	{
		// インプットレイアウト
		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, offsetof(Vertex,Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		};

		// パイプラインステートオブジェクトの生成.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

		// シェーダーのセット
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_offscreenVs.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_offscreenPs.Get());
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
		psoDesc.pRootSignature = m_offscreenRootSignature.Get();
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// マルチサンプル設定
		psoDesc.SampleDesc = { 1, 0 };
		psoDesc.SampleMask = UINT_MAX;

		hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_offscreenPipeline));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateGraphicsPipelineState failed");
		}
	}



	// キューブ描画用のディスクリプタヒープを作成
	{
		// CBV/SRV用のディスクリプタヒープ
		// 0: svb 1,2: cbv (frameの数だけ)
		UINT count = FrameBufferCount + 1;
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			count,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};

		m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_heapSrvCbv));
		m_srvcbvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// ダイナミックサンプラーのディスクリプタヒープ
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_heapSampler));
		m_samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	};


	// キューブ描画用の定数バッファ/ビューを作成
	{
		m_constantBuffers.resize(FrameBufferCount);
		m_cbViews.resize(FrameBufferCount);

		for (UINT i = 0; i < FrameBufferCount; ++i)
		{
			UINT bufferSize = sizeof(ShaderParameters) + 255 & ~255;
			m_constantBuffers[i] = CreateBuffer(bufferSize, nullptr);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
			cbDesc.BufferLocation = m_constantBuffers[i]->GetGPUVirtualAddress();
			cbDesc.SizeInBytes = bufferSize;

			// 一つうえで作成したcbv用のヒープの1からFrameBufferCount分をつかってCBVを作成
			CD3DX12_CPU_DESCRIPTOR_HANDLE handleCBV(
				m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(),
				ConstantBufferDescriptorBase + i,
				m_srvcbvDescriptorSize
			);
			m_device->CreateConstantBufferView(&cbDesc, handleCBV);

			m_cbViews[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(),
				ConstantBufferDescriptorBase + i,
				m_srvcbvDescriptorSize);
		}
	}

	// キューブ描画用のサンプラーを作成
	{
		D3D12_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(
			D3D12_FILTER_TYPE_LINEAR, // min
			D3D12_FILTER_TYPE_LINEAR, // mag
			D3D12_FILTER_TYPE_LINEAR, // mip
			D3D12_FILTER_REDUCTION_TYPE_STANDARD);
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MaxLOD = FLT_MAX;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

		// サンプラー用ディスクリプタヒープの0番目を使用する
		auto descriptorSampler = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_heapSampler->GetCPUDescriptorHandleForHeapStart(),
			SamplerDescriptorBase,
			m_samplerDescriptorSize);
		m_device->CreateSampler(&samplerDesc, descriptorSampler);
		m_sampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_heapSampler->GetGPUDescriptorHandleForHeapStart(),
			SamplerDescriptorBase,
			m_samplerDescriptorSize);
	}

	// キューブテクスチャ用のシェーダーリソースビューを作成
	{
		auto srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(),
			TextureSrvDescriptorBase,
			m_srvcbvDescriptorSize
		);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_device->CreateShaderResourceView(m_offsecreenBuffer.Get(), &srvDesc, srvHandle);
		m_srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(),
			TextureSrvDescriptorBase,
			m_srvcbvDescriptorSize);
	}
	



	// キューブ描画用のディスクリプタテーブルを構築
	CD3DX12_ROOT_PARAMETER rootParams[3];
	{
		CD3DX12_DESCRIPTOR_RANGE cbv, srv, sampler;
		cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 レジスタ
		srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ
		sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // s0 レジスタ

		rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParams[1].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParams[2].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

	}
	
	// キューブ描画用のルートシグネチャを構築
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
		rootSigDesc.Init(
			_countof(rootParams), rootParams,
			0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		ComPtr<ID3D10Blob>signature;
		ComPtr<ID3DBlob> errBlob;
		hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
		if (FAILED(hr))
		{
			throw std::runtime_error("D3D12SerializeRootSignature faild.");
		}
		// RootSignature の生成
		hr = m_device->CreateRootSignature(
			0,
			signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&m_drawRootSignature)
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("CrateRootSignature failed.");
		}
	}
	
	// 描画のパイプラインステートを作成
	{
		// インプットレイアウト
		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, offsetof(Vertex,Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex,UV), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		};

		// パイプラインステートオブジェクトの生成.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

		// シェーダーのセット
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_drawVs.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_drawPs.Get());
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
		psoDesc.pRootSignature = m_drawRootSignature.Get();
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// マルチサンプル設定
		psoDesc.SampleDesc = { 1, 0 };
		psoDesc.SampleMask = UINT_MAX;

		hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_drawPipeline));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateGraphicsPipelineState failed");
		}
	}

	



}

void D3D12AppOffscreen::Cleanup()
{
	auto index = m_swapchain->GetCurrentBackBufferIndex();
	auto fence = m_frameFences[index];
	auto value = ++m_frameFenceValues[index];
	m_commandQueue->Signal(fence.Get(), value);
	fence->SetEventOnCompletion(value, m_fenceWaitEvent);
	WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void D3D12AppOffscreen::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
	// スワップチェイン表示可能からレンダーターゲット描画可能へ
	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	command->ResourceBarrier(1, &barrierToRT);

	// ピクセルシェーダーリソースからレンダーターゲットへ
	auto barrierToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_offsecreenBuffer.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	command->ResourceBarrier(1, &barrierToRenderTarget);

	// render pass 1
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
			m_offscreenRtv->GetCPUDescriptorHandleForHeapStart(),
			0, m_rtvDescriptorSize);
		
		// 描画先をセット
		command->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
		//// パイプラインステートのセット
		command->SetPipelineState(m_offscreenPipeline.Get());
		// ルートシグネチャのセット
		command->SetGraphicsRootSignature(m_offscreenRootSignature.Get());
		// ビューポートとシザーのセット
		command->RSSetViewports(1, &m_viewport);
		command->RSSetScissorRects(1, &m_scissorRect);

		// プリミティブタイプ、頂点・インデックスバッファのセット
		command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		command->IASetVertexBuffers(0, 1, &m_triangleVertexBufferView);
		command->IASetIndexBuffer(&m_triangleIndexBufferView);

		// 描画命令の発行
		command->DrawIndexedInstanced(m_triangleIndexCount, 1, 0, 0, 0);
	}

	// レンダーターゲットからピクセルシェーダーリソースへ
	auto barrierToShaderRerouce = CD3DX12_RESOURCE_BARRIER::Transition(
		m_offsecreenBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
	command->ResourceBarrier(1, &barrierToShaderRerouce);

	// render pass 2
	{
		using namespace DirectX;

		// constant bufferのセット
		ShaderParameters shaderParams;
		XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(45.0f)));
		auto mtxView = XMMatrixLookAtLH(
			XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f),
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), m_viewport.Width / m_viewport.Height, 0.1f, 100.0f);
		XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

		// 定数バッファの更新.
		auto& constantBuffer = m_constantBuffers[m_frameIndex];
		{
			void *p;
			CD3DX12_RANGE range(0, 0);
			constantBuffer->Map(0, &range, &p);
			memcpy(p, &shaderParams, sizeof(shaderParams));
			constantBuffer->Unmap(0, nullptr);
		}


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
		command->SetPipelineState(m_drawPipeline.Get());
		// ルートシグネチャのセット
		command->SetGraphicsRootSignature(m_drawRootSignature.Get());
		// ビューポートとシザーのセット
		command->RSSetViewports(1, &m_viewport);
		command->RSSetScissorRects(1, &m_scissorRect);

		// ディスクリプタヒープをセット.
		ID3D12DescriptorHeap* heaps[] = {
			m_heapSrvCbv.Get(), m_heapSampler.Get()
		};
		command->SetDescriptorHeaps(_countof(heaps), heaps);

		// プリミティブタイプ、頂点・インデックスバッファのセット
		command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		command->IASetVertexBuffers(0, 1, &m_cubeVertexBufferView);
		command->IASetIndexBuffer(&m_cubeIndexBufferView);

		command->SetGraphicsRootDescriptorTable(0, m_cbViews[m_frameIndex]);
		command->SetGraphicsRootDescriptorTable(1, m_srv);
		command->SetGraphicsRootDescriptorTable(2, m_sampler);

		// 描画命令の発行
		command->DrawIndexedInstanced(m_cubeIndexCount, 1, 0, 0, 0);
	}
	
	// レンダーターゲットからスワップチェイン表示可能へ
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	command->ResourceBarrier(1, &barrierToPresent);
}

D3D12AppOffscreen::ComPtr<ID3D12Resource1> D3D12AppOffscreen::CreateBuffer(UINT bufferSize, const void* initialData)
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