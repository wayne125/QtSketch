#include "IndigoService.h"
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <mutex>
#include <functional>
#include "indigo.h"
#include "indigo-inchi.h"
#include "indigo-renderer.h"
#include <QtConcurrent>
#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QJsonArray>

// Read the monomer_library.ket once at first use; cache the bytes.
// Must be called with an active Indigo session — returns a library handle or -1.
static const QByteArray& monomerLibraryContent() {
    static QByteArray content;
    static std::once_flag flag;
    std::call_once(flag, []() {
        const QString rel = "/indigo/data/molecules/basic/monomer_library.ket";
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString cwd    = QDir::currentPath();
        QStringList candidates = {
            QDir::cleanPath(appDir + "/../.." + rel),   // build/<config>/ → project root
            QDir::cleanPath(appDir + "/.."    + rel),   // build/ → project root
            QDir::cleanPath(cwd              + rel),    // CWD is project root
            QDir::cleanPath(cwd   + "/.."    + rel),    // CWD is build/
        };
        for (const QString& path : candidates) {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) {
                content = f.readAll();
                return;
            }
        }
        qWarning() << "Indigo: monomer library not found. Tried:" << candidates;
    });
    return content;
}

// The worker's MOL serializer switches to $RXN (reaction) format whenever the
// structure has a reaction arrow — indigoLoadMoleculeFromString() cannot parse
// that (it expects a single-molecule counts line, not "$RXN"'s multi-$MOL
// header) and fails with a scanner "end of stream" error. Molecule-level
// properties/stereo-descriptors/validation don't have a single well-defined
// value for a whole multi-molecule reaction scheme, so these are skipped
// gracefully here rather than attempted — same treatment as an empty molfile.
static bool isReactionFormat(const QString &data) {
    return data.trimmed().startsWith(QLatin1String("$RXN"));
}

// QPointer's guard is documented as safe only when checked from the pointed-to
// object's own thread (or with extra synchronization) -- checking it directly from
// a QtConcurrent background thread and then emitting on it is a real, if narrow,
// race against ~IndigoService() running concurrently on the GUI thread (mainly at
// app shutdown, if a slow Indigo call is still in flight). This posts the
// check-and-emit onto the GUI thread instead, where both the check and any
// concurrent destruction are serialized by the same single-threaded event loop.
static void emitOnGuiThread(QPointer<IndigoService> self, std::function<void(IndigoService *)> fn) {
    QMetaObject::invokeMethod(qApp, [self, fn]() {
        if (self) fn(self.data());
    }, Qt::QueuedConnection);
}

static int loadMonomerLibrary() {
    const QByteArray& bytes = monomerLibraryContent();
    if (bytes.isEmpty()) return -1;
    return indigoLoadMonomerLibraryFromString(bytes.constData());
}

// Shared helper: expand monomer handles to atoms, run layout, export molfile.
// Must be called with the correct Indigo session already set.
static QString indigoBioExpand(int mol) {
    QString result;
    if (mol < 0) return result;
    indigoExpandMonomers(mol);
    indigoExpandedMonomersToAtoms(mol);
    indigoLayout(mol);
    const char* mf = indigoMolfile(mol);
    if (mf) result = QString::fromUtf8(mf);
    indigoFree(mol);
    return result;
}

IndigoService::IndigoService(QObject *parent) : QObject(parent) {
    m_sessionId = indigoAllocSessionId();
}

IndigoService::~IndigoService() {
    indigoSetSessionId(m_sessionId);
    indigoFreeAllObjects();
    indigoReleaseSessionId(m_sessionId);
}

void IndigoService::layout(const QString &molfile) {
    if (molfile.isEmpty()) {
        emit layoutFinished("");
        return;
    }
    
    QPointer<IndigoService> self = this;
    
    (void)QtConcurrent::run([self, molfile]() {
        QString resultMol = molfile;
        unsigned long long threadSessionId = indigoAllocSessionId();
        
        try {
            indigoSetSessionId(threadSessionId);
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                if (indigoLayout(mol) >= 0) {
                    const char* result = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                    if (result) {
                        resultMol = QString::fromUtf8(result);
                    }
                } else {
                    qWarning() << "Indigo: Layout failed:" << indigoGetLastError();
                }
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for layout:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ Exception in layout:" << e.what();
        } catch (...) {
            qWarning() << "Indigo C++ Unknown Exception in layout";
        }
        
        indigoReleaseSessionId(threadSessionId);
        
        emitOnGuiThread(self, [resultMol](IndigoService *s) { emit s->layoutFinished(resultMol); });
    });
}

