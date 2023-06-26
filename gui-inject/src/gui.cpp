#include "gui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include <tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd, 
    UINT msg, 
    WPARAM w_param,
    LPARAM l_param
);

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
        return true;

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((w_param & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
        break;
    }

    return ::DefWindowProcW(hwnd, msg, w_param, l_param);
}

inline bool gui::CreateDeviceD3D(HWND hWnd) noexcept
{
    DXGI_SWAP_CHAIN_DESC sd;

    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;

    const D3D_FEATURE_LEVEL featureLevelArray[2] = { 
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        createDeviceFlags,
        featureLevelArray,
        2, 
        D3D11_SDK_VERSION, 
        &sd, 
        &swap_chain,
        &device,
        &featureLevel,
        &device_context
    );
   
    if (res == DXGI_ERROR_UNSUPPORTED) 
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, 
            D3D_DRIVER_TYPE_WARP, 
            nullptr, 
            createDeviceFlags,
            featureLevelArray,
            2, 
            D3D11_SDK_VERSION,
            &sd,
            &swap_chain,
            &device,
            &featureLevel,
            &device_context
        );
   
    if (res != S_OK)
        return false;

    CreateRenderTarget();

    return true;
}

void gui::CleanupDeviceD3D() noexcept
{
    CleanupRenderTarget();

    if (swap_chain) 
    { 
        swap_chain->Release();
        swap_chain = nullptr;
    }

    if (device_context)
    {
        device_context->Release();
        device_context = nullptr;
    }

    if (device)
    { 
        device->Release(); 
        device = nullptr; 
    }
}

void gui::CreateRenderTarget() noexcept
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    if (pBackBuffer == nullptr)
    {
        MessageBox(NULL, "pBackBuffer is 0", "no back buffer is loaded", NULL);
    }
    else
    {
        device->CreateRenderTargetView(pBackBuffer, nullptr, &render_target_view);
        pBackBuffer->Release();
    }
}

void gui::CleanupRenderTarget() noexcept
{
    if (render_target_view)
    { 
        render_target_view->Release();
        render_target_view = nullptr;
    }
}

void gui::CreateHWindow() noexcept
{
    wc = {
       sizeof(wc),
       CS_CLASSDC,
       WndProc,
       0L,
       0L,
       GetModuleHandle(nullptr),
       nullptr,
       nullptr,
       nullptr,
       nullptr,
       L"Nindora Launchpad",
       nullptr
    };

    ::RegisterClassExW(&wc);

    hwnd = ::CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        wc.lpszClassName,
        L"Nindora Launchpad",
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        550,
        350,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
}

void gui::CreateImGui() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;     

    ImGui::StyleColorsNew();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, device_context);
}

void gui::Render() noexcept
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (isRunning)
    {
        MSG msg{};

        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                isRunning = !isRunning;
                return;
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiContent();

        ImGui::Render();

        const float clear_color_with_alpha[4] = {
            clear_color.x * clear_color.w, 
            clear_color.y * clear_color.w, 
            clear_color.z * clear_color.w, 
            clear_color.w 
        };

        device_context->OMSetRenderTargets(1, &render_target_view, nullptr);

        device_context->ClearRenderTargetView(render_target_view, clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1, 0);
    }
}

void gui::ImGuiContent() noexcept
{
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ 550,350 });
    ImGui::Begin(
        "Nindora Launchpad",
        &isRunning,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar
    );

    ImGuiIO& io = ImGui::GetIO();
    float fps = io.Framerate;
    ImGui::Text("FPS: %.1f", fps);

    ImGui::Button("subscribe");

    ImGui::End();
}

void gui::cleanRender() noexcept
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}