#include "gui.h"

#include <exception>

int APIENTRY wWinMain(
	_In_     HINSTANCE instance,
	_In_opt_ HINSTANCE prev_instance,
	_In_	 LPWSTR    arguments, 
	_In_	 INT       command
)
{
	try
	{
		gui::CreateHWindow();
		gui::CreateImGui();
		gui::Render();
		gui::cleanRender();
	}
	catch (std::exception& error)
	{
		MessageBox(
			NULL,
			"ImGui cannot load in!",
			error.what(),
			NULL
		);

		gui::cleanRender();
		gui::CleanupRenderTarget();
		return 1;
	}

	return 0;
}