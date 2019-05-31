#include "assimpimporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

#include <QtDemonAssetImport/private/qdemonmeshutilities_p.h>
#include <private/qdemonqmlutilities_p.h>

#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <qmath.h>

QT_BEGIN_NAMESPACE

AssimpImporter::AssimpImporter()
{
    m_importer = new Assimp::Importer();
}

AssimpImporter::~AssimpImporter()
{
    delete m_importer;
}

const QString AssimpImporter::name() const
{
    return QStringLiteral("assimp");
}

const QStringList AssimpImporter::inputExtensions() const
{
    QStringList extensions;
    extensions.append(QStringLiteral("fbx"));
    extensions.append(QStringLiteral("dae"));
    extensions.append(QStringLiteral("obj"));
    extensions.append(QStringLiteral("blend"));
    extensions.append(QStringLiteral("gltf"));
    extensions.append(QStringLiteral("glb"));
    return extensions;
}

const QString AssimpImporter::outputExtension() const
{
    return QStringLiteral(".qml");
}

const QString AssimpImporter::type() const
{
    return QStringLiteral("Scene");
}

const QVariantMap AssimpImporter::importOptions() const
{
    return QVariantMap();
}

namespace {
QVector<QVector3D> calculateBinormals(const aiMesh *mesh, bool isRightHanded = true) {
    QVector<QVector3D> binormals;
    binormals.reserve(mesh->mNumVertices);
    for (uint i = 0; i < mesh->mNumVertices; ++i) {
        QVector3D normal = QVector3D(mesh->mNormals[i].x,
                                     mesh->mNormals[i].y,
                                     mesh->mNormals[i].z).normalized();
        QVector3D tanget = QVector3D(mesh->mTangents[i].x,
                                     mesh->mTangents[i].y,
                                     mesh->mTangents[i].z).normalized();
        QVector3D binormal;
        // B = N x T
        if (isRightHanded)
            binormal = QVector3D::crossProduct(normal, tanget);
        else // B = T x N
            binormal = QVector3D::crossProduct(tanget, normal);
        binormals.append(binormal);
    }
    return binormals;
}
}

