#include "windows.h"
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#pragma warning(disable : 4996)

// Структура для хранения информации о клиенте
struct ClientInfo {
    std::string domain;
    std::string computer;
    std::string ip;
    std::string user;
    time_t lastActivity;
};



inline int GetFilePointer(HANDLE FileHandle) {
    return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}

// Класс для отслеживания клиентов
class ClientTracker {
private:
    std::map<int, ClientInfo> clients;  // Список клиентов, где ключ - идентификатор клиента

public:
    // Добавление нового клиента
    void addClient(int clientId, const std::string& domain, const std::string& computer, const std::string& ip, const std::string& user) {
        ClientInfo client;
        client.domain = domain;
        client.computer = computer;
        client.ip = ip;
        client.user = user;

        // Инициализация времени последней активности клиента
        client.lastActivity = std::time(nullptr);

        clients[clientId] = client;
    }

    // Получение времени последней активности клиента
    time_t getLastActivity(int clientId) {
        if (clients.count(clientId) > 0) {
            return clients[clientId].lastActivity;
        }

        return -1;  // Возвращаем -1, если клиент не найден
    }

    // Получение списка всех подключенных клиентов
    std::map<int, ClientInfo> getAllClients() {
        return clients;
    }
};


bool SaveBMPFile(const char * filename, HBITMAP bitmap, HDC bitmapDC, int width, int height) {
    bool Success = 0;
    HBITMAP OffscrBmp = NULL;
    HDC OffscrDC = NULL;
    LPBITMAPINFO lpbi = NULL;
    LPVOID lpvBits = NULL;
    HANDLE BmpFile = INVALID_HANDLE_VALUE;
    BITMAPFILEHEADER bmfh;
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
        return 0;
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
        return 0;
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL)
        return 0;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(OffscrDC, OldBmp);
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
        return 0;
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((BmpFile = CreateFileA(filename,
        GENERIC_WRITE,
        0, NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
        return 0;
    DWORD Written;
    bmfh.bfType = 19778;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;
    if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))
        return 0;
    if (Written < sizeof(BITMAPINFOHEADER))
        return 0;
    int PalEntries;
    if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
        PalEntries = 3;
    else PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
        (int)(1 << lpbi->bmiHeader.biBitCount) : 0;
    if (lpbi->bmiHeader.biClrUsed)
        PalEntries = lpbi->bmiHeader.biClrUsed;
    if (PalEntries) {
        if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
            return 0;
        if (Written < PalEntries * sizeof(RGBQUAD))
            return 0;
    }
    bmfh.bfOffBits = GetFilePointer(BmpFile);
    if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
        return 0;
    if (Written < lpbi->bmiHeader.biSizeImage)
        return 0;
    bmfh.bfSize = GetFilePointer(BmpFile);
    SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;


    CloseHandle(BmpFile);

    delete[](char*)lpvBits;
    delete[] lpbi;

    DeleteDC(OffscrDC);
    DeleteObject(OffscrBmp);


    return 1;
}

// Функция для получения скриншота с рабочего стола клиента
bool ScreenCapture(int x, int y, int width, int height, const char* filename, HWND hwnd) {


    HDC hDC = GetDC(hwnd);
    HDC hDc = CreateCompatibleDC(hDC);


    HBITMAP hBmp = CreateCompatibleBitmap(hDC, width, height);


    HGDIOBJ old = SelectObject(hDc, hBmp);
    BitBlt(hDc, 0, 0, width, height, hDC, x, y, SRCCOPY);

    bool ret = SaveBMPFile(filename, hBmp, hDc, width, height);


    SelectObject(hDc, old);

    DeleteObject(hBmp);

    DeleteDC(hDc);
    ReleaseDC(hwnd, hDC);

    return ret;
}


int main() {
    //Консоль 
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    //Автозапуск 
    HKEY hKey;
    LPCSTR lpSubKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR lpValueName = "MyProgram";
    //Путь куда будет устанавливаться 
    LPCSTR lpValueData = " C:\\Users\\Alex\\source\\repos\\Project1\\x64\\Debug\\Project1.exe";

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (LPBYTE)lpValueData, strlen(lpValueData));
        RegCloseKey(hKey);
        std::cout << "Программа установлена" << std::endl;
    }
    else {
        std::cout << "Программа ну установлена" << std::endl;
    }

   
    ClientTracker tracker;
    // Добавляем клиентов
    tracker.addClient(1, "example.com", "Компьютер 1", "192.168.0.1", "Пользователь 1");
    tracker.addClient(2, "example.com", "Компьютер 2", "192.168.0.2", "Пользователь 2");
    tracker.addClient(3, "example.com", "Компьютер 3", "192.168.0.3", "Пользователь 3");

   
    // Получаем список всех клиентов
    std::map<int, ClientInfo> clients = tracker.getAllClients();
    for (const auto& pair : clients) {
        int clientId = pair.first;
        const ClientInfo& client = pair.second;

        std::cout << "Клиент ID: " << clientId << std::endl;
        std::cout << "Домен: " << client.domain << std::endl;
        std::cout << "Компьютер: " << client.computer << std::endl;
        std::cout << "IP-адрес: " << client.ip << std::endl;
        std::cout << "Пользователь: " << client.user << std::endl;
        std::cout << "Последняя активность: " << std::ctime(&client.lastActivity) << std::endl;
    }

    // Получаем время последней активности клиента с ID 1
    time_t lastActivity = tracker.getLastActivity(1);
    std::cout << "Время последней активности клиента с ID 1: " << std::ctime(&lastActivity) << std::endl;
    // Указываем путь куда сохранять 
    ScreenCapture(0, 0, 1920, 1080, ("C://Users//Alex//Desktop//screen//scr.bmp"), NULL);
    return 0;
}