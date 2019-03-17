#include "uipimporter.h"
#include "uipparser.h"
#include "uiaparser.h"
#include "uippresentation.h"

QT_BEGIN_NAMESPACE

UipImporter::UipImporter()
{

}

const QString UipImporter::name() const
{
    return QStringLiteral("uip");
}

const QStringList UipImporter::inputExtensions() const
{
    QStringList extensions;
    extensions.append(QStringLiteral("uia"));
    extensions.append(QStringLiteral("uip"));
    return extensions;
}

const QString UipImporter::outputExtension() const
{
    return QStringLiteral("3d.qml");
}

const QString UipImporter::type() const
{
    return QStringLiteral("Scene");
}

const QVariantMap UipImporter::importOptions() const
{
    return QVariantMap();
}

const QString UipImporter::import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles)
{
    m_sourceFile = sourceFile;
    m_exportPath = savePath;
    m_options = options;

    // Verify sourceFile and savePath
    QFileInfo source(sourceFile);
    if (!source.exists())
        return QStringLiteral("Source File: %s does not exist").arg(sourceFile);
    if (!savePath.exists())
        return QStringLiteral("Export Directory Invalid");

    // If sourceFile is a UIA file
    if (sourceFile.endsWith(QStringLiteral(".uia"), Qt::CaseInsensitive)) {
        auto uia = m_uiaParser.parse(sourceFile);
        for (auto presentation : uia.presentations) {
            if (presentation.type == UiaParser::Uia::Presentation::Uip) {
                // UIP
                auto uip = m_uipParser.parse(source.absolutePath() + QDir::separator() + presentation.source, presentation.id);
                processUipPresentation(uip, savePath.absolutePath() + QDir::separator());
            } else {
                // QML
                // Just copy the source file to the export directory as is
            }

        }

    } else if (sourceFile.endsWith(QStringLiteral(".uip"), Qt::CaseInsensitive)) {
        auto uip = m_uipParser.parse(sourceFile, QString());
        processUipPresentation(uip, savePath.absolutePath() + QDir::separator());
    }

    QString errorString;

    // Copy any resource files to export directory
    for (auto file : m_resourcesList) {
        QFileInfo sourceFile(source.absolutePath() + QDir::separator() + file);
        if (!sourceFile.exists())
            errorString += QStringLiteral("Resource file does not exist: ") + sourceFile.absoluteFilePath();
        QFileInfo destFile(savePath.absoluteFilePath(file));
        QDir destDir(destFile.absolutePath());
        destDir.mkpath(".");

        if (QFile::copy(sourceFile.absoluteFilePath(), destFile.absoluteFilePath()))
            m_generatedFiles += destFile.absoluteFilePath();
    }

    generatedFiles = &m_generatedFiles;

    return errorString;
}

namespace {

QString insertTabs(int n)
{
    QString tabs;
    for (int i = 0; i < n; ++i)
        tabs += "    ";
    return tabs;
}



QString qmlComponentName(const QString &name) {
    QString nameCopy = name;
    if (nameCopy.isEmpty())
        return QStringLiteral("Presentation");

    if (nameCopy[0].isLower())
        nameCopy[0] = nameCopy[0].toUpper();

    return nameCopy;
}

}

