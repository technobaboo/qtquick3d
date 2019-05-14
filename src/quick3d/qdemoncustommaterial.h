#ifndef QDEMONCUSTOMMATERIAL_H
#define QDEMONCUSTOMMATERIAL_H

#include <QtQuick3d/qdemonmaterial.h>
#include <QtCore/qvector.h>

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemcommands.h>

QT_BEGIN_NAMESPACE

class QDemonCustomMaterialShader;

class Q_QUICK3D_EXPORT QDemonCustomMaterialTexture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDemonImage * image READ image WRITE setImage)
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

//    enum UsageType {
//        Diffuse,
//        Specular,
//        Roughness,
//        Bump,
//        Environment,
//        Shadow,
//        Displacement,
//        Emissive,
//        EmissiveMask,
//        Anisotropy,
//        Gradient,
//        Storage,
//        Brush,
//        Cutout,
//        Transmission
//    };
//    Q_ENUM(UsageType)

    QDemonCustomMaterialTexture() = default;
    virtual ~QDemonCustomMaterialTexture() = default;
    QDemonImage *m_image = nullptr;
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

        QObject *p = parent();
        while (p != nullptr) {
            if (QDemonMaterial *mat = qobject_cast<QDemonMaterial *>(p)) {
                mat->setDynamicTextureMap(image);
                break;
            }
        }

        m_image = image;
        Q_EMIT textureDirty(this);
    }

Q_SIGNALS:
    void textureDirty(QDemonCustomMaterialTexture * texture);
};

class Q_QUICK3D_EXPORT QDemonCustomMaterialBuffer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TextureFormat format READ format WRITE setFormat)
    Q_PROPERTY(MagnifyingOp magOp READ filterOp WRITE setFilterOp)
    Q_PROPERTY(TextureCoordOp coordOp READ texCoordOp WRITE setTexCoordOp)
    Q_PROPERTY(float sizeMultiplier MEMBER sizeMultiplier)
    Q_PROPERTY(AllocateBufferFlagValues bufferFlags READ bufferFlags WRITE setBufferFlags)
    Q_PROPERTY(QByteArray name MEMBER name)
public:
    QDemonCustomMaterialBuffer() = default;
    ~QDemonCustomMaterialBuffer() override = default;

    enum class Type
    {
        FrameBuffer,
        UserDefined
    };
    Q_ENUM(Type)

    enum class MagnifyingOp
    {
        Unknown = 0,
        Nearest,
        Linear
    };
    Q_ENUM(MagnifyingOp)

    enum class TextureCoordOp
    {

        Unknown = 0,
        ClampToEdge,
        MirroredRepeat,
        Repeat
    };
    Q_ENUM(TextureCoordOp)

    enum class AllocateBufferFlagValues
    {
        None = 0,
        SceneLifetime = 1,
    };
    Q_ENUM(AllocateBufferFlagValues)

    enum class TextureType {
        Ubyte,
        Ushort,
        FP16,
        FP32
    };

    enum class TextureFormat {
        Unknown = 0,
        R8,
        R16,
        R16F,
        R32I,
        R32UI,
        R32F,
        RG8,
        RGBA8,
        RGB8,
        SRGB8,
        SRGB8A8,
        RGB565,
        RGBA5551,
        Alpha8,
        Luminance8,
        Luminance16,
        LuminanceAlpha8,
        RGBA16F,
        RG16F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11G11B10,
        RGB9E5,
        RGBA_DXT1,
        RGB_DXT1,
        RGBA_DXT3,
        RGBA_DXT5,
        Depth16,
        Depth24,
        Depth32,
        Depth24Stencil8
    };
    Q_ENUM(TextureFormat)

    dynamic::QDemonAllocateBuffer command {};
    MagnifyingOp filterOp() const { return MagnifyingOp(command.m_filterOp); }
    void setFilterOp(MagnifyingOp magOp) { command.m_filterOp = QDemonRenderTextureMagnifyingOp(magOp); }

    TextureCoordOp texCoordOp() const { return TextureCoordOp(command.m_texCoordOp); }
    void setTexCoordOp(TextureCoordOp texCoordOp) { command.m_texCoordOp = QDemonRenderTextureCoordOp(texCoordOp); }
    float &sizeMultiplier = command.m_sizeMultiplier;
    dynamic::QDemonCommand *getCommand() { return &command; }

    TextureFormat format() const { return TextureFormat(command.m_format.format); }
    void setFormat(TextureFormat format)
    {
        command.m_format = QDemonRenderTextureFormat::Format(format);
    }

    AllocateBufferFlagValues bufferFlags() const { return AllocateBufferFlagValues(int(command.m_bufferFlags)); }
    void setBufferFlags(AllocateBufferFlagValues flag) { command.m_bufferFlags = quint32(flag);}

    QByteArray &name = command.m_name;
};

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
    Q_PROPERTY(QDemonCustomMaterialBuffer *buffer READ buffer WRITE setBuffer)
    Q_PROPERTY(QByteArray bufferName MEMBER bufferName)
    Q_PROPERTY(QByteArray param MEMBER param)