void IndigoService::clean2d(const QString &molfile) {
    if (molfile.isEmpty()) {
        emit clean2dFinished("");
        return;
    }
    
    QPointer<IndigoService> self = this;
    
    (void)QtConcurrent::run([self, molfile]() {
        QString resultMol = molfile;
        unsigned long long threadSessionId = indigoAllocSessionId();
        
        try {
            indigoSetSessionId(threadSessionId);
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                if (indigoClean2d(mol) >= 0) {
                    const char* result = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                    if (result) {
                        resultMol = QString::fromUtf8(result);
                    }
                } else {
                    qWarning() << "Indigo: Clean2D failed:" << indigoGetLastError();
                }
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for clean2d:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ Exception in clean2d:" << e.what();
        } catch (...) {
            qWarning() << "Indigo C++ Unknown Exception in clean2d";
        }
        
        indigoReleaseSessionId(threadSessionId);
        
        emitOnGuiThread(self, [resultMol](IndigoService *s) { emit s->clean2dFinished(resultMol); });
    });
}

void IndigoService::aromatize(const QString &molfile) {
    if (molfile.isEmpty()) {
        emit aromatizeFinished("");
        return;
    }

    QPointer<IndigoService> self = this;

    (void)QtConcurrent::run([self, molfile]() {
        QString resultMol = molfile;
        unsigned long long threadSessionId = indigoAllocSessionId();

        try {
            indigoSetSessionId(threadSessionId);
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                int ret = indigoAromatize(mol);
                if (ret < 0) {
                    qWarning() << "Indigo: Aromatize failed:" << indigoGetLastError();
                } else {
                    const char* result = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                    if (result) resultMol = QString::fromUtf8(result);
                }
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for aromatize:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ exception in aromatize:" << e.what();
        } catch (...) {
            qWarning() << "Indigo unknown C++ exception in aromatize";
        }

        indigoReleaseSessionId(threadSessionId);

        emitOnGuiThread(self, [resultMol](IndigoService *s) { emit s->aromatizeFinished(resultMol); });
    });
}

void IndigoService::smiles(const QString &molfile) {
    if (molfile.isEmpty()) {
        emit smilesFinished("");
        return;
    }

    QPointer<IndigoService> self = this;

    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long threadSessionId = indigoAllocSessionId();

        try {
            indigoSetSessionId(threadSessionId);
            int mol = isReactionFormat(molfile) ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                                                 : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                // indigoSmiles produces reaction SMILES ("reactants>>products") natively
                // when given a reaction handle — no output-format switch needed here.
                const char* s = indigoSmiles(mol);
                result = s ? QString::fromUtf8(s) : "";
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for SMILES:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ exception in smiles:" << e.what();
        } catch (...) {
            qWarning() << "Indigo unknown C++ exception in smiles";
        }

        indigoReleaseSessionId(threadSessionId);

        emitOnGuiThread(self, [result](IndigoService *s) { emit s->smilesFinished(result); });
    });
}

void IndigoService::dearomatize(const QString &molfile) {
    if (molfile.isEmpty()) { emit dearomatizeFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = molfile;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoDearomatize(mol);
                const char* mf = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                if (mf) result = QString::fromUtf8(mf);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: dearomatize load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->dearomatizeFinished(result); });
    });
}

void IndigoService::unfoldHydrogens(const QString &molfile) {
    if (molfile.isEmpty()) { emit unfoldHydrogensFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = molfile;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoUnfoldHydrogens(mol);
                const char* mf = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                if (mf) result = QString::fromUtf8(mf);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: unfoldHydrogens load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->unfoldHydrogensFinished(result); });
    });
}

void IndigoService::foldHydrogens(const QString &molfile) {
    if (molfile.isEmpty()) { emit foldHydrogensFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = molfile;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoFoldHydrogens(mol);
                const char* mf = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                if (mf) result = QString::fromUtf8(mf);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: foldHydrogens load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->foldHydrogensFinished(result); });
    });
}

void IndigoService::canonicalSmiles(const QString &molfile) {
    if (molfile.isEmpty()) { emit canonicalSmilesFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = isReactionFormat(molfile) ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                                                 : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                // Reaction-aware, same as indigoSmiles above.
                const char* smi = indigoCanonicalSmiles(mol);
                if (smi) result = QString::fromUtf8(smi);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: canonicalSmiles load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->canonicalSmilesFinished(result); });
    });
}

void IndigoService::inchi(const QString &molfile) {
    // InChI has no reaction form (unlike SMILES's native "reactants>>products"),
    // so reactions are guarded out the same way checkStructure guards them below.
    if (molfile.isEmpty() || isReactionFormat(molfile)) { emit inchiFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        indigoInchiInit(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                const char* inchiStr = indigoInchiGetInchi(mol);
                if (inchiStr) result = QString::fromUtf8(inchiStr);
                else qWarning() << "Indigo: InChI generation failed:" << indigoInchiGetWarning();
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for InChI:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ exception in inchi:" << e.what();
        } catch (...) {
            qWarning() << "Indigo unknown C++ exception in inchi";
        }
        indigoInchiDispose(sid);
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->inchiFinished(result); });
    });
}