const QString AssimpImporter::import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles)
{
    QString errorString;
    m_savePath = savePath;

    m_scene = m_importer->ReadFile(sourceFile.toStdString(), aiProcessPreset_TargetRealtime_Quality);
    if (!m_scene) {
        // Scene failed to load, use logger to get the reason
        return QString::fromLocal8Bit(m_importer->GetErrorString());
    }

    // Generate Temp Mesh files
    auto meshBuilder = QDemonMeshUtilities::QDemonMeshBuilder::createMeshBuilder();
    for (unsigned int i = 0; i < m_scene->mNumMeshes; ++i) {
        const aiMesh *mesh = m_scene->mMeshes[i];
        meshBuilder->reset();

        // Vertex Buffer
        QVector<QDemonMeshUtilities::MeshBuilderVBufEntry> entries;

        // Position (attr_pos)
        if (mesh->HasPositions()) {
            QByteArray vertexData(reinterpret_cast<char*>(mesh->mVertices), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry positionAttribute( QDemonMeshUtilities::Mesh::getPositionAttrName(),
                                                                         vertexData,
                                                                         QDemonRenderComponentType::Float32,
                                                                         3);
            entries.append(positionAttribute);
        }

        // Normal (attr_norm)
        if (mesh->HasNormals()) {
            QByteArray vertexData(reinterpret_cast<char*>(mesh->mNormals), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry normalAttribute( QDemonMeshUtilities::Mesh::getNormalAttrName(),
                                                                         vertexData,
                                                                         QDemonRenderComponentType::Float32,
                                                                         3);
            entries.append(normalAttribute);
        }

        // UV1 (attr_uv0)
        if (mesh->HasTextureCoords(0)) {
            uint components = mesh->mNumUVComponents[0];
            QByteArray uv1Data(reinterpret_cast<char*>(mesh->mTextureCoords[0]), mesh->mNumVertices * components * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry uv1Attribute( QDemonMeshUtilities::Mesh::getUVAttrName(),
                                                                         uv1Data,
                                                                         QDemonRenderComponentType::Float32,
                                                                         components);
            entries.append(uv1Attribute);
        }

        // UV2 (attr_uv1)
        if (mesh->HasTextureCoords(1)) {
            uint components = mesh->mNumUVComponents[1];
            QByteArray uv2Data(reinterpret_cast<char*>(mesh->mTextureCoords[1]), mesh->mNumVertices * components * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry uv2Attribute( QDemonMeshUtilities::Mesh::getUV2AttrName(),
                                                                         uv2Data,
                                                                         QDemonRenderComponentType::Float32,
                                                                         components);
            entries.append(uv2Attribute);
        }

        if (mesh->HasTangentsAndBitangents()) {

            // Tangent (attr_textan)
            QByteArray tangentData(reinterpret_cast<char*>(mesh->mTangents), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry tangentsAttribute( QDemonMeshUtilities::Mesh::getTexTanAttrName(),
                                                                         tangentData,
                                                                         QDemonRenderComponentType::Float32,
                                                                         3);
            entries.append(tangentsAttribute);

            // Binormal (attr_binormal)
            // We have to calculate the binormal, because we only have bitangents calculated now
            auto binormalVector = calculateBinormals(mesh);
            QByteArray binormalData(reinterpret_cast<char*>(binormalVector.data()), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry binormalAttribute( QDemonMeshUtilities::Mesh::getTexBinormalAttrName(),
                                                                         binormalData,
                                                                         QDemonRenderComponentType::Float32,
                                                                         3);
            entries.append(binormalAttribute);

        }

        // ### Handle Bone's for rigged animations
        if (mesh->HasBones()) {
            // Weight (attr_weight)

            // BoneID (attr_boneid)
        }

        // Color (attr_color)
        if (mesh->HasVertexColors(0)) {
            QByteArray vertexColorData(reinterpret_cast<char*>(mesh->mColors[0]), mesh->mNumVertices * 4 * getSizeOfType(QDemonRenderComponentType::Float32));
            QDemonMeshUtilities::MeshBuilderVBufEntry vertexColorAttribute( QDemonMeshUtilities::Mesh::getColorAttrName(),
                                                                         vertexColorData,
                                                                         QDemonRenderComponentType::Float32,
                                                                         4);
            entries.append(vertexColorAttribute);
        }
        meshBuilder->setVertexBuffer(entries);

        // Index Buffer
        QVector<quint16> indexes;
        indexes.reserve(mesh->mNumFaces * 3);
        for (int faceIndex = 0;faceIndex < mesh->mNumFaces; ++faceIndex) {
            const auto face = mesh->mFaces[faceIndex];
            // Faces should always have 3 indicides
            Q_ASSERT(face.mNumIndices == 3);
            // ### We need to split meshes so that indexes can never be over ushort max
            indexes.append(quint16(face.mIndices[0]));
            indexes.append(quint16(face.mIndices[1]));
            indexes.append(quint16(face.mIndices[2]));
        }
        QByteArray indexBuffer(reinterpret_cast<const char *>(indexes.constData()), indexes.length() * sizeof(quint16));
        meshBuilder->setIndexBuffer(indexBuffer, QDemonRenderComponentType::UnsignedInteger16);

        // Subsets (in this case its everything in the index buffer)
        QString meshName;
        meshBuilder->addMeshSubset(reinterpret_cast<const char16_t *>(meshName.utf16()), indexes.count(), 0, 0);

        auto &outputMesh = meshBuilder->getMesh();
        const QString saveFileName = m_meshCache.path() + QString::number(i) + QStringLiteral(".mesh");
        QFile saveFile(saveFileName);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Can't write to file " << saveFileName;
            continue;
        }

        outputMesh.saveMulti(saveFile, 0);
        saveFile.close();
    }

    // Generate Embedded Texture Sources

    for (uint i = 0; i < m_scene->mNumTextures; ++i) {
        aiTexture *texture = m_scene->mTextures[i];
        if (texture->mHeight == 0) {
            // compressed format, try to load with Image Loader
            QByteArray data(reinterpret_cast<char *>(texture->pcData), texture->mWidth);
            QBuffer readBuffer(&data);
            QByteArray format = texture->achFormatHint;
            QImageReader imageReader(&readBuffer, format);
            QImage image = imageReader.read();
            if (image.isNull()) {
                qWarning() << imageReader.errorString();
                continue;
            }

            // ### maybe dont always use png
            const QString saveFileName = savePath.absolutePath() +
                    QDir::separator() + QStringLiteral("maps") +
                    QDir::separator() + QString::number(i) +
                    QStringLiteral(".png");
            image.save(saveFileName);

        } else {
            // Raw format, just convert data to QImage
            QImage rawImage(reinterpret_cast<uchar *>(texture->pcData), texture->mWidth, texture->mHeight, QImage::Format_RGBA8888);
            const QString saveFileName = savePath.absolutePath() +
                    QDir::separator() + QStringLiteral("maps") +
                    QDir::separator() + QString::number(i) +
                    QStringLiteral(".png");
            rawImage.save(saveFileName);
        }
    }

    // Check for Cameras
    if (m_scene->HasCameras()) {
        for (uint i = 0; i < m_scene->mNumCameras; ++i) {
            aiCamera *camera = m_scene->mCameras[i];
            aiNode *node = m_scene->mRootNode->FindNode(camera->mName);
            if (camera && node)
                m_cameras.insert(node, camera);
        }
    }

    // Check for Lights
    if (m_scene->HasLights()) {
        for (uint i = 0; i < m_scene->mNumLights; ++i) {
            aiLight *light = m_scene->mLights[i];
            aiNode *node = m_scene->mRootNode->FindNode(light->mName);
            if (light && node)
                m_lights.insert(node, light);
        }
    }

    // Materials

    // Traverse Node Tree

    // Animations (timeline based)

    // Create QML Component
    QFileInfo sourceFileInfo(sourceFile);


    QString targetFileName = savePath.absolutePath() + QDir::separator() +
            QDemonQmlUtilities::qmlComponentName(sourceFileInfo.baseName()) +
            QStringLiteral(".qml");
    QFile targetFile(targetFileName);
    if (!targetFile.open(QIODevice::WriteOnly)) {
        errorString += QString("Could not write to file: ") + targetFileName;
    } else {
        QTextStream output(&targetFile);

        // Imports header
        writeHeader(output);

        // Component Code
        processNode(m_scene->mRootNode, output);

        targetFile.close();
        if (generatedFiles)
            *generatedFiles += targetFileName;
    }

    // Cleanup mesh cache
    m_meshCache.setAutoRemove(true);

    return errorString;
}

void AssimpImporter::writeHeader(QTextStream &output)
{
    output << "import QtDemon 1.0" << endl;
    output << "import QtQuick 2.12" << endl;
    output << "import QtQuick.Timeline 1.0" << endl;
    output << endl;
}

void AssimpImporter::processNode(aiNode *node, QTextStream &output, int tabLevel)
{
    aiNode *currentNode = node;
    if (currentNode) {
        // Figure out what kind of node this is
        if (isModel(currentNode)) {
            // Model
            output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("DemonModel {") << endl;
            generateModelProperties(currentNode, output, tabLevel + 1);
        } else if (isLight(currentNode)) {
            // Light
            output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("DemonLight {") << endl;
            generateLightProperties(currentNode, output, tabLevel + 1);
        } else if (isCamera(currentNode)) {
            // Camera
            output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("DemonCamera {") << endl;
            generateCameraProperties(currentNode, output, tabLevel + 1);
        } else {
            // Transform Node
            output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("DemonNode {") << endl;
            generateNodeProperties(currentNode, output, tabLevel + 1);
        }

        // Process All Children Nodes
        for (uint i = 0; i < currentNode->mNumChildren; ++i)
            processNode(currentNode->mChildren[i], output, tabLevel + 1);

        // Write the QML Footer
        output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("}") << endl;
    }
}

void AssimpImporter::generateModelProperties(aiNode *modelNode, QTextStream &output, int tabLevel)
{
    generateNodeProperties(modelNode, output, tabLevel);

    // source
    // Combine all the meshes referenced by this model into a single MultiMesh file
    //QDemonMeshUtilities::Mesh *meshOutput = nullptr;
    QString outputMeshFile = QStringLiteral("meshes") + QDir::separator() +
            QString::fromUtf8(modelNode->mName.C_Str()) + QStringLiteral(".mesh");
    QFile meshFile(m_savePath.absolutePath() + outputMeshFile);
    if (!meshFile.open(QIODevice::ReadWrite)) {
        qWarning() << "Could not open file " << outputMeshFile;
        return;
    }
    for (uint i = 0; i < modelNode->mNumMeshes; ++i) {
        //aiMesh *mesh = m_scene->mMeshes[modelNode->mMeshes[i]];
        //aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];
        //aiString materialName = material->GetName();

        QString meshFileName = m_meshCache.path() + QString::number(modelNode->mMeshes[i]) + QStringLiteral(".mesh");
        auto loadResult = QDemonMeshUtilities::Mesh::loadMulti(meshFileName.toLocal8Bit(), 0);
        loadResult.m_mesh->saveMulti(meshFile, i);
    }
    meshFile.close();
    output << QDemonQmlUtilities::insertTabs(tabLevel) << "source: " << outputMeshFile << endl;

    // skeletonRoot

    // materials
    // For each mesh, generate a material for this list
}

void AssimpImporter::generateLightProperties(aiNode *lightNode, QTextStream &output, int tabLevel)
{
    generateNodeProperties(lightNode, output, tabLevel);

    aiLight *light = m_lights.value(lightNode);

    // lightType
    if (light->mType == aiLightSource_DIRECTIONAL || light->mType == aiLightSource_AMBIENT ) {
        QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("lightType"), QStringLiteral("DemonLight.Directional"));
    } else if (light->mType == aiLightSource_POINT) {
        QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("lightType"), QStringLiteral("DemonLight.Point"));
    } else if (light->mType == aiLightSource_SPOT) {
        // ### This is not and area light, but it's the closest we have right now
        QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("lightType"), QStringLiteral("DemonLight.Area"));
    }

    // diffuseColor
    QColor diffuseColor = QColor::fromRgbF(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b);
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("diffuseColor"), diffuseColor);

    // specularColor
    QColor specularColor = QColor::fromRgbF(light->mColorSpecular.r, light->mColorSpecular.g, light->mColorSpecular.b);
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("specularColor"), specularColor);

    // ambientColor
    if (light->mType == aiLightSource_AMBIENT) {
        // We only want ambient light color if it is explicit
        QColor ambientColor = QColor::fromRgbF(light->mColorAmbient.r, light->mColorAmbient.g, light->mColorAmbient.b);
        QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("ambientColor"), ambientColor);
    }
    // brightness
    //QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("brightness"), light->mAttenuationConstant);

    // linearFade
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("linearFade"), light->mAttenuationLinear);

    // exponentialFade
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Light, QStringLiteral("exponentialFade"), light->mAttenuationQuadratic);

    // areaWidth

    // areaHeight

    // castShadow

    // shadowBias

    // shadowFactor

    // shadowMapResolution

    // shadowMapFar

    // shadowMapFieldOFView

    // shadowFilter
}

