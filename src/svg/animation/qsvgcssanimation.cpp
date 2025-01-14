// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsvgcssanimation_p.h"

QT_BEGIN_NAMESPACE

QSvgCssAnimation::QSvgCssAnimation()
{
}

QSvgCssAnimation::AnimationType QSvgCssAnimation::animationType() const
{
    return AnimationType::CSS;
}

QT_END_NAMESPACE
