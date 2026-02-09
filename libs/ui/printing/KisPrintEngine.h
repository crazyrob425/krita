/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PRINT_ENGINE_H
#define KIS_PRINT_ENGINE_H

#include <QObject>
#include <QPrinter>

#include "kritaui_export.h"
#include "kis_types.h"

class KisPrintSettings;
class KoColorProfile;

/**
 * @brief Engine for handling actual printing operations
 * 
 * Manages the print process including color conversion,
 * rendering, and print mark generation.
 */
class KRITAUI_EXPORT KisPrintEngine : public QObject
{
    Q_OBJECT

public:
    explicit KisPrintEngine(QObject *parent = nullptr);
    ~KisPrintEngine() override;

    /**
     * Print the image to the specified printer using the given settings
     */
    bool print(KisImageSP image, KisPrintSettings *settings, QPrinter *printer);

Q_SIGNALS:
    void printingStarted();
    void printingProgress(int percent);
    void printingFinished(bool success);
    void printingError(const QString &message);

private:
    void renderPage(QPainter &painter, KisImageSP image, KisPrintSettings *settings, 
                    const QRectF &pageRect);
    void drawPrintMarks(QPainter &painter, const QRectF &pageRect, 
                       const QRectF &imageRect, KisPrintSettings *settings);
    void drawCropMarks(QPainter &painter, const QRectF &rect);
    void drawRegistrationMarks(QPainter &painter, const QRectF &rect);
    void drawColorBars(QPainter &painter, const QRectF &rect);
    void drawPageInfo(QPainter &painter, const QRectF &pageRect, 
                     const QString &filename);
    
    QImage convertColorSpace(KisImageSP image, const KoColorProfile *targetProfile,
                            KisPrintSettings *settings);
    QImage applyAdjustments(const QImage &image, KisPrintSettings *settings);
};

#endif // KIS_PRINT_ENGINE_H
