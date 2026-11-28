#!/usr/bin/env python3
"""Patch out-of-bounds sentinel end-iterator erasures in Imago's beast::Graph
(removeVertex, getDegree, removeEdge) that otherwise cause heap corruption on
small single-ring crops and stereo-wedge exports. Shared by
packaging/macos/build_imago.sh and packaging/linux/build_imago.sh.

Usage: patch_imago_beast.py <imago-src-dir>
"""
import os
import sys


def main(src: str) -> None:
    beast = os.path.join(src, "core/src/beast.h")
    with open(beast, "r", encoding="utf-8") as f:
        btxt = f.read()
    btxt = btxt.replace(
        "void removeVertex(vertex_descriptor desc)\n        {\n            auto u_iter = _vertex_indices[desc.id];",
        "void removeVertex(vertex_descriptor desc)\n        {\n            if (desc.id >= _vertex_indices.size() || _vertex_indices[desc.id] == _vertices.end()) return;\n            auto u_iter = _vertex_indices[desc.id];",
        1,
    )
    btxt = btxt.replace(
        "size_t getDegree(vertex_descriptor desc) const\n        {\n            return _vertex_indices[desc.id]->neighbors.size();",
        "size_t getDegree(vertex_descriptor desc) const\n        {\n            if (desc.id >= _vertex_indices.size() || _vertex_indices[desc.id] == _vertices.end()) return 0;\n            return _vertex_indices[desc.id]->neighbors.size();",
        1,
    )
    btxt = btxt.replace(
        "void removeEdge(edge_descriptor desc)\n        {\n            auto iter = _edge_indices[desc.id];",
        "void removeEdge(edge_descriptor desc)\n        {\n            if (desc.id >= _edge_indices.size() || _edge_indices[desc.id] == _edges.end()) return;\n            auto iter = _edge_indices[desc.id];",
        1,
    )
    with open(beast, "w", encoding="utf-8") as f:
        f.write(btxt)


if __name__ == "__main__":
    main(sys.argv[1])
