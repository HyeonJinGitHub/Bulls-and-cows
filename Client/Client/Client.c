#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <WinSock2.h>

#define PORT 20000

void ErrorHandling(char* msg);
#pragma comment(lib,"ws2_32.lib")

int main() {
	WSADATA wsdata;
	/*WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;*/
	if (WSAStartup(MAKEWORD(2, 2), &wsdata) != 0) {
		ErrorHandling("WSAStartup() error");
		return 0;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		ErrorHandling("socket() error");
		return 0;
	}

	SOCKADDR_IN serverAddress;
	ZeroMemory(&serverAddress, sizeof(serverAddress));

	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);

	connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	//newEvent = WSACreateEvent();
	//if (WSAEventSelect(clientSocket, newEvent, FD_CLOSE) == SOCKET_ERROR)
	//	ErrorHandling("WSAEventSelect() error");

	printf("숫자 야구 게임을 시작합니다.\n");
	while (1) {
		int clientNumber[4];
		
		printf("10 이하의 네 수를 입력해 주세요 ex: 1 2 3 4\n-->");
		scanf("%d %d %d %d", &clientNumber[0], &clientNumber[1], &clientNumber[2], &clientNumber[3]);

		for (int i = 0; i < 4; i++)
			clientNumber[i] = htonl(clientNumber[i]);

		send(clientSocket, (char*)clientNumber, sizeof(clientNumber), 0);

		int score[2];

		recv(clientSocket, (char *)score, sizeof(score), 0);

		int strike = ntohl(score[0]);
		int ball = ntohl(score[1]);
		
		if (strike == 0 && ball == 0)
		{
			printf("아웃!\n\n");
		}
		else
		{
			printf(":%d 스트라이크 %d볼\n\n", strike, ball);
		}

		if (strike == 4) {
			printf("4 스트라이크! 축하합니다. 프로그램을 종료합니다.\n\n");
			break;
		}
	}
	closesocket(clientSocket);
	WSACleanup();
	system("pause");
	return 1;
}
void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}