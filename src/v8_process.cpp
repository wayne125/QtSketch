#include "v8_process.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QClipboard>
#define _USE_MATH_DEFINES
#include <cmath>

V8Process::V8Process(QObject *parent) : QObject(parent) {
    QString appDir = QCoreApplication::applicationDirPath();
    // Walk up from the binary to find src/v8_worker.js regardless of build layout depth.
    // Same discovery logic as before Node was removed -- the file is now read and
    // JS_Eval'd directly instead of spawned, but it still lives in the same place.
    QString scriptPath;
    QDir dir(appDir);
    for (int i = 0; i < 5; ++i) {
        QString candidate = dir.filePath("src/v8_worker.js");
        if (QFile::exists(candidate)) {
            scriptPath = candidate;
            break;
        }
        if (!dir.cdUp()) break;
    }
    if (scriptPath.isEmpty()) {
        qWarning() << "V8Process: could not locate src/v8_worker.js from" << appDir;
        QTimer::singleShot(0, this, [this]() {
            emit errorOccurred("Chemistry engine script not found. Please reinstall the application.");
        });
        return;
    }

    m_engine = std::make_unique<QjsEngine>(
        scriptPath,
        [this](const QString &line) { handleWorkerLine(line); },
        [this](const QString &msg) {
            // Deferred for the same reason as the scriptPath.isEmpty() case above:
            // this can fire synchronously from inside the QjsEngine constructor,
            // i.e. before DocumentManager::addDocument() has even returned this
            // V8Process's docId to QML -- emitting immediately means no QML
            // Connections exists yet and the signal is silently lost.
            QTimer::singleShot(0, this, [this, msg]() { emit errorOccurred(msg); });
        }
    );
}

V8Process::~V8Process() = default;

void V8Process::sendCommand(const QString& cmd, const QVariantList& args) {
    if (!m_engine || !m_engine->isValid()) {
        qWarning() << "V8Process is not running!";
        return;
    }
    m_engine->dispatch(cmd, args);
}

