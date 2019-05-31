#ifndef QDEMONITEMCHANGELISTENER_P_H
#define QDEMONITEMCHANGELISTENER_P_H

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

#include <QDemonObject>

QT_BEGIN_NAMESPACE

class QDemonObjectChangeListener
{
public:
    virtual ~QDemonObjectChangeListener() {}

    virtual void itemSiblingOrderChanged(QDemonObject *) {}
    virtual void itemVisibilityChanged(QDemonObject *) {}
    virtual void itemEnabledChanged(QDemonObject *) {}
    virtual void itemOpacityChanged(QDemonObject *) {}
    virtual void itemDestroyed(QDemonObject *) {}
    virtual void itemChildAdded(QDemonObject *, QDemonObject * /* child */) {}
    virtual void itemChildRemoved(QDemonObject *, QDemonObject * /* child */) {}
    virtual void itemParentChanged(QDemonObject *, QDemonObject * /* parent */) {}
};

QT_END_NAMESPACE

#endif // QDEMONITEMCHANGELISTENER_P_H
