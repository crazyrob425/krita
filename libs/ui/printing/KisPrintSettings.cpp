/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPrintSettings.h"

#include <KSharedConfig>
#include <KConfigGroup>

KisPrintSettings::KisPrintSettings(QObject *parent)
    : QObject(parent)
    , m_orientation(QPrinter::Portrait)
    , m_pageSize(QPrinter::A4)
    , m_copies(1)
    , m_collate(false)
    , m_positionMode(Center)
    , m_scalingMode(FitToPage)
    , m_scalePercentage(100.0)
    , m_lockAspectRatio(true)
    , m_marginTop(10.0)
    , m_marginBottom(10.0)
    , m_marginLeft(10.0)
    , m_marginRight(10.0)
    , m_printerProfile(nullptr)
    , m_renderingIntent(KoColorConversionTransformation::IntentPerceptual)
    , m_blackPointCompensation(true)
    , m_proofColors(false)
    , m_cropMarks(false)
    , m_registrationMarks(false)
    , m_colorBars(false)
    , m_pageInfo(false)
    , m_brightness(0)
    , m_contrast(0)
    , m_saturation(0)
    , m_printBackgroundColor(true)
{
}

KisPrintSettings::~KisPrintSettings()
{
}

void KisPrintSettings::setMargins(double top, double bottom, double left, double right)
{
    m_marginTop = top;
    m_marginBottom = bottom;
    m_marginLeft = left;
    m_marginRight = right;
}

void KisPrintSettings::loadSettings()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("PrintDialog");
    
    m_printerName = cfg.readEntry("PrinterName", QString());
    m_orientation = static_cast<QPrinter::Orientation>(cfg.readEntry("Orientation", static_cast<int>(QPrinter::Portrait)));
    m_pageSize = static_cast<QPrinter::PageSize>(cfg.readEntry("PageSize", static_cast<int>(QPrinter::A4)));
    m_copies = cfg.readEntry("Copies", 1);
    m_collate = cfg.readEntry("Collate", false);
    
    m_positionMode = static_cast<PositionMode>(cfg.readEntry("PositionMode", static_cast<int>(Center)));
    m_scalingMode = static_cast<ScalingMode>(cfg.readEntry("ScalingMode", static_cast<int>(FitToPage)));
    m_scalePercentage = cfg.readEntry("ScalePercentage", 100.0);
    m_lockAspectRatio = cfg.readEntry("LockAspectRatio", true);
    
    m_marginTop = cfg.readEntry("MarginTop", 10.0);
    m_marginBottom = cfg.readEntry("MarginBottom", 10.0);
    m_marginLeft = cfg.readEntry("MarginLeft", 10.0);
    m_marginRight = cfg.readEntry("MarginRight", 10.0);
    
    m_renderingIntent = static_cast<KoColorConversionTransformation::Intent>(
        cfg.readEntry("RenderingIntent", static_cast<int>(KoColorConversionTransformation::IntentPerceptual)));
    m_blackPointCompensation = cfg.readEntry("BlackPointCompensation", true);
    m_proofColors = cfg.readEntry("ProofColors", false);
    
    m_cropMarks = cfg.readEntry("CropMarks", false);
    m_registrationMarks = cfg.readEntry("RegistrationMarks", false);
    m_colorBars = cfg.readEntry("ColorBars", false);
    m_pageInfo = cfg.readEntry("PageInfo", false);
    
    m_brightness = cfg.readEntry("Brightness", 0);
    m_contrast = cfg.readEntry("Contrast", 0);
    m_saturation = cfg.readEntry("Saturation", 0);
    m_printBackgroundColor = cfg.readEntry("PrintBackgroundColor", true);
}

void KisPrintSettings::saveSettings()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("PrintDialog");
    
    cfg.writeEntry("PrinterName", m_printerName);
    cfg.writeEntry("Orientation", static_cast<int>(m_orientation));
    cfg.writeEntry("PageSize", static_cast<int>(m_pageSize));
    cfg.writeEntry("Copies", m_copies);
    cfg.writeEntry("Collate", m_collate);
    
    cfg.writeEntry("PositionMode", static_cast<int>(m_positionMode));
    cfg.writeEntry("ScalingMode", static_cast<int>(m_scalingMode));
    cfg.writeEntry("ScalePercentage", m_scalePercentage);
    cfg.writeEntry("LockAspectRatio", m_lockAspectRatio);
    
    cfg.writeEntry("MarginTop", m_marginTop);
    cfg.writeEntry("MarginBottom", m_marginBottom);
    cfg.writeEntry("MarginLeft", m_marginLeft);
    cfg.writeEntry("MarginRight", m_marginRight);
    
    cfg.writeEntry("RenderingIntent", static_cast<int>(m_renderingIntent));
    cfg.writeEntry("BlackPointCompensation", m_blackPointCompensation);
    cfg.writeEntry("ProofColors", m_proofColors);
    
    cfg.writeEntry("CropMarks", m_cropMarks);
    cfg.writeEntry("RegistrationMarks", m_registrationMarks);
    cfg.writeEntry("ColorBars", m_colorBars);
    cfg.writeEntry("PageInfo", m_pageInfo);
    
    cfg.writeEntry("Brightness", m_brightness);
    cfg.writeEntry("Contrast", m_contrast);
    cfg.writeEntry("Saturation", m_saturation);
    cfg.writeEntry("PrintBackgroundColor", m_printBackgroundColor);
    
    cfg.sync();
}
