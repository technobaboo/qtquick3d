#ifndef QDEMONCUSTOMMATERIAL_H
#define QDEMONCUSTOMMATERIAL_H

#include <qdemonmaterial.h>

class QDemonCustomMaterial : public QDemonMaterial
{
    Q_OBJECT
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

public slots:
    void setHasTransparency(bool hasTransparency);
    void setHasRefraction(bool hasRefraction);
    void setHasVolumetricDF(bool hasVolumetricDF);

signals:
    void hasTransparencyChanged(bool hasTransparency);
    void hasRefractionChanged(bool hasRefraction);
    void hasVolumetricDFChanged(bool hasVolumetricDF);

protected:
    SGraphObject *updateSpatialNode(SGraphObject *node) override;

private:
    bool m_hasTransparency;
    bool m_hasRefraction;
    bool m_hasVolumetricDF;
};

#endif // QDEMONCUSTOMMATERIAL_H
