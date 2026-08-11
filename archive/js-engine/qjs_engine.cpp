#include "qjs_engine.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

QjsEngine::QjsEngine(const QString &workerJsPath,
                     std::function<void(const QString &)> onLine,
                     std::function<void(const QString &)> onFatal)
    : m_onLine(std::move(onLine)), m_onFatal(std::move(onFatal))
{
    m_rt = JS_NewRuntime();
    m_ctx = JS_NewContext(m_rt);
    JS_SetContextOpaque(m_ctx, this);

    QString workerDir = QFileInfo(workerJsPath).absolutePath();

    JSValue global = JS_GetGlobalObject(m_ctx);
    JSValue native = JS_NewObject(m_ctx);
    JS_SetPropertyStr(m_ctx, native, "readFileSync", JS_NewCFunction(m_ctx, native_readFileSync, "readFileSync", 1));
    JS_SetPropertyStr(m_ctx, native, "existsSync", JS_NewCFunction(m_ctx, native_existsSync, "existsSync", 1));
    JS_SetPropertyStr(m_ctx, native, "log", JS_NewCFunction(m_ctx, native_log, "log", 1));
    JS_SetPropertyStr(m_ctx, native, "errorLog", JS_NewCFunction(m_ctx, native_errorLog, "errorLog", 1));
    JS_SetPropertyStr(m_ctx, native, "fatal", JS_NewCFunction(m_ctx, native_fatal, "fatal", 1));
    {
        QByteArray dirUtf8 = workerDir.toUtf8();
        JS_SetPropertyStr(m_ctx, native, "dirname", JS_NewStringLen(m_ctx, dirUtf8.constData(), dirUtf8.size()));
    }
    JS_SetPropertyStr(m_ctx, global, "__native", native);
    JS_FreeValue(m_ctx, global);

    QFile f(workerJsPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "QjsEngine: could not open" << workerJsPath;
        if (m_onFatal) m_onFatal(QStringLiteral("Chemistry engine script not found: %1").arg(workerJsPath));
        return;
    }
    QByteArray src = f.readAll();

    JSValue result = JS_Eval(m_ctx, src.constData(), src.size(), "v8_worker.js", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exc = JS_GetException(m_ctx);
        const char *msg = JS_ToCString(m_ctx, exc);
        qWarning() << "QjsEngine: failed to eval v8_worker.js:" << msg;
        if (m_onFatal) m_onFatal(QStringLiteral("Chemistry engine failed to load: %1").arg(QString::fromUtf8(msg)));
        JS_FreeCString(m_ctx, msg);
        JS_FreeValue(m_ctx, exc);
    } else {
        m_valid = true;
    }
    JS_FreeValue(m_ctx, result);
}

QjsEngine::~QjsEngine()
{
    if (m_ctx) JS_FreeContext(m_ctx);
    if (m_rt) JS_FreeRuntime(m_rt);
}

JSValue QjsEngine::variantToJs(const QVariant &v)
{
    if (!v.isValid() || v.isNull()) return JS_NULL;
    switch (v.typeId()) {
        case QMetaType::Bool:
            return JS_NewBool(m_ctx, v.toBool());
        case QMetaType::Int:
        case QMetaType::LongLong:
        case QMetaType::UInt:
        case QMetaType::ULongLong:
            return JS_NewFloat64(m_ctx, v.toDouble()); // safe: worker args are ids/coords, never >2^53
        case QMetaType::Double:
            return JS_NewFloat64(m_ctx, v.toDouble());
        case QMetaType::QString: {
            QByteArray utf8 = v.toString().toUtf8();
            return JS_NewStringLen(m_ctx, utf8.constData(), utf8.size());
        }
        case QMetaType::QVariantList: {
            // e.g. V8Process::addRing's `coords` -- wrapped as a single nested
            // QVariant(QVariantList) inside sendCommand's args. Without this case
            // it fell through to the default branch below: toDouble() fails on a
            // list, and QVariant::toString() on a list returns "" -- so coords
            // arrived in JS as an empty string instead of an array, silently
            // breaking every ring tool that goes through addRing.
            QVariantList list = v.toList();
            JSValue arr = JS_NewArray(m_ctx);
            for (int i = 0; i < list.size(); ++i) {
                JS_SetPropertyUint32(m_ctx, arr, static_cast<uint32_t>(i), variantToJs(list.at(i)));
            }
            return arr;
        }
        case QMetaType::QVariantMap: {
            QVariantMap map = v.toMap();
            JSValue obj = JS_NewObject(m_ctx);
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                QByteArray keyUtf8 = it.key().toUtf8();
                JS_SetPropertyStr(m_ctx, obj, keyUtf8.constData(), variantToJs(it.value()));
            }
            return obj;
        }
        default: {
            bool ok = false;
            double d = v.toDouble(&ok);
            if (ok) return JS_NewFloat64(m_ctx, d);
            QByteArray utf8 = v.toString().toUtf8();
            return JS_NewStringLen(m_ctx, utf8.constData(), utf8.size());
        }
    }
}

