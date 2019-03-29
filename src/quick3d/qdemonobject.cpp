#include "qdemonobject.h"
#include "qdemonobject_p.h"
#include "qdemonscenemanager_p.h"

#include <QtQml/private/qqmlglobal_p.h>
#include <QtQuick/private/qquickstategroup_p.h>
#include <QtQuick/private/qquickstate_p.h>

#include <private/qv4object_p.h>
#include <private/qv4qobjectwrapper_p.h>

QT_BEGIN_NAMESPACE

QDemonObject::QDemonObject(QDemonObject *parent)
    : QObject(*(new QDemonObjectPrivate), parent)
{
    Q_D(QDemonObject);
    d->init(parent);
}

QDemonObject::~QDemonObject()
{
    Q_D(QDemonObject);
    if (d->windowRefCount > 1)
        d->windowRefCount = 1; // Make sure window is set to null in next call to derefWindow().
    if (d->parentItem)
        setParentItem(nullptr);
    else if (d->sceneRenderer)
        d->derefSceneRenderer();

    // XXX todo - optimize
    while (!d->childItems.isEmpty())
        d->childItems.constFirst()->setParentItem(nullptr);

    delete d->_stateGroup;
    d->_stateGroup = nullptr;
}

void QDemonObject::update()
{
    Q_D(QDemonObject);
    d->dirty(QDemonObjectPrivate::Content);
}

void QDemonObject::setParentItem(QDemonObject *parentItem)
{
    Q_D(QDemonObject);
    if (parentItem == d->parentItem)
        return;

    if (parentItem) {
        QDemonObject *itemAncestor = parentItem;
        while (itemAncestor != nullptr) {
            if (Q_UNLIKELY(itemAncestor == this)) {
                qWarning() << "QDemonObject::setParentItem: Parent" << parentItem << "is already part of the subtree of" << this;
                return;
            }
            itemAncestor = itemAncestor->parentItem();
        }
    }

    d->removeFromDirtyList();

    QDemonObject *oldParentItem = d->parentItem;
    QDemonObject *scopeFocusedItem = nullptr;

    if (oldParentItem) {
        QDemonObjectPrivate *op = QDemonObjectPrivate::get(oldParentItem);

        const bool wasVisible = isVisible();
        op->removeChild(this);
        if (wasVisible) {
            emit oldParentItem->visibleChildrenChanged();
        }
    } else if (d->sceneRenderer) {
        d->sceneRenderer->parentlessItems.remove(this);
    }

    QDemonSceneManager *parentSceneRenderer = parentItem ? QDemonObjectPrivate::get(parentItem)->sceneRenderer : nullptr;
    if (d->sceneRenderer == parentSceneRenderer) {
        // Avoid freeing and reallocating resources if the window stays the same.
        d->parentItem = parentItem;
    } else {
        if (d->sceneRenderer)
            d->derefSceneRenderer();
        d->parentItem = parentItem;
        if (parentSceneRenderer)
            d->refSceneRenderer(parentSceneRenderer);
    }

    d->dirty(QDemonObjectPrivate::ParentChanged);

    if (d->parentItem)
        QDemonObjectPrivate::get(d->parentItem)->addChild(this);
    else if (d->sceneRenderer)
        d->sceneRenderer->parentlessItems.insert(this);

    d->itemChange(ItemParentHasChanged, d->parentItem);

    emit parentChanged(d->parentItem);
    if (isVisible() && d->parentItem)
        emit d->parentItem->visibleChildrenChanged();
}

void QDemonObject::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

void QDemonObject::setVisible(bool visible)
{
    if (m_visible == visible)
        return;

    m_visible = visible;
    emit visibleChanged(m_visible);
}

QByteArray QDemonObject::id() const
{
    return m_id;
}

QString QDemonObject::name() const
{
    return m_name;
}

QString QDemonObject::state() const
{
    Q_D(const QDemonObject);
    return d->state();
}

void QDemonObject::setState(const QString &state)
{
    Q_D(QDemonObject);
    d->setState(state);
}

QList<QDemonObject *> QDemonObject::childItems() const
{
    Q_D(const QDemonObject);
    return d->childItems;
}

QDemonSceneManager *QDemonObject::sceneRenderer() const
{
    Q_D(const QDemonObject);
    return d->sceneRenderer;
}

