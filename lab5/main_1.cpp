#include <windows.h>
#include <string>
#include <fstream>

CRITICAL_SECTION FileLockingCriticalSection;

int ReadFromFile() {
    std::fstream myfile("balance.txt", std::ios_base::in);
    int result = 0;
    myfile >> result;
    myfile.close();
    return result;
}

void WriteToFile(int data) {
    std::fstream myfile("balance.txt", std::ios_base::out);
    myfile << data << std::endl;
    myfile.close();
}

int GetBalance() {
    return ReadFromFile();
}

void Deposit(int money) {
    EnterCriticalSection(&FileLockingCriticalSection);
    int balance = GetBalance();
    balance += money;
    WriteToFile(balance);
    printf("Balance after deposit: %d\n", balance);
    LeaveCriticalSection(&FileLockingCriticalSection);
}

void Withdraw(int money) {
    EnterCriticalSection(&FileLockingCriticalSection);
    int balance = GetBalance();
    if (balance < money) {
        printf("Cannot withdraw money, balance lower than %d\n", money);
    } else {
        balance -= money;
        WriteToFile(balance);
        printf("Balance after withdraw: %d\n", balance);
    }
    LeaveCriticalSection(&FileLockingCriticalSection);
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
    InitializeCriticalSection(&FileLockingCriticalSection);
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

    DeleteCriticalSection(&FileLockingCriticalSection);

    return 0;
}
