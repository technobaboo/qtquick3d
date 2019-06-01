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
    QVector<aiMesh *> meshes;
    QVector<aiMaterial *> materials;
    for (uint i = 0; i < modelNode->mNumMeshes; ++i) {
        aiMesh *mesh = m_scene->mMeshes[modelNode->mMeshes[i]];
        aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];
        meshes.append(mesh);
        materials.append(material);
    }

    QString outputMeshFile = QStringLiteral("meshes/") +
            QString::fromUtf8(modelNode->mName.C_Str()) + QStringLiteral(".mesh");

    QFile meshFile(m_savePath.absolutePath() + QDir::separator() + outputMeshFile);
    generateMeshFile(meshFile, meshes);

    output << QDemonQmlUtilities::insertTabs(tabLevel) << "source: \"" << outputMeshFile << QStringLiteral("\"") << endl;

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

QString AssimpImporter::generateMeshFile(QIODevice &file, const QVector<aiMesh *> &meshes)
{
    if (!file.open(QIODevice::WriteOnly))
        return QStringLiteral("Could not open device to write mesh file");


    auto meshBuilder = QDemonMeshUtilities::QDemonMeshBuilder::createMeshBuilder();

    struct SubsetEntryData {
        QString name;
        int indexLength;
        int indexOffset;
    };

    // Check if we need placeholders in certain channels
    bool needsPositionData = false;
    bool needsNormalData = false;
    bool needsUV1Data = false;
    bool needsUV2Data = false;
    bool needsTangentData = false;
    bool needsVertexColorData = false;
    unsigned uv1Components = 0;
    unsigned uv2Components = 0;
    unsigned totalVertices = 0;
    for (const auto *mesh : meshes) {
        totalVertices += mesh->mNumVertices;
        uv1Components = qMax(mesh->mNumUVComponents[0], uv1Components);
        uv2Components = qMax(mesh->mNumUVComponents[1], uv2Components);
        needsPositionData |= mesh->HasPositions();
        needsNormalData |= mesh->HasNormals();
        needsUV1Data |= mesh->HasTextureCoords(0);
        needsUV2Data |= mesh->HasTextureCoords(1);
        needsTangentData |= mesh->HasTangentsAndBitangents();
        needsVertexColorData |=mesh->HasVertexColors(0);
    }

    QByteArray positionData;
    QByteArray normalData;
    QByteArray uv1Data;
    QByteArray uv2Data;
    QByteArray tangentData;
    QByteArray binormalData;
    QByteArray vertexColorData;
    QByteArray indexBufferData;
    QVector<SubsetEntryData> subsetData;

    for (const auto *mesh : meshes) {
        // Position
        if (mesh->HasPositions())
            positionData += QByteArray(reinterpret_cast<char*>(mesh->mVertices), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
        else if (needsPositionData)
            positionData += QByteArray(mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32), '\0');

        // Normal
        if (mesh->HasNormals())
            normalData += QByteArray(reinterpret_cast<char*>(mesh->mNormals), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
        else if (needsNormalData)
            normalData += QByteArray(mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32), '\0');

        // UV1
        if (mesh->HasTextureCoords(0))
            uv1Data += QByteArray(reinterpret_cast<char*>(mesh->mTextureCoords[0]), mesh->mNumVertices * uv1Components * getSizeOfType(QDemonRenderComponentType::Float32));
        else if (needsUV1Data)
            uv1Data += QByteArray(mesh->mNumVertices * uv1Components * getSizeOfType(QDemonRenderComponentType::Float32), '\0');

        // UV2
        if (mesh->HasTextureCoords(1))
            uv2Data += QByteArray(reinterpret_cast<char*>(mesh->mTextureCoords[1]), mesh->mNumVertices * uv2Components * getSizeOfType(QDemonRenderComponentType::Float32));
        else if (needsUV2Data)
            uv2Data += QByteArray(mesh->mNumVertices * uv2Components * getSizeOfType(QDemonRenderComponentType::Float32), '\0');

        if (mesh->HasTangentsAndBitangents()) {
            // Tangents
            tangentData += QByteArray(reinterpret_cast<char*>(mesh->mTangents), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
            // Binormals
            auto binormalVector = calculateBinormals(mesh);
            binormalData += QByteArray(reinterpret_cast<char*>(binormalVector.data()), mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32));
        } else if (needsTangentData) {
            tangentData += QByteArray(mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32), '\0');
            binormalData += QByteArray(mesh->mNumVertices * 3 * getSizeOfType(QDemonRenderComponentType::Float32), '\0');
        }
        // ### Bones + Weights

        // Color
        if (mesh->HasVertexColors(0))
            vertexColorData += QByteArray(reinterpret_cast<char*>(mesh->mColors[0]), mesh->mNumVertices * 4 * getSizeOfType(QDemonRenderComponentType::Float32));
        else if (needsVertexColorData)
            vertexColorData += QByteArray(mesh->mNumVertices * 4 * getSizeOfType(QDemonRenderComponentType::Float32), '\0');
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
        SubsetEntryData subsetEntry;
        subsetEntry.indexOffset = indexBufferData.length() / sizeof(quint16);
        subsetEntry.indexLength = indexes.length() / sizeof(quint16);
        indexBufferData += QByteArray(reinterpret_cast<const char *>(indexes.constData()), indexes.length() * sizeof(quint16));

        // Subset
        subsetEntry.name = QString::fromUtf8(m_scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str());
        subsetData.append(subsetEntry);
    }

    // Vertex Buffer Entries
    QVector<QDemonMeshUtilities::MeshBuilderVBufEntry> entries;
    if (positionData.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry positionAttribute( QDemonMeshUtilities::Mesh::getPositionAttrName(),
                                                                     positionData,
                                                                     QDemonRenderComponentType::Float32,
                                                                     3);
        entries.append(positionAttribute);
    }
    if (normalData.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry normalAttribute( QDemonMeshUtilities::Mesh::getNormalAttrName(),
                                                                   normalData,
                                                                   QDemonRenderComponentType::Float32,
                                                                   3);
        entries.append(normalAttribute);
    }
    if (uv1Data.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry uv1Attribute( QDemonMeshUtilities::Mesh::getUVAttrName(),
                                                                uv1Data,
                                                                QDemonRenderComponentType::Float32,
                                                                uv1Components);
        entries.append(uv1Attribute);
    }
    if (uv2Data.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry uv2Attribute( QDemonMeshUtilities::Mesh::getUV2AttrName(),
                                                                uv2Data,
                                                                QDemonRenderComponentType::Float32,
                                                                uv2Components);
        entries.append(uv2Attribute);
    }

    if (tangentData.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry tangentsAttribute( QDemonMeshUtilities::Mesh::getTexTanAttrName(),
                                                                     tangentData,
                                                                     QDemonRenderComponentType::Float32,
                                                                     3);
        entries.append(tangentsAttribute);
    }

    if (binormalData.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry binormalAttribute( QDemonMeshUtilities::Mesh::getTexBinormalAttrName(),
                                                                     binormalData,
                                                                     QDemonRenderComponentType::Float32,
                                                                     3);
        entries.append(binormalAttribute);
    }

    if (vertexColorData.length() > 0) {
        QDemonMeshUtilities::MeshBuilderVBufEntry vertexColorAttribute( QDemonMeshUtilities::Mesh::getColorAttrName(),
                                                                        vertexColorData,
                                                                        QDemonRenderComponentType::Float32,
                                                                        4);
        entries.append(vertexColorAttribute);
    }

    meshBuilder->setVertexBuffer(entries);
    meshBuilder->setIndexBuffer(indexBufferData, QDemonRenderComponentType::UnsignedInteger16);

    // Subsets
    for (const auto &subset : subsetData)
        meshBuilder->addMeshSubset(reinterpret_cast<const char16_t *>(subset.name.utf16()),
                                   subset.indexLength,
                                   subset.indexOffset,
                                   0);



    auto &outputMesh = meshBuilder->getMesh();
    outputMesh.saveMulti(file, 0);

    file.close();
    return QString();
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
