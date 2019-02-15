#ifndef QDEMONMATERIAL_H
#define QDEMONMATERIAL_H

#include <qdemonobject.h>
#include <qdemonimage.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonMaterial : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QDemonImage* lightmapIndirect READ lightmapIndirect WRITE setLightmapIndirect NOTIFY lightmapIndirectChanged)
    Q_PROPERTY(QDemonImage* lightmapRadiosity READ lightmapRadiosity WRITE setLightmapRadiosity NOTIFY lightmapRadiosityChanged)
    Q_PROPERTY(QDemonImage* lightmapShadow READ lightmapShadow WRITE setLightmapShadow NOTIFY lightmapShadowChanged)
    Q_PROPERTY(QDemonImage* iblProbe READ iblProbe WRITE setIblProbe NOTIFY iblProbeChanged)

    Q_PROPERTY(QDemonImage* emissiveMap2 READ emissiveMap2 WRITE setEmissiveMap2 NOTIFY emissiveMap2Changed)

    Q_PROPERTY(QDemonImage* displacementMap READ displacementMap WRITE setDisplacementMap NOTIFY displacementMapChanged)
    Q_PROPERTY(float displacementAmount READ displacementAmount WRITE setDisplacementAmount NOTIFY displacementAmountChanged)

public:
    QDemonMaterial();
    ~QDemonMaterial();

    QDemonObject::Type type() const = 0;

    QDemonImage *lightmapIndirect() const;
    QDemonImage *lightmapRadiosity() const;
    QDemonImage *lightmapShadow() const;
    QDemonImage *iblProbe() const;

    QDemonImage* emissiveMap2() const;

    QDemonImage* displacementMap() const;
    float displacementAmount() const;

public Q_SLOTS:
    void setLightmapIndirect(QDemonImage* lightmapIndirect);
    void setLightmapRadiosity(QDemonImage* lightmapRadiosity);
    void setLightmapShadow(QDemonImage* lightmapShadow);
    void setIblProbe(QDemonImage* iblProbe);

    void setEmissiveMap2(QDemonImage* emissiveMap2);

    void setDisplacementMap(QDemonImage* displacementMap);
    void setDisplacementAmount(float displacementAmount);

Q_SIGNALS:
    void lightmapIndirectChanged(QDemonImage* lightmapIndirect);
    void lightmapRadiosityChanged(QDemonImage* lightmapRadiosity);
    void lightmapShadowChanged(QDemonImage* lightmapShadow);
    void iblProbeChanged(QDemonImage* iblProbe);

    void emissiveMap2Changed(QDemonImage* emissiveMap2);

    void displacementMapChanged(QDemonImage* displacementMap);
    void displacementAmountChanged(float displacementAmount);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node);
private:
    QDemonImage* m_lightmapIndirect;
    QDemonImage* m_lightmapRadiosity;
    QDemonImage* m_lightmapShadow;
    QDemonImage* m_iblProbe;

    QDemonImage* m_emissiveMap2;

    QDemonImage* m_displacementMap;
    float m_displacementAmount;
};

QT_END_NAMESPACE

#endif // QDEMONMATERIAL_H
