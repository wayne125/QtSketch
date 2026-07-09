# Third-Party Notices

This application incorporates the following third-party components.

## Ketcher (EPAM Systems)

- **What is used:** the bundled cheminformatics library `chem-core.js` is derived from
  Ketcher's core packages, and the SVG icon set in `icons/` is taken from the Ketcher
  user interface.
- **Source:** https://github.com/epam/ketcher
- **License:** Apache License, Version 2.0
- **Copyright:** © EPAM Systems, Inc.
- **Modifications:** `chem-core.js` is a bundled/repackaged build of Ketcher core code
  adapted to run inside a Node.js worker process. Icon SVGs are modified from the
  originals: `currentColor` fills/strokes were replaced with a fixed color (#202020),
  since Qt's SVG renderer does not resolve CSS `currentColor`.

## Indigo Toolkit (EPAM Systems)

- **What is used:** the Indigo C API (`indigo.dll` and headers under `indigo/`) for
  layout, aromatization, structure checking, CIP stereodescriptors, and biopolymer
  sequence handling.
- **Source:** https://github.com/epam/indigo
- **License:** Apache License, Version 2.0 (full text: `indigo/LICENSE`)
- **Copyright:** © EPAM Systems, Inc.

## Apache License 2.0

Both components above are licensed under the Apache License, Version 2.0.
A full copy of the license text is included in this repository at `indigo/LICENSE`,
and is also available at: http://www.apache.org/licenses/LICENSE-2.0

Neither the Ketcher nor Indigo names, nor the EPAM Systems name, are used to endorse
or promote this application.
