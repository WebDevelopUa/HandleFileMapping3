# HandleFileMapping3

*HandleFileMapping3.cpp*: определяет точку входа для консольного приложения.

https://youtu.be/RtF7Z2kg6Rs
 
Следующий пример создает объект "проекция файла" (**HANDLE hMapFile**), дублирует дескриптор (**HANDLE hMapFileDup**) и передает его в другой поток (**HANDLE hMapFileThread**).

Дублирование описателя гарантирует, что счетчик ссылок будет увеличен, чтобы объект "проекция файла" (**HANDLE hMapFile**)
не был уничтожен, пока оба потока не закрыли описатели.

Некоторые функции *Windows API* ожидают, что вы укажете указатель на функцию **CALLBACK**, которую система должна будет вызывать как часть операции. Такие API-функции, как **SetTimer**, **timeSetEvent**, **CreateThread**, **EnumWindows**, **LineDDA**, даже обработчик оконных сообщений, установленные с помощью **RegisterClassEx** или **CreateDialog**, требуют такого указателя функции. Функции сортировки, такие как используемые стандартным элементом управления **ListView**, используют обратный вызов для сравнения элементов.
 
 ***
 
*HandleFileMapping3.cpp*: specifies the entry point for the console application.

 
The following example creates a file projection object (**HANDLE hMapFile**) duplicates the descriptor (**HANDLE hMapFileDup**) and passes it to another thread (**HANDLE hMapFileThread**).

Duplicating the handle ensures that the reference count is incremented so that the "file projection" object (**HANDLE hMapFile**)
It was not destroyed until both threads were closed by the descriptors.
