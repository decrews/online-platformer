//--------------------------------------------------------------------------------------
// File: source.cpp
//--------------------------------------------------------------------------------------
#include "groundwork.h";
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

ID3D11SamplerState*                 g_Sampler = NULL;
ID3D11BlendState*					g_BlendState;

VS_CONSTANT_BUFFER* VsConstData = new VS_CONSTANT_BUFFER();


//--------------------------------------------------------------------------------------
// Game Variables
//--------------------------------------------------------------------------------------
// Window Size;
int screenWidth = 600;
int screenHeight = 600;

Game* game = new Game();
Level* currentLevel = new Level();
Player* player = new Player(currentLevel);


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
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
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
	VsConstData->scale = 0;
	VsConstData->some_variable_h = 0;
	
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
	default:
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

	// Blocks
	// The last argument determines block type
	// 0 = normal block
	// 1 = falling block (speed constant in Level class)
	currentLevel->blocks.push_back(new Platform(-0.3, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.0, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.1, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.2, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.3, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.4, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.5, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.6, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.7, -0.7, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(0.8, -0.7, 0.05, g_stoneBlockTex, 0));

	currentLevel->blocks.push_back(new Platform(0.9, -0.5, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(1.0, -0.5, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(1.1, -0.5, 0.05, g_stoneBlockTex, 0));

	currentLevel->blocks.push_back(new Platform(1.4, -0.5, 0.05, g_stoneBlockTex, 0));
	currentLevel->blocks.push_back(new Platform(1.5, -0.5, 0.05, g_stoneBlockTex, 0));

	currentLevel->blocks.push_back(new Platform(1.7, -0.3, 0.05, g_blockTex, 1));
	currentLevel->blocks.push_back(new Platform(1.8, -0.3, 0.05, g_blockTex, 1));

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

	// Setup vertex buffer parameters
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Clear the back buffer 
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	// Draw the current level
	currentLevel->draw(VsConstData, g_pImmediateContext, g_pVertexShader, g_pPixelShader,
		g_pConstantBuffer11, g_Sampler, g_pVertexBuffer2, stride, offset);

	// Draw the player
	player->draw(VsConstData, g_pImmediateContext, g_pVertexShader, g_pPixelShader,
		g_pConstantBuffer11, g_Sampler, g_pVertexBuffer, stride, offset);

	// Present the information rendered to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present( 0, 0 );

}
