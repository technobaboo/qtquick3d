/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#include "uipimporter.h"
#include "uipparser.h"
#include "uiaparser.h"
#include "uippresentation.h"
#include "datamodelparser.h"
#include "keyframegroupgenerator.h"
#include <private/qdemonqmlutilities_p.h>

#include <QBuffer>

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
    return QStringLiteral(".qml");
}

const QString UipImporter::type() const
{
    return QStringLiteral("Scene");
}

const QVariantMap UipImporter::importOptions() const
{
    return QVariantMap();
}

namespace  {
bool copyRecursively(const QString &sourceFolder, const QString &destFolder)
{
    bool success = false;
    QDir sourceDir(sourceFolder);

    if(!sourceDir.exists())
        return false;

    QDir destDir(destFolder);
    if(!destDir.exists())
        destDir.mkdir(destFolder);

    auto files = sourceDir.entryList(QDir::Files);
    for (const auto &file : files) {
        QString srcName = sourceFolder + QDir::separator() + file;
        QString destName = destFolder + QDir::separator() + file;
        success = QFile::copy(srcName, destName);
        if(!success)
            return false;
    }

    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const auto &file : files) {
        QString srcName = sourceFolder + QDir::separator() + file;
        QString destName = destFolder + QDir::separator() + file;
        success = copyRecursively(srcName, destName);
        if(!success)
            return false;
    }

    return true;
}
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

    QString uiaComponentName;
    QSize uiaComponentSize;

    // If sourceFile is a UIA file
    if (sourceFile.endsWith(QStringLiteral(".uia"), Qt::CaseInsensitive)) {
        auto uia = m_uiaParser.parse(sourceFile);
        uiaComponentName = uia.initialPresentationId;
        for (auto presentation : uia.presentations)
            if (presentation.type == UiaParser::Uia::Presentation::Qml)
                m_hasQMLSubPresentations = true;

        for (auto presentation : uia.presentations) {
            if (presentation.type == UiaParser::Uia::Presentation::Uip) {
                // UIP
                m_exportPath.mkdir(QStringLiteral("presentations"));
                auto uip = m_uipParser.parse(source.absolutePath() + QDir::separator() + presentation.source, presentation.id);
                processUipPresentation(uip, savePath.absolutePath() + QDir::separator() + QStringLiteral("presentations") + QDir::separator());
                if (presentation.id == uiaComponentName)
                    uiaComponentSize = QSize(uip->presentationWidth(), uip->presentationHeight());
            }
        }
        if (m_hasQMLSubPresentations) {
            // If there is any QML in the project at all, we have to copy the entire
            // qml folder over
            copyRecursively(source.absolutePath() + QDir::separator() + QStringLiteral("qml"),
                            m_exportPath.absolutePath() + QDir::separator() + QStringLiteral("qml"));
        }

    } else if (sourceFile.endsWith(QStringLiteral(".uip"), Qt::CaseInsensitive)) {
        auto uip = m_uipParser.parse(sourceFile, QString());
        processUipPresentation(uip, savePath.absolutePath() + QDir::separator());
    }

    QString errorString;

    // Copy any resource files to export directory
    for (auto file : m_resourcesList) {
        QFileInfo sourceFile(source.absolutePath() + QDir::separator() + file);
        if (!sourceFile.exists()) {
            // Try again after stripping the parent directory
            sourceFile = QFileInfo(source.absolutePath() + QDir::separator() + QDemonQmlUtilities::stripParentDirectory(file));
            if (!sourceFile.exists()) {
                errorString += QStringLiteral("Resource file does not exist: ") + sourceFile.absoluteFilePath() + QChar('\n');
                continue;
            }
        }
        QFileInfo destFile(savePath.absoluteFilePath(QDemonQmlUtilities::stripParentDirectory(file)));
        QDir destDir(destFile.absolutePath());
        destDir.mkpath(".");

        if (QFile::copy(sourceFile.absoluteFilePath(), destFile.absoluteFilePath()))
            m_generatedFiles += destFile.absoluteFilePath();
    }

    // Generate UIA Component if we converted a uia
    if (!uiaComponentName.isEmpty())
        generateApplicationComponent(QDemonQmlUtilities::qmlComponentName(uiaComponentName), uiaComponentSize);

    if (generatedFiles)
        generatedFiles = &m_generatedFiles;

    return errorString;
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

            if (obj->type() != GraphObject::Component && obj->type() != GraphObject::Layer)
                processNode(obj->firstChild(), output, tabLevel + 1);

            if (obj->type() == GraphObject::Layer) {
//                // effects array
//                // get all children that are effects, and add their id's to effects: array
//                QString effects;
//                GraphObject *effectObject = obj->firstChild();
//                while (effectObject) {
//                    if (effectObject->type() == GraphObject::Effect)
//                        effects += effectObject->qmlId() + QStringLiteral(", ");
//                    effectObject = effectObject->nextSibling();
//                }
//                if (!effects.isEmpty()) {
//                    // remove final ", "
//                    effects.chop(2);
//                    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("effects: [") << effects << QStringLiteral("]") << endl;
//                }


                // Check if layer has a sub-presentation set, in which case we can skip children processing and timeline generation
                auto layer = static_cast<LayerNode*>(obj);
                if (layer->m_sourcePath.isEmpty()) {
                    // Process children nodes
                    processNode(obj->firstChild(), output, tabLevel + 1);
//                    // Generate Animation Timeline
//                    generateAnimationTimeLine(obj, m_presentation->masterSlide(), output, tabLevel + 1);
//                    // Generate States from Slides
//                    generateStatesFromSlides(obj, m_presentation->masterSlide(), output, tabLevel + 1);
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
                    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("materials: [") << materials << QStringLiteral("]") << endl;
                }
            } else if (obj->type() == GraphObject::ReferencedMaterial) {
                m_referencedMaterials.append(static_cast<ReferencedMaterial *>(obj));
            } else if (obj->type() == GraphObject::Alias) {
                m_aliasNodes.append(static_cast<AliasNode*>(obj));
            } else if (obj->type() == GraphObject::Component) {
                m_componentNodes.append(static_cast<ComponentNode*>(obj));
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
        if (image->m_subPresentation.isEmpty() && !m_resourcesList.contains(image->m_sourcePath))
            m_resourcesList.append(image->m_sourcePath);
    } else if (object->type() == GraphObject::Model) {
        ModelNode *model = static_cast<ModelNode*>(object);
        QString meshLocation = model->m_mesh_unresolved;
        // Remove trailing # directive
        int hashLocation = meshLocation.indexOf("#");
        // if mesh source starts with #, it's a primitive, so skip
        if (hashLocation == 1)
            return;
        if (hashLocation != -1) {
            meshLocation.chop(meshLocation.length() - hashLocation);
        }
        if (!m_resourcesList.contains(meshLocation))
            m_resourcesList.append(meshLocation);
    } else if (object->type() == GraphObject::Effect) {
        // ### maybe remove #
        //EffectInstance *effect = static_cast<EffectInstance*>(object);
        //if (!m_resourcesList.contains(effect->m_effect_unresolved))
        //    m_resourcesList.append(effect->m_effect_unresolved);
    } else if (object->type() == GraphObject::CustomMaterial) {
        // ### maybe remove #
        // CustomMaterialInstance *material = static_cast<CustomMaterialInstance*>(object);
        //if (!m_resourcesList.contains(material->m_material_unresolved))
        //    m_resourcesList.append(material->m_material_unresolved);
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

    QString materialComponent = QDemonQmlUtilities::qmlComponentName(id);
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
    output << "import QtQuick3D 1.0" << endl << endl;
    processNode(object, output, 0, false);

    materialComponentFile.close();
    m_generatedFiles += targetFile;
}