void IndigoService::inchiKey(const QString &molfile) {
    // InChIKey is a hash of the InChI string, not of the molecule directly --
    // computes the InChI first, then keys it, in one round trip.
    if (molfile.isEmpty() || isReactionFormat(molfile)) { emit inchiKeyFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        indigoInchiInit(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                const char* inchiStr = indigoInchiGetInchi(mol);
                // Must copy before the next Indigo call: indigoInchiGetInchi's
                // return value points into a buffer indigoInchiGetInchiKey
                // itself reuses/overwrites, so passing the raw pointer straight
                // through as its argument reads already-clobbered memory and
                // silently returns null (confirmed via a standalone repro).
                QString inchiCopy = inchiStr ? QString::fromUtf8(inchiStr) : QString();
                if (!inchiCopy.isEmpty()) {
                    const char* key = indigoInchiGetInchiKey(inchiCopy.toUtf8().constData());
                    if (key) result = QString::fromUtf8(key);
                    else qWarning() << "Indigo: InChIKey generation failed:" << indigoInchiGetWarning();
                } else {
                    qWarning() << "Indigo: InChI generation (for key) failed:" << indigoInchiGetWarning();
                }
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: Failed to load molecule for InChIKey:" << indigoGetLastError();
            }
        } catch (const std::exception& e) {
            qWarning() << "Indigo C++ exception in inchiKey:" << e.what();
        } catch (...) {
            qWarning() << "Indigo unknown C++ exception in inchiKey";
        }
        indigoInchiDispose(sid);
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->inchiKeyFinished(result); });
    });
}

void IndigoService::hash(const QString &molfile) {
    if (molfile.isEmpty()) { emit hashFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            bool isRxn = isReactionFormat(molfile);
            int obj = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                             : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (obj >= 0) {
                int64_t h = indigoHash(obj);
                result = QString::number(h);
                indigoFree(obj);
            } else {
                qWarning() << "Indigo: hash load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->hashFinished(result); });
    });
}

void IndigoService::similarity(const QString &molfile, const QString &refSmiles) {
    if (molfile.isEmpty() || refSmiles.isEmpty() || isReactionFormat(molfile)) {
        emit similarityFinished("");
        return;
    }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile, refSmiles]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            int ref = indigoLoadMoleculeFromString(refSmiles.toUtf8().constData());
            if (mol >= 0 && ref >= 0) {
                // Both structures must be aromatized before comparing - the similarity
                // fingerprint is sensitive to Kekule-vs-aromatic bond representation, and the
                // two sides here almost never agree on that by default: the canvas structure
                // arrives via a molfile (drawn bonds, typically Kekule), while a user-pasted
                // reference SMILES is very commonly aromatic notation (e.g. "c1ccccc1" from
                // PubChem/ChemDraw). Verified empirically: without this, comparing benzene
                // against itself (aromatic reference) scored 0.0769, not ~1.0; with it, 1.0000.
                if (indigoAromatize(mol) < 0) qWarning() << "Indigo: similarity aromatize(mol) failed:" << indigoGetLastError();
                if (indigoAromatize(ref) < 0) qWarning() << "Indigo: similarity aromatize(ref) failed:" << indigoGetLastError();
                float sim = indigoSimilarity(mol, ref, "tanimoto");
                result = QString::number(sim, 'f', 4);
            } else {
                qWarning() << "Indigo: similarity load failed:" << indigoGetLastError();
            }
            if (mol >= 0) indigoFree(mol);
            if (ref >= 0) indigoFree(ref);
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->similarityFinished(result); });
    });
}

void IndigoService::massComposition(const QString &molfile) {
    if (molfile.isEmpty() || isReactionFormat(molfile)) { emit massCompositionFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                const char* mc = indigoMassComposition(mol);
                if (mc) result = QString::fromUtf8(mc);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: massComposition load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->massCompositionFinished(result); });
    });
}

void IndigoService::pkaValues(const QString &molfile) {
    if (molfile.isEmpty() || isReactionFormat(molfile)) { emit pkaValuesFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                const char* pv = indigoPkaValues(mol);
                if (pv) result = QString::fromUtf8(pv);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: pkaValues load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->pkaValuesFinished(result); });
    });
}

void IndigoService::renderToFile(const QString &molfile, const QUrl &fileUrl, const QString &format) {
    if (molfile.isEmpty()) { emit renderFinished(false, "No structure to render."); return; }
    QPointer<IndigoService> self = this;
    QString path = fileUrl.toLocalFile();
    (void)QtConcurrent::run([self, molfile, path, format]() {
        bool ok = false;
        QString error;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        indigoRendererInit(sid);
        try {
            int mol = isReactionFormat(molfile) ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                                                 : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoSetOption("render-output-format", format.toUtf8().constData());
                int res = indigoRenderToFile(mol, path.toUtf8().constData());
                if (res >= 0) ok = true;
                else error = QString::fromUtf8(indigoGetLastError());
                indigoFree(mol);
            } else {
                error = QString("Failed to load structure: %1").arg(indigoGetLastError());
            }
        } catch (const std::exception &e) {
            error = QString::fromUtf8(e.what());
        } catch (...) {
            error = "Unknown exception during render.";
        }
        indigoRendererDispose(sid);
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [ok, error](IndigoService *s) { emit s->renderFinished(ok, error); });
    });
}

