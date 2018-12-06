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
#ifndef QDEMON_RENDER_PATH_MATH_H
#define QDEMON_RENDER_PATH_MATH_H
#include <qdemonrender.h>
#include <QVector2D.h>
#include <QVector3D.h>

QT_BEGIN_NAMESPACE

namespace path {
// Solve quadratic equation in with a templated real number system.
template <typename REAL>

int quadratic(REAL b, REAL c, REAL rts[2])
{
    int nquad;
    REAL dis;
    REAL rtdis;

    dis = b * b - 4 * c;
    rts[0] = 0;
    rts[1] = 0;
    if (b == 0) {
        if (c == 0) {
            nquad = 2;
        } else {
            if (c < 0) {
                nquad = 2;
                rts[0] = sqrt(-c);
                rts[1] = -rts[0];
            } else {
                nquad = 0;
            }
        }
    } else if (c == 0) {
        nquad = 2;
        rts[0] = -b;
    } else if (dis >= 0) {
        nquad = 2;
        rtdis = sqrt(dis);
        if (b > 0)
            rts[0] = (-b - rtdis) * (1 / REAL(2));
        else
            rts[0] = (-b + rtdis) * (1 / REAL(2));
        if (rts[0] == 0)
            rts[1] = -b;
        else
            rts[1] = c / rts[0];
    } else {
        nquad = 0;
    }

    return (nquad);
} /* quadratic */

float interest_range[2] = {0, 1};

void cubicInflectionPoint(const QVector2D cp[4], nvvector<float> &key_point)
{
    // Convert control points to cubic monomial polynomial coefficients
    const QVector2D A = cp[3] - cp[0] + (cp[1] - cp[2]) * 3.0;
    const QVector2D B = (cp[0] - cp[1] * 2.0 + cp[2]) * 3.0, C = (cp[1] - cp[0]) * 3.0;
    const QVector2D D = cp[0];

    double a = 3 * (B.x * A.y - A.x * B.y);
    double b = 3 * (C.x * A.y - C.y * A.x);
    double c = C.x * B.y - C.y * B.x;

    double roots[2];
    int solutions;
    // Is the quadratic really a degenerate line?
    if (a == 0) {
        // Is the line really a degenerate point?
        if (b == 0) {
            solutions = 0;
        } else {
            solutions = 1;
            roots[0] = c / b;
        }
    } else {
        solutions = quadratic(b / a, c / a, roots);
    }
    for (int i = 0; i < solutions; i++) {
        float t = static_cast<float>(roots[i]);

        QVector2D p = ((A * t + B) * t + C) * t + D;
        if (t >= interest_range[0] && t <= interest_range[1])
            key_point.push_back(t);
        // else; Outside range of interest, ignore.
    }
}

typedef enum {
    CT_POINT,
    CT_LINE,
    CT_QUADRATIC,
    CT_CUSP,
    CT_LOOP,
    CT_SERPENTINE
} CurveType;

static inline bool isZero(double v)
{
#if 0
    const double eps = 6e-008;

    if (fabs(v) < eps)
        return true;
    else
        return false;
#else
    return v == 0.0;
#endif
}

inline QVector3D crossv1(const QVector2D &a, const QVector2D &b)
{
    return QVector3D(a[1] - b[1], b[0] - a[0], a[0] * b[1] - a[1] * b[0]);
}

inline bool sameVertex(const QVector2D &a, const QVector2D &b)
{
    return (a.x == b.x && a.y == b.y);
}

inline bool sameVertex(const QVector3D &a, const QVector3D &b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

// This function "normalizes" the input vector so the larger of its components
// is in the range [512,1024]. Exploit integer math on the exponent bits to
// do this without expensive DP exponentiation.
inline void scaleTo512To1024(QVector2D &d, int e)
{
    union {
        quint64 u64;
        double f64;
    } x;
    int ie = 10 - (int)e + 1023;
    Q_ASSERT(ie > 0);
    x.u64 = ((quint64)ie) << 52;
    d *= static_cast<float>(x.f64);
}

inline double fastfrexp(double d, int *exponent)
{
    union {
        quint64 u64;
        double f64;
    } x;
    x.f64 = d;
    *exponent = (((int)(x.u64 >> 52)) & 0x7ff) - 0x3ff;
    x.u64 &= (1ULL << 63) - (1ULL << 52);
    x.u64 |= (0x3ffULL << 52);
    return x.f64;
}

QVector3D CreateVec3(QVector2D xy, float z)
{
    return QVector3D(xy.x, xy.y, z);
}

QVector2D GetXY(const QVector3D &data)
{
    return QVector2D(data.x, data.y);
}

CurveType cubicDoublePoint(const QVector2D points[4], nvvector<float> &key_point)
{
#if 0
    const QVector2D AA = points[3] - points[0] + (points[1] - points[2]) * 3.0;
    const QVector3D BB = (points[0] - points[1] * 2.0 + points[2]) * 3.0;
    const QVector3D CC = (points[1] - points[0]) * 3.0, DD = points[0];
#endif

    // Assume control points of the cubic curve are A, B, C, and D.
    const QVector3D A = CreateVec3(points[0], 1);
    const QVector3D B = CreateVec3(points[1], 1);
    const QVector3D C = CreateVec3(points[2], 1);
    const QVector3D D = CreateVec3(points[3], 1);

    // Compute the discriminant of the roots of
    // H(s,t) = -36*(d1^2*s^2 - d1*d2*s*t + (d2^2 - d1*d3)*t^2)
    // where H is the Hessian (the square matrix of second-order
    // partial derivatives of a function) of I(s,t)
    // where I(s,t) determine the inflection points of the cubic
    // Bezier curve C(s,t).
    //
    // d1, d2, and d3 functions of the determinants constructed
    // from the cubic control points.
    //
    // Recall dot(a,cross(b,c)) is determinant of a 3x3 matrix
    // with a, b, c the rows of the matrix.
    const QVector3D DC = crossv1(GetXY(D), GetXY(C));
    const QVector3D AD = crossv1(GetXY(A), GetXY(D));
    const QVector3D BA = crossv1(GetXY(B), GetXY(A));

    const double a1 = A.dot(DC);
    const double a2 = B.dot(AD);
    const double a3 = C.dot(BA);
    const double d1 = a1 - 2 * a2 + 3 * a3;
    const double d2 = -a2 + 3 * a3;
    const double d3 = 3 * a3;
    const double discriminant = (3 * d2 * d2 - 4 * d1 * d3);

    // The sign of the discriminant of I classifies the curbic curve
    // C into one of 6 classifications:
    // 1) discriminant>0 ==> serpentine
    // 2) discriminant=0 ==> cusp
    // 3) discriminant<0 ==> loop

    // If the discriminant or d1 are almost but not exactly zero, the
    // result is really noisy unacceptable (k,l,m) interpolation.
    // If it looks almost like a quadratic or linear case, treat it that way.
    if (isZero(discriminant) && isZero(d1)) {
        // Cusp case

        if (isZero(d2)) {
            // degenerate cases (points, lines, quadratics)...
            if (isZero(d3)) {
                if (sameVertex(A, B) && sameVertex(A, C) && sameVertex(A, D))
                    return CT_POINT;
                else
                    return CT_LINE;
            } else {
                return CT_QUADRATIC;
            }
        } else {
            return CT_CUSP;
        }
    } else if (discriminant < 0) {
        // Loop case

        const float t = static_cast<float>(d2 + sqrt(-discriminant));
        QVector2D d = QVector2D(t, static_cast<float>(2 * d1));
        QVector2D e = QVector2D(static_cast<float>(2 * (d2 * d2 - d1 * d3)),
                                static_cast<float>(d1 * t));

        // There is the situation where r2=c/t results in division by zero, but
        // in this case, the two roots represent a double root at zero so
        // subsitute l for (the otherwise NaN) m in this case.
        //
        // This situation can occur when the 1st and 2nd (or 3rd and 4th?)
        // control point of a cubic Bezier path SubPath are identical.
        if (e.x == 0 && e.y == 0)
            e = d;

        // d, e, or both could be very large values.  To mitigate the risk of
        // floating-point overflow in subsequent calculations
        // scale both vectors to be in the range [768,1024] since their relative
        // scale of their x & y components is irrelevant.

        // Be careful to divide by a power-of-two to disturb mantissa bits.

        double d_max_mag = NVMax(fabs(d.x), fabs(d.y));
        int exponent;
        fastfrexp(d_max_mag, &exponent);
        scaleTo512To1024(d, exponent);

        double e_max_mag = NVMax(fabs(e.x), fabs(e.y));
        fastfrexp(e_max_mag, &exponent);
        scaleTo512To1024(e, exponent);

        const QVector2D roots = QVector2D(d.x / d.y, e.x / e.y);

        double tt;
#if 0
        tt = roots[0];
        if (tt >= interest_range[0] && tt <= interest_range[1])
            // key_point.push_back(tt);
            tt = roots[1];
        if (tt >= interest_range[0] && tt <= interest_range[1])
            // key_point.push_back(tt);
#endif
            tt = (roots[0] + roots[1]) / 2;
        if (tt >= interest_range[0] && tt <= interest_range[1])
            key_point.push_back(static_cast<float>(tt));

        return CT_LOOP;
    } else {
        Q_ASSERT(discriminant >= 0);
        cubicInflectionPoint(points, key_point);
        if (discriminant > 0) {
            // Serpentine case
            return CT_SERPENTINE;
        } else {
            // Cusp with inflection at infinity (treat like serpentine)
            return CT_CUSP;
        }
    }
}

QVector4D CreateVec4(QVector2D p1, QVector2D p2)
{
    return QVector4D(p1.x, p1.y, p2.x, p2.y);
}

QVector2D lerp(QVector2D p1, QVector2D p2, float distance)
{
    return p1 + (p2 - p1) * distance;
}

float lerp(float p1, float p2, float distance)
{
    return p1 + (p2 - p1) * distance;
}

// Using first derivative to get tangent.
// If this equation does not make immediate sense consider that it is the first derivative
// of the de Casteljau bezier expansion, not the polynomial expansion.
float TangentAt(float inT, float p1, float c1, float c2, float p2)
{
    float a = c1 - p1;
    float b = c2 - c1 - a;
    float c = p2 - c2 - a - (2.0f * b);
    float retval = 3.0f * (a + (2.0f * b * inT) + (c * inT * inT));
    return retval;
}

QVector2D midpoint(QVector2D p1, QVector2D p2)
{
    return lerp(p1, p2, .5f);
}

float LineLength(QVector2D inStart, QVector2D inStop)
{
    return (inStop - inStart).magnitude();
}

struct SCubicBezierCurve
{
    QVector2D m_Points[4];
    SCubicBezierCurve(QVector2D a1, QVector2D c1, QVector2D c2, QVector2D a2)
    {
        m_Points[0] = a1;
        m_Points[1] = c1;
        m_Points[2] = c2;
        m_Points[3] = a2;
    }

    // Normal is of course orthogonal to the tangent.
    QVector2D NormalAt(float inT) const
    {
        QVector2D tangent = QVector2D(
                    TangentAt(inT, m_Points[0].x, m_Points[1].x, m_Points[2].x, m_Points[3].x),
                TangentAt(inT, m_Points[0].y, m_Points[1].y, m_Points[2].y, m_Points[3].y));

        QVector2D result(tangent.y, -tangent.x);
        result.normalize();
        return result;
    }

    eastl::pair<SCubicBezierCurve, SCubicBezierCurve> SplitCubicBezierCurve(float inT)
    {
        // compute point on curve based on inT
        // using de Casteljau algorithm
        QVector2D p12 = lerp(m_Points[0], m_Points[1], inT);
        QVector2D p23 = lerp(m_Points[1], m_Points[2], inT);
        QVector2D p34 = lerp(m_Points[2], m_Points[3], inT);
        QVector2D p123 = lerp(p12, p23, inT);
        QVector2D p234 = lerp(p23, p34, inT);
        QVector2D p1234 = lerp(p123, p234, inT);

        return eastl::make_pair(SCubicBezierCurve(m_Points[0], p12, p123, p1234),
                SCubicBezierCurve(p1234, p234, p34, m_Points[3]));
    }
};

#if 0
static QVector2D NormalToLine( QVector2D startPoint, QVector2D endPoint )
{
    QVector2D lineDxDy = endPoint - startPoint;
    QVector2D result( lineDxDy.y, -lineDxDy.x );
    result.normalize();
    return result;
}
#endif

struct SResultCubic
{
    enum Mode {
        Normal = 0,
        BeginTaper = 1,
        EndTaper = 2,
    };
    QVector2D m_P1;
    QVector2D m_C1;
    QVector2D m_C2;
    QVector2D m_P2;
    // Location in the original data where this cubic is taken from
    quint32 m_EquationIndex;
    float m_TStart;
    float m_TStop;
    float m_Length;
    QVector2D m_TaperMultiplier; // normally 1, goes to zero at very end of taper if any taper.
    Mode m_Mode;

    SResultCubic(QVector2D inP1, QVector2D inC1, QVector2D inC2, QVector2D inP2,
                 quint32 equationIndex, float tStart, float tStop, float length)
        : m_P1(inP1)
        , m_C1(inC1)
        , m_C2(inC2)
        , m_P2(inP2)
        , m_EquationIndex(equationIndex)
        , m_TStart(tStart)
        , m_TStop(tStop)
        , m_Length(length)
        , m_TaperMultiplier(1.0f, 1.0f)
        , m_Mode(Normal)
    {
    }
    // Note the vec2 items are *not* initialized in any way here.
    SResultCubic() {}
    float GetP1Width(float inPathWidth, float beginTaperWidth, float endTaperWidth)
    {
        return GetPathWidth(inPathWidth, beginTaperWidth, endTaperWidth, 0);
    }

    float GetP2Width(float inPathWidth, float beginTaperWidth, float endTaperWidth)
    {
        return GetPathWidth(inPathWidth, beginTaperWidth, endTaperWidth, 1);
    }

    float GetPathWidth(float inPathWidth, float beginTaperWidth, float endTaperWidth,
                       quint32 inTaperIndex)
    {
        float retval = inPathWidth;
        switch (m_Mode) {
        case BeginTaper:
            retval = beginTaperWidth * m_TaperMultiplier[inTaperIndex];
            break;
        case EndTaper:
            retval = endTaperWidth * m_TaperMultiplier[inTaperIndex];
            break;
        default:
            break;
        }
        return retval;
    }
};

void PushLine(nvvector<SResultCubic> &ioResultVec, QVector2D inStart, QVector2D inStop,
              quint32 inEquationIndex)
{
    QVector2D range = inStop - inStart;
    ioResultVec.push_back(SResultCubic(inStart, inStart + range * .333f,
                                       inStart + range * .666f, inStop, inEquationIndex,
                                       0.0f, 1.0f, LineLength(inStart, inStop)));
}

struct PathDirtyFlagValues
{
    enum Enum {
        SourceData = 1,
        PathType = 1 << 1,
        Width = 1 << 2,
        BeginTaper = 1 << 3,
        EndTaper = 1 << 4,
        CPUError = 1 << 5,
    };
};

struct SPathDirtyFlags : public QDemonFlags<PathDirtyFlagValues::Enum>
{
    typedef QDemonFlags<PathDirtyFlagValues::Enum> TBase;
    SPathDirtyFlags() {}
    SPathDirtyFlags(int inFlags)
        : TBase(static_cast<PathDirtyFlagValues::Enum>(inFlags))
    {
    }
    void Clear()
    {
        *this = SPathDirtyFlags();
    }
};

struct STaperInformation
{
    float m_CapOffset;
    float m_CapOpacity;
    float m_CapWidth;

    STaperInformation()
        : m_CapOffset(0)
        , m_CapOpacity(0)
        , m_CapWidth(0)
    {
    }
    STaperInformation(float capOffset, float capOpacity, float capWidth)
        : m_CapOffset(capOffset)
        , m_CapOpacity(capOpacity)
        , m_CapWidth(capWidth)
    {
    }

    bool operator==(const STaperInformation &inOther) const
    {
        return m_CapOffset == inOther.m_CapOffset && m_CapOpacity == inOther.m_CapOpacity
                && m_CapWidth == inOther.m_CapWidth;
    }
};

template <typename TOptData>
bool OptionEquals(const Option<TOptData> &lhs, const Option<TOptData> &rhs)
{
    if (lhs.hasValue() != rhs.hasValue())
        return false;
    if (lhs.hasValue())
        return lhs.getValue() == rhs.getValue();
    return true;
}
void OuterAdaptiveSubdivideBezierCurve(nvvector<SResultCubic> &ioResultVec,
                                       nvvector<float> &keyPointVec,
                                       SCubicBezierCurve inCurve, float inLinearError,
                                       quint32 inEquationIndex);

void AdaptiveSubdivideBezierCurve(nvvector<SResultCubic> &ioResultVec,
                                  SCubicBezierCurve &inCurve, float inLinearError,
                                  quint32 inEquationIndex, float inTStart, float inTStop);

// Adaptively subdivide source data to produce m_PatchData.
void AdaptiveSubdivideSourceData(QDemonConstDataRef<SPathAnchorPoint> inSourceData,
                                 nvvector<SResultCubic> &ioResultVec,
                                 nvvector<float> &keyPointVec, float inLinearError)
{
    ioResultVec.clear();
    if (inSourceData.size() < 2)
        return;
    // Assuming no attributes in the source data.
    quint32 numEquations = (inSourceData.size() - 1);
    for (quint32 idx = 0, end = numEquations; idx < end; ++idx) {
        const SPathAnchorPoint &beginAnchor = inSourceData[idx];
        const SPathAnchorPoint &endAnchor = inSourceData[idx + 1];

        QVector2D anchor1(beginAnchor.m_Position);
        QVector2D control1(IPathManagerCore::GetControlPointFromAngleDistance(
                               beginAnchor.m_Position, beginAnchor.m_OutgoingAngle,
                               beginAnchor.m_OutgoingDistance));

        QVector2D control2(IPathManagerCore::GetControlPointFromAngleDistance(
                               endAnchor.m_Position, endAnchor.m_IncomingAngle,
                               endAnchor.m_IncomingDistance));
        QVector2D anchor2(endAnchor.m_Position);

        OuterAdaptiveSubdivideBezierCurve(
                    ioResultVec, keyPointVec,
                    SCubicBezierCurve(anchor1, control1, control2, anchor2), inLinearError, idx);
    }
}

// The outer subdivide function topologically analyzes the curve to ensure that
// the sign of the second derivative does not change, no inflection points.
// Once that condition is held, then we proceed with a simple adaptive subdivision algorithm
// until the curve is accurately approximated by a straight line.
void OuterAdaptiveSubdivideBezierCurve(nvvector<SResultCubic> &ioResultVec,
                                       nvvector<float> &keyPointVec,
                                       SCubicBezierCurve inCurve, float inLinearError,
                                       quint32 inEquationIndex)
{
    // Step 1, find what type of curve we are dealing with and the inflection points.
    keyPointVec.clear();
    CurveType theCurveType = cubicDoublePoint(inCurve.m_Points, keyPointVec);

    float tStart = 0;
    switch (theCurveType) {
    case CT_POINT:
        ioResultVec.push_back(SResultCubic(inCurve.m_Points[0], inCurve.m_Points[0],
                inCurve.m_Points[0], inCurve.m_Points[0],
                inEquationIndex, 0.0f, 1.0f, 0.0f));
        return; // don't allow further recursion
    case CT_LINE:
        PushLine(ioResultVec, inCurve.m_Points[0], inCurve.m_Points[3], inEquationIndex);
        return; // don't allow further recursion
    case CT_CUSP:
    case CT_LOOP:
    case CT_SERPENTINE: {
        // Break the curve at the inflection points if there is one.  If there aren't
        // inflection points
        // the treat as linear (degenerate case that should not happen except in limiting
        // ranges of floating point accuracy)
        if (!keyPointVec.empty()) {
            // It is not clear that the code results in a sorted vector,
            // or a vector where all values are within the range of 0-1
            if (keyPointVec.size() > 1)
                eastl::sort(keyPointVec.begin(), keyPointVec.end());
            for (quint32 idx = 0, end = (quint32)keyPointVec.size();
                 idx < end && keyPointVec[idx] < 1.0f; ++idx) {
                // We have a list of T values I believe sorted from beginning to end, we
                // will create a set of bezier curves
                // Since we split the curves, tValue is relative to tSTart, not 0.
                float range = 1.0f - tStart;
                float splitPoint = keyPointVec[idx] - tStart;
                float tValue = splitPoint / range;
                if (tValue > 0.0f) {
                    eastl::pair<SCubicBezierCurve, SCubicBezierCurve> newCurves
                            = inCurve.SplitCubicBezierCurve(tValue);
                    AdaptiveSubdivideBezierCurve(ioResultVec, newCurves.first,
                                                 inLinearError, inEquationIndex, tStart,
                                                 splitPoint);
                    inCurve = newCurves.second;
                    tStart = splitPoint;
                }
            }
        }
    }
        // fallthrough intentional
        break;
        // fallthrough intentional
    case CT_QUADRATIC:
        break;
    }
    AdaptiveSubdivideBezierCurve(ioResultVec, inCurve, inLinearError, inEquationIndex,
                                 tStart, 1.0f);
}

static float DistanceFromPointToLine(QVector2D inLineDxDy, QVector2D lineStart, QVector2D point)
{
    QVector2D pointToLineStart = lineStart - point;
    return fabs((inLineDxDy.x * pointToLineStart.y) - (inLineDxDy.y * pointToLineStart.x));
}

// There are two options here.  The first is to just subdivide below a given error
// tolerance.
// The second is to fit a quadratic to the curve and then precisely find the length of the
// quadratic.
// Obviously we are choosing the subdivide method at this moment but I think the fitting
// method is probably more robust.
float LengthOfBezierCurve(SCubicBezierCurve &inCurve)
{
    // Find distance of control points from line.  Note that both control points should be
    // on same side of line else we have a serpentine which should have been removed by topological
    // analysis.
    QVector2D lineDxDy = inCurve.m_Points[3] - inCurve.m_Points[0];
    float c1Distance = DistanceFromPointToLine(
                lineDxDy, inCurve.m_Points[0], inCurve.m_Points[1]);
    float c2Distance = DistanceFromPointToLine(
                lineDxDy, inCurve.m_Points[0], inCurve.m_Points[2]);
    const float lineTolerance = 100.0f; // error in world coordinates, squared.
    if (c1Distance > lineTolerance || c2Distance > lineTolerance) {
        eastl::pair<SCubicBezierCurve, SCubicBezierCurve> subdivCurve
                = inCurve.SplitCubicBezierCurve(.5f);
        return LengthOfBezierCurve(subdivCurve.first)
                + LengthOfBezierCurve(subdivCurve.second);
    } else {
        return LineLength(inCurve.m_Points[0], inCurve.m_Points[3]);
    }
}

// The assumption here is the the curve type is not cusp, loop, or serpentine.
// It is either linear or it is a constant curve meaning we can use very simple means to
// figure out the curvature.  There is a possibility to use some math to figure out the point of
// maximum curvature, where the second derivative will have a max value. This is probably not
// necessary.
void AdaptiveSubdivideBezierCurve(nvvector<SResultCubic> &ioResultVec,
                                  SCubicBezierCurve &inCurve, float inLinearError,
                                  quint32 inEquationIndex, float inTStart, float inTStop)
{
    // Find distance of control points from line.  Note that both control points should be
    // on same side of line else we have a serpentine which should have been removed by topological
    // analysis.
    QVector2D lineDxDy = inCurve.m_Points[3] - inCurve.m_Points[0];
    float c1Distance = DistanceFromPointToLine(lineDxDy, inCurve.m_Points[0],
            inCurve.m_Points[1]);
    float c2Distance = DistanceFromPointToLine(lineDxDy, inCurve.m_Points[0],
            inCurve.m_Points[2]);
    const float lineTolerance = inLinearError * inLinearError; // error in world coordinates
    if (c1Distance > lineTolerance || c2Distance > lineTolerance) {
        eastl::pair<SCubicBezierCurve, SCubicBezierCurve> subdivCurve
                = inCurve.SplitCubicBezierCurve(.5f);
        float halfway = lerp(inTStart, inTStop, .5f);
        AdaptiveSubdivideBezierCurve(ioResultVec, subdivCurve.first, inLinearError,
                                     inEquationIndex, inTStart, halfway);
        AdaptiveSubdivideBezierCurve(ioResultVec, subdivCurve.second, inLinearError,
                                     inEquationIndex, halfway, inTStop);
    } else {
        ioResultVec.push_back(SResultCubic(inCurve.m_Points[0], inCurve.m_Points[1],
                inCurve.m_Points[2], inCurve.m_Points[3],
                inEquationIndex, inTStart, inTStop,
                LengthOfBezierCurve(inCurve)));
    }
}
}
QT_END_NAMESPACE
#endif
