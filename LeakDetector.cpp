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

static unsigned long s_memory_allocated = 0; // 未释放的内存大小

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
	// 计算新的大小
	size_t newSize = sizeof(_MemoryList)+_size;

	// 由于 new 已经被重载，我们只能使用 malloc 来分配内存
	_MemoryList *newElem = (_MemoryList*)malloc(newSize);

	newElem->next = s_root.next;
	newElem->prev = &s_root;
	newElem->size = _size;
	newElem->isArray = _array;
	newElem->file = NULL;

	// 如果有文件信息，则保存下来
	if (_file) {
		newElem->file = (char *)malloc(strlen(_file) + 1);
		strcpy(newElem->file, _file);
	}
	// 保存行号
	newElem->line = _line;

	// 更新列表
	s_root.next->prev = newElem;
	s_root.next = newElem;

	// 记录到未释放内存数中
	s_memory_allocated += _size;

	// 返回申请的内存，将 newElem 强转为 char* 来严格控制指针每次 +1 只移动一个 byte
	return (char*)newElem + sizeof(_MemoryList);
}

void  DeleteMemory(void* _ptr, bool _array) {
	// 返回 MemoryList 开始处
	_MemoryList *currentElem = (_MemoryList *)((char *)_ptr - sizeof(_MemoryList));

	if (currentElem->isArray != _array) return;

	// 更新列表
	currentElem->prev->next = currentElem->next;
	currentElem->next->prev = currentElem->prev;
	s_memory_allocated -= currentElem->size;

	// 记得释放存放文件信息时申请的内存
	if (currentElem->file) free(currentElem->file);
	free(currentElem);
}

// 重载 new 运算符
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
// 重载 delete 运算符
void operator delete(void *_ptr) {
	DeleteMemory(_ptr, false);
}
void operator delete[](void *_ptr) {
	DeleteMemory(_ptr, true);
}

unsigned int LeakDetector::leakDetector(void) {
	unsigned int count = 0;
	// 遍历整个列表, 如果有内存泄露，那么 _LeakRoot.next 总不是指向自己的
	_MemoryList *ptr = s_root.next;
	while (ptr && ptr != &s_root)
	{
		// 输出存在内存泄露的相关信息, 如泄露大小, 产生泄露的位置
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