/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPrintEngine.h"

#include <QPainter>
#include <QPagedPaintDevice>
#include <QDateTime>

#include <kis_debug.h>

#include "kis_image.h"
#include "KisPrintSettings.h"
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionTransformation.h>

KisPrintEngine::KisPrintEngine(QObject *parent)
    : QObject(parent)
{
}

KisPrintEngine::~KisPrintEngine()
{
}

bool KisPrintEngine::print(KisImageSP image, KisPrintSettings *settings, QPrinter *printer)
{
    if (!image || !settings || !printer) {
        Q_EMIT printingError(tr("Invalid parameters for printing"));
        return false;
    }

    Q_EMIT printingStarted();

    try {
        QPainter painter;
        if (!painter.begin(printer)) {
            Q_EMIT printingError(tr("Failed to start printer"));
            return false;
        }

        // Get page rect in device coordinates
        QRectF pageRect = printer->pageRect(QPrinter::DevicePixel);

        // Render the page
        renderPage(painter, image, settings, pageRect);

        painter.end();

        Q_EMIT printingProgress(100);
        Q_EMIT printingFinished(true);
        return true;

    } catch (const std::exception &e) {
        Q_EMIT printingError(QString("Printing failed: %1").arg(e.what()));
        return false;
    }
}

void KisPrintEngine::renderPage(QPainter &painter, KisImageSP image, 
                                KisPrintSettings *settings, const QRectF &pageRect)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Calculate margins in device pixels
    double dpmX = painter.device()->logicalDpiX() / 25.4; // dots per mm
    double dpmY = painter.device()->logicalDpiY() / 25.4;

    double marginLeft = settings->leftMargin() * dpmX;
    double marginTop = settings->topMargin() * dpmY;
    double marginRight = settings->rightMargin() * dpmX;
    double marginBottom = settings->bottomMargin() * dpmY;

    QRectF printRect = pageRect.adjusted(marginLeft, marginTop, -marginRight, -marginBottom);

    // Calculate image rectangle
    double imageAspect = static_cast<double>(image->width()) / image->height();
    double printAspect = printRect.width() / printRect.height();

    QRectF imageRect;

    if (settings->scalingMode() == KisPrintSettings::FitToPage) {
        // Fit image to print area maintaining aspect ratio
        if (settings->lockAspectRatio()) {
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
            imageRect = printRect;
        }
    } else if (settings->scalingMode() == KisPrintSettings::Percentage) {
        // Scale by percentage
        double scale = settings->scalePercentage() / 100.0;
        QSizeF scaledSize = printRect.size() * scale;
        imageRect = QRectF(printRect.center() - QPointF(scaledSize.width() / 2, scaledSize.height() / 2),
                          scaledSize);
    } else {
        // Specific dimensions - simplified
        imageRect = printRect;
    }

    // Apply position mode
    if (settings->positionMode() == KisPrintSettings::Center) {
        // Already centered in fit calculations
    } else if (settings->positionMode() == KisPrintSettings::TopLeft) {
        imageRect.moveTopLeft(printRect.topLeft());
    } else if (settings->positionMode() == KisPrintSettings::Custom) {
        QPointF customPos = settings->customPosition();
        imageRect.moveTopLeft(QPointF(customPos.x() * dpmX, customPos.y() * dpmY));
    }

    // Convert image to QImage with color management
    QImage qImage;
    if (settings->printerProfile()) {
        qImage = convertColorSpace(image, settings->printerProfile(), settings);
    } else {
        // Use sRGB as default
        qImage = image->convertToQImage(nullptr, 0, 0, image->width(), image->height(),
                                       QSize(), settings->renderingIntent());
    }

    // Apply image adjustments
    if (settings->brightness() != 0 || settings->contrast() != 0 || settings->saturation() != 0) {
        qImage = applyAdjustments(qImage, settings);
    }

    // Draw the image
    painter.drawImage(imageRect, qImage);

    // Draw print marks if enabled
    drawPrintMarks(painter, pageRect, imageRect, settings);
}

void KisPrintEngine::drawPrintMarks(QPainter &painter, const QRectF &pageRect,
                                   const QRectF &imageRect, KisPrintSettings *settings)
{
    if (settings->cropMarks()) {
        drawCropMarks(painter, imageRect);
    }

    if (settings->registrationMarks()) {
        drawRegistrationMarks(painter, pageRect);
    }

    if (settings->colorBars()) {
        drawColorBars(painter, pageRect);
    }

    if (settings->pageInfo()) {
        drawPageInfo(painter, pageRect, "document.kra");
    }
}

