#ifndef QDEMONPERFTIMER_H
#define QDEMONPERFTIMER_H

#include <QtDemon/qtdemonglobal.h>

#include <QtCore/QVector>
#include <QtCore/qmutex.h>
#include <QtCore/qhash.h>
#include <QtCore/qelapsedtimer.h>

QT_BEGIN_NAMESPACE

struct QDemonTimerEntry;

class Q_DEMON_EXPORT QDemonPerfTimer
{
    Q_DISABLE_COPY(QDemonPerfTimer)
public:
    QAtomicInt ref;

    struct Key {
        const char *id;
    };

    struct Entry {
        Entry(const QString &id)
            : tag(id)
        {}

        void update(qint64 elapsed);
        void reset();
        QString toString(quint32 nFrames) const;

        quint32 count = 0;
        qint64 totalTime = 0;
        qint64 maxTime = 0;
        QString tag;
    };

private:
    QMutex mutex;
    QHash<Key, Entry> entries;

public:
    QDemonPerfTimer();
    ~QDemonPerfTimer();

    // amount is in counter frequency units
    void update(const char *inTag, qint64 inAmount);

    // Dump current summation of timer data.
    void dump(quint32 nFrames = 0);
    void reset();
};

struct QDemonStackPerfTimer
{
    QDemonRef<QDemonPerfTimer> m_timer;
    QElapsedTimer elapsedTimer;
    const char *m_id;

    QDemonStackPerfTimer(const QDemonRef<QDemonPerfTimer> &destination, const char *inId)
        : m_timer(destination), m_id(inId)
    {
        elapsedTimer.start();
    }

    ~QDemonStackPerfTimer()
    {
        if (m_timer) {
            qint64 elapsed = elapsedTimer.nsecsElapsed();
            m_timer->update(m_id, elapsed);
        }
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFTIMER_H
