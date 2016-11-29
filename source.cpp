//--------------------------------------------------------------------------------------
// File: source.cpp
//--------------------------------------------------------------------------------------
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include "groundwork.h"
#include "Server.h"
#include "Pawn.h"
#include "Game.h"
#include "Rect.h"
#include "Player.h"
#include "Level.h"
#include "Platform.h"
#include "STRUCTS.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct sendMovement {
	float x;
	float y;
	int plrId;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;  // Cropped vertex buffer for character
ID3D11Buffer*           g_pVertexBuffer2 = NULL; // "Square" vertex buffer for blocks
ID3D11Buffer*			g_pConstantBuffer11 = NULL;

// Textures
ID3D11ShaderResourceView*           g_playerTex = NULL;
ID3D11ShaderResourceView*           g_bgTex = NULL;
ID3D11ShaderResourceView*           g_blockTex = NULL;
ID3D11ShaderResourceView*           g_stoneBlockTex = NULL;
ID3D11ShaderResourceView*           g_stoneWall = NULL;
ID3D11ShaderResourceView*           g_stoneWall2 = NULL;
ID3D11ShaderResourceView*           g_ground = NULL;
ID3D11ShaderResourceView*           g_corner = NULL;
ID3D11ShaderResourceView*           g_corner2 = NULL;
ID3D11ShaderResourceView*           g_checkPoint = NULL;
ID3D11ShaderResourceView*           g_spike = NULL;
ID3D11ShaderResourceView*           g_spike2 = NULL;
ID3D11ShaderResourceView*           g_spike3 = NULL;
ID3D11ShaderResourceView*           g_spike4 = NULL;
ID3D11ShaderResourceView*           g_otherSprite = NULL;

ID3D11SamplerState*                 g_Sampler = NULL;
ID3D11BlendState*					g_BlendState;

VS_CONSTANT_BUFFER* VsConstData = new VS_CONSTANT_BUFFER();

SOCKET Connection;
bool serverRunning = true;    // Enabled host - Single Player
bool broadcastServer = false; // If server is enabled, broadcast or local?
std::string Ip = "127.0.0.1"; // For client mode
Server* gameServer = NULL;


//--------------------------------------------------------------------------------------
// Game Variables
//--------------------------------------------------------------------------------------
// Window Size;
int screenWidth = 600;
int screenHeight = 600;
float screenRatio = screenWidth/screenHeight;

Game* game = new Game();
Level* currentLevel = new Level();
//Level* test = new Level();
Player* player = new Player(currentLevel);
std::vector<Pawn*> pawns;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();
void InitGame();



//--------------------------------------------------------------------------------------
// Client Thread Function + Process Packet
//--------------------------------------------------------------------------------------
bool processPacket(Packet packetType) {
	sendMovement pkt;

	switch (packetType) {
	case P_Movement:
	{
		recv(Connection, (char*)&pkt, sizeof(pkt), NULL);
		if (pawns.size() > 0) {
			for (int i = 0; i < pawns.size(); i++) {
				if (pawns[i]->id == pkt.plrId) {
					pawns[i]->xPos = pkt.x - currentLevel->levelPosition;
					pawns[i]->yPos = pkt.y;
					pawns[i]->rect->x = pkt.x - currentLevel->levelPosition;
					pawns[i]->rect->y = pkt.y;
				}
				/*
				// In the array, they have 0,1,2,3...
				// the test packet coming in is 10
				int testInt = test.plrId;
				std::wstring test = std::to_wstring(testInt);
				test += L"\n";
				OutputDebugString(test.c_str());
				*/
			}
		}


		break;
	}

	case P_Connection:
		int cntId;
		recv(Connection, (char*)&cntId, sizeof(cntId), NULL);
		pawns.push_back(new Pawn(0.5, 0.5, g_otherSprite, cntId));

		OutputDebugString(L"New player connected.");
		break;

	default:
		OutputDebugString(L"Unrecognized packet.");
	}
	return true;
}

