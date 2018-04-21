#include "TriangleDemo.h"
#include "..\Library\Game.h"
#include "..\Library\GameException.h"
#include "..\Library\MatrixHelper.h"
#include "..\Library\ColorHelper.h"
#include "..\Library\Camera.h"
#include "..\Library\Utility.h"
#include "D3DCompiler.h"

namespace Rendering
{
	RTTI_DEFINITIONS(TriangleDemo)

		TriangleDemo::TriangleDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera),
		mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr),
		mInputLayout(nullptr), mWorldMatrix(MatrixHelper::Identity), mVertexBuffer(nullptr), mAngle(0.0f)
	{
	}

	TriangleDemo::~TriangleDemo()
	{
		ReleaseObject(mWvpVariable);
		ReleaseObject(mPass);
		ReleaseObject(mTechnique);
		ReleaseObject(mEffect);
		ReleaseObject(mInputLayout);
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
	}

	void TriangleDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Compile the shader
		UINT shaderFlags = 0;

#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		// Load in the effect (shader) file and compile it
		ID3D10Blob* compiledShader = nullptr;
		ID3D10Blob* errorMessages = nullptr;
		HRESULT hr = D3DCompileFromFile(L"Content\\Effects\\BasicEffect2.fx", nullptr, nullptr, nullptr, "fx_5_0", shaderFlags, 0, &compiledShader, &errorMessages);
		if (FAILED(hr))
		{
			const char* errorMessage = (errorMessages != nullptr ? (char*)errorMessages->GetBufferPointer() : "D3DX11CompileFromFile() failed");
			GameException ex(errorMessage, hr);
			ReleaseObject(errorMessages);

			throw ex;
		}

		// Create an effect object from the compiled shader
		hr = D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, mGame->Direct3DDevice(), &mEffect);
		if (FAILED(hr))
		{
			throw GameException("D3DX11CreateEffectFromMemory() failed.", hr);
		}

		ReleaseObject(compiledShader);

		// Look up the technique, pass, and WVP variable from the effect
		//each technique contains one or more passes
		//each pass consists of setting one or more texture stage registers of the 3D hardware
		//Often a fallback pass is provided for older hardware
		//Initial pass might render water, specular highlights, caustic textures, lighting
		//Second pass might do the above but no lighting (assuming older hardware not supporting this)
		//Thus first pass is tried, if it fails, second pass is tried
		//Variables can be passed to GPU for effect functions e.g. the WorldViewProjection (WVP) in this case
		//which is needed for the vertex shader to move vertices into the world view
		mTechnique = mEffect->GetTechniqueByName("main11");
		if (mTechnique == nullptr)
		{
			throw GameException("ID3DX11Effect::GetTechniqueByName() could not find the specified technique.", hr);
		}

		mPass = mTechnique->GetPassByName("p0");
		if (mPass == nullptr)
		{
			throw GameException("ID3DX11EffectTechnique::GetPassByName() could not find the specified pass.", hr);
		}

		ID3DX11EffectVariable* variable = mEffect->GetVariableByName("WorldViewProjection");
		ID3DX11EffectVariable* variableCam = mEffect->GetVariableByName("CameraPos");

		if (variable == nullptr)
		{
			throw GameException("ID3DX11Effect::GetVariableByName() could not find the specified variable.", hr);
		}
		//As the variable is the WVP which is a matrix, need to have it returned (cast) as a matrix type
		mWvpVariable = variable->AsMatrix();
		mWvpVariableCam = variableCam->AsVector();
		if (mWvpVariable->IsValid() == false)
		{
			throw GameException("Invalid effect variable cast.");
		}

		// Create the input layout to map the vertex data from the CPU to the GPU
		//From the pass we obtain the pipeline, signature of the pass (params), stencil etc.
		D3DX11_PASS_DESC passDesc;
		mPass->GetDesc(&passDesc);

		//Define our input elements (consists of position format and colour format for this example)
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		//Now can create the InputLayout to provide the mapping of vertex data from CPU to GPU
		if (FAILED(hr = mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout)))
		{
			throw GameException("ID3D11Device::CreateInputLayout() failed.", hr);
		}

		// Create the vertex buffer
		BasicEffectVertex vertices[] =
		{
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Green))),
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Yellow))),
			BasicEffectVertex(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::White))),
			BasicEffectVertex(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::BlueGreen))),

			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Blue))),
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Purple))),
			BasicEffectVertex(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Red))),
			BasicEffectVertex(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(reinterpret_cast<const float*>(&ColorHelper::Black))) 
		};

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(BasicEffectVertex) * ARRAYSIZE(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		//A subsource is a mip-map level
		//The vertex buffer above is Immutable thus cannot be modified
		//A subresource initialized with the vertex buffer data is instead used 
		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;

		//Create buffer will hold the mutable vertices used by the GPU
		if (FAILED(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, &mVertexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed.");
		}

		UINT indices[] =
		{
			0, 1, 2,
			0, 2, 3,

			4, 5, 6,
			4, 6, 7,

			3, 2, 5,
			3, 5, 4,

			2, 1, 6,
			2, 6, 5,

			1, 7, 6,
			1, 0, 7,

			0, 3, 4,
			0, 4, 7
		};

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * ARRAYSIZE(indices);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = indices;
		if (FAILED(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, &mIndexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed.");
		}

	}

	void TriangleDemo::Update(const GameTime& gameTime)
	{
		mAngle += XM_PI * static_cast<float>(gameTime.ElapsedGameTime());
		//	XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationZ(mAngle));
	}

	void TriangleDemo::Draw(const GameTime& gameTime)
	{

		//As in OpenCL we need a context
		//The context here is a simple primitive
		//With a context we can set our vertex buffer
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(BasicEffectVertex);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//Define the world and position (camera or eye) then set the Effect variable WVP
		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mWvpVariable->SetMatrix(reinterpret_cast<const float*>(&wvp));
		mWvpVariableCam->SetFloatVector(reinterpret_cast<const float*>(&mCamera->PositionVector()));

		//Apply the pass to all the vertices and pixels
		mPass->Apply(0, direct3DDeviceContext);

		//Draw the vertices now
		direct3DDeviceContext->DrawIndexed(36, 0,0);
	}
}