void UipImporter::generateAliasComponent(GraphObject *reference)
{
    // create materials folder
    QDir aliasPath = m_exportPath.absolutePath() + QDir::separator() + QStringLiteral("aliases");

    QString aliasComponentName = QDemonQmlUtilities::qmlComponentName(reference->qmlId());
    QString targetFile = aliasPath.absolutePath() + QDir::separator() + aliasComponentName + QStringLiteral(".qml");
    QFile aliasComponentFile(targetFile);

    if (m_generatedFiles.contains(targetFile))
        return;

    if (!aliasComponentFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not write to file: " << aliasComponentFile;
        return;
    }

    QTextStream output(&aliasComponentFile);
    output << "import QtQuick3D 1.0" << endl << endl;
    processNode(reference, output, 0, false);

    aliasComponentFile.close();
    m_generatedFiles += targetFile;
}

namespace {

QSet<GraphObject*> getSubtreeItems(GraphObject *node)
{
    QSet<GraphObject *> items;
    if (!node)
        return items;

    std::function<void(GraphObject *, QSet<GraphObject *> &)> treeWalker;
    treeWalker = [&treeWalker](GraphObject *obj, QSet<GraphObject *> &items) {
        while (obj) {
            items.insert(obj);
            treeWalker(obj->firstChild(), items);
            obj = obj->nextSibling();
        }
    };

    treeWalker(node->firstChild(), items);

    return items;
}

void generateTimelineAnimation(Slide *slide, float startFrame, float endFrame, bool isRunning, QTextStream &output, int tabLevel)
{
    QString looping = QStringLiteral("1");
    QString pingPong = QStringLiteral("false");
    if (slide->m_playMode == Slide::Looping) {
        looping = QStringLiteral("-1");
    } else if (slide->m_playMode == Slide::PingPong) {
        looping = QStringLiteral("-1");
        pingPong = QStringLiteral("true");
    }

    output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("TimelineAnimation {") << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("duration: ") << (endFrame - startFrame) * 1000.0f << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("from: ") << startFrame << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("to: ") << endFrame << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("running: " ) << (isRunning ? QStringLiteral("true") : QStringLiteral("false")) << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("loops: ") << looping << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("pingPong: ") << pingPong << endl;
    output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("}");
}

}