void IndigoService::renderReactionGridToFile(const QString &molfile, const QUrl &fileUrl, const QString &format) {
    if (molfile.isEmpty()) { emit renderFinished(false, "No structure to render."); return; }
    if (!isReactionFormat(molfile)) {
        emit renderFinished(false, "Grid export is for reactions only - the active document is a plain molecule.");
        return;
    }
    QPointer<IndigoService> self = this;
    QString path = fileUrl.toLocalFile();
    (void)QtConcurrent::run([self, molfile, path, format]() {
        bool ok = false;
        QString error;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        indigoRendererInit(sid);
        try {
            int rx = indigoLoadReactionFromString(molfile.toUtf8().constData());
            if (rx >= 0) {
                int arr = indigoCreateArray();
                int count = 0;
                int comp;
                int reactants = indigoIterateReactants(rx);
                while ((comp = indigoNext(reactants)) > 0) { indigoArrayAdd(arr, comp); count++; }
                int products = indigoIterateProducts(rx);
                while ((comp = indigoNext(products)) > 0) { indigoArrayAdd(arr, comp); count++; }
                if (count > 0) {
                    indigoSetOption("render-output-format", format.toUtf8().constData());
                    int res = indigoRenderGridToFile(arr, nullptr, count, path.toUtf8().constData());
                    if (res >= 0) ok = true;
                    else error = QString::fromUtf8(indigoGetLastError());
                } else {
                    error = "Reaction has no reactants or products to render.";
                }
                indigoFree(arr);
                indigoFree(rx);
            } else {
                error = QString("Failed to load reaction: %1").arg(indigoGetLastError());
            }
        } catch (const std::exception &e) {
            error = QString::fromUtf8(e.what());
        } catch (...) {
            error = "Unknown exception during grid render.";
        }
        indigoRendererDispose(sid);
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [ok, error](IndigoService *s) { emit s->renderFinished(ok, error); });
    });
}

void IndigoService::normalize(const QString &molfile) {
    if (molfile.isEmpty()) { emit normalizeFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = molfile;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoNormalize(mol, "");
                const char* mf = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                if (mf) result = QString::fromUtf8(mf);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: normalize load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->normalizeFinished(result); });
    });
}

void IndigoService::standardize(const QString &molfile) {
    if (molfile.isEmpty()) { emit standardizeFinished(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = molfile;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            const bool isRxn = isReactionFormat(molfile);
            int mol = isRxn ? indigoLoadReactionFromString(molfile.toUtf8().constData())
                            : indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoStandardize(mol);
                const char* mf = isRxn ? indigoRxnfile(mol) : indigoMolfile(mol);
                if (mf) result = QString::fromUtf8(mf);
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: standardize load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->standardizeFinished(result); });
    });
}

void IndigoService::calcProperties(const QString &molfile) {
    if (molfile.isEmpty() || isReactionFormat(molfile)) {
        emit propertiesReady(0, 0, "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, 0);
        return;
    }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        double mw = 0, mono = 0, tpsa = 0, logp = 0, molarRefractivity = 0, pka = 0, mostAbundantMass = 0;
        QString mf;
        int atoms = 0, bonds = 0, hba = 0, hbd = 0, rotBonds = 0;
        int heavyAtoms = 0;
        bool isChiral = false;
        int fragmentCount = 0;
        int ringCount = 0;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                mw       = indigoMolecularWeight(mol);
                mono     = indigoMonoisotopicMass(mol);
                atoms    = indigoCountAtoms(mol);
                bonds    = indigoCountBonds(mol);
                tpsa     = indigoTPSA(mol, 1);
                logp     = indigoLogP(mol);
                hba      = indigoNumHydrogenBondAcceptors(mol);
                hbd      = indigoNumHydrogenBondDonors(mol);
                rotBonds = indigoNumRotatableBonds(mol);
                molarRefractivity = indigoMolarRefractivity(mol);
                pka      = indigoPka(mol);
                heavyAtoms = indigoCountHeavyAtoms(mol);
                isChiral = indigoIsChiral(mol) != 0;
                mostAbundantMass = indigoMostAbundantMass(mol);
                fragmentCount = indigoCountComponents(mol);
                ringCount = indigoCountSSSR(mol);
                int fmHandle = indigoMolecularFormula(mol);
                if (fmHandle >= 0) {
                    const char* fmStr = indigoToString(fmHandle);
                    if (fmStr) mf = QString::fromUtf8(fmStr);
                    indigoFree(fmHandle);
                }
                indigoFree(mol);
            } else {
                qWarning() << "Indigo: calcProperties load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [=](IndigoService *s) { emit s->propertiesReady(mw, mono, mf, atoms, bonds, tpsa, logp, hba, hbd, rotBonds, molarRefractivity, pka, heavyAtoms, isChiral, mostAbundantMass, fragmentCount, ringCount); });
    });
}

