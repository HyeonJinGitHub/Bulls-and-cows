#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char* msg);

int main() {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;

	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent;
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS netEvents;

	int numOfClntSock = 0;
	int posInfo, startIdx;
	int clntAdrLen;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi("20000"));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	newEvent = WSACreateEvent();
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;
	puts("Waiting new client....");

	while (1) {
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		startIdx = posInfo - WSA_WAIT_EVENT_0;
		int serverNumber[100][4];
		for (int i = startIdx; i < numOfClntSock; i++) {
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
			{
				continue;
			}
			else {
				sigEventIdx = i;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						puts("Accept Error");
						break;
					}
					clntAdrLen = sizeof(clntAdr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					printf("connected new client!! 현재 클라이언트 수 : %d\n",numOfClntSock);
			
					srand((unsigned)time(NULL));
					do {
						serverNumber[numOfClntSock][0] = rand() % 10;
						serverNumber[numOfClntSock][1] = rand() % 10;
						serverNumber[numOfClntSock][2] = rand() % 10;
						serverNumber[numOfClntSock][3] = rand() % 10;
					}
					while ((serverNumber[numOfClntSock][0] == serverNumber[numOfClntSock][1]) | 
						(serverNumber[numOfClntSock][0] == serverNumber[numOfClntSock][2]) |
						(serverNumber[numOfClntSock][0] == serverNumber[numOfClntSock][3]) |
						(serverNumber[numOfClntSock][1] == serverNumber[numOfClntSock][2]) |
						(serverNumber[numOfClntSock][1] == serverNumber[numOfClntSock][3]) |
						(serverNumber[numOfClntSock][2] == serverNumber[numOfClntSock][3]));
					
					printf("서버 숫자 --->%d %d %d %d\n", serverNumber[numOfClntSock][0], serverNumber[numOfClntSock][1], serverNumber[numOfClntSock][2], serverNumber[numOfClntSock][3]);
					numOfClntSock++;
				}
				int fromClientNumber[100][4]; 
				int toClientScore[2];

				if (netEvents.lNetworkEvents & FD_READ) {
					if (netEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						puts("Read Error");
						break;
					}
					
					recv(hSockArr[sigEventIdx], (char*)fromClientNumber[sigEventIdx], sizeof(fromClientNumber[sigEventIdx]), 0);
					
					for (int index = 0; index < 4; index++)
						fromClientNumber[sigEventIdx][index] = ntohl(fromClientNumber[sigEventIdx][index]);
					
					printf("%d번 서버 숫자 --->%d %d %d %d\n",sigEventIdx, serverNumber[sigEventIdx][0], serverNumber[sigEventIdx][1], serverNumber[sigEventIdx][2], serverNumber[sigEventIdx][3]);
					printf("%d번 클라이언트가 입력한 수 -->%d %d %d %d\n",sigEventIdx,
						fromClientNumber[sigEventIdx][0], fromClientNumber[sigEventIdx][1], fromClientNumber[sigEventIdx][2], fromClientNumber[sigEventIdx][3]);
					int strike = 0; 
					int ball = 0;  
					for (int i = 0; i < 4; i++) {
						int oneServerNumber = serverNumber[sigEventIdx][i];
						for (int j = 0; j < 4; j++) {
							int oneClientNumber = fromClientNumber[sigEventIdx][j];
							if (oneServerNumber == oneClientNumber) {
								if (i == j)
									strike++;
								else
									ball++;
							}
						} 
					}
					if (strike == 0 && ball == 0)
					{
						printf("아웃!\n\n");
					}
					else
					{
						printf("%d 스트라이크 %d 볼 \n\n", strike, ball);
					}
					
					toClientScore[0] = htonl(strike);
					toClientScore[1] = htonl(ball);
					send(hSockArr[sigEventIdx], (char*)toClientScore, sizeof(toClientScore), 0);
					
					if (strike == 4) {
						printf("%d번 클라이언트 4 스트라이크!! %d번 클라이언트를 종료합니다. 클라이언트 수 1 감소\n",sigEventIdx,sigEventIdx);
					}
				}
				if (netEvents.lNetworkEvents & FD_CLOSE) {
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
						puts("Close Error");
						break;
					}
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					numOfClntSock--;
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
					if(numOfClntSock==1)
						puts("Waiting new client....");
				}
			}
		}
	}
	WSACleanup();
	return 0;
}
void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	for (int i = idx; i < total; i++) {
		hSockArr[i] = hSockArr[i + 1];
	}

}
void CompressEvents(WSAEVENT hEventArr[], int idx, int total) {
	for (int i = idx; i < total; i++)
		hEventArr[i] = hEventArr[i + 1];
}
void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}