void AssimpImporter::generateCameraProperties(aiNode *cameraNode, QTextStream &output, int tabLevel)
{
    generateNodeProperties(cameraNode, output, tabLevel);

    aiCamera *camera = m_cameras.value(cameraNode);

    // clipNear
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Camera, QStringLiteral("clipNear"), camera->mClipPlaneNear);

    // clipFar
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Camera, QStringLiteral("clipFar"), camera->mClipPlaneFar);

    // fieldOfView
    float fov = qRadiansToDegrees(camera->mHorizontalFOV * 2);
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Camera, QStringLiteral("fieldOfView"), fov);

    // isFieldOFViewHorizontal
    QDemonQmlUtilities::writeQmlPropertyHelper(output,tabLevel, QDemonQmlUtilities::PropertyMap::Camera, QStringLiteral("isFieldOFViewHorizontal"), true);


    // projectionMode

    // scaleMode

    // scaleAnchor

    // frustomScaleX

    // frustomScaleY

}

void AssimpImporter::generateNodeProperties(aiNode *node, QTextStream &output, int tabLevel)
{
    // id
    QString name = QString::fromUtf8(node->mName.C_Str());
    if (!name.isEmpty()) {
        // ### we may need to account of non-unique and empty names
        output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("id: ") <<
                  QDemonQmlUtilities::sanitizeQmlId(name) << endl;
    }

    // Decompose Transform Matrix to get properties
    aiVector3t<ai_real> scaling;
    aiVector3t<ai_real> rotation;
    aiVector3t<ai_real> translation;
    node->mTransformation.Decompose(scaling, rotation, translation);

    // translate
    QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Node, QStringLiteral("x"), translation.x);
    QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Node, QStringLiteral("y"), translation.y);
    QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Node, QStringLiteral("z"), translation.z);

    // rotation
    QVector3D rotationAngles(qRadiansToDegrees(rotation.x),
                             qRadiansToDegrees(rotation.y),
                             qRadiansToDegrees(rotation.z));
    QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Node, QStringLiteral("rotation"), rotationAngles);

    // scale
    QDemonQmlUtilities::writeQmlPropertyHelper(output, tabLevel, QDemonQmlUtilities::PropertyMap::Node, QStringLiteral("scale"), QVector3D(scaling.x, scaling.y, scaling.z));

    // pivot

    // opacity

    // boneid

    // rotation order

    // orientation

    // visible

}

bool AssimpImporter::isModel(aiNode *node)
{
    return node && node->mNumMeshes > 0;
}

bool AssimpImporter::isLight(aiNode *node)
{
    return node && m_lights.contains(node);
}

bool AssimpImporter::isCamera(aiNode *node)
{
    return node && m_cameras.contains(node);
}

QT_END_NAMESPACE