void IndigoService::calcStereoDescriptors(const QString &molfile) {
    if (molfile.isEmpty() || isReactionFormat(molfile)) {
        emit stereoDescriptorsReady("{\"atoms\":{},\"bonds\":{}}");
        return;
    }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result = "{\"atoms\":{},\"bonds\":{}}";
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                indigoAddCIPStereoDescriptors(mol);
                QJsonObject atomMap;
                // R/S (and rare r/s pseudo-asymmetric) atom descriptors: these are
                // reachable directly via the atom-stereocenter iterator.
                int iter = indigoIterateStereocenters(mol);
                if (iter >= 0) {
                    int atom;
                    while ((atom = indigoNext(iter)) != 0) {
                        if (atom == -1) break;
                        int idx = indigoIndex(atom);
                        int cip = indigoStereocenterCIPDescriptor(atom);
                        int stereoType = indigoStereocenterType(atom);
                        int group = indigoStereocenterGroup(atom);
                        QString label;
                        switch (cip) {
                            case 2: label = "s"; break;
                            case 3: label = "r"; break;
                            case 4: label = "S"; break;
                            case 5: label = "R"; break;
                            // Note: cip values 6/7 (E/Z) never occur here - Indigo
                            // stores double-bond CIP descriptors in a separate,
                            // bond-indexed map that indigoStereocenterCIPDescriptor
                            // (atom-only) cannot read. See the sgroup-based pass below.
                            default: label = ""; break;
                        }
                        if (!label.isEmpty()) {
                            QJsonObject entry;
                            entry["cipLabel"] = label;
                            entry["type"] = stereoType;
                            entry["group"] = group;
                            atomMap[QString::number(idx)] = entry;
                        }
                        indigoFree(atom);
                    }
                    indigoFree(iter);
                }

                // E/Z bond descriptors: no C API exposes Indigo's bond-indexed CIP
                // map directly. The only public path is to have the JSON/KET saver
                // embed them as DAT sgroups (fieldName "INDIGO_CIP_DESC", a
                // two-atom "atoms" list, fieldData "(E)"/"(Z)") and read them back
                // out of the serialized structure.
                QJsonObject bondMap;
                indigoSetOptionBool("json-saving-add-stereo-desc", 1);
                const char* ketStr = indigoJson(mol);
                if (ketStr) {
                    QJsonDocument ketDoc = QJsonDocument::fromJson(QByteArray(ketStr));
                    QJsonObject ketRoot = ketDoc.object();
                    QJsonArray nodes = ketRoot.value("root").toObject().value("nodes").toArray();
                    for (const QJsonValue &nodeVal : nodes) {
                        QString ref = nodeVal.toObject().value("$ref").toString();
                        if (ref.isEmpty()) continue;
                        QJsonObject molObj = ketRoot.value(ref).toObject();
                        if (molObj.value("type").toString() != "molecule") continue;
                        const QJsonArray sgroups = molObj.value("sgroups").toArray();
                        for (const QJsonValue &sgVal : sgroups) {
                            QJsonObject sg = sgVal.toObject();
                            if (sg.value("type").toString() != "DAT") continue;
                            if (sg.value("fieldName").toString() != "INDIGO_CIP_DESC") continue;
                            QJsonArray sgAtoms = sg.value("atoms").toArray();
                            if (sgAtoms.size() != 2) continue; // atom (R/S) entries already covered above
                            QString label = sg.value("fieldData").toString();
                            label.remove('(');
                            label.remove(')');
                            if (label != "E" && label != "Z") continue;
                            int a1 = sgAtoms.at(0).toInt();
                            int a2 = sgAtoms.at(1).toInt();
                            QString key = QString("%1-%2").arg(qMin(a1, a2)).arg(qMax(a1, a2));
                            QJsonObject entry;
                            entry["cipLabel"] = label;
                            bondMap[key] = entry;
                        }
                    }
                }

                indigoFree(mol);
                QJsonObject combined;
                combined["atoms"] = atomMap;
                combined["bonds"] = bondMap;
                result = QString::fromUtf8(QJsonDocument(combined).toJson(QJsonDocument::Compact));
            } else {
                qWarning() << "Indigo: calcStereoDescriptors load failed:" << indigoGetLastError();
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->stereoDescriptorsReady(result); });
    });
}

