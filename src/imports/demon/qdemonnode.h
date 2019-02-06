#ifndef QDEMONNODE_H
#define QDEMONNODE_H

#include <qdemonobject.h>

#include <QVector3D>

#include <QtDemonRuntimeRender/qdemonrendereulerangles.h>

class QDemonNode : public QDemonObject
{
    Q_OBJECT
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

public slots:
    void setRotation(QVector3D rotation);
    void setPosition(QVector3D position);
    void setScale(QVector3D scale);
    void setPivot(QVector3D pivot);
    void setLocalOpacity(float opacity);
    void setSkeletonId(qint32 boneid);
    void setRotationOrder(RotationOrder rotationorder);
    void setOrientation(Orientation orientation);
    void setVisible(bool visible);

signals:
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
    SGraphObject *updateSpacialNode(SGraphObject *node) override;

private:
    QVector3D m_rotation;
    QVector3D m_position;
    QVector3D m_scale;
    QVector3D m_pivot;
    float m_opacity;
    qint32 m_boneid;
    RotationOrder m_rotationorder;
    Orientation m_orientation;
    bool m_visible;
};

#endif // QDEMONNODE_H
