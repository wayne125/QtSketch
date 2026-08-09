// src/app/molecule/EditCommand.h
#ifndef EDITCOMMAND_H
#define EDITCOMMAND_H

// Direct C++ match to src/worker/10-state.js's makeCmd(executeFn, invertFn) /
// Command object (sub-project 2 of the chem-core.js migration; see
// docs/superpowers/specs/2026-08-01-document-state-cpp-design.md). Closures
// capture whatever they need (typically an EditableMolecule& plus ids/values)
// at construction time, exactly like the JS closures capture _struct.

#include <functional>

struct EditCommand {
    std::function<void()> execute;
    std::function<void()> invert;
};

#endif // EDITCOMMAND_H