// Parses indigoCheckObj's report shape ({"checkType": "...(id1,id2,...)"}) into
// structured issues. Shared by checkStructure's molecule and per-reaction-
// component branches, since indigoCheckObj gives back this exact same shape
// for a lone molecule handle or a single reaction-component handle alike.
static QJsonArray parseCheckReport(const QString &rawReport) {
    QJsonArray issuesArray;
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(rawReport.toUtf8(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) return issuesArray;
    static const QSet<QString> atomChecks = {
        "valence", "radical", "pseudoatom", "stereo",
        "ambiguous_h", "3d_coord", "overlap_atom"
    };
    static const QRegularExpression idRe("\\((\\d+(?:,\\d+)*)\\)\\s*$");
    // doc.object() must be held in a named variable — QJsonObject::const_iterator
    // stores a raw pointer back to the QJsonObject it came from, so calling
    // doc.object() separately for constBegin()/constEnd() (as this used to) creates
    // two temporaries; the one behind the begin iterator is destroyed at the end of
    // the for-loop's init-statement, leaving a dangling iterator used for the rest
    // of the loop.
    const QJsonObject checkObj = doc.object();
    for (auto it = checkObj.constBegin(); it != checkObj.constEnd(); ++it) {
        QString checkType = it.key();
        QString value = it.value().toString();
        auto match = idRe.match(value);
        if (match.hasMatch()) {
            QStringList idStrs = match.captured(1).split(',');
            QJsonArray ids;
            for (const QString& s : idStrs) {
                bool ok = false;
                int idx = s.trimmed().toInt(&ok);
                if (ok && idx >= 1) ids.append(idx - 1); // normalize 1-based to 0-based
            }
            if (!ids.isEmpty()) {
                QJsonObject issue;
                issue["type"] = checkType;
                issue["target"] = atomChecks.contains(checkType) ? "atom" : "bond";
                issue["ids"] = ids;
                issuesArray.append(issue);
            }
        }
    }
    return issuesArray;
}

void IndigoService::checkStructure(const QString &molfile) {
    if (molfile.isEmpty()) { emit checkFinished("No structure to check."); emit checkIssuesReady("{\"issues\":[]}"); return; }
    bool isRxn = isReactionFormat(molfile);
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile, isRxn]() {
        QString report;
        QString structured;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            if (isRxn) {
                // indigoCheckObj on the whole reaction handle gives back a report shape
                // (empty-string key, no component association) parseCheckReport can't
                // use -- checking each reactant/product component individually gives the
                // same clean per-type shape the molecule branch already parses, just with
                // ids local to that one component. Inline atom/bond highlighting
                // (checkIssuesReady below) deliberately stays off for reactions: mapping
                // those per-component-local ids back to global canvas ids would need
                // matching chem-core.js's own reactant/product fragment ordering when it
                // serializes a $RXN string, which isn't a safely verifiable assumption
                // here -- a wrong mapping would silently highlight the wrong atom.
                int rx = indigoLoadReactionFromString(molfile.toUtf8().constData());
                if (rx >= 0) {
                    // Same "show the raw per-component check JSON" style the molecule
                    // branch below already uses (it doesn't reformat indigoCheckObj's
                    // text into prose either) -- just labeled by component here.
                    QStringList lines;
                    auto checkComponents = [&](int iterHandle, const char* label) {
                        int idx = 0;
                        int comp;
                        while ((comp = indigoNext(iterHandle)) > 0) {
                            idx++;
                            const char* res = indigoCheckObj(comp, "");
                            QString rawIssue = res ? QString::fromUtf8(res).trimmed() : QString();
                            if (!rawIssue.isEmpty() && rawIssue != "{}") {
                                lines << QString("%1 %2: %3").arg(label).arg(idx).arg(rawIssue);
                            }
                        }
                    };
                    checkComponents(indigoIterateReactants(rx), "Reactant");
                    checkComponents(indigoIterateProducts(rx), "Product");
                    report = lines.isEmpty() ? "No problems found." : lines.join("\n");
                    indigoFree(rx);
                } else {
                    report = QString("Parse error: %1").arg(indigoGetLastError());
                }
                structured = "{\"issues\":[]}";
            } else {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* res = indigoCheckObj(mol, "");
                    report = res ? QString::fromUtf8(res).trimmed() : QString();
                    QJsonParseError perr;
                    QJsonDocument reportDoc = QJsonDocument::fromJson(report.toUtf8(), &perr);
                    QJsonObject reportObj = (perr.error == QJsonParseError::NoError && reportDoc.isObject())
                        ? reportDoc.object() : QJsonObject();
                    // indigoCheckChirality returns 0 when inconsistent (on error), 1 when consistent (or unset)
                    if (indigoCheckChirality(mol) == 0) {
                        reportObj["chirality"] = "Chiral flag is inconsistent with the drawn stereocenters -- "
                            "either the structure is marked chiral with no stereocenters defined, or the chiral "
                            "flag itself is unset/ambiguous. Check the CHIRAL setting and any wedge/hash bonds.";
                    }
                    if (indigoCheckStereo(mol) > 0) {
                        reportObj["stereocenters"] = "One or more marked stereocenters are inconsistent with the "
                            "molecule's own symmetry (invalid or redundant stereo descriptor) -- re-check the "
                            "wedge/hash bonds around them.";
                    }
                    report = QString::fromUtf8(QJsonDocument(reportObj).toJson(QJsonDocument::Compact));
                    QJsonObject structuredObj;
                    structuredObj["issues"] = parseCheckReport(report);
                    structured = QString::fromUtf8(
                        QJsonDocument(structuredObj).toJson(QJsonDocument::Compact));
                    indigoFree(mol);
                } else {
                    report = QString("Parse error: %1").arg(indigoGetLastError());
                }
            }
        } catch (const std::exception &e) {
            report = QString::fromUtf8(e.what());
        }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [report, structured](IndigoService *s) {
            emit s->checkFinished(report.isEmpty() ? "{}" : report);
            emit s->checkIssuesReady(structured.isEmpty() ? "{\"issues\":[]}" : structured);
        });
    });
}

