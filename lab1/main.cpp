#include <windows.h>
#include <string>
#include <iostream>

using namespace std;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam) 
{
	int num = *static_cast<int*>(lpParam);

	std::cout << "Thread #" << num << std::endl;
	ExitThread(0);
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << "<thread_count>" << std::endl;
		return 1;
	}

	auto threadCount = std::stoi(argv[1]);

	HANDLE* handles = new HANDLE[threadCount];
	int* handleNums = new int[threadCount];

	for (int i = 0; i < threadCount; i++) {
		handleNums[i] = i;
		handles[i] = CreateThread(NULL, 0, &ThreadProc, &handleNums[i], CREATE_SUSPENDED, NULL);
		if (handles[i] == NULL) {
			std::cout << "Error in thread creating" << std::endl;
			return 1;
		}
	}

	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(handles[i]);
	}

	
	WaitForMultipleObjects(threadCount, handles, true, INFINITE);


	delete[] handles;
	delete[] handleNums;

	return 0;
}
