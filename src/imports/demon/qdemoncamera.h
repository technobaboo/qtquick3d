#ifndef QDEMONCAMERA_H
#define QDEMONCAMERA_H

#include <qdemonnode.h>

class QDemonCamera : public QDemonNode
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

public slots:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setFieldOfView(float fieldOfView);

    void setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal);

    void setScaleMode(CameraScaleModes scaleMode);

    void setScaleAnchor(CameraScaleAnchors scaleAnchor);

    void setFrustumScaleX(float frustumScaleX);

    void setFrustumScaleY(float frustumScaleY);

signals:
    void clipNearChanged(float clipNear);

    void clipFarChanged(float clipFar);

    void fieldOfViewChanged(float fieldOfView);

    void isFieldOFViewHorizontalChanged(bool isFieldOFViewHorizontal);

    void scaleModeChanged(CameraScaleModes scaleMode);

    void scaleAnchorChanged(CameraScaleAnchors scaleAnchor);

    void frustumScaleXChanged(float frustumScaleX);

    void frustumScaleYChanged(float frustumScaleY);

protected:
    SGraphObject *updateSpatialNode(SGraphObject *node) override;

private:

    float m_clipNear;
    float m_clipFar;
    float m_fieldOfView;
    bool m_isFieldOFViewHorizontal;
    CameraScaleModes m_scaleMode;
    CameraScaleAnchors m_scaleAnchor;
    float m_frustumScaleX;
    float m_frustumScaleY;



};



#endif // QDEMONCAMERA_H