void IndigoService::copyToClipboard(const QString &text) {
    QGuiApplication::clipboard()->setText(text);
}

void IndigoService::substructureSearch(const QString &molfile, const QString &smarts) {
    if (molfile.isEmpty() || smarts.isEmpty() || isReactionFormat(molfile)) {
        emit substructureSearchFinished("{\"error\":\"No structure or empty pattern\"}");
        return;
    }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile, smarts]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
            if (mol >= 0) {
                int query = indigoLoadSmartsFromString(smarts.toUtf8().constData());
                if (query >= 0) {
                    int matcher = indigoSubstructureMatcher(mol, "");
                    if (matcher >= 0) {
                        const int kMaxMatches = 200;
                        QJsonArray matchesArr;
                        int matchIter = indigoIterateMatches(matcher, query);
                        if (matchIter >= 0) {
                            int mapping;
                            while (matchesArr.size() < kMaxMatches && (mapping = indigoNext(matchIter)) != 0) {
                                if (mapping == -1) break;
                                QJsonArray oneMatch;
                                int qAtomIter = indigoIterateAtoms(query);
                                if (qAtomIter >= 0) {
                                    int qAtom;
                                    while ((qAtom = indigoNext(qAtomIter)) != 0) {
                                        if (qAtom == -1) break;
                                        int tAtom = indigoMapAtom(mapping, qAtom);
                                        if (tAtom >= 0) { oneMatch.append(indigoIndex(tAtom) + 1); indigoFree(tAtom); }
                                        indigoFree(qAtom);
                                    }
                                    indigoFree(qAtomIter);
                                }
                                matchesArr.append(oneMatch);
                                indigoFree(mapping);
                            }
                            indigoFree(matchIter);
                        }
                        QJsonObject obj;
                        obj["matchCount"] = matchesArr.size();
                        obj["truncated"] = (matchesArr.size() >= kMaxMatches);
                        obj["matches"] = matchesArr;
                        result = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
                        indigoFree(matcher);
                    } else {
                        result = "{\"error\":\"Failed to build substructure matcher\"}";
                    }
                    indigoFree(query);
                } else {
                    QString err = QString::fromUtf8(indigoGetLastError()).replace("\"", "'");
                    result = QString("{\"error\":\"Invalid SMARTS: %1\"}").arg(err);
                }
                indigoFree(mol);
            } else {
                result = "{\"error\":\"Failed to load structure\"}";
            }
        } catch (const std::exception& e) {
            result = QString("{\"error\":\"%1\"}").arg(QString::fromUtf8(e.what()).replace("\"", "'"));
        } catch (...) {
            result = "{\"error\":\"Unknown exception\"}";
        }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->substructureSearchFinished(result); });
    });
}

// ── Biopolymer loading ────────────────────────────────────────────────────────
// Each function: parse notation → expand monomers to atoms → 2D layout → molfile
void IndigoService::loadBioSequence(const QString &text, const QString &seqType) {
    if (text.isEmpty()) return;
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, text, seqType]() {
        QString result; QString err;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib < 0) { err = "Monomer library could not be loaded."; }
            else {
                int mol = indigoLoadSequenceFromString(
                    text.toUtf8().constData(), seqType.toUtf8().constData(), lib);
                if (mol >= 0) result = indigoBioExpand(mol);
                else err = QString::fromUtf8(indigoGetLastError());
                indigoFree(lib);
            }
        } catch (const std::exception &e) { err = QString::fromUtf8(e.what()); }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, err](IndigoService *s) {
            if (!result.isEmpty()) emit s->biopolymerLoaded(result);
            else emit s->biopolymerLoadError(err.isEmpty() ? "Failed to parse sequence." : err);
        });
    });
}

void IndigoService::loadBioFasta(const QString &text, const QString &seqType) {
    if (text.isEmpty()) return;
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, text, seqType]() {
        QString result; QString err;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib < 0) { err = "Monomer library could not be loaded."; }
            else {
                int mol = indigoLoadFastaFromString(
                    text.toUtf8().constData(), seqType.toUtf8().constData(), lib);
                if (mol >= 0) result = indigoBioExpand(mol);
                else err = QString::fromUtf8(indigoGetLastError());
                indigoFree(lib);
            }
        } catch (const std::exception &e) { err = QString::fromUtf8(e.what()); }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, err](IndigoService *s) {
            if (!result.isEmpty()) emit s->biopolymerLoaded(result);
            else emit s->biopolymerLoadError(err.isEmpty() ? "Failed to parse FASTA." : err);
        });
    });
}

