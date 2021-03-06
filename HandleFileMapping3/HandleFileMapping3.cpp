// HandleFileMapping3.cpp: определяет точку входа для консольного приложения.
// https://youtu.be/RtF7Z2kg6Rs

/*
Следующий пример создает объект "проекция файла" (HANDLE hMapFile),
дублирует дескриптор (HANDLE hMapFileDup) и передает его в другой поток (HANDLE hMapFileThread).

Дублирование описателя гарантирует, что счетчик ссылок будет увеличен, чтобы объект "проекция файла" (HANDLE hMapFile)
не был уничтожен, пока оба потока не закрыли описатели.

Некоторые функции Windows API ожидают, что вы укажете указатель на функцию CALLBACK, которую система должна
будет вызывать как часть операции. Такие API-функции, как SetTimer, timeSetEvent, CreateThread, EnumWindows,
LineDDA, даже обработчик оконных сообщений, установленные с помощью RegisterClassEx или CreateDialog, требуют
такого указателя функции. Функции сортировки, такие как используемые стандартным элементом управления ListView,
используют обратный вызов для сравнения элементов.

*/

#include "stdafx.h"
#include <windows.h>

#define BUF_SIZE 256
#define COPY_SIZE 256

DWORD CALLBACK ThreadProc(PVOID pvParam);
TCHAR szName[] = TEXT("MyFileMappingObject");
TCHAR szMsg[] = TEXT("Message from Process #1");

void MyCopyMemory(TCHAR *buf, TCHAR *szMsg, SIZE_T cbData, SIZE_T bufsize)
{
	CopyMemory(buf, szMsg, min(cbData, bufsize));
}

int main()
{
	HANDLE hMapFile;
	HANDLE hMapFileDup;
	HANDLE hMapFileThread;
	TCHAR *pBuf;
	DWORD dwThreadId;

	// создаем объект "проекция файла", связанный с массивом данных по имени - szName
	hMapFile = CreateFileMapping(

		// использование файла подкачки
		INVALID_HANDLE_VALUE,

		// защита по умолчанию 
		NULL,

		// доступ к записи-чтению
		PAGE_READWRITE,

		// макс. размер объекта 
		0,

		// размер буфера  
		BUF_SIZE,

		// имя отраженного в памяти объекта
		szName);

	// Если объекта ядра с указанным именем нет, 
	// функции возвращают NULL, a GetLastError() — код 2 (ERROR_FILE_NOT_FOUND).
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
	{
		//Не может создать отраженный в памяти объект
		printf("Failed with error %d: | ERROR %x - Can not create a mapped object \n",
			GetLastError());
		return 0;
	}

	// представление всего массива (szName) на адресном пространстве
	pBuf = (TCHAR*)MapViewOfFile(

		// дескриптор отраженного в памяти объекта
		hMapFile,

		// разрешение записи-чтения
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);

	// Узнать подробности об ошибке можно с помощью GetLastError()
	if (pBuf == NULL)
	{
		// Представление проецируемого файла невозможно 
		printf("Failed with ERROR %x | %d The projected file can not be displayed \n",
			GetLastError());
		return 0;
	}

	// вызов функции CopyMemory(), используя обертку MyCopyMemory()
	MyCopyMemory(pBuf, szMsg, COPY_SIZE * sizeof(TCHAR), BUF_SIZE * sizeof(TCHAR));
	_tprintf(TEXT("Destination buffer contents: %s\n"), pBuf);

	// создание дубликата дескриптора 
	// берет запись в таблице описателей одного процесса и создает ее копию в таблице другого
	DuplicateHandle(

		// описатель объекта ядра исходного процесса
		GetCurrentProcess(),

		//описатель созданный фуекцией CreateFileMapping()
		hMapFile,

		// описатель объекта ядра целевого процесса
		GetCurrentProcess(),

		// адрес переменной типа HANDLE, в который возвращается индекс записи с копией описателя из процесса-источника
		&hMapFileDup,

		// позволяют задать маску доступа
		0,

		// позволяют задать флаг наследования
		FALSE,

		// подсказывает функции DuplicateHandle() у описателя, получаемого процессом-приемником, 
		// должна быть та же маска доступа, что и у описателя в процессе-источнике
		DUPLICATE_SAME_ACCESS
	);

	// создаем Поток #2
	hMapFileThread = CreateThread(
		NULL,
		0,
		ThreadProc,
		(LPVOID)hMapFileDup,
		0,
		&dwThreadId
	);

	// функция ждёт нажатия ENTER
	getwchar();

	// снятие отображения содержимого файла
	UnmapViewOfFile(pBuf);

	// Если счетчик ссылок равен нулю, объект уничтожается.
	CloseHandle(hMapFile);

	// Подождите, пока рабочий Поток #2 завершится и очистится.
	WaitForSingleObject(hMapFileThread, INFINITE);
	CloseHandle(hMapFileThread);


	return 0;
}

// Поток #2
DWORD CALLBACK ThreadProc(PVOID pvParam)
{
	HANDLE hMapFile = (HANDLE)pvParam;
	TCHAR *pBuf;

	// представление всего массива данных (szName) на адресном пространстве
	pBuf = (TCHAR*)MapViewOfFile(

		// дескриптор "проецируемого" объекта
		hMapFile,

		// разрешение записи-чтения
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUF_SIZE);


	// Узнать подробности об ошибке можно с помощью GetLastError()
	if (pBuf == NULL)
	{
		// Представление проецируемого файла невозможно 
		printf("Failed with ERROR %x | %d The projected file can not be displayed \n",
			GetLastError());
		return 0;
	}

	// вывод сообщения через диалоговое окно из Потока #2
	MessageBox(NULL, pBuf, TEXT("Thread #2"), MB_OK);

	// Если счетчик ссылок равен нулю, объект уничтожается.
	CloseHandle(hMapFile);

	return 0;
}