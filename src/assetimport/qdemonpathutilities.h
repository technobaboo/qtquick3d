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

struct Q_DEMONASSETIMPORT_EXPORT QDemonPathBuffer
{
    // 64 bit random number to uniquely identify this file type.
    static quint64 getFileTag() { return 0x7b1a41633c43a6afULL; }
    static quint32 getFileVersion() { return 1; }
    QDemonConstDataRef<PathCommand::Enum> commands;
    QDemonConstDataRef<float> data;
    QDemonPathBuffer() = default;
    void save(QIODevice &outStream) const;
    static QDemonPathBuffer *load(QIODevice &inStream);
};

class Q_DEMONASSETIMPORT_EXPORT QDemonPathBufferBuilderInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonPathBufferBuilderInterface();
    virtual void clear() = 0;

    virtual void moveTo(const QVector2D &inPos) = 0;
    virtual void cubicCurveTo(const QVector2D &inC1, const QVector2D &inC2, const QVector2D &inP2) = 0;
    virtual void close() = 0;
    // Points back to internal data structures, must use or copy.
    virtual QDemonPathBuffer getPathBuffer() = 0;

    static QDemonRef<QDemonPathBufferBuilderInterface> createBuilder();
};
} // end namespace

QT_END_NAMESPACE

#endif // QDEMONPATHUTILITIES_H
