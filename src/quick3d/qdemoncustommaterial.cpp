#include "qdemoncustommaterial.h"
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

#include "qdemonobject_p.h"
#include "qdemonview3d.h"

QT_BEGIN_NAMESPACE

QDemonCustomMaterial::QDemonCustomMaterial() {}

QDemonCustomMaterial::~QDemonCustomMaterial() {}

QDemonObject::Type QDemonCustomMaterial::type() const
{
    return QDemonObject::CustomMaterial;
}

bool QDemonCustomMaterial::hasTransparency() const
{
    return m_hasTransparency;
}

bool QDemonCustomMaterial::hasRefraction() const
{
    return m_hasRefraction;
}

bool QDemonCustomMaterial::hasVolumetricDF() const
{
    return m_hasVolumetricDF;
}

QString QDemonCustomMaterial::source() const
{
    return m_source;
}

QDemonCustomMaterialShaderInfo *QDemonCustomMaterial::shaderInfo() const
{
    return m_shaderInfo;
}

QQmlListProperty<QDemonCustomMaterialShader> QDemonCustomMaterial::shaders()
{
    return QQmlListProperty<QDemonCustomMaterialShader>(this,
                                                        nullptr,
                                                        QDemonCustomMaterial::qmlAppendShader,
                                                        QDemonCustomMaterial::qmlShaderCount,
                                                        QDemonCustomMaterial::qmlShaderAt,
                                                        nullptr);
}

QQmlListProperty<QDemonCustomMaterialTexture> QDemonCustomMaterial::textures()
{
    return QQmlListProperty<QDemonCustomMaterialTexture>(this,
                                                         nullptr,
                                                         QDemonCustomMaterial::qmlAppendTexture,
                                                         QDemonCustomMaterial::qmlTextureCount,
                                                         QDemonCustomMaterial::qmlTextureAt,
                                                         nullptr);
}

QQmlListProperty<QDemonCustomMaterialRenderPass> QDemonCustomMaterial::passes()
{
    return QQmlListProperty<QDemonCustomMaterialRenderPass>(this,
                                                            nullptr,
                                                            QDemonCustomMaterial::qmlAppendPass,
                                                            QDemonCustomMaterial::qmlPassCount,
                                                            QDemonCustomMaterial::qmlPassAt,
                                                            nullptr);
}

bool QDemonCustomMaterial::alwaysDirty() const
{
    return m_alwaysDirty;
}

void QDemonCustomMaterial::setHasTransparency(bool hasTransparency)
{
    if (m_hasTransparency == hasTransparency)
        return;

    m_hasTransparency = hasTransparency;
    emit hasTransparencyChanged(m_hasTransparency);
}

void QDemonCustomMaterial::setHasRefraction(bool hasRefraction)
{
    if (m_hasRefraction == hasRefraction)
        return;

    m_hasRefraction = hasRefraction;
    emit hasRefractionChanged(m_hasRefraction);
}

void QDemonCustomMaterial::setHasVolumetricDF(bool hasVolumetricDF)
{
    if (m_hasVolumetricDF == hasVolumetricDF)
        return;

    m_hasVolumetricDF = hasVolumetricDF;
    emit hasVolumetricDFChanged(m_hasVolumetricDF);
}

void QDemonCustomMaterial::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
}

void QDemonCustomMaterial::setShaderInfo(QDemonCustomMaterialShaderInfo *shaderInfo)
{
    m_shaderInfo = shaderInfo;
}

void QDemonCustomMaterial::setAlwaysDirty(bool alwaysDirty)
{
    if (m_alwaysDirty == alwaysDirty)
        return;

    m_alwaysDirty = alwaysDirty;
    emit alwaysDirtyChanged(m_alwaysDirty);
}