void UipImporter::generateAnimationTimeLine(Slide *masterSlide, QTextStream &output, int tabLevel)
{
    // Generate a 1 Timeline and 1 TimelineAnimation for each slide
    // Each combines the master slide and current slide

    // For each slide, create a TimelineAnimation
    auto slide = static_cast<Slide*>(masterSlide->firstChild());
    while (slide) {
        output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("Timeline {") << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("id: ") << QDemonQmlUtilities::sanitizeQmlId(slide->m_name + QStringLiteral("Timeline")) << endl;;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("startFrame: ") << masterSlide->startTime() << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("endFrame: ") << masterSlide->endTime() << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("currentFrame: ") << masterSlide->startTime() << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("enabled: false") << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("animations: [") << endl;
        generateTimelineAnimation(masterSlide, slide->startTime(), slide->endTime(), true, output, tabLevel + 2);
        output << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel + 1) << QStringLiteral("]") << endl;

        // Keyframe groups for master + current slide
        // Get a list off all animations for the master and first slide
        auto animations = masterSlide->animations();
        animations.append(slide->animations());
        // Create a list of KeyframeGroups
        KeyframeGroupGenerator generator;

        for (auto animation: animations) {
            generator.addAnimation(animation);
        }

        generator.generateKeyframeGroups(output, tabLevel + 1);

        output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("}") << endl;
        slide = static_cast<Slide*>(slide->nextSibling());
    }
}

void UipImporter::generateStatesFromSlides(Slide *masterSlide, QTextStream &output, int tabLevel)
{
    output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("states: [") << endl;

    auto slide = static_cast<Slide*>(masterSlide->firstChild());
    bool isFirst = true;
    QString firstStateName;
    while (slide) {
        if (isFirst) {
            isFirst = false;
            firstStateName = slide->m_name;
        } else {
            output << QStringLiteral(",") << endl;
        }

        output << QDemonQmlUtilities::insertTabs(tabLevel+1) << QStringLiteral("State {") << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel+2) << QStringLiteral("name: \"") << slide->m_name << QStringLiteral("\"") << endl;

        // Add property changes here
        // First enable the Timeline for this state
        output << QDemonQmlUtilities::insertTabs(tabLevel+2) << QStringLiteral("PropertyChanges {") << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel+3) << QStringLiteral("target: ") << QDemonQmlUtilities::sanitizeQmlId(slide->m_name + QStringLiteral("Timeline")) << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel+3) << QStringLiteral("enabled: true") << endl;
        output << QDemonQmlUtilities::insertTabs(tabLevel+2) << QStringLiteral("}") << endl;

        // Now all other properties changed by the slide
        auto changeList = slide->propertyChanges();
        for (auto it = changeList.cbegin(), ite = changeList.cend(); it != ite; ++it) {
            // First see if there are any property changes we actually care about
            QBuffer propertyChangesBuffer;
            propertyChangesBuffer.open(QBuffer::WriteOnly);
            QTextStream propertyChangesOutput(&propertyChangesBuffer);
            // output each of the properties changed
            it.key()->writeQmlProperties(*it.value(), propertyChangesOutput, tabLevel + 3);
            propertyChangesBuffer.close();

            if (!propertyChangesBuffer.data().isEmpty()) {

                output << QDemonQmlUtilities::insertTabs(tabLevel+2) << QStringLiteral("PropertyChanges {") << endl;
                output << QDemonQmlUtilities::insertTabs(tabLevel+3) << QStringLiteral("target: ") << it.key()->qmlId() << endl;

                output << propertyChangesBuffer.data();

                output << QDemonQmlUtilities::insertTabs(tabLevel+2) << QStringLiteral("}") << endl;
            }
        }

        output << QDemonQmlUtilities::insertTabs(tabLevel+1) << QStringLiteral("}");

        slide = static_cast<Slide*>(slide->nextSibling());
    }

    output << endl;

    output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("]") << endl;

    // Set the initial state (works correctly even when empty)
    output << QDemonQmlUtilities::insertTabs(tabLevel) << QStringLiteral("state: \"") << firstStateName << QStringLiteral("\"") << endl;
 }

