#ifndef UIPPARSER_H
#define UIPPARSER_H

#include <abstractxmlparser.h>
#include <functional>


QT_BEGIN_NAMESPACE

class UipPresentation;
class GraphObject;
class Slide;
class AnimationTrack;
class UipParser : public AbstractXmlParser
{
public:
    UipPresentation *parse(const QString &filename, const QString &presentationName);
    UipPresentation *parseData(const QByteArray &data, const QString &presentationName);

private:
    UipPresentation *createPresentation(const QString &presentationName);
    void parseUIP();
    void parseProject();
    void parseClasses();
    void parseBufferData();
    void parseImageBuffer();
    void parseGraph();
    void parseScene();
    void parseObjects(GraphObject *parent);
    void parseLogic();
    Slide *parseSlide(Slide *parent = nullptr, const QByteArray &idPrefix = QByteArray());
    void parseAddSet(Slide *slide, bool isSet, bool isMaster);
    void parseAnimationKeyFrames(const QString &data, AnimationTrack *animTrack);

    QByteArray getId(const QStringRef &desc, bool required = true);
    void resolveReferences(GraphObject *obj);

    typedef std::function<bool(const QByteArray &, const QString &)> ExternalFileLoadCallback;
    void parseExternalFileRef(ExternalFileLoadCallback callback);

    UipPresentation *m_presentation = nullptr;
};

QT_END_NAMESPACE

#endif // UIPPARSER_H
