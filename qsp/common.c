/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common.h"
#include "actions.h"
#include "callbacks.h"
#include "errors.h"
#include "game.h"
#include "locations.h"
#include "mathops.h"
#include "objects.h"
#include "playlist.h"
#include "regexp.h"
#include "statements.h"
#include "text.h"
#include "time.h"
#include "variables.h"

static unsigned int qspRandX[55], qspRandY[256], qspRandZ;
static int qspRandI, qspRandJ;
QSP_BOOL qspIsDebug = QSP_FALSE;
QSPBufString qspCurDesc;
QSPBufString qspCurVars;
QSPString qspCurInput;
QSPString qspViewPath;
int qspTimerInterval = 0;
QSP_TINYINT qspCurWindowsChangedState = 0;
QSP_TINYINT qspCurWindowsDisplayState = 0;

INLINE unsigned int qspURand(void);
INLINE int qspRand(void);
INLINE double qspRandN(void);
INLINE double qspNormalCdf(double x);
INLINE double qspPoly8Value(double const c[], double x);
INLINE double qspNormalInvCdf(double p);

void qspInitRuntime(void)
{
    setlocale(LC_ALL, QSP_LOCALE);

    qspNullString = qspStringFromPair(0, 0);
    qspNullTuple = qspCopyToNewTuple(0, 0);
    qspNullVar = qspGetUnknownVar();

    qspIsDebug = QSP_FALSE;
    qspLocationState = 0;
    qspFullRefreshCount = 0;
    qspQstCRC = 0;
    qspMSCount = 0;
    qspLocs = 0;
    qspLocsCount = 0;
    qspLocsNames = 0;
    qspLocsNamesCount = 0;
    qspCurLoc = -1;
    qspTimerInterval = 0;
    qspCurWindowsDisplayState = QSP_WIN_MAIN | QSP_WIN_VARS | QSP_WIN_ACTS | QSP_WIN_OBJS | QSP_WIN_INPUT;

    qspSetSeed(0);
    qspInitVarTypes();
    qspInitSymbolClasses();
    qspInitVarsScope(&qspGlobalVars, QSP_VARSGLOBALBUCKETS);
    qspPrepareExecution(QSP_TRUE);
    qspMemClear(QSP_TRUE);
    qspInitCallbacks();
    qspInitStats();
    qspInitMath();
}

void qspTerminateRuntime(void)
{
    qspMemClear(QSP_FALSE);
    qspClearVarsScope(&qspGlobalVars); /* completely destroy the global scope */
    qspResizeWorld(0);
    qspUpdateLocsNames();
    qspTerminateMath();
    qspResetError(QSP_FALSE);
}

void qspPrepareExecution(QSP_BOOL toInit)
{
    qspResetError(toInit);

    if (!qspIsInCallback)
    {
        /* Reset local variables & switch to the global scope */
        qspClearLocalVarsScopes(qspCurrentLocalVars);
        qspCurrentLocalVars = 0;
        /* Reset location call depth */
        qspCurLocCallDepth = 0;
    }

    /* Reset the execution state */
    qspRealCurLoc = -1;
    qspRealActIndex = -1;
    qspRealLineNum = 0;
    qspRealLine = 0;

    /* Reset the state of changes */
    qspCurWindowsChangedState = 0;
}

void qspMemClear(QSP_BOOL toInit)
{
    qspClearAllIncludes(toInit);
    qspClearAllVars(toInit);
    qspClearAllObjects(toInit);
    qspClearAllActions(toInit);
    qspClearPlayList(toInit);
    qspClearAllRegExps(toInit);
    qspClearAllMathExps(toInit);
    if (!toInit)
    {
        if (qspCurDesc.Len > 0)
        {
            qspFreeBufString(&qspCurDesc);
            qspCurWindowsChangedState |= QSP_WIN_MAIN;
        }
        if (qspCurVars.Len > 0)
        {
            qspFreeBufString(&qspCurVars);
            qspCurWindowsChangedState |= QSP_WIN_VARS;
        }

        qspFreeString(&qspCurInput);
        qspFreeString(&qspViewPath);
    }
    qspCurDesc = qspNewBufString(0, 512);
    qspCurVars = qspNewBufString(0, 512);
    qspCurInput = qspNullString;
    qspViewPath = qspNullString;
}

void qspSetSeed(unsigned int seed)
{
    int i;
    qspRandX[0] = 1;
    qspRandX[1] = seed;
    for (i = 2; i < 55; ++i)
        qspRandX[i] = qspRandX[i - 1] + qspRandX[i - 2];
    qspRandI = 23;
    qspRandJ = 54;
    for (i = 255; i >= 0; --i) qspURand();
    for (i = 255; i >= 0; --i) qspRandY[i] = qspURand();
    qspRandZ = qspURand();
}

INLINE unsigned int qspURand(void)
{
    if (--qspRandI < 0) qspRandI = 54;
    if (--qspRandJ < 0) qspRandJ = 54;
    return qspRandX[qspRandJ] += qspRandX[qspRandI];
}

INLINE int qspRand(void)
{
    int i = qspRandZ >> 24;
    qspRandZ = qspRandY[i];
    if (--qspRandI < 0) qspRandI = 54;
    if (--qspRandJ < 0) qspRandJ = 54;
    qspRandY[i] = qspRandX[qspRandJ] += qspRandX[qspRandI];
    return qspRandZ & QSP_RANDMASK;
}

int qspUniformRand(int min, int max)
{
    if (min == max)
        return min;
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    return qspRand() % (max - min + 1) + min;
}

