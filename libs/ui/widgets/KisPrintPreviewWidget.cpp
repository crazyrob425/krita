/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPrintPreviewWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRectF>

#include <klocalizedstring.h>

#include "kis_image.h"
#include "printing/KisPrintSettings.h"

KisPrintPreviewWidget::KisPrintPreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_settings(nullptr)
    , m_zoomLevel(1.0)
    , m_needsUpdate(true)
{
    setMinimumSize(300, 400);
    setBackgroundRole(QPalette::Mid);
    setAutoFillBackground(true);
}

KisPrintPreviewWidget::~KisPrintPreviewWidget()
{
}

void KisPrintPreviewWidget::setImage(KisImageSP image)
{
    m_image = image;
    m_needsUpdate = true;
    update();
}

void KisPrintPreviewWidget::setSettings(KisPrintSettings *settings)
{
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, &KisPrintSettings::settingsChanged, 
                this, &KisPrintPreviewWidget::updatePreview);
    }
    m_needsUpdate = true;
    update();
}

void KisPrintPreviewWidget::setZoomLevel(double zoom)
{
    m_zoomLevel = qBound(0.1, zoom, 5.0);
    m_needsUpdate = true;
    update();
}

void KisPrintPreviewWidget::fitToWindow()
{
    if (!m_image || !m_settings) return;
    
    // Calculate zoom to fit page in widget
    QSizeF pageSize(210, 297); // A4 in mm (default)
    double aspectRatio = pageSize.width() / pageSize.height();
    
    double widgetAspect = static_cast<double>(width()) / height();
    
    if (widgetAspect > aspectRatio) {
        // Widget is wider, fit to height
        m_zoomLevel = (height() - 40) / pageSize.height();
    } else {
        // Widget is taller, fit to width
        m_zoomLevel = (width() - 40) / pageSize.width();
    }
    
    m_needsUpdate = true;
    update();
}

void KisPrintPreviewWidget::updatePreview()
{
    m_needsUpdate = true;
    update();
}

void KisPrintPreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if (m_needsUpdate) {
        renderPreview();
        m_needsUpdate = false;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), palette().color(QPalette::Mid));
    
    if (!m_image || !m_settings) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, i18n("No image loaded"));
        return;
    }
    
    // Calculate page rectangle
    QSizeF pageSize(210, 297); // A4 in mm
    QRectF pageRect(0, 0, pageSize.width() * m_zoomLevel, 
                           pageSize.height() * m_zoomLevel);
    pageRect.moveCenter(rect().center());
    
    // Draw page (white)
    painter.fillRect(pageRect, Qt::white);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(pageRect);
    
    // Draw margins
    QPen marginPen(Qt::lightGray, 1, Qt::DashLine);
    painter.setPen(marginPen);
    
    double marginTop = m_settings->topMargin() * m_zoomLevel;
    double marginBottom = m_settings->bottomMargin() * m_zoomLevel;
    double marginLeft = m_settings->leftMargin() * m_zoomLevel;
    double marginRight = m_settings->rightMargin() * m_zoomLevel;
    
    QRectF printRect = pageRect.adjusted(marginLeft, marginTop, -marginRight, -marginBottom);
    painter.drawRect(printRect);
    
    // Draw image preview
    if (!m_preview.isNull()) {
        QRectF imageRect = calculateImageRect();
        painter.drawImage(imageRect, m_preview);
        
        // Draw image boundary
        painter.setPen(QPen(Qt::blue, 1));
        painter.drawRect(imageRect);
    }
}

void KisPrintPreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_needsUpdate = true;
}

void KisPrintPreviewWidget::renderPreview()
{
    if (!m_image) return;
    
    // Create a low-resolution preview for performance
    QSize previewSize(400, 400);
    m_preview = m_image->convertToQImage(m_image->profile(), 
                                          0, 0,
                                          m_image->width(), 
                                          m_image->height(),
                                          previewSize,
                                          KoColorConversionTransformation::IntentPerceptual);
}

QRectF KisPrintPreviewWidget::calculateImageRect()
{
    if (!m_image || !m_settings) return QRectF();
    
    // Calculate page and print area
    QSizeF pageSize(210, 297); // A4 in mm
    QRectF pageRect(0, 0, pageSize.width() * m_zoomLevel, 
                           pageSize.height() * m_zoomLevel);
    pageRect.moveCenter(rect().center());
    
    double marginTop = m_settings->topMargin() * m_zoomLevel;
    double marginBottom = m_settings->bottomMargin() * m_zoomLevel;
    double marginLeft = m_settings->leftMargin() * m_zoomLevel;
    double marginRight = m_settings->rightMargin() * m_zoomLevel;
    
    QRectF printRect = pageRect.adjusted(marginLeft, marginTop, -marginRight, -marginBottom);
    
    // Calculate image size based on scaling mode
    double imageAspect = static_cast<double>(m_image->width()) / m_image->height();
    double printAspect = printRect.width() / printRect.height();
    
    QRectF imageRect;
    
    if (m_settings->scalingMode() == KisPrintSettings::FitToPage) {
        // Fit image to print area maintaining aspect ratio
        if (imageAspect > printAspect) {
            // Image is wider, fit to width
            double height = printRect.width() / imageAspect;
            imageRect = QRectF(printRect.left(), 
                              printRect.top() + (printRect.height() - height) / 2,
                              printRect.width(), height);
        } else {
            // Image is taller, fit to height
            double width = printRect.height() * imageAspect;
            imageRect = QRectF(printRect.left() + (printRect.width() - width) / 2,
                              printRect.top(),
                              width, printRect.height());
        }
    } else {
        // Other scaling modes - simplified for now
        imageRect = printRect;
    }
    
    // Apply position mode
    if (m_settings->positionMode() == KisPrintSettings::Center) {
        // Already centered
    }
    
    return imageRect;
}
