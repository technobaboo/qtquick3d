#include <QtDemon/qdemontime.h>

QT_BEGIN_NAMESPACE

//namespace {
//::qt3ds::QT3DSI64 getTimeTicks()
//{
//    LARGE_INTEGER a;
//    QueryPerformanceCounter(&a);
//    return a.QuadPart;
//}

//double getTickDuration()
//{
//    LARGE_INTEGER a;
//    QueryPerformanceFrequency(&a);
//    return 1.0f / double(a.QuadPart);
//}

//double sTickDuration = getTickDuration();
//} // namespace

//const QDemonTimeCounterFrequencyToTensOfNanos QDemonTime::sCounterFreq = QDemonTime::getCounterFrequency();

//QDemonTimeCounterFrequencyToTensOfNanos QDemonTime::getCounterFrequency()
//{
//    LARGE_INTEGER freq;
//    QueryPerformanceFrequency(&freq);
//    return CounterFrequencyToTensOfNanos(Time::sNumTensOfNanoSecondsInASecond, freq.QuadPart);
//}

//quint64 QDemonTime::getCurrentCounterValue()
//{
//    LARGE_INTEGER ticks;
//    QueryPerformanceCounter(&ticks);
//    return ticks.QuadPart;
//}

//QDemonTime::QDemonTime()
//    : mTickCount(0)
//{
//    getElapsedSeconds();
//}

//QDemonTime::Second QDemonTime::getElapsedSeconds()
//{
//    QT3DSI64 lastTickCount = mTickCount;
//    mTickCount = getTimeTicks();
//    return (mTickCount - lastTickCount) * sTickDuration;
//}

//QDemonTime::Second QDemonTime::peekElapsedSeconds()
//{
//    return (getTimeTicks() - mTickCount) * sTickDuration;
//}

//QDemonTime::Second QDemonTime::getLastTime() const { return mTickCount * sTickDuration; }


QT_END_NAMESPACE
