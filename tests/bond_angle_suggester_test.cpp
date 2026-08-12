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

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
