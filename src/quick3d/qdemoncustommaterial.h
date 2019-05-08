#ifndef QDEMONCUSTOMMATERIAL_H
#define QDEMONCUSTOMMATERIAL_H

#include <QtQuick3d/qdemonmaterial.h>
#include <QtCore/qvector.h>

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemcommands.h>

QT_BEGIN_NAMESPACE


class Q_QUICK3D_EXPORT QDemonCustomMaterialTexture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDemonImage * image READ image WRITE setImage)
    Q_PROPERTY(QByteArray name MEMBER name)
    Q_PROPERTY(TextureType type MEMBER type)
    Q_PROPERTY(bool enabled MEMBER enabled)

public:
    enum class TextureType
    {
        Unknown = 0,
        Diffuse,
        Specular,
        Environment,
        Bump,
        Normal,
        Displace,
        Emissive,
        Emissive2,
        Anisotropy,
        Translucent,
        LightmapIndirect,
        LightmapRadiosity,
        LightmapShadow
    };
    Q_ENUM(TextureType)

    QDemonCustomMaterialTexture() = default;
    virtual ~QDemonCustomMaterialTexture() = default;
    QDemonImage *m_image = nullptr;
    QByteArray name;
    TextureType type;
    bool enabled = true;
    QDemonImage *image() const
    {
        return m_image;
    }

public Q_SLOTS:
    void setImage(QDemonImage * image)
    {
        if (m_image == image)
            return;

        m_image = image;
        Q_EMIT textureDirty(this);
    }

Q_SIGNALS:
    void textureDirty(QDemonCustomMaterialTexture * texture);
};

Q_DECLARE_OPAQUE_POINTER(QDemonCustomMaterialTexture)

class Q_QUICK3D_EXPORT QDemonCustomMaterialRenderCommand : public QObject
{
    Q_OBJECT
public:
    QDemonCustomMaterialRenderCommand() = default;
    ~QDemonCustomMaterialRenderCommand() override = default;
    virtual dynamic::QDemonCommand *getCommand() { Q_ASSERT(0); return nullptr; }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialBufferInput : public QDemonCustomMaterialRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QByteArray bufferName MEMBER bufferName)
    Q_PROPERTY(QByteArray param MEMBER param)
public:
    QDemonCustomMaterialBufferInput() = default;
    ~QDemonCustomMaterialBufferInput() override = default;
    dynamic::QDemonApplyBufferValue command { QByteArray(), QByteArray() };
    QByteArray &bufferName = command.m_bufferName;
    QByteArray &param = command.m_paramName;
    dynamic::QDemonCommand *getCommand() override { return &command; }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialBufferBlit : public QDemonCustomMaterialRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QByteArray source MEMBER source)
    Q_PROPERTY(QByteArray destination MEMBER destination)
public:
    QDemonCustomMaterialBufferBlit() = default;
    ~QDemonCustomMaterialBufferBlit() override = default;
    dynamic::QDemonApplyBlitFramebuffer command { QByteArray(), QByteArray() };
    QByteArray &source = command.m_sourceBufferName;
    QByteArray &destination = command.m_destBufferName;
    dynamic::QDemonCommand *getCommand() override { return &command; }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialBlending : public QDemonCustomMaterialRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(SrcBlending srcBlending READ srcBlending WRITE setSrcBlending)
    Q_PROPERTY(DestBlending destBlending READ destBlending WRITE setDestBlending)

public:
    enum class SrcBlending
    {
        Unknown = 0,
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate
    };
    Q_ENUM(SrcBlending)

    enum class DestBlending
    {
        Unknown = 0,
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };
    Q_ENUM(DestBlending)

    QDemonCustomMaterialBlending() = default;
    ~QDemonCustomMaterialBlending() override = default;
    dynamic::QDemonApplyBlending command { QDemonRenderSrcBlendFunc::Unknown, QDemonRenderDstBlendFunc::Unknown };
    DestBlending destBlending() const
    {
        return DestBlending(command.m_dstBlendFunc);
    }
    SrcBlending srcBlending() const
    {
        return SrcBlending(command.m_srcBlendFunc);
    }

    dynamic::QDemonCommand *getCommand() override { return &command; }

