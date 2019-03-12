#ifndef QDEMONEFFECT_H
#define QDEMONEFFECT_H

#include <QtQuick3d/qdemonobject.h>
QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonEffect : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    QDemonEffect();
    ~QDemonEffect() override;

    QDemonObject::Type type() const override;

    QString source() const;

public Q_SLOTS:
    void setSource(QString source);

Q_SIGNALS:
    void sourceChanged(QString source);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    QString m_source;
};

QT_END_NAMESPACE
#endif // QDEMONEFFECT_H