void UipImporter::processNode(GraphObject *object, QTextStream &output, int tabLevel, bool processSiblings)
{
    GraphObject *obj = object;
    while (obj) {
        if (obj->type() == GraphObject::Scene) {
            // Ignore Scene for now
            processNode(obj->firstChild(), output, tabLevel);
        } else if ( obj->type() == GraphObject::DefaultMaterial &&
                    obj->qmlId() == QStringLiteral("__Container")) {
            // UIP version > 5 which tries to be clever with reference materials
            // Instead of parsing these items as normal, instead we iterate the
            // materials container and generate new Components for each one and output them
            // to the "materials" folder
            GraphObject *materialObject = obj->firstChild();
            while(materialObject) {
                generateMaterialComponent(materialObject);
                materialObject = materialObject->nextSibling();
            }
        } else {
            // Ouput QML
            obj->writeQmlHeader(output, tabLevel);
            obj->writeQmlProperties(output, tabLevel + 1);
            output << endl;

            processNode(obj->firstChild(), output, tabLevel + 1);

            if (obj->type() == GraphObject::Layer) {
                // effects array
                // get all children that are effects, and add their id's to effects: array
                QString effects;
                GraphObject *effectObject = obj->firstChild();
                while (effectObject) {
                    if (effectObject->type() == GraphObject::Effect)
                        effects += effectObject->qmlId() + QStringLiteral(", ");
                    effectObject = effectObject->nextSibling();
                }
                if (!effects.isEmpty()) {
                    // remove final ", "
                    effects.chop(2);
                    output << insertTabs(tabLevel + 1) << QStringLiteral("effects: [") << effects << QStringLiteral("]") << endl;
                }


            } else if (obj->type() == GraphObject::Model) {
                // materials array
                // get all children that are materials, and add their id's to materials: array
                QString materials;
                GraphObject *materialObject = obj->firstChild();
                while (materialObject) {
                    if (materialObject->type() == GraphObject::DefaultMaterial ||
                        materialObject->type() == GraphObject::CustomMaterial ||
                        materialObject->type() == GraphObject::ReferencedMaterial)
                        materials += materialObject->qmlId() + QStringLiteral(", ");
                    materialObject = materialObject->nextSibling();
                }
                if (!materials.isEmpty()) {
                    // remove final ", "
                    materials.chop(2);
                    output << insertTabs(tabLevel + 1) << QStringLiteral("materials: [") << materials << QStringLiteral("]") << endl;
                }
            } else if (obj->type() == GraphObject::ReferencedMaterial) {
                m_referencedMaterials.append(static_cast<ReferencedMaterial *>(obj));
            } else if (obj->type() == GraphObject::Alias) {
                m_aliasNodes.append(static_cast<AliasNode*>(obj));
            }

            checkForResourceFiles(obj);

            obj->writeQmlFooter(output, tabLevel);
        }
        if (processSiblings)
            obj = obj->nextSibling();
        else
            break;
    }
}

void UipImporter::checkForResourceFiles(GraphObject *object)
{
    if (!object)
        return;
    if (object->type() == GraphObject::Image) {
        Image *image = static_cast<Image*>(object);
        if (!m_resourcesList.contains(image->m_sourcePath))
            m_resourcesList.append(image->m_sourcePath);
    } else if (object->type() == GraphObject::Model) {
        ModelNode *model = static_cast<ModelNode*>(object);
        QString meshLocation = model->m_mesh_unresolved;
        // Remove trailing # directive
        int hashLocation = meshLocation.indexOf("#");
        // if mesh source starts with #, it's a primative, so skip
        if (hashLocation == 1)
            return;
        if (hashLocation != -1) {
            meshLocation.chop(meshLocation.length() - hashLocation);
        }
        if (!m_resourcesList.contains(meshLocation))
            m_resourcesList.append(meshLocation);
    } else if (object->type() == GraphObject::Effect) {
        // ### maybe remove #
        EffectInstance *effect = static_cast<EffectInstance*>(object);
        if (!m_resourcesList.contains(effect->m_effect_unresolved))
            m_resourcesList.append(effect->m_effect_unresolved);
    } else if (object->type() == GraphObject::CustomMaterial) {
        // ### maybe remove #
        CustomMaterialInstance *material = static_cast<CustomMaterialInstance*>(object);
        if (!m_resourcesList.contains(material->m_material_unresolved))
            m_resourcesList.append(material->m_material_unresolved);
    }
}