public Q_SLOTS:
    void setDestBlending(DestBlending destBlending)
    {
        command.m_dstBlendFunc = QDemonRenderDstBlendFunc(destBlending);
    }
    void setSrcBlending(SrcBlending srcBlending)
    {
        command.m_srcBlendFunc= QDemonRenderSrcBlendFunc(srcBlending);
    }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialRenderState : public QDemonCustomMaterialRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(RenderState renderState READ renderState WRITE setRenderState)
    Q_PROPERTY(bool enabled MEMBER enabled)

public:
    enum class RenderState
    {
        Unknown = 0,
        Blend,
        CullFace,
        DepthTest,
        StencilTest,
        ScissorTest,
        DepthWrite,
        Multisample
    };
    Q_ENUM(RenderState)

    QDemonCustomMaterialRenderState() = default;
    ~QDemonCustomMaterialRenderState() override = default;
    dynamic::QDemonApplyRenderState command { QDemonRenderState::Unknown, false };
    bool &enabled = command.m_enabled;
    RenderState renderState() const
    {
        return RenderState(command.m_renderState);
    }

    dynamic::QDemonCommand *getCommand() override { return &command; }
public Q_SLOTS:
    void setRenderState(RenderState renderState)
    {
        command.m_renderState = QDemonRenderState(renderState);
    }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialRenderPass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QDemonCustomMaterialRenderCommand> commands READ commands)
public:
    QDemonCustomMaterialRenderPass() = default;
    ~QDemonCustomMaterialRenderPass() override = default;

    static void qmlAppendCommand(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, QDemonCustomMaterialRenderCommand *command);
    static QDemonCustomMaterialRenderCommand *qmlCommandAt(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, int index);
    static int qmlCommandCount(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list);

    QQmlListProperty<QDemonCustomMaterialRenderCommand> commands();
    QVector<QDemonCustomMaterialRenderCommand *> m_commands;
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialShaderInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray version MEMBER version)
    Q_PROPERTY(QByteArray type MEMBER type)
    Q_PROPERTY(qint32 shaderKey MEMBER shaderKey)
    Q_PROPERTY(qint32 layers MEMBER layers)
public:
    QDemonCustomMaterialShaderInfo() = default;
    ~QDemonCustomMaterialShaderInfo() override = default;
    QByteArray version;
    QByteArray type; // I.e., GLSL
    QByteArray shaderPrefix = QByteArrayLiteral("#include \"customMaterial.glsllib\"\n");

    enum class MaterialShaderKeyValues
    {
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Glossy = 1 << 2,
        Cutout = 1 << 3,
        Refraction = 1 << 4,
        Transparent = 1 << 5,
        Displace = 1 << 6,
        Volumetric = 1 << 7,
        Transmissive = 1 << 8,
    };
    Q_ENUM(MaterialShaderKeyValues)
    Q_DECLARE_FLAGS(MaterialShaderKeyFlags, MaterialShaderKeyValues)

    qint32 shaderKey {0};
    qint32 layers {0};
    bool isValid() const { return !(version.isEmpty() && type.isEmpty() && shaderPrefix.isEmpty()); }
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialShader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray shader MEMBER shader)
    Q_PROPERTY(Stage stage MEMBER stage)
public:
    QDemonCustomMaterialShader() = default;
    virtual ~QDemonCustomMaterialShader() = default;
    enum class Stage : quint8
    {
        Shared,
        Vertex,
        Fragment,
        Geometry
    };
    Q_ENUM(Stage)

    QByteArray shader;
    Stage stage;
};