QDemonObject *QDemonObject::parentItem() const
{
    Q_D(const QDemonObject);
    return d->parentItem;
}

bool QDemonObject::isEnabled() const
{
    return m_enabled;
}

bool QDemonObject::isVisible() const
{
    return m_visible;
}

void QDemonObject::setName(QString name)
{
    m_name = name;
}

void QDemonObject::itemChange(QDemonObject::ItemChange change, const QDemonObject::ItemChangeData &value)
{
    if (change == ItemSceneChange)
        emit sceneRendererChanged(value.sceneRenderer);
}

QDemonObject::QDemonObject(QDemonObjectPrivate &dd, QDemonObject *parent) : QObject(dd, parent)
{
    Q_D(QDemonObject);
    d->init(parent);
}

void QDemonObject::classBegin()
{
    Q_D(QDemonObject);
    d->componentComplete = false;
    if (d->_stateGroup)
        d->_stateGroup->classBegin();
}

void QDemonObject::componentComplete()
{
    Q_D(QDemonObject);
    d->componentComplete = true;
    if (d->_stateGroup)
        d->_stateGroup->componentComplete();

    if (d->sceneRenderer && d->dirtyAttributes) {
        d->addToDirtyList();
        d->sceneRenderer->dirtyItem(this);
    }
}

QDemonObjectPrivate::QDemonObjectPrivate()
    : componentComplete(true)
    , _stateGroup(nullptr)
    , dirtyAttributes(0)
    , nextDirtyItem(nullptr)
    , prevDirtyItem(nullptr)
    , sceneRenderer(nullptr)
    , windowRefCount(0)
    , parentItem(nullptr)
    , sortedChildItems(&childItems)
    , subFocusItem(nullptr)
{
}

QDemonObjectPrivate::~QDemonObjectPrivate()
{
    if (sortedChildItems != &childItems)
        delete sortedChildItems;
}

void QDemonObjectPrivate::init(QDemonObject *parent)
{
    Q_Q(QDemonObject);

    if (parent)
        q->setParentItem(parent);
}

QQmlListProperty<QObject> QDemonObjectPrivate::data()
{
    return QQmlListProperty<QObject>(q_func(),
                                     nullptr,
                                     QDemonObjectPrivate::data_append,
                                     QDemonObjectPrivate::data_count,
                                     QDemonObjectPrivate::data_at,
                                     QDemonObjectPrivate::data_clear);
}

QQmlListProperty<QObject> QDemonObjectPrivate::resources()
{
    return QQmlListProperty<QObject>(q_func(),
                                     nullptr,
                                     QDemonObjectPrivate::resources_append,
                                     QDemonObjectPrivate::resources_count,
                                     QDemonObjectPrivate::resources_at,
                                     QDemonObjectPrivate::resources_clear);
}

QQmlListProperty<QDemonObject> QDemonObjectPrivate::children()
{
    return QQmlListProperty<QDemonObject>(q_func(),
                                          nullptr,
                                          QDemonObjectPrivate::children_append,
                                          QDemonObjectPrivate::children_count,
                                          QDemonObjectPrivate::children_at,
                                          QDemonObjectPrivate::children_clear);
}

QQmlListProperty<QDemonObject> QDemonObjectPrivate::visibleChildren()
{
    return QQmlListProperty<QDemonObject>(q_func(), nullptr, QDemonObjectPrivate::visibleChildren_count, QDemonObjectPrivate::visibleChildren_at);
}

QQmlListProperty<QQuickState> QDemonObjectPrivate::states()
{
    return _states()->statesProperty();
}

QQmlListProperty<QQuickTransition> QDemonObjectPrivate::transitions()
{
    return _states()->transitionsProperty();
}

QString QDemonObjectPrivate::state() const
{
    if (!_stateGroup)
        return QString();
    else
        return _stateGroup->state();
}

void QDemonObjectPrivate::setState(const QString &state)
{
    _states()->setState(state);
}

