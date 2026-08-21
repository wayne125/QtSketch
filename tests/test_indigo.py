from indigo import Indigo
indigo = Indigo()
mol = indigo.loadMolecule("CCCCC(=O)NN")
for a in mol.iterateAtoms():
    if a.atomicNumber() == 1: continue
    print(f"Atom {a.index()} Z={a.atomicNumber()}")
    for nei in a.iterateNeighbors():
        if nei.atomicNumber() == 1: continue
        print(f"  Nei {nei.index()} Z={nei.atomicNumber()} Bond={nei.bond().bondOrder()}")
