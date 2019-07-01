#ifndef QDEMONMODEL_H
#define QDEMONMODEL_H

#include <QtQuick3d/qdemonnode.h>
#include <QtQml/QQmlListProperty>
#include <QtQuick3d/QDemonMaterial>
#include <QtCore/QVector>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonModel : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int skeletonRoot READ skeletonRoot WRITE setSkeletonRoot NOTIFY skeletonRootChanged)
    Q_PROPERTY(QDemonTessModeValues tesselationMode READ tesselationMode WRITE setTesselationMode NOTIFY tesselationModeChanged)
    Q_PROPERTY(float edgeTess READ edgeTess WRITE setEdgeTess NOTIFY edgeTessChanged)
    Q_PROPERTY(float innerTess READ innerTess WRITE setInnerTess NOTIFY innerTessChanged)
    Q_PROPERTY(bool isWireframeMode READ isWireframeMode WRITE setIsWireframeMode NOTIFY isWireframeModeChanged)
    Q_PROPERTY(QQmlListProperty<QDemonMaterial> materials READ materials)

public:
    enum QDemonTessModeValues {
        NoTess = 0,
        TessLinear = 1,
        TessPhong = 2,
        TessNPatch = 3,
    };
    Q_ENUM(QDemonTessModeValues)


    QDemonModel();
    ~QDemonModel() override;

    QDemonObject::Type type() const override;

    QUrl source() const;
    int skeletonRoot() const;
    QDemonTessModeValues tesselationMode() const;
    float edgeTess() const;
    float innerTess() const;
    bool isWireframeMode() const;

    QQmlListProperty<QDemonMaterial> materials();

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setSkeletonRoot(int skeletonRoot);
    void setTesselationMode(QDemonTessModeValues tesselationMode);
    void setEdgeTess(float edgeTess);
    void setInnerTess(float innerTess);
    void setIsWireframeMode(bool isWireframeMode);

Q_SIGNALS:
    void sourceChanged(const QUrl &source);
    void skeletonRootChanged(int skeletonRoot);
    void tesselationModeChanged(QDemonTessModeValues tesselationMode);
    void edgeTessChanged(float edgeTess);
    void innerTessChanged(float innerTess);
    void isWireframeModeChanged(bool isWireframeMode);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    enum QDemonModelDirtyType {
        SourceDirty =           0x00000001,
        SkeletonRootDirty =     0x00000002,
        TesselationModeDirty =  0x00000004,
        TesselationEdgeDirty =  0x00000008,
        TesselationInnerDirty = 0x00000010,
        WireframeDirty =        0x00000020,
        MaterialsDirty =        0x00000040
    };

    QString translateSource();
    QUrl m_source;
    int m_skeletonRoot = -1;
    QDemonTessModeValues m_tesselationMode = QDemonTessModeValues::NoTess;
    float m_edgeTess = 1.0f;
    float m_innerTess = 1.0f;
    bool m_isWireframeMode = false;

    quint32 m_dirtyAttributes = 0xffffffff; // all dirty by default
    void markDirty(QDemonModelDirtyType type);

    static void qmlAppendMaterial(QQmlListProperty<QDemonMaterial> *list, QDemonMaterial *material);
    static QDemonMaterial *qmlMaterialAt(QQmlListProperty<QDemonMaterial> *list, int index);
    static int qmlMaterialsCount(QQmlListProperty<QDemonMaterial> *list);
    static void qmlClearMaterials(QQmlListProperty<QDemonMaterial> *list);

    QVector<QDemonMaterial *> m_materials;
};

QT_END_NAMESPACE

#endif // QDEMONMODEL_H
