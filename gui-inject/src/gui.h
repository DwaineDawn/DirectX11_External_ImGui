#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <d3d11.h>

namespace gui
{
	static inline ID3D11Device*		      device                 = nullptr;
	static inline ID3D11DeviceContext*    device_context         = nullptr;
	static inline IDXGISwapChain*	      swap_chain             = nullptr;
	static inline ID3D11RenderTargetView* render_target_view     = nullptr;

	inline WNDCLASSEXW wc = {  };
	inline HWND hwnd = nullptr;

	inline bool isRunning = true;
	inline bool CreateDeviceD3D(HWND hWnd) noexcept;

	void CleanupDeviceD3D() noexcept;
	void CreateRenderTarget() noexcept;
	void CleanupRenderTarget() noexcept;
	void CreateHWindow() noexcept;
	void CreateImGui() noexcept;
	void Render() noexcept;
	void ImGuiContent() noexcept;
	void cleanRender() noexcept;

}