void ClientThread() {
	Packet packetType;
	while (true) {
		recv(Connection, (char*)&packetType, sizeof(Packet), NULL);
		if (!processPacket(packetType)) {
			break;
		}
	}
}



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	if (serverRunning) {
		gameServer = new Server(1111, broadcastServer);
		gameServer->start();
	}


	// Client Starts
	// --------------------------------
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	SOCKADDR_IN addr; // Address that we will bind our listening socket to
	int addrlen = sizeof(addr); // Length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr(Ip.c_str());
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL); // Set connection socket
	if (connect(Connection, (SOCKADDR*)&addr, addrlen) != 0) { // If we are unable to connect
		MessageBoxA(NULL, "Faied to Connect", "Error", MB_OK | MB_ICONERROR);
		return 0; // Failed to connect;
	}

	std::cout << "Connected" << std::endl;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL); // Create thead to handle this client.  The index in the socket array for this thread is the value(i);
																					  // Tell other players about connection:
	Packet connectionPacket = P_Connection;
	int cntId = 10;
	send(Connection, (char*)&connectionPacket, sizeof(Packet), NULL); //Send test packet
	send(Connection, (char*)&cntId, sizeof(cntId), NULL); //send movement data
	// --------------------------------
	// Client Ends


    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

	// Clean up the server thread.
	if (serverRunning) {
		gameServer->close();
	}

    CleanupDevice();
    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, screenWidth, screenHeight };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Homework 3",
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	screenRatio = 800 / (float)1280;

    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );


    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"shader.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"shader.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

	// Create buffer description for verticies
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    // Create vertex buffer one (player, cropped)
    SimpleVertex vertices[] =
    {
		XMFLOAT3(-0.053f, 0.08f, 0.5f),
		XMFLOAT2(0.0f, 0.0f),
		
		XMFLOAT3(0.053f, 0.08f, 0.5f),
		XMFLOAT2(player->adjustedWidth, 0.0f),
		
		XMFLOAT3(0.053f, -0.08f, 0.5f),
		XMFLOAT2(player->adjustedWidth, player->adjustedHeight),
		
		XMFLOAT3(-0.053f, 0.08f, 0.5f),
		XMFLOAT2(0.0f, 0.0f),
		
		XMFLOAT3(0.053f, -0.08f, 0.5f),
		XMFLOAT2(player->adjustedWidth, player->adjustedHeight),
		
		XMFLOAT3(-0.053f, -0.08f, 0.5f),
		XMFLOAT2(0.0f, player->adjustedHeight),
    };
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

	// Create vertex buffer two (not cropped, square)
	SimpleVertex vertices2[] =
	{
		XMFLOAT3(-1.0f, 1.0f, 0.5f),
		XMFLOAT2(0.0f, 0.0f),

		XMFLOAT3(1.0f, 1.0f, 0.5f),
		XMFLOAT2(1.0, 0.0f),

		XMFLOAT3(1.0f, -1.0f, 0.5f),
		XMFLOAT2(1.0, 1.0),

		XMFLOAT3(-1.0f, 1.0f, 0.5f),
		XMFLOAT2(0.0f, 0.0f),

		XMFLOAT3(1.0f, -1.0f, 0.5f),
		XMFLOAT2(1.0f, 1.0),

		XMFLOAT3(-1.0f, -1.0f, 0.5f),
		XMFLOAT2(0.0, 1.0),
	};
	
	D3D11_SUBRESOURCE_DATA InitData2;
	ZeroMemory(&InitData2, sizeof(InitData2));
	InitData2.pSysMem = vertices2;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData2, &g_pVertexBuffer2);
	if (FAILED(hr))
		return hr;

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );


	// Supply the vertex shader constant data.
	VsConstData->currentFrameColumn = 0;
	VsConstData->adjustedWidth = 0;
	VsConstData->currentFrameRow = 0;
	VsConstData->adjustedHeight = 0;
	VsConstData->x = 0;
	VsConstData->y = 0;
	VsConstData->scaleX = 0;
	VsConstData->scaleY = 0;
	VsConstData->extra = 0;
	VsConstData->extraTwo = 0;
	VsConstData->extraThree = 0;
	VsConstData->extraFour = 0;
	VsConstData->extraFive = 0;
	VsConstData->extraSix = 0;
	VsConstData->extraSeven = 0;
	VsConstData->extraEight = 0;
	
	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Create the buffer.
	hr = g_pd3dDevice->CreateBuffer(&cbDesc, NULL,	&g_pConstantBuffer11);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"player.png", NULL, NULL, &g_playerTex, NULL);
	if (FAILED(hr))
		return hr;
	
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"bg0.png", NULL, NULL, &g_bgTex, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"grass.png", NULL, NULL, &g_blockTex, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"stone.png", NULL, NULL, &g_stoneBlockTex, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"stoneWall.png", NULL, NULL, &g_stoneWall, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"stoneWall2.png", NULL, NULL, &g_stoneWall2, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"groundtest.png", NULL, NULL, &g_ground, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"corner.png", NULL, NULL, &g_corner, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"corner2.png", NULL, NULL, &g_corner2, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Door.jpg", NULL, NULL, &g_checkPoint, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Flag.png", NULL, NULL, &g_checkPoint, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Spike.png", NULL, NULL, &g_spike, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"SpikeLeft.png", NULL, NULL, &g_spike2, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"SpikeRight.png", NULL, NULL, &g_spike3, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"SpikeDown.png", NULL, NULL, &g_spike4, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"other_sprite.png", NULL, NULL, &g_otherSprite, NULL);
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_Sampler);
	if (FAILED(hr))
		return hr;

	// Create blend state
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	g_pd3dDevice->CreateBlendState(&blendStateDesc, &g_BlendState);

	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	g_pImmediateContext->OMSetBlendState(g_BlendState, blendFactor, sampleMask);

	
	// Setup Player, Level, etc
	InitGame();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
	if (g_pVertexBuffer2 ) g_pVertexBuffer2->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}

