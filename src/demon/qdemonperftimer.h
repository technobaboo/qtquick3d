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
    struct Key {
        const char *id;
    };

    struct Entry {
        Entry(const QString &id)
            : tag(id)
        {}
        Entry() = default;

        void update(qint64 elapsed);
        void reset();
        QString toString(quint32 nFrames) const;

        quint32 count = 0;
        qint64 totalTime = 0;
        qint64 maxTime = 0;
        QString tag;
    };

private:
    bool m_isEnabled = false;
    int frameCount = 0;
    QMutex mutex;
    QHash<Key, Entry> entries;

public:
    QDemonPerfTimer();
    ~QDemonPerfTimer();

    // amount is in counter frequency units
    void update(const char *inTag, qint64 inAmount);

    // Dump current summation of timer data.
    void dump();
    void reset();

    int newFrame() { return ++frameCount; }

    void setEnabled(bool b) { m_isEnabled = b; }
    bool isEnabled() const { return m_isEnabled; }
};

struct QDemonStackPerfTimer
{
    QDemonPerfTimer *m_timer;
    QElapsedTimer elapsedTimer;
    const char *m_id;

    QDemonStackPerfTimer(QDemonPerfTimer *timer, const char *inId)
        : m_timer(timer), m_id(inId)
    {
        if (timer->isEnabled())
            elapsedTimer.start();
    }

    ~QDemonStackPerfTimer()
    {
        if (m_timer->isEnabled()) {
            qint64 elapsed = elapsedTimer.nsecsElapsed();
            m_timer->update(m_id, elapsed);
        }
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFTIMER_H
