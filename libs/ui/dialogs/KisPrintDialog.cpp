/*
 *  SPDX-FileCopyrightText: 2025 Krita Contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPrintDialog.h"

#include <QPrinter>
#include <QPrinterInfo>
#include <QPageSetupDialog>
#include <QPushButton>

#include <klocalizedstring.h>
#include <kis_debug.h>

#include "kis_image.h"
#include "printing/KisPrintSettings.h"
#include "printing/KisPrintEngine.h"
#include "widgets/KisPrintPreviewWidget.h"

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

KisPrintDialog::KisPrintDialog(KisImageSP image, QWidget *parent)
    : KoDialog(parent)
    , m_image(image)
    , m_settings(new KisPrintSettings(this))
    , m_printEngine(new KisPrintEngine(this))
    , m_printer(new QPrinter(QPrinter::HighResolution))
{
    setCaption(i18n("Print"));
    setButtons(User1 | Ok | Cancel);
    setButtonText(User1, i18n("Page Setup..."));
    setButtonText(Ok, i18n("Print"));
    setDefaultButton(Ok);

    m_page = new WdgPrintOptions(this);
    setMainWidget(m_page);

    // Load saved settings
    m_settings->loadSettings();

    // Initialize UI
    loadPrinters();
    loadColorProfiles();
    updateUIFromSettings();

    // Set up the preview widget
    m_page->previewWidget->setImage(m_image);
    m_page->previewWidget->setSettings(m_settings);

    // Setup connections
    setupConnections();

    // Initial preview update
    slotUpdatePreview();

    resize(1000, 700);
}

KisPrintDialog::~KisPrintDialog()
{
    delete m_printer;
}

bool KisPrintDialog::showDialog()
{
    return exec() == QDialog::Accepted;
}

void KisPrintDialog::setupConnections()
{
    // Dialog buttons
    connect(this, &KoDialog::okClicked, this, &KisPrintDialog::slotPrint);
    connect(this, &KoDialog::user1Clicked, this, &KisPrintDialog::slotPageSetup);

    // Printer settings
    connect(m_page->cmbPrinter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotPrinterChanged);
    connect(m_page->cmbPageSize, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotPageSizeChanged);
    connect(m_page->cmbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotOrientationChanged);
    connect(m_page->spinCopies, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KisPrintDialog::slotUpdatePreview);

    // Layout and positioning
    connect(m_page->cmbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotPositionChanged);
    connect(m_page->cmbScaling, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotScalingChanged);
    connect(m_page->spinScalePercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KisPrintDialog::slotScalePercentChanged);

    // Margins
    connect(m_page->spinMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KisPrintDialog::slotMarginChanged);
    connect(m_page->spinMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KisPrintDialog::slotMarginChanged);
    connect(m_page->spinMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KisPrintDialog::slotMarginChanged);
    connect(m_page->spinMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KisPrintDialog::slotMarginChanged);

    // Color management
    connect(m_page->cmbRenderingIntent, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KisPrintDialog::slotRenderingIntentChanged);
    connect(m_page->chkBlackPointCompensation, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotColorSettingsChanged);
    connect(m_page->chkProofColors, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotColorSettingsChanged);

    // Print marks
    connect(m_page->chkCropMarks, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotPrintMarksChanged);
    connect(m_page->chkRegistrationMarks, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotPrintMarksChanged);
    connect(m_page->chkColorBars, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotPrintMarksChanged);
    connect(m_page->chkPageInfo, &QCheckBox::stateChanged,
            this, &KisPrintDialog::slotPrintMarksChanged);

    // Image adjustments
    connect(m_page->sliderBrightness, &QSlider::valueChanged,
            this, &KisPrintDialog::slotImageAdjustmentChanged);
    connect(m_page->sliderContrast, &QSlider::valueChanged,
            this, &KisPrintDialog::slotImageAdjustmentChanged);
    connect(m_page->sliderSaturation, &QSlider::valueChanged,
            this, &KisPrintDialog::slotImageAdjustmentChanged);

    // Preview controls
    connect(m_page->btnFitToWindow, &QPushButton::clicked,
            this, &KisPrintDialog::slotFitToWindow);
    connect(m_page->btnZoom100, &QPushButton::clicked,
            this, &KisPrintDialog::slotZoom100);
}

void KisPrintDialog::loadPrinters()
{
    m_page->cmbPrinter->clear();

    QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
    for (const QPrinterInfo &info : printers) {
        m_page->cmbPrinter->addItem(info.printerName());
    }

    // Select default printer
    QPrinterInfo defaultPrinter = QPrinterInfo::defaultPrinter();
    if (!defaultPrinter.isNull()) {
        int index = m_page->cmbPrinter->findText(defaultPrinter.printerName());
        if (index >= 0) {
            m_page->cmbPrinter->setCurrentIndex(index);
        }
    }

    // Populate page sizes
    m_page->cmbPageSize->addItem(i18n("A4"), QPrinter::A4);
    m_page->cmbPageSize->addItem(i18n("Letter"), QPrinter::Letter);
    m_page->cmbPageSize->addItem(i18n("Legal"), QPrinter::Legal);
    m_page->cmbPageSize->addItem(i18n("A3"), QPrinter::A3);
    m_page->cmbPageSize->addItem(i18n("A5"), QPrinter::A5);
}

void KisPrintDialog::loadColorProfiles()
{
    m_page->cmbPrinterProfile->clear();

    // Add default sRGB
    m_page->cmbPrinterProfile->addItem(i18n("sRGB (default)"));

    // Get source color space info
    if (m_image) {
        const KoColorSpace *cs = m_image->colorSpace();
        if (cs) {
            QString profileName = cs->profile()->name();
            m_page->lblSourceProfileValue->setText(profileName);
        }
    }
}

void KisPrintDialog::updateSettings()
{
    // Update settings from UI
    m_settings->setCopies(m_page->spinCopies->value());
    m_settings->setCollate(m_page->chkCollate->isChecked());

    // Position
    m_settings->setPositionMode(
        static_cast<KisPrintSettings::PositionMode>(m_page->cmbPosition->currentIndex()));

    // Scaling
    m_settings->setScalingMode(
        static_cast<KisPrintSettings::ScalingMode>(m_page->cmbScaling->currentIndex()));
    m_settings->setScalePercentage(m_page->spinScalePercent->value());
    m_settings->setLockAspectRatio(m_page->chkLockAspectRatio->isChecked());

    // Margins
    m_settings->setMargins(
        m_page->spinMarginTop->value(),
        m_page->spinMarginBottom->value(),
        m_page->spinMarginLeft->value(),
        m_page->spinMarginRight->value()
    );

    // Orientation
    m_settings->setOrientation(
        m_page->cmbOrientation->currentIndex() == 0 ? 
        QPrinter::Portrait : QPrinter::Landscape);

    // Page size
    int pageSizeIndex = m_page->cmbPageSize->currentData().toInt();
    m_settings->setPageSize(static_cast<QPrinter::PageSize>(pageSizeIndex));

    // Color management
    m_settings->setRenderingIntent(
        static_cast<KoColorConversionTransformation::Intent>(
            m_page->cmbRenderingIntent->currentIndex()));
    m_settings->setBlackPointCompensation(m_page->chkBlackPointCompensation->isChecked());
    m_settings->setProofColors(m_page->chkProofColors->isChecked());

    // Print marks
    m_settings->setCropMarks(m_page->chkCropMarks->isChecked());
    m_settings->setRegistrationMarks(m_page->chkRegistrationMarks->isChecked());
    m_settings->setColorBars(m_page->chkColorBars->isChecked());
    m_settings->setPageInfo(m_page->chkPageInfo->isChecked());

    // Image adjustments
    m_settings->setBrightness(m_page->sliderBrightness->value());
    m_settings->setContrast(m_page->sliderContrast->value());
    m_settings->setSaturation(m_page->sliderSaturation->value());
    m_settings->setPrintBackgroundColor(m_page->chkPrintBackgroundColor->isChecked());

    // Notify settings changed
    Q_EMIT m_settings->settingsChanged();
}

void KisPrintDialog::updateUIFromSettings()
{
    m_page->spinCopies->setValue(m_settings->copies());
    m_page->chkCollate->setChecked(m_settings->collate());

    m_page->cmbPosition->setCurrentIndex(static_cast<int>(m_settings->positionMode()));
    m_page->cmbScaling->setCurrentIndex(static_cast<int>(m_settings->scalingMode()));
    m_page->spinScalePercent->setValue(m_settings->scalePercentage());
    m_page->chkLockAspectRatio->setChecked(m_settings->lockAspectRatio());

    m_page->spinMarginTop->setValue(m_settings->topMargin());
    m_page->spinMarginBottom->setValue(m_settings->bottomMargin());
    m_page->spinMarginLeft->setValue(m_settings->leftMargin());
    m_page->spinMarginRight->setValue(m_settings->rightMargin());

    m_page->cmbOrientation->setCurrentIndex(
        m_settings->orientation() == QPrinter::Portrait ? 0 : 1);

    m_page->cmbRenderingIntent->setCurrentIndex(static_cast<int>(m_settings->renderingIntent()));
    m_page->chkBlackPointCompensation->setChecked(m_settings->blackPointCompensation());
    m_page->chkProofColors->setChecked(m_settings->proofColors());

    m_page->chkCropMarks->setChecked(m_settings->cropMarks());
    m_page->chkRegistrationMarks->setChecked(m_settings->registrationMarks());
    m_page->chkColorBars->setChecked(m_settings->colorBars());
    m_page->chkPageInfo->setChecked(m_settings->pageInfo());

    m_page->sliderBrightness->setValue(m_settings->brightness());
    m_page->sliderContrast->setValue(m_settings->contrast());
    m_page->sliderSaturation->setValue(m_settings->saturation());
    m_page->chkPrintBackgroundColor->setChecked(m_settings->printBackgroundColor());
}

void KisPrintDialog::slotPrint()
{
    if (!m_image) {
        return;
    }

    updateSettings();

    // Configure printer
    QString printerName = m_page->cmbPrinter->currentText();
    m_printer->setPrinterName(printerName);
    m_printer->setOrientation(m_settings->orientation());
    m_printer->setPageSize(m_settings->pageSize());
    m_printer->setCopyCount(m_settings->copies());
    m_printer->setCollateCopies(m_settings->collate());

    // Perform the print
    bool success = m_printEngine->print(m_image, m_settings, m_printer);

    if (success) {
        // Save settings for next time
        m_settings->saveSettings();
        accept();
    } else {
        // Error message already shown by print engine
        dbgUI << "Print failed";
    }
}

void KisPrintDialog::slotPageSetup()
{
    QPageSetupDialog pageSetup(m_printer, this);
    if (pageSetup.exec() == QDialog::Accepted) {
        // Update UI to reflect changes
        slotUpdatePreview();
    }
}

void KisPrintDialog::slotUpdatePreview()
{
    updateSettings();
    m_page->previewWidget->updatePreview();
}

void KisPrintDialog::slotPrinterChanged(int index)
{
    Q_UNUSED(index);
    slotUpdatePreview();
}

void KisPrintDialog::slotPageSizeChanged(int index)
{
    Q_UNUSED(index);
    slotUpdatePreview();
}

void KisPrintDialog::slotOrientationChanged(int index)
{
    Q_UNUSED(index);
    slotUpdatePreview();
}

void KisPrintDialog::slotPositionChanged(int index)
{
    Q_UNUSED(index);
    slotUpdatePreview();
}

void KisPrintDialog::slotScalingChanged(int index)
{
    Q_UNUSED(index);
    // Enable/disable scale percentage based on mode
    bool enablePercent = (index == KisPrintSettings::Percentage);
    m_page->spinScalePercent->setEnabled(enablePercent);
    m_page->lblScalePercent->setEnabled(enablePercent);
    slotUpdatePreview();
}

void KisPrintDialog::slotScalePercentChanged(double value)
{
    Q_UNUSED(value);
    slotUpdatePreview();
}

void KisPrintDialog::slotMarginChanged(double value)
{
    Q_UNUSED(value);
    slotUpdatePreview();
}

void KisPrintDialog::slotRenderingIntentChanged(int index)
{
    Q_UNUSED(index);
    slotUpdatePreview();
}

void KisPrintDialog::slotColorSettingsChanged()
{
    slotUpdatePreview();
}

void KisPrintDialog::slotPrintMarksChanged()
{
    slotUpdatePreview();
}

void KisPrintDialog::slotImageAdjustmentChanged(int value)
{
    Q_UNUSED(value);
    slotUpdatePreview();
}

void KisPrintDialog::slotFitToWindow()
{
    m_page->previewWidget->fitToWindow();
}

void KisPrintDialog::slotZoom100()
{
    m_page->previewWidget->setZoomLevel(1.0);
}
