// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdbool.h>



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�


//extern const unsigned char BitSwapTable[256];

void ProgressInit(void);
void ProgressShow(int percentage);
void ProgressDone(void);


