/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "objassetimporter.h"

#include <QtDemonAssetImport/private/qdemonscenegraphtranslation_p.h>

#include <QtCore/QVector>

#include <QtGui/QVector3D>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

ObjAssetImporter::ObjAssetImporter() = default;

const QString ObjAssetImporter::name() const
{
    return QStringLiteral("obj");
}

const QStringList ObjAssetImporter::inputExtensions() const
{
    QStringList extensions;
    extensions.append(QStringLiteral("obj"));
    return extensions;
}

const QString ObjAssetImporter::outputExtension() const
{
    return QStringLiteral("3d.qml");
}

const QString ObjAssetImporter::type() const
{
    return QStringLiteral("Model");
}

const QVariantMap ObjAssetImporter::importOptions() const
{
    // Generate Tangents ?
    // Scale Mesh ?
    // Optimize Mesh ?
    return QVariantMap();
}

const QString ObjAssetImporter::import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles)
{
    // This importer reads a *.obj file and outputs a series of mesh files and a .3d.qml file defining the Model3D
    Q_UNUSED(savePath)
    Q_UNUSED(options)
    Q_UNUSED(generatedFiles)

    // Validate file
    QFile objFile(sourceFile);
    if (!objFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringLiteral("Failed to open %1 for reading").arg(sourceFile);

    QTextStream stream(&objFile);
    parseObj(stream);

    objFile.close();

    QFileInfo sourceInfo(objFile);

    const QString outputFileName = savePath.absolutePath() + QDir::separator() + sourceInfo.baseName() + QStringLiteral(".3d.qml");
    m_sceneTranslator.save(outputFileName);

    if (generatedFiles)
        generatedFiles->append(outputFileName);

    return QString();
}

void ObjAssetImporter::parseObj(QTextStream &stream)
{
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector2D> uvs;
    QString name;
    QString currentMaterialLibrary;
    QString currentMaterial;
    QString currentGroup;

    QString line;
    while (true) {
        line = stream.readLine().trimmed();
        // check for escaped lines
        while (!line.isEmpty() && line.endsWith('\\')) {
            QString nextLine = stream.readLine().trimmed();
            line += nextLine;
            if (nextLine.isEmpty())
                break;
        }

        if (line.startsWith("v ")) {
            // vertex
            auto vertexString = line.split(" ", QString::SkipEmptyParts);
            Q_ASSERT(vertexString.size() >= 4);
            QVector3D vertex;
            vertex.setX(vertexString[1].toFloat());
            vertex.setY(vertexString[2].toFloat());
            vertex.setZ(vertexString[3].toFloat());
            vertices.push_back(vertex);
        } else if (line.startsWith("vt ")) {
            // uv
            auto uvString = line.split(" ", QString::SkipEmptyParts);
            Q_ASSERT(uvString.size() >= 3);
            QVector2D uv;
            uv.setX(uvString[1].toFloat());
            uv.setY(uvString[2].toFloat());
            uvs.push_back(uv);

        } else if (line.startsWith("vn ")) {
            // normal
            auto normalString = line.split(" ", QString::SkipEmptyParts);
            Q_ASSERT(normalString.size() >= 4);
            QVector3D normal;
            normal.setX(normalString[1].toFloat());
            normal.setX(normalString[2].toFloat());
            normal.setX(normalString[3].toFloat());
            normals.push_back(normal);
        } else if (line.startsWith("f ")) {
            // face

            auto faceString = line.split(" ", QString::SkipEmptyParts);
            Q_ASSERT(faceString.size() >= 5);

            QStringList face[3];
            face[0] = faceString[1].split("/");
            face[1] = faceString[2].split("/");
            Q_ASSERT(!face[0].isEmpty());
            Q_ASSERT(face[0].size() == face[1].size());

            for (int i = 2; i < faceString.size() - 1; i++) {

                face[2] = faceString[i + 1].split("/");
                Q_ASSERT(face[0].size() == face[2].size());

                for (int j = 0; j < 3; j++) {

                    int idx = j;

                    //                    if (!flip_faces && idx < 2) {
                    //                        idx = 1 ^ idx;
                    //                    }

                    // Normal
                    if (face[idx].size() == 3) {
                        int norm = face[idx][2].toInt() - 1;
                        if (norm < 0)
                            norm += normals.size() + 1;
                        // ### add normal
                        // surf_tool->add_normal(normals[norm]);
                    }

                    // UV
                    if (face[idx].size() >= 2 && !face[idx][1].isEmpty()) {
                        int uv = face[idx][1].toInt() - 1;
                        if (uv < 0)
                            uv += uvs.size() + 1;
                        // ### add UV
                        // surf_tool->add_uv(uvs[uv]);
                    }

                    // Vertex
                    int vtx = face[idx][0].toInt() - 1;
                    if (vtx < 0)
                        vtx += vertices.size() + 1;

                    // QVector3D vertex = vertices[vtx];
                    // ### add Vertex
                    // surf_tool->add_vertex(vertex);
                }

                face[1] = face[2];
            }
        } else if (line.startsWith("s ")) {
            QString smoothingString = line.right(line.length() - 2).trimmed();
            //            if (smoothingString == "off")
            //                //surf_tool->add_smooth_group(false);
            //            else
            //                //surf_tool->add_smooth_group(true);
        } else if (line.startsWith("usemtl ") || (line.startsWith("o ") || line.isNull())) { // commit group to mesh

            //            if (surf_tool->get_vertex_array().size()) {
            //                //another group going on, commit it
            //                if (normals.size() == 0) {
            //                    surf_tool->generate_normals();
            //                }

            //                if (generate_tangents && uvs.size()) {
            //                    surf_tool->generate_tangents();
            //                }

            //                surf_tool->index();

            //                print_line("current material library " + current_material_library + " has " + itos(material_map.has(current_material_library)));
            //                print_line("current material " + current_material + " has " + itos(material_map.has(current_material_library) && material_map[current_material_library].has(current_material)));

            //                if (material_map.has(current_material_library) && material_map[current_material_library].has(current_material)) {
            //                    surf_tool->set_material(material_map[current_material_library][current_material]);
            //                }

            //                mesh = surf_tool->commit(mesh, mesh_flags);

            //                if (current_material != String()) {
            //                    mesh->surface_set_name(mesh->get_surface_count() - 1, current_material.get_basename());
            //                } else if (current_group != String()) {
            //                    mesh->surface_set_name(mesh->get_surface_count() - 1, current_group);
            //                }

            //                print_line("Added surface :" + mesh->surface_get_name(mesh->get_surface_count() - 1));
            //                surf_tool->clear();
            //                surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
            //            }

            if (line.startsWith("o ") || line.isNull()) {

                //                if (!p_single_mesh) {
                //                    mesh->set_name(name);
                //                    r_meshes.push_back(mesh);
                //                    mesh.instance();
                //                    current_group = "";
                //                    current_material = "";
                //                }
            }

            // End of File
            if (line.isNull())
                break;

            if (line.startsWith("o "))
                name = line.right(line.length() - 2).trimmed();

            if (line.startsWith("usemtl "))
                currentMaterial = line.remove(0, 6).trimmed();

            if (line.startsWith("g "))
                currentGroup = line.right(line.length() - 2).trimmed();

        } else if (line.startsWith("mtllib ")) { // parse material

            currentMaterialLibrary = line.remove(0, 6).trimmed();
            //            if (!material_map.has(current_material_library)) {
            //                Map<String, Ref<SpatialMaterial> > lib;
            //                Error err = _parse_material_library(current_material_library, lib, r_missing_deps);
            //                if (err == ERR_CANT_OPEN) {
            //                    String dir = p_path.get_base_dir();
            //                    err = _parse_material_library(dir.plus_file(current_material_library), lib, r_missing_deps);
            //                }
            //                if (err == OK) {
            //                    material_map[current_material_library] = lib;
            //                }
            //            }
        }
    }
}

QT_END_NAMESPACE
