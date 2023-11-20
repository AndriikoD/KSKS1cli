#include <iostream>
#include <winsock2.h>
#include <clocale>
#include <windows.h>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996) 

#define BUFLEN 2048
#define PORT 2927


bool invalid_com = false;
int paint = 0;

int int_param[6];
string string_param[3];

struct Command {
    std::string name;
    std::vector<std::string> params;
};

Command ParseCommand(const std::string& commandStr) {
    Command command;

    int number;

    std::istringstream s_stream(commandStr);
    std::string get_word;
    s_stream >> command.name;

    int j = 0;
    int i = 0;
    while (s_stream >> get_word) {
        if (get_word == ":")
        {
            break;
        }
        command.name += " " + get_word;
    }

    if (command.name == "clear display") {
        invalid_com = false;
        paint = 1;
    }
    else if (command.name == "draw pixel") {
        paint = 2;
        invalid_com = false;
    }
    else if (command.name == "draw line") {
        paint = 5;
        invalid_com = false;
    }
    else if (command.name == "draw rectangle")
    {
        paint = 10;
        invalid_com = false;
    }
    else if (command.name == "fill rectangle") {
        paint = 15;
        invalid_com = false;
    }
    else if (command.name == "draw ellipse") {
        paint = 20;
        invalid_com = false;
    }
    else if (command.name == "fill ellipse")
    {
        paint = 25;
        invalid_com = false;
    }
    else {
        invalid_com = true;
    }


    if (!invalid_com)
    {
        while (s_stream >> get_word) {
            switch (paint)
            {
            case 1: case 4: case 9: case 14: case 19: case 24: case 29:
                command.params.push_back(get_word);
                try {
                    number = std::stoi(get_word);
                }
                catch (const std::invalid_argument& e) {
                    invalid_com = false;
                    return command;
                }
                invalid_com = true;
                string_param[j] = get_word;
                j++;
                if (paint != 52)
                {
                    return command;
                }

            case 2: case 3: case 5: case 6: case 7: case 8: case 10: case 11: case 12: case 13:
            case 15: case 16: case 17: case 18: case 20: case 21: case 22: case 23:
            case 25: case 26: case 27: case 28:
                command.params.push_back(get_word);
                try {
                    number = std::stoi(get_word);
                }
                catch (const std::invalid_argument& e) {
                    invalid_com = true;
                    return command;
                }
                invalid_com = false;
                paint++;
                int_param[i] = number;
                i++;
                break;
            }
        }
        invalid_com = true;
        return command;
    }
    else {
        return command;
    }
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "ru");
    sockaddr_in server, client;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        exit(0);
    }

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        exit(EXIT_FAILURE);
    }
    while (true)
    {
        fflush(stdout);
        char message[BUFLEN] = {};

        int message_len;
        int slen = sizeof(sockaddr_in);
        if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
        {
            exit(0);
        }

        cout << "Received packet from " << inet_ntoa(client.sin_addr) << ": " << ntohs(client.sin_port) << endl;
        std::istringstream commandStream(message);
        std::string commandLine;

        while (std::getline(commandStream, commandLine, '\n')) {
            Command command = ParseCommand(commandLine);

            std::string firstWord, secondWord;
            std::istringstream nameStream(command.name);
            nameStream >> firstWord >> secondWord;

            if (!invalid_com) {
                std::cout << "Main command: " << command.name << std::endl;
                std::cout << "Parameters:";
                for (const std::string& param : command.params) {
                    std::cout << " " << param;
                }
                std::cout << std::endl;
            }
        }

        if (invalid_com)
        {
            if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
            {
                return 3;
            }
        }

        if (!invalid_com)
        {
            if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
            {
                return 3;
            }
        }

    }
    closesocket(server_socket);
    WSACleanup();
}
