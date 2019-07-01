#ifndef QDEMONCAMERA_H
#define QDEMONCAMERA_H

#include <QtQuick3d/qdemonnode.h>

QT_BEGIN_NAMESPACE

struct QDemonRenderCamera;
class Q_QUICK3D_EXPORT QDemonCamera : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged)
    Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)
    Q_PROPERTY(bool isFieldOFViewHorizontal READ isFieldOFViewHorizontal WRITE setIsFieldOFViewHorizontal NOTIFY isFieldOFViewHorizontalChanged)
    Q_PROPERTY(QDemonCameraProjectionMode projectionMode READ projectionMode WRITE setProjectionMode NOTIFY projectionModeChanged)
    Q_PROPERTY(QDemonCameraScaleModes scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(QDemonCameraScaleAnchors scaleAnchor READ scaleAnchor WRITE setScaleAnchor NOTIFY scaleAnchorChanged)
    Q_PROPERTY(float frustumScaleX READ frustumScaleX WRITE setFrustumScaleX NOTIFY frustumScaleXChanged)
    Q_PROPERTY(float frustumScaleY READ frustumScaleY WRITE setFrustumScaleY NOTIFY frustumScaleYChanged)

public:
    enum QDemonCameraScaleModes {
        Fit = 0,
        SameSize,
        FitHorizontal,
        FitVertical,
    };
    Q_ENUM(QDemonCameraScaleModes)

    enum QDemonCameraScaleAnchors {
        Center = 0,
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
    };
    Q_ENUM(QDemonCameraScaleAnchors)

    enum QDemonCameraProjectionMode {
        Perspective,
        Orthographic
    };
    Q_ENUM(QDemonCameraProjectionMode)

    QDemonCamera();

    float clipNear() const;
    float clipFar() const;
    float fieldOfView() const;
    bool isFieldOFViewHorizontal() const;
    QDemonCameraScaleModes scaleMode() const;
    QDemonCameraScaleAnchors scaleAnchor() const;
    float frustumScaleX() const;
    float frustumScaleY() const;
    QDemonObject::Type type() const override;

    QDemonRenderCamera *getCameraNode() const;

    QDemonCameraProjectionMode projectionMode() const;

    Q_INVOKABLE QVector3D worldToViewport(const QVector3D &worldPos) const;

public Q_SLOTS:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setFieldOfView(float fieldOfView);

    void setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal);

    void setScaleMode(QDemonCameraScaleModes scaleMode);

    void setScaleAnchor(QDemonCameraScaleAnchors scaleAnchor);

    void setFrustumScaleX(float frustumScaleX);

    void setFrustumScaleY(float frustumScaleY);

    void setProjectionMode(QDemonCameraProjectionMode projectionMode);

Q_SIGNALS:
    void clipNearChanged(float clipNear);

    void clipFarChanged(float clipFar);

    void fieldOfViewChanged(float fieldOfView);

    void isFieldOFViewHorizontalChanged(bool isFieldOFViewHorizontal);

    void scaleModeChanged(QDemonCameraScaleModes scaleMode);

    void scaleAnchorChanged(QDemonCameraScaleAnchors scaleAnchor);

    void frustumScaleXChanged(float frustumScaleX);

    void frustumScaleYChanged(float frustumScaleY);

    void projectionModeChanged(QDemonCameraProjectionMode projectionMode);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    float m_clipNear = 10.0f;
    float m_clipFar = 10000.0f;
    float m_fieldOfView = 60.0f;
    float m_frustumScaleX = 0.0f;
    float m_frustumScaleY = 0.0f;
    QDemonCameraScaleModes m_scaleMode = QDemonCameraScaleModes::Fit;
    QDemonCameraScaleAnchors m_scaleAnchor = QDemonCameraScaleAnchors::Center;
    bool m_isFieldOFViewHorizontal = false;

    QDemonRenderCamera *m_cameraNode = nullptr;
    QDemonCameraProjectionMode m_projectionMode;
};

QT_END_NAMESPACE

#endif // QDEMONCAMERA_H
