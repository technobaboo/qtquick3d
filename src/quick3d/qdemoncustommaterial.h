#ifndef QDEMONCUSTOMMATERIAL_H
#define QDEMONCUSTOMMATERIAL_H

#include <QtQuick3d/qdemonmaterial.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonCustomMaterial : public QDemonMaterial
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool hasTransparency READ hasTransparency WRITE setHasTransparency NOTIFY hasTransparencyChanged)
    Q_PROPERTY(bool hasRefraction READ hasRefraction WRITE setHasRefraction NOTIFY hasRefractionChanged)
    Q_PROPERTY(bool hasVolumetricDF READ hasVolumetricDF WRITE setHasVolumetricDF NOTIFY hasVolumetricDFChanged)

public:
    QDemonCustomMaterial();
    ~QDemonCustomMaterial() override;

    QDemonObject::Type type() const override;

    bool hasTransparency() const;
    bool hasRefraction() const;
    bool hasVolumetricDF() const;

    QString source() const;

public Q_SLOTS:
    void setHasTransparency(bool hasTransparency);
    void setHasRefraction(bool hasRefraction);
    void setHasVolumetricDF(bool hasVolumetricDF);

    void setSource(QString source);

Q_SIGNALS:
    void hasTransparencyChanged(bool hasTransparency);
    void hasRefractionChanged(bool hasRefraction);
    void hasVolumetricDFChanged(bool hasVolumetricDF);

    void sourceChanged(QString source);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    bool m_hasTransparency;
    bool m_hasRefraction;
    bool m_hasVolumetricDF;
    QString m_source;
};

QT_END_NAMESPACE

#endif // QDEMONCUSTOMMATERIAL_H