QDemonRenderGraphObject *QDemonCustomMaterial::updateSpatialNode(QDemonRenderGraphObject *node)
{
    static const auto updateShaderPrefix = [](QByteArray &shaderPrefix, const QByteArray &name) {
        const char *filter = "linear";
        const char *clamp = "clamp";
        // Output macro so we can change the set of variables used for this
        // independent of the
        // meta data system.
        shaderPrefix.append("SNAPPER_SAMPLER2D(");
        shaderPrefix.append(name);
        shaderPrefix.append(", ");
        shaderPrefix.append(name);
        shaderPrefix.append(", ");
        shaderPrefix.append(filter);
        shaderPrefix.append(", ");
        shaderPrefix.append(clamp);
        shaderPrefix.append(", ");
        shaderPrefix.append("false )\n");
    };

    static const auto appendShaderUniform = [](const QString &type, const QByteArray &name, QByteArray *shaderPrefix, const QString &value = QString()) {
        shaderPrefix->append(QStringLiteral("uniform %1 %2 %3;\n").arg(type).arg(QString::fromLatin1(name)).arg(value.isEmpty() ? QString() : QString("= %1").arg(value)).toLatin1());
    };

    static const auto resolveShader = [](const QByteArray &shader) -> QByteArray {
        int offset = -1;
        if (shader.startsWith("qrc"))
            offset = 3;
        else if (shader.startsWith("file:/"))
            offset = 0;
        else if (shader.startsWith(":/"))
            offset = 0;

        QString path;
        if (offset == -1) {
            QUrl u(QString::fromUtf8(shader));
            if (u.isLocalFile())
                path = u.toLocalFile();
        }

        if (offset == -1 && path.isEmpty())
            path = QString::fromLatin1(":/") + QString::fromUtf8(shader);
        else
            path = QString::fromUtf8(shader.constData() + offset);

        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
            return f.readAll();

        return shader;
    };


    // Sanity check(s)
    if (!m_shaderInfo->isValid()) {
        qWarning("ShaderInfo is not valid!");
        return node;
    }

    if (node)
        QDemonMaterial::updateSpatialNode(node);

    QDemonRenderCustomMaterial *customMaterial = static_cast<QDemonRenderCustomMaterial *>(node);
    if (!customMaterial) {
        customMaterial = new QDemonRenderCustomMaterial;
        customMaterial->m_layerCount = m_shaderInfo->layers;
        customMaterial->m_shaderKeyValues = static_cast<QDemonRenderCustomMaterial::MaterialShaderKeyFlags>(m_shaderInfo->shaderKey);
        customMaterial->className = metaObject()->className();
        customMaterial->m_alwaysDirty = m_alwaysDirty;
        customMaterial->m_hasTransparency = m_hasTransparency;
        customMaterial->m_hasRefraction = m_hasRefraction;
        customMaterial->m_hasVolumetricDF = m_hasVolumetricDF;

        // Shader info
        auto &shaderInfo = customMaterial->shaderInfo;
        shaderInfo.type = m_shaderInfo->type;
        shaderInfo.version = m_shaderInfo->version;
        shaderInfo.shaderPrefix = m_shaderInfo->shaderPrefix;

        QMetaMethod propertyDirtyMethod;
        const int idx = metaObject()->indexOfSlot("onPropertyDirty()");
        if (idx != -1)
            propertyDirtyMethod = metaObject()->method(idx);

        // Properties
        const int propCount = metaObject()->propertyCount();
        const int propOffset = metaObject()->propertyOffset();
        for (int i = propOffset; i != propCount; ++i) {
            const auto property = metaObject()->property(i);
            if (Q_UNLIKELY(!property.isValid()))
                continue;

            // Track the property changes
            if (property.hasNotifySignal() && propertyDirtyMethod.isValid())
                connect(this, property.notifySignal(), this, propertyDirtyMethod);

            const QVariant value = property.read(this);
            if (Q_UNLIKELY(!value.isValid()))
                continue;

            QDemonRenderCustomMaterial::Property demonProp;
            demonProp.name = property.name();
            demonProp.value = value;
            demonProp.pid = i;
            QString type;
            // TODO: Only types for copper are vec3, bool and float
            if (value.type() == QVariant::Double) {
                type = QStringLiteral("float");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Float;
            } else if (value.type() == QVariant::Bool) {
                type = QStringLiteral("bool");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Boolean;
            } else if (value.type() == QVariant::Vector2D) {
                type = QStringLiteral("vec2");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Vec2;
            } else if (value.type() == QVariant::Vector3D) {
                type = QStringLiteral("vec3");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Vec3;
            } else if (value.type() == QVariant::Vector4D) {
                type = QStringLiteral("vec4");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Vec4;
            } else if (value.type() == QVariant::Int) {
                type = QStringLiteral("int");
                demonProp.shaderDataType = QDemonRenderShaderDataType::Integer;
            } else {
                Q_ASSERT(0);
            }

            appendShaderUniform(type, demonProp.name, &shaderInfo.shaderPrefix);
            customMaterial->properties.push_back(demonProp);
        }

        // Textures
        for (const auto &texture : qAsConst(m_textures)) {
            QDemonRenderCustomMaterial::TextureProperty textureData;
            const QByteArray &name = texture->name;
            if (name.isEmpty()) // Warnings here will just drown in the shader error messages
                continue;
            QDemonImage *image = texture->image(); //
            connect(texture, &QDemonCustomMaterialTexture::textureDirty, this, &QDemonCustomMaterial::onTextureDirty);
            textureData.name = name;
            if (texture->enabled)
                textureData.texImage = image->getRenderImage();
            textureData.usageType = QDemonRenderTextureTypeValue(texture->type);
            textureData.shaderDataType = QDemonRenderShaderDataType::Texture2D;
            textureData.clampType = image->horizontalTiling() == QDemonImage::Repeat ? QDemonRenderTextureCoordOp::Repeat
                                                                                     : (image->horizontalTiling() == QDemonImage::ClampToEdge) ? QDemonRenderTextureCoordOp::ClampToEdge
                                                                                                                                               : QDemonRenderTextureCoordOp::MirroredRepeat;
            updateShaderPrefix(shaderInfo.shaderPrefix, textureData.name);
            customMaterial->textureProperties.push_back(textureData);
        }

        // Shaders
        QByteArray shaderCode = shaderInfo.shaderPrefix;
        QByteArray shared, vertex, geometry, fragment;
        for (const auto &shader : qAsConst(m_shaders)) {
            const QByteArray code = resolveShader(shader->shader);
            switch (shader->stage) {
            case QDemonCustomMaterialShader::Stage::Shared:
                if (!code.isEmpty())
                    shared.append(code);
                break;
            case QDemonCustomMaterialShader::Stage::Geometry:
                if (!code.isEmpty()) {
                    geometry.append(QByteArrayLiteral("\n#ifdef USER_GEOMETRY_SHADER\n"));
                    geometry.append(code);
                    geometry.append(QByteArrayLiteral("\n#endif\n"));
                    // TODO: Set m_HasGeomShader to true
                }
                break;
            case QDemonCustomMaterialShader::Stage::Vertex:
                vertex.append(QByteArrayLiteral("\n#ifdef VERTEX_SHADER\n"));
                if (code.isEmpty())
                    vertex.append(QByteArrayLiteral("void vert(){}"));
                else
                    vertex.append(code);
                vertex.append(QByteArrayLiteral("\n#endif\n"));
                break;
            case QDemonCustomMaterialShader::Stage::Fragment:
                fragment.append(QByteArrayLiteral("\n#ifdef FRAGMENT_SHADER\n"));
                if (code.isEmpty())
                    fragment.append(QByteArrayLiteral("void frag(){}"));
                else
                    fragment.append(code);
                fragment.append(QByteArrayLiteral("\n#endif\n"));
                break;
            }
        }

        if (!shared.isEmpty())
            shaderCode.append(shared);
        if (!vertex.isEmpty())
            shaderCode.append(vertex);
        if (!geometry.isEmpty())
            shaderCode.append(geometry);
        if (!fragment.isEmpty())
            shaderCode.append(fragment);

        if (!m_shaders.isEmpty()) {
            // Find the parent view
            QObject *p = this;
            QDemonView3D *view = nullptr;
            while (p != nullptr && view == nullptr) {
                p = p->parent();
                if ((view = qobject_cast<QDemonView3D *>(p)))
                    break;
            }

            Q_ASSERT(view);

            customMaterial->commands.push_back(new dynamic::QDemonBindShader(metaObject()->className()));
            customMaterial->commands.push_back(new dynamic::QDemonApplyInstanceValue());

            QDemonRenderContextInterface::QDemonRenderContextInterfacePtr renderContext = QDemonRenderContextInterface::getRenderContextInterface(quintptr(view->window()));
            renderContext->customMaterialSystem()->setMaterialClassShader(QString::fromLatin1(customMaterial->className), shaderInfo.type, shaderInfo.version, shaderCode, false, false);
        }

        for (const auto &pass : qAsConst(m_passes)) {
            QDemonRenderCustomMaterial::Pass demonPass;
            for (const auto &command : pass->m_commands)
                customMaterial->commands.push_back(command->getCommand());
            customMaterial->passes.push_back(demonPass);
        }

        customMaterial->commands.push_back(new dynamic::QDemonRender(false));
    }

    if (m_dirtyAttributes & Dirty::PropertyDirty) {
        for (const auto &prop : qAsConst(customMaterial->properties)) {
            auto p = metaObject()->property(prop.pid);
            if (Q_LIKELY(p.isValid()))
                prop.value = p.read(this);
        }
    }

    if (m_dirtyAttributes & Dirty::TextureDirty) {
        // TODO:
        for (const auto &texture : qAsConst(m_textures));
    }

    return customMaterial;
}