void V8Process::init() { sendCommand("init"); }
void V8Process::loadMol(const QString& molfile) { sendCommand("loadMol", {molfile}); }
void V8Process::addAtom(const QString& label, double x, double y, int charge) { sendCommand("addAtom", {label, x, y, charge}); }
void V8Process::addBondAndAtom(int beginAtomId, const QString& endAtomLabel, double x, double y, int bondType, int stereo) { sendCommand("addBondAndAtom", {beginAtomId, endAtomLabel, x, y, bondType, stereo}); }
void V8Process::addBondBetweenCoords(double x1, double y1, double x2, double y2, int type, int stereo) { sendCommand("addBondBetweenCoords", {x1, y1, x2, y2, type, stereo}); }
void V8Process::addBond(int beginAtomId, int endAtomId, int bondType, int stereoDir) { sendCommand("addBond", {beginAtomId, endAtomId, bondType, stereoDir}); }
void V8Process::addRing(const QVariantList& coords, bool aromatic) { QVariantList wrapper; wrapper.append(QVariant(coords)); wrapper.append(aromatic); sendCommand("addRing", wrapper); }
void V8Process::deleteAtomById(int id) { sendCommand("deleteAtomById", {id}); }
void V8Process::deleteBondById(int id) { sendCommand("deleteBondById", {id}); }
void V8Process::deleteSelection() { sendCommand("deleteSelection"); }
void V8Process::undo() { sendCommand("undo"); }
void V8Process::redo() { sendCommand("redo"); }
void V8Process::copySelection() { sendCommand("copySelection"); }
void V8Process::cutSelection() { sendCommand("cutSelection"); }
void V8Process::pasteSelection(double cx, double cy) { sendCommand("pasteSelection", {cx, cy}); }
void V8Process::selectAll() { sendCommand("selectAll"); }
void V8Process::clearCanvas() { sendCommand("clearCanvas"); }
void V8Process::loadBenzene() { sendCommand("loadBenzene"); }
void V8Process::deserializeMol(const QString& data) { sendCommand("deserializeMol", {data}); }
void V8Process::requestStructure(const QString& fmt, const QString& reqId) {
    sendCommand("getStructure", {fmt, reqId});
}
void V8Process::requestSelectionStructure(const QString& reqId) {
    sendCommand("getSelectionStructure", {reqId});
}
void V8Process::requestSerialize(const QString& reqId) {
    requestStructure("mol", reqId);
}
QString V8Process::getStructure(const QString& fmt) {
    static int counter = 0;
    int myId = ++counter;
    QString reqId = "SYNC_" + fmt + "_" + QString::number(myId);

    QString result;
    QEventLoop loop;
    auto conn = connect(this, &V8Process::structureReady, [&](const QString &rid, const QString &data) {
        if (rid == reqId) {
            result = data;
            loop.quit();
        }
    });

    sendCommand("getStructure", {fmt, reqId});
    
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    
    disconnect(conn);
    return result;
}
QString V8Process::serializeMol() {
    return getStructure("mol");
}
void V8Process::loadStructure(const QString& format, const QString& data, bool centerOnPage) {
    if (format == "mol") {
        sendCommand("loadMol", {data, centerOnPage});
    } else if (format == "sdf") {
        sendCommand("deserializeSdf", {data});
    } else if (format == "ket") {
        sendCommand("deserializeKet", {data});
    }
}
void V8Process::insertFunctionalGroup(const QString& fgName, double cx, double cy, int targetAtomId, bool fullStructure) {
    sendCommand("insertFunctionalGroup", {fgName, cx, cy, targetAtomId, fullStructure});
}
void V8Process::insertLibraryTemplateFused(const QString& fgName, double cx, double cy, int targetBondId) {
    sendCommand("insertLibraryTemplateFused", {fgName, cx, cy, targetBondId});
}
void V8Process::requestSaltsAndSolventsList() { sendCommand("getSaltsAndSolventsList"); }
void V8Process::requestFunctionalGroupsList() { sendCommand("getFunctionalGroupsList"); }
void V8Process::requestTemplateLibraryList() { sendCommand("getTemplateLibraryList"); }
void V8Process::requestTemplateThumbnail(const QString& name, const QString& reqId) { sendCommand("getTemplateThumbnail", {name, reqId}); }
void V8Process::addRxnArrow(double x, double y, const QString& mode) { sendCommand("addRxnArrow", {x, y, mode}); }
void V8Process::addRxnPlus(double x, double y) { sendCommand("addRxnPlus", {x, y}); }
void V8Process::addCurvedArrow(double x1, double y1, double ctrlX, double ctrlY, double x2, double y2) { sendCommand("addCurvedArrow", {x1, y1, ctrlX, ctrlY, x2, y2}); }
void V8Process::setRxnArrowMode(int id, const QString& mode) { sendCommand("setRxnArrowMode", {id, mode}); }
void V8Process::setRxnArrowConditions(int id, const QString& above, const QString& below) { sendCommand("setRxnArrowConditions", {id, above, below}); }
void V8Process::setStereoFlags(const QString& type, int groupId) { sendCommand("setStereoFlags", {type, groupId}); }
void V8Process::transformSelection(const QString& mode) { sendCommand("transformSelection", {mode}); }
void V8Process::addChain(double x1, double y1, double x2, double y2) { sendCommand("addChain", {x1, y1, x2, y2}); }
void V8Process::addText(const QString& content, double x, double y, bool bold, bool italic) { sendCommand("addText", {content, x, y, bold, italic}); }
void V8Process::updateText(int id, const QString& content, bool bold, bool italic) { sendCommand("updateText", {id, content, bold, italic}); }
void V8Process::deleteText(int id) { sendCommand("deleteText", {id}); }
void V8Process::addImage(const QString& base64DataUri, double cx, double cy, double halfW, double halfH) { sendCommand("addImage", {base64DataUri, cx, cy, halfW, halfH}); }
void V8Process::deleteImage(int id) { sendCommand("deleteImage", {id}); }
void V8Process::addRGroup(int rgroupNumber) { sendCommand("addRGroup", {rgroupNumber}); }
void V8Process::deleteRGroup(int rgroupNumber) { sendCommand("deleteRGroup", {rgroupNumber}); }
void V8Process::setRGroupLogic(int rgroupNumber, const QString& range, bool resth, int ifthen) { sendCommand("setRGroupLogic", {rgroupNumber, range, resth, ifthen}); }
void V8Process::addRGroupMember(int rgroupNumber) { sendCommand("addRGroupMember", {rgroupNumber}); }
void V8Process::removeRGroupMember(int rgroupNumber, int fragId) { sendCommand("removeRGroupMember", {rgroupNumber, fragId}); }
void V8Process::setAtomQueryList(int atomId, const QString& elementsCsv, bool notList) { sendCommand("setAtomQueryList", {atomId, elementsCsv, notList}); }
void V8Process::clearAtomQueryList(int atomId, const QString& fallbackLabel) { sendCommand("clearAtomQueryList", {atomId, fallbackLabel}); }
void V8Process::addMultitailArrow(double x, double y) { sendCommand("addMultitailArrow", {x, y}); }
void V8Process::deleteMultitailArrow(int id) { sendCommand("deleteMultitailArrow", {id}); }
void V8Process::addMultitailArrowTail(int id) { sendCommand("addMultitailArrowTail", {id}); }
void V8Process::changeAtomLabel(int id, const QString& label) { sendCommand("changeAtomLabel", {id, label}); }
void V8Process::setAtomMapping(int id, int mapping) { sendCommand("setAtomMapping", {id, mapping}); }
void V8Process::changeBondType(int id, int type, int stereo) { sendCommand("changeBondType", {id, type, stereo}); }
void V8Process::changeAtomCharge(int id, int charge) { sendCommand("changeAtomCharge", {id, charge}); }
void V8Process::setAttachmentPoint(int id, int order) { sendCommand("setAttachmentPoint", {id, order}); }
void V8Process::changeAtomIsotope(int id, int isotope) { sendCommand("changeAtomIsotope", {id, isotope}); }
void V8Process::changeAtomRadical(int id, int radical) { sendCommand("changeAtomRadical", {id, radical}); }
void V8Process::changeAtomValence(int id, int valence) { sendCommand("changeAtomValence", {id, valence}); }
void V8Process::requestAtomProperties(int id) { sendCommand("getAtomProperties", {id}); }
void V8Process::selectByRect(double x1, double y1, double x2, double y2) { sendCommand("selectByRect", {x1, y1, x2, y2}); }
void V8Process::addSelectionByRect(double x1, double y1, double x2, double y2) { sendCommand("addSelectionByRect", {x1, y1, x2, y2}); }
void V8Process::selectByLasso(const QVariantList& pointsFlat) { sendCommand("selectByLasso", {QVariant(pointsFlat)}); }
void V8Process::selectItem(const QVariant& atomId, const QVariant& bondId, const QVariant& rxnArrowId, const QVariant& rxnPlusId, const QVariant& multitailArrowId) { sendCommand("selectItem", {atomId, bondId, rxnArrowId, rxnPlusId, multitailArrowId}); }
void V8Process::addItemToSelection(const QVariant& atomId, const QVariant& bondId) { sendCommand("addItemToSelection", {atomId, bondId}); }
void V8Process::removeItemFromSelection(const QVariant& atomId, const QVariant& bondId) { sendCommand("removeItemFromSelection", {atomId, bondId}); }
void V8Process::selectFragment(const QVariant& atomId, const QVariant& bondId) { sendCommand("selectFragment", {atomId, bondId}); }
void V8Process::moveSelection(double dx, double dy) { sendCommand("moveSelection", {dx, dy}); }
void V8Process::commitMove() { sendCommand("commitMove"); }
void V8Process::rotateSelectionLive(double angleDelta) { sendCommand("rotateSelectionLive", {angleDelta}); }
void V8Process::commitRotate() { sendCommand("commitRotate"); }
void V8Process::scaleSelectionLive(double factor, double anchorX, double anchorY) { sendCommand("scaleSelectionLive", {factor, anchorX, anchorY}); }
void V8Process::commitScale() { sendCommand("commitScale"); }
void V8Process::centerStructure() { sendCommand("centerStructure"); }
void V8Process::normalizeStructure() { sendCommand("normalizeStructure"); }