void IndigoService::loadBioHelm(const QString &text) {
    if (text.isEmpty()) return;
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, text]() {
        QString result; QString err;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib < 0) { err = "Monomer library could not be loaded."; }
            else {
                int mol = indigoLoadHelmFromString(text.toUtf8().constData(), lib);
                if (mol >= 0) result = indigoBioExpand(mol);
                else err = QString::fromUtf8(indigoGetLastError());
                indigoFree(lib);
            }
        } catch (const std::exception &e) { err = QString::fromUtf8(e.what()); }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, err](IndigoService *s) {
            if (!result.isEmpty()) emit s->biopolymerLoaded(result);
            else emit s->biopolymerLoadError(err.isEmpty() ? "Failed to parse HELM." : err);
        });
    });
}

void IndigoService::loadBioIdt(const QString &text) {
    if (text.isEmpty()) return;
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, text]() {
        QString result; QString err;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib < 0) { err = "Monomer library could not be loaded."; }
            else {
                int mol = indigoLoadIdtFromString(text.toUtf8().constData(), lib);
                if (mol >= 0) result = indigoBioExpand(mol);
                else err = QString::fromUtf8(indigoGetLastError());
                indigoFree(lib);
            }
        } catch (const std::exception &e) { err = QString::fromUtf8(e.what()); }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, err](IndigoService *s) {
            if (!result.isEmpty()) emit s->biopolymerLoaded(result);
            else emit s->biopolymerLoadError(err.isEmpty() ? "Failed to parse IDT." : err);
        });
    });
}

void IndigoService::loadBioAxoLabs(const QString &text) {
    if (text.isEmpty()) return;
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, text]() {
        QString result; QString err;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib < 0) { err = "Monomer library could not be loaded."; }
            else {
                int mol = indigoLoadAxoLabsFromString(text.toUtf8().constData(), lib);
                if (mol >= 0) result = indigoBioExpand(mol);
                else err = QString::fromUtf8(indigoGetLastError());
                indigoFree(lib);
            }
        } catch (const std::exception &e) { err = QString::fromUtf8(e.what()); }
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, err](IndigoService *s) {
            if (!result.isEmpty()) emit s->biopolymerLoaded(result);
            else emit s->biopolymerLoadError(err.isEmpty() ? "Failed to parse AxoLabs." : err);
        });
    });
}

// ── Biopolymer export ─────────────────────────────────────────────────────────
// Each function: load molfile → call Indigo serialiser → emit ready signal

void IndigoService::exportBioSequence(const QString &molfile) {
    if (molfile.isEmpty()) { emit bioSequenceReady(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib >= 0) {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* s = indigoSequence(mol, lib);
                    if (s) result = QString::fromUtf8(s);
                    indigoFree(mol);
                }
                indigoFree(lib);
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->bioSequenceReady(result); });
    });
}

void IndigoService::exportBioFasta(const QString &molfile) {
    if (molfile.isEmpty()) { emit bioFastaReady(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib >= 0) {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* s = indigoFasta(mol, lib);
                    if (s) result = QString::fromUtf8(s);
                    indigoFree(mol);
                }
                indigoFree(lib);
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->bioFastaReady(result); });
    });
}

void IndigoService::exportBioHelm(const QString &molfile) {
    if (molfile.isEmpty()) { emit bioHelmReady(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib >= 0) {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* s = indigoHelm(mol, lib);
                    if (s) result = QString::fromUtf8(s);
                    indigoFree(mol);
                }
                indigoFree(lib);
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->bioHelmReady(result); });
    });
}

void IndigoService::exportBioIdt(const QString &molfile) {
    if (molfile.isEmpty()) { emit bioIdtReady(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib >= 0) {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* s = indigoIdt(mol, lib);
                    if (s) result = QString::fromUtf8(s);
                    indigoFree(mol);
                }
                indigoFree(lib);
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->bioIdtReady(result); });
    });
}

void IndigoService::exportBioAxoLabs(const QString &molfile) {
    if (molfile.isEmpty()) { emit bioAxoLabsReady(""); return; }
    QPointer<IndigoService> self = this;
    (void)QtConcurrent::run([self, molfile]() {
        QString result;
        unsigned long long sid = indigoAllocSessionId();
        indigoSetSessionId(sid);
        try {
            int lib = loadMonomerLibrary();
            if (lib >= 0) {
                int mol = indigoLoadMoleculeFromString(molfile.toUtf8().constData());
                if (mol >= 0) {
                    const char* s = indigoAxoLabs(mol, lib);
                    if (s) result = QString::fromUtf8(s);
                    indigoFree(mol);
                }
                indigoFree(lib);
            }
        } catch (...) {}
        indigoReleaseSessionId(sid);
        emitOnGuiThread(self, [result](IndigoService *s) { emit s->bioAxoLabsReady(result); });
    });
}
