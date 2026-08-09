// src/app/molecule/RenderPrimitivesToVariant.h
#ifndef RENDERPRIMITIVESTOVARIANT_H
#define RENDERPRIMITIVESTOVARIANT_H

// Converts RenderPrimitives/SelectionState (sub-projects 2 and 4) into the exact QVariantMap
// shape V8Process currently receives from the JS engine's buildRenderPrimitives/currentSelection
// (10-state.js:386-388,1006-1394) -- see
// docs/superpowers/specs/2026-08-04-render-primitives-qvariant-bridge-design.md for the full
// field-by-field mapping and the two confirmed real-JSON-fidelity cases (bond ring-center key
// omission, rxnArrow curvature explicit null) this must replicate exactly, since QML's existing
// bindings are written against this shape and must not need to change. Pure, standalone --
// no dependency on V8Process/DocumentState/QjsEngine, not wired into the app by this sub-project.

#include <QVariantMap>
#include "RenderPrimitives.h"
#include "SelectionState.h"

QVariantMap renderPrimitivesToVariant(const RenderPrimitives& prims);
QVariantMap selectionStateToVariant(const SelectionState& sel);

#endif // RENDERPRIMITIVESTOVARIANT_H
