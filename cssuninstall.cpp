#include <windows.h>
#include <shlobj.h>
#include <string>
#include <iostream>

const char* cssExePath = R"(C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Source\hl2.exe)";
const char* appFolderName = "css_wallpaper_check";
const char* textureFileName = "missing_texture.jpg";

// Check if Counter-Strike: Source is installed
bool isCSSInstalled() {
    DWORD attr = GetFileAttributesA(cssExePath);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Set desktop wallpaper
bool setWallpaper(const std::string& imagePath) {
    return SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)imagePath.c_str(),
                                 SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) != 0;
}

// Get AppData\Roaming path
std::string getAppDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path);
    }
    return "";
}

// Make a file or directory hidden
void makeHidden(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        SetFileAttributesA(path.c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
    }
}

// Copy missing texture to hidden folder in AppData
std::string copyTextureToAppData() {
    std::string appData = getAppDataPath();
    if (appData.empty()) return "";

    std::string hiddenFolder = appData + "\\" + appFolderName;
    CreateDirectoryA(hiddenFolder.c_str(), NULL);
    makeHidden(hiddenFolder);

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir(exePath);
    size_t pos = exeDir.find_last_of("\\/");
    exeDir = (pos != std::string::npos) ? exeDir.substr(0, pos) : ".";

    std::string source = exeDir + "\\" + textureFileName;
    std::string destination = hiddenFolder + "\\" + textureFileName;

    // Copy if not already copied
    DWORD attr = GetFileAttributesA(destination.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        if (!CopyFileA(source.c_str(), destination.c_str(), FALSE)) {
            return ""; // Copy failed
        }
        makeHidden(destination);
    }

    return destination;
}

int main() {
    std::cout << "[CSS Wallpaper Helper]\n";

    std::string texturePath = copyTextureToAppData();
    if (texturePath.empty()) {
        std::cerr << "Error: Could not find or copy missing_texture.jpg.\n";
        MessageBoxA(NULL, "Missing texture file not found. Place it next to the EXE and try again.",
                    "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!isCSSInstalled()) {
        std::cout << "Counter-Strike: Source is NOT installed.\n";
        std::cout << "Setting missing texture wallpaper...\n";
        if (!setWallpaper(texturePath)) {
            std::cerr << "Failed to set wallpaper.\n";
            MessageBoxA(NULL, "Could not set wallpaper.", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        MessageBoxA(NULL,
                    "Counter-Strike: Source not found.\nMissing texture wallpaper has been applied.",
                    "CSS Wallpaper Helper",
                    MB_OK | MB_ICONINFORMATION);
    } else {
        std::cout << "Counter-Strike: Source is installed. No changes made.\n";
        MessageBoxA(NULL,
                    "CSS is installed — no missing texture needed.",
                    "CSS Wallpaper Helper",
                    MB_OK | MB_ICONINFORMATION);
    }

    return 0;
}
