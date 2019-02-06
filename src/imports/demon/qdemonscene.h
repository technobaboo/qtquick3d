#ifndef QDEMONSCENE_H
#define QDEMONSCENE_H

#include <qdemonobject.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QColor>

class QDemonScene : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(bool useClearColor READ useClearColor WRITE setUseClearColor NOTIFY useClearColorChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
public:
    QDemonScene();

    bool useClearColor() const;
    QColor clearColor() const;

public slots:
    void setUseClearColor(bool useClearColor);
    void setClearColor(QColor clearColor);

Q_SIGNALS:
    void useClearColorChanged(bool useClearColor);
    void clearColorChanged(QColor clearColor);

protected:
    SGraphObject *updateSpacialNode(SGraphObject *node) override;

private:
    SScene *m_sceneNode;
    bool m_useClearColor;
    QColor m_clearColor;
};

#endif // QDEMONSCENE_H
