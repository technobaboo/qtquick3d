#ifndef QDEMONOBJECT_P_H
#define QDEMONOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qdemonobject.h"
#include "qdemonwindow_p.h"

#include "qdemonobjectchangelistener_p.h"

#include <private/qobject_p.h>
#include <private/qquickstate_p.h>
#include <private/qqmlnotifier_p.h>
#include <private/qlazilyallocated_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_PRIVATE_EXPORT QDemonObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDemonObject)
public:
    static QDemonObjectPrivate *get(QDemonObject *item) { return item->d_func(); }
    static const QDemonObjectPrivate *get(const QDemonObject *item) { return item->d_func(); }

    QDemonObjectPrivate();
    ~QDemonObjectPrivate() override;
    void init(QDemonObject *parent);

    QQmlListProperty<QObject> data();
    QQmlListProperty<QObject> resources();
    QQmlListProperty<QDemonObject> children();
    QQmlListProperty<QDemonObject> visibleChildren();

    QQmlListProperty<QQuickState> states();
    QQmlListProperty<QQuickTransition> transitions();

    QString state() const;
    void setState(const QString &);

    // data property
    static void data_append(QQmlListProperty<QObject> *, QObject *);
    static int data_count(QQmlListProperty<QObject> *);
    static QObject *data_at(QQmlListProperty<QObject> *, int);
    static void data_clear(QQmlListProperty<QObject> *);

    // resources property
    static QObject *resources_at(QQmlListProperty<QObject> *, int);
    static void resources_append(QQmlListProperty<QObject> *, QObject *);
    static int resources_count(QQmlListProperty<QObject> *);
    static void resources_clear(QQmlListProperty<QObject> *);

    // children property
    static void children_append(QQmlListProperty<QDemonObject> *, QDemonObject *);
    static int children_count(QQmlListProperty<QDemonObject> *);
    static QDemonObject *children_at(QQmlListProperty<QDemonObject> *, int);
    static void children_clear(QQmlListProperty<QDemonObject> *);

    // visibleChildren property
    static void visibleChildren_append(QQmlListProperty<QDemonObject> *prop, QDemonObject *o);
    static int visibleChildren_count(QQmlListProperty<QDemonObject> *prop);
    static QDemonObject *visibleChildren_at(QQmlListProperty<QDemonObject> *prop, int index);

    void _q_resourceObjectDeleted(QObject *);
    quint64 _q_createJSWrapper(QV4::ExecutionEngine *engine);

    enum ChangeType {
        Geometry = 0x01,
        SiblingOrder = 0x02,
        Visibility = 0x04,
        Opacity = 0x08,
        Destroyed = 0x10,
        Parent = 0x20,
        Children = 0x40,
        Rotation = 0x80,
        ImplicitWidth = 0x100,
        ImplicitHeight = 0x200,
        Enabled = 0x400,
    };

    Q_DECLARE_FLAGS(ChangeTypes, ChangeType)

    struct ChangeListener
    {
        using ChangeTypes = QDemonObjectPrivate::ChangeTypes;

        ChangeListener(QDemonObjectChangeListener *l = nullptr, ChangeTypes t = nullptr) : listener(l), types(t) {}

        ChangeListener(QDemonObjectChangeListener *l) : listener(l), types(Geometry) {}

        bool operator==(const ChangeListener &other) const
        {
            return listener == other.listener && types == other.types;
        }

        QDemonObjectChangeListener *listener;
        ChangeTypes types;

        QVector<QDemonObjectPrivate::ChangeListener> changeListeners;
    };

    struct ExtraData
    {
        ExtraData();

        //        qreal z;
        //        qreal scale;
        //        qreal rotation;
        //        qreal opacity;

        //        QQuickContents *contents;
        //        QQuickScreenAttached *screenAttached;
        //        QQuickLayoutMirroringAttached* layoutDirectionAttached;
        //        QQuickEnterKeyAttached *enterKeyAttached;
        //        QQuickItemKeyFilter *keyHandler;
        //        QVector<QQuickPointerHandler *> pointerHandlers;
        //#if QT_CONFIG(quick_shadereffect)
        //        mutable QQuickItemLayer *layer;
        //#endif
        //#if QT_CONFIG(cursor)
        //        QCursor cursor;
        //#endif
        //        QPointF userTransformOriginPoint;

        //        // these do not include child items
        //        int effectRefCount;
        int hideRefCount;
        //        // updated recursively for child items as well
        //        int recursiveEffectRefCount;

        //        QSGOpacityNode *opacityNode;
        //        QQuickDefaultClipNode *clipNode;
        //        QSGRootNode *rootNode;

        //        // Mask contains() method
        //        QMetaMethod maskContains;

        QObjectList resourcesList;

        //        // Although acceptedMouseButtons is inside ExtraData, we actually store
        //        // the LeftButton flag in the extra.flag() bit.  This is because it is
        //        // extremely common to set acceptedMouseButtons to LeftButton, but very
        //        // rare to use any of the other buttons.
        //        Qt::MouseButtons acceptedMouseButtons;

        //        QQuickItem::TransformOrigin origin:5;
        //        uint transparentForPositioner : 1;

        //        // 26 bits padding
    };
    QLazilyAllocated<ExtraData> extra;

    QVector<QDemonObjectPrivate::ChangeListener> changeListeners;

    void addItemChangeListener(QDemonObjectChangeListener *listener, ChangeTypes types);
    void updateOrAddItemChangeListener(QDemonObjectChangeListener *listener, ChangeTypes types);
    void removeItemChangeListener(QDemonObjectChangeListener *, ChangeTypes types);
    //    void updateOrAddGeometryChangeListener(QDemonObjectChangeListener *listener, QQuickGeometryChange types);
    //    void updateOrRemoveGeometryChangeListener(QDemonObjectChangeListener *listener, QQuickGeometryChange types);

    QQuickStateGroup *_states();
    QQuickStateGroup *_stateGroup;

    enum DirtyType {
        TransformOrigin = 0x00000001,
        Transform = 0x00000002,
        BasicTransform = 0x00000004,
        Position = 0x00000008,
        Size = 0x00000010,

        ZValue = 0x00000020,
        Content = 0x00000040,
        Smooth = 0x00000080,
        OpacityValue = 0x00000100,
        ChildrenChanged = 0x00000200,
        ChildrenStackingChanged = 0x00000400,
        ParentChanged = 0x00000800,

        Clip = 0x00001000,
        Window = 0x00002000,

        EffectReference = 0x00008000,
        Visible = 0x00010000,
        HideReference = 0x00020000,
        Antialiasing = 0x00040000,
        // When you add an attribute here, don't forget to update
        // dirtyToString()

        TransformUpdateMask = TransformOrigin | Transform | BasicTransform | Position | Window,
        ComplexTransformUpdateMask = Transform | Window,
        ContentUpdateMask = Size | Content | Smooth | Window | Antialiasing,
        ChildrenUpdateMask = ChildrenChanged | ChildrenStackingChanged | EffectReference | Window
    };

    quint32 dirtyAttributes;
    QString dirtyToString() const;
    void dirty(DirtyType);
    void addToDirtyList();
    void removeFromDirtyList();
    QDemonObject *nextDirtyItem;
    QDemonObject **prevDirtyItem;

    bool isResourceNode() const;
    bool isSpatialNode() const;

    void setCulled(bool);

    QDemonWindow *window;
    int windowRefCount;
    inline QDemonRenderContextInterface *sceneGraphContext() const;
    inline QDemonRenderContext *sceneGraphRenderContext() const;

    QDemonObject *parentItem;

    QList<QDemonObject *> childItems;
    mutable QList<QDemonObject *> *sortedChildItems;
    QList<QDemonObject *> paintOrderChildItems() const;
    void addChild(QDemonObject *);
    void removeChild(QDemonObject *);
    void siblingOrderChanged();

    void markSortedChildrenDirty(QDemonObject *child);

    void refWindow(QDemonWindow *);
    void derefWindow();

    QDemonObject *subFocusItem;
    void updateSubFocusItem(QDemonObject *scope, bool focus);

    void itemChange(QDemonObject::ItemChange, const QDemonObject::ItemChangeData &);

    virtual void updatePolish() {}

    QDemonRenderGraphObject *spatialNode = nullptr;

    bool componentComplete;
    bool culled;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonObjectPrivate::ChangeTypes)
Q_DECLARE_TYPEINFO(QDemonObjectPrivate::ChangeListener, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif // QDEMONOBJECT_P_H
