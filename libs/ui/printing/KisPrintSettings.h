/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PRINT_SETTINGS_H
#define KIS_PRINT_SETTINGS_H

#include <QObject>
#include <QString>
#include <QSizeF>
#include <QPointF>
#include <QPrinter>

#include <KoColorConversionTransformation.h>

#include "kritaui_export.h"

class KoColorProfile;

/**
 * @brief Settings structure for print operations
 * 
 * Stores all user-configurable print settings including layout, 
 * color management, and advanced options.
 */
class KRITAUI_EXPORT KisPrintSettings : public QObject
{
    Q_OBJECT

public:
    enum PositionMode {
        Center,
        TopLeft,
        Custom
    };

    enum ScalingMode {
        FitToPage,
        Percentage,
        SpecificDimensions
    };

    explicit KisPrintSettings(QObject *parent = nullptr);
    ~KisPrintSettings() override;

    // Printer settings
    void setPrinterName(const QString &name) { m_printerName = name; }
    QString printerName() const { return m_printerName; }

    // Page setup
    void setOrientation(QPrinter::Orientation orientation) { m_orientation = orientation; }
    QPrinter::Orientation orientation() const { return m_orientation; }

    void setPageSize(QPrinter::PageSize pageSize) { m_pageSize = pageSize; }
    QPrinter::PageSize pageSize() const { return m_pageSize; }

    // Copies
    void setCopies(int copies) { m_copies = copies; }
    int copies() const { return m_copies; }

    void setCollate(bool collate) { m_collate = collate; }
    bool collate() const { return m_collate; }

    // Layout and positioning
    void setPositionMode(PositionMode mode) { m_positionMode = mode; }
    PositionMode positionMode() const { return m_positionMode; }

    void setCustomPosition(const QPointF &pos) { m_customPosition = pos; }
    QPointF customPosition() const { return m_customPosition; }

    // Scaling
    void setScalingMode(ScalingMode mode) { m_scalingMode = mode; }
    ScalingMode scalingMode() const { return m_scalingMode; }

    void setScalePercentage(double percent) { m_scalePercentage = percent; }
    double scalePercentage() const { return m_scalePercentage; }

    void setSpecificDimensions(const QSizeF &dims) { m_specificDimensions = dims; }
    QSizeF specificDimensions() const { return m_specificDimensions; }

    void setLockAspectRatio(bool lock) { m_lockAspectRatio = lock; }
    bool lockAspectRatio() const { return m_lockAspectRatio; }

    // Margins (in mm)
    void setMargins(double top, double bottom, double left, double right);
    void setTopMargin(double margin) { m_marginTop = margin; }
    void setBottomMargin(double margin) { m_marginBottom = margin; }
    void setLeftMargin(double margin) { m_marginLeft = margin; }
    void setRightMargin(double margin) { m_marginRight = margin; }
    
    double topMargin() const { return m_marginTop; }
    double bottomMargin() const { return m_marginBottom; }
    double leftMargin() const { return m_marginLeft; }
    double rightMargin() const { return m_marginRight; }

    // Color management
    void setPrinterProfile(const KoColorProfile *profile) { m_printerProfile = profile; }
    const KoColorProfile *printerProfile() const { return m_printerProfile; }

    void setRenderingIntent(KoColorConversionTransformation::Intent intent) { m_renderingIntent = intent; }
    KoColorConversionTransformation::Intent renderingIntent() const { return m_renderingIntent; }

    void setBlackPointCompensation(bool enable) { m_blackPointCompensation = enable; }
    bool blackPointCompensation() const { return m_blackPointCompensation; }

    void setProofColors(bool enable) { m_proofColors = enable; }
    bool proofColors() const { return m_proofColors; }

    // Print marks and output
    void setCropMarks(bool enable) { m_cropMarks = enable; }
    bool cropMarks() const { return m_cropMarks; }

    void setRegistrationMarks(bool enable) { m_registrationMarks = enable; }
    bool registrationMarks() const { return m_registrationMarks; }

    void setColorBars(bool enable) { m_colorBars = enable; }
    bool colorBars() const { return m_colorBars; }

    void setPageInfo(bool enable) { m_pageInfo = enable; }
    bool pageInfo() const { return m_pageInfo; }

    // Image adjustments (print-specific)
    void setBrightness(int value) { m_brightness = value; }
    int brightness() const { return m_brightness; }

    void setContrast(int value) { m_contrast = value; }
    int contrast() const { return m_contrast; }

    void setSaturation(int value) { m_saturation = value; }
    int saturation() const { return m_saturation; }

    void setPrintBackgroundColor(bool enable) { m_printBackgroundColor = enable; }
    bool printBackgroundColor() const { return m_printBackgroundColor; }

    // Load/save settings from KConfig
    void loadSettings();
    void saveSettings();

Q_SIGNALS:
    void settingsChanged();

private:
    // Printer settings
    QString m_printerName;
    QPrinter::Orientation m_orientation;
    QPrinter::PageSize m_pageSize;
    int m_copies;
    bool m_collate;

    // Layout
    PositionMode m_positionMode;
    QPointF m_customPosition;

    // Scaling
    ScalingMode m_scalingMode;
    double m_scalePercentage;
    QSizeF m_specificDimensions;
    bool m_lockAspectRatio;

    // Margins
    double m_marginTop;
    double m_marginBottom;
    double m_marginLeft;
    double m_marginRight;

    // Color management
    const KoColorProfile *m_printerProfile;
    KoColorConversionTransformation::Intent m_renderingIntent;
    bool m_blackPointCompensation;
    bool m_proofColors;

    // Print marks
    bool m_cropMarks;
    bool m_registrationMarks;
    bool m_colorBars;
    bool m_pageInfo;

    // Image adjustments
    int m_brightness;
    int m_contrast;
    int m_saturation;
    bool m_printBackgroundColor;
};

#endif // KIS_PRINT_SETTINGS_H