public:
    QDemonCustomMaterialBufferInput() = default;
    ~QDemonCustomMaterialBufferInput() override = default;
    dynamic::QDemonApplyBufferValue command { QByteArray(), QByteArray() };
    QByteArray &bufferName = command.m_bufferName;
    QByteArray &param = command.m_paramName;
    dynamic::QDemonCommand *getCommand() override { return &command; }

    QDemonCustomMaterialBuffer *buffer() const { return m_buffer; }
    void setBuffer(QDemonCustomMaterialBuffer *buffer) {
        if (m_buffer == buffer)
            return;

        if (buffer) {
            Q_ASSERT(!buffer->name.isEmpty());
            command.m_bufferName = buffer->name;
        }
        m_buffer = buffer;
    }

    QDemonCustomMaterialBuffer *m_buffer = nullptr;

};

class Q_QUICK3D_EXPORT QDemonCustomMaterialBufferBlit : public QDemonCustomMaterialRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QDemonCustomMaterialBuffer *source READ source WRITE setSource)
    Q_PROPERTY(QDemonCustomMaterialBuffer *destination READ destination WRITE setDestination)
public:
    QDemonCustomMaterialBufferBlit() = default;
    ~QDemonCustomMaterialBufferBlit() override = default;
    dynamic::QDemonApplyBlitFramebuffer command { QByteArray(), QByteArray() };
    dynamic::QDemonCommand *getCommand() override { return &command; }

    QDemonCustomMaterialBuffer *source() const { return m_source; }
    void setSource(QDemonCustomMaterialBuffer *src)
    {
        if (src == m_source)
            return;

        if (src) {
            Q_ASSERT(!src->name.isEmpty());
            command.m_sourceBufferName = src->name;
        }
        m_source = src;
    }

    QDemonCustomMaterialBuffer *destination() const { return m_destination; }
    void setDestination(QDemonCustomMaterialBuffer *dest)
    {
        if (dest == m_destination)
            return;

        if (dest) {
            Q_ASSERT(!dest->name.isEmpty());
            command.m_destBufferName = dest->name;
        }
        m_destination = dest;
    }

    QDemonCustomMaterialBuffer *m_source = nullptr;
    QDemonCustomMaterialBuffer *m_destination = nullptr;
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
    Q_PROPERTY(QDemonCustomMaterialBuffer *output MEMBER outputBuffer)
    Q_PROPERTY(QDemonCustomMaterialShader *shader MEMBER shader)
public:
    QDemonCustomMaterialRenderPass() = default;
    ~QDemonCustomMaterialRenderPass() override = default;

    static void qmlAppendCommand(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, QDemonCustomMaterialRenderCommand *command);
    static QDemonCustomMaterialRenderCommand *qmlCommandAt(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, int index);
    static int qmlCommandCount(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list);

    QQmlListProperty<QDemonCustomMaterialRenderCommand> commands();
    QVector<QDemonCustomMaterialRenderCommand *> m_commands;
    QDemonCustomMaterialBuffer *outputBuffer = nullptr;
    QDemonCustomMaterialShader *shader = nullptr;
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
    Q_PROPERTY(bool hasTransparency READ hasTransparency WRITE setHasTransparency NOTIFY hasTransparencyChanged)
    Q_PROPERTY(bool hasRefraction READ hasRefraction WRITE setHasRefraction NOTIFY hasRefractionChanged)
    Q_PROPERTY(bool hasVolumetricDF READ hasVolumetricDF WRITE setHasVolumetricDF NOTIFY hasVolumetricDFChanged)
    Q_PROPERTY(bool alwaysDirty READ alwaysDirty WRITE setAlwaysDirty NOTIFY alwaysDirtyChanged)

    Q_PROPERTY(QDemonCustomMaterialShaderInfo *shaderInfo READ shaderInfo WRITE setShaderInfo)
    Q_PROPERTY(QQmlListProperty<QDemonCustomMaterialRenderPass> passes READ passes)

public:
    QDemonCustomMaterial();
    ~QDemonCustomMaterial() override;

    QDemonObject::Type type() const override;

    bool hasTransparency() const;
    bool hasRefraction() const;
    bool hasVolumetricDF() const;


    QDemonCustomMaterialShaderInfo *shaderInfo() const;
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
    QVector<QDemonCustomMaterialRenderPass *> m_passes;
    bool m_alwaysDirty = false;
};

QT_END_NAMESPACE

#endif // QDEMONCUSTOMMATERIAL_H
