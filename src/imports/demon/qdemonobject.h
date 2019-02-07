#ifndef QDEMONOBJECT_H
#define QDEMONOBJECT_H

#include <QtCore/QObject>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

class QDemonObject : public QObject
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

public slots:
    void setName(QString name);

protected:
    virtual SGraphObject *updateSpatialNode(SGraphObject *node) = 0;

private:
    void markDirty();
    void addToDirtyList();
    void removeFromDirtyList();
    QByteArray m_id;
    QString m_name;
};

#endif // QDEMONOBJECT_H
