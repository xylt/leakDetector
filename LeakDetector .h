/**************************************************************************
Copyright:陕西恒巨软件科技有限公司
Author: 李涛
Date: $time$
Description: functions  
**************************************************************************/
#pragma once
#include <iostream>

void* operator new(size_t _size, char *_file, unsigned int _line);
void* operator new[](size_t _size, char *_file, unsigned int _line);

#ifndef __NEW_OVERLOAD_IMPLEMENTATION__
#define new new(__FILE__, __LINE__)
#endif // !__NEW_OVERLOAD_IMPLEMENTATION

class LeakDetector
{
public:
	static unsigned int callCount;
	LeakDetector();
	~LeakDetector();

private:
	static unsigned int leakDetector();
};

static LeakDetector exitCounter;
