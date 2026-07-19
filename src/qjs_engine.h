#pragma once
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <functional>

extern "C" {
#include "quickjs.h"
}

// Embeds v8_worker.js + chem-core.js directly in-process via QuickJS-ng,
// replacing the Node.js child-process transport. Same command-in / JSON-line-out
// shape V8Process already speaks -- see the "Node.js IPC Interface" section this
// replaces in src/v8_worker.js (now "Transport", guarded by `_hasNative`).
class QjsEngine {
public:
    // workerJsPath: absolute path to src/v8_worker.js (its directory becomes
    //   __native.dirname, i.e. what __dirname resolved to under Node).
    // onLine: called once per JSON line the worker emits via console.log --
    //   feed this to the same parsing logic V8Process::onReadyReadStandardOutput
    //   used to apply to lines read from the child process's stdout.
    // onFatal: called on an uncaught JS exception or explicit process.exit();
    //   wire to V8Process::errorOccurred.
    QjsEngine(const QString &workerJsPath,
              std::function<void(const QString &)> onLine,
              std::function<void(const QString &)> onFatal);
    ~QjsEngine();

    bool isValid() const { return m_valid; }

    // Calls __dispatchCommand(cmd, args) directly with real JS values --
    // no JSON stringify/parse round-trip, unlike the old stdin/stdout pipe.
    void dispatch(const QString &cmd, const QVariantList &args);

private:
    JSRuntime *m_rt = nullptr;
    JSContext *m_ctx = nullptr;
    bool m_valid = false;
    std::function<void(const QString &)> m_onLine;
    std::function<void(const QString &)> m_onFatal;

    JSValue variantToJs(const QVariant &v);

    static JSValue native_readFileSync(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue native_existsSync(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue native_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue native_errorLog(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue native_fatal(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
};
