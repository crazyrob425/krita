/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PRINT_DIALOG_H
#define KIS_PRINT_DIALOG_H

#include <KoDialog.h>
#include <QPrinter>

#include "kritaui_export.h"
#include "kis_types.h"

#include "ui_wdgprintoptions.h"

class KisPrintSettings;
class KisPrintEngine;

class KRITAUI_EXPORT WdgPrintOptions : public QWidget, public Ui::WdgPrintOptions
{
    Q_OBJECT

public:
    WdgPrintOptions(QWidget *parent = nullptr) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * @brief Comprehensive print dialog for Krita
 * 
 * Provides professional-grade printing capabilities with:
 * - Live print preview
 * - Advanced layout and positioning
 * - Color management integration
 * - Print marks and output options
 * - Image adjustments
 */
class KRITAUI_EXPORT KisPrintDialog : public KoDialog
{
    Q_OBJECT

public:
    explicit KisPrintDialog(KisImageSP image, QWidget *parent = nullptr);
    ~KisPrintDialog() override;

    /**
     * Show the dialog and return true if user accepted (clicked Print)
     */
    bool showDialog();

private Q_SLOTS:
    void slotPrint();
    void slotPageSetup();
    void slotUpdatePreview();
    
    // Printer settings slots
    void slotPrinterChanged(int index);
    void slotPageSizeChanged(int index);
    void slotOrientationChanged(int index);
    
    // Layout slots
    void slotPositionChanged(int index);
    void slotScalingChanged(int index);
    void slotScalePercentChanged(double value);
    void slotMarginChanged(double value);
    
    // Color management slots
    void slotRenderingIntentChanged(int index);
    void slotColorSettingsChanged();
    
    // Advanced slots
    void slotPrintMarksChanged();
    void slotImageAdjustmentChanged(int value);
    
    // Preview control slots
    void slotFitToWindow();
    void slotZoom100();

private:
    void setupConnections();
    void loadPrinters();
    void loadColorProfiles();
    void updateSettings();
    void updateUIFromSettings();

private:
    KisImageSP m_image;
    WdgPrintOptions *m_page;
    KisPrintSettings *m_settings;
    KisPrintEngine *m_printEngine;
    QPrinter *m_printer;
};

#endif // KIS_PRINT_DIALOG_H
