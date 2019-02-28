#ifndef QDEMONNODE_H
#define QDEMONNODE_H

#include <QtQuick3d/qdemonobject.h>

#include <QVector3D>

#include <QtDemonRuntimeRender/qdemonrendereulerangles.h>

QT_BEGIN_NAMESPACE

class QDemonWindowPrivate;
class Q_QUICK3D_EXPORT QDemonNode : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(float x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(float y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(float z READ z WRITE setZ NOTIFY zChanged)
    Q_PROPERTY(QVector3D rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QVector3D pivot READ pivot WRITE setPivot NOTIFY pivotChanged)
    Q_PROPERTY(float opacity READ localOpacity WRITE setLocalOpacity NOTIFY localOpacityChanged)
    Q_PROPERTY(qint32 boneid READ skeletonId WRITE setSkeletonId NOTIFY skeletonIdChanged)
    Q_PROPERTY(RotationOrder rotationorder READ rotationOrder WRITE setRotationOrder NOTIFY rotationOrderChanged)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    enum RotationOrder {
        XYZ = EulOrdXYZs,
        YZX = EulOrdYZXs,
        ZXY = EulOrdZXYs,
        XZY = EulOrdXZYs,
        YXZ = EulOrdYXZs,
        ZYX = EulOrdZYXs,
        XYZr = EulOrdXYZr,
        YZXr = EulOrdYZXr,
        ZXYr = EulOrdZXYr,
        XZYr = EulOrdXZYr,
        YXZr = EulOrdYXZr,
        ZYXr = EulOrdZYXr
    };
    Q_ENUM(RotationOrder)

    enum Orientation {
        LeftHanded = 0,
        RightHanded
    };
    Q_ENUM(Orientation)
    QDemonNode();
    ~QDemonNode() override;

    float x() const;
    float y() const;
    float z() const;
    QVector3D rotation() const;
    QVector3D position() const;
    QVector3D scale() const;
    QVector3D pivot() const;
    float localOpacity() const;
    qint32 skeletonId() const;
    RotationOrder rotationOrder() const;
    Orientation orientation() const;
    bool visible() const;

    QDemonObject::Type type() const override;

public Q_SLOTS:
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    void setRotation(QVector3D rotation);
    void setPosition(QVector3D position);
    void setScale(QVector3D scale);
    void setPivot(QVector3D pivot);
    void setLocalOpacity(float opacity);
    void setSkeletonId(qint32 boneid);
    void setRotationOrder(RotationOrder rotationorder);
    void setOrientation(Orientation orientation);
    void setVisible(bool visible);

Q_SIGNALS:
    void xChanged(float x);
    void yChanged(float y);
    void zChanged(float z);
    void rotationChanged(QVector3D rotation);
    void positionChanged(QVector3D position);
    void scaleChanged(QVector3D scale);
    void pivotChanged(QVector3D pivot);
    void localOpacityChanged(float opacity);
    void skeletonIdChanged(qint32 boneid);
    void rotationOrderChanged(RotationOrder rotationorder);
    void orientationChanged(Orientation orientation);
    void visibleChanged(bool visible);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) override;

private:
    QVector3D m_rotation;
    QVector3D m_position;
    QVector3D m_scale { 1.0f, 1.0f, 1.0f };
    QVector3D m_pivot;
    float m_opacity = 1.0f;
    qint32 m_boneid = -1;
    RotationOrder m_rotationorder = XYZ;
    Orientation m_orientation = LeftHanded;
    bool m_visible = true;
    friend QDemonWindow;
    friend QDemonWindowPrivate;
};

QT_END_NAMESPACE

#endif // QDEMONNODE_H
