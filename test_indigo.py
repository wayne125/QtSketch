from indigo import Indigo
indigo = Indigo()
mol = indigo.loadMolecule("O=C1CCC(=O)O1")
for atom in mol.iterateAtoms():
    print(f"Atom {atom.index()}: {atom.symbol()}")
    for nei in atom.iterateNeighbors():
        print(f"  Neighbor {nei.index()}: bond order {nei.bond().bondOrder()}")
