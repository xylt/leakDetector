#define __NEW_OVERLOAD_IMPLEMENTATION__
#include "LeakDetector .h"

#include <cstring>

typedef struct _MemoryList{
	struct _MemoryList *next, *prev;
	size_t size;
	bool isArray;
	char *file;
	unsigned int line;
}_MemoryList;

static unsigned long s_memory_allocated = 0; // δ�ͷŵ��ڴ��С

static _MemoryList s_root = { &s_root, &s_root, 0, false, NULL, 0 };

unsigned int LeakDetector::callCount = 0;

LeakDetector::LeakDetector()
{
	++callCount;
}

LeakDetector::~LeakDetector()
{
	if (--callCount == 0)
	{
		leakDetector();
	}
}

void* AllocateMemory(size_t _size, bool _array, char *_file, unsigned _line) {
	// �����µĴ�С
	size_t newSize = sizeof(_MemoryList)+_size;

	// ���� new �Ѿ������أ�����ֻ��ʹ�� malloc �������ڴ�
	_MemoryList *newElem = (_MemoryList*)malloc(newSize);

	newElem->next = s_root.next;
	newElem->prev = &s_root;
	newElem->size = _size;
	newElem->isArray = _array;
	newElem->file = NULL;

	// ������ļ���Ϣ���򱣴�����
	if (_file) {
		newElem->file = (char *)malloc(strlen(_file) + 1);
		strcpy(newElem->file, _file);
	}
	// �����к�
	newElem->line = _line;

	// �����б�
	s_root.next->prev = newElem;
	s_root.next = newElem;

	// ��¼��δ�ͷ��ڴ�����
	s_memory_allocated += _size;

	// ����������ڴ棬�� newElem ǿתΪ char* ���ϸ����ָ��ÿ�� +1 ֻ�ƶ�һ�� byte
	return (char*)newElem + sizeof(_MemoryList);
}

void  DeleteMemory(void* _ptr, bool _array) {
	// ���� MemoryList ��ʼ��
	_MemoryList *currentElem = (_MemoryList *)((char *)_ptr - sizeof(_MemoryList));

	if (currentElem->isArray != _array) return;

	// �����б�
	currentElem->prev->next = currentElem->next;
	currentElem->next->prev = currentElem->prev;
	s_memory_allocated -= currentElem->size;

	// �ǵ��ͷŴ���ļ���Ϣʱ������ڴ�
	if (currentElem->file) free(currentElem->file);
	free(currentElem);
}

// ���� new �����
void* operator new(size_t _size){
	return AllocateMemory(_size, false, NULL, 0);
}
void* operator new[](size_t _size) {
	return AllocateMemory(_size, true, NULL, 0);
}
void* operator new(size_t _size, char *_file, unsigned int _line){
	return AllocateMemory(_size, false, _file, _line);
}
void* operator new[](size_t _size, char *_file, unsigned int _line) {
	return AllocateMemory(_size, true, _file, _line);
}
// ���� delete �����
void operator delete(void *_ptr) {
	DeleteMemory(_ptr, false);
}
void operator delete[](void *_ptr) {
	DeleteMemory(_ptr, true);
}

unsigned int LeakDetector::leakDetector(void) {
	unsigned int count = 0;
	// ���������б�, ������ڴ�й¶����ô _LeakRoot.next �ܲ���ָ���Լ���
	_MemoryList *ptr = s_root.next;
	while (ptr && ptr != &s_root)
	{
		// ��������ڴ�й¶�������Ϣ, ��й¶��С, ����й¶��λ��
		if (ptr->isArray)
			std::cout << "leak[] ";
		else
			std::cout << "leak   ";
		std::cout << ptr << " size " << ptr->size;
		if (ptr->file)
			std::cout << " (locate in " << ptr->file << " line " << ptr->line << ")";
		else
			std::cout << " (Cannot find position)";
		std::cout << std::endl;

		++count;
		ptr = ptr->next;
	}

	if (count)
		std::cout << "Total " << count << " leaks, size " << s_memory_allocated << " byte." << std::endl;
	return count;
}