/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PRINT_PREVIEW_WIDGET_H
#define KIS_PRINT_PREVIEW_WIDGET_H

#include <QWidget>
#include <QImage>

#include "kritaui_export.h"

class KisPrintSettings;
class KisImage;
typedef KisSharedPtr<KisImage> KisImageSP;

/**
 * @brief Widget for displaying print preview
 * 
 * Shows a live preview of how the image will be printed on the page,
 * including page boundaries, margins, and print area.
 */
class KRITAUI_EXPORT KisPrintPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KisPrintPreviewWidget(QWidget *parent = nullptr);
    ~KisPrintPreviewWidget() override;

    void setImage(KisImageSP image);
    void setSettings(KisPrintSettings *settings);

    void setZoomLevel(double zoom);
    double zoomLevel() const { return m_zoomLevel; }

    void fitToWindow();

public Q_SLOTS:
    void updatePreview();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void renderPreview();
    QRectF calculateImageRect();

private:
    KisImageSP m_image;
    KisPrintSettings *m_settings;
    QImage m_preview;
    double m_zoomLevel;
    bool m_needsUpdate;
};

#endif // KIS_PRINT_PREVIEW_WIDGET_H
