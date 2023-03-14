
#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

//연결된 클라이언트와 통신(Echo 서비스)를 하기 위한 작업자 스레드 함수
DWORD WINAPI ThreadFunction(LPVOID pParam)
{

	char szBuffer[128] = { 0 };
	int nReceive = 0;
	SOCKET hClient = (SOCKET)pParam;

	puts("새 클라이언트가 연결되었습니다.");

	while ((nReceive = recv(hClient, szBuffer, sizeof(szBuffer), 0))) 	// 5바이트만 받았다면 5바이트만 복사가 된다.
	{
		send(hClient, szBuffer, sizeof(szBuffer), 0);
		puts(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));

	}
	puts("클라이언트 연결이 끊겼습니다.");
	::closesocket(hClient);
	return 0;
}

int main()
{
	//※윈속 초기화
	//윈속을 초기화 해야 윈속 라이브러리를 사용할 수 있다.
	WSADATA wsa = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		puts("ERROR: 윈속을 초기화 할 수 없습니다.");
		return 0;
	}

	// 1. 소켓 열기
	SOCKET hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		puts("ERROR: 접속 대기 소켓을 생성할 수 없습니다.");
		return 0;
	}

		// 바인딩 전에 IP주소와 포트를 재사용하도록 소켓 옵션을 변경한다.
	BOOL bOption = TRUE;
	if (setsockopt(hSocket, SOL_SOCKET,
		SO_REUSEADDR, (char*)&bOption, sizeof(BOOL) == SOCKET_ERROR))
	{
		puts("ERROR: 소켓 옵션을 변경할 수 없습니다.");
		return 0;
	}

	// 2. 포트 바인딩
	SOCKADDR_IN svraddr = { 0 };

	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	//svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	svraddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		puts("ERROR: 소켓에 IP주소와 포트를 바인드 할 수 없습니다.");
		return 0;
	}


	puts("Echo 서버를 실행합니다.");

	//3. 접속 대기 상태로 전환
	if (listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		puts("ERROR: 리슨 상태로 전환할 수 없습니다.");
		return 0;
	}


	//4 클라이언트 접속 처리 및 대응
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = 0;
	DWORD dwThreadID = 0;
	HANDLE hThread;


	//4.1 클라이언트 연결을 받아들이고 새로운 소켓 생성(개방)
	// clientaddr 구조체에 연결한 클라이언트의 정보가 담김
	while ((hClient = accept(hSocket, (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{

		hThread = CreateThread(
			NULL, // 보안속성 상속
			0, // 스택 메모리는 기본크기(1MB)
			ThreadFunction, //스레드로 실행할 함수이름
			(LPVOID)hClient, //세로 생성된 클라이언트 소켓
			0, //생성 플래그는 기본괎 사용
			&dwThreadID); //생성된 스레드ID가 저장될 변수 주소
		CloseHandle(hThread);
	}

	closesocket(hSocket);

	WSACleanup();
	return 0;
}