void QDemonObjectPrivate::data_append(QQmlListProperty<QObject> *prop, QObject *o)
{
    if (!o)
        return;

    QDemonObject *that = static_cast<QDemonObject *>(prop->object);

    if (QDemonObject *item = qmlobject_cast<QDemonObject *>(o)) {
        item->setParentItem(that);
    } else {
//        QDemonSceneRenderer *thisSceneRenderer = qmlobject_cast<QDemonSceneRenderer *>(o);
//        item = that;
//        QDemonSceneRenderer *itemSceneRenderer = that->sceneRenderer();
//        while (!itemSceneRenderer && item && item->parentItem()) {
//            item = item->parentItem();
//            itemSceneRenderer = item->sceneRenderer();
//        }

//        if (thisSceneRenderer) {
//            if (itemSceneRenderer) {
//                // qCDebug(lcTransient) << thisWindow << "is transient for" << itemWindow;
//                thisSceneRenderer->setTransientParent(itemSceneRenderer);
//            } else {
//                QObject::connect(item, SIGNAL(sceneRendererChanged(QDemonSceneRenderer *)), thisSceneRenderer, SLOT(setTransientParent_helper(QDemonSceneRenderer *)));
//            }
//        }
        o->setParent(that);
    }

    resources_append(prop, o);
}

int QDemonObjectPrivate::data_count(QQmlListProperty<QObject> *property)
{
    QDemonObject *item = static_cast<QDemonObject *>(property->object);
    QDemonObjectPrivate *privateItem = QDemonObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QDemonObject> childrenProperty = privateItem->children();

    return resources_count(&resourcesProperty) + children_count(&childrenProperty);
}

QObject *QDemonObjectPrivate::data_at(QQmlListProperty<QObject> *property, int i)
{
    QDemonObject *item = static_cast<QDemonObject *>(property->object);
    QDemonObjectPrivate *privateItem = QDemonObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QDemonObject> childrenProperty = privateItem->children();

    int resourcesCount = resources_count(&resourcesProperty);
    if (i < resourcesCount)
        return resources_at(&resourcesProperty, i);
    const int j = i - resourcesCount;
    if (j < children_count(&childrenProperty))
        return children_at(&childrenProperty, j);
    return nullptr;
}

void QDemonObjectPrivate::data_clear(QQmlListProperty<QObject> *property)
{
    QDemonObject *item = static_cast<QDemonObject *>(property->object);
    QDemonObjectPrivate *privateItem = QDemonObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QDemonObject> childrenProperty = privateItem->children();

    resources_clear(&resourcesProperty);
    children_clear(&childrenProperty);
}

QObject *QDemonObjectPrivate::resources_at(QQmlListProperty<QObject> *prop, int index)
{
    QDemonObjectPrivate *quickItemPrivate = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    return quickItemPrivate->extra.isAllocated() ? quickItemPrivate->extra->resourcesList.value(index) : 0;
}

void QDemonObjectPrivate::resources_append(QQmlListProperty<QObject> *prop, QObject *object)
{
    QDemonObject *quickItem = static_cast<QDemonObject *>(prop->object);
    QDemonObjectPrivate *quickItemPrivate = QDemonObjectPrivate::get(quickItem);
    if (!quickItemPrivate->extra.value().resourcesList.contains(object)) {
        quickItemPrivate->extra.value().resourcesList.append(object);
        // clang-format off
        qmlobject_connect(object, QObject, SIGNAL(destroyed(QObject*)),
                          quickItem, QDemonObject, SLOT(_q_resourceObjectDeleted(QObject*)));
        // clang-format on
    }
}

int QDemonObjectPrivate::resources_count(QQmlListProperty<QObject> *prop)
{
    QDemonObjectPrivate *quickItemPrivate = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    return quickItemPrivate->extra.isAllocated() ? quickItemPrivate->extra->resourcesList.count() : 0;
}

void QDemonObjectPrivate::resources_clear(QQmlListProperty<QObject> *prop)
{
    QDemonObject *quickItem = static_cast<QDemonObject *>(prop->object);
    QDemonObjectPrivate *quickItemPrivate = QDemonObjectPrivate::get(quickItem);
    if (quickItemPrivate->extra.isAllocated()) { // If extra is not allocated resources is empty.
        for (QObject *object : qAsConst(quickItemPrivate->extra->resourcesList)) {
            // clang-format off
            qmlobject_disconnect(object, QObject, SIGNAL(destroyed(QObject*)),
                                 quickItem, QDemonObject, SLOT(_q_resourceObjectDeleted(QObject*)));
            // clang-format on
        }
        quickItemPrivate->extra->resourcesList.clear();
    }
}

