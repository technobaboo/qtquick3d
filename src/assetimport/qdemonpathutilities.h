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

namespace QDemonPathUtilities
{
struct PathCommand
{
    enum Enum {
        Noner = 0,
        MoveTo, // 2 floats
        CubicCurveTo, // 6 floats, c1, c2, p2.  p1 is existing location
        Close, // 0 floats
    };
};

struct Q_DEMONASSETIMPORT_EXPORT SPathBuffer
{
    // 64 bit random number to uniquely identify this file type.
    static quint64 GetFileTag() { return 0x7b1a41633c43a6afULL; }
    static quint32 GetFileVersion() { return 1; }
    QDemonConstDataRef<PathCommand::Enum> m_Commands;
    QDemonConstDataRef<float> m_Data;
    SPathBuffer();
    void Save(QIODevice &outStream) const;
    static SPathBuffer *Load(QIODevice &inStream);
};

class Q_DEMONASSETIMPORT_EXPORT IPathBufferBuilder
{
public:
    virtual ~IPathBufferBuilder() {}
    virtual void Clear() = 0;

    virtual void MoveTo(const QVector2D &inPos) = 0;
    virtual void CubicCurveTo(const QVector2D &inC1, const QVector2D &inC2, const QVector2D &inP2) = 0;
    virtual void Close() = 0;
    // Points back to internal data structures, must use or copy.
    virtual SPathBuffer GetPathBuffer() = 0;

    static QSharedPointer<IPathBufferBuilder> CreateBuilder();
};
} // end namespace

QT_END_NAMESPACE

#endif // QDEMONPATHUTILITIES_H