///////////////////////////////////
//		This Function is called every time the Left Mouse Button is down
///////////////////////////////////
void OnLBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{

}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is down
///////////////////////////////////
void OnRBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{

}
///////////////////////////////////
//		This Function is called every time a character key is pressed
///////////////////////////////////
void OnChar(HWND hwnd, UINT ch, int cRepeat)
{

}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is up
///////////////////////////////////
void OnLBU(HWND hwnd, int x, int y, UINT keyFlags)
{


}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is up
///////////////////////////////////
void OnRBU(HWND hwnd, int x, int y, UINT keyFlags)
{


}
///////////////////////////////////
//		This Function is called every time the Mouse Moves
///////////////////////////////////
void OnMM(HWND hwnd, int x, int y, UINT keyFlags)
{

	if ((keyFlags & MK_LBUTTON) == MK_LBUTTON)
	{
	}

	if ((keyFlags & MK_RBUTTON) == MK_RBUTTON)
	{
	}
}

BOOL OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	//editfont = CreateFont(-10, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, "Arial");
	if (!SetTimer(hwnd, 1, 200, NULL))
	{
		MessageBox(hwnd, L"No Timers Available", L"Info", MB_OK);
		return FALSE;
	}

	return TRUE;
}

void OnTimer(HWND hwnd, UINT id)
{
	if (player->animationActive) {
		// Increment to the next column
		player->currentFrameColumn += 1.0f;
		// Reset the column if the edge is reached and move to next row
		if (player->currentFrameColumn >= player->totalSpriteColumns) {
			player->currentFrameColumn = 0.0f;
		}
	}
}
//*************************************************************************
void OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 65: //a
		player->a_down = false;
		break;
	case 68: //d
		player->d_down = false;
		break;
	case 83: //s
		player->s_down = false;
		break;
	case 87: //w
		player->w_down = false;
		break;
	case 32: //space
		player->w_down = false;
		break;
	case 75: //k
		if (VsConstData->extra == 0) {
			VsConstData->extra = 1;
		}
		else {
			VsConstData->extra = 0;;
		}
		break;
	
	default	:
		break;
	}
}
void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 27: //esc
		PostQuitMessage(0);
		break;
	case 65: //a
		player->a_down = true;
		break;
	case 68: //d
		player->d_down = true;
		break;
	case 83: //s
		player->s_down = true;
		break;
	case 87: //w
		player->w_down = true;
		break;
	case 32: //space
		player->w_down = true;
		break;
	default:
		break;
	}
}
//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	SCROLLINFO si;

	switch (message)
	{
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLBD);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLBU);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMM);
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
		HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);
		HANDLE_MSG(hwnd, WM_KEYUP, OnKeyUp);

	case WM_ERASEBKGND:
		return (LRESULT)1;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}


