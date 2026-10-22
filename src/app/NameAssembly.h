#pragma once

/*
===============================================================================
Candidate inspection record for Task F (pure NameAssembly formatting extraction)
===============================================================================

A sample of candidate formatting blocks in IupacNamer.cpp was inspected
for purity (zero indigoXxx() calls, zero graph traversals, operating
purely on already-computed values like QString, QStringList, or locant
lists). This is not an exhaustive sweep of the whole file -- treat it
as a seed list for a future pass, not a closed inventory.

--- MOVED TO NameAssembly ---
1. chainRoot(int len):
   Pure array lookup mapping chain length (1..20) to IUPAC root ("meth", "eth", ...).
2. multiPrefix(int count):
   Pure array lookup mapping multiplier counts (2..10) to IUPAC prefix ("di", "tri", ...).
3. isVowel(QChar c):
   Pure character check for vowel elision logic ('a', 'e', 'i', 'o', 'u').

--- IDENTIFIED AS LIKELY PURE, NOT YET MOVED (candidates for a future pass) ---
Do not trust this list as "already verified pure" the way the 3 moved
functions above are -- re-confirm each on its own before moving it.
Items 1-3 are real, already-named free functions in IupacNamer.cpp
today (confirmed by grep, exact names below). Items 4-6 are NOT real
function names -- an earlier draft of this comment invented signatures
for inline code/lambdas that don't exist as named functions; corrected
here to point at the real current code instead.
1. alphabetizationKey(const QString &pName) -- IupacNamer.cpp:~503:
   Candidate: string transformation stripping bracket wrapping, stereo
   parentheticals, and leading locants down to the base alphabetization
   root. Used across substituent prefix sorting and tiebreaking.
2. toFusionPrefix(const QString &name) & toFusionPrefixBracketed(const QString &name)
   -- IupacNamer.cpp:~917/~934:
   Candidate: string transformations converting terminal 'e' to 'o' and
   bracket-wrapping leading heteroatom locants for attached fusion components.
3. wrapCompoundSuffix(QString name, const QString &suffix) &
   wrapBorinicSubstituent(QString otherName) -- IupacNamer.cpp:~1816/~1826:
   Candidate: string formatting handling paren/bracket nesting for compound
   suffixes ("-amido", "-carbonyloxy", etc.) and borinic acid prefixes.
4. NOT a named function -- a local lambda `formatLocants` inside fusion
   naming, IupacNamer.cpp:~6591: `[](std::pair<int,int> locs, int level) -> QString`,
   adding prime marks to fusion edge locants by depth level. Would need
   promoting to a free function (with a real name) before it could move.
5. NOT a named function -- inline P-31 saturation/indicated-H assembly,
   IupacNamer.cpp:~7223 (`QString satPart = hydroPrefix; ... satPart += ihPrefix;`).
   Note: a DIFFERENT, unrelated function named `buildHydroPrefix(const
   QStringList&, bool&)` already exists in `RetainedRingMatcher.cpp:692`
   -- do not confuse the two or grep for this name expecting to land on
   the code described here.
6. NOT a named function -- inline branching-fusion bracket combining,
   IupacNamer.cpp:~7990-7998 (`combinedLocant = "[" + contents.join(",") + "]"`
   for level 1, `join(":")` for level >= 2).

--- REJECTED AS IMPURE / TIED TO DOMAIN LOGIC (LEAVE IN PLACE) ---
1. halogenPrefix(int z) / halogenSuffixWord(int z):
   Actually pure (z-to-string lookup tables, same shape as chainRoot/
   multiPrefix above) -- left in place as a judgment call, not a true
   purity rejection: they're element-property tables, not name-assembly
   formatting, so moving them to NameAssembly would muddy this file's
   scope even though it would compile fine.
2. getFusionPrefixShared(RingType t) / getBaseNameShared(RingType t):
   REJECTED: Couples directly to internal RingType enum and ring perception logic
   rather than general string formatting.
3. hwSeniorityRank(int z):
   REJECTED: Chemical priority/seniority scoring rule (P-22.2.2.1.3 citation order)
   on atomic numbers, not string assembly.
4. formatBranchStereoPrefix(...):
   REJECTED (confirmed): takes `const Graph &g`, walks `g.nodes[].neighbors`
   with a stack-based longest-chain search and ring exclusion. Interleaved
   with chemical structure analysis.
5. tryNameAcyloxySubstituent / tryNameAmidoSubstituent / tryNameNitrosamine:
   REJECTED: Functional group perception containing Indigo calls and graph walks.
6. nameBranchGraph / nameAcyclicChainParentWithSubstituents / simpleRingSubstituentName:
   REJECTED: Core recursive naming logic with deep Graph and Indigo dependencies.
7. getFaceLocants(...) -- IupacNamer.cpp:~6597:
   Actually pure (a local lambda doing index lookups on a numbering
   `std::vector<int>`, no cycle walk) -- left in place as a judgment
   call: it's locant computation feeding into naming decisions, not
   string formatting, and it's a local lambda tied to its enclosing
   function's scope, not a natural standalone utility.
8. NameScore (confirmed): a function-local struct whose `operator<` is a
   multi-level tie-break over locant vectors and maps -- fusion isomer
   numbering tie-break scoring, not string assembly.
===============================================================================
*/

#include <QString>

// Chain length root prefixes C1..C20
QString chainRoot(int len);

// Multiplying prefixes (2..10)
QString multiPrefix(int count);

// Helper: check if character is a vowel
bool isVowel(QChar c);