void UipImporter::generateComponent(GraphObject *component)
{
    QDir componentPath = m_exportPath.absolutePath() + QDir::separator() + QStringLiteral("components");

    QString componentName = QDemonQmlUtilities::qmlComponentName(component->qmlId());
    QString targetFileName = componentPath.absolutePath() + QDir::separator() + componentName + QStringLiteral(".qml");
    QFile componentFile(targetFileName);
    if (!componentFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not write to file: " << componentFile;
        return;
    }

    QTextStream output(&componentFile);
    writeHeader(output);

    output << QStringLiteral("Node {") << endl;
    component->writeQmlProperties(output, 1);

    processNode(component->firstChild(), output, 1);

    // Generate Animation Timeline
    auto componentNode = static_cast<ComponentNode*>(component);
    generateAnimationTimeLine(componentNode->m_masterSlide, output, 1);

    // Generate States from Slides
    generateStatesFromSlides(componentNode->m_masterSlide, output, 1);

    // Footer
    component->writeQmlFooter(output, 0);

    componentFile.close();
    m_generatedFiles += targetFileName;
}

void UipImporter::writeHeader(QTextStream &output)
{
    output << "import QtQuick3D 1.0" << endl;
    output << "import QtQuick 2.12" << endl;
    output << "import QtQuick.Timeline 1.0" << endl;
    if (m_referencedMaterials.count() > 0) {
        output << "import \"../materials\" as Materials" << endl;
    }

    if (m_aliasNodes.count() > 0) {
        output << "import \"../aliases\" as Aliases" << endl;
    }

    if (m_componentNodes.count() > 0) {
        output << "import \"../components\"" << endl;
    }

    if (m_hasQMLSubPresentations)
        output << "import \"../qml\"" << endl;

    output << endl;
}

void UipImporter::generateApplicationComponent(const QString &initialPresentationComponent, const QSize &size)
{
    // Create File
    QString targetFileName = m_exportPath.absolutePath() + QDir::separator() + initialPresentationComponent + QStringLiteral("Window.qml");
    QFile applicationComponentFile(targetFileName);
    if (!applicationComponentFile.open(QIODevice::WriteOnly)) {
        qDebug() << "couldn't open " << targetFileName << " for writing";
        return;
    }

    QTextStream output(&applicationComponentFile);

    // Header
    output << "import QtQuick 2.12" << endl;
    output << "import QtQuick.Window 2.12" << endl;
    output << "import \"presentations\"" << endl;
    output << endl;

    // Window
    output << "Window {" << endl;
    output << QDemonQmlUtilities::insertTabs(1) << "width: " << size.width() << endl;
    output << QDemonQmlUtilities::insertTabs(1) << "height: " << size.height() << endl;
    output << QDemonQmlUtilities::insertTabs(1) << "title: " << "\"" << initialPresentationComponent << "\"" << endl;
    output << endl;

    // Component
    output << QDemonQmlUtilities::insertTabs(1) << initialPresentationComponent << " {" << endl;
    output << QDemonQmlUtilities::insertTabs(2) << "anchors.fill: parent" << endl;
    output << QDemonQmlUtilities::insertTabs(1) << "}" << endl;

    output << "}" << endl;

    applicationComponentFile.close();
    m_generatedFiles += targetFileName;
}