//--------------------------------------------------------------------------------------
// Initialize Game Variables / Build Levels
//--------------------------------------------------------------------------------------
void InitGame() {
	game->player = player;
	player->tex = g_playerTex;
	currentLevel->bgTex = g_bgTex;
	currentLevel->levelPosChange = -0.6;


	// Blocks
	// The last argument determines block type
	// 0 = normal block
	// 1 = falling block (speed constant in Level class)
	// 2 = pushable blocks

	//center of the universe? (-0.2, -0.6)
	
	//TEST PUSH BLOCK
	currentLevel->blocks.push_back(new Platform(0.5, -0.5, 0.05, g_stoneBlockTex, 2));

	//giant wall to the left
	for (float i = -0.1; i < 1.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(-0.9, i, 0.05, g_stoneWall, 0));
	}
	for (float i = -1.0; i < -0.1; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(-0.9, i, 0.05, g_ground, 0));
	}
	for (float i = -1.0; i < 1.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(-1.0, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.1, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.2, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.3, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.4, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.5, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.6, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.7, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-1.8, i, 0.05, g_ground, 0));
	}

	//upper layer of ground
	currentLevel->blocks.push_back(new Platform(-0.8, -0.2, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(-0.7, -0.2, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(-0.6, -0.2, 0.05, g_stoneBlockTex, 0));

	//lower layers of ground
	for (float i = -1.0; i < -0.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(-0.8, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-0.7, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-0.6, i, 0.05, g_ground, 0));

	}

	//details on the first wall after spawn
	currentLevel->blocks.push_back(new Platform(-0.5, -0.2, 0.05, g_corner2, 0));
	currentLevel->blocks.push_back(new Platform(-0.5, -0.3, 0.05, g_stoneWall, 0));
	currentLevel->blocks.push_back(new Platform(-0.5, -0.4, 0.05, g_stoneWall, 0));
	currentLevel->blocks.push_back(new Platform(-0.5, -0.5, 0.05, g_stoneWall, 0));

	for (float i = -0.8; i < -0.5; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(-0.5, i, 0.05, g_stoneWall, 0));
		currentLevel->blocks.push_back(new Platform(-0.5, -1.0, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(-0.5, -0.9, 0.05, g_ground, 0));
	}

	//long stretch after start
	for (float i = -0.4; i <= 4.5; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.9, 0.05, g_stoneBlockTex, 0));
		currentLevel->blocks.push_back(new Platform(i, -1.0, 0.05, g_ground, 0));
	}

	//floating platforms
	for (float i = 0.4; i <= 2.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.6, 0.05, g_stoneBlockTex, 0));
	}

	//roadblock
	for (float i = -0.8; i <= -0.3; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(2.0, i, 0.05, g_stoneWall2, 0));
		currentLevel->blocks.push_back(new Platform(2.1, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(2.2, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(2.3, i, 0.05, g_stoneWall, 0));

		//details on top 
		currentLevel->blocks.push_back(new Platform(2.0, -0.3, 0.05, g_corner, 0));
		currentLevel->blocks.push_back(new Platform(2.1, -0.3, 0.05, g_stoneBlockTex, 0));
		currentLevel->blocks.push_back(new Platform(2.2, -0.3, 0.05, g_stoneBlockTex, 0));
		currentLevel->blocks.push_back(new Platform(2.3, -0.3, 0.05, g_corner2, 0));
	}


	//jump blocks
	//currentLevel->blocks.push_back(new Platform(2.6, 0.0, 0.05, g_stoneBlockTex, 1));
	currentLevel->blocks.push_back(new Platform(2.7, 0.0, 0.05, g_stoneBlockTex, 1));

	currentLevel->blocks.push_back(new Platform(3.1, 0.3, 0.05, g_stoneBlockTex, 1));
	//currentLevel->blocks.push_back(new Platform(3.2, 0.3, 0.05, g_stoneBlockTex, 1));

	currentLevel->blocks.push_back(new Platform(3.5, 0.6, 0.05, g_stoneBlockTex, 1));
	//currentLevel->blocks.push_back(new Platform(3.7, 0.6, 0.05, g_stoneBlockTex, 1));

	//floating platform
	for (float i = 4.1; i < 5.4; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, 0.7, 0.05, g_stoneBlockTex, 1));
	}

	for (float i = 5.5; i < 7.5; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, 0.7, 0.05, g_stoneBlockTex, 0));
	}

	//platform before jump fest
	for (float i = -1.0; i < -0.7; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(4.7, i, 0.05, g_ground, 0));
	}

	currentLevel->blocks.push_back(new Platform(4.6, -1.0, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(4.6, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(4.6, -0.8, 0.05, g_stoneWall2, 0));
	currentLevel->blocks.push_back(new Platform(4.6, -0.7, 0.05, g_corner, 0));
	currentLevel->blocks.push_back(new Platform(4.7, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(4.8, -0.6, 0.05, g_corner, 0));
	currentLevel->blocks.push_back(new Platform(4.9, -0.6, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(5.0, -0.5, 0.05, g_corner, 0));
	currentLevel->blocks.push_back(new Platform(5.1, -0.5, 0.05, g_corner2, 0));


	for (float i = -1.0; i < -0.6; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(4.8, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(4.9, i, 0.05, g_ground, 0));
	}
	for (float i = -1.0; i < -0.5; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(5.0, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(5.1, i, 0.05, g_stoneWall, 0));
	}

	//jump fest
	currentLevel->blocks.push_back(new Platform(5.7, -1.0, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(5.7, -0.5, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(5.7, 0.0, 0.05, g_ground, 0));

	currentLevel->blocks.push_back(new Platform(6.1, -0.8, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(6.1, -0.3, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(6.5, -1.0, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(6.5, -0.5, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(6.5, 0.0, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(6.9, -0.8, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(6.9, -0.3, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(7.3, -1.0, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(7.3, -0.5, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(7.3, 0.0, 0.05, g_ground, 0));

	for (float i = 7.8; i < 8.5; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, 0.0, 0.05, g_ground, 0));
	}

	//first check point
	currentLevel->doors.push_back(new Door(10.0, -0.26, 0.09, g_checkPoint));

	//lower stairs after checkpoint
	currentLevel->blocks.push_back(new Platform(8.9, 0.3, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(8.5, -0.1, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(8.6, -0.2, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(8.7, -0.3, 0.05, g_ground, 0));

	for (float i = 9.3; i < 13.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, 0.0, 0.05, g_ground, 0));
	}

	for (float i = 8.8; i <= 10.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.4, 0.05, g_ground, 0));
	}

	currentLevel->blocks.push_back(new Platform(10.2, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.3, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.4, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.5, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.6, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.7, -0.4, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.8, -0.4, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(10.9, -0.4, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.0, -0.4, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.0, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.0, -0.2, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.0, -0.1, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.0, 0.0, 0.05, g_ground, 0));

	for (float i = -1.0; i < -0.4; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(10.0, i, 0.05, g_ground, 0));
	}

	//side spikes
	currentLevel->blocks.push_back(new Platform(11.1, -0.4, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.1, -0.1, 0.05, g_ground, 0));
	currentLevel->spikes.push_back(new Spike(11.09, -0.2, 0.05, g_spike3, 0));
	currentLevel->spikes.push_back(new Spike(11.09, -0.3, 0.05, g_spike3, 0));

	currentLevel->blocks.push_back(new Platform(10.9, -0.1, 0.05, g_ground, 0));
	currentLevel->spikes.push_back(new Spike(10.91, -0.2, 0.05, g_spike2, 0));
	currentLevel->spikes.push_back(new Spike(10.91, -0.3, 0.05, g_spike2, 0));

	for (float i = 11.2; i < 12.9; i += 0.1)
	{
		currentLevel->spikes.push_back(new Spike(i, -0.09, 0.05, g_spike4, 0));
	}
	currentLevel->blocks.push_back(new Platform(12.9, -0.1, 0.05, g_ground, 0));

	//bottom spike layer
	for (float i = 10.5; i <= 14.0; i += 0.1)
	{
		currentLevel->spikes.push_back(new Spike(i, -1.0, 0.05, g_spike, 0));
	}

	currentLevel->blocks.push_back(new Platform(10.1, -1.0, 0.05, g_ground, 0));

	//details on the side spikes
	for (float i = -0.9; i < -0.4; i += 0.1)
	{
		currentLevel->spikes.push_back(new Spike(10.09, i, 0.05, g_spike3, 0));
	}

	currentLevel->spikes.push_back(new Spike(10.2, -1.0, 0.05, g_spike, 0));
	currentLevel->spikes.push_back(new Spike(10.3, -1.0, 0.05, g_spike, 0));
	currentLevel->spikes.push_back(new Spike(10.4, -1.0, 0.05, g_spike, 0));

	currentLevel->blocks.push_back(new Platform(10.5, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.6, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.7, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.8, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.9, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(11.0, -0.9, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(11.1, -0.9, 0.05, g_ground, 1));

	/*currentLevel->blocks.push_back(new Platform(11.2, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.2, -0.8, 0.05, g_ground, 0));*/
	currentLevel->blocks.push_back(new Platform(11.2, -0.7, 0.05, g_ground, 0));

	//upper floating
	currentLevel->blocks.push_back(new Platform(11.5, -0.4, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.6, -0.4, 0.05, g_ground, 0));
	for (float i = 11.7; i < 13.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.4, 0.05, g_ground, 0));
	}

	for (float i = -0.4; i < 0.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(13.0, i, 0.05, g_ground, 0));
	}

	currentLevel->spikes.push_back(new Spike(12.91, -0.2, 0.05, g_spike2, 0));
	currentLevel->spikes.push_back(new Spike(12.91, -0.3, 0.05, g_spike2, 0));

	//lower floating
	currentLevel->blocks.push_back(new Platform(11.5, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.6, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(11.7, -0.9, 0.05, g_ground, 0));

	currentLevel->blocks.push_back(new Platform(12.2, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(12.3, -0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(12.4, -0.9, 0.05, g_ground, 0));

	//lower ground below spikes
	for (float i = 10.0; i <= 14.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -1.1, 0.05, g_ground, 0));
	}

	for (float i = 12.9; i < 14.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.9, 0.05, g_ground, 0));
	}

	for (float i = -1.0; i < 0.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(14.0, i, 0.05, g_ground, 0));
	}

	for (float i = 13.3; i < 13.8; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.6, 0.05, g_ground, 0));
	}

	currentLevel->blocks.push_back(new Platform(13.1, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.2, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.3, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.7, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.8, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.9, -0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(13.5, 0.3, 0.05, g_ground, 0));

	for (float i = 14.0; i < 15.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, 0.1, 0.05, g_ground, 0));
	}

	//second check point
	currentLevel->doors.push_back(new Door(14.5, 0.24, 0.09, g_checkPoint));
	currentLevel->doors.push_back(new Door(9.4, 0.14, 0.09, g_checkPoint));

	//upper level
	//for (float i = 9.3; i < 13.0; i += 0.1)
	//{
	//	currentLevel->blocks.push_back(new Platform(i, 1.0, 0.05, g_ground, 0));
	//}

	for (float i = 0.3; i < 1.1; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(9.3, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(12.9, i, 0.05, g_ground, 0));
	}
	currentLevel->blocks.push_back(new Platform(10.1, 0.1, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 0.2, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 0.3, 0.05, g_ground, 0));

	currentLevel->blocks.push_back(new Platform(10.1, 0.6, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 0.7, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 0.8, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 0.9, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.1, 1.0, 0.05, g_ground, 0));


	for (float i = 0.0; i < 0.9; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(9.7, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(12.5, i, 0.05, g_ground, 0));

	}
	currentLevel->blocks.push_back(new Platform(9.4, 0.3, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(9.6, 0.6, 0.05, g_ground, 0));

	//currentLevel->blocks.push_back(new Platform(9.8, 0.7, 0.05, g_ground, 0));
	currentLevel->blocks.push_back(new Platform(10.0, 0.3, 0.05, g_ground, 0));

	//wtf spikes
	for (float i = 0.7; i < 1.1; i += 0.1)
	{
		currentLevel->spikes.push_back(new Spike(9.39, i, 0.05, g_spike3, 0));
		currentLevel->spikes.push_back(new Spike(10.01, i, 0.05, g_spike2, 0));
	}

	currentLevel->spikes.push_back(new Spike(9.79, 0.6, 0.05, g_spike3, 0));

	currentLevel->blocks.push_back(new Platform(10.2, 0.3, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(10.5, 0.5, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(10.9, 0.2, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(10.9, 0.8, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(11.3, 0.5, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(11.7, 0.2, 0.05, g_ground, 1));
	currentLevel->blocks.push_back(new Platform(11.7, 0.8, 0.05, g_ground, 1));

	currentLevel->blocks.push_back(new Platform(12.1, 0.485, 0.05, g_ground, 1));

	currentLevel->spikes.push_back(new Spike(12.59, 0.5, 0.05, g_spike3, 0));
	currentLevel->blocks.push_back(new Platform(12.8, 0.3, 0.05, g_ground, 0));
	currentLevel->spikes.push_back(new Spike(12.81, 0.4, 0.05, g_spike2, 0));

	for (float i = 10.2; i < 12.5; i += 0.1)
	{
		currentLevel->spikes.push_back(new Spike(i, 0.09, 0.05, g_spike, 0));
	}

	// Testing spike scaling
	for (Spike* spk : currentLevel->spikes) {
		spk->scale = 0.045;
	}

	for (float i = -1.0; i < 0.2; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(14.1, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.2, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.3, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.4, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.5, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.6, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.7, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.8, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(14.9, i, 0.05, g_ground, 0));
		currentLevel->blocks.push_back(new Platform(15.0, i, 0.05, g_ground, 0));
	}


	for (float i = 15.1; i < 20.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.9, 0.05, g_stoneBlockTex, 0));
		currentLevel->blocks.push_back(new Platform(i, -1.0, 0.05, g_ground, 0));
	}

	for (float i = 20.7; i < 21.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.9, 0.05, g_stoneBlockTex, 0));
		currentLevel->blocks.push_back(new Platform(i, -1.0, 0.05, g_ground, 0));
	}

	currentLevel->blocks.push_back(new Platform(16.0, -0.8, 0.05, g_stoneBlockTex, 2));

	currentLevel->blocks.push_back(new Platform(20.8, -0.8, 0.05, g_stoneBlockTex, 2));

	for (float i = 21.1; i < 25.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(i, -0.3, 0.05, g_ground, 0));
	}

	for (float i = -0.1; i <= 1.0; i += 0.1)
	{
		currentLevel->blocks.push_back(new Platform(21.5, i, 0.05, g_ground, 0));
	}

	/*currentLevel->spikes.push_back(new Spike(12.9, -0.2, 0.05, g_spike2, 0));
	currentLevel->spikes.push_back(new Spike(12.9, -0.2, 0.05, g_spike2, 0));*/

	//test->bgTex = g_spike;
	//currentLevel = test;
	//currentLevel->blocks.push_back(new Platform(0.0, -0.5, 0.05, g_ground, 0));
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
	static StopWatchMicro_ stopwatch;
	long elapsed = stopwatch.elapse_micro();
	stopwatch.start(); //restart

	currentLevel->update(elapsed);
	player->update(elapsed);



	// Testing out server connection
	// ------------------------------------
	// Position Packet
	sendMovement test;
	test.x = currentLevel->levelPosition;
	test.y = player->rect->y;
	test.plrId = 10;
	// Packet Type
	Packet packetType = P_Movement;

	// Send out packet
	send(Connection, (char *)&packetType, sizeof(Packet), NULL);
	send(Connection, (char*)&test, sizeof(test), NULL);

	/*
	std::wstring testTwo = std::to_wstring(pawns.size());
	testTwo += L"\n";
	OutputDebugString(testTwo.c_str());
	*/

	// ------------------------------------
	// Testing out server connection


	// Setup vertex buffer parameters
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Clear the back buffer 
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	// Draw the current level
	currentLevel->draw(VsConstData, g_pImmediateContext, g_pVertexShader, g_pPixelShader,
		g_pConstantBuffer11, g_Sampler, g_pVertexBuffer2, stride, offset);

	// Draw the other players
	for (Pawn* pwn : pawns) {
		pwn->draw(VsConstData, g_pImmediateContext, g_pVertexShader, g_pPixelShader,
			g_pConstantBuffer11, g_Sampler, g_pVertexBuffer2, stride, offset);
	}

	// Draw the player
	player->draw(VsConstData, g_pImmediateContext, g_pVertexShader, g_pPixelShader,
		g_pConstantBuffer11, g_Sampler, g_pVertexBuffer, stride, offset);

	// Present the information rendered to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present( 0, 0 );

}
