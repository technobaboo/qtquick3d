#ifndef QDEMONCUSTOMMATERIAL_H
#define QDEMONCUSTOMMATERIAL_H

#include <QtQuick3d/qdemonmaterial.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonCustomMaterial : public QDemonMaterial
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

public Q_SLOTS:
    void setHasTransparency(bool hasTransparency);
    void setHasRefraction(bool hasRefraction);
    void setHasVolumetricDF(bool hasVolumetricDF);

Q_SIGNALS:
    void hasTransparencyChanged(bool hasTransparency);
    void hasRefractionChanged(bool hasRefraction);
    void hasVolumetricDFChanged(bool hasVolumetricDF);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) override;

private:
    bool m_hasTransparency;
    bool m_hasRefraction;
    bool m_hasVolumetricDF;
};

QT_END_NAMESPACE

#endif // QDEMONCUSTOMMATERIAL_H
