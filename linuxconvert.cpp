#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;

#define PORT 6051

// the linux version of the server

void writetofile(string toshit, const char* filePath)
{
    ofstream file;
    file.open(filePath);
    file << toshit;
    file.close();
}

void SendFileContents(int clientSocket, const char* filePath)
{
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file) {
        printf("Failed to open file\n");
        return;
    }

    // Get the size of the file
    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // Allocate a buffer to hold the file contents
    char* buffer = new char[fileSize];

    // Read the file contents into the buffer
    if (file.read(buffer, fileSize)) {
        // Send the file contents
        send(clientSocket, buffer, fileSize, 0);
        printf("File contents sent\n");

        // Clear the file content
        ofstream clearFile(filePath);
        clearFile.close();
    }
    else {
        printf("Failed to read file\n");
    }

    // Clean up
    delete[] buffer;
    file.close();
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char keepAlive = 0; // 1-byte keep-alive message

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Socket creation error\n");
        return -1;
    }

    // Prepare server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind server socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        printf("Socket bind failed\n");
        close(serverSocket);
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == -1) {
        printf("Socket listen failed\n");
        close(serverSocket);
        return -1;
    }

    printf("Server listening on port %d\n", PORT);

    // Accept incoming connections and handle them
    while (1) {
        // Accept a client connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen)) == -1) {
            printf("Accept connection failed\n");
            close(serverSocket);
            return -1;
        }

        printf("Client connected\n");
        char buffer[1024] = { 0 };
        int valread = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (valread > 0)
            writetofile(((string)buffer), "user.txt");
        // Keep the connection open and wait for data
        while (1) {
            // Send the keep-alive message
            send(clientSocket, &keepAlive, sizeof(keepAlive), 0);

            // Receive data from the client
            char buffer[1024] = { 0 };
            int valread = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (valread > 0) {
                // Check if the text file has been modified
                struct stat fileStat;
                if (stat("data.txt", &fileStat) == 0) {
                    static time_t previousModTime = 0;
                    time_t currentModTime = fileStat.st_mtime;

                    if (currentModTime != previousModTime) {
                        // File has been modified, send the contents
                        SendFileContents(clientSocket, "data.txt");

                        // Update the previous modification time
                        previousModTime = currentModTime;
                    }
                }
            }
            else if (valread == 0) {
                printf("Client disconnected\n");
                ofstream clearFile("user.txt");
                break;
            }
            else {
                printf("Receiving data failed\n");
                ofstream clearFile("user.txt");
                break;
            }
        }

        // Close the client socket
        close(clientSocket);
        ofstream clearFile("user.txt");
    }

    // Close the server socket
    close(serverSocket);
    ofstream clearFile("user.txt");

    return 0;
}