INLINE double qspRandN(void)
{
    int x;
    do
    {
        x = qspRand();
    }
    while (x == 0);
    return (double)x / (1.0 + QSP_RANDMAX);
}

INLINE double qspNormalCdf(double x)
{
    return erfc(-x / 1.41421356237309504880) / 2.0;
}

INLINE double qspPoly8Value(const double c[], double x)
{
    int i;
    double value = c[7];
    for (i = 6; i >= 0; --i)
        value = value * x + c[i];
    return value;
}

INLINE double qspNormalInvCdf(double p)
{
    /*
    Michael Wichura,
    The Percentage Points of the Normal Distribution,
    Algorithm AS 241,
    Applied Statistics,
    Volume 37, Number 3, pages 477-484, 1988.
    */
    const double a[8] = {
        3.3871328727963666080,     1.3314166789178437745E+2,
        1.9715909503065514427E+3,  1.3731693765509461125E+4,
        4.5921953931549871457E+4,  6.7265770927008700853E+4,
        3.3430575583588128105E+4,  2.5090809287301226727E+3 };
    const double b[8] = {
        1.0,                       4.2313330701600911252E+1,
        6.8718700749205790830E+2,  5.3941960214247511077E+3,
        2.1213794301586595867E+4,  3.9307895800092710610E+4,
        2.8729085735721942674E+4,  5.2264952788528545610E+3 };
    const double c[8] = {
        1.42343711074968357734,     4.63033784615654529590,
        5.76949722146069140550,     3.64784832476320460504,
        1.27045825245236838258,     2.41780725177450611770E-1,
        2.27238449892691845833E-2,  7.74545014278341407640E-4 };
    const double d[8] = {
        1.0,                        2.05319162663775882187,
        1.67638483018380384940,     6.89767334985100004550E-1,
        1.48103976427480074590E-1,  1.51986665636164571966E-2,
        5.47593808499534494600E-4,  1.05075007164441684324E-9 };
    const double e[8] = {
        6.65790464350110377720,     5.46378491116411436990,
        1.78482653991729133580,     2.96560571828504891230E-1,
        2.65321895265761230930E-2,  1.24266094738807843860E-3,
        2.71155556874348757815E-5,  2.01033439929228813265E-7 };
    const double f[8] = {
        1.0,                        5.99832206555887937690E-1,
        1.36929880922735805310E-1,  1.48753612908506148525E-2,
        7.86869131145613259100E-4,  1.84631831751005468180E-5,
        1.42151175831644588870E-7,  2.04426310338993978564E-15 };
    const double split1 = 0.425, split2 = 5.0;
    const double const1 = 0.180625, const2 = 1.6;

    double value;
    double q, r;

    q = p - 0.5;

    if (fabs(q) <= split1)
    {
        r = const1 - q * q;
        return q * qspPoly8Value(a, r) / qspPoly8Value(b, r);
    }

    r = (q < 0.0 ? p : 1.0 - p);
    r = sqrt(-log(r));

    if (r <= split2)
    {
        r = r - const2;
        value = qspPoly8Value(c, r) / qspPoly8Value(d, r);
    }
    else
    {
        r = r - split2;
        value = qspPoly8Value(e, r) / qspPoly8Value(f, r);
    }

    if (q < 0.0) return -value;
    return value;
}

int qspNormalRand(int min, int max, int mean)
{
    /*
    Zdravko Botev, Pierre L'Ecuyer,
    Simulation from the Tail of the Univariate and Multivariate Normal Distribution
    */
    double realMin, realMax, realMean, a, b, res, sigma, leftDist, rightDist, maxDist;
    if (min == max)
        return min;
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }

    realMin = (double)min;
    realMax = (double)max;
    realMean = (double)mean;

    leftDist = fabs(realMin - realMean);
    rightDist = fabs(realMax - realMean);
    maxDist = rightDist > leftDist ? rightDist : leftDist;
    sigma = maxDist / QSP_NORMAL_SCALE;

    a = (realMin - realMean) / sigma;
    b = (realMax - realMean) / sigma;
    if (a > 0.0)
    {
        double minProbComp = 1.0 - qspNormalCdf(a);
        double maxProbComp = 1.0 - qspNormalCdf(b);
        double u = minProbComp - (minProbComp - maxProbComp) * qspRandN();
        res = realMean - qspNormalInvCdf(u) * sigma;
    }
    else
    {
        double minProb = qspNormalCdf(a);
        double maxProb = qspNormalCdf(b);
        double u = minProb + (maxProb - minProb) * qspRandN();
        res = realMean + qspNormalInvCdf(u) * sigma;
    }

    /* Transform to integer bins */
    /* (res - min) / (max - min) * (max - min + 1.0) + min */
    res = (realMin - res) / (realMin - realMax) + res;

    if (res < min)
        return min;
    if (res > max)
        return max;
    return (int)res;
}

void qspSetWindowState(int type, QSP_BOOL toShow)
{
    if (toShow)
        qspCurWindowsDisplayState = (QSP_TINYINT)(qspCurWindowsDisplayState | type);
    else
        qspCurWindowsDisplayState = (QSP_TINYINT)(qspCurWindowsDisplayState & ~type);
}

QSP_TINYINT qspGetEnabledWindows(void)
{
    return (QSP_TINYINT)(qspCurWindowsDisplayState & QSP_WIN_ALL);
}

QSP_TINYINT qspGetDisabledWindows(void)
{
    return (QSP_TINYINT)(~qspCurWindowsDisplayState & QSP_WIN_ALL);
}