void V8Process::alignAtoms(const QString& direction) { sendCommand("alignAtoms", {direction}); }
void V8Process::distributeAtoms(const QString& direction) { sendCommand("distributeAtoms", {direction}); }

void V8Process::setStereoDescriptors(const QString& jsonMap) { sendCommand("setStereoDescriptors", {jsonMap}); }
void V8Process::setCheckIssues(const QString& jsonMap) { sendCommand("setCheckIssues", {jsonMap}); }

QString V8Process::getOsClipboardText() const {
    return QGuiApplication::clipboard()->text();
}

void V8Process::setOsClipboardText(const QString& text) {
    QGuiApplication::clipboard()->setText(text);
}

void V8Process::requestClipboardKet() {
    sendCommand("getClipboardAsKet", {});
}

void V8Process::importKetAtPosition(const QString& ket, double cx, double cy) {
    sendCommand("importKetAtPosition", {ket, cx, cy});
}

void V8Process::setOverlayState(const QVariantMap& state) {
    if (m_overlayState != state) {
        m_overlayState = state;
        emit overlayStateChanged();
    }
}

static double getAngle(double x1, double y1, double x2, double y2) {
    return std::atan2(y2 - y1, x2 - x1);
}

double V8Process::getLargestEmptyAngle(const QVariant& atomId, const QVariantMap& primitives) {
    QVariantMap atomsById = primitives.value("atomsById").toMap();
    if (!atomsById.contains(atomId.toString())) return 0;
    
    QVariantMap atom = atomsById.value(atomId.toString()).toMap();
    double ax = atom.value("x").toDouble();
    double ay = atom.value("y").toDouble();
    
    QList<double> neighbors;
    QVariantList bonds = primitives.value("bonds").toList();
    for (const QVariant& bVar : bonds) {
        QVariantMap b = bVar.toMap();
        QVariant beginId = b.value("begin");
        QVariant endId = b.value("end");
        if (beginId == atomId) {
            QVariantMap n1 = atomsById.value(endId.toString()).toMap();
            if (!n1.isEmpty()) neighbors.append(getAngle(ax, ay, n1.value("x").toDouble(), n1.value("y").toDouble()));
        } else if (endId == atomId) {
            QVariantMap n2 = atomsById.value(beginId.toString()).toMap();
            if (!n2.isEmpty()) neighbors.append(getAngle(ax, ay, n2.value("x").toDouble(), n2.value("y").toDouble()));
        }
    }
    
    if (neighbors.isEmpty()) return 0;
    if (neighbors.size() == 1) return neighbors.first() + 2.61799;
    
    std::sort(neighbors.begin(), neighbors.end());
    double maxGap = 0;
    double bestAngle = 0;
    for (int i = 0; i < neighbors.size(); i++) {
        double a1 = neighbors[i];
        double a2 = neighbors[(i + 1) % neighbors.size()];
        double gap = a2 - a1;
        while (gap <= 0) gap += M_PI * 2;
        if (gap > maxGap) {
            maxGap = gap;
            bestAngle = a1 + gap / 2;
        }
    }
    return bestAngle;
}