void QDemonObjectPrivate::children_append(QQmlListProperty<QDemonObject> *prop, QDemonObject *o)
{
    if (!o)
        return;

    QDemonObject *that = static_cast<QDemonObject *>(prop->object);
    if (o->parentItem() == that)
        o->setParentItem(nullptr);

    o->setParentItem(that);
}

int QDemonObjectPrivate::children_count(QQmlListProperty<QDemonObject> *prop)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    return p->childItems.count();
}

QDemonObject *QDemonObjectPrivate::children_at(QQmlListProperty<QDemonObject> *prop, int index)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    if (index >= p->childItems.count() || index < 0)
        return nullptr;
    else
        return p->childItems.at(index);
}

void QDemonObjectPrivate::children_clear(QQmlListProperty<QDemonObject> *prop)
{
    QDemonObject *that = static_cast<QDemonObject *>(prop->object);
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(that);
    while (!p->childItems.isEmpty())
        p->childItems.at(0)->setParentItem(nullptr);
}

int QDemonObjectPrivate::visibleChildren_count(QQmlListProperty<QDemonObject> *prop)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    int visibleCount = 0;
    int c = p->childItems.count();
    while (c--) {
        if (p->childItems.at(c)->isVisible())
            visibleCount++;
    }

    return visibleCount;
}

QDemonObject *QDemonObjectPrivate::visibleChildren_at(QQmlListProperty<QDemonObject> *prop, int index)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(static_cast<QDemonObject *>(prop->object));
    const int childCount = p->childItems.count();
    if (index >= childCount || index < 0)
        return nullptr;

    int visibleCount = -1;
    for (int i = 0; i < childCount; i++) {
        if (p->childItems.at(i)->isVisible())
            visibleCount++;
        if (visibleCount == index)
            return p->childItems.at(i);
    }
    return nullptr;
}

void QDemonObjectPrivate::_q_resourceObjectDeleted(QObject *object)
{
    if (extra.isAllocated() && extra->resourcesList.contains(object))
        extra->resourcesList.removeAll(object);
}

void QDemonObjectPrivate::addItemChangeListener(QDemonObjectChangeListener *listener, ChangeTypes types)
{
    changeListeners.append(ChangeListener(listener, types));
}

void QDemonObjectPrivate::updateOrAddItemChangeListener(QDemonObjectChangeListener *listener, ChangeTypes types)
{
    const ChangeListener changeListener(listener, types);
    const int index = changeListeners.indexOf(changeListener);
    if (index > -1)
        changeListeners[index].types = changeListener.types;
    else
        changeListeners.append(changeListener);
}

void QDemonObjectPrivate::removeItemChangeListener(QDemonObjectChangeListener *listener, ChangeTypes types)
{
    ChangeListener change(listener, types);
    changeListeners.removeOne(change);
}

QQuickStateGroup *QDemonObjectPrivate::_states()
{
    Q_Q(QDemonObject);
    if (!_stateGroup) {
        _stateGroup = new QQuickStateGroup;
        if (!componentComplete)
            _stateGroup->classBegin();
        // clang-format off
        qmlobject_connect(_stateGroup, QQuickStateGroup, SIGNAL(stateChanged(QString)),
                          q, QDemonObject, SIGNAL(stateChanged(QString)))
        // clang-format on
    }

    return _stateGroup;
}

