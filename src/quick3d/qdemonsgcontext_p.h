#ifndef QDEMONSGCONTEXT_P_H
#define QDEMONSGCONTEXT_P_H

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


#include <QObject>

class QDemonSGContext : public QObject
{
    Q_OBJECT
public:
    explicit QDemonSGContext(QObject *parent = nullptr);
    ~QDemonSGContext() override;



};

#endif // QDEMONSGCONTEXT_P_H