void QDemonCustomMaterial::onPropertyDirty()
{
    markDirty(Dirty::PropertyDirty);
    update();
}

void QDemonCustomMaterial::onTextureDirty(QDemonCustomMaterialTexture *texture)
{
    Q_UNUSED(texture)
    markDirty(Dirty::TextureDirty);
    update();
}

void QDemonCustomMaterial::qmlAppendShader(QQmlListProperty<QDemonCustomMaterialShader> *list, QDemonCustomMaterialShader *shader)
{
    if (!shader)
        return;

    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    that->m_shaders.push_back(shader);
}

QDemonCustomMaterialShader *QDemonCustomMaterial::qmlShaderAt(QQmlListProperty<QDemonCustomMaterialShader> *list, int index)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_shaders.at(index);
}

int QDemonCustomMaterial::qmlShaderCount(QQmlListProperty<QDemonCustomMaterialShader> *list)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_shaders.size();
}

void QDemonCustomMaterial::qmlAppendTexture(QQmlListProperty<QDemonCustomMaterialTexture> *textures, QDemonCustomMaterialTexture *texture)
{
    if (!texture)
        return;

    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(textures->object);
    if (texture->type == QDemonCustomMaterialTexture::TextureType::Displace)
        that->setDisplacementMap(texture->image());
    else if (texture->type == QDemonCustomMaterialTexture::TextureType::Emissive2)
        that->setEmissiveMap2(texture->image());
    else
        that->setDynamicTextureMap(texture->image());
    that->m_textures.push_back(texture);
    that->markDirty(Dirty::TextureDirty);
}