QVariantList V8Process::getRingPreviewCoords(int n, double cx, double cy, const QVariant& hoverAtomId, const QVariant& hoverBondId) {
    // Page/canvas boundary hard clamp, mirroring src/v8_worker.js's PAGE_MIN_X/
    // MAX_X/MIN_Y/MAX_Y (kept in sync manually -- must match). Only affects the
    // free-floating ring case below: when fusing onto an existing hover atom/bond
    // (checked further down), the ring's geometry is derived from that atom's own
    // already-in-bounds position instead, not from cx/cy, so clamping here can't
    // misalign a fused ring's seam.
    cx = std::max(-30.0, std::min(30.0, cx));
    cy = std::max(-21.0, std::min(21.0, cy));
    QVariantList coords;
    // Must match chem-core.js's StandardBondLength (MonomerSize * 2 = 0.75 * 2 = 1.5),
    // not an arbitrary 1.0 -- this function's output is used directly as the final
    // ring geometry passed to addRing() (see ChemCanvas.qml's call sites), not just a
    // cosmetic hover preview, so every ring placed via a TEMPLATE_ tool was rendering
    // at 1.0/1.5 (~67%) of the standard bond length used everywhere else in the app.
    double L = 1.5;
    double R = L / (2 * std::sin(M_PI / n));
    double angleStep = (2 * M_PI) / n;
    
    if (!hoverBondId.isNull() && hoverBondId.isValid()) {
        QVariantMap bond;
        QVariantList bonds = m_primitives.value("bonds").toList();
        for (const QVariant& bVar : bonds) {
            if (bVar.toMap().value("id") == hoverBondId) {
                bond = bVar.toMap();
                break;
            }
        }
        if (!bond.isEmpty()) {
            QVariantMap atomsById = m_primitives.value("atomsById").toMap();
            QVariantMap a1 = atomsById.value(bond.value("begin").toString()).toMap();
            QVariantMap a2 = atomsById.value(bond.value("end").toString()).toMap();
            if (!a1.isEmpty() && !a2.isEmpty()) {
                double a1x = a1.value("x").toDouble();
                double a1y = a1.value("y").toDouble();
                double a2x = a2.value("x").toDouble();
                double a2y = a2.value("y").toDouble();
                double midX = (a1x + a2x) / 2;
                double midY = (a1y + a2y) / 2;
                double vx = a2x - a1x;
                double vy = a2y - a1y;
                double len = std::sqrt(vx*vx + vy*vy);
                if (len != 0) {
                    double ux = vx / len;
                    double uy = vy / len;
                    double nx1 = -uy, ny1 = ux;
                    double nx2 = uy, ny2 = -ux;
                    double score1 = 0, score2 = 0;
                    for (auto it = atomsById.constBegin(); it != atomsById.constEnd(); ++it) {
                        if (it.key() == bond.value("begin").toString() || it.key() == bond.value("end").toString()) continue;
                        QVariantMap a = it.value().toMap();
                        double dx = a.value("x").toDouble() - midX;
                        double dy = a.value("y").toDouble() - midY;
                        double dot1 = dx * nx1 + dy * ny1;
                        double dot2 = dx * nx2 + dy * ny2;
                        if (dot1 > 0) score1 += dot1;
                        if (dot2 > 0) score2 += dot2;
                    }
                    double nx = (score1 < score2) ? nx1 : nx2;
                    double ny = (score1 < score2) ? ny1 : ny2;
                    double h = L / (2 * std::tan(M_PI / n));
                    double center_x = midX + nx * h;
                    double center_y = midY + ny * h;
                    double startAngle = getAngle(center_x, center_y, a1x, a1y);
                    double angle2 = getAngle(center_x, center_y, a2x, a2y);
                    double angleDiff = angle2 - startAngle;
                    while (angleDiff <= -M_PI) angleDiff += 2 * M_PI;
                    while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
                    double step = (angleDiff > 0) ? angleStep : -angleStep;
                    
                    QVariantMap p1; p1["x"] = a2x; p1["y"] = a2y; coords.append(p1);
                    QVariantMap p2; p2["x"] = a1x; p2["y"] = a1y; coords.append(p2);
                    for (int i = 2; i < n; i++) {
                        double a = startAngle - (i - 1) * step;
                        QVariantMap p;
                        p["x"] = center_x + R * std::cos(a);
                        p["y"] = center_y + R * std::sin(a);
                        coords.append(p);
                    }
                    return coords;
                }
            }
        }
    }
    
    if (!hoverAtomId.isNull() && hoverAtomId.isValid()) {
        QVariantMap atomsById = m_primitives.value("atomsById").toMap();
        QVariantMap atom = atomsById.value(hoverAtomId.toString()).toMap();
        if (!atom.isEmpty()) {
            double ax = atom.value("x").toDouble();
            double ay = atom.value("y").toDouble();
            double emptyAngle = getLargestEmptyAngle(hoverAtomId, m_primitives);
            double center_x = ax + R * std::cos(emptyAngle);
            double center_y = ay + R * std::sin(emptyAngle);
            double startAngle = getAngle(center_x, center_y, ax, ay);
            for (int i = 0; i < n; i++) {
                double a = startAngle + i * angleStep;
                QVariantMap p;
                p["x"] = center_x + R * std::cos(a);
                p["y"] = center_y + R * std::sin(a);
                coords.append(p);
            }
            return coords;
        }
    }
    
    double startAngle = -M_PI / 2;
    if (n % 2 == 0) startAngle += angleStep / 2;
    for (int i = 0; i < n; i++) {
        double a = startAngle + i * angleStep;
        QVariantMap p;
        p["x"] = cx + R * std::cos(a);
        p["y"] = cy + R * std::sin(a);
        coords.append(p);
    }
    return coords;
}

void V8Process::handleWorkerLine(const QString &line) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON from worker:" << error.errorString() << line;
        return;
    }

    QJsonObject obj = doc.object();
    if (obj["type"].toString() == "structureResponse") {
        emit structureReady(obj["reqId"].toString(), obj["data"].toString());
        return;
    }

    if (obj["status"].toString() == "ok") {
        m_primitives = obj["state"].toVariant().toMap();
        m_selection = obj["selection"].toVariant().toMap();
        emit primitivesChanged();
        emit selectionChanged();

        emit stateUpdated(
            m_primitives,
            m_selection,
            obj["isDirty"].toBool(),
            obj["canUndo"].toBool(),
            obj["canRedo"].toBool(),
            obj["result"].toVariant()
        );
    } else if (obj["status"].toString() == "error") {
        qWarning() << "Worker Error:" << obj["message"].toString();
        QFile f("worker_error.log"); if(f.open(QIODevice::Append)) { f.write(obj["message"].toString().toUtf8() + "\n"); f.close(); }
        emit errorOccurred(obj["message"].toString());
    }
}

