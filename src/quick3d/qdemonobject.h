#ifndef QDEMONOBJECT_H
#define QDEMONOBJECT_H

#include <QtQuick3d/qtquick3dglobal.h>
#include <QtCore/QObject>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QDemonObject::Type type READ type CONSTANT)
public:
    enum Type {
        Unknown = 0,
        Presentation,
        Scene,
        Node,
        Layer,
        Light,
        Camera,
        Model,
        DefaultMaterial,
        Image,
        Text,
        Effect,
        CustomMaterial,
        RenderPlugin,
        ReferencedMaterial,
        Path,
        PathSubPath,
        Lightmaps,
        LastKnownGraphObjectType,
    };
    Q_ENUM(Type)

    explicit QDemonObject(QObject *parent = nullptr);
    virtual ~QDemonObject();

    void update();

    QByteArray id() const;
    QString name() const;
    virtual QDemonObject::Type type() const = 0;

public Q_SLOTS:
    void setName(QString name);

protected:
    virtual QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) = 0;

private:
    void markDirty();
    void addToDirtyList();
    void removeFromDirtyList();
    QByteArray m_id;
    QString m_name;
};

QT_END_NAMESPACE

#endif // QDEMONOBJECT_H
