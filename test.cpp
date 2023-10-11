#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <set>

// Hàm hi?n th? thông tin v? m?t lu?ng
void DisplayThreadInfo(const THREADENTRY32& threadEntry)
{
    std::cout << "Thread ID: " << threadEntry.th32ThreadID << std::endl;
    std::cout << "Process ID: " << threadEntry.th32OwnerProcessID << std::endl;
    std::cout << "Base Priority: " << threadEntry.tpBasePri << std::endl;

    // L?y thông tin v? d?a ch? b?t d?u c?a lu?ng
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadEntry.th32ThreadID);
    if (hThread != NULL)
    {
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime))
        {
            // Chuy?n d?i FILETIME sang ULARGE_INTEGER d? tính toán s? l?n chuy?n ng? c?nh
            ULARGE_INTEGER kernelTimeValue;
            kernelTimeValue.LowPart = kernelTime.dwLowDateTime;
            kernelTimeValue.HighPart = kernelTime.dwHighDateTime;

            ULARGE_INTEGER userTimeValue;
            userTimeValue.LowPart = userTime.dwLowDateTime;
            userTimeValue.HighPart = userTime.dwHighDateTime;

            std::cout << "Kernel Time (ms): " << kernelTimeValue.QuadPart / 10000 << std::endl;
            std::cout << "User Time (ms): " << userTimeValue.QuadPart / 10000 << std::endl;
        }

        CloseHandle(hThread);
    }

    std::cout << std::endl;
}

// Hàm hi?n th? danh sách các lu?ng
void DisplayProcessInfo(){
	HANDLE hSnapshot;
    PROCESSENTRY32 pe32;

    // T?o snapshot c?a các ti?n trình dang ch?y
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Khong the tao snapshot cua tien trinh. Loi " << GetLastError() << std::endl;
        return ;
    }

    // Thi?t l?p kích thu?c c?u trúc PROCESSENTRY32
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // L?y thông tin c?a ti?n trình d?u tiên
    if (!Process32First(hSnapshot, &pe32))
    {
        std::cout << "Khong the lay thông tin tien trinh. Loi " << GetLastError() << std::endl;
        CloseHandle(hSnapshot);
        return ;
    }

    // Li?t kê danh sách các ti?n trình
    std::cout << "Danh sach cac tien trinh:" << std::endl;
    do
    {
    	if (pe32.th32ProcessID != GetCurrentProcessId()){
    		continue;
		}
    	
        std::cout << "Process ID: " << pe32.th32ProcessID << ", Process Name: " << pe32.szExeFile << std::endl;
        
        // L?y ID c?a ti?n trình hi?n t?i
	    DWORD currentProcessId = pe32.th32ProcessID;
	
	    // L?y snapshot c?a t?t c? các ti?n trình và lu?ng dang ch?y trên h? th?ng
	    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	    if (snapshot == INVALID_HANDLE_VALUE)
	    {
	        std::cerr << "Không th? l?y snapshot c?a ti?n trình và lu?ng." << std::endl;
	        return ;
	    }
	
	    // Kh?i t?o kích thu?c c?a THREADENTRY32
	    THREADENTRY32 threadEntry;
	    threadEntry.dwSize = sizeof(THREADENTRY32);
	
	    // Khai báo std::set d? theo dõi các lu?ng dã hi?n th?
		std::set<DWORD> displayedThreads;
		
		// L?p qua t?t c? các lu?ng trong snapshot
		if (Thread32First(snapshot, &threadEntry))
		{
		    do
		    {
		        // Ki?m tra xem lu?ng thu?c v? ti?n trình hi?n t?i hay không
		        if (threadEntry.th32OwnerProcessID == currentProcessId)
		        {
		            // Ki?m tra xem lu?ng dã du?c hi?n th? hay chua
		            if (displayedThreads.find(threadEntry.th32ThreadID) == displayedThreads.end())
		            {
		                // Hi?n th? thông tin c?a lu?ng
		                DisplayThreadInfo(threadEntry);
		
		                // Thêm th32ThreadID vào std::set d? theo dõi
		                displayedThreads.insert(threadEntry.th32ThreadID);
		            }
		        }
		    } while (Thread32Next(snapshot, &threadEntry));
		}
	
	    // Ðóng snapshot
	    CloseHandle(snapshot);
        
    } while (Process32Next(hSnapshot, &pe32));
    
    // Ðóng handle c?a snapshot
    CloseHandle(hSnapshot);
}

DWORD WINAPI Thread1(LPVOID lpParam)
{
    // Th?i gian ch?y c?a lu?ng (10 phút = 600,000 milliseconds)
    const DWORD totalRuntime = 60000;

    DWORD startTime = GetTickCount(); // Th?i di?m b?t d?u ch?y lu?ng

    while (GetTickCount() - startTime < totalRuntime)
    {
        // Mã công vi?c c?a lu?ng
        std::cout << "Thread 1 is running" << std::endl << std::endl;
        // Các công vi?c khác c?a lu?ng...

        Sleep(1000); // T?m d?ng th?c thi lu?ng trong 1 giây
    }

    return 0;
}

DWORD WINAPI Thread2(LPVOID lpParam)
{
    // Th?i gian ch?y c?a lu?ng (10 phút = 600,000 milliseconds)
    const DWORD totalRuntime = 60000;

    DWORD startTime = GetTickCount(); // Th?i di?m b?t d?u ch?y lu?ng

    while (GetTickCount() - startTime < totalRuntime)
    {
        // Mã công vi?c c?a lu?ng
        std::cout << "Thread 2 is running" << std::endl << std::endl;
        // Các công vi?c khác c?a lu?ng...

        Sleep(1000); // T?m d?ng th?c thi lu?ng trong 1 giây
    }

    return 0;
}

int main()
{
	DisplayProcessInfo();
    int i=1;
    // T?o lu?ng 1
    HANDLE hThread1 = CreateThread(NULL, 0, Thread1, NULL, 0, NULL);
    if (hThread1 == NULL)
    {
        std::cerr << "Failed to create Thread 1" << std::endl;
        return 1;
    }
    
    // T?o lu?ng 2
    HANDLE hThread2 = CreateThread(NULL, 0, Thread2, NULL, 0, NULL);
    if (hThread2 == NULL)
    {
        std::cerr << "Failed to create Thread 2" << std::endl;
        return 1;
    }
    
    DisplayProcessInfo();
    
    std::cout << "Thay doi do uu tien cua Thread 2" << std::endl << std::endl;
    
    // Thay d?i d? uu tiên c?a lu?ng thành ABOVE_NORMAL_PRIORITY_CLASS
    if (!SetThreadPriority(hThread2, THREAD_PRIORITY_ABOVE_NORMAL))
    {
        std::cerr << "Failed to set thread priority." << std::endl;
        CloseHandle(hThread2);
        return 1;
    }
    
    DisplayProcessInfo();
    
    if (!TerminateThread(hThread1, 0))
    {
        std::cerr << "Failed to terminate thread" << std::endl;
        return 1;
    }

    // Ðóng handle c?a lu?ng
    CloseHandle(hThread1);
    
    DisplayProcessInfo();

    return 0;
}
