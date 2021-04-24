#include "Engine.h"
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	Engine engine(hInstance, "window class");
	engine.Initialize("window title", 800, 600, nCmdShow);

	while (engine.ProcessMessage())
	{
		Sleep(10);
	}
	return 0;
}