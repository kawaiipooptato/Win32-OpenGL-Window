#include "WinClass.h"

int main()
{
	Win32GLWindow window;
	window.Create("Test", 800, 600, false);

	while (true)
	{
		window.ProcessMessages();
	}
	return 0;
}
