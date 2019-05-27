#include "assimpimporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

#include <QtDemonAssetImport/private/qdemonmeshutilities_p.h>

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
    const aiScene *scene = m_importer->ReadFile(sourceFile.toStdString(), aiProcessPreset_TargetRealtime_Quality);
    if (!scene) {
        // Scene failed to load, use logger to get the reason
        return QString::fromLocal8Bit(m_importer->GetErrorString());
    }

    // Generate Mesh files
    savePath.mkpath("meshes");
    auto meshBuilder = QDemonMeshUtilities::QDemonMeshBuilder::createMeshBuilder();
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh *mesh = scene->mMeshes[i];
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
        // ### save temp mesh file
        const QString saveFileName = savePath.absolutePath() +
                QDir::separator() + QStringLiteral("meshes") +
                QDir::separator() + QString::number(i) +
                QStringLiteral(".mesh");
        QFile saveFile(saveFileName);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Can't write to file " << saveFileName;
            continue;
        }

        outputMesh.saveMulti(saveFile, 0);
        saveFile.close();
    }

    // Generate Texture Sources

    // Materials

    // Traverse Node Tree

    // Animations (timeline based)

    return QString();
}

QT_END_NAMESPACE
