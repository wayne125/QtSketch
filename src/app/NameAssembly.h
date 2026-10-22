#pragma once

/*
===============================================================================
Candidate inspection record for Task F (pure NameAssembly formatting extraction)
===============================================================================

Every candidate formatting block in IupacNamer.cpp was inspected for purity
(zero indigoXxx() calls, zero graph traversals, operating purely on
already-computed values like QString, QStringList, or locant lists).

--- MOVED TO NameAssembly ---
1. chainRoot(int len):
   Pure array lookup mapping chain length (1..20) to IUPAC root ("meth", "eth", ...).
2. multiPrefix(int count):
   Pure array lookup mapping multiplier counts (2..10) to IUPAC prefix ("di", "tri", ...).
3. isVowel(QChar c):
   Pure character check for vowel elision logic ('a', 'e', 'i', 'o', 'u').

--- IDENTIFIED AS LIKELY PURE, NOT YET MOVED (candidates for a future pass) ---
The delegate dispatch that produced this file's candidate survey was
interrupted (environment/quota issue) before finishing the actual move
for these -- they remain in IupacNamer.cpp today, unmoved. Listed here
so a future session doesn't redo the reading from scratch, but do not
trust this list as "already verified pure" the way the 3 moved
functions above are -- re-confirm each on its own before moving it.
1. alphabetizationKey(const QString &pName):
   Candidate: string transformation stripping bracket wrapping, stereo
   parentheticals, and leading locants down to the base alphabetization
   root. Used across substituent prefix sorting and tiebreaking.
2. toFusionPrefix(const QString &name) & toFusionPrefixBracketed(const QString &name):
   Candidate: string transformations converting terminal 'e' to 'o' and
   bracket-wrapping leading heteroatom locants for attached fusion components.
3. wrapCompoundSuffix(QString name, const QString &suffix) &
   wrapBorinicSubstituent(QString otherName):
   Candidate: string formatting handling paren/bracket nesting for compound
   suffixes ("-amido", "-carbonyloxy", etc.) and borinic acid prefixes.
4. formatFusionLocants(std::pair<int, int> locs, int level):
   Candidate: string formatting adding prime marks to fusion edge locants by depth level.
5. buildHydroPrefix(const std::vector<QString> &hydroLocStrs) &
   assembleP31Name(const QString &prefixPart, const QString &hydroPrefix,
                   const QString &ihPrefix, const QString &parentName):
   Candidate: string formatting for Blue Book P-31 saturation/substituent/parent name assembly.
6. combineFusionLocantBrackets(const QStringList &locantBrackets, int level):
   Candidate: string formatting for branching fusion bracket combining (comma-separated
   for level 1, colon-separated for level >= 2).

--- REJECTED AS IMPURE / TIED TO DOMAIN LOGIC (LEAVE IN PLACE) ---
1. halogenPrefix(int z) / halogenSuffixWord(int z):
   REJECTED: Atomic number lookup tables (z=9,17,35,53) for elemental halogens.
   Tightly coupled to atom classification / elemental properties, not name assembly.
2. getFusionPrefixShared(RingType t) / getBaseNameShared(RingType t):
   REJECTED: Couples directly to internal RingType enum and ring perception logic
   rather than general string formatting.
3. hwSeniorityRank(int z):
   REJECTED: Chemical priority/seniority scoring rule (P-22.2.2.1.3 citation order)
   on atomic numbers, not string assembly.
4. formatBranchStereoPrefix(...):
   REJECTED: Contains Graph &g traversal, longest-path BFS/DFS searches, and
   cycle checks. Interleaved with chemical structure analysis.
5. tryNameAcyloxySubstituent / tryNameAmidoSubstituent / tryNameNitrosamine:
   REJECTED: Functional group perception containing Indigo calls and graph walks.
6. nameBranchGraph / nameAcyclicChainParentWithSubstituents / simpleRingSubstituentName:
   REJECTED: Core recursive naming logic with deep Graph and Indigo dependencies.
7. getFaceLocants(...):
   REJECTED: Cycle-walk topological matching on atom node vectors, not string formatting.
8. NameScore comparison operators:
   REJECTED: Fusion isomer numbering tie-break scoring rules comparing locant vectors.
===============================================================================
*/

#include <QString>
#include <QStringList>
#include <vector>
#include <utility>

// Chain length root prefixes C1..C20
QString chainRoot(int len);

// Multiplying prefixes (2..10)
QString multiPrefix(int count);

// Helper: check if character is a vowel
bool isVowel(QChar c);
