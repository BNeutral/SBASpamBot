#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <filesystem>
#include <fstream>

#include <windows.h> //Never include windows headers before this one
#include <TlHelp32.h>
#include <WinUser.h>
#include <synchapi.h>

#ifdef _MSC_VER
 #pragma comment(lib, "user32.lib")
#endif

using namespace std;

const char* PROC_TITLE = "Steambirds Alliance";
const string NAMES_PATH("./names");

void addEnter(vector<INPUT>* vec) {
    INPUT input;
    WORD vkey = VK_RETURN;
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = vkey;
    input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
    vec->push_back(input);

    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    vec->push_back(input);
}

void sendEnter() {
    INPUT input;
    WORD vkey = VK_RETURN; // see link below
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = vkey;
    input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void SendInputsToGame(const string &line)
{
    string giveLine("/give ");
    giveLine += line;
    vector<INPUT> vec;
    for(auto ch : giveLine)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        input.ki.wScan = ch;
        vec.push_back(input);

        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        vec.push_back(input);
    }
    addEnter(&vec);
    sendEnter();
    Sleep(100);
    SendInput(vec.size(), vec.data(), sizeof(INPUT));
    Sleep(800);
}

vector<string> FetchFiles() {
    vector<string> names;
    for (const auto & entry : experimental::filesystem::directory_iterator(NAMES_PATH))
        names.push_back(entry.path().string());
    return names;
}

vector<string> ReadLines(string filePath) {
    ifstream file(filePath.c_str());
    vector<string> result;
	if(!file)
		cout << "Cannot open the File : "<< filePath << endl;
 	string str;
	while (getline(file, str))
		if(str.size() > 0)
			result.push_back(str);
	file.close();
	return result;
}

void InputFromFile(string filePath, int repeats, bool useItem) {
    vector<string> lines = ReadLines(filePath);
    HWND handle = FindWindowA(NULL, PROC_TITLE);
    if (handle) {
        SetForegroundWindow(handle);
        for(auto &line : lines){
             SendInputsToGame(line);
        }
    }
    else {
        cout << "No window found!" << endl;
    }   
}

int main(int argc, char** argv) {
    vector<string> files = FetchFiles();
    InputFromFile(files[0], 1, false);
    /*for (int i = 0; i < 10; ++i) {
        Sleep(800);
        SendInputsToGame();
    }*/
    cout << "Done" << endl;
}