QString UipImporter::processUipPresentation(UipPresentation *presentation, const QString &ouputFilePath)
{
    m_referencedMaterials.clear();
    m_aliasNodes.clear();
    m_componentNodes.clear();
    m_presentation = presentation;

    // Apply the properties of the first slide before running generator
    Slide *firstSlide = static_cast<Slide*>(presentation->masterSlide()->firstChild());
    if (firstSlide)
        presentation->applySlidePropertyChanges(firstSlide);
    QString errorString;

    // create one component per layer
    GraphObject *layer = presentation->scene()->firstChild();
    QHash<QString, QBuffer *> layerComponentsMap;
    while (layer) {
        if (layer->type() == GraphObject::Layer) {    
            // Create qml component from .uip presentation
            QString targetFile = ouputFilePath + QDemonQmlUtilities::qmlComponentName(presentation->name()) + QDemonQmlUtilities::qmlComponentName(layer->qmlId());
            QBuffer *qmlBuffer = new QBuffer();
            qmlBuffer->open(QIODevice::WriteOnly);
            QTextStream output(qmlBuffer);
            processNode(layer, output, 0, false);
            qmlBuffer->close();
            layerComponentsMap.insert(targetFile, qmlBuffer);

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

   // create aliases folder
    if (m_referencedMaterials.count() > 0) {
        m_exportPath.mkdir("materials");
    }

    if (m_aliasNodes.count() > 0) {
        m_exportPath.mkdir("aliases");
    }

    if(m_componentNodes.count() > 0) {
        m_exportPath.mkdir("components");
    }

    // Generate Alias, Components, and ReferenceMaterials (2nd pass)
    // Use iterators because generateComponent can contain additional nested components
    QVector<ComponentNode *>::iterator componentIterator;
    for (componentIterator = m_componentNodes.begin(); componentIterator != m_componentNodes.end(); ++componentIterator)
        generateComponent(*componentIterator);

    for (auto material : m_referencedMaterials) {
        QString id = material->m_referencedMaterial_unresolved;
        if (id.startsWith("#"))
            id.remove(0, 1);
        auto obj = presentation->object(id.toUtf8());
        if (!obj) {
            qWarning("Couldn't find object with id: %s", qPrintable(id));
            continue;
        }
        generateMaterialComponent(obj);
    }

    for (auto alias : m_aliasNodes) {
        QString id = alias->m_referencedNode_unresolved;
        if (id.startsWith("#"))
            id.remove(0, 1);
        generateAliasComponent(presentation->object(id.toUtf8()));
    }

    // Generate actual files from the buffers we created
    if (m_generateWindowComponent) {
        // only one component to create
        QString outputFileName = ouputFilePath + QDemonQmlUtilities::qmlComponentName(presentation->name()) + QStringLiteral(".qml");
        QFile outputFile(outputFileName);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            errorString += QString(QStringLiteral("Could not write to file: ") + outputFileName);
        } else {
            QTextStream output(&outputFile);
            // Write header
            writeHeader(output);

            // Window header
            if (m_presentation->scene()->m_useClearColor)
                output << QStringLiteral("Rectangle {") << endl;
            else
                output << QStringLiteral("Item {") << endl;
            output << QDemonQmlUtilities::insertTabs(1) << QStringLiteral("width: ") << m_presentation->presentationWidth()<< endl;
            output << QDemonQmlUtilities::insertTabs(1) << QStringLiteral("height: ") << m_presentation->presentationHeight() << endl;
            //output << QDemonQmlUtilities::insertTabs(1) << QStringLiteral("title: \"") << m_presentation->name() << QStringLiteral("\"") << endl;
            if (m_presentation->scene()->m_useClearColor)
                output << QDemonQmlUtilities::insertTabs(1) << QStringLiteral("color: ") << QDemonQmlUtilities::colorToQml(m_presentation->scene()->m_clearColor) << endl << endl;

            // For each component buffer paste in each line with tablevel +1
            for (auto buffer : layerComponentsMap.values()) {
                buffer->open(QIODevice::ReadOnly);
                buffer->seek(0);
                while(!buffer->atEnd()) {
                    QByteArray line = buffer->readLine();
                    output << QDemonQmlUtilities::insertTabs(1) << line;
                }
                buffer->close();
                output << endl;
            }
            // Do States and AnimationTimelines here (same for all layers of the presentation)
            // Generate Animation Timeline
            generateAnimationTimeLine(m_presentation->masterSlide(), output, 1);
            // Generate States from Slides
            generateStatesFromSlides(m_presentation->masterSlide(), output, 1);

            // Window footer
            output << QStringLiteral("}") << endl;
            outputFile.close();
            m_generatedFiles += outputFileName;
        }
    } else {
        // Create a file for each component buffer
        for (auto targetName : layerComponentsMap.keys()) {
            QString targetFileName = targetName + QStringLiteral(".qml");
            QFile targetFile(targetFileName);
            if (!targetFile.open(QIODevice::WriteOnly)) {
                errorString += QString("Could not write to file: ") + targetFileName;
            } else {
                QTextStream output(&targetFile);
                writeHeader(output);
                QBuffer *componentBuffer = layerComponentsMap.value(targetName);
                componentBuffer->open(QIODevice::ReadOnly);
                output << componentBuffer->readAll();
                componentBuffer->close();
                targetFile.close();
                m_generatedFiles += targetFileName;
            }
        }
    }

    // Cleanup
    for (auto buffer : layerComponentsMap.values())
        delete buffer;

    return errorString;
}

QT_END_NAMESPACE
