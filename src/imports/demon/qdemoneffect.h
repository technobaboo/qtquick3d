#ifndef QDEMONEFFECT_H
#define QDEMONEFFECT_H

#include <qdemonobject.h>

class QDemonEffect : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    QDemonEffect();
    ~QDemonEffect() override;

    QDemonObject::Type type() const override;

    QString source() const;

public slots:
    void setSource(QString source);

signals:
    void sourceChanged(QString source);

protected:
    SGraphObject *updateSpatialNode(SGraphObject *node) override;

private:

    QString m_source;
};

#endif // QDEMONEFFECT_H
