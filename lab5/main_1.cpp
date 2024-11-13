#include <windows.h>
#include <string>
#include <fstream>

CRITICAL_SECTION FileReadingLockingCriticalSection;
CRITICAL_SECTION FileWritingLockingCriticalSection;

int ReadFromFile() {
    EnterCriticalSection(&FileReadingLockingCriticalSection);
    std::fstream myfile("balance.txt", std::ios_base::in);
    int result = 0;
    myfile >> result;
    myfile.close();
    LeaveCriticalSection(&FileReadingLockingCriticalSection);
    return result;
}

void WriteToFile(int data) {
    EnterCriticalSection(&FileWritingLockingCriticalSection);
    std::fstream myfile("balance.txt", std::ios_base::out);
    myfile << data << std::endl;
    myfile.close();
    LeaveCriticalSection(&FileWritingLockingCriticalSection);
}

int GetBalance() {
    return ReadFromFile();
}

void Deposit(int money) {
    int balance = GetBalance();
    balance += money;
    WriteToFile(balance);
    printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money) {
    int balance = GetBalance();
    if (balance < money) {
        printf("Cannot withdraw money, balance lower than %d\n", money);
    } else {
        balance -= money;
        WriteToFile(balance);
        printf("Balance after withdraw: %d\n", balance);
    }
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter) {
    Deposit(static_cast<int>(reinterpret_cast<intptr_t>(lpParameter)));
    ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter) {
    Withdraw(static_cast<int>(reinterpret_cast<intptr_t>(lpParameter)));
    ExitThread(0);
}

int main() {
    HANDLE handles[50];
    InitializeCriticalSection(&FileReadingLockingCriticalSection);
    InitializeCriticalSection(&FileWritingLockingCriticalSection);
    WriteToFile(0);

    SetProcessAffinityMask(GetCurrentProcess(), 1);

    for (int i = 0; i < 50; i++) {
        handles[i] = (i % 2 == 0)
                         ? CreateThread(NULL, 0, &DoDeposit, reinterpret_cast<LPVOID>(230), CREATE_SUSPENDED, NULL)
                         : CreateThread(NULL, 0, &DoWithdraw, reinterpret_cast<LPVOID>(1000), CREATE_SUSPENDED, NULL);
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(50, handles, TRUE, INFINITE);

    printf("Final Balance: %d\n", GetBalance());

    for (const auto &handle: handles) {
        CloseHandle(handle);
    }

    DeleteCriticalSection(&FileWritingLockingCriticalSection);
    DeleteCriticalSection(&FileReadingLockingCriticalSection);

    return 0;
}
