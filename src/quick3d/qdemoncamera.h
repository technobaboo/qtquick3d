#ifndef QDEMONCAMERA_H
#define QDEMONCAMERA_H

#include <QtQuick3d/qdemonnode.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonCamera : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged)
    Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)
    Q_PROPERTY(bool isFieldOFViewHorizontal READ isFieldOFViewHorizontal WRITE setIsFieldOFViewHorizontal NOTIFY isFieldOFViewHorizontalChanged)
    Q_PROPERTY(CameraScaleModes scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(CameraScaleAnchors scaleAnchor READ scaleAnchor WRITE setScaleAnchor NOTIFY scaleAnchorChanged)
    Q_PROPERTY(float frustumScaleX READ frustumScaleX WRITE setFrustumScaleX NOTIFY frustumScaleXChanged)
    Q_PROPERTY(float frustumScaleY READ frustumScaleY WRITE setFrustumScaleY NOTIFY frustumScaleYChanged)

public:

    enum CameraScaleModes {
        Fit = 0,
        SameSize,
        FitHorizontal,
        FitVertical,
    };

    enum CameraScaleAnchors {
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
    QDemonCamera();

    float clipNear() const;
    float clipFar() const;
    float fieldOfView() const;
    bool isFieldOFViewHorizontal() const;
    CameraScaleModes scaleMode() const;
    CameraScaleAnchors scaleAnchor() const;
    float frustumScaleX() const;
    float frustumScaleY() const;
    QDemonObject::Type type() const override;

public Q_SLOTS:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setFieldOfView(float fieldOfView);

    void setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal);

    void setScaleMode(CameraScaleModes scaleMode);

    void setScaleAnchor(CameraScaleAnchors scaleAnchor);

    void setFrustumScaleX(float frustumScaleX);

    void setFrustumScaleY(float frustumScaleY);

Q_SIGNALS:
    void clipNearChanged(float clipNear);

    void clipFarChanged(float clipFar);

    void fieldOfViewChanged(float fieldOfView);

    void isFieldOFViewHorizontalChanged(bool isFieldOFViewHorizontal);

    void scaleModeChanged(CameraScaleModes scaleMode);

    void scaleAnchorChanged(CameraScaleAnchors scaleAnchor);

    void frustumScaleXChanged(float frustumScaleX);

    void frustumScaleYChanged(float frustumScaleY);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) override;

private:

    float m_clipNear = 10.0f;
    float m_clipFar = 10000.0f;
    float m_fieldOfView = 60.0f;
    float m_frustumScaleX = 0.0f;
    float m_frustumScaleY = 0.0f;
    CameraScaleModes m_scaleMode = CameraScaleModes::Fit;
    CameraScaleAnchors m_scaleAnchor = CameraScaleAnchors::Center;
    bool m_isFieldOFViewHorizontal = false;



};

QT_END_NAMESPACE


#endif // QDEMONCAMERA_H
