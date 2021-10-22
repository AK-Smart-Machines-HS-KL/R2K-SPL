#include "Platform/Time.h"
#include <Windows.h>

unsigned Time::getRealSystemTime()
{
  const unsigned time = timeGetTime();
  if(!base)
    base = time - 100000; // avoid time == 0, because it is often used as a marker
  return static_cast<unsigned>(time - base);
}

unsigned long long Time::getCurrentThreadTime()
{
  static LARGE_INTEGER frequency = { 0 };
  if(frequency.QuadPart == 0)
  {
    QueryPerformanceFrequency(&frequency);
  }
  LARGE_INTEGER timeLL;
  QueryPerformanceCounter(&timeLL);

  const unsigned long long time = static_cast<unsigned long long>(timeLL.QuadPart * 1000000ll / frequency.QuadPart);
  if(!threadTimebase)
    threadTimebase = time - 1000000ll;
  return time - threadTimebase;
}
