#include <QtDemon/qdemontime.h>

#include <chrono>

QT_BEGIN_NAMESPACE

namespace {
quint64 getTimeTicks()
{
    quint64 a;
    a = std::chrono::high_resolution_clock::rep();
    return a;
}

double getTickDuration()
{
    quint64 a;
    a = std::chrono::high_resolution_clock::period::den;
    return 1.0 / double(a);
}

double sTickDuration = getTickDuration();
} // namespace

const QDemonTimeCounterFrequencyToTensOfNanos QDemonTime::sCounterFreq = QDemonTime::getCounterFrequency();

QDemonTimeCounterFrequencyToTensOfNanos QDemonTime::getCounterFrequency()
{
    quint64 freq;
    freq = std::chrono::high_resolution_clock::period::den;
    return QDemonTimeCounterFrequencyToTensOfNanos(QDemonTime::sNumTensOfNanoSecondsInASecond, freq);
}

quint64 QDemonTime::getCurrentCounterValue()
{
    quint64 ticks;
    ticks = std::chrono::high_resolution_clock::rep();
    return ticks;
}

QDemonTime::QDemonTime()
    : mTickCount(0)
{
    getElapsedSeconds();
}

QDemonTime::Second QDemonTime::getElapsedSeconds()
{
    qint64 lastTickCount = mTickCount;
    mTickCount = getTimeTicks();
    return (mTickCount - lastTickCount) * sTickDuration;
}

QDemonTime::Second QDemonTime::peekElapsedSeconds()
{
    return (getTimeTicks() - mTickCount) * sTickDuration;
}

QDemonTime::Second QDemonTime::getLastTime() const { return mTickCount * sTickDuration; }


QT_END_NAMESPACE