QString QDemonObjectPrivate::dirtyToString() const
{
#define DIRTY_TO_STRING(value)                                                                                         \
    if (dirtyAttributes & value) {                                                                                     \
        if (!rv.isEmpty())                                                                                             \
            rv.append(QLatin1Char('|'));                                                                               \
        rv.append(QLatin1String(#value));                                                                              \
    }

    //    QString rv = QLatin1String("0x") + QString::number(dirtyAttributes, 16);
    QString rv;

    DIRTY_TO_STRING(TransformOrigin);
    DIRTY_TO_STRING(Transform);
    DIRTY_TO_STRING(BasicTransform);
    DIRTY_TO_STRING(Position);
    DIRTY_TO_STRING(Size);
    DIRTY_TO_STRING(ZValue);
    DIRTY_TO_STRING(Content);
    DIRTY_TO_STRING(Smooth);
    DIRTY_TO_STRING(OpacityValue);
    DIRTY_TO_STRING(ChildrenChanged);
    DIRTY_TO_STRING(ChildrenStackingChanged);
    DIRTY_TO_STRING(ParentChanged);
    DIRTY_TO_STRING(Clip);
    DIRTY_TO_STRING(Window);
    DIRTY_TO_STRING(EffectReference);
    DIRTY_TO_STRING(Visible);
    DIRTY_TO_STRING(HideReference);
    DIRTY_TO_STRING(Antialiasing);

    return rv;
}

void QDemonObjectPrivate::dirty(QDemonObjectPrivate::DirtyType type)
{
    Q_Q(QDemonObject);
    if (!(dirtyAttributes & type) || (sceneRenderer && !prevDirtyItem)) {
        dirtyAttributes |= type;
        if (sceneRenderer && componentComplete) {
            addToDirtyList();
            sceneRenderer->dirtyItem(q);
        }
    }
}

void QDemonObjectPrivate::addToDirtyList()
{
    Q_Q(QDemonObject);

    Q_ASSERT(sceneRenderer);
    if (!prevDirtyItem) {
        Q_ASSERT(!nextDirtyItem);

        if (isResourceNode()) {
            if (q->type() == QDemonObject::Image) {
                // Will likely need to refactor this, but images need to come before other
                // resources
                nextDirtyItem = sceneRenderer->dirtyImageList;
                if (nextDirtyItem)
                    QDemonObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
                prevDirtyItem = &sceneRenderer->dirtyImageList;
                sceneRenderer->dirtyImageList = q;
            } else {
                nextDirtyItem = sceneRenderer->dirtyResourceList;
                if (nextDirtyItem)
                    QDemonObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
                prevDirtyItem = &sceneRenderer->dirtyResourceList;
                sceneRenderer->dirtyResourceList = q;
            }
        } else {
            nextDirtyItem = sceneRenderer->dirtySpatialNodeList;
            if (nextDirtyItem)
                QDemonObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
            prevDirtyItem = &sceneRenderer->dirtySpatialNodeList;
            sceneRenderer->dirtySpatialNodeList = q;
        }

        sceneRenderer->dirtyItem(q);
    }
    Q_ASSERT(prevDirtyItem);
}

void QDemonObjectPrivate::removeFromDirtyList()
{
    if (prevDirtyItem) {
        if (nextDirtyItem)
            QDemonObjectPrivate::get(nextDirtyItem)->prevDirtyItem = prevDirtyItem;
        *prevDirtyItem = nextDirtyItem;
        prevDirtyItem = nullptr;
        nextDirtyItem = nullptr;
    }
    Q_ASSERT(!prevDirtyItem);
    Q_ASSERT(!nextDirtyItem);
}

bool QDemonObjectPrivate::isResourceNode() const
{
    Q_Q(const QDemonObject);
    switch (q->type()) {
    case QDemonObject::Layer:
    case QDemonObject::Node:
    case QDemonObject::Light:
    case QDemonObject::Camera:
    case QDemonObject::Model:
    case QDemonObject::Text:
    case QDemonObject::Path:
        return false;
    case QDemonObject::Presentation:
    case QDemonObject::Scene:
    case QDemonObject::DefaultMaterial:
    case QDemonObject::Image:
    case QDemonObject::Effect:
    case QDemonObject::CustomMaterial:
    case QDemonObject::ReferencedMaterial:
    case QDemonObject::PathSubPath:
    case QDemonObject::Lightmaps:
        return true;
    default:
        return false;
    }
}

bool QDemonObjectPrivate::isSpatialNode() const
{
    Q_Q(const QDemonObject);
    switch (q->type()) {
    case QDemonObject::Layer:
    case QDemonObject::Node:
    case QDemonObject::Light:
    case QDemonObject::Camera:
    case QDemonObject::Model:
    case QDemonObject::Text:
    case QDemonObject::Path:
        return true;
    case QDemonObject::Presentation:
    case QDemonObject::Scene:
    case QDemonObject::DefaultMaterial:
    case QDemonObject::Image:
    case QDemonObject::Effect:
    case QDemonObject::CustomMaterial:
    case QDemonObject::ReferencedMaterial:
    case QDemonObject::PathSubPath:
    case QDemonObject::Lightmaps:
    default:
        return false;
    }
}

void QDemonObjectPrivate::setCulled(bool cull)
{
    if (cull == culled)
        return;

    culled = cull;
    if ((cull && ++extra.value().hideRefCount == 1) || (!cull && --extra.value().hideRefCount == 0))
        dirty(HideReference);
}

//QDemonRenderContextInterface *QDemonObjectPrivate::sceneGraphContext() const
//{
//    Q_ASSERT(sceneRenderer);
//    return sceneRenderer->m_context.data();
//}

//QDemonRenderContext *QDemonObjectPrivate::sceneGraphRenderContext() const
//{
//    Q_ASSERT(sceneRenderer);
//    return sceneRenderer->m_context->renderContext().data();
//}

QList<QDemonObject *> QDemonObjectPrivate::paintOrderChildItems() const
{
    if (sortedChildItems)
        return *sortedChildItems;

    sortedChildItems = const_cast<QList<QDemonObject *> *>(&childItems);

    return childItems;
}

void QDemonObjectPrivate::addChild(QDemonObject *child)
{
    Q_Q(QDemonObject);

    Q_ASSERT(!childItems.contains(child));

    childItems.append(child);

    markSortedChildrenDirty(child);
    dirty(QDemonObjectPrivate::ChildrenChanged);

    itemChange(QDemonObject::ItemChildAddedChange, child);

    emit q->childrenChanged();
}

void QDemonObjectPrivate::removeChild(QDemonObject *child)
{
    Q_Q(QDemonObject);

    Q_ASSERT(child);
    Q_ASSERT(childItems.contains(child));
    childItems.removeOne(child);
    Q_ASSERT(!childItems.contains(child));

    markSortedChildrenDirty(child);
    dirty(QDemonObjectPrivate::ChildrenChanged);

    itemChange(QDemonObject::ItemChildRemovedChange, child);

    emit q->childrenChanged();
}

void QDemonObjectPrivate::siblingOrderChanged()
{
    Q_Q(QDemonObject);
    if (!changeListeners.isEmpty()) {
        const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
        for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
            if (change.types & QDemonObjectPrivate::SiblingOrder) {
                change.listener->itemSiblingOrderChanged(q);
            }
        }
    }
}

void QDemonObjectPrivate::markSortedChildrenDirty(QDemonObject *child)
{
}

void QDemonObjectPrivate::refSceneRenderer(QDemonSceneManager *c)
{
    // An item needs a window if it is referenced by another item which has a window.
    // Typically the item is referenced by a parent, but can also be referenced by a
    // ShaderEffect or ShaderEffectSource. 'windowRefCount' counts how many items with
    // a window is referencing this item. When the reference count goes from zero to one,
    // or one to zero, the window of this item is updated and propagated to the children.
    // As long as the reference count stays above zero, the window is unchanged.
    // refWindow() increments the reference count.
    // derefWindow() decrements the reference count.

    Q_Q(QDemonObject);
    Q_ASSERT((sceneRenderer != nullptr) == (windowRefCount > 0));
    Q_ASSERT(c);
    if (++windowRefCount > 1) {
        if (c != sceneRenderer)
            qWarning("QDemonObject: Cannot use same item on different windows at the same time.");
        return; // Window already set.
    }

    Q_ASSERT(sceneRenderer == nullptr);
    sceneRenderer = c;

    //    if (polishScheduled)
    //        QDemonWindowPrivate::get(window)->itemsToPolish.append(q);

    if (!parentItem)
        sceneRenderer->parentlessItems.insert(q);

    for (int ii = 0; ii < childItems.count(); ++ii) {
        QDemonObject *child = childItems.at(ii);
        QDemonObjectPrivate::get(child)->refSceneRenderer(c);
    }

    dirty(Window);

    itemChange(QDemonObject::ItemSceneChange, c);
}

void QDemonObjectPrivate::derefSceneRenderer()
{
    Q_Q(QDemonObject);

    if (!sceneRenderer)
        return; // This can happen when destroying recursive shader effect sources.

    if (--windowRefCount > 0)
        return; // There are still other references, so don't set window to null yet.

    removeFromDirtyList();
    QDemonSceneManager *c = sceneRenderer;

    if (spatialNode)
        c->cleanup(spatialNode);
    if (!parentItem)
        c->parentlessItems.remove(q);

    sceneRenderer = nullptr;

    spatialNode = nullptr;

    for (int ii = 0; ii < childItems.count(); ++ii) {
        QDemonObject *child = childItems.at(ii);
        QDemonObjectPrivate::get(child)->derefSceneRenderer();
    }

    dirty(Window);

    itemChange(QDemonObject::ItemSceneChange, (QDemonSceneManager *)nullptr);
}

void QDemonObjectPrivate::updateSubFocusItem(QDemonObject *scope, bool focus)
{
    Q_Q(QDemonObject);
    Q_ASSERT(scope);

    QDemonObjectPrivate *scopePrivate = QDemonObjectPrivate::get(scope);

    QDemonObject *oldSubFocusItem = scopePrivate->subFocusItem;
    // Correct focus chain in scope
    if (oldSubFocusItem) {
        QDemonObject *sfi = scopePrivate->subFocusItem->parentItem();
        while (sfi && sfi != scope) {
            QDemonObjectPrivate::get(sfi)->subFocusItem = nullptr;
            sfi = sfi->parentItem();
        }
    }

    if (focus) {
        scopePrivate->subFocusItem = q;
        QDemonObject *sfi = scopePrivate->subFocusItem->parentItem();
        while (sfi && sfi != scope) {
            QDemonObjectPrivate::get(sfi)->subFocusItem = q;
            sfi = sfi->parentItem();
        }
    } else {
        scopePrivate->subFocusItem = nullptr;
    }
}

void QDemonObjectPrivate::itemChange(QDemonObject::ItemChange change, const QDemonObject::ItemChangeData &data)
{
    Q_Q(QDemonObject);
    switch (change) {
    case QDemonObject::ItemChildAddedChange: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Children) {
                    change.listener->itemChildAdded(q, data.item);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemChildRemovedChange: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Children) {
                    change.listener->itemChildRemoved(q, data.item);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemSceneChange:
        q->itemChange(change, data);
        break;
    case QDemonObject::ItemVisibleHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Visibility) {
                    change.listener->itemVisibilityChanged(q);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemEnabledHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Enabled) {
                    change.listener->itemEnabledChanged(q);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemParentHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Parent) {
                    change.listener->itemParentChanged(q, data.item);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemOpacityHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QDemonObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QDemonObjectPrivate::Opacity) {
                    change.listener->itemOpacityChanged(q);
                }
            }
        }
        break;
    }
    case QDemonObject::ItemActiveFocusHasChanged:
        q->itemChange(change, data);
        break;
    case QDemonObject::ItemAntialiasingHasChanged:
        // fall through
    case QDemonObject::ItemDevicePixelRatioHasChanged:
        q->itemChange(change, data);
        break;
    }
}

