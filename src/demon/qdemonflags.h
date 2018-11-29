/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
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

#ifndef QDEMONFLAGS_H
#define QDEMONFLAGS_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

template <typename enumtype, typename storagetype = quint32>
class QDemonFlags
{
public:
    QDemonFlags();
    QDemonFlags(enumtype e);
    QDemonFlags(const QDemonFlags<enumtype, storagetype> &f);
    explicit QDemonFlags(storagetype b);

    bool operator==(enumtype e) const;
    bool operator==(const QDemonFlags<enumtype, storagetype> &f) const;
    bool operator==(bool b) const;
    bool operator!=(enumtype e) const;
    bool operator!=(const QDemonFlags<enumtype, storagetype> &f) const;

    QDemonFlags<enumtype, storagetype> &operator=(enumtype e);

    QDemonFlags<enumtype, storagetype> &operator|=(enumtype e);
    QDemonFlags<enumtype, storagetype> &operator|=(const QDemonFlags<enumtype, storagetype> &f);
    QDemonFlags<enumtype, storagetype> operator|(enumtype e) const;
    QDemonFlags<enumtype, storagetype>
    operator|(const QDemonFlags<enumtype, storagetype> &f) const;

    QDemonFlags<enumtype, storagetype> &operator&=(enumtype e);
    QDemonFlags<enumtype, storagetype> &operator&=(const QDemonFlags<enumtype, storagetype> &f);
    QDemonFlags<enumtype, storagetype> operator&(enumtype e) const;
    QDemonFlags<enumtype, storagetype>
    operator&(const QDemonFlags<enumtype, storagetype> &f) const;

    QDemonFlags<enumtype, storagetype> &operator^=(enumtype e);
    QDemonFlags<enumtype, storagetype> &operator^=(const QDemonFlags<enumtype, storagetype> &f);
    QDemonFlags<enumtype, storagetype> operator^(enumtype e) const;
    QDemonFlags<enumtype, storagetype>
    operator^(const QDemonFlags<enumtype, storagetype> &f) const;

    QDemonFlags<enumtype, storagetype> operator~(void)const;

    operator bool(void) const;
    operator quint8(void) const;
    operator quint16(void) const;
    operator quint32(void) const;

    void clear(enumtype e);

    void clearOrSet(bool value, enumtype enumVal);

public:
    friend  QDemonFlags<enumtype, storagetype> operator&(enumtype a,
                                                         QDemonFlags<enumtype, storagetype> &b)
    {
        QDemonFlags<enumtype, storagetype> out;
        out.mBits = a & b.mBits;
        return out;
    }

private:
    storagetype mBits;
};

#define QDEMON_FLAGS_OPERATORS(enumtype, storagetype)                                                  \
    QDemonFlags<enumtype, storagetype> operator|(enumtype a, enumtype b)                     \
{                                                                                              \
    QDemonFlags<enumtype, storagetype> r(a);                                                       \
    r |= b;                                                                                    \
    return r;                                                                                  \
    }                                                                                              \
    QDemonFlags<enumtype, storagetype> operator&(enumtype a, enumtype b)                     \
{                                                                                              \
    QDemonFlags<enumtype, storagetype> r(a);                                                       \
    r &= b;                                                                                    \
    return r;                                                                                  \
    }                                                                                              \
    QDemonFlags<enumtype, storagetype> operator~(enumtype a)                                 \
{                                                                                              \
    return ~QDemonFlags<enumtype, storagetype>(a);                                                 \
    }

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::QDemonFlags()
{
    mBits = 0;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::QDemonFlags(enumtype e)
{
    mBits = static_cast<storagetype>(e);
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::QDemonFlags(const QDemonFlags<enumtype, storagetype> &f)
{
    mBits = f.mBits;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::QDemonFlags(storagetype b)
{
    mBits = b;
}

template <typename enumtype, typename storagetype>
bool QDemonFlags<enumtype, storagetype>::operator==(enumtype e) const
{
    return mBits == static_cast<storagetype>(e);
}

template <typename enumtype, typename storagetype>
bool QDemonFlags<enumtype, storagetype>::
operator==(const QDemonFlags<enumtype, storagetype> &f) const
{
    return mBits == f.mBits;
}

template <typename enumtype, typename storagetype>
bool QDemonFlags<enumtype, storagetype>::operator==(bool b) const
{
    return (bool(*this)) == b;
}

template <typename enumtype, typename storagetype>
bool QDemonFlags<enumtype, storagetype>::operator!=(enumtype e) const
{
    return mBits != static_cast<storagetype>(e);
}

template <typename enumtype, typename storagetype>
bool QDemonFlags<enumtype, storagetype>::
operator!=(const QDemonFlags<enumtype, storagetype> &f) const
{
    return mBits != f.mBits;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::operator=(enumtype e)
{
    mBits = static_cast<storagetype>(e);
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::operator|=(enumtype e)
{
    mBits |= static_cast<storagetype>(e);
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::
operator|=(const QDemonFlags<enumtype, storagetype> &f)
{
    mBits |= f.mBits;
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::operator|(enumtype e) const
{
    QDemonFlags<enumtype, storagetype> out(*this);
    out |= e;
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::
operator|(const QDemonFlags<enumtype, storagetype> &f) const
{
    QDemonFlags<enumtype, storagetype> out(*this);
    out |= f;
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::operator&=(enumtype e)
{
    mBits &= static_cast<storagetype>(e);
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::
operator&=(const QDemonFlags<enumtype, storagetype> &f)
{
    mBits &= f.mBits;
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::operator&(enumtype e) const
{
    QDemonFlags<enumtype, storagetype> out = *this;
    out.mBits &= static_cast<storagetype>(e);
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::
operator&(const QDemonFlags<enumtype, storagetype> &f) const
{
    QDemonFlags<enumtype, storagetype> out = *this;
    out.mBits &= f.mBits;
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::operator^=(enumtype e)
{
    mBits ^= static_cast<storagetype>(e);
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> &QDemonFlags<enumtype, storagetype>::
operator^=(const QDemonFlags<enumtype, storagetype> &f)
{
    mBits ^= f.mBits;
    return *this;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::operator^(enumtype e) const
{
    QDemonFlags<enumtype, storagetype> out = *this;
    out.mBits ^= static_cast<storagetype>(e);
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::
operator^(const QDemonFlags<enumtype, storagetype> &f) const
{
    QDemonFlags<enumtype, storagetype> out = *this;
    out.mBits ^= f.mBits;
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype> QDemonFlags<enumtype, storagetype>::operator~(void)const
{
    QDemonFlags<enumtype, storagetype> out;
    out.mBits = ~mBits;
    return out;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::operator bool(void) const
{
    return mBits ? true : false;
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::operator quint8(void) const
{
    return static_cast<quint8>(mBits);
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::operator quint16(void) const
{
    return static_cast<quint16>(mBits);
}

template <typename enumtype, typename storagetype>
QDemonFlags<enumtype, storagetype>::operator quint32(void) const
{
    return static_cast<quint32>(mBits);
}

template <typename enumtype, typename storagetype>
void QDemonFlags<enumtype, storagetype>::clear(enumtype e)
{
    mBits &= ~static_cast<storagetype>(e);
}

template <typename enumtype, typename storagetype>
void QDemonFlags<enumtype, storagetype>::clearOrSet(bool value, enumtype enumVal)
{
    if (value)
        this->operator|=(enumVal);
    else
        clear(enumVal);
}

QT_END_NAMESPACE

#endif // QDEMONFLAGS_H