void QjsEngine::dispatch(const QString &cmd, const QVariantList &args)
{
    if (!m_valid) return;

    JSValue global = JS_GetGlobalObject(m_ctx);
    JSValue fn = JS_GetPropertyStr(m_ctx, global, "__dispatchCommand");

    JSValue jsArgs = JS_NewArray(m_ctx);
    for (int i = 0; i < args.size(); ++i) {
        JS_SetPropertyUint32(m_ctx, jsArgs, static_cast<uint32_t>(i), variantToJs(args[i]));
    }
    QByteArray cmdUtf8 = cmd.toUtf8();
    JSValue jsCmd = JS_NewStringLen(m_ctx, cmdUtf8.constData(), cmdUtf8.size());

    JSValueConst callArgs[2] = { jsCmd, jsArgs };
    JSValue ret = JS_Call(m_ctx, fn, global, 2, callArgs);
    if (JS_IsException(ret)) {
        JSValue exc = JS_GetException(m_ctx);
        const char *msg = JS_ToCString(m_ctx, exc);
        qWarning() << "QjsEngine: dispatch(" << cmd << ") threw:" << msg;
        if (m_onFatal) m_onFatal(QStringLiteral("Chemistry engine error: %1").arg(QString::fromUtf8(msg)));
        JS_FreeCString(m_ctx, msg);
        JS_FreeValue(m_ctx, exc);
    }
    JS_FreeValue(m_ctx, ret);
    JS_FreeValue(m_ctx, jsCmd);
    JS_FreeValue(m_ctx, jsArgs);
    JS_FreeValue(m_ctx, fn);
    JS_FreeValue(m_ctx, global);
}

JSValue QjsEngine::native_readFileSync(JSContext *ctx, JSValueConst /*this_val*/, int argc, JSValueConst *argv)
{
    if (argc < 1) return JS_ThrowTypeError(ctx, "readFileSync requires a path");
    const char *pathC = JS_ToCString(ctx, argv[0]);
    QFile f(QString::fromUtf8(pathC));
    JS_FreeCString(ctx, pathC);
    if (!f.open(QIODevice::ReadOnly)) return JS_ThrowTypeError(ctx, "readFileSync: cannot open file");
    QByteArray content = f.readAll();
    return JS_NewStringLen(ctx, content.constData(), content.size());
}

JSValue QjsEngine::native_existsSync(JSContext *ctx, JSValueConst /*this_val*/, int argc, JSValueConst *argv)
{
    if (argc < 1) return JS_NewBool(ctx, false);
    const char *pathC = JS_ToCString(ctx, argv[0]);
    bool exists = QFile::exists(QString::fromUtf8(pathC));
    JS_FreeCString(ctx, pathC);
    return JS_NewBool(ctx, exists);
}

JSValue QjsEngine::native_log(JSContext *ctx, JSValueConst /*this_val*/, int argc, JSValueConst *argv)
{
    auto *self = static_cast<QjsEngine *>(JS_GetContextOpaque(ctx));
    if (argc >= 1 && self && self->m_onLine) {
        const char *s = JS_ToCString(ctx, argv[0]);
        self->m_onLine(QString::fromUtf8(s));
        JS_FreeCString(ctx, s);
    }
    return JS_UNDEFINED;
}

JSValue QjsEngine::native_errorLog(JSContext *ctx, JSValueConst /*this_val*/, int argc, JSValueConst *argv)
{
    if (argc >= 1) {
        const char *s = JS_ToCString(ctx, argv[0]);
        qWarning().noquote() << "[worker]" << s;
        JS_FreeCString(ctx, s);
    }
    return JS_UNDEFINED;
}

JSValue QjsEngine::native_fatal(JSContext *ctx, JSValueConst /*this_val*/, int argc, JSValueConst *argv)
{
    auto *self = static_cast<QjsEngine *>(JS_GetContextOpaque(ctx));
    if (argc >= 1 && self && self->m_onFatal) {
        const char *s = JS_ToCString(ctx, argv[0]);
        self->m_onFatal(QString::fromUtf8(s));
        JS_FreeCString(ctx, s);
    }
    return JS_UNDEFINED;
}