namespace QV4 {
namespace Heap {
struct QDemonItemWrapper : public QObjectWrapper
{
    static void markObjects(QV4::Heap::Base *that, QV4::MarkStack *markStack);
};
}
}

struct QDemonItemWrapper : public QV4::QObjectWrapper
{
    V4_OBJECT2(QDemonItemWrapper, QV4::QObjectWrapper)
};

DEFINE_OBJECT_VTABLE(QDemonItemWrapper);

void QV4::Heap::QDemonItemWrapper::markObjects(QV4::Heap::Base *that, QV4::MarkStack *markStack)
{
    QObjectWrapper *This = static_cast<QObjectWrapper *>(that);
    if (QDemonObject *item = static_cast<QDemonObject *>(This->object())) {
        for (QDemonObject *child : qAsConst(QDemonObjectPrivate::get(item)->childItems))
            QV4::QObjectWrapper::markWrapper(child, markStack);
    }
    QObjectWrapper::markObjects(that, markStack);
}

quint64 QDemonObjectPrivate::_q_createJSWrapper(QV4::ExecutionEngine *engine)
{
    return (engine->memoryManager->allocate<QDemonItemWrapper>(q_func()))->asReturnedValue();
}

QDemonObjectPrivate::ExtraData::ExtraData() : hideRefCount(0) {}

QT_END_NAMESPACE

#include <moc_qdemonobject.cpp>
