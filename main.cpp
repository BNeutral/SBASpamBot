//Note from the developer: The memory management is quite bad because I don't care
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <filesystem>
#include <fstream>

//Never include windows headers before this one
#include <windows.h> 
#include <TlHelp32.h>
#include <WinUser.h>
#include <synchapi.h>

#ifdef _MSC_VER
 #pragma comment(lib, "user32.lib")
#endif

using namespace std;

const char* PROC_TITLE = "Steambirds Alliance";
const string NAMES_PATH("./names");
const string ELEM_UPGRADE("elemUpgrades");
const string OTHER_UPGRADE("otherUpgrades");
const char* PREPEND = "/give ";
const char USE_SLOT = 0x31; //1

/**
 * Adds an enter key input to an input vector
 **/
void addEnter(vector<INPUT> &vec) {
    INPUT input;
    WORD vkey = VK_RETURN;
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = vkey;
    input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
    vec.push_back(input);

    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    vec.push_back(input);
}

/**
 * Sends a key input to whatever is on focus
 **/
void sendKey(WORD VKEY) {
    INPUT input;
    WORD vkey = VKEY;
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = vkey;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

/**
 * Adds a char key input to an input vector
 **/
void addChar(vector<INPUT> &vec, char ch) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.dwFlags = KEYEVENTF_UNICODE;
    input.ki.wScan = ch;
    vec.push_back(input);

    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    vec.push_back(input);
}

//Hack so switching focus doesn't interfer with life with 500 key inputs
void waitForFocus(HWND handle) {
    while(handle != GetForegroundWindow()) { 
        Sleep(1000);
    }
}

/**
 * Sends enter->string->enter to whatever is on focus
 * Has some small sleeps because the game has some timing issues
 **/
void sendInputsToGame(const string &line, HWND handle, bool useItem)
{
    string giveLine(PREPEND);
    giveLine += line;
    vector<INPUT> vec;
    for(auto ch : giveLine)
    {
        addChar(vec, ch);
    }
    addEnter(vec);
    waitForFocus(handle);
    sendKey(VK_RETURN);
    Sleep(100); // Seems to get stuck without this
    waitForFocus(handle);
    SendInput(vec.size(), vec.data(), sizeof(INPUT));
    Sleep(900); // Seems to get stuck without this
    if (useItem) {
        waitForFocus(handle);
        sendKey(USE_SLOT);
        Sleep(100); // Seems to get stuck without this
    }
}

/**
 * Returns any files in the names directory
 **/
vector<string> fetchFiles(const string &dir) {
    vector<string> names;
    for (const auto & entry : experimental::filesystem::directory_iterator(dir))
        names.push_back(entry.path().string());
    return names;
}

/**
 * Returns a text file as a vector of strings
 **/
vector<string> readLines(const string &filePath) {
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

/**
 * Inputs each line of the file to the game, splits them up to check for loop amounts
 **/
void inputFromFile(string filePath) {
    vector<string> lines = readLines(filePath);
    HWND handle = FindWindowA(NULL, PROC_TITLE);
    if (handle) {
        SetForegroundWindow(handle);
        for(auto &line : lines){
            int loops = 1;
            bool useItem = false;
            size_t matchPos = line.find("|");
            if (matchPos != string::npos) {
                loops = std::stoi(line.substr(matchPos+1));
                line.erase(matchPos);    
                useItem = true;                
            }
            for (int i = 0; i < loops; ++i)
                sendInputsToGame(line, handle, loops>1);
        }
    }
    else {
        cout << "No window found!" << endl;
    }   
}

/**
 * User interface
 **/
void menuLoop() {
    int select = 0;
    bool dontExit = false;
    string currentDir = NAMES_PATH;
    vector<string> previousDirs = vector<string>();
    vector<string> files = fetchFiles(currentDir);
    do {
        dontExit = false;
        int i = 1;
        cout << "0. Back" << endl;
        for(auto file : files) {
            cout << i++ << ". " << file << endl;
        }
        cin >> select;
        if (select > 0 && select <= files.size()) {
            string &filePath = files[select-1];
            if (experimental::filesystem::is_directory(filePath)) {
                previousDirs.push_back(currentDir);
                currentDir = filePath;
                files = fetchFiles(filePath);
            }
            else {
                inputFromFile(filePath);
            }
        }            
        else if (select == 0 && previousDirs.size() > 0) {
            currentDir = previousDirs.back();
            files = fetchFiles(previousDirs.back());
            previousDirs.pop_back();
            dontExit = true;
        }
    } while (select != 0 || dontExit );
}

int main(int argc, char** argv) {
    cout << "Note that when using upgrades you need your first cargo slot empty!" << endl;
    cout << "Default cmd program break on Windows is ctrl+pause." << endl;
    menuLoop();
    return 0;
}