class Q_QUICK3D_EXPORT QDemonCustomMaterial : public QDemonMaterial
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged) // NOT NEEDED
    Q_PROPERTY(bool hasTransparency READ hasTransparency WRITE setHasTransparency NOTIFY hasTransparencyChanged)
    Q_PROPERTY(bool hasRefraction READ hasRefraction WRITE setHasRefraction NOTIFY hasRefractionChanged)
    Q_PROPERTY(bool hasVolumetricDF READ hasVolumetricDF WRITE setHasVolumetricDF NOTIFY hasVolumetricDFChanged)
    Q_PROPERTY(bool alwaysDirty READ alwaysDirty WRITE setAlwaysDirty NOTIFY alwaysDirtyChanged)

    Q_PROPERTY(QDemonCustomMaterialShaderInfo *shaderInfo READ shaderInfo WRITE setShaderInfo)
    Q_PROPERTY(QQmlListProperty<QDemonCustomMaterialShader> shaders READ shaders)
    Q_PROPERTY(QQmlListProperty<QDemonCustomMaterialRenderPass> passes READ passes)


public:
    QDemonCustomMaterial();
    ~QDemonCustomMaterial() override;

    QDemonObject::Type type() const override;

    bool hasTransparency() const;
    bool hasRefraction() const;
    bool hasVolumetricDF() const;

    QString source() const;

    QDemonCustomMaterialShaderInfo *shaderInfo() const;

    QQmlListProperty<QDemonCustomMaterialShader> shaders();
    QQmlListProperty<QDemonCustomMaterialRenderPass> passes();

    bool alwaysDirty() const;

public Q_SLOTS:
    void setHasTransparency(bool hasTransparency);
    void setHasRefraction(bool hasRefraction);
    void setHasVolumetricDF(bool hasVolumetricDF);

    void setSource(QString source);
    void setShaderInfo(QDemonCustomMaterialShaderInfo *shaderInfo);

    void setAlwaysDirty(bool alwaysDirty);

Q_SIGNALS:
    void hasTransparencyChanged(bool hasTransparency);
    void hasRefractionChanged(bool hasRefraction);
    void hasVolumetricDFChanged(bool hasVolumetricDF);

    void sourceChanged(QString source);

    void alwaysDirtyChanged(bool alwaysDirty);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private Q_SLOTS:
    void onPropertyDirty();
    void onTextureDirty(QDemonCustomMaterialTexture *texture);

private:
    enum Dirty {
        TextureDirty = 0x1,
        PropertyDirty = 0x2
    };

    // Shader
    static void qmlAppendShader(QQmlListProperty<QDemonCustomMaterialShader> *list, QDemonCustomMaterialShader *shader);
    static QDemonCustomMaterialShader *qmlShaderAt(QQmlListProperty<QDemonCustomMaterialShader> *list, int index);
    static int qmlShaderCount(QQmlListProperty<QDemonCustomMaterialShader> *list);

    // Passes
    static void qmlAppendPass(QQmlListProperty<QDemonCustomMaterialRenderPass> *list, QDemonCustomMaterialRenderPass *pass);
    static QDemonCustomMaterialRenderPass *qmlPassAt(QQmlListProperty<QDemonCustomMaterialRenderPass> *list, int index);
    static int qmlPassCount(QQmlListProperty<QDemonCustomMaterialRenderPass> *list);

    void markDirty(QDemonCustomMaterial::Dirty type)
    {
        if (!(m_dirtyAttributes & quint32(type))) {
            m_dirtyAttributes |= quint32(type);
            update();
        }
    }

    quint32 m_dirtyAttributes = 0xffffffff;
    bool m_hasTransparency = false;
    bool m_hasRefraction = false;
    bool m_hasVolumetricDF = false;
    QString m_source;
    QDemonCustomMaterialShaderInfo *m_shaderInfo;
    QVector<QDemonCustomMaterialShader *> m_shaders;
    QVector<QDemonCustomMaterialRenderPass *> m_passes;
    bool m_alwaysDirty = false;
};

QT_END_NAMESPACE

#endif // QDEMONCUSTOMMATERIAL_H
