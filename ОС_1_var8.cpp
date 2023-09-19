#include <iostream>
#include <windows.h>
#include <ctime>

using namespace std;

#define FileSizeMin 100 * 1024  // Минимальный размер файла (100 Кбайт)
#define FileSizeMax 199 * 1024  // Максимальный размер файла (200 Кбайт)
#define StepSize 10 * 1024      // Шаг изменения размера файла
#define M 10                    // Количество итераций

int main() {
    __int64 c1, c2, fr;
    byte* MemoryArea;
    int i;
    HANDLE RandomAccessFile;
    HANDLE SequentialAccessFile;
    DWORD c;
    float t, v;
    LARGE_INTEGER start, end, frequency;

    QueryPerformanceFrequency(&frequency);

    const wchar_t* RandomAccessFileName = TEXT("random_access_file.bin");
    const wchar_t* SequentialAccessFileName = TEXT("sequential_access_file.bin");

    srand(static_cast<unsigned int>(time(NULL)));

    // Задаем максимальный размер файла в байтах (200 Кбайт)
    const int MaxFileSizeBytes = 199 * 1024;
    float totalRandomAccessSpeed = 0.0;
    float totalSequentialAccessSpeed = 0.0;

    for (int fileSize = FileSizeMin; fileSize <= FileSizeMax; fileSize += StepSize) {
        if (fileSize > MaxFileSizeBytes) {
            break; // Выходим из цикла, если размер файла превышает максимальный размер
        }

        for (int z = 0; z < M; z++) {
            RandomAccessFile = CreateFile(RandomAccessFileName,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                0);

            if (RandomAccessFile == INVALID_HANDLE_VALUE || RandomAccessFile == NULL) {
                cout << "Ошибка при создании файла прямого доступа: " << GetLastError() << endl;
                cin.get();
                return 0;
            }

            SequentialAccessFile = CreateFile(SequentialAccessFileName,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                0);

            if (SequentialAccessFile == INVALID_HANDLE_VALUE || SequentialAccessFile == NULL) {
                cout << "Ошибка при создании файла последовательного доступа: " << GetLastError() << endl;
                CloseHandle(RandomAccessFile);
                cin.get();
                return 0;
            }

            MemoryArea = (byte*)GlobalAlloc(GPTR, fileSize);

            for (i = 0; i < fileSize; i += sizeof(int)) {
                int randomValue = rand() % 1000 + 1;  // Генерируем случайное число (от 1 до 1000)
                CopyMemory(MemoryArea + i, &randomValue, sizeof(int));
            }

            WriteFile(RandomAccessFile, MemoryArea, fileSize, &c, 0);
            WriteFile(SequentialAccessFile, MemoryArea, fileSize, &c, 0);

            GlobalFree(MemoryArea);

            // Измерение времени чтения с конца файла для файла прямого доступа

            QueryPerformanceCounter(&start);

            i = fileSize;
            t = 0.0;
            MemoryArea = (byte*)GlobalAlloc(GPTR, i);

            for (int j = 0; j < i; j++) {
                SetFilePointer(RandomAccessFile, -j - 1, NULL, FILE_END); // Изменяем сдвиг на -j - 1
                ReadFile(RandomAccessFile, MemoryArea + j, 1, &c, 0);
            }

            QueryPerformanceCounter(&end);
            t = static_cast<float>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
            v = fileSize / (1024 * t);

            totalRandomAccessSpeed += v;

            GlobalFree(MemoryArea);

            // Измерение времени чтения с конца файла для файла последовательного доступа

            QueryPerformanceCounter(&start);

            i = fileSize;
            t = 0.0;
            MemoryArea = (byte*)GlobalAlloc(GPTR, i);

            for (int j = 0; j < i; j++) {
                SetFilePointer(SequentialAccessFile, -j, NULL, FILE_END);
                ReadFile(SequentialAccessFile, MemoryArea + j, 1, &c, 0);
            }

            QueryPerformanceCounter(&end);
            t = static_cast<float>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
            v = fileSize / (1024 * t);

            totalSequentialAccessSpeed += v;

            GlobalFree(MemoryArea);

            CloseHandle(RandomAccessFile);
            CloseHandle(SequentialAccessFile);
        }

        // Выводим средние скорости для данного размера файла
        float averageRandomAccessSpeed = totalRandomAccessSpeed / M;
        float averageSequentialAccessSpeed = totalSequentialAccessSpeed / M;

        cout << "File Size: " << fileSize << " bytes" << endl;
        cout << "Average Random Access Speed(KB/s): " << averageRandomAccessSpeed << endl;
        cout << "Average Sequential Access Speed(KB/s): " << averageSequentialAccessSpeed << endl;
    }

    cin.get();
    return 0;
}