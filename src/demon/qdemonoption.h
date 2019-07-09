/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
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

#ifndef QDEMONQDemonOption_H
#define QDEMONQDemonOption_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

struct QDemonEmpty
{
};

template<typename TDataType>
class QDemonOption
{
    TDataType mData;
    bool mHasValue;

public:
    QDemonOption(const TDataType &data) : mData(data), mHasValue(true) {}
    QDemonOption(const QDemonEmpty &) : mHasValue(false) {}
    QDemonOption() : mHasValue(false) {}
    QDemonOption(const QDemonOption &other) : mData(other.mData), mHasValue(other.mHasValue) {}
    QDemonOption &operator=(const QDemonOption &other)
    {
        mData = other.mData;
        mHasValue = other.mHasValue;
        return *this;
    }

    bool isEmpty() const { return !mHasValue; }
    void setEmpty() { mHasValue = false; }
    bool hasValue() const { return mHasValue; }

    const TDataType &getValue() const
    {
        Q_ASSERT(mHasValue);
        return mData;
    }
    TDataType &getValue()
    {
        Q_ASSERT(mHasValue);
        return mData;
    }
    TDataType &unsafeGetValue() { return mData; }

    operator const TDataType &() const { return getValue(); }
    operator TDataType &() { return getValue(); }

    const TDataType *operator->() const { return &getValue(); }
    TDataType *operator->() { return &getValue(); }

    const TDataType &operator*() const { return getValue(); }
    TDataType &operator*() { return getValue(); }

    friend bool operator==(const QDemonOption &a, const QDemonOption &b) {
        return a.mHasValue == b.mHasValue && a.mData == b.mData;
    }
    friend bool operator!=(const QDemonOption &a, const QDemonOption &b) {
        return !(a == b);
    }
};

QT_END_NAMESPACE

#endif // QDEMONQDemonOption_H
