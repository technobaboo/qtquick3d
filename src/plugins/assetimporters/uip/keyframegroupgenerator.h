#ifndef KEYFRAMEGROUPGENERATOR_H
#define KEYFRAMEGROUPGENERATOR_H

#include <QString>
#include <QHash>
#include "uippresentation.h"
#include "uipparser.h"

QT_BEGIN_NAMESPACE

class KeyframeGroupGenerator
{
public:
    struct KeyframeGroup {
        enum AnimationType {
            NoAnimation = 0,
            Linear,
            EaseInOut,
            Bezier
        };
        struct KeyFrame {
            enum ValueType {
                Unhandled = -1,
                Float = 0,
                Vector2D,
                Vector3D,
                Vector4D,
                Color
            };

            KeyFrame() = default;
            KeyFrame(const AnimationTrack::KeyFrame &keyframe, ValueType type, const QString &field = QStringLiteral("x"));
            void setValue(float newValue, const QString &field = QStringLiteral("x"));
            float time = 0; // seconds
            QVector4D value;
            ValueType valueType = Float;
            union {
                float easeIn;
                float c2time;
            };
            union {
                float easeOut;
                float c2value;
            };
            float c1time;
            float c1value;
            QString valueToString() const;
        };
        using KeyFrameList = QVector<KeyFrame*>;
        KeyframeGroup() = default;
        KeyframeGroup(const AnimationTrack &animation, const QString &p, const QString &field = QStringLiteral("x"));
        ~KeyframeGroup();

        void generateKeyframeGroupQml(QTextStream &output, int tabLevel) const;

        KeyFrame::ValueType getPropertyValueType(const QString &propertyName);
        QString getQmlPropertyName(const QString &propertyName);

        AnimationType type = NoAnimation;
        GraphObject *target;
        QString property;
        bool isDynamic = false;
        KeyFrameList keyframes;
    };

    KeyframeGroupGenerator();
    ~KeyframeGroupGenerator();

    void addAnimation(const AnimationTrack &animation);

    void generateKeyframeGroups(QTextStream &output, int tabLevel);

private:
    using KeyframeGroupMap = QHash<QString, KeyframeGroup *>;
    QHash<GraphObject *, KeyframeGroupMap> m_targetKeyframeMap;
};

QT_END_NAMESPACE

#endif // KEYFRAMEGROUPGENERATOR_H
