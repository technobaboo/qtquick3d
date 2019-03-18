#ifndef UIPIMPORTER_H
#define UIPIMPORTER_H

#include <QtDemonAssetImport/private/qdemonassetimporter_p.h>
#include <QtDemonAssetImport/private/qdemonscenegraphtranslation_p.h>

#include "uipparser.h"
#include "uiaparser.h"

QT_BEGIN_NAMESPACE

class UipPresentation;
class ReferencedMaterial;
class ComponentNode;
class AliasNode;
class UipImporter : public QDemonAssetImporter
{
public:
    UipImporter();

    const QString name() const override;
    const QStringList inputExtensions() const override;
    const QString outputExtension() const override;
    const QString type() const override;
    const QVariantMap importOptions() const override;
    const QString import(const QString &sourceFile, const QDir &savePath, const QVariantMap &options, QStringList *generatedFiles) override;

private:
    QString processUipPresentation(UipPresentation *presentation, const QString &ouputFilePath);
    void processNode(GraphObject *object, QTextStream &output, int tabLevel, bool processSiblings = true);
    void checkForResourceFiles(GraphObject *object);
    void generateMaterialComponent(GraphObject *object);
    void generateAliasComponent(GraphObject *reference);
    void generateAnimationTimeLine(GraphObject *layer, QTextStream &output, int tabLevel);
    void generateKeyframeGroup(const AnimationTrack &animation, QTextStream &output, int tabLevel);

    QVector<QString> m_resourcesList;
    UiaParser m_uiaParser;
    UipParser m_uipParser;
    UipPresentation *m_presentation;

    QString m_sourceFile;
    QDir m_exportPath;
    QVariantMap m_options;
    QStringList m_generatedFiles;

    // per presentation
    QVector <ReferencedMaterial *> m_referencedMaterials;
    QVector <AliasNode *> m_aliasNodes;
};

QT_END_NAMESPACE

#endif // UIPIMPORTER_H