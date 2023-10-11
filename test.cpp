#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <set>
using namespace std;

// Hàm hiển thị thông tin về mộtt luồngng
void DisplayThreadInfo(const THREADENTRY32& threadEntry)
{
    cout << "Thread ID: " << threadEntry.th32ThreadID << std::endl;
    cout << "Process ID: " << threadEntry.th32OwnerProcessID << std::endl;
    cout << "Base Priority: " << threadEntry.tpBasePri << endl;

    // Lấy thông tin về địa chỉ bắt đầu của luồng
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadEntry.th32ThreadID);
    if (hThread != NULL)
    {
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime))
        {
            // Chuyển đổi FILETIME sang ULARGE_INTEGER để tính toán số lần chuyển ngữ cảnnh
            ULARGE_INTEGER kernelTimeValue;
            kernelTimeValue.LowPart = kernelTime.dwLowDateTime;
            kernelTimeValue.HighPart = kernelTime.dwHighDateTime;

            ULARGE_INTEGER userTimeValue;
            userTimeValue.LowPart = userTime.dwLowDateTime;
            userTimeValue.HighPart = userTime.dwHighDateTime;

            cout << "Kernel Time (ms): " << kernelTimeValue.QuadPart / 10000 << endl;
            cout << "User Time (ms): " << userTimeValue.QuadPart / 10000 << endl;
        }

        CloseHandle(hThread);
    }

    cout << endl;
}


// Hàm hiển thị danh sách các luồng
void DisplayProcessInfo(){
	HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    // Tạo snapshot của các tiến trình dang chạy
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE){
        cout << "Khong the tao snapshot cua tien trinh. Loi " << GetLastError() << endl;
        return ;
    }
    // Thiết lập kích thước cấu trúc PROCESSENTRY32
    pe32.dwSize = sizeof(PROCESSENTRY32);
    // Lấy thông tin của tiến trình đầu tiên
    if (!Process32First(hSnapshot, &pe32)){
        cout << "Khong the lay thông tin tien trinh. Loi " << GetLastError() << endl;
        CloseHandle(hSnapshot);
        return ;
    }
    // Liệt kê danh sách các tiến trình
    cout << "Danh sach cac tien trinh:" << endl;
    do{
    	if (pe32.th32ProcessID != GetCurrentProcessId()){   continue;   }
        cout << "Process ID: " << pe32.th32ProcessID << ", Process Name: " << pe32.szExeFile << endl;
        // L?y ID c?a ti?n trình hi?n t?i
	    DWORD currentProcessId = pe32.th32ProcessID;
	    // L?y snapshot c?a t?t c? các ti?n trình và lu?ng dang ch?y trên h? th?ng
	    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	    if (snapshot == INVALID_HANDLE_VALUE){
	        cerr << "Khong the lay snapshot cuaa tien trinh và luong." << endl;
	        return ;
	    }
	    // Khởi tao kích thước của THREADENTRY32
	    THREADENTRY32 threadEntry;
	    threadEntry.dwSize = sizeof(THREADENTRY32);
	    // Khai báo set dể theo dõi các luồng dã hiển thị
		set<DWORD> displayedThreads;
		// Lặp qua tất cả các luồng trong snapshot
		if (Thread32First(snapshot, &threadEntry)){
		    do{
		        // Kiểm tra xem luồng thuộc về tiến trình hiện tại hay không
		        if (threadEntry.th32OwnerProcessID == currentProcessId){
		            // Kiểm tra xem luồng dã dượcc hiện thị hay chưa
		            if (displayedThreads.find(threadEntry.th32ThreadID) == displayedThreads.end()){
		                // Hiển thị thông tin của luồng
		                DisplayThreadInfo(threadEntry);
		                // Thêm th32ThreadID vào set để theo dõi
		                displayedThreads.insert(threadEntry.th32ThreadID);
		            }
		        }
		    } while (Thread32Next(snapshot, &threadEntry));
		}
	    // Ðóng snapshot
	    CloseHandle(snapshot);
    } while (Process32Next(hSnapshot, &pe32));
    // Ðóng handle của snapshot
    CloseHandle(hSnapshot);
}

// Thân luồng khởi tạo
DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
    // Thời gian chạy của luồng (10 phút = 600,000 milliseconds)
    const DWORD totalRuntime = 60000;

    DWORD startTime = GetTickCount(); // Thời diểm bắt đàu chạy luồng

    while (GetTickCount() - startTime < totalRuntime)
    {
    	intptr_t threadId = reinterpret_cast<intptr_t>(lpParam);
        // Mã công việc củaa luồng
        cout << "Thread " << threadId << " is running." << endl << endl;

        Sleep(1000); // Tạmm dừng thực thi luồng trong 1 giây
    }

    return 0;
}


int main()
{
    // Hiển thị danh sách các luồng
	DisplayProcessInfo();
    // Tạo luồng 1
    HANDLE hThread1 = CreateThread(NULL, 0, ThreadFunc, reinterpret_cast<LPVOID>(1), 0, NULL);
    if (hThread1 == NULL)
    {
        cerr << "Failed to create Thread 1" << endl;
        return 1;
    }
    // Tạo luồng 2
    HANDLE hThread2 = CreateThread(NULL, 0, ThreadFunc, reinterpret_cast<LPVOID>(2) ,0 , NULL);
    if (hThread2 == NULL)
    {
        cerr << "Failed to create Thread 2" << endl;
        return 1;
    }
    // Hiển thị danh sách các luồng
    DisplayProcessInfo();
    // Thay đổi đọ ưu tiên của luồng thành ABOVE_NORMAL_PRIORITY_CLASS
    if (!SetThreadPriority(hThread2, THREAD_PRIORITY_ABOVE_NORMAL))
    {
        cerr << "Failed to set thread 2 priority." << endl;
        CloseHandle(hThread2);
        return 1;
    }
    else
	{
    	cout << "Thay doi do uu tien cua Thread 2" << endl << endl;
	}
    // Hiển thị danh sách các luồng
    DisplayProcessInfo();
    // Giết luồng 1
    if (!TerminateThread(hThread1, 0))
    {
        cerr << "Failed to terminate thread1" << endl;
        return 1;
    }
	else
	{
		cout << "Giet Thread 1" << endl << endl;
	}
    // Ðóng handle của luồng 1
    CloseHandle(hThread1);
    
    DisplayProcessInfo();

    return 0;
}