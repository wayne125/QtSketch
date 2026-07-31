# IUPAC Blue Book Coverage — `src/app/IupacNamer.cpp`

Tracking doc for working through IUPAC nomenclature rules one section at a time.
Two source documents are in scope:

- **Blue Book** — "Nomenclature of Organic Chemistry: IUPAC Recommendations and
  Preferred Names 2013" (P-numbered sections). A full local text conversion exists
  at `C:\Users\jp18b\Downloads\BlueBookV2.md` (converted from the local
  `BlueBookV2.pdf` via `pdftotext -layout` + heuristic markdown structuring — see
  that file's own header for conversion caveats). **Every P-number in this doc
  below was verified directly against that local text**, not recalled from memory
  or a partial web fetch. Web mirror (used only where the local PDF doesn't apply,
  e.g. cross-checking): https://iupac.qmul.ac.uk/BlueBook/
- **FR document** — "Nomenclature of Fused and Bridged Fused Ring Systems" (1998,
  FR-numbered sections). A separate, standalone document. Blue Book P-25.3
  incorporates its content (P-25.3.2 ≈ FR-2/FR-3, P-25.3.3 ≈ FR-4/FR-5) but keeps
  its own P-numbering rather than citing FR-x.x directly — both numbering schemes
  are given below since this project's code comments currently reference FR-x.x.
  Mirror: https://iupac.qmul.ac.uk/fusedring/

Legend: ✅ Covered · ◐ Partial · ✗ Not covered · ？ Section number itself not yet
verified (only remains on FR-document rows now — every Blue Book row below is
confirmed).

Rule for updating this doc: when a row moves from ◐/✗ to ✅ (or gets a real fix),
update its Note with the code location and a one-line summary, the same way
`Features To Be Implemented.md` and `Architecture Map.md` get phase entries.
Verify against the actual fetched/local rule text before marking anything ✅ —
this project's standing discipline is to never trust memory for rule specifics.

---

## Chapter P-1 — General Principles, Rules, and Conventions

Real subsections (verified): P-10 Introduction, P-11 Scope, P-12 Preferred/
Preselected/Retained Names, P-13 Operations in Nomenclature, **P-14 General
Rules**, P-15 Types of Nomenclature, P-16 Name Writing. P-14 is the one that
matters most for this codebase and has its own real sub-numbering:
P-14.0 Introduction, P-14.1 Bonding number, P-14.2 Multiplicative prefixes,
P-14.3 Locants, **P-14.4 Numbering**, **P-14.5 Alphanumerical order**,
P-14.6 Nonalphanumerical order, **P-14.7 Indicated and "added" indicated
hydrogen**, P-14.8 Adducts.

| Section | Title | Status | Note |
|---|---|---|---|
| P-14.2 | Multiplicative prefixes | ◐ | `multiPrefix` (di/tri/tetra... and bis/tris/tetrakis for compound substituents). |
| P-14.4 | Numbering (lowest locants) | ◐ | Lowest-locant tie-breaks used throughout: `PathSignature`, `RingSignature`, `NaphthaleneSignature`, `computePeripheralNumbering`. Not the full P-14.4 hierarchy of tie-break criteria — only the subset this project has needed so far (principal group → unsaturation → substituents → alphabetical-first). |
| P-14.5 | Alphanumerical order | ◐ | Substituent-prefix alphabetization implemented (ignoring multiplying prefixes, e.g. `dimethyl` sorts under `m`). |
| P-14.7 | Indicated and added indicated hydrogen | ◐ | `preferIndicatedHydrogenLocant` + Phase 35/38's `nH-` prefix citation for NH-bearing fused systems (FR-9). **Phase 60** added P-14.7.1 indicated hydrogen citation (`nH-` prefix) for the general Hantzsch-Widman (GENERAL_HETEROCYCLE) path: detects saturated ring positions (both ring bonds single OR all bonds aromatic in odd-membered rings) and prepends `<locant>H-` to the name. General (non-fusion, non-general-heterocycle) indicated hydrogen and "added" indicated hydrogen for suffix-bearing rings remain not covered. |
| P-10-P-13, P-15, P-16, P-14.0/.1/.3/.6/.8 | Everything else in chapter P-1 | ✗ | No dedicated handling; general conventions applied ad hoc rather than as a distinct module. |

## Chapter P-2 — Parent Hydrides

Real subsections: P-20 Introduction, P-21 Mononuclear & acyclic polynuclear
parent hydrides, P-22 Monocyclic parent hydrides, **P-23 Polyalicyclic parent
hydrides (extended von Baeyer system)**, P-24 Spiro ring systems, **P-25 Fused
and bridged fused ring systems**, P-26 Phane nomenclature, P-27 Fullerenes,
P-28 Ring assemblies, P-29 Prefixes denoting substituent groups derived from
parent hydrides.

| Section | Title | Status | Note |
|---|---|---|---|
| P-21 | Mononuclear & acyclic polynuclear parent hydrides | ✅ | Phase 1 acyclic path: `chainRoot` (C1-C20), `an/en/yn` infix + locants, `nameAcidChainFrom`. |
| P-22.1 | Monocyclic hydrocarbon parent hydrides | ✅ | Phase 2 monocyclic: cycloalkane/cycloalkene/benzene via `RingType::CYCLOALKANE/CYCLOALKENE/BENZENE`. |
| P-22.2 | Heteromonocyclic parent hydrides (Hantzsch-Widman) | ◐ | `classifyMonocyclicHeteroRing` recognizes a curated set of retained/systematic names (furan, thiophene, selenophene, tellurophene [Phase 47], pyrrole, pyridine, pyrazole, imidazole, pyridazine, pyrimidine, pyrazine, oxazole, isoxazole, thiazole, isothiazole, selenazole, isoselenazole [Phase 47] + saturated piperidine/pyrrolidine/THF/THT). **Phase 51** replaced the `<GENERAL_HETEROCYCLE>` placeholder with real general Hantzsch-Widman construction (`hwSeniorityRank`, `hwAPrefix`, `hwSixMemberStem` + the citation-order-driven locant/prefix assembly in `generateName()` and `nameRingAsSubstituent()`) for 5- and 6-membered rings only, covering O, S, Se, Te, N, P, As, Sb, Bi, Si, Ge, Sn, Pb, B per verified P-22.2.2.1.1-.1.6 text (element citation order O>S>Se>Te>N>P>As>Sb>Bi>Si>Ge>Sn>Pb>B per P-22.2.2.1.3; 5-ring stem `-ole`; 6-ring stem `-ine`/`-inine` selected by the least-senior-heteroatom's group per P-22.2.2.1.6, with no requirement that nitrogen be present — confirmed against real PINs `1,4-dioxine`, `1,3,5-triazine`, `1,3,5-triphosphinine`). Verified with new tests: `1,3,5-triazine`, `1,2,4-oxadiazole`, `1,2,4-selenadiazole`, `1,3,5-triphosphinine` (355/355 passing). **Phase 55** extended general Hantzsch-Widman support to ring sizes 3, 4, 7, 8, 9, 10 via new `hwGeneralRingStem` function implementing Blue Book Table 2.5 stems: size 3 uses '-irine' if all heteroatoms are nitrogen else '-irene'; sizes 4/7/8/9/10 use '-ete'/'-epine'/'-ocine'/'-onine'/'-ecine' (no composition-based split); size 5/6 unchanged. **Phase 60** added P-14.7.1 indicated hydrogen citation (`<locant>H-` prefix) for general heterocycles: detects saturated positions (both ring bonds single, or all-aromatic odd-membered rings) and cites the locant at the front of the name (e.g. `3H-1,2,4-triazepine`, `2H-1,5-diazonine`, `3H-1,2-diazirine`). Verified with updated tests for Phase 55 examples (394/394 passing). **Gap**: 11+-membered general heterocycles (P-22.2.4) still unimplemented; multiple indicated hydrogen positions (>1) in general heterocycles not yet supported (returns clean error). |
| P-23.0-P-23.2 | Definitions, terminology, naming/numbering von Baeyer hydrocarbons | ◐ | Phase 27: `bicyclo[a.b.c]alkane` — all-carbon, exactly 2 bridgeheads. **Phase 53** added real per-atom numbering (P-23.2.3) + simple-substituent prefixes: exocyclic branches are now permitted if each is a plain saturated acyclic alkyl (named via `nameBranchGraph`) or a bare terminal halogen, classified by the new `simpleRingSubstituentName`; numbering candidates (starting bridgehead × non-increasing bridge permutation) are enumerated and the lowest-locant-set candidate is kept (`PathSignature`/`RingSignature` convention). **Phase 57** added support for double and triple bonds within the ring union, appending `-ene` or `-yne` infixes per standard lowest-locant rules. Verified: `bicyclo[2.2.1]hept-2-ene` plus Phase 53 regressions. **Gaps**: skeletal-replacement heteroatoms (P-23.3-P-23.6), retained names (P-23.7), polycyclic (3+ rings). |
| P-23.3-P-23.6 | Heterogeneous/homogeneous heterocyclic von Baeyer, alternating heteroatoms, nonstandard bonding numbers | ✗ | Not implemented — von Baeyer path is all-carbon only. |
| P-23.7 | Retained names for von Baeyer parent hydrides | ✗ | Not implemented. |
| P-24.0-P-24.2 | Introduction, definitions, spiro with only monocyclic components | ◐ | Phase 28: `spiro[a.b]alkane` — all-carbon, single two-ring spiro. **Phase 53** added real per-atom numbering (P-24.2.1) + simple-substituent prefixes (same `simpleRingSubstituentName` gate as the von Baeyer path, smaller ring numbered first, equal-ring tie and direction chosen by lowest-locants-to-substituents, `PathSignature`/`RingSignature` convention). **Phase 57** added support for double and triple bonds within the ring union, appending `-ene` or `-yne` infixes per standard lowest-locant rules. Verified: `spiro[3.5]non-5-ene`, `spiro[5.5]undec-2-ene` plus Phase 53 regressions. **Gaps**: skeletal-replacement heteroatoms, polyspiro/branched spiro (P-24.3-P-24.8), or nonstandard bonding numbers. |
| P-24.3-P-24.8 | Polyspiro, branched spiro, nonstandard bonding numbers | ✗ | Not implemented — spiro path is exactly-two-monocyclic-rings only. |
| P-25.0 | Introduction | ◐ | Conceptual; see P-25.1-P-25.7 below for the actual implemented/gap breakdown. |
| P-25.1 | Names of hydrocarbon parent ring components | ◐ | Naphthalene (Phase 3), pentalene (Phase 33) covered as retained names. General polyacene/polyaphene/polyalene/polyphenylene/polynaphthylene/polyhelicene systematic naming (P-25.1.2.x) not covered. |
| P-25.2 | Names of heterocyclic parent ring components | ◐ | `classifyMonocyclicHeteroRing`'s curated set (as components) + purine (Phase 32) and pyrrolizine (Phase 34) as retained/special-cased systems. General heteromonocyclic/heteranthrene systematic component naming beyond the curated set not covered. |
| **P-25.3.1** | Definitions, terminology, general principles (ortho-fused, ortho-and-peri-fused, spiro, bridged) | ◐ | Ortho-fused (shared-bond) detection via Indigo SSSR. Ortho-and-peri-fused (3 shared atoms across 2 rings) and bridged-fused relationships not detected/handled. |
| **P-25.3.2** | Constructing two-component fusion names — **this is the Blue Book's own version of what this project's code comments call "FR-2.3"/"FR-3"** | ◐ | Base-component seniority: **all 10 official rules (a)-(j) implemented** (`getRankHetero`, `getVariety`, `getTopAltRank`, `getOwnLocants`, `getFusionLocants`) — verified against P-25.3.2.4's own worked examples during Phases 40-41. Rule (g) confirmed inapplicable for a strict 2-ring system; rules (i)/(j) confirmed unreachable by any of the 13 currently-supported ring types (rule (h) always resolves first) but implemented correctly per spec regardless. Still marked ◐ overall because `getFusionPrefix` + `[n,m-letter]` descriptor construction covers exactly one attached component only — multi-component/3+ ring selection not covered. |
| **P-25.3.2.3** | Orientation of fused ring systems — **the Blue Book's own version of "FR-5.2"** | ◐ | Phase 42: standalone `computePreferredOrientation` (hex-grid embedding + 12-symmetry search, rules a-d). Phase 43: standalone `detectFusedRingDirections` now computes the real per-atom fusion directions this feeds on from an actual Indigo molecule (`dir_out = (dir_in + 3 + k) % 6`, k = outgoing bond's cyclic offset from incoming within the ring's own 6-atom cycle). 6/6 + 5/5 tests pass across both modules, verified on real anthracene/phenanthrene/naphthacene/chrysene/pentacene SMILES. **Still not wired into `generateName()`** — both modules exist and are proven correct standalone, but have no caller yet; extending base-component selection and the fusion-descriptor builder to 3+ rings is separate future work. |
| **P-25.3.3** | Numbering of fused ring systems — **the Blue Book's own version of "FR-4"/"FR-5"** | ◐ | `computePeripheralNumbering` (2-ring) + Phase 45's `computePeripheralNumberingChain3` (exactly 3-ring chain) — both use the same candidate-enumeration + heteroatom-locant tie-break scheme (rules a/b/f from P-25.3.3.1.2), confirmed this correctly does NOT need Phases 42-43's orientation modules (a locant-minimization problem, not a drawing/orientation one). General N-ring (4+) and interior-atom numbering (P-25.3.3.2/.3, for peri-fused/bridged systems) not covered. |
| **P-25.3.4** | Constructing polycomponent fusion names | ◐ | Phase 44: exactly 3 mutually ortho-fused monocyclic rings in a simple chain. Two sub-cases: **middle ring senior** → both neighbors first-order attached, cited together alphabetically (P-25.3.4.2.3.1), base numbering minimizes the {letterA,letterB} pair as a set (P-25.3.4.2.4 rule a) — verified against the real PIN `furo[3,2-b]thieno[2,3-e]pyridine`. **End ring senior** (Phase 46, P-25.3.4.1.1) → the other neighbor becomes a genuine second-order attached component, colon-separated numeric locants with the higher-order component's own locants primed — verified against an independently re-derived (atom-by-atom, from scratch) example, `thieno[3',4':4,5]furo[3,2-c]pyridine`; the rule text's own worked example was found to have a likely PDF-extraction artifact (missing prime marks) that was independently confirmed against other sources before implementing. Phase 45 added real indicated-hydrogen citation for the middle-ring-senior case (verified by hand-enumerating all 8 candidate numberings for a real NH-bearing molecule); indicated hydrogen for the end-ring-senior/second-order case is explicitly deferred with a clean rejection. Phase 48 extended to **4-ring chains, middle-ring-senior sub-case only**: citation combines Phase 44's plain-first-order block with Phase 46's second-order-nested-in-first-order block for the two "sides" of the base — verified against `furo[2,3-b]thieno[2',3':4,5]pyrido[2,3-e]pyrazine`, structurally checked against the same worked-example pattern used to verify Phase 46. 4-ring end-ring-senior (needing third-order/doubly-primed nesting) rejects cleanly, matching the same incremental pattern already used at the 3-ring level. Indicated hydrogen for the 4-ring case is wired in (confirmed `computePeripheralNumberingChain3` generalizes to N=4 with zero changes) but **no test exercises it — present, unverified, not claimed as working**. **Gaps**: multiparent names, identical-attached multiplying prefixes, third-order+ nesting (both 3-ring end-senior and 4-ring end-senior), branching/peri-fusion, more than 4 rings, indicated hydrogen for any second-order sub-case — all explicitly out of scope, cleanly rejected. |
| P-25.3.5 | Heteromonocyclics fused to a benzene ring | ✅ | Phase 31 (benzo-fused heterobicyclics) covers this directly. |
| P-25.3.6 | Identical attached components | ✗ | Not implemented. |
| P-25.3.7 | Multiparent ring systems | ✗ | Not implemented. |
| P-25.3.8 | Omission of locants in fusion descriptors | ✗ | Not implemented — locants are always cited in full. |
| P-25.4 | Bridged fused ring systems | ✗ | Not implemented. |
| P-25.5 | Limitations of fusion nomenclature: three components ortho- and peri-fused together | ✗ | Not implemented — 3+ SSSR rings rejected early unless mutually disjoint. |
| P-25.6 | Fused ring systems with skeletal atoms with nonstandard bonding numbers | ✗ | Not implemented. |
| P-25.7 | Double bonds, indicated hydrogen, and the λ-convention | ◐ | Indicated-hydrogen half covered (see P-14.7/FR-9 rows); the λ-convention (nonstandard bonding number notation) not covered at all. |
| P-26 | Phane nomenclature | ✗ | Not implemented. |
| P-27 | Fullerenes | ✗ | Not implemented, not relevant to this app's scope. |
| P-28 | Ring assemblies | ◐ | `checkRingAssemblyOneSide`: biphenyl/bicyclohexyl style only — two 6-membered rings joined by one single bond, one exocyclic substituent each. **Gap**: other ring sizes/orientations, 3+ ring assemblies. |
| P-29.0-P-29.3 | Definitions, general methodology, simple substituent prefixes from saturated parent hydrides | ✅ | `nameBranchGraph` (acyclic -yl), `nameRingAsSubstituent` (cycloalkyl, phenyl, naphthalen-1/2-yl, heterocyclic -yl with locants). |
| P-29.4 | Compound substituent groups | ◐ | Nested substituents supported (Phase 39's whole point — arbitrary nesting depth), but not the full compound-substituent-group naming rules. |
| P-29.5 | Complex substituent groups | ✗ | Not implemented (multiplicative/complex substituent nomenclature beyond simple nesting). |
| P-29.6 | Retained names for prefixes of simple substituent groups | ◐ | A few retained substituent names present incidentally (e.g. phenyl); not a systematic implementation of the full retained-prefix list. |

## Chapter P-3 — Characteristic (Functional) and Substituent Groups

Real subsections: P-30 Introduction, P-31 Modification of the degree of
hydrogenation of parent hydrides, P-32 Prefixes for substituent groups derived
from parent hydrides with skeletal replacement, P-33 Suffixes, P-34 Functional
parent compounds, P-35 Prefixes corresponding to characteristic groups.

| Section | Title | Status | Note |
|---|---|---|---|
| P-33 | Suffixes | ◐ | The mechanism by which every suffix (-oic acid, -amide, -nitrile, -al, -one, -ol, -amine, -oate, etc.) gets attached to a parent name — implemented per functional-group class (see P-6 chapter rows below for which classes). |
| P-35 | Prefixes corresponding to characteristic groups | ◐ | carboxy-, oxo-, hydroxy-, amino-, sulfo-, sulfanyl-, halo-, -oxy prefix forms all implemented for the classes this project supports. |
| P-31, P-32, P-34 | Hydro/dehydro prefixes; replacement-derived substituent prefixes; functional parent compounds (e.g. "...acid", "...alcohol" as separate-word functional class names) | ✗ | Not implemented. |

## Chapter P-4 — Rules for Name Construction

Real subsections: P-40 Introduction, **P-41 Seniority order for classes**,
**P-42 Seniority order for acids**, **P-43 Seniority order for suffixes**,
**P-44 Seniority order for parent structures**, P-45 Selection of the
preferred IUPAC name, P-46 The principal chain in substituent groups.

| Section | Title | Status | Note |
|---|---|---|---|
| P-41 | Seniority order for classes | ◐ | `GroupType` enum + `groupRank`: sulfonic > sulfinic > carboxylic > phosphonic > boronic > ester > acyl halide > amide > nitrile > aldehyde > thial > ketone > thione > alcohol > thiol > amine > phosphine. Not the full official class list (e.g. no radicals/anions/cations classes, since those are rejected earlier anyway). |
| P-42 | Seniority order for acids | ◐ | Partial — sulfonic > sulfinic > carboxylic > phosphonic > boronic ordering exists, but not the full acid-subclass seniority table (e.g. chalcogen/replacement-analogue acids). |
| P-43 | Seniority order for suffixes | ◐ | Follows directly from P-41's `groupRank` since this project treats "class seniority" and "suffix seniority" as one and the same table — the real Blue Book keeps them as separate (related but distinct) rules; no separate suffix-only override exists in code. |
| **P-44** | **Seniority order for parent structures (ring vs. chain, etc.)** | **◐** | **Phase 52** implemented the real P-44.1.1 + P-44.1.2.2 ring-vs-chain decision at both sites in `generateName()` (the naphthalene block and the monocyclic block): the principal characteristic group is the single most-senior class across ring-attached and chain-attached instances **combined**, and the senior parent structure is the side with MORE occurrences of that class (P-44.1.1); genuine count ties go to the ring (P-44.1.2.2). **Phase 54** generalized chain-as-parent naming from acid-only to `GroupType::ACID`, `AMIDE`, `NITRILE`, `ALDEHYDE`, `KETONE`, `ALCOHOL`, `THIOL`, and `AMINE`, via a new shared `nameAcyclicChainParentWithSubstituents` (extracted from the pure-acyclic path's already-correct per-class suffix table via the new `principalGroupSuffix`/`isPrincipalGroupHeteroNeighbor` helpers, so the suffix logic is single-copy, not reinvented) — `nameAcidChainFrom` is now a thin wrapper over it, and `nameChainParentWithRingSubstituent` takes an explicit `winningType` instead of assuming ACID. Phase 54 also fixed 3 real bugs surfaced while generalizing: (1) chain-attached thiols were never classified at all due to a stray `ringNodeSet.count(i) > 0` restriction on `carbonThiol` at both call sites; (2) the ring-vs-chain instance count incorrectly attributed an exocyclic (ring-ipso-adjacent) principal-group carbon to the ring side, manufacturing false ties for diol/diamine-style cases — fixed by tracking a `chainDeepCount` of unambiguous chain-only instances and preferring chain when a real tie includes at least one; (3) `nameChainParentWithRingSubstituent` excluded exocyclic chain carbons from its own principal-carbon set, so a diol/diamine with one exocyclic + one deep instance was named as a mono-ol/mono-amine instead of a di-. Verified with new tests (373/373 passing): `3-(2-methylphenyl)propanamide`, `propanenitrile`, `propanal`, `4-(2-methylphenyl)butan-2-one` (mid-chain principal-locant case), `propan-1-ol`, `propane-1-thiol`, `propan-1-amine`, and two multi-instance cases `1-(2-methylcyclohexyl)ethane-1,2-diol` / `-diamine`. **Phase 61** closed the last two classes: `BORONIC_ACID` and `PHOSPHINE` are NOT suffix-on-a-numbered-chain classes (there is no "chain root + oic acid"-style suffix for them), so they don't route through `nameChainParentWithRingSubstituent` — instead, when `combinedWinner` is one of these two and the chain side wins, the code finds the boron/phosphorus atom's carbon attachment point and names the whole branch (chain + embedded ring) via the existing `nameBranchGraph(g, carbon, heteroatom, allSSSRRings, ringNodeSet)` — the exact same mechanism the pure-acyclic path already used successfully for plain acyclic boronic acid/phosphine — then appends " boronic acid"/"phosphine". Root cause of the original gap: `carbonBoronicAcid`/`carbonPhosphine` classification (`carbonGroup[i] = GroupType::BORONIC_ACID`/`PHOSPHINE`) was missing from both ring-vs-chain `carbonGroup` construction sites even though the underlying detection maps are populated unconditionally near the top of `generateName()` — added the two missing `else if` branches at both sites. Also fixed a real, previously-latent bug surfaced while wiring this: `nameBranchGraph`'s generic longest-carbon-chain walk (the `while(true)` loop) never actually checked its own `forbiddenNodes` parameter, even though the function threads it through and honors it in its halogen/ether/amine branches — this let the chain-walk wander into a `forbiddenNodes`-excluded cyclic ring and loop forever whenever a caller (like this new Phase 61 path) passed the ring's own atom set as `forbiddenNodes` to keep the chain-walk from wandering into it; fixed by adding the same `!forbiddenNodes.count(...)` guard already used elsewhere in the function to the two neighbor-candidate checks inside that loop. Verified with new tests: `3-(2-methylcyclohexyl)propylboronic acid`, `3-(2-methylcyclohexyl)propylphosphine`, plus regressions for the pure-acyclic `ethylboronic acid`/`ethylphosphine` and an existing Phase 52/54 acid chain-wins case (399/399 passing). **Gaps**: `THIAL`, `THIONE`, `SULFONIC_ACID` are supported as of Phase 56; `ESTER` and `ACYL_HALIDE` are supported as of Phase 58; `P-44.1.2` (heteroatom-in-skeleton seniority, e.g. Si chain vs C ring) remains out of scope; the chain-wins path still requires the ring to attach to the chain at exactly one point and to be a 5/6-membered monocycle (`nameRingAsSubstituent`'s scope) — fused/bridged/polycyclic rings as a chain-parent's substituent are NOT yet supported (a separate future phase). |
| P-45 | Selection of the preferred IUPAC name | ◐ | Tie-break order implemented: principal-group locants → unsaturation locants → substituent locants → alphabetically-first-substituent lowest locant. Not the full PIN-selection hierarchy. |
| P-46 | The principal chain in substituent groups | ◐ | Overlaps with P-29.4's nested-substituent support (Phase 39); not a separately verified implementation of this specific rule. |

## Chapter P-5 — Selecting Preferred IUPAC Names

Real subsections: P-50 Introduction, P-51 Selecting the preferred type of
IUPAC nomenclature, P-52 Selecting PINs/preselected names for parent hydrides,
P-53 Selecting the preferred retained names of parent hydrides, P-54 Selecting the
preferred method for modifying the degree of hydrogenation, P-55 Selecting the
preferred retained name for functional parent compounds, **P-56 Selecting the
preferred suffix for the principal characteristic groups**, P-57 Selecting
preferred/preselected prefixes for substituent group names, **P-58 Selection
of preferred IUPAC names**, **P-59 Name construction**.

| Section | Title | Status | Note |
|---|---|---|---|
| P-56 | Selecting the preferred suffix for the principal characteristic groups | ◐ | This is the real number for the principal-group-selection step (`winningType`/`GroupType` selection in `generateName`) — implemented for the supported class list, not the full official one. |
| P-58 | Selection of preferred IUPAC names | ◐ | Same tie-break machinery as P-45 above (this project doesn't currently distinguish these as separate steps the way the Blue Book does). |
| P-59 | Name construction | ◐ | The final assembly step: `[locants+alphabetized prefixes] + [chain root] + [unsaturation infix] + [suffix]`, vowel-elision (`isVowel`) before a vowel-starting suffix. |
| P-51, P-52, P-53, P-54, P-55, P-57 | Everything else in chapter P-5 | ✗ | Not implemented as distinct steps. |

## Chapter P-6 / P-6a — Applications to Specific Classes of Compounds

Real subsections: P-60 Introduction, P-61 Substitutive nomenclature (prefix
mode), **P-62 Amines and imines**, **P-63 Hydroxy compounds, ethers, peroxols,
peroxides, chalcogen analogues**, **P-64 Ketones, pseudoketones, heterones,
chalcogen analogues**, **P-65 Acids, acyl halides/pseudohalides, salts,
esters, anhydrides**, **P-66 Amides, imides, hydrazides, nitriles,
aldehydes**, **P-67 Mononuclear and polynuclear noncarbon acids**, P-68
Nomenclature of other classes of compounds, P-69 Organometallic compounds.

| Section | Title | Status | Note |
|---|---|---|---|
| P-62.2 | Amines | ✅ | `GroupType::AMINE`, suffix `-amine`, prefix `amino-`. |
| P-62.3 | Imines | ✗ | Not implemented. |
| P-63.1 | Hydroxy compounds (alcohols) and chalcogen analogues | ◐ | `GroupType::ALCOHOL` (suffix `-ol`, prefix `hydroxy-`) and `GroupType::THIOL` implemented; selenium/tellurium chalcogen analogues not. |
| P-63.2 | Ethers and chalcogen analogues | ◐ | Ether oxygen always handled as `-oxy` prefix (correct — ethers have no suffix form); thioether (`etherOxygens`-equivalent for sulfur) present, selenium/tellurium analogues not. |
| P-63.3-P-63.7 | Peroxides, hydroperoxides, cyclic ethers/sulfides as ring parents, sulfoxides/sulfones, polyfunctional compounds | ✗ | Not implemented. |
| P-64.2 | Ketones | ✅ | `GroupType::KETONE`, suffix `-one`, prefix `oxo-`. |
| P-64.3, P-64.4 | Pseudoketones, heterones | ✗ | Not implemented. |
| P-65.1 | Carboxylic acids and functional replacement analogues | ✅ | `GroupType::ACID`, `-oic acid`, via `nameAcidChainFrom`. Functional replacement analogues (thio-, seleno-, telluro- acids) not covered. |
| P-65.4 | Acyl groups as substituent groups | ◐ | Present for the acid/amide/ester code paths that need it; not a general standalone acyl-substituent mechanism. |
| P-65.5 | Acyl halides and pseudohalides | ◐ | `GroupType::ACYL_HALIDE` covers halides (Cl/Br/F/I); pseudohalides (cyanide, azide as acyl-pseudohalide forms) not covered as this class. |
| P-65.6 | Salts and esters | ◐ | Esters covered (`GroupType::ESTER`, `esterAlkylRoot`/`esterOxygen`); salts not covered at all (charged species rejected early). |
| P-65.7 | Anhydrides and their analogues | ◐ | Symmetric/single-chain anhydride naming via `nameAcidChainFrom`; true two-parent-name anhydride construction not covered. |
| P-66.1 | Amides | ✅ | `GroupType::AMIDE`, suffix `carboxamide`/`-amide`. |
| P-66.2, P-66.3, P-66.4 | Imides, hydrazides, amidines/amidrazones/hydrazidines/amidoximes | ✗ | Not implemented. |
| P-66.5 | Nitriles | ✅ | `GroupType::NITRILE`, suffix `-nitrile`/`carbonitrile`, prefix `cyano-`. |
| P-66.6 | Aldehydes | ✅ | `GroupType::ALDEHYDE`, suffix `-al`/`carbaldehyde`, prefix `oxo-`/`formyl-`. |
| P-67.1 | Mononuclear noncarbon oxoacids | ◐ | Sulfonic acid (`GroupType::SULFONIC_ACID`), sulfinic acid (`GroupType::SULFINIC_ACID`), boronic acid (`GroupType::BORONIC_ACID`), and phosphonic acid (`GroupType::PHOSPHONIC_ACID`) covered via the same numbered-chain-suffix machinery as SULFONIC_ACID; the general noncarbon-oxoacid system (phosphinic, arsonic, etc.) is not — phosphine (`GroupType::PHOSPHINE`) covers the amine-analogue class, not the oxoacid class. |
| P-67.2, P-67.3 | Di-/polynuclear noncarbon oxoacids; substitutive/functional-class names of polyacids | ✗ | Not implemented. |
| P-68 | Nomenclature of other classes of compounds (isocyanates, azides, nitro compounds, etc. mostly live here) | ◐ | Azide, nitro, isocyanate, disulfide all implemented as individual detected groups even though this section's exact subsection breakdown wasn't individually verified this pass. |
| P-69 | Organometallic compounds | ✗ | Not implemented — organometallics out of scope. |
| P-61 | Substitutive nomenclature: prefix mode | ◐ | This is the general rule underlying every non-principal-group substituent becoming a prefix — implemented as the default behavior throughout, not as a separately cited rule. |

## Chapter P-7 — Radicals, Ions, and Related Species

Real subsections: P-70 Introduction, P-71 Radicals, P-72 Anions, P-73 Cations,
P-74 Zwitterions, P-75 Radical ions, P-76 Delocalized radicals/ions, P-77
Salts.

| Section | Title | Status | Note |
|---|---|---|---|
| P-71 | Radicals | ✗ | Rejected early (`indigoGetRadicalElectrons` check). |
| P-72, P-73 | Anions, Cations | ✗ | Charged atoms rejected early, with narrow exemptions (nitro N+/O-, azide, aci-nitro resonance forms) that pass the charge gate since they're drawn charged but named as neutral functional groups. |
| P-74, P-75, P-76, P-77 | Zwitterions, radical ions, delocalized species, salts | ✗ | Not implemented. |

## Chapter P-8 — Isotopically Modified Compounds

Real subsections: P-80 Introduction, P-81 Symbols and definitions, P-82
Isotopically substituted compounds, P-83 Isotopically labeled compounds, P-84
Comparative examples.

| Section | Title | Status | Note |
|---|---|---|---|
| P-82, P-83 | Isotopically substituted / labeled compounds | ✗ | Rejected early (`indigoIsotope` check). |

## Chapter P-9 — Specification of Configuration and Conformation

Real subsections: P-90 Introduction, P-91 Stereoisomer graphical
representation and naming, **P-92 The Cahn-Ingold-Prelog (CIP) priority
system and the Sequence Rules**, **P-93 Configuration specification**, P-94
Conformation and conformational stereodescriptors. P-92 breaks down further
into P-92.1 General methodology, P-92.2-P-92.6 Sequence Rules 1-5. P-93 breaks
into P-93.0 Introduction, P-93.1 General aspects, P-93.2 Nontetrahedral/other-
element tetrahedral configuration, P-93.3 Nontetrahedral configuration,
**P-93.4 Configuration specification of acyclic organic compounds**, P-93.5
cyclic compounds, P-93.6 compounds composed of rings and chains.

| Section | Title | Status | Note |
|---|---|---|---|
| P-92.1-P-92.5 | CIP general methodology + Sequence Rules 1-4 | ◐ | `indigoAddCIPStereoDescriptors` delegates to Indigo's own CIP implementation rather than this codebase re-implementing the Sequence Rules directly — `formatStereoPrefix` just consumes Indigo's result and formats `(nR)-`/`(nS)-`. |
| P-92.6 | Sequence Rule 5 (R precedes S, etc. — the rule that resolves pseudoasymmetry) | ✗ | Pseudoasymmetric centers (lowercase r/s) explicitly rejected, so this rule is never reached. |
| P-93.4 | Configuration specification of acyclic organic compounds (covers both R/S and E/Z for acyclic systems) | ◐ | R/S via Indigo CIP (see above). E/Z via `processDoubleBondStereo` — disubstituted C=C only, priority by first-atom atomic number; trisubstituted double bonds, second-shell CIP tie-breaks, and cumulated double bonds (allenes) all rejected. |
| P-93.5, P-93.6 | Configuration specification of cyclic compounds; compounds composed of rings and chains | ✗ | Stereocenters on ring/substituent-branch atoms are rejected rather than named. |
| P-91, P-94 | Stereoisomer graphical representation; conformation/conformational stereodescriptors | ✗ | Not implemented. |

## Chapter P-10 — Parent Structures for Natural Products

Real subsections: P-100 Introduction, P-101 Nomenclature for natural products
based on parent structures (trivial/semisystematic names for alkaloids,
steroids, terpenes, and related compounds), P-102 Carbohydrate nomenclature,
P-103 Amino acids and peptides, P-104 Cyclitols, P-105 Nucleosides, P-106
Nucleotides, P-107 Lipids.

| Section | Title | Status | Note |
|---|---|---|---|
| P-100-P-107 | All natural-product parent structures | ✗ | Not implemented. Purine (Phase 32) is the closest thing present, and it's handled as a P-25 fused-ring retained name, not a P-10 natural-product parent. |

---

## Standalone FR Document — "Nomenclature of Fused and Bridged Fused Ring Systems" (1998)

Referenced by Blue Book P-25.3 (see the P-25.3.x rows above for the Blue-Book-
native numbering of the same rules) but independently numbered in its own
document. Everything below is scoped to the two-ring ortho-fused case only —
nothing here handles 3+ rings or bridged fusion. FR-x.x numbers below are still
sourced from the standalone `fusedring` site, not the local Blue Book PDF.

| Section | Title | Status | Note |
|---|---|---|---|
| FR-0 | Introduction | — | Conceptual; ortho-fusion/spiro relationships detected via Indigo SSSR shared-atom/shared-bond checks. |
| FR-1 | Definitions | ◐ | ≈ Blue Book P-25.3.1. Ortho-fused and spiro recognized. Bridged and peri-fused relationships not exercised. |
| FR-2.1 / FR-2.2 | Ring systems used as components | ◐ | ≈ Blue Book P-25.1/P-25.2. Component recognition via `classifyMonocyclicHeteroRing` + benzo detection — curated set only. |
| **FR-2.3** | **Base-component seniority** | **◐** | ≈ Blue Book P-25.3.2. Rules (a)-(f), (h), (i) implemented (`getRankHetero`, `getVariety`, `getTopAltRank`, `getOwnLocants`) — verified against the source's own worked examples during Phase 40. Rule (g) confirmed inapplicable for a strict 2-ring system — documented as skipped, not silently dropped. Rule (j) (lower locants for bridgehead carbons) is now also implemented (Phase 41) — `getCandidates` was moved to run before the base-component choice, and `getFusionLocants` compares each ring's own lowest-achievable bridgehead-locant pair. Confirmed unreachable by any of the 13 currently-supported ring types (rule (h) always resolves first), same status as rule (i) — not a gap, a structural non-issue until heavier heteroatoms are added. |
| FR-2.4 | (title not yet fetched from the standalone FR site) | ？ | Not verified — this is in the separate 1998 document, not the Blue Book, so the local PDF conversion doesn't help here. |
| FR-3.1-3.3 | Construction of fusion names (first-order) | ◐ | ≈ Blue Book P-25.3.2. `getFusionPrefix` + `[n,m-letter]` descriptor built for exactly one attached component. |
| FR-3.4-3.6 | Higher-order / selection among multiple attached components | ✗ | ≈ Blue Book P-25.3.4/P-25.3.6/P-25.3.7. Not implemented (needs 3+ ring support first). |
| FR-4.1 | Sides of the base component (letter locants) | ◐ | ≈ Blue Book P-25.3.3. Letter computed from fusion-bond position (`minLetterIdx` → `'a' + idx`). Two-ring, single-fusion-bond case only. |
| FR-4.2 | Sides of the attached component (numerical locants) | ◐ | `bestAttPair`. Two-ring only. |
| FR-4.3 | First-order fusion descriptor assembly | ✅ | Produces `furo[2,3-b]pyridine`-style names correctly. |
| FR-4.4 | Higher-order attached-fusion descriptor | ✗ | Not implemented. |
| FR-4.5 | Choice among locant alternatives | ◐ | Lowest base-letter then lowest attached-pair implemented; the fuller (a)-(h) sub-rule hierarchy is not. |
| FR-4.6-4.9 | Omission of locants, etc. | ✗ | ≈ Blue Book P-25.3.8. Not implemented, not separately fetched from the FR site. |
| FR-5.1 | Drawing / orientation conventions | ✗ | Uses Indigo's own 2D layout; no hexagonal-grid orientation normalization. |
| **FR-5.2** | **Preferred orientation (max rings in a row, quadrant placement)** | **◐** | ≈ Blue Book P-25.3.2.3. Built (Phase 42) as a standalone module, now fed real molecule data by Phase 43's direction detector — see the P-25.3.2.3 row above for detail. Not yet wired into the naming pipeline, and not yet consulted by rule (g) above, so real end-to-end 3+ ring naming still doesn't work — but the hardest algorithmic pieces are done and tested. |
| FR-5.3 | Peripheral numbering | ◐ | ≈ Blue Book P-25.3.3. `computePeripheralNumbering` — two-ring ortho-fused case only (clockwise from upper-right, fusion carbons lettered). No general multi-ring peripheral numbering. |
| FR-5.4 | Choice among alternative numberings | ◐ | Heteroatom-lowest-locant-set tie-break used; the fuller (a)-(f) hierarchy is not fully applied. |
| FR-5.5 | Interior numbering (peri-fused/bridged interior atoms) | ✗ | Not implemented. |
| FR-6 | Multi-parent (more than 2 base components) systems | ✗ | ≈ Blue Book P-25.3.7. Not implemented. |
| FR-7 | Three components ortho- and peri-fused together | ✗ | ≈ Blue Book P-25.5. Not implemented — 3+ SSSR rings are rejected early unless mutually disjoint. |
| FR-8 | Bridged fused ring systems | ✗ | ≈ Blue Book P-25.4. Not implemented. |
| FR-9 | Indicated hydrogen | ◐ | ≈ Blue Book P-14.7 and P-25.7. `preferIndicatedHydrogenLocant` branch in `computePeripheralNumbering` + Phase 35's `nH-` prefix for NH-bearing fused systems (pyrrole/imidazole/pyrazole). **Phase 60** extended to general monocyclic heterocycles (GENERAL_HETEROCYCLE path): `<locant>H-` prefix citation for saturated positions in odd-membered rings. General indicated-hydrogen for non-fusion, non-general-heterocycle cases not covered. |
| Appendix 1/2 | Full seniority tables (hydrocarbon/heterocyclic components) | ◐ | Phase 47 added Se and Te; Phase 49 added P (phosphinine only — phosphole was scoped out as not reliably aromatic). N,O,S,Se,Te,P now covered, correctly ordered in both the primary rule-(a) order and the alternate rule-(f) order. Remaining elements (As,Sb,Bi,Si,Ge,Sn,Pb,B,Al,Ga,In,Tl) not implemented — most also have nonstandard bonding numbers requiring the λ-convention (P-25.3.2.5.2/P-14.1.3), a separate unimplemented piece. |

---

## Explicitly-documented gaps (call these out honestly wherever hit, don't paper over)

- **P-44 ring-vs-chain seniority — partially covered as of Phase 63**: the P-44.1.1 instance-count decision and the P-44.1.2.2 ring-wins-on-genuine-tie default are implemented, and the chain-wins case is now named for `ACID`, `AMIDE`, `NITRILE`, `ALDEHYDE`, `KETONE`, `ALCOHOL`, `THIOL`, `AMINE`, `THIAL`, `THIONE`, `SULFONIC_ACID`, `SULFINIC_ACID`, `ESTER`, `ACYL_HALIDE` (via `nameAcyclicChainParentWithSubstituents`), `PHOSPHONIC_ACID`, and now `BORONIC_ACID`/`PHOSPHINE` too (via the pure-acyclic path's own `nameBranchGraph`-based whole-branch naming, reused rather than duplicated). Still gaps: `P-44.1.2` heteroatom-skeleton seniority (e.g. Si chain vs C ring) is not implemented; chain-wins still requires a single ring<->chain attachment and a 5/6-membered monocyclic ring (a polysubstituted naphthalene + acid chain still rejects; fused/bridged/polycyclic-ring-as-substituent is a separate future phase).
- P-92.6 (Sequence Rule 5) never reached — pseudoasymmetric stereocenters, substituent-branch stereocenters, P-93.4's 2nd-shell/trisubstituted E/Z, and allenes all rejected.
- Multi-component (disconnected) molecules rejected outright (P-72/P-73/P-77 charged/salt forms out of scope entirely).
- 3+ SSSR rings rejected unless all mutually disjoint (no fused/bridged/spiro combination beyond the specific phases listed above; P-25.4/P-25.5/P-25.6 all out of scope).
- **Phase 59**: `CC1CCCCC1CCC(=O)Oc1ccccc1` (phenyl ester of 3-(2-methylcyclohexyl)propanoic acid — a molecule with 2 separate, non-fused monocyclic rings joined only by an acyclic ester linkage) now correctly produces `success=true, name="phenyl 3-(2-methylcyclohexyl)propanoate"`. Fix: added early guard at start of `ringCount == 2` dispatch sequence (before biphenyl detection at line 2661) that detects when the two SSSR rings are fully disjoint (no shared atoms and no direct bond between them) and marks `twoRingsAreDisjoint=true`; then modified ring-substituent detection condition (line 2852) from `mainChainExoCount == 1` to `mainChainExoCount == 1 || twoRingsAreDisjoint` to allow both disjoint rings to be treated as ring substituents and funneled into the P-44 acyclic path, where Phase 58's ester logic correctly names both the phenyl ester alkyl group and the methylcyclohexyl-substituted acid chain.

## Numbers still needing verification

Everything Blue Book (P-x.x) is now verified against the local full-text
conversion. Only the standalone FR-1998 document (separate from the Blue Book)
still has gaps:

- FR-2.4 — title/rule text not yet fetched from https://iupac.qmul.ac.uk/fusedring/.
- FR-4.6-4.9 — only FR-4.1-4.5 were fetched from that site.

---

## Suggested working order ("one by one")

Roughly cheapest/highest-value first, but re-prioritize freely — this is a queue, not a commitment:

**Note on naming collision**: this codebase's own "Phase 44" (the 3-ring fusion work)
and Blue Book **section P-44** ("Seniority order for parent structures", ring-vs-chain)
are unrelated things that happen to share the number 44 — disambiguated explicitly below
wherever both could appear.

1. **Third-order nesting (doubly-primed locants) and genuine N-ring generalization.**
   Phase 48 closed the 4-ring "middle ring is senior" gap. **Note (Phase 49/50):** Phase
   48's code, along with Phase 45 (3-ring indicated-H) and Phase 46 (3-ring second-order),
   was entirely lost to an uncommitted-work-discarding delegation and had to be rebuilt
   from scratch (see `Features To Be Implemented.md`'s Phase 49/50 entry for the full
   account) — the capability described here is real and re-verified (351/351), but the
   underlying code is a fresh rewrite, not the original Phase 48 implementation. What's
   still left: the "end ring is senior" sub-case at 4 rings (and any N-ring chain
   generally) needs real third-order attachment with doubly-primed locants — the
   topology-detection and citation-construction logic in Phases 44/46/48 is currently
   hand-written per ring-count (3, then 4) rather than genuinely N-generic; a real N-ring
   generalization would replace all three growing, near-duplicate blocks with one. Also:
   the rewritten 4-ring block explicitly requires all 4 rings to be non-NH-type and falls
   through to a clean rejection otherwise — indicated hydrogen for the 4-ring case isn't
   wired up at all (a disclosed gap, not untested code masquerading as working).
2. **P-25.4/P-25.5 (peri-fused / bridged-fused / 3-component-limit systems)** — needs
   `FusedRingOrientation`/`FusedRingDirectionDetector` extended beyond tree-only ring-
   adjacency graphs, plus P-25.3.2.5.1's common-heteroatom-at-fusion rule
   (`imidazo[2,1-b][1,3]thiazole`-style bridgehead-N systems) as a related sub-case. This
   remains the actual point where Phases 42-43's orientation modules would first become
   load-bearing for a monocyclic-chain-adjacent case (peri-fused geometry genuinely needs
   drawing/orientation reasoning, unlike the simple-chain numbering Phase 45 just solved
   without them).
3. **P-22.2 general Hantzsch-Widman stem construction** — **Phase 51** implemented for ring sizes 5-6; **Phase 55** extended to sizes 3-4 and 7-10. Remaining: 11+ membered rings per P-22.2.4.
4. **P-23/P-24 von Baeyer & spiro**: add unsaturation and heteroatoms (simple alkyl/halogen substituents now named in Phase 53 for saturated all-carbon bicyclic/spiro; still saturated+all-carbon-at-skeleton only — P-23.3-P-23.7 and P-24.3-P-24.8 entirely unimplemented).
5. **Blue Book section P-44 (ring-vs-chain and other parent-structure seniority)** — **Phase 52 closed the P-44.1.1 + P-44.1.2.2 core** (count-based decision + ring-wins-on-tie); **Phase 54 generalized chain-as-parent naming** from carboxylic-acid-only to ACID/AMIDE/NITRILE/ALDEHYDE/KETONE/ALCOHOL/THIOL/AMINE (`nameChainParentWithRingSubstituent` + new `nameAcyclicChainParentWithSubstituents`), fixing 3 bugs surfaced in the process (a stray ring-only restriction blocking chain-attached thiol classification; a false-tie in the ring/chain instance count caused by miscounting an exocyclic principal carbon as ring-side; and `nameChainParentWithRingSubstituent` itself dropping exocyclic principal carbons from its own count, under-naming diol/diamine cases). Remaining: `P-44.1.2` heteroatom-skeleton seniority (Si/Ge/... chain vs C ring) (SULFONIC_ACID/THIAL/THIONE added in Phase 56; ESTER/ACYL_HALIDE added in Phase 58; BORONIC_ACID/PHOSPHINE added in Phase 61, via `nameBranchGraph` rather than the numbered-chain-suffix machinery), and lifting the single-attachment / monocyclic-ring restrictions (fused/bridged/polycyclic ring as a chain substituent — scoped as a separate future phase). (Not to be confused with this codebase's own Phase 44, above.)
6. **P-9 stereochemistry completeness**: P-92.6 (pseudoasymmetric/Sequence Rule 5), P-93.4/93.5/93.6 (substituent-branch and cyclic stereocenters, 2nd-shell E/Z tie-breaks, allenes).
7. Lower priority / rarely load-bearing for this app: P-26 (phane), P-27 (fullerenes), P-7/P-8 (ions/isotopes), P-10 (natural products).