void KisPrintEngine::drawCropMarks(QPainter &painter, const QRectF &rect)
{
    painter.save();
    painter.setPen(QPen(Qt::black, 1));

    double markLength = 20.0;
    double offset = 10.0;

    // Top-left
    painter.drawLine(QPointF(rect.left() - offset, rect.top()),
                    QPointF(rect.left() - offset - markLength, rect.top()));
    painter.drawLine(QPointF(rect.left(), rect.top() - offset),
                    QPointF(rect.left(), rect.top() - offset - markLength));

    // Top-right
    painter.drawLine(QPointF(rect.right() + offset, rect.top()),
                    QPointF(rect.right() + offset + markLength, rect.top()));
    painter.drawLine(QPointF(rect.right(), rect.top() - offset),
                    QPointF(rect.right(), rect.top() - offset - markLength));

    // Bottom-left
    painter.drawLine(QPointF(rect.left() - offset, rect.bottom()),
                    QPointF(rect.left() - offset - markLength, rect.bottom()));
    painter.drawLine(QPointF(rect.left(), rect.bottom() + offset),
                    QPointF(rect.left(), rect.bottom() + offset + markLength));

    // Bottom-right
    painter.drawLine(QPointF(rect.right() + offset, rect.bottom()),
                    QPointF(rect.right() + offset + markLength, rect.bottom()));
    painter.drawLine(QPointF(rect.right(), rect.bottom() + offset),
                    QPointF(rect.right(), rect.bottom() + offset + markLength));

    painter.restore();
}

void KisPrintEngine::drawRegistrationMarks(QPainter &painter, const QRectF &rect)
{
    painter.save();
    painter.setPen(QPen(Qt::black, 1));

    double radius = 10.0;
    double offset = 30.0;

    // Draw crosshair marks at corners
    QPointF centers[] = {
        QPointF(rect.left() + offset, rect.top() + offset),
        QPointF(rect.right() - offset, rect.top() + offset),
        QPointF(rect.left() + offset, rect.bottom() - offset),
        QPointF(rect.right() - offset, rect.bottom() - offset)
    };

    for (const QPointF &center : centers) {
        painter.drawEllipse(center, radius, radius);
        painter.drawLine(center - QPointF(radius * 1.5, 0), center + QPointF(radius * 1.5, 0));
        painter.drawLine(center - QPointF(0, radius * 1.5), center + QPointF(0, radius * 1.5));
    }

    painter.restore();
}

void KisPrintEngine::drawColorBars(QPainter &painter, const QRectF &rect)
{
    painter.save();

    double barHeight = 20.0;
    double barWidth = rect.width() / 8.0;

    QColor colors[] = {
        QColor(0, 255, 255),    // Cyan
        QColor(255, 0, 255),    // Magenta
        QColor(255, 255, 0),    // Yellow
        QColor(0, 0, 0),        // Black
        QColor(255, 0, 0),      // Red
        QColor(0, 255, 0),      // Green
        QColor(0, 0, 255),      // Blue
        QColor(255, 255, 255)   // White
    };

    double x = rect.left();
    double y = rect.bottom() + 30;

    for (int i = 0; i < 8; ++i) {
        painter.fillRect(QRectF(x, y, barWidth, barHeight), colors[i]);
        x += barWidth;
    }

    painter.restore();
}

void KisPrintEngine::drawPageInfo(QPainter &painter, const QRectF &pageRect, 
                                 const QString &filename)
{
    painter.save();
    painter.setPen(Qt::black);

    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    QString info = QString("%1 - %2")
        .arg(filename)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

    painter.drawText(QRectF(pageRect.left(), pageRect.bottom() + 60,
                           pageRect.width(), 20),
                    Qt::AlignCenter, info);

    painter.restore();
}

QImage KisPrintEngine::convertColorSpace(KisImageSP image, 
                                        const KoColorProfile *targetProfile,
                                        KisPrintSettings *settings)
{
    Q_UNUSED(targetProfile);
    
    // For now, use the simple conversion
    // TODO: Implement proper color space conversion with LCMS
    return image->convertToQImage(nullptr, 0, 0, image->width(), image->height(),
                                 QSize(), settings->renderingIntent());
}

QImage KisPrintEngine::applyAdjustments(const QImage &image, KisPrintSettings *settings)
{
    QImage result = image.copy();

    int brightness = settings->brightness();
    int contrast = settings->contrast();
    int saturation = settings->saturation();

    // Apply simple adjustments
    // This is a simplified implementation - full implementation would use proper color transforms
    if (brightness != 0 || contrast != 0 || saturation != 0) {
        dbgUI << "Applying image adjustments: brightness=" << brightness 
              << ", contrast=" << contrast << ", saturation=" << saturation;
        // TODO: Implement actual adjustment filters
    }

    return result;
}
