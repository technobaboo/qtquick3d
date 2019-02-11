#ifndef QDEMONSCENE_H
#define QDEMONSCENE_H

#include <QtQuick3d/qdemonobject.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonScene : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(bool useClearColor READ useClearColor WRITE setUseClearColor NOTIFY useClearColorChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
public:
    QDemonScene();
    ~QDemonScene() override;

    QDemonObject::Type type() const override;

    bool useClearColor() const;
    QColor clearColor() const;

public Q_SLOTS:
    void setUseClearColor(bool useClearColor);
    void setClearColor(QColor clearColor);

Q_SIGNALS:
    void useClearColorChanged(bool useClearColor);
    void clearColorChanged(QColor clearColor);

protected:
    SGraphObject *updateSpatialNode(SGraphObject *node) override;

private:
    SScene *m_sceneNode;
    bool m_useClearColor;
    QColor m_clearColor;

};

QT_END_NAMESPACE

#endif // QDEMONSCENE_H