QDemonCustomMaterialTexture *QDemonCustomMaterial::qmlTextureAt(QQmlListProperty<QDemonCustomMaterialTexture> *list, int index)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_textures.at(index);
}

int QDemonCustomMaterial::qmlTextureCount(QQmlListProperty<QDemonCustomMaterialTexture> *list)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_textures.size();
}

void QDemonCustomMaterial::qmlAppendPass(QQmlListProperty<QDemonCustomMaterialRenderPass> *list, QDemonCustomMaterialRenderPass *pass)
{
    if (!pass)
        return;

    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    that->m_passes.push_back(pass);
}

QDemonCustomMaterialRenderPass *QDemonCustomMaterial::qmlPassAt(QQmlListProperty<QDemonCustomMaterialRenderPass> *list, int index)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_passes.at(index);
}

int QDemonCustomMaterial::qmlPassCount(QQmlListProperty<QDemonCustomMaterialRenderPass> *list)
{
    QDemonCustomMaterial *that = qobject_cast<QDemonCustomMaterial *>(list->object);
    return that->m_textures.count();
}

void QDemonCustomMaterialRenderPass::qmlAppendCommand(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, QDemonCustomMaterialRenderCommand *command)
{
    if (!command)
        return;

    QDemonCustomMaterialRenderPass *that = qobject_cast<QDemonCustomMaterialRenderPass *>(list->object);
    that->m_commands.push_back(command);
}

QDemonCustomMaterialRenderCommand *QDemonCustomMaterialRenderPass::qmlCommandAt(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list, int index)
{
    QDemonCustomMaterialRenderPass *that = qobject_cast<QDemonCustomMaterialRenderPass *>(list->object);
    return that->m_commands.at(index);
}

int QDemonCustomMaterialRenderPass::qmlCommandCount(QQmlListProperty<QDemonCustomMaterialRenderCommand> *list)
{
    QDemonCustomMaterialRenderPass *that = qobject_cast<QDemonCustomMaterialRenderPass *>(list->object);
    return that->m_commands.count();
}

QQmlListProperty<QDemonCustomMaterialRenderCommand> QDemonCustomMaterialRenderPass::commands()
{
    return QQmlListProperty<QDemonCustomMaterialRenderCommand>(this,
                                                               nullptr,
                                                               QDemonCustomMaterialRenderPass::qmlAppendCommand,
                                                               QDemonCustomMaterialRenderPass::qmlCommandCount,
                                                               QDemonCustomMaterialRenderPass::qmlCommandAt,
                                                               nullptr);
}

QT_END_NAMESPACE
