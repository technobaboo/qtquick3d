#ifndef QDEMONPATHUTILITIES_H
#define QDEMONPATHUTILITIES_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDemonAssetImport/qtdemonassetimportglobal.h>

#include <QtDemon/qdemondataref.h>

#include <QtCore/QIODevice>
#include <QtCore/QSharedPointer>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

namespace QDemonPathUtilities {

enum class PathCommand : quint8
{
    None = 0,
    MoveTo, // 2 floats
    CubicCurveTo, // 6 floats, c1, c2, p2.  p1 is existing location
    Close, // 0 floats
};

struct Q_DEMONASSETIMPORT_EXPORT QDemonPathBuffer
{
    // 64 bit random number to uniquely identify this file type.
    static quint64 getFileTag() { return 0x7b1a41633c43a6afULL; }
    static quint32 getFileVersion() { return 1; }
    QDemonConstDataRef<PathCommand> commands;
    QDemonConstDataRef<float> data;
    QDemonPathBuffer() = default;
    void save(QIODevice &outStream) const;
    static QDemonPathBuffer *load(QIODevice &inStream);
};

struct Q_DEMONASSETIMPORT_EXPORT QDemonPathBufferBuilder
{
    QVector<PathCommand> m_commands;
    QVector<float> m_data;

    void clear()
    {
        m_commands.clear();
        m_data.clear();
    }

    void push(const QVector2D &inPos)
    {
        m_data.push_back(inPos.x());
        m_data.push_back(inPos.y());
    }

    void moveTo(const QVector2D &inPos)
    {
        m_commands.push_back(PathCommand::MoveTo);
        push(inPos);
    }

    void cubicCurveTo(const QVector2D &inC1, const QVector2D &inC2, const QVector2D &inP2)
    {
        m_commands.push_back(PathCommand::CubicCurveTo);
        push(inC1);
        push(inC2);
        push(inP2);
    }

    void close() { m_commands.push_back(PathCommand::Close); }

    // Points back to internal data structures, must use or copy.
    QDemonPathBuffer getPathBuffer()
    {
        QDemonPathBuffer retval;
        retval.data = toConstDataRef(static_cast<const float *>(m_data.constData()), m_data.size());
        retval.commands = toConstDataRef(static_cast<const PathCommand *>(m_commands.constData()), m_commands.size());
        ;
        return retval;
    }
};

} // end namespace

QT_END_NAMESPACE

#endif // QDEMONPATHUTILITIES_H
