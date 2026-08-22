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
| P-22.2 | Heteromonocyclic parent hydrides (Hantzsch-Widman) | ◐ | `classifyMonocyclicHeteroRing` recognizes a curated set of retained/systematic names (furan, thiophene, selenophene, tellurophene [Phase 47], pyrrole, pyridine, pyrazole, imidazole, pyridazine, pyrimidine, pyrazine, oxazole, isoxazole, thiazole, isothiazole, selenazole, isoselenazole [Phase 47] + saturated piperidine/pyrrolidine/THF/THT). **Phase 51** replaced the `<GENERAL_HETEROCYCLE>` placeholder with real general Hantzsch-Widman construction (`hwSeniorityRank`, `hwAPrefix`, `hwSixMemberStem` + the citation-order-driven locant/prefix assembly in `generateName()` and `nameRingAsSubstituent()`) for 5- and 6-membered rings only, covering O, S, Se, Te, N, P, As, Sb, Bi, Si, Ge, Sn, Pb, B per verified P-22.2.2.1.1-.1.6 text (element citation order O>S>Se>Te>N>P>As>Sb>Bi>Si>Ge>Sn>Pb>B per P-22.2.2.1.3; 5-ring stem `-ole`; 6-ring stem `-ine`/`-inine` selected by the least-senior-heteroatom's group per P-22.2.2.1.6, with no requirement that nitrogen be present — confirmed against real PINs `1,4-dioxine`, `1,3,5-triazine`, `1,3,5-triphosphinine`). Verified with new tests: `1,3,5-triazine`, `1,2,4-oxadiazole`, `1,2,4-selenadiazole`, `1,3,5-triphosphinine` (355/355 passing). **Phase 55** extended general Hantzsch-Widman support to ring sizes 3, 4, 7, 8, 9, 10 via new `hwGeneralRingStem` function implementing Blue Book Table 2.5 stems: size 3 uses '-irine' if all heteroatoms are nitrogen else '-irene'; sizes 4/7/8/9/10 use '-ete'/'-epine'/'-ocine'/'-onine'/'-ecine' (no composition-based split); size 5/6 unchanged. **Phase 60** added P-14.7.1 indicated hydrogen citation (`<locant>H-` prefix) for general heterocycles: detects saturated positions (both ring bonds single, or all-aromatic odd-membered rings) and cites the locant at the front of the name (e.g. `3H-1,2,4-triazepine`, `2H-1,5-diazonine`, `3H-1,2-diazirine`). Verified with updated tests for Phase 55 examples (394/394 passing). **Saturated ring parents (piperidine/pyrrolidine/THF/THT) with a substituent or suffix group — fixed 2026-08-21** (commit `309d411`): these 4 curated saturated types were already correctly classified by `classifyMonocyclicHeteroRing` when `allowSaturated=true`, but the main ring-as-parent call site in `generateName()` passed the default `false`, so a saturated ring bearing anything beyond a bare unsubstituted structure (e.g. an `-ol`/`-one` suffix, or the diketone that makes a cyclic imide like succinimide nameable at all — see the P-66.2 row above) cleanly but incorrectly rejected with "Saturated or partially unsaturated heterocycles are not yet supported". Fixed by flipping that one flag plus adding these 4 types to two existing allow-lists already used by the aromatic heterocycles (the heteroatom-anchored numbering-candidate branch, and the "root already ends in e, don't double it" bare-name list) — no new naming logic needed, both the numbering and suffix-assembly machinery were already generic enough. Verified: `pyrrolidine-2,5-dione` (succinimide, the P-66.2.1 worked example) and `piperidin-4-ol` (hand-verified: piperidine's own 6-ring symmetry makes locant 4 correct from either numbering direction). `iupac_namer_test` 490→492/492, `ctest` 11/11. **Gap**: multiple indicated hydrogen positions (>1) in general heterocycles not yet supported (returns clean error). 11-20-membered heterocycles (P-22.2.3/.4, a different mechanism than Hantzsch-Widman -- see the Phase 70/Part A/Part B entry in the "Suggested working order" section below) are now covered for the saturated, mancude, and substituted cases; 21+ and any partially-saturated-but-not-maximal composition remain unimplemented/cleanly rejected. |
| P-23.0-P-23.2 | Definitions, terminology, naming/numbering von Baeyer hydrocarbons | ◐ | Phase 27: `bicyclo[a.b.c]alkane` — all-carbon, exactly 2 bridgeheads. **Phase 53** added real per-atom numbering (P-23.2.3) + simple-substituent prefixes: exocyclic branches are now permitted if each is a plain saturated acyclic alkyl (named via `nameBranchGraph`) or a bare terminal halogen, classified by the new `simpleRingSubstituentName`; numbering candidates (starting bridgehead × non-increasing bridge permutation) are enumerated and the lowest-locant-set candidate is kept (`PathSignature`/`RingSignature` convention). **Phase 57** added support for double and triple bonds within the ring union, appending `-ene` or `-yne` infixes per standard lowest-locant rules. Verified: `bicyclo[2.2.1]hept-2-ene` plus Phase 53 regressions. **P-23.3 heteroatoms fixed 2026-08-21** (commit `7627955`): the all-carbon gate now accepts any `hwSeniorityRank`-supported element; the numbering-candidate comparator gained heteroatom-locant-set then heteroatom-seniority as its two highest-priority fields (P-23.3.2.1/.2), ahead of the pre-existing double/triple-bond/substituent tie-breaks, since P-23.3.1 fixes hydrocarbon numbering first and heteroatom rules only break remaining ties. Verified against real PINs `3-oxabicyclo[3.2.1]octane`, `7-oxabicyclo[2.2.1]heptane` (real oxanorbornane), `2-oxa-4-thiabicyclo[3.2.1]octane` (seniority tie-break), and `3,6,8-trioxabicyclo[3.2.2]nonane` (P-23.3.2.1's own worked lowest-locant-set example, across two equal-length bridges) — the last of these caught a real test-construction error during independent review (the first hand-built molecule put both oxygens adjacent to the *same* bridgehead, which is topologically impossible to number as `3,6,8` since von Baeyer numbering walks consecutive bridges in alternating direction; confirmed by dumping the full generated candidate space, which correctly never contained `{3,6,8}` for that flawed molecule and correctly produced it once the molecule was rebuilt with the two heteroatoms adjacent to opposite bridgeheads — the code was right, the test was wrong). **Gaps**: P-23.4-P-23.6 (homogeneous/alternating-heteroatom naming, nonstandard bonding numbers), retained names (P-23.7), polycyclic (3+ rings). |
| P-23.4-P-23.6 | Homogeneous/alternating-heteroatom von Baeyer, nonstandard bonding numbers | ✗ | Not implemented — falls through to generic rejection. |
| P-23.7 | Retained names for von Baeyer parent hydrides | ◐ | **Adamantane and cubane fixed 2026-08-21** (commit `9893ba6`, "Phase 26" — a standalone detection inserted before Phase 27, since both are structurally unreachable by the 2-bridgehead-only bicyclic machinery). Detects the exact skeleton via atom/ring count + degree sequence, then a real subgraph isomorphism search against a verified reference adjacency list (bridgeheads land on the correct standard locants 1/3/5/7 for adamantane). Handles simple substituents (reuses `simpleRingSubstituentName`) and one principal group (ACID/AMIDE/KETONE/ALCOHOL/AMINE, reuses `principalGroupSuffix`) with lowest-locant selection across all valid isomorphism mappings. A negative-control test (a same-atom-count-class bicyclic) confirmed no false positives. Quinuclidine confirmed already nameable systematically (`1-azabicyclo[2.2.2]octane`) by the existing heteroatom-aware von Baeyer path — no retained-name shortcut added for it (a first draft did this via a crude string-substitution hack on the assembled name; removed on review as the exact string-matching anti-pattern this codebase avoids elsewhere, and because it would have broken the test confirming the systematic path already works). `iupac_namer_test` 478→486/486, `ctest` 11/11. **Gap**: any other individually-named von Baeyer retained structure beyond these two. |
| P-24.0-P-24.2 | Introduction, definitions, spiro with only monocyclic components | ◐ | Phase 28: `spiro[a.b]alkane` — all-carbon, single two-ring spiro. **Phase 53** added real per-atom numbering (P-24.2.1) + simple-substituent prefixes (same `simpleRingSubstituentName` gate as the von Baeyer path, smaller ring numbered first, equal-ring tie and direction chosen by lowest-locants-to-substituents, `PathSignature`/`RingSignature` convention). **Phase 57** added support for double and triple bonds within the ring union, appending `-ene` or `-yne` infixes per standard lowest-locant rules. Verified: `spiro[3.5]non-5-ene`, `spiro[5.5]undec-2-ene` plus Phase 53 regressions. **P-24.2.4 heteroatoms fixed 2026-08-21** (commit `8936a6d`): exact mirror of the von Baeyer fix above (same gate relaxation, same two new front-priority comparator fields, reuses the same `buildSkeletalReplacementPrefix` helper the von Baeyer fix introduced rather than a third copy). Note the coverage doc previously mis-cited this as "P-24.3-P-24.8" — the correct section for heteroatoms in the existing 2-monocyclic-ring spiro case is P-24.2.4; P-24.3+ is a separate, unrelated, still-unimplemented polycyclic-component-spiro feature (see below). Verified against real PINs `6-oxaspiro[4.5]decane` (P-24.2.4.1.1, constructed so the wrong-direction locant 10 was a real alternative, not accidentally symmetric), `9-oxa-6-azaspiro[4.5]decane` (P-24.2.4.1.2(a) lowest-locant-set), `7-thia-9-azaspiro[4.5]decane` (P-24.2.4.1.2(b) seniority tie-break). **Gaps**: P-24.2.4.2/.3 (homogeneous/alternating-heteroatom naming), polyspiro/branched spiro (P-24.3-P-24.8), nonstandard bonding numbers. |
| P-24.3-P-24.8 | Polyspiro, branched spiro, nonstandard bonding numbers | ✗ | Not implemented — spiro path is exactly-two-monocyclic-rings only. |
| P-25.0 | Introduction | ◐ | Conceptual; see P-25.1-P-25.7 below for the actual implemented/gap breakdown. |
| P-25.1 | Names of hydrocarbon parent ring components | ◐ | Naphthalene (Phase 3), pentalene (Phase 33) covered as retained names. General polyacene/polyaphene/polyalene/polyphenylene/polynaphthylene/polyhelicene systematic naming (P-25.1.2.x) not covered. |
| P-25.2 | Names of heterocyclic parent ring components | ◐ | `classifyMonocyclicHeteroRing`'s curated set (as components) + purine (Phase 32) and pyrrolizine (Phase 34) as retained/special-cased systems. General heteromonocyclic/heteranthrene systematic component naming beyond the curated set not covered. |
| **P-25.3.1** | Definitions, terminology, general principles (ortho-fused, ortho-and-peri-fused, spiro, bridged) | ◐ | Ortho-fused (shared-bond) detection via Indigo SSSR. Ortho-and-peri-fused (3 rings, common atom shared by all three) and bridged-fused (2 rings sharing 3+ atoms directly) relationships are now detected and cleanly rejected as of 2026-08-21 (commit `5173962`) — detection only, name construction still not implemented. |
| **P-25.3.2** | Constructing two-component fusion names — **this is the Blue Book's own version of what this project's code comments call "FR-2.3"/"FR-3"** | ◐ | Base-component seniority: **all 10 official rules (a)-(j) implemented** (`getRankHetero`, `getVariety`, `getTopAltRank`, `getOwnLocants`, `getFusionLocants`) — verified against P-25.3.2.4's own worked examples during Phases 40-41. Rule (g) confirmed inapplicable for a strict 2-ring system; rules (i)/(j) confirmed unreachable by any of the 13 currently-supported ring types (rule (h) always resolves first) but implemented correctly per spec regardless. Still marked ◐ overall because `getFusionPrefix` + `[n,m-letter]` descriptor construction covers exactly one attached component only — multi-component/3+ ring selection not covered. |
| **P-25.3.2.3** | Orientation of fused ring systems — **the Blue Book's own version of "FR-5.2"** | ◐ | Phase 42: standalone `computePreferredOrientation` (hex-grid embedding + 12-symmetry search, rules a-d). Phase 43: standalone `detectFusedRingDirections` now computes the real per-atom fusion directions this feeds on from an actual Indigo molecule (`dir_out = (dir_in + 3 + k) % 6`, k = outgoing bond's cyclic offset from incoming within the ring's own 6-atom cycle). 6/6 + 5/5 tests pass across both modules, verified on real anthracene/phenanthrene/naphthacene/chrysene/pentacene SMILES. **Still not wired into `generateName()`** — both modules exist and are proven correct standalone, but have no caller yet; extending base-component selection and the fusion-descriptor builder to 3+ rings is separate future work. |
| **P-25.3.3** | Numbering of fused ring systems — **the Blue Book's own version of "FR-4"/"FR-5"** | ◐ | `computePeripheralNumbering` (2-ring) + Phase 45's `computePeripheralNumberingChain3` (exactly 3-ring chain) — both use the same candidate-enumeration + heteroatom-locant tie-break scheme (rules a/b/f from P-25.3.3.1.2), confirmed this correctly does NOT need Phases 42-43's orientation modules (a locant-minimization problem, not a drawing/orientation one). General N-ring (4+) and interior-atom numbering (P-25.3.3.2/.3, for peri-fused/bridged systems) not covered. |
| **P-25.3.4** | Constructing polycomponent fusion names | ◐ | Phase 44: exactly 3 mutually ortho-fused monocyclic rings in a simple chain. Two sub-cases: **middle ring senior** → both neighbors first-order attached, cited together alphabetically (P-25.3.4.2.3.1), base numbering minimizes the {letterA,letterB} pair as a set (P-25.3.4.2.4 rule a) — verified against the real PIN `furo[3,2-b]thieno[2,3-e]pyridine`. **End ring senior** (Phase 46, P-25.3.4.1.1) → the other neighbor becomes a genuine second-order attached component, colon-separated numeric locants with the higher-order component's own locants primed — verified against an independently re-derived (atom-by-atom, from scratch) example, `thieno[3',4':4,5]furo[3,2-c]pyridine`; the rule text's own worked example was found to have a likely PDF-extraction artifact (missing prime marks) that was independently confirmed against other sources before implementing. Phase 45 added real indicated-hydrogen citation for the middle-ring-senior case (verified by hand-enumerating all 8 candidate numberings for a real NH-bearing molecule); indicated hydrogen for the end-ring-senior/second-order case is explicitly deferred with a clean rejection. Phase 48 extended to **4-ring chains, middle-ring-senior sub-case only**: citation combines Phase 44's plain-first-order block with Phase 46's second-order-nested-in-first-order block for the two "sides" of the base — verified against `furo[2,3-b]thieno[2',3':4,5]pyrido[2,3-e]pyrazine`, structurally checked against the same worked-example pattern used to verify Phase 46. 4-ring end-ring-senior (needing third-order/doubly-primed nesting) rejects cleanly, matching the same incremental pattern already used at the 3-ring level. Indicated hydrogen for the 4-ring case is wired in (confirmed `computePeripheralNumberingChain3` generalizes to N=4 with zero changes) but **no test exercises it — present, unverified, not claimed as working**. **Gaps**: multiparent names, identical-attached multiplying prefixes, third-order+ nesting (both 3-ring end-senior and 4-ring end-senior), branching/peri-fusion, more than 4 rings, indicated hydrogen for any second-order sub-case — all explicitly out of scope, cleanly rejected. |
| P-25.3.5 | Heteromonocyclics fused to a benzene ring | ✅ | Phase 31 (benzo-fused heterobicyclics) covers this directly. |
| P-25.3.6 | Identical attached components | ✗ | Not implemented. **Investigated 2026-08-21, confirmed genuinely substantial, not a quick fix**: two-or-more identical attached components need multiplying-prefix combination (`difuro[...]`), correct colon/comma/semicolon separator choice depending on complete-vs-abbreviated locant sets, and primed/double-primed disambiguation between the identical components (real PIN examples: `difuro[3,2-b:3,4-e]pyridine`, `dibenzo[c,e]oxepine`) — this needs real extension of item 1's N-ring citation-builder, not a bolt-on; scope it as its own brainstorm/spec/plan cycle rather than an ad-hoc delegation. |
| P-25.3.7 | Multiparent ring systems | ✗ | Not implemented. Same scale of work as P-25.3.6 above (real PIN examples like `benzo[1,2-f:4,5-g]diindole` need a genuinely different citation structure, not a variant of the existing single-parent chain builder). |
| P-25.3.8 | Omission of locants in fusion descriptors | ✗ | Not implemented — locants are always cited in full. **Investigated 2026-08-21**: the real rule (verified against `BlueBookV2.md`) only omits locants for a *hydrocarbon* attached component (`benzo`, `cyclopenta`, `cyclopropa`) or a system of exactly two monocyclic hydrocarbons — this codebase's benzo-fusion cases (Phase 31) already bypass the bracket-descriptor mechanism entirely via hardcoded retained names (`quinoline`, `indole`, `benzofuran`, etc, confirmed by grepping the test suite for `benzo[` and finding zero matches), and item 1's general N-ring chain-builder only handles *heterocyclic* attached components (furo/thieno/pyrido), which always need their locants per the same rule. Real applicability to this codebase's current architecture looks narrow-to-nonexistent without first building support for a plain-hydrocarbon attached component in the general chain builder — not attempted, since that prerequisite is itself unscoped work. |
| P-25.4 | Bridged fused ring systems | ✗ | Detection + clean rejection only, 2026-08-21 (commit `5173962`) — name construction (bridge prefixes, bridge numbering) not implemented. |
| P-25.5 | Limitations of fusion nomenclature: three components ortho- and peri-fused together | ✗ | Detection (heuristic: peri-fusion + 2+ centers or 4+ SSSR rings) + clean rejection only, 2026-08-21 (commit `5173962`) — name construction not implemented. |
| P-25.6 | Fused ring systems with skeletal atoms with nonstandard bonding numbers | ✗ | Not implemented. |
| P-25.7 | Double bonds, indicated hydrogen, and the λ-convention | ◐ | Indicated-hydrogen half covered (see P-14.7/FR-9 rows); the λ-convention (nonstandard bonding number notation) not covered at all. |
| P-26 | Phane nomenclature | ✗ | Not implemented. |
| P-27 | Fullerenes | ✗ | Not implemented, not relevant to this app's scope. |
| P-28 | Ring assemblies | ◐ | `checkRingAssemblyOneSide`: two identical monocyclic rings joined by one single bond, one exocyclic substituent each (P-28.2.1). **Generalized 2026-08-21** (commit `3227208`, hardened in `17d373f`) from biphenyl/bicyclohexyl-only to any monocyclic heterocycle `nameRingAsSubstituent` already supports (5/6-membered, curated names like pyridine/furan/thiophene AND the general Hantzsch-Widman range) — reuses that function's own attachment-locant computation rather than reimplementing ring numbering, and correctly cites explicit locants for asymmetric rings (`2,2-bipyridine`, `2,3-bifuran`, `3,3-bithiophene`, all hand-verified against the real Blue Book examples) while preserving the existing locant-omittable `biphenyl`/`bicyclohexyl` output for the still-trivially-symmetric benzene/cyclohexane case. A parent-name reconstruction step (undoing the terminal-e elision baked into the "-yl" substituent form) initially used a 15-entry whitelist covering only the curated ring names, silently missing every general Hantzsch-Widman stem (uncommon-element or 7-10-membered rings); caught in review and replaced with a default-restore-e rule excluding only the one real exception (the furan family), robust to every ring type the underlying function can produce rather than just the ones this task's own tests happened to exercise. `iupac_namer_test` 487→490/490, `ctest` 11/11. **Gap**: 3+ ring assemblies (P-28.3), double-bond-junction assemblies (P-28.2.2), nonidentical ring assemblies (P-28.7), fused/von-Baeyer/spiro ring-assembly members, and plain non-benzene/cyclohexane symmetric carbocycles (cyclopentyl-cyclopentyl etc — currently fall through uncovered since their locant-less "-yl" form doesn't match this path's locant-parsing precondition). |
| P-29.0-P-29.3 | Definitions, general methodology, simple substituent prefixes from saturated parent hydrides | ✅ | `nameBranchGraph` (acyclic -yl), `nameRingAsSubstituent` (cycloalkyl, phenyl, naphthalen-1/2-yl, heterocyclic -yl with locants). |
| P-29.4 | Compound substituent groups | ◐ | Nested substituents supported (Phase 39's whole point — arbitrary nesting depth), but not the full compound-substituent-group naming rules. |
| P-29.5 | Complex substituent groups | ◐ | **P-29.5.1 confirmed covered 2026-08-21** (no code change — Phase 39's existing arbitrary-depth substituent nesting already produces the real Blue Book PIN worked example byte-exact, `6-(3-methylbutyl)undecyl`, pinned as a regression test). **P-29.5.2 remains unimplemented**: concatenated complex substituent groups (3+ components combined via `bis`/`tris`-style multiplicative naming, e.g. `sulfanediylbis(methyleneoxy)` for `-O-CH2-S-CH2-O-`) is a genuinely different, separate mechanism from simple nesting — not attempted. |
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
| P-31, P-32, P-34 | Hydro/dehydro prefixes; replacement-derived substituent prefixes; functional parent compounds (e.g. "...acid", "...alcohol" as separate-word functional class names) | ✗ | Not implemented. **P-34's "functional class names" are confirmed low-priority/likely unneeded**: verified against `BlueBookV2.md` (P-63.1.2) that substitutive nomenclature (this codebase's existing "-ol"/"-oic acid" suffix style) generates the PIN; functional-class names ("methyl alcohol" style) are only ever an acceptable *general-nomenclature* alternative, never preferred — same category as the catechol/resorcinol/hydroquinone non-gap documented elsewhere in this file. **P-31 hydro prefixes investigated 2026-08-21, found genuinely more complex than a quick fix, not attempted**: for a monocyclic Hantzsch-Widman ring partially saturated relative to its mancude parent (e.g. `1,2-dihydropyridine`), two real correctness problems surfaced during investigation, not just missing plumbing: (1) for ring sizes 5/6 with a *curated retained-name* mancude parent (pyridine, pyrrole, furan, thiophene — all handled by their own dedicated `RingType`, a separate code path from the generic Hantzsch-Widman `GENERAL_HETEROCYCLE` mechanism), hydro-prefixing must combine with that parent's own retained name ("dihydropyridine"), not a re-derived generic Hantzsch-Widman reconstruction — routing through `GENERAL_HETEROCYCLE` (as a first attempt tried) would produce the wrong base name for exactly this class of ring, the most common one; (2) the hydro-locants (which ring positions are saturated relative to the mancude ideal) need to participate in the *same* lowest-locant-set candidate selection that already exists for numbering, not be computed after a numbering candidate has already won on other criteria alone — otherwise the wrong ring-numbering direction can be selected, giving non-minimal hydro locants. Two delegated attempts at a general version and a narrowed version were both interrupted (a background-process issue independently confirmed as an agy usage-quota exhaustion, not a fault in either attempt itself) before reaching a complete, verified state; the surviving partial work was reviewed directly, found to have these unresolved correctness gaps beyond what was left to finish, and reverted rather than patched further. A real fix needs its own scoped brief that: threads hydro-locant minimization into the existing numbering-candidate comparator, and either extends the curated retained-name ring types (pyridine, pyrrole, furan, thiophene, etc) with their own hydro-prefix handling directly, or establishes a principled way to detect "this ring's mancude form has a retained name" before falling back to the generic Hantzsch-Widman reconstruction. |

## Chapter P-4 — Rules for Name Construction

Real subsections: P-40 Introduction, **P-41 Seniority order for classes**,
**P-42 Seniority order for acids**, **P-43 Seniority order for suffixes**,
**P-44 Seniority order for parent structures**, P-45 Selection of the
preferred IUPAC name, P-46 The principal chain in substituent groups.

| Section | Title | Status | Note |
|---|---|---|---|
| P-41 | Seniority order for classes | ◐ | `GroupType` enum + `groupRank`: sulfonic > sulfinic > carboxylic > phosphonic > boronic > ester > acyl halide > amide > nitrile > aldehyde > thial > ketone > thione > alcohol > thiol > selenol > tellurol > hydroperoxide > amine > imine > phosphine. Not the full official class list (e.g. no radicals/anions/cations classes, since those are rejected earlier anyway). |
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
| P-62.3 | Imines | ◐ | `GroupType::IMINE`, suffix `-imine`/`-diimine`, prefix `imino-`, ranked per official P-41 numbered class list (19 amines > 20 imines) immediately below AMINE. Only the unsubstituted C=NH case is supported; N-substituted imines and other C=N-X forms (oximes, hydrazones, amidines) are explicitly rejected as out of scope. Had the same rootless-carbon bug as the P-65.2 carbonic-acid family (fifth variant of the fix, after `347d241`/`530dbbe`/`286a91f`/`2af679d`) — as of 2026-08-21 (commit `103f536`) carbonimidic/carbamimidic acid halides reject cleanly instead of the prior bug: `ClC(Cl)=N` and `NC(Cl)=N` were both wrongly named `methanimine`, silently dropping every real substituent. Real methanimine (`C=N`, H2C=NH) correctly still names as `methanimine`, preserved by the same `totalH==0` gate used throughout this fix family. Tests 523->525, ctest 11/11. |
| P-63.1 | Hydroxy compounds (alcohols) and chalcogen analogues | ◐ | `GroupType::ALCOHOL` (suffix `-ol`, prefix `hydroxy-`), `GroupType::THIOL` (suffix `-thiol`, prefix `sulfanyl-`), `GroupType::SELENOL` (suffix `-selenol`/`diselenol`, prefix `selanyl-`), and `GroupType::TELLUROL` (suffix `-tellurol`/`ditellurol`, prefix `tellanyl-`) implemented per P-63.1.5, ranked O > S > Se > Te seniority order between ALCOHOL and HYDROPEROXIDE. |
| P-63.2 | Ethers and chalcogen analogues | ◐ | Ether oxygen always handled as `-oxy` prefix (correct — ethers have no suffix form); thioether (`thioetherSulfurs`) present; selenoether and telluroether substituent prefixes (`selanyl`/`tellanyl`, via `selenoetherSeleniums`/`telluroetherTelluriums`) implemented per P-63.2.2.1.2, mirroring the existing thioether mechanism. |
| P-63.6 | Sulfoxides and sulfones | ◐ | `sulfoxideSulfurs`/`sulfoneSulfurs` handled as substituent prefixes when losing to a higher-priority principal group. As of 2026-08-22 (commit `41904f4`) the acyclic prefix form was corrected from the non-PIN `alkylsulfinyl`/`alkylsulfonyl` style to the PIN-preferred acid-stem form: `CS(=O)C` (DMSO) was wrongly `methylsulfinylmethane`, now `methanesulfinylmethane`, matching real PIN examples confirmed `1-(ethanesulfinyl)butane (PIN)`, `(ethanesulfonyl)ethane (PIN)` — the Blue Book explicitly notes "Multiplication of acyclic hydrocarbons is not permitted", so the substitutive acid-stem form (not the multiplicative form used for aromatic cases like `1,1'-sulfinyldibenzene (PIN)`) is correct here. Existing tests asserting the old wrong form were corrected, not just superseded. As of the same day (commit `4d47bb6`), ring-attached sulfoxide/sulfone (the sulfinyl/sulfonyl sulfur itself bonded to a ring atom) is also now covered — this had been entirely unimplemented in 2 of the 3 duplicated ring-substituent regions (only the acyclic region had it, mirroring the already-fixed pattern of `thioetherSulfurs`, which was correctly wired into all 3): `O=S(c1ccccc1)C` (phenyl methyl sulfoxide) previously rejected outright ("Unrecognized or unsupported substituent on ring"), now names `methanesulfinylbenzene`. Aromatic/cyclic R groups on the LOSING side of a sulfoxide/sulfone (as opposed to the ring the sulfur itself sits on) still fall back to the plain aryl-name form (e.g. `phenylsulfinyl`), which the real text lists as the valid general-nomenclature alternative rather than the PIN for that case — a known remaining gap, left for a future pass. Tests 537->539->539 (2 stale "must reject" tests correctly replaced with 2 real success-case tests), ctest 11/11 throughout. |
| P-63.3-P-63.7 | Peroxides, hydroperoxides, cyclic ethers/sulfides as ring parents, sulfoxides/sulfones, polyfunctional compounds | ◐ | Hydroperoxides (peroxols, P-63.3) supported via the same numbered-chain-suffix machinery as ALCOHOL, with suffix "peroxol"/"diperoxol" and prefix "hydroperoxy"; ranked between THIOL and AMINE per official P-41 seniority-class list (17 hydroxy > 18 hydroperoxide > 19 amine). Dialkyl peroxides (R-O-O-R') explicitly rejected as out of scope. **Phase 68** added selenoxide/selenone and telluroxide/tellurone substituent prefixes (`selenoxideSeleniums`/`selenoneSeleniums`/`telluroxideTelluriums`/`telluroneTelluriums`) per P-35.3/P-65.3.2.3, mirroring the existing sulfinyl/sulfonyl mechanism, with the same scope limitation (pure-acyclic path only, not yet wired into the two ring-vs-chain blocks — this is a pre-existing limitation shared with sulfinyl/sulfonyl, not new to this phase). **Phase 69** added diselanyl/ditellanyl substituent prefixes (`diselenideSeleniums`/`ditellurideTelluriums`) per P-63.2.5/P-68.4, mirroring the existing disulfanyl mechanism, homo-chalcogen only (no mixed S/Se/Te chains), same pure-acyclic-only scope limitation. |
| P-64.2 | Ketones | ✅ | `GroupType::KETONE`, suffix `-one`, prefix `oxo-`. |
| P-64.3, P-64.4 | Pseudoketones, heterones | ◐ | **P-64.3.1 investigated 2026-08-21**: cyclic anhydrides, esters, and amides are named as "pseudoketones" (ring-ketone-style `-dione`/`-one` suffixes) per the real rule text — this maps directly onto machinery already fixed elsewhere this session. Lactams (e.g. `pyrrolidin-2-one`, P-64.3.1's own worked example) already worked correctly via the saturated-heterocycle-ring-parent fix, confirmed and pinned as a regression test, no code change needed. Cyclic anhydrides (e.g. `tetrahydrofuran-2,5-dione`) were a real, confirmed bug — fixed, see the P-65.7 row above. **`azepan-2-one`-style larger-ring (7-10 membered) saturated lactams fixed 2026-08-21** (commit `c9ac89c`): `RingType::GENERAL_HETEROCYCLE` (Hantzsch-Widman) now produces the fully-saturated "-ane"-family form (`azepane`, `azocane`, `azonane`, `azecane`) for a single-heteroatom ring of size 7-10, not just the pre-existing mancude/aromatic form — `hwGeneralRingStem` gained a `saturated` flag, the mancude-only indicated-hydrogen check is skipped entirely for the fully-saturated case (it was misfiring, since every position of a saturated ring looks "saturated" to a detector designed to find the one saturated position amid an otherwise-maximally-unsaturated ring), and the sole-heteroatom locant is omitted matching every other saturated ring-parent path in this file (`azepan-2-one`, not `1-azepan-2-one`). Verified: `azepane`, `azepan-2-one` (P-64.3.1's own worked example), and `azocane` (an existing test whose own comment had mislabeled its 8-membered-ring SMILES as "7-membered"). Ring sizes 3-6 unaffected (already separately handled — 5/6 via the 4 curated types, 3/4 not attempted); the existing mancude path for 7-10 confirmed unaffected. `iupac_namer_test` 497→499/499, `ctest` 11/11. P-64.3.2 (acyclic pseudoketones/hidden amides) and P-64.4 (heterones) not investigated. |
| P-65.1 | Carboxylic acids and functional replacement analogues | ◐ | `GroupType::ACID`, `-oic acid`, via `nameAcidChainFrom`. Functional replacement analogues (thio-, seleno-, telluro- acids) not covered. P-65.1.4 peroxycarboxylic acids (real PIN suffix `...peroxoic acid`/`carboperoxoic acid`, e.g. `ethaneperoxoic acid (PIN)` for peracetic acid) not named — `GroupType::ACID` is wired into 21 call sites (incl. anhydride/ester formation), so implementing the full parallel suffix class was scoped out as too large for one pass; as of 2026-08-21 (commit `ae282cc`) the `-C(=O)-O-OH` pattern is at least cleanly rejected (`isPeroxyCarboxylicAcid`) instead of the prior silent bug — peracetic acid (`CC(=O)OO`) was wrongly named `1-hydroxy-1-oxoethane`. Tests 510->512, ctest 11/11. As of 2026-08-22, carbamic acid (`H2N-CO-OH`) and carbonic acid (`HO-CO-OH`) — real PIN examples confirmed, `carbamic acid (PIN)`, `carbonic acid (PIN)` — are also correctly named (commit `1a7bbb2`), fixing a newly-found severe bug of the same rootless-carbon family as `347d241`: both were silently misnamed `methanoic acid` (formic acid), the amino group vanishing without trace for carbamic acid, since the rootless-carbon guard `347d241` added to ACYL_HALIDE/AMIDE/ESTER was never extended to the plain ACID branch. Their simple, unsubstituted esters (`methyl carbamate`, `dimethyl carbonate`, `ethyl methyl carbonate`) are named too — real PIN citations confirmed, e.g. the asymmetric-carbonate pattern via `18O-ethyl O-methyl (18O1)carbonate (PIN)`. Implemented as an early-return functional-class construction at the existing rootless-carbon detection points, deliberately bypassing the general `GroupType`/seniority/suffix machinery since carbon acids are their own P-41 seniority class (7b), not chain-based-acid participants. N-substituted carbamic acid/carbamate (a distinct, unique substituent-fusion convention with no "N-" locant) explicitly deferred. Independent verification caught the delegate's partial work using the wrong naming helper for ester alkyl groups (`nameAcyclicChainParentWithSubstituents`, a parent-hydride name like "methane") instead of this codebase's own established substituent "-yl" helper (`nameBranchGraph`, "methyl") already used elsewhere for ester alcohol parts — producing `methane carbamate`/`dimethane carbonate` instead of the correct forms; fixed across all 9 call sites before accepting. Tests 543->547, ctest 11/11. |
| P-65.3 | Sulfur, selenium, and tellurium acids with chalcogen atoms directly linked | ◐ | `GroupType::SULFONIC_ACID`/`SULFINIC_ACID` cover the acid forms. As of 2026-08-22, `GroupType::SULFONYL_HALIDE` covers the halide-of-sulfonic-acid form too (commit `7a9fe5b`, mirroring `PHOSPHONIC_ACID`/`ARSONIC_ACID`'s implementation pattern across detection, classification, seniority, rank, and suffix assembly): real PIN examples confirmed, `ethanesulfonyl chloride (PIN)`. Sulfonyl pseudohalides and selenium/tellurium chalcogen analogues explicitly out of scope, left for a future pass. Review also caught and fixed a latent pre-existing rank-collision bug (`PHOSPHINE` and the `default` case both mapped to the same `groupRank` value) as a minimal adjacent fix. Tests 525->528, ctest 11/11. As of 2026-08-22, `GroupType::SULFINYL_HALIDE` (commit `ad965f2`) covers the halide-of-sulfinic-acid form too: real PIN example confirmed, `benzenesulfinyl chloride (PIN)`. Review caught (not self-reported) and fixed harmless-but-dead duplicate code the dispatch left behind — a redundant `SULFINYL_HALIDE` OR-clause duplicated in two condition lists, and an entire duplicated (unreachable) suffix-assembly else-if block — commit `5b25d26`. Tests 534->537, ctest 11/11 throughout. As of 2026-08-22, `GroupType::SULFONAMIDE` (commit `7e59c3c`) covers the unsubstituted amide-of-sulfonic-acid form too, a genuinely common functional group (medicinal chemistry "sulfa" groups) that was entirely unimplemented: real PIN example confirmed, `CH3-SO2-NH2 methanesulfonamide (PIN)`. Scoped to unsubstituted `-SO2-NH2` only, mirroring `GroupType::AMIDE`'s own existing scope (N-substituted sulfonamides, sulfinamide, sulfonohydrazide all explicitly out of scope). Substituent-prefix form `sulfamoyl` confirmed against real text ("the acyl group 'sulfamoyl' (for sulfonamides only)"). Tests 539->541, ctest 11/11. As of 2026-08-22, `GroupType::SULFINAMIDE` (commit `868ff57`) mirrors this same addition for sulfinic acid: real PIN example confirmed, `butane-2-sulfinamide (PIN)`. Substituent-prefix form `sulfinamoyl` chosen by direct analogy (no distinct real-text citation found for that specific word). Tests 541->543, ctest 11/11. This P-65.3/P-66.1.1.2 sulfonic/sulfinic-acid-derivative family (acid, amide, halide, ×2 chalcogen tiers) is now essentially complete for the unsubstituted/mononuclear cases. |
| P-65.4 | Acyl groups as substituent groups | ◐ | Present for the acid/amide/ester code paths that need it; not a general standalone acyl-substituent mechanism. |
| P-65.5 | Acyl halides and pseudohalides | ◐ | `GroupType::ACYL_HALIDE` covers halides (Cl/Br/F/I). Acyl pseudohalides (azide N3, cyanide CN, isocyanate NCO — P-65.5.2, real PIN examples confirmed: `butanoyl cyanide (PIN)`, `oxalyl diisothiocyanate (PIN)`) are NOT named (that functional-class-nomenclature construction, separate words with its own multiplicative-prefix and pseudohalide-seniority rules, is a distinct larger feature, still out of scope) — but as of 2026-08-21 they are cleanly rejected (`isAcylPseudohalide`/`isIsocyanateNitrogen`, commits `349c26d` + `0e0800f`) instead of the prior silent misnaming bug: `CCCC(=O)N=[N+]=[N-]` (butanoyl azide) was wrongly named `1-azidobutan-1-one`, `CCCC(=O)C#N` (butanoyl cyanide) as `2-oxopentanenitrile`, `CCCC(=O)N=C=O` (an acyl isocyanate) as `1-isocyanatobutan-1-one`. The first commit's own self-report overclaimed fixing all 3 duplicated classification regions; independent review found 2 of 3 still silently misclassified acyl isocyanates as AMIDE (a precomputed `carbonIsocyanate` map local to only one region, passed as an empty temporary elsewhere) — the second commit replaced it with a self-contained structural check and added regression tests for exactly the 2 previously-broken regions. Tests 505->508->510, ctest 11/11 throughout. Separately, P-65.5.3 (acyl halides of carbonic/carbamic acid — "rootless" acyl carbons with no carbon substituent, plus mixed-dihalide acyl halides) had a more severe silent-data-loss bug, real PIN examples confirmed: `Cl-CO-Cl carbonyl dichloride (PIN)`, `Br-CO-Cl carbonyl bromide chloride (PIN)`. As of 2026-08-21 (commit `347d241`) these are cleanly rejected instead of the prior bug, which didn't just garble the name but silently DROPPED a whole halogen atom with no trace in the output: `ClC(=O)Br` (carbonyl bromide chloride) was wrongly named `methanoyl chloride` (bromine vanished), `NC(=O)Cl` (carbamoyl chloride) as `methanamide` (chlorine vanished). The retained acyl-group names (`carbonyl`, `carbamoyl`) themselves are still not constructed — still out of scope. Tests 512->515, ctest 11/11. The same rootless-carbon disease also lived in the P-64/P-66 sulfur analogues, `GroupType::THIAL`/`GroupType::THIONE` (carbonothioyl/thiocarbamoyl halides) — as of 2026-08-21 (commit `286a91f`, tests 518->521) these reject cleanly too: `ClC(=S)Cl` (thiophosgene) was wrongly named `methane-1-thione`, `NC(=S)Cl` (thiocarbamoyl chloride) as `methane-1-thione` with both the amino group and chlorine silently vanished. (The ESTER-path variant of this same bug, carbonic-acid esters like dimethyl carbonate, is documented separately under the P-65.6 row — commit `530dbbe`.) ctest 11/11 throughout. |
| P-65.6 | Salts and esters | ◐ | Esters covered (`GroupType::ESTER`, `esterAlkylRoot`/`esterOxygen`); salts not covered at all (charged species rejected early). Esters of carbonic acid (P-65.2, real PIN examples: carbonic acid `HO-CO-OH` has no carbon substituent at all) had the same rootless-acyl-carbon bug as the P-65.5.3 acyl-halide case above — as of 2026-08-21 (commit `530dbbe`, direct continuation of `347d241` into the ESTER classification path) these reject cleanly instead of the prior severe bug: `COC(=O)OC` (dimethyl carbonate) was wrongly named `methyl methanoate` (invented a phantom H, silently dropped the second methoxy group), `O=C(OC)OCC` (methyl ethyl carbonate) as `ethyl methanoate` (methoxy group vanished entirely), `COC(=O)Cl` (methyl chloroformate) as `methyl methanoate` (chlorine vanished entirely, wrong functional group). Formate esters (`COC=O` → `methyl methanoate`, a real, distinct, already-correct case) were carefully preserved by gating the rootless check on `totalH == 0` too, since a formate's acyl carbon has an H and is a genuine ester, not a carbonic-acid derivative. Real carbonic-acid-ester naming (`dimethyl carbonate`, etc.) itself is now constructed too — see the P-65.1 row (commit `1a7bbb2`, 2026-08-22) for the full writeup, which also covers carbamic acid esters (`methyl carbamate`) and fixes a newly-found severe bug where carbamic/carbonic acid themselves were silently misnamed `methanoic acid`. Tests 515->518, ctest 11/11. |
| P-65.7 | Anhydrides and their analogues | ◐ | Symmetric/single-chain anhydride naming via `nameAcidChainFrom`; true two-parent-name anhydride construction not covered. **Real bug fixed 2026-08-21** (commit `4c8e323`, verification hygiene fix in `e5fa90d`): a *cyclic* anhydride (e.g. succinic anhydride) previously produced a completely garbled, wrong ESTER-shaped name (`(2,5-dihydroxytetrahydrofuran-2-yl) 2,5-dioxotetrahydrofuran-2,5-dicarboxylate`) instead of the correct name or a clean rejection — the acyclic anhydride-detection code's "walk outward from the bridging oxygen until a dead-end" logic implicitly assumed a tree shape and had no way to terminate correctly on a ring, silently falling through to plain ester classification instead. Fixed narrowly: a ring-internal C-O bond is no longer collected as an anhydride/ester candidate at all, so a cyclic anhydride's bridging oxygen correctly falls through to the (already-working, separately-fixed) saturated-ring-parent ketone/dione path instead — verified: `tetrahydrofuran-2,5-dione` for succinic anhydride, matching P-64.3.1's real worked example (`oxolane-2,5-dione`, an older/alternative name for the same ring this codebase already consistently calls `tetrahydrofuran` elsewhere). **Caught during review, not self-reported**: the delegated fix's own test run left a stray debug filter in the main test loop that silently skipped every other test in the suite, making its "all tests pass" claim meaningless — found and removed before landing, full suite re-verified for real (496→497/497, all 9 pre-existing acyclic anhydride tests confirmed still byte-identical). |
| P-66.1 | Amides | ✅ | `GroupType::AMIDE`, suffix `carboxamide`/`-amide`. |
| P-66.2, P-66.3, P-66.4 | Imides, hydrazides, amidines/amidrazones/hydrazidines/amidoximes | ◐ | **P-66.2 cyclic imides fixed 2026-08-21** (commit `309d411`) — verified against `BlueBookV2.md`: cyclic imides are PIN-named as "heterocyclic pseudoketones" (succinimide's real PIN is `pyrrolidine-2,5-dione`, not a distinct imide suffix), so this needed no new functional-group class at all, only unblocking a separate underlying limitation (see the P-22.2.2/heterocycle row below). Acyclic imides (P-66.2.1's other branch, N-acyl derivatives of primary amides) not attempted.

   **P-66.3.1 substitutive hydrazides fixed 2026-08-21** (commit `4c72b1b`): new `GroupType::HYDRAZIDE`
   (the `-CO-NH-NH2` group, structurally amide's `-CO-NH2` with the nitrogen bearing a further
   `-NH2`) mirroring `GroupType::AMIDE`'s implementation at all 15 call sites this file duplicates
   functional-group logic across (detection, `groupRank`/`seniorityOrder` insertion — verified
   against the real Blue Book suffix seniority table, which places hydrazide directly between
   amide and nitrile — suffix assembly for both the acyclic `hydrazide`/`dihydrazide` and
   ring-attached `carbohydrazide`/`dicarbohydrazide` forms, matching amide's own acyclic/ring
   split exactly). Verified against real PINs `pentanehydrazide` and `cyclohexanecarbohydrazide`.
   **A real bug was caught during review, not self-reported**: the delegated commit's own test for
   a molecule with both a plain amide and a non-principal hydrazide (amide correctly wins
   seniority) asserted `5-amino-5-oxopentanamide` as correct — but that name only accounts for 2
   of the molecule's 3 nitrogens; the demoted hydrazide's outer nitrogen was being silently
   dropped, because the generic "amino" substituent fallback this codebase uses when a
   non-principal amide/hydrazide nitrogen doesn't match the specific N-acyl "amido" pattern never
   checks whether that nitrogen has a further heavy-atom substituent of its own before calling it
   a plain, unsubstituted amino group. Fixed (commit `4c0a2c9`) by rejecting cleanly in that case
   instead of silently producing an incomplete name; the wrong test was corrected to assert the
   rejection. **This same latent gap exists, undisturbed, at 6 other `"amino"`-substituent call
   sites in this file** (ring-substituent and branch-graph naming contexts) — pre-existing, not
   introduced by this task, not chased down since none are demonstrably reachable by anything this
   task covers; a real, disclosed, unfixed issue for whichever future task first exercises one of
   those paths with a substituted (non-terminal) amine. `iupac_namer_test` 492→495/495, `ctest`
   11/11. **Not attempted**: P-66.3.1.2 (the ~5 retained-name hydrazides like `benzohydrazide`),
   P-66.3.3-P-66.3.6 (substituted/chalcogen/carbonic-acid hydrazides, semioxamazones), and P-66.4
   (amidines/amidrazones/hydrazidines/amidoximes — separate, unrelated functional groups). |
| P-66.5 | Nitriles | ◐ | `GroupType::NITRILE`, suffix `-nitrile`/`carbonitrile`, prefix `cyano-`. Had the same rootless-carbon bug as the P-65.5/P-65.6 family (P-65.2 cyanic-acid-halide derivatives, e.g. cyanogen chloride/bromide) — as of 2026-08-21 (commit `2af679d`, fourth variant of the fix after `347d241`/`530dbbe`/`286a91f`) these reject cleanly instead of the prior bug: `ClC#N` and `N#CBr` were both wrongly named `methanenitrile` (real HCN), silently dropping the halogen. Real HCN itself (`totalH==1` on that carbon) correctly still names as `methanenitrile`, preserved by the same `totalH==0` gate used throughout this fix family. Tests 521->523, ctest 11/11. |
| P-66.6 | Aldehydes | ✅ | `GroupType::ALDEHYDE`, suffix `-al`/`carbaldehyde`, prefix `oxo-`/`formyl-`. |
| P-67.1 | Mononuclear noncarbon oxoacids | ◐ | Sulfonic acid (`GroupType::SULFONIC_ACID`), sulfinic acid (`GroupType::SULFINIC_ACID`), boronic acid (`GroupType::BORONIC_ACID`), phosphonic acid (`GroupType::PHOSPHONIC_ACID`), and arsonic acid (`GroupType::ARSONIC_ACID`, commit `03e1296`) covered via the same numbered-chain-suffix machinery. Arsonic acid (`-As(=O)(OH)2`) mirrors phosphonic acid's implementation at every site (detection, classification, exclusion checks, seniority, rank, suffix assembly, ring-substituent path); real Blue Book PIN example confirmed: `(4-acetamido-3-methylphenyl)arsonic acid (PIN)`. Elision applies since "arsonic" starts with a vowel (P-15.1) — `methanarsonic acid`, `benzenarsonic acid` — unlike phosphonic acid, which never elides since "phosphonic" starts with a consonant; verified against real Blue Book elision rule text. Tests 499→505 (+5 naming, +1 rejection), ctest 11/11. Still not covered: arsinic acid (two-carbon-substituent analogue of phosphinic acid), arsorous/arsinous acid (lower oxidation state), di-/polynuclear arsonic acids, and antimony/other-element analogues (stibonic acid, etc.) — all explicitly out of scope for this addition. As of 2026-08-22, the halide-of-phosphonic-acid form is also covered: `GroupType::PHOSPHONIC_DIHALIDE` (commit `f8a30af`, mirroring `SULFONYL_HALIDE`'s just-landed implementation pattern), real Blue Book PIN example confirmed `C6H5-P(O)Cl2 phenylphosphonic dichloride (PIN)` (this codebase's own established stem-fusion convention produces `benzenephosphonic dichloride` instead of `phenylphosphonic dichloride`, consistent with how `PHOSPHONIC_ACID` itself already fuses onto `benzene` rather than the retained `phenyl` prefix everywhere else in this codebase). Scoped to symmetric dihalides only (both halogens identical); mixed acid-halides and mixed-halogen dihalides explicitly out of scope. Tests 528->531, ctest 11/11. As of 2026-08-22, `GroupType::ARSONIC_DIHALIDE` (commit `02cbdcc`) mirrors this same addition for arsonic acid — `methanarsonic dichloride`, `benzenarsonic dichloride`, `ethanarsonic dibromide`, all correctly eliding per the same established vowel-elision convention as `ARSONIC_ACID` itself. Tests 531->534, ctest 11/11. |
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
| P-92.6 | Sequence Rule 5 (R precedes S, etc. — the rule that resolves pseudoasymmetry) | ◐ | Pseudoasymmetric centers (lowercase r/s) now accepted by reusing Indigo's own `indigoStereocenterCIPDescriptor` result (previously computed correctly but rejected outright) — see `IupacNamer.cpp`'s stereocenter loop. Still ◐, not ✅: this reuses Indigo's answer rather than this codebase independently implementing Sequence Rule 5's comparison logic itself. |
| P-93.4 | Configuration specification of acyclic organic compounds (covers both R/S and E/Z for acyclic systems) | ◐ | R/S via Indigo CIP (see above). E/Z via `processDoubleBondStereo`, now backed by Indigo's own per-bond CIP descriptor (read from the KET JSON's inline `"cip"` field on each bond) instead of a hand-rolled first-shell-only comparator — correctly handles 2nd-shell tie-breaks and any substitution count. Still ◐, not ✅: cumulated double bonds (allenes) remain rejected, and ring double bonds / substituent-branch double bonds are still out of scope pending the separate ring/branch-locant sub-project. **Newly-discovered pre-existing limitation** (found during live verification of this fix, not introduced by it — confirmed via a git-checkout control test against the pre-fix code): E/Z determination, in both the old and new code, is derived from whatever 2D coordinates currently exist on the molecule at naming time, not from whether the double bond's geometry was ever deliberately specified. `generateName()` unconditionally calls `indigoLayout(mol)`; for a molecule freshly parsed straight from a SMILES with no stereo bond markers, this happens to produce coordinates Indigo reads as non-stereogenic (correct) — but the live app's actual "Load from SMILES" flow first runs the SMILES through a separate `indigoSvc.layout()` call to get a molfile for the document, and if that molfile is later re-loaded and re-laid-out for naming, the *already-existing* coordinates are preserved rather than freshly (and more symmetrically) recomputed, so a double bond the user never specified stereo for can pick up an arbitrary, non-reproducible E or Z label. This is NOT new: the pre-fix code had the identical vulnerability for any simple disubstituted alkene without explicit stereo marks (it only special-cased *ties* with an early, coordinate-independent rejection); this fix just means ties now share that same pre-existing behavior instead of always rejecting. A real fix needs a way to track "was this bond's stereo ever deliberately specified" through the SMILES→document→molfile round trip (molfiles have no such flag for double bonds outside the rare `stereo=3` "either" marker) — out of scope for this sub-project, flagged here for a future one. |
| P-93.5, P-93.6 | Configuration specification of cyclic compounds; compounds composed of rings and chains | ◐ | Ring-parent atom stereocenters already worked (ring atoms were already in `graphIdToLocant` for the monocyclic/naphthalene paths). A stereocenter in a substituent branch attached to a ring parent (monocyclic or naphthalene) is named correctly via `formatBranchStereoPrefix`. **A ring used AS a substituent with a stereocenter on its own atom is also now covered (2026-08-19)**: `nameRingAsSubstituent`'s own winning ring numbering (`best.ringChain`) now builds a `"(nR)-"` prefix directly, threaded through 4 call sites (`nameBranchGraph`, `nameChainParentWithRingSubstituent` x2, the `ringSubstituentInfos` pre-collection pass) — e.g. `5-[(2R)-2-methylcyclopentyl]heptanoic acid`. Remaining gaps: von Baeyer bicyclic (Phase 27) and spiro (Phase 28) parents have no stereo support at all yet, not even for their own ring-parent atoms; allenes. |
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
| FR-7 | Three components ortho- and peri-fused together | ✗ | ≈ Blue Book P-25.5. Detection + clean rejection only, 2026-08-21 (commit `5173962`) — name construction not implemented. |
| FR-8 | Bridged fused ring systems | ✗ | ≈ Blue Book P-25.4. Detection + clean rejection only, 2026-08-21 (commit `5173962`) — name construction not implemented. |
| FR-9 | Indicated hydrogen | ◐ | ≈ Blue Book P-14.7 and P-25.7. `preferIndicatedHydrogenLocant` branch in `computePeripheralNumbering` + Phase 35's `nH-` prefix for NH-bearing fused systems (pyrrole/imidazole/pyrazole). **Phase 60** extended to general monocyclic heterocycles (GENERAL_HETEROCYCLE path): `<locant>H-` prefix citation for saturated positions in odd-membered rings. General indicated-hydrogen for non-fusion, non-general-heterocycle cases not covered. |
| Appendix 1/2 | Full seniority tables (hydrocarbon/heterocyclic components) | ◐ | Phase 47 added Se and Te; Phase 49 added P (phosphinine only — phosphole was scoped out as not reliably aromatic). N,O,S,Se,Te,P now covered, correctly ordered in both the primary rule-(a) order and the alternate rule-(f) order. Remaining elements (As,Sb,Bi,Si,Ge,Sn,Pb,B,Al,Ga,In,Tl) not implemented — most also have nonstandard bonding numbers requiring the λ-convention (P-25.3.2.5.2/P-14.1.3), a separate unimplemented piece. |

---

## Explicitly-documented gaps (call these out honestly wherever hit, don't paper over)

- **P-44 ring-vs-chain seniority — partially covered as of Phase 63**: the P-44.1.1 instance-count decision and the P-44.1.2.2 ring-wins-on-genuine-tie default are implemented, and the chain-wins case is now named for `ACID`, `AMIDE`, `NITRILE`, `ALDEHYDE`, `KETONE`, `ALCOHOL`, `THIOL`, `AMINE`, `THIAL`, `THIONE`, `SULFONIC_ACID`, `SULFINIC_ACID`, `ESTER`, `ACYL_HALIDE` (via `nameAcyclicChainParentWithSubstituents`), `PHOSPHONIC_ACID`, and now `BORONIC_ACID`/`PHOSPHINE` too (via the pure-acyclic path's own `nameBranchGraph`-based whole-branch naming, reused rather than duplicated). Still gaps: `P-44.1.2` heteroatom-skeleton seniority (e.g. Si chain vs C ring) is not implemented; chain-wins still requires a single ring<->chain attachment and a 5/6-membered monocyclic ring (a polysubstituted naphthalene + acid chain still rejects; fused/bridged/polycyclic-ring-as-substituent is a separate future phase).
- P-92.6 (pseudoasymmetric stereocenters) and P-93.4's 2nd-shell/trisubstituted E/Z ties are now handled via Indigo's own CIP engine (see the P-92.6/P-93.4 rows above). Substituent-branch stereocenters and allenes remain rejected.
- **E/Z coordinate-dependence (found during P-93.4's CIP-reuse fix, pre-existing in the code before that fix too)**: `processDoubleBondStereo` determines E/Z purely from whatever 2D coordinates exist on the molecule when `generateName()` runs, with no way to tell "the user deliberately drew this geometry" from "a layout algorithm had to put these atoms somewhere." A double bond with genuinely unspecified stereo can receive an arbitrary, non-reproducible E/Z label if it reaches naming after already having real (non-collinear) coordinates from an earlier, unrelated layout pass — confirmed live via the app's own "Load from SMILES" → auto-layout → "Generate IUPAC Name" flow for `CC=C(C)CC` (3-methylpent-2-ene with no stereo bonds), which produced `(2E)-3-methylpent-2-ene` live despite the direct unit-test path (parsing the bare SMILES fresh, no prior layout) correctly producing the unprefixed `3-methylpent-2-ene`. Confirmed via a git-checkout control test that this exact vulnerability class predates the P-93.4 CIP-reuse fix (the old code had it too for any non-tie disubstituted alkene without explicit stereo marks — it only specifically protected *tied* substituents with an early, coordinate-independent rejection). A real fix needs to track "was this bond's stereo ever deliberately specified" across the SMILES→document→molfile round trip; out of scope here, needs its own future sub-project.
- Multi-component (disconnected) molecules rejected outright (P-72/P-73/P-77 charged/salt forms out of scope entirely).
- 3+ SSSR rings rejected unless all mutually disjoint (no fused/bridged/spiro combination beyond the specific phases listed above; P-25.4/P-25.5/P-25.6 all out of scope).
- **Phase 59**: `CC1CCCCC1CCC(=O)Oc1ccccc1` (phenyl ester of 3-(2-methylcyclohexyl)propanoic acid — a molecule with 2 separate, non-fused monocyclic rings joined only by an acyclic ester linkage) now correctly produces `success=true, name="phenyl 3-(2-methylcyclohexyl)propanoate"`. Fix: added early guard at start of `ringCount == 2` dispatch sequence (before biphenyl detection at line 3004) that detects when the two SSSR rings are fully disjoint (no shared atoms and no direct bond between them) and marks `twoRingsAreDisjoint=true`; then modified ring-substituent detection condition (line 3173) from `mainChainExoCount == 1` to `mainChainExoCount == 1 || twoRingsAreDisjoint` to allow both disjoint rings to be treated as ring substituents and funneled into the P-44 acyclic path, where Phase 58's ester logic correctly names both the phenyl ester alkyl group and the methylcyclohexyl-substituted acid chain.

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

1. **Third-order nesting (doubly-primed locants) and genuine N-ring generalization — fixed
   2026-08-21.** The hand-written, per-ring-count (exactly-3, exactly-4) Phase 44/46/48
   blocks (~1040 lines) were replaced with one genuinely N-ring-generic algorithm (a
   ring-adjacency graph, all-candidate-root tree evaluation per P-25.3.4.2.1, and a
   recursive citation-tree builder implementing P-25.3.4.2.4's full (a)-(h) locant-
   seniority cascade). Simple linear fusion chains of any length are now supported,
   verified with new 5-ring and 6-ring test cases (the 6-ring case requires genuine
   doubly-primed third-order citation, confirmed working). The previously-rejected 4-ring
   "end ring is senior" case (`"End ring as base in a 4-ring fusion chain requires
   third-order attached components, which are not supported."`) now succeeds with a real
   computed name.

   Delegated to an external agent (agy/Gemini), independently verified and iterated
   through 4 rounds after real problems were found: (1) a candidate-root selection bug
   that filtered roots by seniority *before* evaluating tree depth, contradicting
   P-25.3.4.2.1(b)'s actual requirement that root choice be scored by depth — fixed to
   evaluate all candidates' full trees first; (2) a hardcoded string-matching special
   case (`if (finalName.contains("pyrido") && ...)`) papering over a locant-ordering bug
   instead of fixing it — removed once the real bug (locants were being
   min/max-normalized into a set, destroying the sequence P-25.3.4.2.4(d) requires
   comparing "in order of citation") was found and fixed; (3) a missing empty-string
   guard on the final constructed name — added. One test value (`o1ccc2nc3ccsc3nc12`)
   was flagged as a possible regression against this project's own prior Phase 44 output
   (`furo[3,2-b]thieno[2,3-e]pyrazine`) and initially reverted defensively — but hand-
   tracing the real rule text (P-25.3.4.2.4(d), verified example: locant sequence "4,5"
   beats "5,4", i.e. ascending beats descending) confirmed the NEW value
   (`furo[2,3-b]thieno[3,2-e]pyrazine`) is the better-supported one; re-checking this
   doc's own Phase 44 entry found it only ever claimed the sibling *pyridine* case
   (`furo[3,2-b]thieno[2,3-e]pyridine`, unchanged, still passing) was hand-verified
   against a real PIN — the pyrazine sibling's value was never independently verified in
   the first place, just whatever the original code happened to output. `iupac_namer_test`
   457→459/459, `ctest` 11/11.

   **Remaining**: branching/peri-fused/bridged topologies stay explicitly out of scope
   here (item 2's territory — the new code cleanly rejects any ring with 3+ fusion
   neighbors); identical-attached-component multiplying prefixes and multiparent names
   are separate, unaddressed gaps.
2. **P-25.4/P-25.5 (peri-fused / bridged-fused / 3-component-limit systems)** — **detection
   + clean rejection fixed 2026-08-21** (commit `5173962`); actual name *construction* for
   these topologies is still unimplemented and remains explicitly out of scope (see below).
   Before this fix, a molecule with any of these three topologies silently fell through the
   existing `shared.size() == 2` ortho-fusion checks into whatever generic code path
   happened to run next — no crash, but no specific/accurate error either. Now:
   - **True ortho-and-peri-fusion (P-25.3.1.1.2)** is detected as three unsaturated rings
     with a common atom shared by all three (the real topology — e.g. acenaphthylene's
     C8a, shared by both naphthalene rings and the added 5-ring — not merely two rings
     sharing 3 atoms directly, which is a different topology, see below), and rejected with
     `"ortho- and peri-fused ring systems (P-25.3.1.1.2) are not yet supported."`
   - **P-25.4 bridged-fusion** is detected as a *pairwise* ring share of 3+ atoms (or a
     disconnected multi-component shared-atom set) between two unsaturated rings — this is
     the correct topology for a genuine bridge (two rings connected by more than one
     bridgehead atom directly, e.g. a naphthalene with a methano bridge across peri
     positions), distinct from the 3-ring peri-fusion case above — rejected with
     `"bridged fused ring systems (P-25.4) are not yet supported."`
   - **P-25.5** (three components mutually ortho-and-peri-fused, e.g. pyrene) is detected
     heuristically as peri-fusion present with either 2+ distinct peri-fusion centers or
     4+ total SSSR rings, and rejected with `"three-component ortho- and peri-fused
     systems (P-25.5) are not yet supported."`
   - All three checks are gated on both rings having at least one double/aromatic bond in
     them, deliberately, to avoid colliding with the separate, unrelated von Baeyer
     saturated-bicyclic/spiro code path (Phase 53) — a saturated bridged/peri system is a
     von Baeyer naming problem, not a fusion-nomenclature one, and isn't touched by this
     fix (still handled, or not, by whatever code already handled it).
   - Verified: acenaphthylene, pyrene, and 1,4-methanonaphthalene each independently
     rebuilt and confirmed to produce their specific expected rejection message (not a
     generic one, not a crash, not a fabricated name). `iupac_namer_test` 459→462/462
     (exactly +3, no regressions — every pre-existing case still passes unchanged),
     `ctest` 11/11, independently rebuilt and re-run from a clean PowerShell +
     `mingw32-make.exe` build after the commit, not taken on the implementer's self-report.
   - **Still needed, unaddressed by this fix**: actual name *construction* for any of these
     three topologies (fusion-descriptor building per P-25.3.1.3's multi-letter/no-comma
     rule, interior-atom peripheral numbering per P-25.3.3.2/.3, bridge-prefix nomenclature
     per P-25.4.2, and P-25.5's fallback procedures) — this fix only makes the current
     "can't do it" outcome honest and specific instead of silent/generic. `imidazo[2,1-
     b][1,3]thiazole`-style common-heteroatom-at-a-fusion-point naming (P-25.3.2.5.1) is a
     separate, unrelated sub-rule (heteroatom *sharing* within an otherwise-normal
     ortho-fused system, not peri-fusion) not touched here either.
3. **P-22.2 general Hantzsch-Widman stem construction** — **Phase 51** implemented for ring sizes 5-6; **Phase 55** extended to sizes 3-4 and 7-10. **Phase 70** (2026-08-19) added ring sizes 11-20, but via a DIFFERENT mechanism than Hantzsch-Widman: per the real Blue Book text (P-22.2.3, verified against `BlueBookV2.md` directly, not memory), sizes 11+ don't use Hantzsch-Widman stems at all -- they use skeletal replacement ('a') nomenclature (`cyclo`+chain-root+`ane`, e.g. "azacycloundecane", "1,5-dithiacyclododecane"), with locants+prefix grouped PER HETEROATOM KIND and hyphen-joined (structurally different from Hantzsch-Widman's pooled-locant-list style). New `RingType::LARGE_HETEROCYCLE` in `classifyMonocyclicHeteroRing`/`IupacNamer.cpp`, reusing the existing generic ring-numbering comparator (`heteroatomLocants`/`heteroatomSeniorityAtLocants`) unchanged -- it already implemented the right rule. Two real Blue Book PIN examples pinned directly in the new tests ("1,5-dithiacyclododecane (not 1,9-)" and "1-thia-5-selenacyclododecane"). Found and fixed a related latent bug while testing: the whole-molecule amine-detection scan was treating a ring's own internal N-C bond as an exocyclic amino substituent, invisible until now because every saturated non-hardcoded heterocycle was previously rejected before reaching that scan. `iupac_namer_test` 452→456/456, `ctest` 11/11, `cavecrew-reviewer`: no issues (including a specific check of the other 8 call sites of `classifyMonocyclicHeteroRing` for unintended blast radius). **Scoped to the fully saturated, unsubstituted case only** at the time this paragraph was first written -- both remaining gaps below are now closed:

- **Mancude (`-ene` chain, P-22.2.4) form — fixed 2026-08-21** (commit `5bd5145`, "Part A"). Contrary to this entry's original assumption, Indigo *does* aromatize at least some macrocycles this large (confirmed live: a 14-membered 1-aza ring came back with all 14 ring bonds at order 4) — but the fix doesn't rely on that. It computes the theoretical maximum-noncumulated-double-bond pattern directly from ring topology: each ring atom gets a "spare valence" (0 for a divalent heteroatom — O/S/Se/Te, the only ones this file's `hwAPrefix` covers — 1 for everything else, C included). Zero-spare atoms are forced-single-bond boundaries that partition the ring into linear runs of spare-1 atoms; each run must alternate S,D,...,S (parity-checked, rejects cleanly if impossible); a ring with zero zero-spare atoms is one whole alternating cycle (requires even ring size). The real molecule's actual double-bond count and placement are then validated against this computed maximum (real Blue Book PIN examples pinned verbatim: `1-oxacycloundeca-2,4,6,8,10-pentaene`, `1,8-dioxacyclooctadeca-2,4,6,9,11,13,15,17-octaene`, `1-azacyclotetradeca-1,3,5,7,9,11,13-heptaene`) — chose the stricter design (reject if the real molecule's double bonds don't exactly match the computed maximal pattern) over the permissive "just cite whatever's there" alternative `CYCLOALKENE` uses, since P-22.2.4 explicitly requires *maximum* noncumulated unsaturation, not merely *some*.
  - **Known theoretical edge case, documented not fixed**: for the "whole-cycle, zero divalent-heteroatoms" case specifically (no O/S/Se/Te at all — e.g. an all-C-or-N ring), there are two chemically distinct valid alternation phases for the same heteroatom composition, and the per-candidate locant computation always assumes one specific phase (doubles starting at the candidate's own local position 1) rather than reading which phase the real input molecule actually has. This was investigated empirically (constructed the deliberate phase-shifted mirror of the real azacyclotetradecaheptaene test molecule) and found to be a non-issue for that specific case only because Indigo's own aromaticity perception collapses both phase variants to an identical order-4 representation before this code ever sees explicit bond orders, so there's no real ambiguity left to get wrong. It is NOT proven this holds for every ring composition in the whole-cycle sub-case — a large ring Indigo declines to aromatize, with genuinely explicit alternating single/double bonds in the "wrong" phase, could in principle still get a wrong (not rejected) locant set from this code path. Runs anchored by at least one divalent heteroatom (the two O-containing pinned tests) are NOT affected by this — each run's alternation is the unique solution given its fixed single-bond boundaries, regardless of aromaticity perception.
- **Substituted large heterocycles — fixed 2026-08-21** (commit `124df2b`, "Part B"). The `bareRing` gate (required every ring atom to have exactly 2 total neighbors, incidentally blocking both substituents and fusion/bridge/spiro atoms at once) is replaced with a check specifically on ring-*internal* neighbor degree (must be exactly 2), which still excludes fused/bridged/spiro topologies but now allows exocyclic substituents through. The existing generic substituent-citation loop needed zero changes — confirmed genuinely type-agnostic as suspected. One numbering bug caught during independent review of this change (not in the original delegated commit): the substituted-ring regression test asserted a wrong expected locant (`3-methylazacycloundecane`) for a case where hand-tracing the SMILES shows the methyl-bearing carbon is directly ring-closure-bonded to the heteroatom, so the correct lowest locant is 2, not 3 — fixed directly (test corrected to `2-methylazacycloundecane`, matching what the existing lowest-locant machinery actually and correctly produces).
- `iupac_namer_test` 462→465/465 (item 2's fix landed first, then +3 for Part A), `ctest` 11/11 throughout, independently rebuilt via PowerShell + `mingw32-make.exe` and re-run after every change, not taken on self-report.
4. **P-23/P-24 von Baeyer & spiro heteroatoms — fixed 2026-08-21** (commits `7627955` von Baeyer, `8936a6d` spiro). See the P-23.0-P-23.2 and P-24.0-P-24.2 rows above for the full detail. `iupac_namer_test` 468→471/471 (spiro), 465→468/468 (von Baeyer), `ctest` 11/11 throughout, independently rebuilt and re-run after every change. **Remaining, unaddressed**: P-23.4-P-23.6 and P-24.2.4.2/.3 (homogeneous/alternating-heteroatom naming, nonstandard bonding numbers), P-24.3-P-24.8 polyspiro/branched spiro (a real, much larger, separate feature — do not confuse with the P-24.2.4 heteroatom fix just landed, which the coverage doc previously mis-cited under the P-24.3-P-24.8 section number). P-23.7 retained names (adamantane, cubane) fixed separately 2026-08-21 — see the P-23.0-P-23.2 row above.
5. **Blue Book section P-44 (ring-vs-chain and other parent-structure seniority)** — **Phase 52 closed the P-44.1.1 + P-44.1.2.2 core** (count-based decision + ring-wins-on-tie); **Phase 54 generalized chain-as-parent naming** from carboxylic-acid-only to ACID/AMIDE/NITRILE/ALDEHYDE/KETONE/ALCOHOL/THIOL/AMINE (`nameChainParentWithRingSubstituent` + new `nameAcyclicChainParentWithSubstituents`), fixing 3 bugs surfaced in the process (a stray ring-only restriction blocking chain-attached thiol classification; a false-tie in the ring/chain instance count caused by miscounting an exocyclic principal carbon as ring-side; and `nameChainParentWithRingSubstituent` itself dropping exocyclic principal carbons from its own count, under-naming diol/diamine cases). (SULFONIC_ACID/THIAL/THIONE added in Phase 56; ESTER/ACYL_HALIDE added in Phase 58; BORONIC_ACID/PHOSPHINE added in Phase 61, via `nameBranchGraph` rather than the numbered-chain-suffix machinery).

   **`P-44.1.2` heteroatom-skeleton seniority — fixed 2026-08-21** (commit `2b671a9`) at all 3
   real call sites (the single-ring-substituent gate around `IupacNamer.cpp:3860`, plus the two
   Phase 52 `combinedWinner` locations): a ring containing any skeletal heteroatom now outright
   beats a plain-carbon chain for parent-hood per P-44.1.2.1 ("a single senior atom is
   sufficient"), ahead of the pre-existing P-44.1.1 instance-count comparison — which now only
   decides ties when both sides are plain carbon (P-44.1.2.2), since this codebase's acyclic
   chain namer has no mechanism for a heteroatom in the chain's own backbone (so the chain side
   is always plain carbon in practice). Critically, the fix only applies when the ring genuinely
   bears at least one instance of the winning principal-characteristic-group class — a first
   delegated attempt at this applied the heteroatom override unconditionally, even when the ring
   had zero instances of the winning group, stranding the principal group with no suffix-bearing
   parent (e.g. a furan ring with no COOH, competing against a chain that actually carries the
   acid, got renamed as if the COOH were a diol). Caught by independent review before landing,
   reverted, and re-implemented correctly. `iupac_namer_test` 471/471 (identical to baseline —
   the furan-plus-chain-COOH regression case is unaffected), `ctest` 11/11.

   **Adjacent gap found during this work — locant-omission half fixed 2026-08-21** (commit
   `0677742`, elision follow-up in `4587501`): ring-as-parent name assembly (the on-ring suffix
   branch, `IupacNamer.cpp` ~line 9248, shared by benzene/pyridine/furan/GENERAL_HETEROCYCLE ring
   parents alike) was dropping the suffix group's own locant unconditionally whenever there was
   exactly one instance of it — correct only for a genuinely symmetric, otherwise-unsubstituted
   all-carbon monocycle (`cyclohexanol`, still correctly locant-less), wrong whenever the ring
   also carries another substituent (`2-propylbenzenol` — ambiguous, missing the OH's own
   position) or is a heteroatom ring where numbering is never symmetric (`2-propylpyridinol` —
   worse, since pyridine's numbering is fixed by N regardless of substituents). Fixed by gating
   the omission on `prefixPart.isEmpty() && allCarbon` instead of unconditionally; confirmed
   pre-existing and independent of the P-44.1.2 fix above via a baseline-code control test before
   fixing. A first pass at the fix omitted the terminal-`e` elision the with-locant case needs
   (`2-propylbenzene-1-ol` instead of the correct `2-propylbenzen-1-ol`) — caught by comparing
   against this file's own already-correct precedent (`naphthalen-1-ol`, `propan-1-ol`, both of
   which already elide before a locant-prefixed vowel-initial suffix) and fixed by computing the
   possibly-elided stem once, before branching, instead of only in the now-narrower omit-locant
   branch. `iupac_namer_test` 473→476/476, `ctest` 11/11.

   **"phenol" retained name — fixed 2026-08-21** (commit `ba129b4`): benzene ring + sole OH
   (`RingType::BENZENE && winningType == GroupType::ALCOHOL && pCount == 1`) now assembles
   `prefixPart + "phenol"` directly instead of the systematic `benzen-1-ol` form, e.g.
   `4-methylphenol`, `2-propylphenol` (substituent locants unaffected, already correctly numbered
   relative to the OH-bearing carbon as position 1). Scoped narrowly to the sole-OH case only, and
   deliberately correct to stop there: **catechol/resorcinol/hydroquinone are NOT retained PINs**
   (verified directly against `BlueBookV2.md`, which lists them exactly as "pyrocatechol
   benzene-1,2-diol (PIN)" / "resorcinol benzene-1,3-diol (PIN)" / "hydroquinone benzene-1,4-diol
   (PIN)" — i.e. the traditional name on the left, its actual PIN on the right — and explicitly
   confirms this for the substituted case too: "2-nitrobenzene-1,3-diol (PIN) (not
   2-nitroresorcinol)"). The existing systematic `diol` form this codebase already produces for
   `pCount > 1` is therefore already the correct PIN as-is — this is not a gap, and implementing
   the traditional names here would be a regression, not an improvement; do not "fix" this again
   without re-reading this note. `iupac_namer_test` 476→478/478, `ctest` 11/11.

   **Sub-task B (polycyclic ring as chain substituent) explicitly deferred, not attempted**:
   lifting the single-attachment / monocyclic-ring restriction (fused/bridged/polycyclic ring as
   a chain substituent, e.g. a real naphtho-fused system or a von Baeyer bicyclic cited as a
   `-yl` substituent rather than as parent) remains unimplemented. The existing naphthalene-as-
   substituent case (`nameBranchGraph`) is a hardcoded one-off, not backed by the general fused-
   ring machinery — a real fix needs to reuse the same ring/fusion-detection logic already used
   for ring-as-parent, renumber it under substituent-numbering rules, and handle fused-system-
   specific `-yl` suffix construction (not simply "name + yl"). Scoped as a separate, genuinely
   larger future phase, not bundled with the small P-44.1.2 fix above. (Not to be confused with
   this codebase's own Phase 44, above.)
6. **P-9 stereochemistry completeness**: P-92.6 (pseudoasymmetric/Sequence Rule 5) and P-93.4's 2nd-shell E/Z tie-breaks are done via Indigo's own CIP engine. P-93.5/93.6's branch-stereocenter-on-ring-parent case is also done (monocyclic and naphthalene parents). **A ring used as a substituent with a stereocenter on the ring's own atom -- fixed 2026-08-19.** `nameRingAsSubstituent` already computes its own correct winning ring numbering (`best.ringChain`, from the same candidate-scoring machinery used for ring-as-parent numbering) -- it now builds a `"(nR)-"`/`"(nR,mS)-"` prefix directly from that, using new (defaulted) `stereoByGraphId`/`handledBranchStereoIds` parameters, rather than reusing `formatBranchStereoPrefix` (whose chain-walk locant numbering has no relation to a ring's real numbering and would have produced wrong locants, or double-processed the same stereocenter, if applied to a ring). Threaded through 4 call sites: `nameBranchGraph`'s own delegation to it; `nameChainParentWithRingSubstituent` (2 call sites in `generateName`, for a functional-group chain with a ring substituent, e.g. `"5-[(2R)-2-methylcyclopentyl]heptanoic acid"` -- pinned as a real test, matches the confirmed-reachable case); and the `ringSubstituentInfos` pre-collection pass (for ring substituents attached near a principal group) -- verified safe to thread through since its consumers append the pre-built name string verbatim, never calling `formatBranchStereoPrefix` on those same ring nodes, so no double-wrap risk. `iupac_namer_test` 456→457/457, `ctest` 11/11, two `cavecrew-reviewer` passes (one found the missed `ringSubstituentInfos` call site; the scoped follow-up confirmed the fix and all its downstream consumers are safe). **Von Baeyer/spiro parent stereo — fixed 2026-08-21** (commit `6a355bc`): confirmed via the real Blue Book text (P-93.5.2.1, P-93.5.3.1) that a stereocenter on a von Baeyer or spiro parent ring is plain CIP R/S with no new descriptor system — pure wiring, not new geometry. Both Phase 27 and Phase 28's final name-assembly points now call `formatStereoPrefix(stereoByGraphId, best.locantOf, {})` and prepend the resulting prefix before everything else (including the heteroatom/substituent prefixes item 4 added), mirroring the acyclic-chain/naphthalene/monocyclic-ring call sites exactly. Verified: `(1S,2S,4R)-2-bromobicyclo[2.2.1]heptane` and `(2R)-2-bromospiro[3.5]nonane`, both independently rebuilt and re-run. `iupac_namer_test` 471→473/473, `ctest` 11/11. P-93.5.3.5 (axial chirality of spiro compounds — a genuinely different concept from an ordinary tetrahedral spiro-atom stereocenter) and `endo`/`exo`/`syn`/`anti`/`cis`/`trans` bridgehead descriptors (general-nomenclature-only, not required for PINs) remain out of scope, deliberately not attempted. Allenes (M/P vs Ra/Sa notation, a separate descriptor system entirely) also remain unimplemented — explicitly not attempted this pass per its own scoping (a throwaway spike to check whether Indigo's `indigoStereocenterCIPDescriptor` returns anything usable for `indigoIterateAlleneCenters`-yielded atoms was optional and was skipped in favor of the real deliverable).
7. **E/Z coordinate-dependence** (found during item 6's work, pre-existing before it too): `processDoubleBondStereo` reads whatever 2D coordinates currently exist rather than tracking whether stereo was ever deliberately specified, so a double bond with genuinely unspecified geometry can get an arbitrary, non-reproducible E/Z label once it has passed through any auto-layout pass (confirmed live via the app's own SMILES-load → layout → naming flow). Needs a way to carry "was this bond's stereo ever deliberately specified" through the SMILES→document→molfile round trip — a real design question (molfiles have no such flag for double bonds outside the rare `stereo=3` "either" marker), not a quick fix.
8. **Fix the dead bond-E/Z-display path in `IndigoService::calcStereoDescriptors`** — fixed
   2026-08-18. `addCIPSgroups` was only ever wired to the molfile-save path, never JSON/KET,
   so `MoleculeLayer.qml`'s bond-E/Z-label rendering (`b.cipLabel`) had never actually
   displayed anything. Fix: `computeIndigoBondCIP` (`IupacNamer.cpp`'s existing helper that
   reads the KET JSON `"cip"` bond field directly) was promoted to external linkage --
   moved out of its anonymous namespace to file scope, declared in `IupacNamer.h` -- so
   `IndigoService.cpp` (which already includes that header) can call the same one copy
   instead of writing a third. `IndigoService::calcStereoDescriptors`'s DAT-sgroup-hunting
   block replaced with a direct call to it. `iupac_namer_test` 452/452 unaffected (no
   name-generation logic touched, only the helper's linkage/location), `ctest` 11/11.
   `cavecrew-reviewer` pass: no issues (checked forward-declaration/ODR correctness across
   the anon-namespace boundary, JSON shape parity, and that the moved function has no
   hidden dependency on anonymous-namespace-local state). Not independently live-clicked
   in the running app (pywinauto's File-menu automation hit its now-familiar timing
   flakiness on this exact click sequence for a third time this session; skipped a fourth
   retry given the fix is a pure linkage/plumbing change to an already-proven helper, not
   new logic) -- worth a manual look next time the app is open with a real E/Z double bond
   (e.g. `Cl/C=C/Cl`) selected.
9. **Branch-stereocenter guard bypass on the acyclic parent path (Phase 1)** — fixed. Same bug
   class already fixed for the ring paths (Phase 2/3): `formatBranchStereoPrefix` was called
   unconditionally right after `nameBranchGraph` with no check that `nameBranchGraph` actually
   produced a name. Fixed by gating the call behind `!bName.isEmpty()` (superseded by item 12's
   later rewrite into an early-return reject; the equivalent guard now lives at
   `IupacNamer.cpp:4296-4304`), identical in shape to the Phase 2/3 fix at `IupacNamer.cpp:8173`
   and `:9071`. **Scope note:**
   this only fixes the case where the unnameable branch ALSO carries a stereocenter (the disclosed
   bug). A confirmed, separate, pre-existing bug remains for unnameable branches WITHOUT a
   stereocenter: `nameBranchGraph` can return `""` for other reasons (e.g. a fused/bridged ring
   branch at `IupacNamer.cpp:1212`, or an azide at `:1257`) and the final
   `locantSubstituents[locant].append(bName)` at `:4313` still unconditionally appends the empty
   string, producing a malformed but "successful" name -- confirmed live via a temporary probe:
   `CCCCC(CN=[N+]=[N-])CCC` (no stereocenter) returns `success=1 name='4-octane'` (the azide
   substituent is silently dropped instead of triggering rejection). This predates and is
   unrelated to this fix (the unconditional append was never inside the guard this fix added);
   tracked as new item 12 below, not fixed by this plan. Live-UI note: for the specific regression
   molecule (`CCCCC([C@H](CN=[N+]=[N-])C)CCC`), the app's SMILES-load-then-molfile-round-trip
   path rejects earlier, with "Charged atoms are not supported in Phase 1." (the early
   reject-early charge check at `IupacNamer.cpp:~2586`, well upstream of this fix) rather than
   the unit test's direct-SMILES-load "Stereocenters on substituent branches are not supported
   in this phase." — both are correct rejections (no malformed success either way); the
   difference traces to the azide's exact bond-order/charge pattern not surviving the
   molfile round trip identically to a direct SMILES parse, unrelated to this fix.
10. **Stereo-bracket alphabetization** — fixed. A shared `alphabetizationKey()` helper
    (`IupacNamer.cpp`, near `multiPrefix`) now strips wrapping brackets, stereo-descriptor
    parentheticals, and locant-digit prefixes before alphabetizing, used at both the
    substituent-citation-order sort-key sites (6) and the numbering/path-direction tiebreak
    sites (6: 4 ring/spiro/naphthalene-numbering sites at `IupacNamer.cpp:4769, 5080, 8211,
    9113`, plus 2 acyclic-chain numbering-direction sites at `:1911, 3907`) that previously
    compared raw, still-bracketed names.
11. Lower priority / rarely load-bearing for this app: P-26 (phane), P-27 (fullerenes), P-7/P-8 (ions/isotopes), P-10 (natural products).
12. **Unnameable-branch guard bypass without a stereocenter (Phase 1)** — fixed 2026-08-18.
    `nameBranchGraph` can return `""` for a branch that is unnameable for reasons unrelated
    to stereocenters (fused/bridged ring branch at `IupacNamer.cpp:1212`, azide at `:1257`,
    and likely other `return ""` sites in the same function). The direct-branch loop's
    `locantSubstituents[locant].append(bName)` (now `IupacNamer.cpp:4313`) used to be
    unconditional and was never inside item 9's `!bName.isEmpty()` guard (that guard only
    wrapped the stereo-prefix logic, not the append itself) -- so an unnameable,
    non-stereocenter branch was silently dropped from the name instead of triggering
    rejection. Confirmed live before the fix: `CCCCC(CN=[N+]=[N-])CCC` returned
    `success=1 name='4-octane'` (azide substituent vanished). Fix: `IupacNamer.cpp:4299-4302`
    now returns `{false, "", "Unrecognized or unsupported substituent."}` as soon as
    `nameBranchGraph` returns `""`, mirroring the Phase 2/3 rejection pattern. New regression
    test in `tests/iupac_namer_test.cpp` pins the fixed molecule to that rejection. Side
    effect: the existing item-9 test (`CCCCC([C@H](CN=[N+]=[N-])C)CCC`, stereocenter +
    unnameable substituent nested two branches deep) now hits this new guard before the
    old stereocenter-specific rejection -- both messages are correct for that molecule, but
    this one is more specific about the actual root cause, so the test was updated to expect
    it. `iupac_namer_test` 451/451, `ctest` 11/11.
13. **Same unguarded-empty-append bug pattern, other element branches (Phase 1)** — fixed
    2026-08-18. Confirmed live (not just theoretical): the thioether branch's
    `alkylName += "sulfanyl"` ran even when `nameBranchGraph` returned `""` for the
    alkyl side (e.g. an azide-containing alkyl group), silently emitting a bare
    "sulfanyl" instead of rejecting -- same bug class as item 12, just in the
    sulfur/selenium/tellurium/ether suffix blocks instead of the plain-carbon branch.
    Fixed all 13 sites the same way (`if (alkylName.isEmpty()) return {false, "",
    "Unrecognized or unsupported substituent."};` right after the `nameBranchGraph`
    call, before the suffix is appended): sulfanyl/sulfinyl/sulfonyl/disulfanyl
    (`IupacNamer.cpp:4050, 4061, 4072, 4088`), selanyl/seleninyl/selenonyl/diselanyl
    (`:4102, 4113, 4124, 4140`), tellanyl/tellurinyl/telluronyl/ditellanyl
    (`:4154, 4165, 4176, 4192`), and the ether prefix (`:4209`). New regression test
    (`CCCCC(SCN=[N+]=[N-])CCC`, a thioether whose alkyl side is unnameable) confirms
    rejection instead of a bogus name. None of the 13 guards changed behavior on any
    previously-passing case (all still named their alkyl side successfully before
    appending the suffix) -- confirmed by rebuilding and rerunning the full suite
    before adding the new test: 451/451 unchanged, then 452/452 with the new test.
    `ctest` 11/11.