void UipImporter::generateMaterialComponent(GraphObject *object)
{
    QDir materialPath = m_exportPath.absolutePath() + QDir::separator() + QStringLiteral("materials");

    QString id = object->qmlId();
    if (id.startsWith("materials_"))
        id = id.remove(QStringLiteral("materials_"));
    // Default matterial has two //'s
    if (id.startsWith("_"))
        id.remove(0, 1);

    QString materialComponent = qmlComponentName(id);
    QString targetFile = materialPath.absolutePath() + QDir::separator() + materialComponent + QStringLiteral(".qml");
    QFile materialComponentFile(targetFile);

    if (m_generatedFiles.contains(targetFile)) {
        // if we already generated this material
        return;
    }

    if (!materialComponentFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not write to file : " << materialComponentFile;
        return;
    }

    QTextStream output(&materialComponentFile);
    output << "import QtDemon 1.0" << endl << endl;
    processNode(object, output, 0, false);

    materialComponentFile.close();
    m_generatedFiles += targetFile;
}

void UipImporter::generateAliasComponent(GraphObject *reference)
{
    // create materials folder
    QDir aliasPath = m_exportPath.absolutePath() + QDir::separator() + QStringLiteral("materials");

    QString aliasComponentName = qmlComponentName(reference->qmlId());
    QString targetFile = aliasPath.absolutePath() + QDir::separator() + aliasComponentName + QStringLiteral(".qml");
    QFile aliasComponentFile(targetFile);

    if (m_generatedFiles.contains(targetFile))
        return;

    if (!aliasComponentFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not write to file: " << aliasComponentFile;
        return;
    }

    QTextStream output(&aliasComponentFile);
    output << "import QtDemon 1.0" << endl << endl;
    processNode(reference, output, 0, false);

    aliasComponentFile.close();
    m_generatedFiles += targetFile;
}

QString UipImporter::processUipPresentation(UipPresentation *presentation, const QString &ouputFilePath)
{
    m_referencedMaterials.clear();
    m_aliasNodes.clear();

    // create one component per layer
    GraphObject *layer = presentation->scene()->firstChild();
    while (layer) {
        if (layer->type() == GraphObject::Layer) {
            // Create *3d.qml file from .uip presentation
            QString targetFile = ouputFilePath + qmlComponentName(presentation->name()) + qmlComponentName(layer->qmlId()) + QStringLiteral(".qml");
            QFile qmlFile(targetFile);
            if (!qmlFile.open(QIODevice::WriteOnly)) {
                return QString(QStringLiteral("Could not open file: ") + targetFile + QStringLiteral(" for writing"));
            }

            QTextStream output(&qmlFile);

            output << "import QtDemon 1.0" << endl;
            output << "import \"./materials\" as Materials" << endl;
            output << "import \"./aliases\" as Aliases" << endl << endl;

            processNode(layer, output, 0);

            qmlFile.close();
            m_generatedFiles += targetFile;
        } else if ( layer->type() == GraphObject::DefaultMaterial &&
                    layer->qmlId() == QStringLiteral("__Container")) {
            // UIP version > 5 which tries to be clever with reference materials
            // Instead of parsing these items as normal, instead we iterate the
            // materials container and generate new Components for each one and output them
            // to the "materials" folder
            GraphObject *object = layer->firstChild();
            while (object) {
                generateMaterialComponent(object);
                object = object->nextSibling();
            }
        }

        layer = layer->nextSibling();
    }

    // create component type folder
    m_exportPath.mkdir("materials");
    m_exportPath.mkdir("aliases");


    // create aliases folder

    // Generate Alias, and ReferenceMaterials (2nd pass)
    for (auto material : m_referencedMaterials) {
        // Find the material being referenced
        QString id = material->m_referencedMaterial_unresolved;
        if (id.startsWith("#"))
            id.remove(0, 1);
        generateMaterialComponent(presentation->object(id.toUtf8()));
    }

    for (auto alias : m_aliasNodes) {
        QString id = alias->m_referencedNode_unresolved;
        if (id.startsWith("#"))
            id.remove(0, 1);
        generateAliasComponent(presentation->object(id.toUtf8()));
    }

    return QString();
}

QT_END_NAMESPACE
