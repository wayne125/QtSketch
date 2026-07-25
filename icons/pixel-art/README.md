# 8-bit Pixel Art Icons for QtSketch

This directory contains 8-bit pixel art style icons for QtSketch. All icons are 16x16 SVG files with a retro pixel art aesthetic.

## Style Characteristics

- **Size**: 16x16 pixels
- **Color Palette**: Limited to black (#000000) and white (#FFFFFF) for authentic 8-bit look
- **Design**: Blocky, pixel-perfect shapes with no anti-aliasing
- **Format**: SVG (scalable while maintaining pixel art look)

## Icons Included

### File Operations
- `new-file.svg` - New file
- `open.svg` - Open file
- `save.svg` - Save file
- `file-thumbnail.svg` - File thumbnail

### Edit Operations
- `undo.svg` - Undo
- `redo.svg` - Redo
- `copy.svg` - Copy
- `cut.svg` - Cut
- `paste.svg` - Paste
- `clear.svg` - Clear
- `clean.svg` - Clean

### Drawing Tools
- `select.svg` - Select tool
- `select-lasso.svg` - Lasso select
- `select-fragment.svg` - Fragment select
- `erase.svg` - Erase tool
- `hand.svg` - Hand tool

### Bond Tools
- `single_bond.svg` - Single bond
- `double_bond.svg` - Double bond
- `triple_bond.svg` - Triple bond
- `up_bond.svg` - Up bond
- `down_bond.svg` - Down bond
- `updown_bond.svg` - Up/Down bond
- `bond-any.svg` - Any bond
- `bond-aromatic.svg` - Aromatic bond
- `bond-crossed.svg` - Crossed bond
- `bond-dative.svg` - Dative bond
- `bond-doublearomatic.svg` - Double aromatic bond

### Shapes and Structures
- `benzene.svg` - Benzene ring
- `chain.svg` - Chain
- `atoms.svg` - Atoms
- `atoms-white.svg` - Atoms (white)
- `any-atom.svg` - Any atom
- `arrange-ring.svg` - Arrange ring
- `biopolymer.svg` - Biopolymer
- `3d.svg` - 3D mode
- `3d-white.svg` - 3D mode (white)

### Templates
- `template-0.svg` - Template 0
- `template-2.svg` - Template 2
- `template-3.svg` - Template 3
- `template-4.svg` - Template 4
- `template-5.svg` - Template 5
- `template-6.svg` - Template 6
- `template-7.svg` - Template 7
- `template-lib.svg` - Template library

### Reactions
- `reaction-arrow-open-angle.svg` - Open angle arrow
- `reaction-arrow-filled-triangle.svg` - Filled triangle arrow
- `reaction-arrow-equilibrium-filled-triangle.svg` - Equilibrium filled triangle
- `reaction-arrow-equilibrium-filled-half-bow.svg` - Equilibrium half bow
- `reaction-arrow-elliptical-arc-arrow-filled-triangle.svg` - Elliptical arc arrow
- `reaction-plus.svg` - Plus
- `reaction-map.svg` - Reaction map
- `reaction-arrow-multitail.svg` - Multitail arrow
- `reaction-arrow-retrosynthetic-arrow.svg` - Retrosynthetic arrow

### Text and Labels
- `text.svg` - Text tool
- `text-bold.svg` - Bold text
- `text-superscript.svg` - Superscript
- `rgroup-label.svg` - R-group label
- `charge-plus.svg` - Positive charge
- `charge-minus.svg` - Negative charge

### View and Navigation
- `zoom-in.svg` - Zoom in
- `zoom-out.svg` - Zoom out
- `fit.svg` - Fit to screen
- `arrows-left.svg` - Arrows left
- `arrows-right.svg` - Arrows right
- `arrows-up-down.svg` - Arrows up/down
- `arrow-upward.svg` - Arrow upward

### Chemistry
- `period-table.svg` - Periodic table
- `generic-groups.svg` - Generic groups
- `explicit-hydrogens.svg` - Explicit hydrogens
- `angstrom.svg` - Angstrom
- `antisense-strand.svg` - Antisense strand
- `approximately-equal.svg` - Approximately equal
- `analyse.svg` - Analyze
- `arom.svg` - Aromatize
- `dearom.svg` - Dearomatize
- `base.svg` - Base
- `promille.svg` - Promille

### Other
- `settings.svg` - Settings
- `about.svg` - About
- `search.svg` - Search
- `check.svg` - Check
- `add-image.svg` - Add image
- `copy_image.svg` - Copy image
- `smiles_in.svg` - SMILES input
- `server-white.svg` - Server (white)
- `right-arrow-crossed-out.svg` - Right arrow crossed out
- `rap-left-link.svg` - Rap left link
- `fullscreen-exit.svg` - Fullscreen exit
- `shape-rectangle.svg` - Rectangle shape
- `double_bond.svg` - Double bond (alternate)
- `preset.svg` - Preset
- `extended-table.svg` - Extended table

## Usage

To use these icons in QtSketch:

1. Update the `CMakeLists.txt` file to reference the pixel-art versions instead of the original icons
2. Or create a theme system that allows switching between icon sets

## Color Customization

To change the color scheme while maintaining the pixel art look:
- Replace `#000000` with your preferred dark color
- Replace `#FFFFFF` with your preferred light color
- Keep the blocky, pixel-perfect shapes

## Contributing

When adding new pixel art icons:
1. Use 16x16 viewBox
2. Stick to the limited color palette
3. Use integer coordinates for crisp pixels
4. Avoid anti-aliasing (use whole pixel coordinates)
