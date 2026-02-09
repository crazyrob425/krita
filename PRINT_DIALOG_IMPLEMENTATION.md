# Krita Print Dialog Implementation

## Overview

This implementation adds a comprehensive, Photoshop-style print dialog to Krita with professional-grade printing capabilities.

## Features Implemented

### 1. Print Dialog UI
- **Location**: `libs/ui/dialogs/KisPrintDialog.{h,cpp}`
- **UI Form**: `libs/ui/forms/wdgprintoptions.ui`

The print dialog includes:
- **Print Preview Panel**: Live canvas preview showing the image positioned on the page
- **Zoom Controls**: Fit to window, 100%, custom zoom
- **Real-time Updates**: Preview updates when settings change

### 2. Basic Print Settings
- Printer selection dropdown (integrates with Qt's QPrinter/QPrintDialog)
- Page setup: orientation (portrait/landscape), page size (A4, Letter, Legal, A3, A5)
- Number of copies with collation option
- All settings accessible via clean tabbed interface

### 3. Layout & Positioning
- **Position Controls**: 
  - Center (default)
  - Top-left
  - Custom X/Y coordinates
- **Scaling Options**:
  - Fit to page (maintains aspect ratio)
  - Scale to percentage (1-1000%)
  - Scale to specific dimensions
  - Lock aspect ratio toggle
- **Margins**: Top, bottom, left, right (in mm)

### 4. Color Management
**Location**: `libs/ui/printing/KisPrintSettings.{h,cpp}`

Integrates with Krita's existing color management system:
- Source color profile (from document)
- Printer color profile selection
- Rendering intent dropdown:
  - Perceptual
  - Relative Colorimetric
  - Saturation
  - Absolute Colorimetric
- Black point compensation toggle
- Proof colors option (soft-proof before printing)

### 5. Advanced Print Options

**Print Marks**:
- Crop marks/corner marks
- Registration marks
- Color bars
- Page information (filename, date)

**Image Adjustments** (Print-specific):
- Brightness adjustment slider (-100 to +100)
- Contrast adjustment slider (-100 to +100)
- Saturation adjustment slider (-100 to +100)
- Option to print background color or transparent as white

### 6. Backend Integration

**Print Engine**: `libs/ui/printing/KisPrintEngine.{h,cpp}`
- Handles actual printing using QPrinter
- Applies color management transforms before printing
- Renders the image with selected settings
- Supports high-resolution printing
- Generates print marks if requested

**Print Preview Widget**: `libs/ui/widgets/KisPrintPreviewWidget.{h,cpp}`
- Custom widget for displaying print preview
- Shows page boundaries, margins, and print area
- Updates in real-time when settings change

### 7. Menu Integration

**Modified Files**:
- `krita/krita5.xmlgui`: Uncommented print action in File menu
- `krita/kritamenu.action`: Added file_print action definition
- `libs/ui/KisMainWindow.{h,cpp}`: Added slotPrintFile() handler

**Keyboard Shortcut**: Ctrl+P

### 8. Settings Persistence

**Implementation**: Uses KConfig for settings storage
- Last used printer
- Default scaling option
- Color management preferences
- Print marks preferences
- Saved in `~/.config/kritarc`

## Architecture

### Class Structure

```
KisPrintDialog (KoDialog)
├── WdgPrintOptions (UI Widget)
│   ├── KisPrintPreviewWidget (Custom Widget)
│   ├── Basic Settings Tab
│   ├── Color Management Tab
│   └── Advanced Tab
├── KisPrintSettings (Data Model)
└── KisPrintEngine (Print Backend)
```

### Data Flow

1. User opens File > Print (Ctrl+P)
2. `KisMainWindow::slotPrintFile()` creates `KisPrintDialog`
3. Dialog loads saved settings from `KisPrintSettings`
4. User adjusts settings, preview updates via `KisPrintPreviewWidget`
5. On Print:
   - Settings are updated from UI
   - `KisPrintEngine::print()` is called
   - Color conversion is applied
   - Print marks are generated
   - Output is sent to QPrinter

## Files Created

### Header Files
- `libs/ui/dialogs/KisPrintDialog.h`
- `libs/ui/printing/KisPrintSettings.h`
- `libs/ui/printing/KisPrintEngine.h`
- `libs/ui/widgets/KisPrintPreviewWidget.h`

### Implementation Files
- `libs/ui/dialogs/KisPrintDialog.cpp`
- `libs/ui/printing/KisPrintSettings.cpp`
- `libs/ui/printing/KisPrintEngine.cpp`
- `libs/ui/widgets/KisPrintPreviewWidget.cpp`

### UI Forms
- `libs/ui/forms/wdgprintoptions.ui`

### Modified Files
- `libs/ui/CMakeLists.txt`: Added new files to build
- `libs/ui/KisMainWindow.h`: Added print slot declaration
- `libs/ui/KisMainWindow.cpp`: Added print action and implementation
- `krita/krita5.xmlgui`: Enabled print menu item
- `krita/kritamenu.action`: Added print action definition

## Dependencies

### Qt Modules
- Qt::PrintSupport (for QPrinter, QPrintDialog)
- Qt::Widgets (for UI components)

### Krita Modules
- `kritaimage` (for KisImage)
- `kritacolor` (for color management)
- `kritawidgets` (for UI components)
- `kritapigment` (for KoColorSpace, KoColorProfile)

### KDE Frameworks
- KF::I18n (for internationalization)
- KF::ConfigCore (for settings persistence)

## Usage

### For Users

1. Open an image in Krita
2. Go to File > Print (or press Ctrl+P)
3. Configure print settings:
   - Select printer
   - Choose page size and orientation
   - Adjust layout and margins
   - Configure color management
   - Add print marks if needed
4. Preview updates automatically
5. Click Print to send to printer

### For Developers

To extend the print functionality:

1. **Add new settings**: Modify `KisPrintSettings` class
2. **Update UI**: Edit `wdgprintoptions.ui` in Qt Designer
3. **Modify rendering**: Update `KisPrintEngine::renderPage()`
4. **Add preview features**: Enhance `KisPrintPreviewWidget::paintEvent()`

## Future Enhancements

Potential improvements for future versions:

1. **Multi-page printing**: Support for printing large images across multiple pages
2. **Print templates**: Save and load print configurations
3. **Advanced color management**: 
   - Gamut warning display in preview
   - More sophisticated color space conversion
4. **Bleed area settings**: For professional printing
5. **Custom print marks**: Allow users to define custom marks
6. **Print queue management**: Better integration with system print queue
7. **PDF export options**: Direct PDF export with all settings

## Testing Recommendations

When testing this feature:

1. Test with different image sizes (small, large, very large)
2. Test with different color spaces (RGB, CMYK, Grayscale)
3. Test with different printers (PDF printer, physical printers)
4. Verify color management accuracy
5. Test print preview updates
6. Verify settings persistence across sessions
7. Test print marks generation
8. Test on different platforms (Linux, Windows, macOS)

## Known Limitations

1. Color space conversion to printer profile uses simplified implementation
   - Full LCMS2 integration planned for future versions
2. Image adjustments (brightness/contrast/saturation) are placeholders
   - Need implementation of actual adjustment filters
3. Multi-page printing for large images not yet implemented
4. Gamut warning in preview not yet implemented

## Integration Points

This feature integrates with:
- Krita's action management system
- Krita's color management system (via KoColorSpace)
- Qt's printing framework (QPrinter)
- KDE's configuration system (KConfig)
- Krita's UI framework (KoDialog)

## License

All new files are licensed under GPL-2.0-or-later, consistent with Krita's licensing.

## Authors

Implementation by Krita Contributors, 2025
