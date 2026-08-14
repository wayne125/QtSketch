#include "app/BondAngleSuggester.h"
#include <cstdio>
#include <cmath>

#define CHECK(cond, msg) do { \
    if (!(cond)) { std::printf("FAIL: %s\n", msg); failures++; } \
    else { std::printf("PASS: %s\n", msg); } \
} while (0)

static QVariantMap atom(double x, double y) {
    QVariantMap m; m["x"] = x; m["y"] = y; return m;
}
static QVariantMap bond(int begin, int end) {
    QVariantMap m; m["begin"] = begin; m["end"] = end; return m;
}

int main() {
    int failures = 0;

    // Test 1: 0 existing neighbors -> nullopt.
    {
        QVariantMap atomsById; atomsById["1"] = atom(0, 0);
        auto r = BondAngleSuggester::suggestAngle(1, atomsById, QVariantList());
        CHECK(!r.has_value(), "0 neighbors returns nullopt");
    }

    // Test 2: 1 existing neighbor (atom 2 due east of atom 1) -> 60-degree kink.
    // Continuation of the 1->2 bond, viewed from atom 1, is due WEST (M_PI). The kink is
    // +/- 60 degrees (PI/3) off that, and with no grandparent to alternate against, turnSign
    // defaults to +1.0, so the expected angle is M_PI + M_PI/3.
    {
        QVariantMap atomsById; atomsById["1"] = atom(0, 0); atomsById["2"] = atom(1.5, 0);
        QVariantList bonds; bonds.append(bond(1, 2));
        auto r = BondAngleSuggester::suggestAngle(1, atomsById, bonds);
        CHECK(r.has_value(), "1 neighbor returns a value");
        double expected = M_PI + M_PI / 3.0;
        CHECK(r.has_value() && std::abs(*r - expected) < 1e-9, "1 neighbor kinks 60 degrees off continuation");
    }

    // Test 3: 2 existing neighbors 90 degrees apart (east and north of atom 1) -> bisects
    // the LARGER gap (the 270-degree one), landing at its midpoint: -PI/4 (south-west-ish),
    // i.e. halfway around the 270-degree arc starting from the "east" angle (0) going
    // clockwise through south, west, to north (PI/2).
    {
        QVariantMap atomsById; atomsById["1"] = atom(0, 0); atomsById["2"] = atom(1, 0); atomsById["3"] = atom(0, 1);
        QVariantList bonds; bonds.append(bond(1, 2)); bonds.append(bond(1, 3));
        auto r = BondAngleSuggester::suggestAngle(1, atomsById, bonds);
        CHECK(r.has_value(), "2 neighbors returns a value");
        // lo=0, hi=PI/2, width1=PI/2, width2=3PI/2 -> width2 bigger -> lo + width1/2 + PI = PI/4 + PI
        double expected = M_PI / 4.0 + M_PI;
        CHECK(r.has_value() && std::abs(*r - expected) < 1e-9, "2 neighbors bisects the larger gap");
    }

    // Test 4: 3 existing neighbors -> nullopt (left to caller's own fallback).
    {
        QVariantMap atomsById;
        atomsById["1"] = atom(0, 0); atomsById["2"] = atom(1, 0);
        atomsById["3"] = atom(0, 1); atomsById["4"] = atom(-1, 0);
        QVariantList bonds; bonds.append(bond(1, 2)); bonds.append(bond(1, 3)); bonds.append(bond(1, 4));
        auto r = BondAngleSuggester::suggestAngle(1, atomsById, bonds);
        CHECK(!r.has_value(), "3 neighbors returns nullopt");
    }

    // Test 5: fromAtomId not present -> nullopt.
    {
        QVariantMap atomsById; atomsById["1"] = atom(0, 0);
        auto r = BondAngleSuggester::suggestAngle(99, atomsById, QVariantList());
        CHECK(!r.has_value(), "unknown fromAtomId returns nullopt");
    }

    // Test 6: 1 existing neighbor WITH a grandparent -- turnSign should flip based on the
    // existing zigzag direction, not default to +1. Chain A(0,0)-B(1.5,0)-C, where C is B's
    // second bonded atom (target atom is C) placed via a 60-degree kink below the A-B line:
    // C = B + 1.5*(cos(-60deg), sin(-60deg)).
    // Hand-derived: existingAngle (C's angle to B) = atan2(0-Cy, 1.5-Cx) = 120 degrees.
    // continuation = existingAngle + 180 = 300 degrees. dirIn (A's angle to B) = atan2(0,1.5) =
    // 0 degrees. delta = dirOut(=continuation=300, i.e. -60) - dirIn(0) = -60 degrees ->
    // turnSign = +1 (delta < 0). Expected = continuation + turnSign*60 = 300 + 60 = 360 = 0
    // degrees. (A non-grandparent-aware implementation defaulting turnSign to +1 would ALSO get
    // +1 here by coincidence of this geometry's sign -- Test 7 below is the one that actually
    // distinguishes correct alternation from a stuck default.)
    {
        double bx = 1.5, by = 0.0;
        double cx = bx + 1.5 * std::cos(-M_PI / 3.0);
        double cy = by + 1.5 * std::sin(-M_PI / 3.0);
        QVariantMap atomsById;
        atomsById["1"] = atom(0.0, 0.0);   // A (grandparent)
        atomsById["2"] = atom(bx, by);     // B (existing neighbor)
        atomsById["3"] = atom(cx, cy);     // C (target atom, fromAtomId)
        QVariantList bonds; bonds.append(bond(1, 2)); bonds.append(bond(2, 3));
        auto r = BondAngleSuggester::suggestAngle(3, atomsById, bonds);
        CHECK(r.has_value(), "1 neighbor with grandparent returns a value");
        double expected = 0.0;
        double got = r.has_value() ? *r : 0.0;
        while (got > M_PI) got -= 2 * M_PI;
        while (got < -M_PI) got += 2 * M_PI;
        CHECK(r.has_value() && std::abs(got - expected) < 1e-6,
              "grandparent-aware kink lands at the flat-zigzag-continuation angle");
    }

    // Test 7: same setup as Test 6, MIRRORED across the A-B line (C now above instead of
    // below) -- this flips turnSign to -1 (the opposite branch from Test 6), and a
    // non-grandparent-aware implementation stuck at turnSign=+1 would compute
    // continuation + 60 = 60 + 60 = 120 degrees (curling toward a hexagon) instead of the
    // correct 0 degrees -- this is the test that actually discriminates alternation from a
    // stuck default, unlike Test 6 where +1 happens to be both the default AND the correct
    // answer.
    // Hand-derived: existingAngle = atan2(0-Cy, 1.5-Cx) = -120 degrees. continuation =
    // -120+180 = 60 degrees. dirIn = 0 degrees (unchanged, A-B unchanged). delta = 60 - 0 = 60
    // degrees -> turnSign = -1 (delta >= 0). Expected = continuation + turnSign*60 = 60 - 60 =
    // 0 degrees.
    {
        double bx = 1.5, by = 0.0;
        double cx = bx + 1.5 * std::cos(M_PI / 3.0);
        double cy = by + 1.5 * std::sin(M_PI / 3.0);
        QVariantMap atomsById;
        atomsById["1"] = atom(0.0, 0.0);   // A (grandparent)
        atomsById["2"] = atom(bx, by);     // B (existing neighbor)
        atomsById["3"] = atom(cx, cy);     // C (target atom, fromAtomId)
        QVariantList bonds; bonds.append(bond(1, 2)); bonds.append(bond(2, 3));
        auto r = BondAngleSuggester::suggestAngle(3, atomsById, bonds);
        CHECK(r.has_value(), "1 neighbor with mirrored grandparent returns a value");
        double expected = 0.0;
        double got = r.has_value() ? *r : 0.0;
        while (got > M_PI) got -= 2 * M_PI;
        while (got < -M_PI) got += 2 * M_PI;
        CHECK(r.has_value() && std::abs(got - expected) < 1e-6,
              "mirrored grandparent-aware kink lands at the flat-zigzag-continuation angle via the opposite turnSign branch");
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
