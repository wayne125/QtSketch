.pragma library
var ChemCore = (() => {
  var __create = Object.create;
  var __defProp = Object.defineProperty;
  var __defProps = Object.defineProperties;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropDescs = Object.getOwnPropertyDescriptors;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __getOwnPropSymbols = Object.getOwnPropertySymbols;
  var __getProtoOf = Object.getPrototypeOf;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __propIsEnum = Object.prototype.propertyIsEnumerable;
  var __typeError = (msg) => {
    throw String(msg);
  };
  var __defNormalProp = (obj, key, value) => key in obj ? __defProp(obj, key, { enumerable: true, configurable: true, writable: true, value }) : obj[key] = value;
  var __spreadValues = (a, b) => {
    for (var prop in b || (b = {}))
      if (__hasOwnProp.call(b, prop))
        __defNormalProp(a, prop, b[prop]);
    if (__getOwnPropSymbols)
      for (var prop of __getOwnPropSymbols(b)) {
        if (__propIsEnum.call(b, prop))
          __defNormalProp(a, prop, b[prop]);
      }
    return a;
  };
  var __spreadProps = (a, b) => __defProps(a, __getOwnPropDescs(b));
  var __objRest = (source, exclude) => {
    var target = {};
    for (var prop in source)
      if (__hasOwnProp.call(source, prop) && exclude.indexOf(prop) < 0)
        target[prop] = source[prop];
    if (source != null && __getOwnPropSymbols)
      for (var prop of __getOwnPropSymbols(source)) {
        if (exclude.indexOf(prop) < 0 && __propIsEnum.call(source, prop))
          target[prop] = source[prop];
      }
    return target;
  };
  var __commonJS = (cb, mod) => function __require() {
    return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
  };
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };
  var __copyProps = (to, from, except, desc) => {
    if (from && typeof from === "object" || typeof from === "function") {
      for (let key of __getOwnPropNames(from))
        if (!__hasOwnProp.call(to, key) && key !== except)
          __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
    }
    return to;
  };
  var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
    // If the importer is in node compatibility mode or this is not an ESM
    // file that has been converted to a CommonJS file using a Babel-
    // compatible transform (i.e. "__esModule" has not been set), then set
    // "default" to the CommonJS "module.exports" for node compatibility.
    isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
    mod
  ));
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);
  var __publicField = (obj, key, value) => __defNormalProp(obj, typeof key !== "symbol" ? key + "" : key, value);
  var __accessCheck = (obj, member, msg) => member.has(obj) || __typeError("Cannot " + msg);
  var __privateGet = (obj, member, getter) => (__accessCheck(obj, member, "read from private field"), getter ? getter.call(obj) : member.get(obj));
  var __privateAdd = (obj, member, value) => member.has(obj) ? __typeError("Cannot add the same private member more than once") : member instanceof WeakSet ? member.add(obj) : member.set(obj, value);
  var __privateSet = (obj, member, value, setter) => (__accessCheck(obj, member, "write to private field"), setter ? setter.call(obj, value) : member.set(obj, value), value);
  var __privateMethod = (obj, member, method) => (__accessCheck(obj, member, "access private method"), method);

  // stub-empty:utilities
  var require_utilities = __commonJS({
    "stub-empty:utilities"(exports, module) {
      module.exports = new Proxy({
        ifDef: function(target, key, value, defaultValue) {
          if (value !== undefined && value !== defaultValue) target[key] = value;
        },
        toFixed: function(num, digits) {
          return Number(Number(num).toFixed(digits !== undefined ? digits : 4));
        },
        sketchLogger: { info: function() {}, warn: function() {}, error: function() {}, debug: function() {} },
        SettingsManager: { editorLineLength: { "snake-layout-mode": 30 } }
      }, { get: (t, p) => (p in t ? t[p] : function() {}) });
    }
  });

  // stub-empty:application/editor/shared/coordinates
  var require_coordinates = __commonJS({
    "stub-empty:application/editor/shared/coordinates"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // node_modules/lodash/lodash.js
  var require_lodash = __commonJS({
    "node_modules/lodash/lodash.js"(exports, module) {
      (function() {
        var undefined2;
        var VERSION = "4.18.1";
        var LARGE_ARRAY_SIZE = 200;
        var CORE_ERROR_TEXT = "Unsupported core-js use. Try https://npms.io/search?q=ponyfill.", FUNC_ERROR_TEXT = "Expected a function", INVALID_TEMPL_VAR_ERROR_TEXT = "Invalid `variable` option passed into `_.template`", INVALID_TEMPL_IMPORTS_ERROR_TEXT = "Invalid `imports` option passed into `_.template`";
        var HASH_UNDEFINED = "__lodash_hash_undefined__";
        var MAX_MEMOIZE_SIZE = 500;
        var PLACEHOLDER = "__lodash_placeholder__";
        var CLONE_DEEP_FLAG = 1, CLONE_FLAT_FLAG = 2, CLONE_SYMBOLS_FLAG = 4;
        var COMPARE_PARTIAL_FLAG = 1, COMPARE_UNORDERED_FLAG = 2;
        var WRAP_BIND_FLAG = 1, WRAP_BIND_KEY_FLAG = 2, WRAP_CURRY_BOUND_FLAG = 4, WRAP_CURRY_FLAG = 8, WRAP_CURRY_RIGHT_FLAG = 16, WRAP_PARTIAL_FLAG = 32, WRAP_PARTIAL_RIGHT_FLAG = 64, WRAP_ARY_FLAG = 128, WRAP_REARG_FLAG = 256, WRAP_FLIP_FLAG = 512;
        var DEFAULT_TRUNC_LENGTH = 30, DEFAULT_TRUNC_OMISSION = "...";
        var HOT_COUNT = 800, HOT_SPAN = 16;
        var LAZY_FILTER_FLAG = 1, LAZY_MAP_FLAG = 2, LAZY_WHILE_FLAG = 3;
        var INFINITY = 1 / 0, MAX_SAFE_INTEGER = 9007199254740991, MAX_INTEGER = 17976931348623157e292, NAN = 0 / 0;
        var MAX_ARRAY_LENGTH = 4294967295, MAX_ARRAY_INDEX = MAX_ARRAY_LENGTH - 1, HALF_MAX_ARRAY_LENGTH = MAX_ARRAY_LENGTH >>> 1;
        var wrapFlags = [
          ["ary", WRAP_ARY_FLAG],
          ["bind", WRAP_BIND_FLAG],
          ["bindKey", WRAP_BIND_KEY_FLAG],
          ["curry", WRAP_CURRY_FLAG],
          ["curryRight", WRAP_CURRY_RIGHT_FLAG],
          ["flip", WRAP_FLIP_FLAG],
          ["partial", WRAP_PARTIAL_FLAG],
          ["partialRight", WRAP_PARTIAL_RIGHT_FLAG],
          ["rearg", WRAP_REARG_FLAG]
        ];
        var argsTag = "[object Arguments]", arrayTag = "[object Array]", asyncTag = "[object AsyncFunction]", boolTag = "[object Boolean]", dateTag = "[object Date]", domExcTag = "[object DOMException]", errorTag = "[object Error]", funcTag = "[object Function]", genTag = "[object GeneratorFunction]", mapTag = "[object Map]", numberTag = "[object Number]", nullTag = "[object Null]", objectTag = "[object Object]", promiseTag = "[object Promise]", proxyTag = "[object Proxy]", regexpTag = "[object RegExp]", setTag = "[object Set]", stringTag = "[object String]", symbolTag = "[object Symbol]", undefinedTag = "[object Undefined]", weakMapTag = "[object WeakMap]", weakSetTag = "[object WeakSet]";
        var arrayBufferTag = "[object ArrayBuffer]", dataViewTag = "[object DataView]", float32Tag = "[object Float32Array]", float64Tag = "[object Float64Array]", int8Tag = "[object Int8Array]", int16Tag = "[object Int16Array]", int32Tag = "[object Int32Array]", uint8Tag = "[object Uint8Array]", uint8ClampedTag = "[object Uint8ClampedArray]", uint16Tag = "[object Uint16Array]", uint32Tag = "[object Uint32Array]";
        var reEmptyStringLeading = /\b__p \+= '';/g, reEmptyStringMiddle = /\b(__p \+=) '' \+/g, reEmptyStringTrailing = /(__e\(.*?\)|\b__t\)) \+\n'';/g;
        var reEscapedHtml = /&(?:amp|lt|gt|quot|#39);/g, reUnescapedHtml = /[&<>"']/g, reHasEscapedHtml = RegExp(reEscapedHtml.source), reHasUnescapedHtml = RegExp(reUnescapedHtml.source);
        var reEscape = /<%-([\s\S]+?)%>/g, reEvaluate = /<%([\s\S]+?)%>/g, reInterpolate = /<%=([\s\S]+?)%>/g;
        var reIsDeepProp = /\.|\[(?:[^[\]]*|(["'])(?:(?!\1)[^\\]|\\.)*?\1)\]/, reIsPlainProp = /^\w*$/, rePropName = /[^.[\]]+|\[(?:(-?\d+(?:\.\d+)?)|(["'])((?:(?!\2)[^\\]|\\.)*?)\2)\]|(?=(?:\.|\[\])(?:\.|\[\]|$))/g;
        var reRegExpChar = /[\\^$.*+?()[\]{}|]/g, reHasRegExpChar = RegExp(reRegExpChar.source);
        var reTrimStart = /^\s+/;
        var reWhitespace = /\s/;
        var reWrapComment = /\{(?:\n\/\* \[wrapped with .+\] \*\/)?\n?/, reWrapDetails = /\{\n\/\* \[wrapped with (.+)\] \*/, reSplitDetails = /,? & /;
        var reAsciiWord = /[^\x00-\x2f\x3a-\x40\x5b-\x60\x7b-\x7f]+/g;
        var reForbiddenIdentifierChars = /[()=,{}\[\]\/\s]/;
        var reEscapeChar = /\\(\\)?/g;
        var reEsTemplate = /\$\{([^\\}]*(?:\\.[^\\}]*)*)\}/g;
        var reFlags = /\w*$/;
        var reIsBadHex = /^[-+]0x[0-9a-f]+$/i;
        var reIsBinary = /^0b[01]+$/i;
        var reIsHostCtor = /^\[object .+?Constructor\]$/;
        var reIsOctal = /^0o[0-7]+$/i;
        var reIsUint = /^(?:0|[1-9]\d*)$/;
        var reLatin = /[\xc0-\xd6\xd8-\xf6\xf8-\xff\u0100-\u017f]/g;
        var reNoMatch = /($^)/;
        var reUnescapedString = /['\n\r\u2028\u2029\\]/g;
        var rsAstralRange = "\\ud800-\\udfff", rsComboMarksRange = "\\u0300-\\u036f", reComboHalfMarksRange = "\\ufe20-\\ufe2f", rsComboSymbolsRange = "\\u20d0-\\u20ff", rsComboRange = rsComboMarksRange + reComboHalfMarksRange + rsComboSymbolsRange, rsDingbatRange = "\\u2700-\\u27bf", rsLowerRange = "a-z\\xdf-\\xf6\\xf8-\\xff", rsMathOpRange = "\\xac\\xb1\\xd7\\xf7", rsNonCharRange = "\\x00-\\x2f\\x3a-\\x40\\x5b-\\x60\\x7b-\\xbf", rsPunctuationRange = "\\u2000-\\u206f", rsSpaceRange = " \\t\\x0b\\f\\xa0\\ufeff\\n\\r\\u2028\\u2029\\u1680\\u180e\\u2000\\u2001\\u2002\\u2003\\u2004\\u2005\\u2006\\u2007\\u2008\\u2009\\u200a\\u202f\\u205f\\u3000", rsUpperRange = "A-Z\\xc0-\\xd6\\xd8-\\xde", rsVarRange = "\\ufe0e\\ufe0f", rsBreakRange = rsMathOpRange + rsNonCharRange + rsPunctuationRange + rsSpaceRange;
        var rsApos = "['\u2019]", rsAstral = "[" + rsAstralRange + "]", rsBreak = "[" + rsBreakRange + "]", rsCombo = "[" + rsComboRange + "]", rsDigits = "\\d+", rsDingbat = "[" + rsDingbatRange + "]", rsLower = "[" + rsLowerRange + "]", rsMisc = "[^" + rsAstralRange + rsBreakRange + rsDigits + rsDingbatRange + rsLowerRange + rsUpperRange + "]", rsFitz = "\\ud83c[\\udffb-\\udfff]", rsModifier = "(?:" + rsCombo + "|" + rsFitz + ")", rsNonAstral = "[^" + rsAstralRange + "]", rsRegional = "(?:\\ud83c[\\udde6-\\uddff]){2}", rsSurrPair = "[\\ud800-\\udbff][\\udc00-\\udfff]", rsUpper = "[" + rsUpperRange + "]", rsZWJ = "\\u200d";
        var rsMiscLower = "(?:" + rsLower + "|" + rsMisc + ")", rsMiscUpper = "(?:" + rsUpper + "|" + rsMisc + ")", rsOptContrLower = "(?:" + rsApos + "(?:d|ll|m|re|s|t|ve))?", rsOptContrUpper = "(?:" + rsApos + "(?:D|LL|M|RE|S|T|VE))?", reOptMod = rsModifier + "?", rsOptVar = "[" + rsVarRange + "]?", rsOptJoin = "(?:" + rsZWJ + "(?:" + [rsNonAstral, rsRegional, rsSurrPair].join("|") + ")" + rsOptVar + reOptMod + ")*", rsOrdLower = "\\d*(?:1st|2nd|3rd|(?![123])\\dth)(?=\\b|[A-Z_])", rsOrdUpper = "\\d*(?:1ST|2ND|3RD|(?![123])\\dTH)(?=\\b|[a-z_])", rsSeq = rsOptVar + reOptMod + rsOptJoin, rsEmoji = "(?:" + [rsDingbat, rsRegional, rsSurrPair].join("|") + ")" + rsSeq, rsSymbol = "(?:" + [rsNonAstral + rsCombo + "?", rsCombo, rsRegional, rsSurrPair, rsAstral].join("|") + ")";
        var reApos = RegExp(rsApos, "g");
        var reComboMark = RegExp(rsCombo, "g");
        var reUnicode = RegExp(rsFitz + "(?=" + rsFitz + ")|" + rsSymbol + rsSeq, "g");
        var reUnicodeWord = RegExp([
          rsUpper + "?" + rsLower + "+" + rsOptContrLower + "(?=" + [rsBreak, rsUpper, "$"].join("|") + ")",
          rsMiscUpper + "+" + rsOptContrUpper + "(?=" + [rsBreak, rsUpper + rsMiscLower, "$"].join("|") + ")",
          rsUpper + "?" + rsMiscLower + "+" + rsOptContrLower,
          rsUpper + "+" + rsOptContrUpper,
          rsOrdUpper,
          rsOrdLower,
          rsDigits,
          rsEmoji
        ].join("|"), "g");
        var reHasUnicode = RegExp("[" + rsZWJ + rsAstralRange + rsComboRange + rsVarRange + "]");
        var reHasUnicodeWord = /[a-z][A-Z]|[A-Z]{2}[a-z]|[0-9][a-zA-Z]|[a-zA-Z][0-9]|[^a-zA-Z0-9 ]/;
        var contextProps = [
          "Array",
          "Buffer",
          "DataView",
          "Date",
          "Error",
          "Float32Array",
          "Float64Array",
          "Function",
          "Int8Array",
          "Int16Array",
          "Int32Array",
          "Map",
          "Math",
          "Object",
          "Promise",
          "RegExp",
          "Set",
          "String",
          "Symbol",
          "TypeError",
          "Uint8Array",
          "Uint8ClampedArray",
          "Uint16Array",
          "Uint32Array",
          "WeakMap",
          "_",
          "clearTimeout",
          "isFinite",
          "parseInt",
          "setTimeout"
        ];
        var templateCounter = -1;
        var typedArrayTags = {};
        typedArrayTags[float32Tag] = typedArrayTags[float64Tag] = typedArrayTags[int8Tag] = typedArrayTags[int16Tag] = typedArrayTags[int32Tag] = typedArrayTags[uint8Tag] = typedArrayTags[uint8ClampedTag] = typedArrayTags[uint16Tag] = typedArrayTags[uint32Tag] = true;
        typedArrayTags[argsTag] = typedArrayTags[arrayTag] = typedArrayTags[arrayBufferTag] = typedArrayTags[boolTag] = typedArrayTags[dataViewTag] = typedArrayTags[dateTag] = typedArrayTags[errorTag] = typedArrayTags[funcTag] = typedArrayTags[mapTag] = typedArrayTags[numberTag] = typedArrayTags[objectTag] = typedArrayTags[regexpTag] = typedArrayTags[setTag] = typedArrayTags[stringTag] = typedArrayTags[weakMapTag] = false;
        var cloneableTags = {};
        cloneableTags[argsTag] = cloneableTags[arrayTag] = cloneableTags[arrayBufferTag] = cloneableTags[dataViewTag] = cloneableTags[boolTag] = cloneableTags[dateTag] = cloneableTags[float32Tag] = cloneableTags[float64Tag] = cloneableTags[int8Tag] = cloneableTags[int16Tag] = cloneableTags[int32Tag] = cloneableTags[mapTag] = cloneableTags[numberTag] = cloneableTags[objectTag] = cloneableTags[regexpTag] = cloneableTags[setTag] = cloneableTags[stringTag] = cloneableTags[symbolTag] = cloneableTags[uint8Tag] = cloneableTags[uint8ClampedTag] = cloneableTags[uint16Tag] = cloneableTags[uint32Tag] = true;
        cloneableTags[errorTag] = cloneableTags[funcTag] = cloneableTags[weakMapTag] = false;
        var deburredLetters = {
          // Latin-1 Supplement block.
          "\xC0": "A",
          "\xC1": "A",
          "\xC2": "A",
          "\xC3": "A",
          "\xC4": "A",
          "\xC5": "A",
          "\xE0": "a",
          "\xE1": "a",
          "\xE2": "a",
          "\xE3": "a",
          "\xE4": "a",
          "\xE5": "a",
          "\xC7": "C",
          "\xE7": "c",
          "\xD0": "D",
          "\xF0": "d",
          "\xC8": "E",
          "\xC9": "E",
          "\xCA": "E",
          "\xCB": "E",
          "\xE8": "e",
          "\xE9": "e",
          "\xEA": "e",
          "\xEB": "e",
          "\xCC": "I",
          "\xCD": "I",
          "\xCE": "I",
          "\xCF": "I",
          "\xEC": "i",
          "\xED": "i",
          "\xEE": "i",
          "\xEF": "i",
          "\xD1": "N",
          "\xF1": "n",
          "\xD2": "O",
          "\xD3": "O",
          "\xD4": "O",
          "\xD5": "O",
          "\xD6": "O",
          "\xD8": "O",
          "\xF2": "o",
          "\xF3": "o",
          "\xF4": "o",
          "\xF5": "o",
          "\xF6": "o",
          "\xF8": "o",
          "\xD9": "U",
          "\xDA": "U",
          "\xDB": "U",
          "\xDC": "U",
          "\xF9": "u",
          "\xFA": "u",
          "\xFB": "u",
          "\xFC": "u",
          "\xDD": "Y",
          "\xFD": "y",
          "\xFF": "y",
          "\xC6": "Ae",
          "\xE6": "ae",
          "\xDE": "Th",
          "\xFE": "th",
          "\xDF": "ss",
          // Latin Extended-A block.
          "\u0100": "A",
          "\u0102": "A",
          "\u0104": "A",
          "\u0101": "a",
          "\u0103": "a",
          "\u0105": "a",
          "\u0106": "C",
          "\u0108": "C",
          "\u010A": "C",
          "\u010C": "C",
          "\u0107": "c",
          "\u0109": "c",
          "\u010B": "c",
          "\u010D": "c",
          "\u010E": "D",
          "\u0110": "D",
          "\u010F": "d",
          "\u0111": "d",
          "\u0112": "E",
          "\u0114": "E",
          "\u0116": "E",
          "\u0118": "E",
          "\u011A": "E",
          "\u0113": "e",
          "\u0115": "e",
          "\u0117": "e",
          "\u0119": "e",
          "\u011B": "e",
          "\u011C": "G",
          "\u011E": "G",
          "\u0120": "G",
          "\u0122": "G",
          "\u011D": "g",
          "\u011F": "g",
          "\u0121": "g",
          "\u0123": "g",
          "\u0124": "H",
          "\u0126": "H",
          "\u0125": "h",
          "\u0127": "h",
          "\u0128": "I",
          "\u012A": "I",
          "\u012C": "I",
          "\u012E": "I",
          "\u0130": "I",
          "\u0129": "i",
          "\u012B": "i",
          "\u012D": "i",
          "\u012F": "i",
          "\u0131": "i",
          "\u0134": "J",
          "\u0135": "j",
          "\u0136": "K",
          "\u0137": "k",
          "\u0138": "k",
          "\u0139": "L",
          "\u013B": "L",
          "\u013D": "L",
          "\u013F": "L",
          "\u0141": "L",
          "\u013A": "l",
          "\u013C": "l",
          "\u013E": "l",
          "\u0140": "l",
          "\u0142": "l",
          "\u0143": "N",
          "\u0145": "N",
          "\u0147": "N",
          "\u014A": "N",
          "\u0144": "n",
          "\u0146": "n",
          "\u0148": "n",
          "\u014B": "n",
          "\u014C": "O",
          "\u014E": "O",
          "\u0150": "O",
          "\u014D": "o",
          "\u014F": "o",
          "\u0151": "o",
          "\u0154": "R",
          "\u0156": "R",
          "\u0158": "R",
          "\u0155": "r",
          "\u0157": "r",
          "\u0159": "r",
          "\u015A": "S",
          "\u015C": "S",
          "\u015E": "S",
          "\u0160": "S",
          "\u015B": "s",
          "\u015D": "s",
          "\u015F": "s",
          "\u0161": "s",
          "\u0162": "T",
          "\u0164": "T",
          "\u0166": "T",
          "\u0163": "t",
          "\u0165": "t",
          "\u0167": "t",
          "\u0168": "U",
          "\u016A": "U",
          "\u016C": "U",
          "\u016E": "U",
          "\u0170": "U",
          "\u0172": "U",
          "\u0169": "u",
          "\u016B": "u",
          "\u016D": "u",
          "\u016F": "u",
          "\u0171": "u",
          "\u0173": "u",
          "\u0174": "W",
          "\u0175": "w",
          "\u0176": "Y",
          "\u0177": "y",
          "\u0178": "Y",
          "\u0179": "Z",
          "\u017B": "Z",
          "\u017D": "Z",
          "\u017A": "z",
          "\u017C": "z",
          "\u017E": "z",
          "\u0132": "IJ",
          "\u0133": "ij",
          "\u0152": "Oe",
          "\u0153": "oe",
          "\u0149": "'n",
          "\u017F": "s"
        };
        var htmlEscapes = {
          "&": "&amp;",
          "<": "&lt;",
          ">": "&gt;",
          '"': "&quot;",
          "'": "&#39;"
        };
        var htmlUnescapes = {
          "&amp;": "&",
          "&lt;": "<",
          "&gt;": ">",
          "&quot;": '"',
          "&#39;": "'"
        };
        var stringEscapes = {
          "\\": "\\",
          "'": "'",
          "\n": "n",
          "\r": "r",
          "\u2028": "u2028",
          "\u2029": "u2029"
        };
        var freeParseFloat = parseFloat, freeParseInt = parseInt;
        var freeGlobal = typeof global == "object" && global && global.Object === Object && global;
        var freeSelf = typeof self == "object" && self && self.Object === Object && self;
        var root = freeGlobal || freeSelf || Function("return this")();
        var freeExports = typeof exports == "object" && exports && !exports.nodeType && exports;
        var freeModule = freeExports && typeof module == "object" && module && !module.nodeType && module;
        var moduleExports = freeModule && freeModule.exports === freeExports;
        var freeProcess = moduleExports && freeGlobal.process;
        var nodeUtil = (function() {
          try {
            var types = freeModule && freeModule.require && freeModule.require("util").types;
            if (types) {
              return types;
            }
            return freeProcess && freeProcess.binding && freeProcess.binding("util");
          } catch (e) {
          }
        })();
        var nodeIsArrayBuffer = nodeUtil && nodeUtil.isArrayBuffer, nodeIsDate = nodeUtil && nodeUtil.isDate, nodeIsMap = nodeUtil && nodeUtil.isMap, nodeIsRegExp = nodeUtil && nodeUtil.isRegExp, nodeIsSet = nodeUtil && nodeUtil.isSet, nodeIsTypedArray = nodeUtil && nodeUtil.isTypedArray;
        function apply(func, thisArg, args) {
          switch (args.length) {
            case 0:
              return func.call(thisArg);
            case 1:
              return func.call(thisArg, args[0]);
            case 2:
              return func.call(thisArg, args[0], args[1]);
            case 3:
              return func.call(thisArg, args[0], args[1], args[2]);
          }
          return func.apply(thisArg, args);
        }
        function arrayAggregator(array, setter, iteratee, accumulator) {
          var index = -1, length = array == null ? 0 : array.length;
          while (++index < length) {
            var value = array[index];
            setter(accumulator, value, iteratee(value), array);
          }
          return accumulator;
        }
        function arrayEach(array, iteratee) {
          var index = -1, length = array == null ? 0 : array.length;
          while (++index < length) {
            if (iteratee(array[index], index, array) === false) {
              break;
            }
          }
          return array;
        }
        function arrayEachRight(array, iteratee) {
          var length = array == null ? 0 : array.length;
          while (length--) {
            if (iteratee(array[length], length, array) === false) {
              break;
            }
          }
          return array;
        }
        function arrayEvery(array, predicate) {
          var index = -1, length = array == null ? 0 : array.length;
          while (++index < length) {
            if (!predicate(array[index], index, array)) {
              return false;
            }
          }
          return true;
        }
        function arrayFilter(array, predicate) {
          var index = -1, length = array == null ? 0 : array.length, resIndex = 0, result = [];
          while (++index < length) {
            var value = array[index];
            if (predicate(value, index, array)) {
              result[resIndex++] = value;
            }
          }
          return result;
        }
        function arrayIncludes(array, value) {
          var length = array == null ? 0 : array.length;
          return !!length && baseIndexOf(array, value, 0) > -1;
        }
        function arrayIncludesWith(array, value, comparator) {
          var index = -1, length = array == null ? 0 : array.length;
          while (++index < length) {
            if (comparator(value, array[index])) {
              return true;
            }
          }
          return false;
        }
        function arrayMap(array, iteratee) {
          var index = -1, length = array == null ? 0 : array.length, result = Array(length);
          while (++index < length) {
            result[index] = iteratee(array[index], index, array);
          }
          return result;
        }
        function arrayPush(array, values2) {
          var index = -1, length = values2.length, offset = array.length;
          while (++index < length) {
            array[offset + index] = values2[index];
          }
          return array;
        }
        function arrayReduce(array, iteratee, accumulator, initAccum) {
          var index = -1, length = array == null ? 0 : array.length;
          if (initAccum && length) {
            accumulator = array[++index];
          }
          while (++index < length) {
            accumulator = iteratee(accumulator, array[index], index, array);
          }
          return accumulator;
        }
        function arrayReduceRight(array, iteratee, accumulator, initAccum) {
          var length = array == null ? 0 : array.length;
          if (initAccum && length) {
            accumulator = array[--length];
          }
          while (length--) {
            accumulator = iteratee(accumulator, array[length], length, array);
          }
          return accumulator;
        }
        function arraySome(array, predicate) {
          var index = -1, length = array == null ? 0 : array.length;
          while (++index < length) {
            if (predicate(array[index], index, array)) {
              return true;
            }
          }
          return false;
        }
        var asciiSize = baseProperty("length");
        function asciiToArray(string) {
          return string.split("");
        }
        function asciiWords(string) {
          return string.match(reAsciiWord) || [];
        }
        function baseFindKey(collection, predicate, eachFunc) {
          var result;
          eachFunc(collection, function(value, key, collection2) {
            if (predicate(value, key, collection2)) {
              result = key;
              return false;
            }
          });
          return result;
        }
        function baseFindIndex(array, predicate, fromIndex, fromRight) {
          var length = array.length, index = fromIndex + (fromRight ? 1 : -1);
          while (fromRight ? index-- : ++index < length) {
            if (predicate(array[index], index, array)) {
              return index;
            }
          }
          return -1;
        }
        function baseIndexOf(array, value, fromIndex) {
          return value === value ? strictIndexOf(array, value, fromIndex) : baseFindIndex(array, baseIsNaN, fromIndex);
        }
        function baseIndexOfWith(array, value, fromIndex, comparator) {
          var index = fromIndex - 1, length = array.length;
          while (++index < length) {
            if (comparator(array[index], value)) {
              return index;
            }
          }
          return -1;
        }
        function baseIsNaN(value) {
          return value !== value;
        }
        function baseMean(array, iteratee) {
          var length = array == null ? 0 : array.length;
          return length ? baseSum(array, iteratee) / length : NAN;
        }
        function baseProperty(key) {
          return function(object) {
            return object == null ? undefined2 : object[key];
          };
        }
        function basePropertyOf(object) {
          return function(key) {
            return object == null ? undefined2 : object[key];
          };
        }
        function baseReduce(collection, iteratee, accumulator, initAccum, eachFunc) {
          eachFunc(collection, function(value, index, collection2) {
            accumulator = initAccum ? (initAccum = false, value) : iteratee(accumulator, value, index, collection2);
          });
          return accumulator;
        }
        function baseSortBy(array, comparer) {
          var length = array.length;
          array.sort(comparer);
          while (length--) {
            array[length] = array[length].value;
          }
          return array;
        }
        function baseSum(array, iteratee) {
          var result, index = -1, length = array.length;
          while (++index < length) {
            var current = iteratee(array[index]);
            if (current !== undefined2) {
              result = result === undefined2 ? current : result + current;
            }
          }
          return result;
        }
        function baseTimes(n, iteratee) {
          var index = -1, result = Array(n);
          while (++index < n) {
            result[index] = iteratee(index);
          }
          return result;
        }
        function baseToPairs(object, props) {
          return arrayMap(props, function(key) {
            return [key, object[key]];
          });
        }
        function baseTrim(string) {
          return string ? string.slice(0, trimmedEndIndex(string) + 1).replace(reTrimStart, "") : string;
        }
        function baseUnary(func) {
          return function(value) {
            return func(value);
          };
        }
        function baseValues(object, props) {
          return arrayMap(props, function(key) {
            return object[key];
          });
        }
        function cacheHas(cache, key) {
          return cache.has(key);
        }
        function charsStartIndex(strSymbols, chrSymbols) {
          var index = -1, length = strSymbols.length;
          while (++index < length && baseIndexOf(chrSymbols, strSymbols[index], 0) > -1) {
          }
          return index;
        }
        function charsEndIndex(strSymbols, chrSymbols) {
          var index = strSymbols.length;
          while (index-- && baseIndexOf(chrSymbols, strSymbols[index], 0) > -1) {
          }
          return index;
        }
        function countHolders(array, placeholder) {
          var length = array.length, result = 0;
          while (length--) {
            if (array[length] === placeholder) {
              ++result;
            }
          }
          return result;
        }
        var deburrLetter = basePropertyOf(deburredLetters);
        var escapeHtmlChar = basePropertyOf(htmlEscapes);
        function escapeStringChar(chr) {
          return "\\" + stringEscapes[chr];
        }
        function getValue(object, key) {
          return object == null ? undefined2 : object[key];
        }
        function hasUnicode(string) {
          return reHasUnicode.test(string);
        }
        function hasUnicodeWord(string) {
          return reHasUnicodeWord.test(string);
        }
        function iteratorToArray(iterator) {
          var data, result = [];
          while (!(data = iterator.next()).done) {
            result.push(data.value);
          }
          return result;
        }
        function mapToArray(map) {
          var index = -1, result = Array(map.size);
          map.forEach(function(value, key) {
            result[++index] = [key, value];
          });
          return result;
        }
        function overArg(func, transform) {
          return function(arg) {
            return func(transform(arg));
          };
        }
        function replaceHolders(array, placeholder) {
          var index = -1, length = array.length, resIndex = 0, result = [];
          while (++index < length) {
            var value = array[index];
            if (value === placeholder || value === PLACEHOLDER) {
              array[index] = PLACEHOLDER;
              result[resIndex++] = index;
            }
          }
          return result;
        }
        function setToArray(set) {
          var index = -1, result = Array(set.size);
          set.forEach(function(value) {
            result[++index] = value;
          });
          return result;
        }
        function setToPairs(set) {
          var index = -1, result = Array(set.size);
          set.forEach(function(value) {
            result[++index] = [value, value];
          });
          return result;
        }
        function strictIndexOf(array, value, fromIndex) {
          var index = fromIndex - 1, length = array.length;
          while (++index < length) {
            if (array[index] === value) {
              return index;
            }
          }
          return -1;
        }
        function strictLastIndexOf(array, value, fromIndex) {
          var index = fromIndex + 1;
          while (index--) {
            if (array[index] === value) {
              return index;
            }
          }
          return index;
        }
        function stringSize(string) {
          return hasUnicode(string) ? unicodeSize(string) : asciiSize(string);
        }
        function stringToArray(string) {
          return hasUnicode(string) ? unicodeToArray(string) : asciiToArray(string);
        }
        function trimmedEndIndex(string) {
          var index = string.length;
          while (index-- && reWhitespace.test(string.charAt(index))) {
          }
          return index;
        }
        var unescapeHtmlChar = basePropertyOf(htmlUnescapes);
        function unicodeSize(string) {
          var result = reUnicode.lastIndex = 0;
          while (reUnicode.test(string)) {
            ++result;
          }
          return result;
        }
        function unicodeToArray(string) {
          return string.match(reUnicode) || [];
        }
        function unicodeWords(string) {
          return string.match(reUnicodeWord) || [];
        }
        var runInContext = (function runInContext2(context) {
          context = context == null ? root : _.defaults(root.Object(), context, _.pick(root, contextProps));
          var Array2 = context.Array, Date2 = context.Date, Error2 = context.Error, Function2 = context.Function, Math2 = context.Math, Object2 = context.Object, RegExp2 = context.RegExp, String2 = context.String, TypeError2 = context.TypeError;
          var arrayProto = Array2.prototype, funcProto = Function2.prototype, objectProto = Object2.prototype;
          var coreJsData = context["__core-js_shared__"];
          var funcToString = funcProto.toString;
          var hasOwnProperty = objectProto.hasOwnProperty;
          var idCounter = 0;
          var maskSrcKey = (function() {
            var uid = /[^.]+$/.exec(coreJsData && coreJsData.keys && coreJsData.keys.IE_PROTO || "");
            return uid ? "Symbol(src)_1." + uid : "";
          })();
          var nativeObjectToString = objectProto.toString;
          var objectCtorString = funcToString.call(Object2);
          var oldDash = root._;
          var reIsNative = RegExp2(
            "^" + funcToString.call(hasOwnProperty).replace(reRegExpChar, "\\$&").replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g, "$1.*?") + "$"
          );
          var Buffer2 = moduleExports ? context.Buffer : undefined2, Symbol2 = context.Symbol, Uint8Array2 = context.Uint8Array, allocUnsafe = Buffer2 ? Buffer2.allocUnsafe : undefined2, getPrototype = overArg(Object2.getPrototypeOf, Object2), objectCreate = Object2.create, propertyIsEnumerable = objectProto.propertyIsEnumerable, splice = arrayProto.splice, spreadableSymbol = Symbol2 ? Symbol2.isConcatSpreadable : undefined2, symIterator = Symbol2 ? Symbol2.iterator : undefined2, symToStringTag = Symbol2 ? Symbol2.toStringTag : undefined2;
          var defineProperty = (function() {
            try {
              var func = getNative(Object2, "defineProperty");
              func({}, "", {});
              return func;
            } catch (e) {
            }
          })();
          var ctxClearTimeout = context.clearTimeout !== root.clearTimeout && context.clearTimeout, ctxNow = Date2 && Date2.now !== root.Date.now && Date2.now, ctxSetTimeout = context.setTimeout !== root.setTimeout && context.setTimeout;
          var nativeCeil = Math2.ceil, nativeFloor = Math2.floor, nativeGetSymbols = Object2.getOwnPropertySymbols, nativeIsBuffer = Buffer2 ? Buffer2.isBuffer : undefined2, nativeIsFinite = context.isFinite, nativeJoin = arrayProto.join, nativeKeys = overArg(Object2.keys, Object2), nativeMax = Math2.max, nativeMin = Math2.min, nativeNow = Date2.now, nativeParseInt = context.parseInt, nativeRandom = Math2.random, nativeReverse = arrayProto.reverse;
          var DataView = getNative(context, "DataView"), Map2 = getNative(context, "Map"), Promise2 = getNative(context, "Promise"), Set2 = getNative(context, "Set"), WeakMap2 = getNative(context, "WeakMap"), nativeCreate = getNative(Object2, "create");
          var metaMap = WeakMap2 && new WeakMap2();
          var realNames = {};
          var dataViewCtorString = toSource(DataView), mapCtorString = toSource(Map2), promiseCtorString = toSource(Promise2), setCtorString = toSource(Set2), weakMapCtorString = toSource(WeakMap2);
          var symbolProto = Symbol2 ? Symbol2.prototype : undefined2, symbolValueOf = symbolProto ? symbolProto.valueOf : undefined2, symbolToString = symbolProto ? symbolProto.toString : undefined2;
          function lodash(value) {
            if (isObjectLike(value) && !isArray(value) && !(value instanceof LazyWrapper)) {
              if (value instanceof LodashWrapper) {
                return value;
              }
              if (hasOwnProperty.call(value, "__wrapped__")) {
                return wrapperClone(value);
              }
            }
            return new LodashWrapper(value);
          }
          var baseCreate = /* @__PURE__ */ (function() {
            function object() {
            }
            return function(proto) {
              if (!isObject(proto)) {
                return {};
              }
              if (objectCreate) {
                return objectCreate(proto);
              }
              object.prototype = proto;
              var result2 = new object();
              object.prototype = undefined2;
              return result2;
            };
          })();
          function baseLodash() {
          }
          function LodashWrapper(value, chainAll) {
            this.__wrapped__ = value;
            this.__actions__ = [];
            this.__chain__ = !!chainAll;
            this.__index__ = 0;
            this.__values__ = undefined2;
          }
          lodash.templateSettings = {
            /**
             * Used to detect `data` property values to be HTML-escaped.
             *
             * @memberOf _.templateSettings
             * @type {RegExp}
             */
            "escape": reEscape,
            /**
             * Used to detect code to be evaluated.
             *
             * @memberOf _.templateSettings
             * @type {RegExp}
             */
            "evaluate": reEvaluate,
            /**
             * Used to detect `data` property values to inject.
             *
             * @memberOf _.templateSettings
             * @type {RegExp}
             */
            "interpolate": reInterpolate,
            /**
             * Used to reference the data object in the template text.
             *
             * @memberOf _.templateSettings
             * @type {string}
             */
            "variable": "",
            /**
             * Used to import variables into the compiled template.
             *
             * @memberOf _.templateSettings
             * @type {Object}
             */
            "imports": {
              /**
               * A reference to the `lodash` function.
               *
               * @memberOf _.templateSettings.imports
               * @type {Function}
               */
              "_": lodash
            }
          };
          lodash.prototype = baseLodash.prototype;
          lodash.prototype.constructor = lodash;
          LodashWrapper.prototype = baseCreate(baseLodash.prototype);
          LodashWrapper.prototype.constructor = LodashWrapper;
          function LazyWrapper(value) {
            this.__wrapped__ = value;
            this.__actions__ = [];
            this.__dir__ = 1;
            this.__filtered__ = false;
            this.__iteratees__ = [];
            this.__takeCount__ = MAX_ARRAY_LENGTH;
            this.__views__ = [];
          }
          function lazyClone() {
            var result2 = new LazyWrapper(this.__wrapped__);
            result2.__actions__ = copyArray(this.__actions__);
            result2.__dir__ = this.__dir__;
            result2.__filtered__ = this.__filtered__;
            result2.__iteratees__ = copyArray(this.__iteratees__);
            result2.__takeCount__ = this.__takeCount__;
            result2.__views__ = copyArray(this.__views__);
            return result2;
          }
          function lazyReverse() {
            if (this.__filtered__) {
              var result2 = new LazyWrapper(this);
              result2.__dir__ = -1;
              result2.__filtered__ = true;
            } else {
              result2 = this.clone();
              result2.__dir__ *= -1;
            }
            return result2;
          }
          function lazyValue() {
            var array = this.__wrapped__.value(), dir = this.__dir__, isArr = isArray(array), isRight = dir < 0, arrLength = isArr ? array.length : 0, view = getView(0, arrLength, this.__views__), start = view.start, end = view.end, length = end - start, index = isRight ? end : start - 1, iteratees = this.__iteratees__, iterLength = iteratees.length, resIndex = 0, takeCount = nativeMin(length, this.__takeCount__);
            if (!isArr || !isRight && arrLength == length && takeCount == length) {
              return baseWrapperValue(array, this.__actions__);
            }
            var result2 = [];
            outer:
              while (length-- && resIndex < takeCount) {
                index += dir;
                var iterIndex = -1, value = array[index];
                while (++iterIndex < iterLength) {
                  var data = iteratees[iterIndex], iteratee2 = data.iteratee, type = data.type, computed = iteratee2(value);
                  if (type == LAZY_MAP_FLAG) {
                    value = computed;
                  } else if (!computed) {
                    if (type == LAZY_FILTER_FLAG) {
                      continue outer;
                    } else {
                      break outer;
                    }
                  }
                }
                result2[resIndex++] = value;
              }
            return result2;
          }
          LazyWrapper.prototype = baseCreate(baseLodash.prototype);
          LazyWrapper.prototype.constructor = LazyWrapper;
          function Hash(entries) {
            var index = -1, length = entries == null ? 0 : entries.length;
            this.clear();
            while (++index < length) {
              var entry = entries[index];
              this.set(entry[0], entry[1]);
            }
          }
          function hashClear() {
            this.__data__ = nativeCreate ? nativeCreate(null) : {};
            this.size = 0;
          }
          function hashDelete(key) {
            var result2 = this.has(key) && delete this.__data__[key];
            this.size -= result2 ? 1 : 0;
            return result2;
          }
          function hashGet(key) {
            var data = this.__data__;
            if (nativeCreate) {
              var result2 = data[key];
              return result2 === HASH_UNDEFINED ? undefined2 : result2;
            }
            return hasOwnProperty.call(data, key) ? data[key] : undefined2;
          }
          function hashHas(key) {
            var data = this.__data__;
            return nativeCreate ? data[key] !== undefined2 : hasOwnProperty.call(data, key);
          }
          function hashSet(key, value) {
            var data = this.__data__;
            this.size += this.has(key) ? 0 : 1;
            data[key] = nativeCreate && value === undefined2 ? HASH_UNDEFINED : value;
            return this;
          }
          Hash.prototype.clear = hashClear;
          Hash.prototype["delete"] = hashDelete;
          Hash.prototype.get = hashGet;
          Hash.prototype.has = hashHas;
          Hash.prototype.set = hashSet;
          function ListCache(entries) {
            var index = -1, length = entries == null ? 0 : entries.length;
            this.clear();
            while (++index < length) {
              var entry = entries[index];
              this.set(entry[0], entry[1]);
            }
          }
          function listCacheClear() {
            this.__data__ = [];
            this.size = 0;
          }
          function listCacheDelete(key) {
            var data = this.__data__, index = assocIndexOf(data, key);
            if (index < 0) {
              return false;
            }
            var lastIndex = data.length - 1;
            if (index == lastIndex) {
              data.pop();
            } else {
              splice.call(data, index, 1);
            }
            --this.size;
            return true;
          }
          function listCacheGet(key) {
            var data = this.__data__, index = assocIndexOf(data, key);
            return index < 0 ? undefined2 : data[index][1];
          }
          function listCacheHas(key) {
            return assocIndexOf(this.__data__, key) > -1;
          }
          function listCacheSet(key, value) {
            var data = this.__data__, index = assocIndexOf(data, key);
            if (index < 0) {
              ++this.size;
              data.push([key, value]);
            } else {
              data[index][1] = value;
            }
            return this;
          }
          ListCache.prototype.clear = listCacheClear;
          ListCache.prototype["delete"] = listCacheDelete;
          ListCache.prototype.get = listCacheGet;
          ListCache.prototype.has = listCacheHas;
          ListCache.prototype.set = listCacheSet;
          function MapCache(entries) {
            var index = -1, length = entries == null ? 0 : entries.length;
            this.clear();
            while (++index < length) {
              var entry = entries[index];
              this.set(entry[0], entry[1]);
            }
          }
          function mapCacheClear() {
            this.size = 0;
            this.__data__ = {
              "hash": new Hash(),
              "map": new (Map2 || ListCache)(),
              "string": new Hash()
            };
          }
          function mapCacheDelete(key) {
            var result2 = getMapData(this, key)["delete"](key);
            this.size -= result2 ? 1 : 0;
            return result2;
          }
          function mapCacheGet(key) {
            return getMapData(this, key).get(key);
          }
          function mapCacheHas(key) {
            return getMapData(this, key).has(key);
          }
          function mapCacheSet(key, value) {
            var data = getMapData(this, key), size2 = data.size;
            data.set(key, value);
            this.size += data.size == size2 ? 0 : 1;
            return this;
          }
          MapCache.prototype.clear = mapCacheClear;
          MapCache.prototype["delete"] = mapCacheDelete;
          MapCache.prototype.get = mapCacheGet;
          MapCache.prototype.has = mapCacheHas;
          MapCache.prototype.set = mapCacheSet;
          function SetCache(values3) {
            var index = -1, length = values3 == null ? 0 : values3.length;
            this.__data__ = new MapCache();
            while (++index < length) {
              this.add(values3[index]);
            }
          }
          function setCacheAdd(value) {
            this.__data__.set(value, HASH_UNDEFINED);
            return this;
          }
          function setCacheHas(value) {
            return this.__data__.has(value);
          }
          SetCache.prototype.add = SetCache.prototype.push = setCacheAdd;
          SetCache.prototype.has = setCacheHas;
          function Stack(entries) {
            var data = this.__data__ = new ListCache(entries);
            this.size = data.size;
          }
          function stackClear() {
            this.__data__ = new ListCache();
            this.size = 0;
          }
          function stackDelete(key) {
            var data = this.__data__, result2 = data["delete"](key);
            this.size = data.size;
            return result2;
          }
          function stackGet(key) {
            return this.__data__.get(key);
          }
          function stackHas(key) {
            return this.__data__.has(key);
          }
          function stackSet(key, value) {
            var data = this.__data__;
            if (data instanceof ListCache) {
              var pairs = data.__data__;
              if (!Map2 || pairs.length < LARGE_ARRAY_SIZE - 1) {
                pairs.push([key, value]);
                this.size = ++data.size;
                return this;
              }
              data = this.__data__ = new MapCache(pairs);
            }
            data.set(key, value);
            this.size = data.size;
            return this;
          }
          Stack.prototype.clear = stackClear;
          Stack.prototype["delete"] = stackDelete;
          Stack.prototype.get = stackGet;
          Stack.prototype.has = stackHas;
          Stack.prototype.set = stackSet;
          function arrayLikeKeys(value, inherited) {
            var isArr = isArray(value), isArg = !isArr && isArguments(value), isBuff = !isArr && !isArg && isBuffer(value), isType = !isArr && !isArg && !isBuff && isTypedArray(value), skipIndexes = isArr || isArg || isBuff || isType, result2 = skipIndexes ? baseTimes(value.length, String2) : [], length = result2.length;
            for (var key in value) {
              if ((inherited || hasOwnProperty.call(value, key)) && !(skipIndexes && // Safari 9 has enumerable `arguments.length` in strict mode.
              (key == "length" || // Node.js 0.10 has enumerable non-index properties on buffers.
              isBuff && (key == "offset" || key == "parent") || // PhantomJS 2 has enumerable non-index properties on typed arrays.
              isType && (key == "buffer" || key == "byteLength" || key == "byteOffset") || // Skip index properties.
              isIndex(key, length)))) {
                result2.push(key);
              }
            }
            return result2;
          }
          function arraySample(array) {
            var length = array.length;
            return length ? array[baseRandom(0, length - 1)] : undefined2;
          }
          function arraySampleSize(array, n) {
            return shuffleSelf(copyArray(array), baseClamp(n, 0, array.length));
          }
          function arrayShuffle(array) {
            return shuffleSelf(copyArray(array));
          }
          function assignMergeValue(object, key, value) {
            if (value !== undefined2 && !eq(object[key], value) || value === undefined2 && !(key in object)) {
              baseAssignValue(object, key, value);
            }
          }
          function assignValue(object, key, value) {
            var objValue = object[key];
            if (!(hasOwnProperty.call(object, key) && eq(objValue, value)) || value === undefined2 && !(key in object)) {
              baseAssignValue(object, key, value);
            }
          }
          function assocIndexOf(array, key) {
            var length = array.length;
            while (length--) {
              if (eq(array[length][0], key)) {
                return length;
              }
            }
            return -1;
          }
          function baseAggregator(collection, setter, iteratee2, accumulator) {
            baseEach(collection, function(value, key, collection2) {
              setter(accumulator, value, iteratee2(value), collection2);
            });
            return accumulator;
          }
          function baseAssign(object, source) {
            return object && copyObject(source, keys(source), object);
          }
          function baseAssignIn(object, source) {
            return object && copyObject(source, keysIn(source), object);
          }
          function baseAssignValue(object, key, value) {
            if (key == "__proto__" && defineProperty) {
              defineProperty(object, key, {
                "configurable": true,
                "enumerable": true,
                "value": value,
                "writable": true
              });
            } else {
              object[key] = value;
            }
          }
          function baseAt(object, paths) {
            var index = -1, length = paths.length, result2 = Array2(length), skip = object == null;
            while (++index < length) {
              result2[index] = skip ? undefined2 : get(object, paths[index]);
            }
            return result2;
          }
          function baseClamp(number, lower, upper) {
            if (number === number) {
              if (upper !== undefined2) {
                number = number <= upper ? number : upper;
              }
              if (lower !== undefined2) {
                number = number >= lower ? number : lower;
              }
            }
            return number;
          }
          function baseClone(value, bitmask, customizer2, key, object, stack) {
            var result2, isDeep = bitmask & CLONE_DEEP_FLAG, isFlat = bitmask & CLONE_FLAT_FLAG, isFull = bitmask & CLONE_SYMBOLS_FLAG;
            if (customizer2) {
              result2 = object ? customizer2(value, key, object, stack) : customizer2(value);
            }
            if (result2 !== undefined2) {
              return result2;
            }
            if (!isObject(value)) {
              return value;
            }
            var isArr = isArray(value);
            if (isArr) {
              result2 = initCloneArray(value);
              if (!isDeep) {
                return copyArray(value, result2);
              }
            } else {
              var tag = getTag(value), isFunc = tag == funcTag || tag == genTag;
              if (isBuffer(value)) {
                return cloneBuffer(value, isDeep);
              }
              if (tag == objectTag || tag == argsTag || isFunc && !object) {
                result2 = isFlat || isFunc ? {} : initCloneObject(value);
                if (!isDeep) {
                  return isFlat ? copySymbolsIn(value, baseAssignIn(result2, value)) : copySymbols(value, baseAssign(result2, value));
                }
              } else {
                if (!cloneableTags[tag]) {
                  return object ? value : {};
                }
                result2 = initCloneByTag(value, tag, isDeep);
              }
            }
            stack || (stack = new Stack());
            var stacked = stack.get(value);
            if (stacked) {
              return stacked;
            }
            stack.set(value, result2);
            if (isSet(value)) {
              value.forEach(function(subValue) {
                result2.add(baseClone(subValue, bitmask, customizer2, subValue, value, stack));
              });
            } else if (isMap(value)) {
              value.forEach(function(subValue, key2) {
                result2.set(key2, baseClone(subValue, bitmask, customizer2, key2, value, stack));
              });
            }
            var keysFunc = isFull ? isFlat ? getAllKeysIn : getAllKeys : isFlat ? keysIn : keys;
            var props = isArr ? undefined2 : keysFunc(value);
            arrayEach(props || value, function(subValue, key2) {
              if (props) {
                key2 = subValue;
                subValue = value[key2];
              }
              assignValue(result2, key2, baseClone(subValue, bitmask, customizer2, key2, value, stack));
            });
            return result2;
          }
          function baseConforms(source) {
            var props = keys(source);
            return function(object) {
              return baseConformsTo(object, source, props);
            };
          }
          function baseConformsTo(object, source, props) {
            var length = props.length;
            if (object == null) {
              return !length;
            }
            object = Object2(object);
            while (length--) {
              var key = props[length], predicate = source[key], value = object[key];
              if (value === undefined2 && !(key in object) || !predicate(value)) {
                return false;
              }
            }
            return true;
          }
          function baseDelay(func, wait, args) {
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            return setTimeout(function() {
              func.apply(undefined2, args);
            }, wait);
          }
          function baseDifference(array, values3, iteratee2, comparator) {
            var index = -1, includes2 = arrayIncludes, isCommon = true, length = array.length, result2 = [], valuesLength = values3.length;
            if (!length) {
              return result2;
            }
            if (iteratee2) {
              values3 = arrayMap(values3, baseUnary(iteratee2));
            }
            if (comparator) {
              includes2 = arrayIncludesWith;
              isCommon = false;
            } else if (values3.length >= LARGE_ARRAY_SIZE) {
              includes2 = cacheHas;
              isCommon = false;
              values3 = new SetCache(values3);
            }
            outer:
              while (++index < length) {
                var value = array[index], computed = iteratee2 == null ? value : iteratee2(value);
                value = comparator || value !== 0 ? value : 0;
                if (isCommon && computed === computed) {
                  var valuesIndex = valuesLength;
                  while (valuesIndex--) {
                    if (values3[valuesIndex] === computed) {
                      continue outer;
                    }
                  }
                  result2.push(value);
                } else if (!includes2(values3, computed, comparator)) {
                  result2.push(value);
                }
              }
            return result2;
          }
          var baseEach = createBaseEach(baseForOwn);
          var baseEachRight = createBaseEach(baseForOwnRight, true);
          function baseEvery(collection, predicate) {
            var result2 = true;
            baseEach(collection, function(value, index, collection2) {
              result2 = !!predicate(value, index, collection2);
              return result2;
            });
            return result2;
          }
          function baseExtremum(array, iteratee2, comparator) {
            var index = -1, length = array.length;
            while (++index < length) {
              var value = array[index], current = iteratee2(value);
              if (current != null && (computed === undefined2 ? current === current && !isSymbol(current) : comparator(current, computed))) {
                var computed = current, result2 = value;
              }
            }
            return result2;
          }
          function baseFill(array, value, start, end) {
            var length = array.length;
            start = toInteger(start);
            if (start < 0) {
              start = -start > length ? 0 : length + start;
            }
            end = end === undefined2 || end > length ? length : toInteger(end);
            if (end < 0) {
              end += length;
            }
            end = start > end ? 0 : toLength(end);
            while (start < end) {
              array[start++] = value;
            }
            return array;
          }
          function baseFilter(collection, predicate) {
            var result2 = [];
            baseEach(collection, function(value, index, collection2) {
              if (predicate(value, index, collection2)) {
                result2.push(value);
              }
            });
            return result2;
          }
          function baseFlatten(array, depth, predicate, isStrict, result2) {
            var index = -1, length = array.length;
            predicate || (predicate = isFlattenable);
            result2 || (result2 = []);
            while (++index < length) {
              var value = array[index];
              if (depth > 0 && predicate(value)) {
                if (depth > 1) {
                  baseFlatten(value, depth - 1, predicate, isStrict, result2);
                } else {
                  arrayPush(result2, value);
                }
              } else if (!isStrict) {
                result2[result2.length] = value;
              }
            }
            return result2;
          }
          var baseFor = createBaseFor();
          var baseForRight = createBaseFor(true);
          function baseForOwn(object, iteratee2) {
            return object && baseFor(object, iteratee2, keys);
          }
          function baseForOwnRight(object, iteratee2) {
            return object && baseForRight(object, iteratee2, keys);
          }
          function baseFunctions(object, props) {
            return arrayFilter(props, function(key) {
              return isFunction(object[key]);
            });
          }
          function baseGet(object, path) {
            path = castPath(path, object);
            var index = 0, length = path.length;
            while (object != null && index < length) {
              object = object[toKey(path[index++])];
            }
            return index && index == length ? object : undefined2;
          }
          function baseGetAllKeys(object, keysFunc, symbolsFunc) {
            var result2 = keysFunc(object);
            return isArray(object) ? result2 : arrayPush(result2, symbolsFunc(object));
          }
          function baseGetTag(value) {
            if (value == null) {
              return value === undefined2 ? undefinedTag : nullTag;
            }
            return symToStringTag && symToStringTag in Object2(value) ? getRawTag(value) : objectToString(value);
          }
          function baseGt(value, other) {
            return value > other;
          }
          function baseHas(object, key) {
            return object != null && hasOwnProperty.call(object, key);
          }
          function baseHasIn(object, key) {
            return object != null && key in Object2(object);
          }
          function baseInRange(number, start, end) {
            return number >= nativeMin(start, end) && number < nativeMax(start, end);
          }
          function baseIntersection(arrays, iteratee2, comparator) {
            var includes2 = comparator ? arrayIncludesWith : arrayIncludes, length = arrays[0].length, othLength = arrays.length, othIndex = othLength, caches = Array2(othLength), maxLength = Infinity, result2 = [];
            while (othIndex--) {
              var array = arrays[othIndex];
              if (othIndex && iteratee2) {
                array = arrayMap(array, baseUnary(iteratee2));
              }
              maxLength = nativeMin(array.length, maxLength);
              caches[othIndex] = !comparator && (iteratee2 || length >= 120 && array.length >= 120) ? new SetCache(othIndex && array) : undefined2;
            }
            array = arrays[0];
            var index = -1, seen = caches[0];
            outer:
              while (++index < length && result2.length < maxLength) {
                var value = array[index], computed = iteratee2 ? iteratee2(value) : value;
                value = comparator || value !== 0 ? value : 0;
                if (!(seen ? cacheHas(seen, computed) : includes2(result2, computed, comparator))) {
                  othIndex = othLength;
                  while (--othIndex) {
                    var cache = caches[othIndex];
                    if (!(cache ? cacheHas(cache, computed) : includes2(arrays[othIndex], computed, comparator))) {
                      continue outer;
                    }
                  }
                  if (seen) {
                    seen.push(computed);
                  }
                  result2.push(value);
                }
              }
            return result2;
          }
          function baseInverter(object, setter, iteratee2, accumulator) {
            baseForOwn(object, function(value, key, object2) {
              setter(accumulator, iteratee2(value), key, object2);
            });
            return accumulator;
          }
          function baseInvoke(object, path, args) {
            path = castPath(path, object);
            object = parent(object, path);
            var func = object == null ? object : object[toKey(last(path))];
            return func == null ? undefined2 : apply(func, object, args);
          }
          function baseIsArguments(value) {
            return isObjectLike(value) && baseGetTag(value) == argsTag;
          }
          function baseIsArrayBuffer(value) {
            return isObjectLike(value) && baseGetTag(value) == arrayBufferTag;
          }
          function baseIsDate(value) {
            return isObjectLike(value) && baseGetTag(value) == dateTag;
          }
          function baseIsEqual(value, other, bitmask, customizer2, stack) {
            if (value === other) {
              return true;
            }
            if (value == null || other == null || !isObjectLike(value) && !isObjectLike(other)) {
              return value !== value && other !== other;
            }
            return baseIsEqualDeep(value, other, bitmask, customizer2, baseIsEqual, stack);
          }
          function baseIsEqualDeep(object, other, bitmask, customizer2, equalFunc, stack) {
            var objIsArr = isArray(object), othIsArr = isArray(other), objTag = objIsArr ? arrayTag : getTag(object), othTag = othIsArr ? arrayTag : getTag(other);
            objTag = objTag == argsTag ? objectTag : objTag;
            othTag = othTag == argsTag ? objectTag : othTag;
            var objIsObj = objTag == objectTag, othIsObj = othTag == objectTag, isSameTag = objTag == othTag;
            if (isSameTag && isBuffer(object)) {
              if (!isBuffer(other)) {
                return false;
              }
              objIsArr = true;
              objIsObj = false;
            }
            if (isSameTag && !objIsObj) {
              stack || (stack = new Stack());
              return objIsArr || isTypedArray(object) ? equalArrays(object, other, bitmask, customizer2, equalFunc, stack) : equalByTag(object, other, objTag, bitmask, customizer2, equalFunc, stack);
            }
            if (!(bitmask & COMPARE_PARTIAL_FLAG)) {
              var objIsWrapped = objIsObj && hasOwnProperty.call(object, "__wrapped__"), othIsWrapped = othIsObj && hasOwnProperty.call(other, "__wrapped__");
              if (objIsWrapped || othIsWrapped) {
                var objUnwrapped = objIsWrapped ? object.value() : object, othUnwrapped = othIsWrapped ? other.value() : other;
                stack || (stack = new Stack());
                return equalFunc(objUnwrapped, othUnwrapped, bitmask, customizer2, stack);
              }
            }
            if (!isSameTag) {
              return false;
            }
            stack || (stack = new Stack());
            return equalObjects(object, other, bitmask, customizer2, equalFunc, stack);
          }
          function baseIsMap(value) {
            return isObjectLike(value) && getTag(value) == mapTag;
          }
          function baseIsMatch(object, source, matchData, customizer2) {
            var index = matchData.length, length = index, noCustomizer = !customizer2;
            if (object == null) {
              return !length;
            }
            object = Object2(object);
            while (index--) {
              var data = matchData[index];
              if (noCustomizer && data[2] ? data[1] !== object[data[0]] : !(data[0] in object)) {
                return false;
              }
            }
            while (++index < length) {
              data = matchData[index];
              var key = data[0], objValue = object[key], srcValue = data[1];
              if (noCustomizer && data[2]) {
                if (objValue === undefined2 && !(key in object)) {
                  return false;
                }
              } else {
                var stack = new Stack();
                if (customizer2) {
                  var result2 = customizer2(objValue, srcValue, key, object, source, stack);
                }
                if (!(result2 === undefined2 ? baseIsEqual(srcValue, objValue, COMPARE_PARTIAL_FLAG | COMPARE_UNORDERED_FLAG, customizer2, stack) : result2)) {
                  return false;
                }
              }
            }
            return true;
          }
          function baseIsNative(value) {
            if (!isObject(value) || isMasked(value)) {
              return false;
            }
            var pattern = isFunction(value) ? reIsNative : reIsHostCtor;
            return pattern.test(toSource(value));
          }
          function baseIsRegExp(value) {
            return isObjectLike(value) && baseGetTag(value) == regexpTag;
          }
          function baseIsSet(value) {
            return isObjectLike(value) && getTag(value) == setTag;
          }
          function baseIsTypedArray(value) {
            return isObjectLike(value) && isLength(value.length) && !!typedArrayTags[baseGetTag(value)];
          }
          function baseIteratee(value) {
            if (typeof value == "function") {
              return value;
            }
            if (value == null) {
              return identity;
            }
            if (typeof value == "object") {
              return isArray(value) ? baseMatchesProperty(value[0], value[1]) : baseMatches(value);
            }
            return property(value);
          }
          function baseKeys(object) {
            if (!isPrototype(object)) {
              return nativeKeys(object);
            }
            var result2 = [];
            for (var key in Object2(object)) {
              if (hasOwnProperty.call(object, key) && key != "constructor") {
                result2.push(key);
              }
            }
            return result2;
          }
          function baseKeysIn(object) {
            if (!isObject(object)) {
              return nativeKeysIn(object);
            }
            var isProto = isPrototype(object), result2 = [];
            for (var key in object) {
              if (!(key == "constructor" && (isProto || !hasOwnProperty.call(object, key)))) {
                result2.push(key);
              }
            }
            return result2;
          }
          function baseLt(value, other) {
            return value < other;
          }
          function baseMap(collection, iteratee2) {
            var index = -1, result2 = isArrayLike(collection) ? Array2(collection.length) : [];
            baseEach(collection, function(value, key, collection2) {
              result2[++index] = iteratee2(value, key, collection2);
            });
            return result2;
          }
          function baseMatches(source) {
            var matchData = getMatchData(source);
            if (matchData.length == 1 && matchData[0][2]) {
              return matchesStrictComparable(matchData[0][0], matchData[0][1]);
            }
            return function(object) {
              return object === source || baseIsMatch(object, source, matchData);
            };
          }
          function baseMatchesProperty(path, srcValue) {
            if (isKey(path) && isStrictComparable(srcValue)) {
              return matchesStrictComparable(toKey(path), srcValue);
            }
            return function(object) {
              var objValue = get(object, path);
              return objValue === undefined2 && objValue === srcValue ? hasIn(object, path) : baseIsEqual(srcValue, objValue, COMPARE_PARTIAL_FLAG | COMPARE_UNORDERED_FLAG);
            };
          }
          function baseMerge(object, source, srcIndex, customizer2, stack) {
            if (object === source) {
              return;
            }
            baseFor(source, function(srcValue, key) {
              stack || (stack = new Stack());
              if (isObject(srcValue)) {
                baseMergeDeep(object, source, key, srcIndex, baseMerge, customizer2, stack);
              } else {
                var newValue = customizer2 ? customizer2(safeGet(object, key), srcValue, key + "", object, source, stack) : undefined2;
                if (newValue === undefined2) {
                  newValue = srcValue;
                }
                assignMergeValue(object, key, newValue);
              }
            }, keysIn);
          }
          function baseMergeDeep(object, source, key, srcIndex, mergeFunc, customizer2, stack) {
            var objValue = safeGet(object, key), srcValue = safeGet(source, key), stacked = stack.get(srcValue);
            if (stacked) {
              assignMergeValue(object, key, stacked);
              return;
            }
            var newValue = customizer2 ? customizer2(objValue, srcValue, key + "", object, source, stack) : undefined2;
            var isCommon = newValue === undefined2;
            if (isCommon) {
              var isArr = isArray(srcValue), isBuff = !isArr && isBuffer(srcValue), isTyped = !isArr && !isBuff && isTypedArray(srcValue);
              newValue = srcValue;
              if (isArr || isBuff || isTyped) {
                if (isArray(objValue)) {
                  newValue = objValue;
                } else if (isArrayLikeObject(objValue)) {
                  newValue = copyArray(objValue);
                } else if (isBuff) {
                  isCommon = false;
                  newValue = cloneBuffer(srcValue, true);
                } else if (isTyped) {
                  isCommon = false;
                  newValue = cloneTypedArray(srcValue, true);
                } else {
                  newValue = [];
                }
              } else if (isPlainObject(srcValue) || isArguments(srcValue)) {
                newValue = objValue;
                if (isArguments(objValue)) {
                  newValue = toPlainObject(objValue);
                } else if (!isObject(objValue) || isFunction(objValue)) {
                  newValue = initCloneObject(srcValue);
                }
              } else {
                isCommon = false;
              }
            }
            if (isCommon) {
              stack.set(srcValue, newValue);
              mergeFunc(newValue, srcValue, srcIndex, customizer2, stack);
              stack["delete"](srcValue);
            }
            assignMergeValue(object, key, newValue);
          }
          function baseNth(array, n) {
            var length = array.length;
            if (!length) {
              return;
            }
            n += n < 0 ? length : 0;
            return isIndex(n, length) ? array[n] : undefined2;
          }
          function baseOrderBy(collection, iteratees, orders) {
            if (iteratees.length) {
              iteratees = arrayMap(iteratees, function(iteratee2) {
                if (isArray(iteratee2)) {
                  return function(value) {
                    return baseGet(value, iteratee2.length === 1 ? iteratee2[0] : iteratee2);
                  };
                }
                return iteratee2;
              });
            } else {
              iteratees = [identity];
            }
            var index = -1;
            iteratees = arrayMap(iteratees, baseUnary(getIteratee()));
            var result2 = baseMap(collection, function(value, key, collection2) {
              var criteria = arrayMap(iteratees, function(iteratee2) {
                return iteratee2(value);
              });
              return { "criteria": criteria, "index": ++index, "value": value };
            });
            return baseSortBy(result2, function(object, other) {
              return compareMultiple(object, other, orders);
            });
          }
          function basePick(object, paths) {
            return basePickBy(object, paths, function(value, path) {
              return hasIn(object, path);
            });
          }
          function basePickBy(object, paths, predicate) {
            var index = -1, length = paths.length, result2 = {};
            while (++index < length) {
              var path = paths[index], value = baseGet(object, path);
              if (predicate(value, path)) {
                baseSet(result2, castPath(path, object), value);
              }
            }
            return result2;
          }
          function basePropertyDeep(path) {
            return function(object) {
              return baseGet(object, path);
            };
          }
          function basePullAll(array, values3, iteratee2, comparator) {
            var indexOf2 = comparator ? baseIndexOfWith : baseIndexOf, index = -1, length = values3.length, seen = array;
            if (array === values3) {
              values3 = copyArray(values3);
            }
            if (iteratee2) {
              seen = arrayMap(array, baseUnary(iteratee2));
            }
            while (++index < length) {
              var fromIndex = 0, value = values3[index], computed = iteratee2 ? iteratee2(value) : value;
              while ((fromIndex = indexOf2(seen, computed, fromIndex, comparator)) > -1) {
                if (seen !== array) {
                  splice.call(seen, fromIndex, 1);
                }
                splice.call(array, fromIndex, 1);
              }
            }
            return array;
          }
          function basePullAt(array, indexes) {
            var length = array ? indexes.length : 0, lastIndex = length - 1;
            while (length--) {
              var index = indexes[length];
              if (length == lastIndex || index !== previous) {
                var previous = index;
                if (isIndex(index)) {
                  splice.call(array, index, 1);
                } else {
                  baseUnset(array, index);
                }
              }
            }
            return array;
          }
          function baseRandom(lower, upper) {
            return lower + nativeFloor(nativeRandom() * (upper - lower + 1));
          }
          function baseRange(start, end, step, fromRight) {
            var index = -1, length = nativeMax(nativeCeil((end - start) / (step || 1)), 0), result2 = Array2(length);
            while (length--) {
              result2[fromRight ? length : ++index] = start;
              start += step;
            }
            return result2;
          }
          function baseRepeat(string, n) {
            var result2 = "";
            if (!string || n < 1 || n > MAX_SAFE_INTEGER) {
              return result2;
            }
            do {
              if (n % 2) {
                result2 += string;
              }
              n = nativeFloor(n / 2);
              if (n) {
                string += string;
              }
            } while (n);
            return result2;
          }
          function baseRest(func, start) {
            return setToString(overRest(func, start, identity), func + "");
          }
          function baseSample(collection) {
            return arraySample(values2(collection));
          }
          function baseSampleSize(collection, n) {
            var array = values2(collection);
            return shuffleSelf(array, baseClamp(n, 0, array.length));
          }
          function baseSet(object, path, value, customizer2) {
            if (!isObject(object)) {
              return object;
            }
            path = castPath(path, object);
            var index = -1, length = path.length, lastIndex = length - 1, nested = object;
            while (nested != null && ++index < length) {
              var key = toKey(path[index]), newValue = value;
              if (key === "__proto__" || key === "constructor" || key === "prototype") {
                return object;
              }
              if (index != lastIndex) {
                var objValue = nested[key];
                newValue = customizer2 ? customizer2(objValue, key, nested) : undefined2;
                if (newValue === undefined2) {
                  newValue = isObject(objValue) ? objValue : isIndex(path[index + 1]) ? [] : {};
                }
              }
              assignValue(nested, key, newValue);
              nested = nested[key];
            }
            return object;
          }
          var baseSetData = !metaMap ? identity : function(func, data) {
            metaMap.set(func, data);
            return func;
          };
          var baseSetToString = !defineProperty ? identity : function(func, string) {
            return defineProperty(func, "toString", {
              "configurable": true,
              "enumerable": false,
              "value": constant(string),
              "writable": true
            });
          };
          function baseShuffle(collection) {
            return shuffleSelf(values2(collection));
          }
          function baseSlice(array, start, end) {
            var index = -1, length = array.length;
            if (start < 0) {
              start = -start > length ? 0 : length + start;
            }
            end = end > length ? length : end;
            if (end < 0) {
              end += length;
            }
            length = start > end ? 0 : end - start >>> 0;
            start >>>= 0;
            var result2 = Array2(length);
            while (++index < length) {
              result2[index] = array[index + start];
            }
            return result2;
          }
          function baseSome(collection, predicate) {
            var result2;
            baseEach(collection, function(value, index, collection2) {
              result2 = predicate(value, index, collection2);
              return !result2;
            });
            return !!result2;
          }
          function baseSortedIndex(array, value, retHighest) {
            var low = 0, high = array == null ? low : array.length;
            if (typeof value == "number" && value === value && high <= HALF_MAX_ARRAY_LENGTH) {
              while (low < high) {
                var mid = low + high >>> 1, computed = array[mid];
                if (computed !== null && !isSymbol(computed) && (retHighest ? computed <= value : computed < value)) {
                  low = mid + 1;
                } else {
                  high = mid;
                }
              }
              return high;
            }
            return baseSortedIndexBy(array, value, identity, retHighest);
          }
          function baseSortedIndexBy(array, value, iteratee2, retHighest) {
            var low = 0, high = array == null ? 0 : array.length;
            if (high === 0) {
              return 0;
            }
            value = iteratee2(value);
            var valIsNaN = value !== value, valIsNull = value === null, valIsSymbol = isSymbol(value), valIsUndefined = value === undefined2;
            while (low < high) {
              var mid = nativeFloor((low + high) / 2), computed = iteratee2(array[mid]), othIsDefined = computed !== undefined2, othIsNull = computed === null, othIsReflexive = computed === computed, othIsSymbol = isSymbol(computed);
              if (valIsNaN) {
                var setLow = retHighest || othIsReflexive;
              } else if (valIsUndefined) {
                setLow = othIsReflexive && (retHighest || othIsDefined);
              } else if (valIsNull) {
                setLow = othIsReflexive && othIsDefined && (retHighest || !othIsNull);
              } else if (valIsSymbol) {
                setLow = othIsReflexive && othIsDefined && !othIsNull && (retHighest || !othIsSymbol);
              } else if (othIsNull || othIsSymbol) {
                setLow = false;
              } else {
                setLow = retHighest ? computed <= value : computed < value;
              }
              if (setLow) {
                low = mid + 1;
              } else {
                high = mid;
              }
            }
            return nativeMin(high, MAX_ARRAY_INDEX);
          }
          function baseSortedUniq(array, iteratee2) {
            var index = -1, length = array.length, resIndex = 0, result2 = [];
            while (++index < length) {
              var value = array[index], computed = iteratee2 ? iteratee2(value) : value;
              if (!index || !eq(computed, seen)) {
                var seen = computed;
                result2[resIndex++] = value === 0 ? 0 : value;
              }
            }
            return result2;
          }
          function baseToNumber(value) {
            if (typeof value == "number") {
              return value;
            }
            if (isSymbol(value)) {
              return NAN;
            }
            return +value;
          }
          function baseToString(value) {
            if (typeof value == "string") {
              return value;
            }
            if (isArray(value)) {
              return arrayMap(value, baseToString) + "";
            }
            if (isSymbol(value)) {
              return symbolToString ? symbolToString.call(value) : "";
            }
            var result2 = value + "";
            return result2 == "0" && 1 / value == -INFINITY ? "-0" : result2;
          }
          function baseUniq(array, iteratee2, comparator) {
            var index = -1, includes2 = arrayIncludes, length = array.length, isCommon = true, result2 = [], seen = result2;
            if (comparator) {
              isCommon = false;
              includes2 = arrayIncludesWith;
            } else if (length >= LARGE_ARRAY_SIZE) {
              var set2 = iteratee2 ? null : createSet(array);
              if (set2) {
                return setToArray(set2);
              }
              isCommon = false;
              includes2 = cacheHas;
              seen = new SetCache();
            } else {
              seen = iteratee2 ? [] : result2;
            }
            outer:
              while (++index < length) {
                var value = array[index], computed = iteratee2 ? iteratee2(value) : value;
                value = comparator || value !== 0 ? value : 0;
                if (isCommon && computed === computed) {
                  var seenIndex = seen.length;
                  while (seenIndex--) {
                    if (seen[seenIndex] === computed) {
                      continue outer;
                    }
                  }
                  if (iteratee2) {
                    seen.push(computed);
                  }
                  result2.push(value);
                } else if (!includes2(seen, computed, comparator)) {
                  if (seen !== result2) {
                    seen.push(computed);
                  }
                  result2.push(value);
                }
              }
            return result2;
          }
          function baseUnset(object, path) {
            path = castPath(path, object);
            var index = -1, length = path.length;
            if (!length) {
              return true;
            }
            while (++index < length) {
              var key = toKey(path[index]);
              if (key === "__proto__" && !hasOwnProperty.call(object, "__proto__")) {
                return false;
              }
              if ((key === "constructor" || key === "prototype") && index < length - 1) {
                return false;
              }
            }
            var obj = parent(object, path);
            return obj == null || delete obj[toKey(last(path))];
          }
          function baseUpdate(object, path, updater, customizer2) {
            return baseSet(object, path, updater(baseGet(object, path)), customizer2);
          }
          function baseWhile(array, predicate, isDrop, fromRight) {
            var length = array.length, index = fromRight ? length : -1;
            while ((fromRight ? index-- : ++index < length) && predicate(array[index], index, array)) {
            }
            return isDrop ? baseSlice(array, fromRight ? 0 : index, fromRight ? index + 1 : length) : baseSlice(array, fromRight ? index + 1 : 0, fromRight ? length : index);
          }
          function baseWrapperValue(value, actions) {
            var result2 = value;
            if (result2 instanceof LazyWrapper) {
              result2 = result2.value();
            }
            return arrayReduce(actions, function(result3, action) {
              return action.func.apply(action.thisArg, arrayPush([result3], action.args));
            }, result2);
          }
          function baseXor(arrays, iteratee2, comparator) {
            var length = arrays.length;
            if (length < 2) {
              return length ? baseUniq(arrays[0]) : [];
            }
            var index = -1, result2 = Array2(length);
            while (++index < length) {
              var array = arrays[index], othIndex = -1;
              while (++othIndex < length) {
                if (othIndex != index) {
                  result2[index] = baseDifference(result2[index] || array, arrays[othIndex], iteratee2, comparator);
                }
              }
            }
            return baseUniq(baseFlatten(result2, 1), iteratee2, comparator);
          }
          function baseZipObject(props, values3, assignFunc) {
            var index = -1, length = props.length, valsLength = values3.length, result2 = {};
            while (++index < length) {
              var value = index < valsLength ? values3[index] : undefined2;
              assignFunc(result2, props[index], value);
            }
            return result2;
          }
          function castArrayLikeObject(value) {
            return isArrayLikeObject(value) ? value : [];
          }
          function castFunction(value) {
            return typeof value == "function" ? value : identity;
          }
          function castPath(value, object) {
            if (isArray(value)) {
              return value;
            }
            return isKey(value, object) ? [value] : stringToPath(toString(value));
          }
          var castRest = baseRest;
          function castSlice(array, start, end) {
            var length = array.length;
            end = end === undefined2 ? length : end;
            return !start && end >= length ? array : baseSlice(array, start, end);
          }
          var clearTimeout = ctxClearTimeout || function(id2) {
            return root.clearTimeout(id2);
          };
          function cloneBuffer(buffer, isDeep) {
            if (isDeep) {
              return buffer.slice();
            }
            var length = buffer.length, result2 = allocUnsafe ? allocUnsafe(length) : new buffer.constructor(length);
            buffer.copy(result2);
            return result2;
          }
          function cloneArrayBuffer(arrayBuffer) {
            var result2 = new arrayBuffer.constructor(arrayBuffer.byteLength);
            new Uint8Array2(result2).set(new Uint8Array2(arrayBuffer));
            return result2;
          }
          function cloneDataView(dataView, isDeep) {
            var buffer = isDeep ? cloneArrayBuffer(dataView.buffer) : dataView.buffer;
            return new dataView.constructor(buffer, dataView.byteOffset, dataView.byteLength);
          }
          function cloneRegExp(regexp) {
            var result2 = new regexp.constructor(regexp.source, reFlags.exec(regexp));
            result2.lastIndex = regexp.lastIndex;
            return result2;
          }
          function cloneSymbol(symbol) {
            return symbolValueOf ? Object2(symbolValueOf.call(symbol)) : {};
          }
          function cloneTypedArray(typedArray, isDeep) {
            var buffer = isDeep ? cloneArrayBuffer(typedArray.buffer) : typedArray.buffer;
            return new typedArray.constructor(buffer, typedArray.byteOffset, typedArray.length);
          }
          function compareAscending(value, other) {
            if (value !== other) {
              var valIsDefined = value !== undefined2, valIsNull = value === null, valIsReflexive = value === value, valIsSymbol = isSymbol(value);
              var othIsDefined = other !== undefined2, othIsNull = other === null, othIsReflexive = other === other, othIsSymbol = isSymbol(other);
              if (!othIsNull && !othIsSymbol && !valIsSymbol && value > other || valIsSymbol && othIsDefined && othIsReflexive && !othIsNull && !othIsSymbol || valIsNull && othIsDefined && othIsReflexive || !valIsDefined && othIsReflexive || !valIsReflexive) {
                return 1;
              }
              if (!valIsNull && !valIsSymbol && !othIsSymbol && value < other || othIsSymbol && valIsDefined && valIsReflexive && !valIsNull && !valIsSymbol || othIsNull && valIsDefined && valIsReflexive || !othIsDefined && valIsReflexive || !othIsReflexive) {
                return -1;
              }
            }
            return 0;
          }
          function compareMultiple(object, other, orders) {
            var index = -1, objCriteria = object.criteria, othCriteria = other.criteria, length = objCriteria.length, ordersLength = orders.length;
            while (++index < length) {
              var result2 = compareAscending(objCriteria[index], othCriteria[index]);
              if (result2) {
                if (index >= ordersLength) {
                  return result2;
                }
                var order = orders[index];
                return result2 * (order == "desc" ? -1 : 1);
              }
            }
            return object.index - other.index;
          }
          function composeArgs(args, partials, holders, isCurried) {
            var argsIndex = -1, argsLength = args.length, holdersLength = holders.length, leftIndex = -1, leftLength = partials.length, rangeLength = nativeMax(argsLength - holdersLength, 0), result2 = Array2(leftLength + rangeLength), isUncurried = !isCurried;
            while (++leftIndex < leftLength) {
              result2[leftIndex] = partials[leftIndex];
            }
            while (++argsIndex < holdersLength) {
              if (isUncurried || argsIndex < argsLength) {
                result2[holders[argsIndex]] = args[argsIndex];
              }
            }
            while (rangeLength--) {
              result2[leftIndex++] = args[argsIndex++];
            }
            return result2;
          }
          function composeArgsRight(args, partials, holders, isCurried) {
            var argsIndex = -1, argsLength = args.length, holdersIndex = -1, holdersLength = holders.length, rightIndex = -1, rightLength = partials.length, rangeLength = nativeMax(argsLength - holdersLength, 0), result2 = Array2(rangeLength + rightLength), isUncurried = !isCurried;
            while (++argsIndex < rangeLength) {
              result2[argsIndex] = args[argsIndex];
            }
            var offset = argsIndex;
            while (++rightIndex < rightLength) {
              result2[offset + rightIndex] = partials[rightIndex];
            }
            while (++holdersIndex < holdersLength) {
              if (isUncurried || argsIndex < argsLength) {
                result2[offset + holders[holdersIndex]] = args[argsIndex++];
              }
            }
            return result2;
          }
          function copyArray(source, array) {
            var index = -1, length = source.length;
            array || (array = Array2(length));
            while (++index < length) {
              array[index] = source[index];
            }
            return array;
          }
          function copyObject(source, props, object, customizer2) {
            var isNew = !object;
            object || (object = {});
            var index = -1, length = props.length;
            while (++index < length) {
              var key = props[index];
              var newValue = customizer2 ? customizer2(object[key], source[key], key, object, source) : undefined2;
              if (newValue === undefined2) {
                newValue = source[key];
              }
              if (isNew) {
                baseAssignValue(object, key, newValue);
              } else {
                assignValue(object, key, newValue);
              }
            }
            return object;
          }
          function copySymbols(source, object) {
            return copyObject(source, getSymbols(source), object);
          }
          function copySymbolsIn(source, object) {
            return copyObject(source, getSymbolsIn(source), object);
          }
          function createAggregator(setter, initializer) {
            return function(collection, iteratee2) {
              var func = isArray(collection) ? arrayAggregator : baseAggregator, accumulator = initializer ? initializer() : {};
              return func(collection, setter, getIteratee(iteratee2, 2), accumulator);
            };
          }
          function createAssigner(assigner) {
            return baseRest(function(object, sources) {
              var index = -1, length = sources.length, customizer2 = length > 1 ? sources[length - 1] : undefined2, guard = length > 2 ? sources[2] : undefined2;
              customizer2 = assigner.length > 3 && typeof customizer2 == "function" ? (length--, customizer2) : undefined2;
              if (guard && isIterateeCall(sources[0], sources[1], guard)) {
                customizer2 = length < 3 ? undefined2 : customizer2;
                length = 1;
              }
              object = Object2(object);
              while (++index < length) {
                var source = sources[index];
                if (source) {
                  assigner(object, source, index, customizer2);
                }
              }
              return object;
            });
          }
          function createBaseEach(eachFunc, fromRight) {
            return function(collection, iteratee2) {
              if (collection == null) {
                return collection;
              }
              if (!isArrayLike(collection)) {
                return eachFunc(collection, iteratee2);
              }
              var length = collection.length, index = fromRight ? length : -1, iterable = Object2(collection);
              while (fromRight ? index-- : ++index < length) {
                if (iteratee2(iterable[index], index, iterable) === false) {
                  break;
                }
              }
              return collection;
            };
          }
          function createBaseFor(fromRight) {
            return function(object, iteratee2, keysFunc) {
              var index = -1, iterable = Object2(object), props = keysFunc(object), length = props.length;
              while (length--) {
                var key = props[fromRight ? length : ++index];
                if (iteratee2(iterable[key], key, iterable) === false) {
                  break;
                }
              }
              return object;
            };
          }
          function createBind(func, bitmask, thisArg) {
            var isBind = bitmask & WRAP_BIND_FLAG, Ctor = createCtor(func);
            function wrapper() {
              var fn = this && this !== root && this instanceof wrapper ? Ctor : func;
              return fn.apply(isBind ? thisArg : this, arguments);
            }
            return wrapper;
          }
          function createCaseFirst(methodName) {
            return function(string) {
              string = toString(string);
              var strSymbols = hasUnicode(string) ? stringToArray(string) : undefined2;
              var chr = strSymbols ? strSymbols[0] : string.charAt(0);
              var trailing = strSymbols ? castSlice(strSymbols, 1).join("") : string.slice(1);
              return chr[methodName]() + trailing;
            };
          }
          function createCompounder(callback) {
            return function(string) {
              return arrayReduce(words(deburr(string).replace(reApos, "")), callback, "");
            };
          }
          function createCtor(Ctor) {
            return function() {
              var args = arguments;
              switch (args.length) {
                case 0:
                  return new Ctor();
                case 1:
                  return new Ctor(args[0]);
                case 2:
                  return new Ctor(args[0], args[1]);
                case 3:
                  return new Ctor(args[0], args[1], args[2]);
                case 4:
                  return new Ctor(args[0], args[1], args[2], args[3]);
                case 5:
                  return new Ctor(args[0], args[1], args[2], args[3], args[4]);
                case 6:
                  return new Ctor(args[0], args[1], args[2], args[3], args[4], args[5]);
                case 7:
                  return new Ctor(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
              }
              var thisBinding = baseCreate(Ctor.prototype), result2 = Ctor.apply(thisBinding, args);
              return isObject(result2) ? result2 : thisBinding;
            };
          }
          function createCurry(func, bitmask, arity) {
            var Ctor = createCtor(func);
            function wrapper() {
              var length = arguments.length, args = Array2(length), index = length, placeholder = getHolder(wrapper);
              while (index--) {
                args[index] = arguments[index];
              }
              var holders = length < 3 && args[0] !== placeholder && args[length - 1] !== placeholder ? [] : replaceHolders(args, placeholder);
              length -= holders.length;
              if (length < arity) {
                return createRecurry(
                  func,
                  bitmask,
                  createHybrid,
                  wrapper.placeholder,
                  undefined2,
                  args,
                  holders,
                  undefined2,
                  undefined2,
                  arity - length
                );
              }
              var fn = this && this !== root && this instanceof wrapper ? Ctor : func;
              return apply(fn, this, args);
            }
            return wrapper;
          }
          function createFind(findIndexFunc) {
            return function(collection, predicate, fromIndex) {
              var iterable = Object2(collection);
              if (!isArrayLike(collection)) {
                var iteratee2 = getIteratee(predicate, 3);
                collection = keys(collection);
                predicate = function(key) {
                  return iteratee2(iterable[key], key, iterable);
                };
              }
              var index = findIndexFunc(collection, predicate, fromIndex);
              return index > -1 ? iterable[iteratee2 ? collection[index] : index] : undefined2;
            };
          }
          function createFlow(fromRight) {
            return flatRest(function(funcs) {
              var length = funcs.length, index = length, prereq = LodashWrapper.prototype.thru;
              if (fromRight) {
                funcs.reverse();
              }
              while (index--) {
                var func = funcs[index];
                if (typeof func != "function") {
                  throw String(FUNC_ERROR_TEXT);
                }
                if (prereq && !wrapper && getFuncName(func) == "wrapper") {
                  var wrapper = new LodashWrapper([], true);
                }
              }
              index = wrapper ? index : length;
              while (++index < length) {
                func = funcs[index];
                var funcName = getFuncName(func), data = funcName == "wrapper" ? getData(func) : undefined2;
                if (data && isLaziable(data[0]) && data[1] == (WRAP_ARY_FLAG | WRAP_CURRY_FLAG | WRAP_PARTIAL_FLAG | WRAP_REARG_FLAG) && !data[4].length && data[9] == 1) {
                  wrapper = wrapper[getFuncName(data[0])].apply(wrapper, data[3]);
                } else {
                  wrapper = func.length == 1 && isLaziable(func) ? wrapper[funcName]() : wrapper.thru(func);
                }
              }
              return function() {
                var args = arguments, value = args[0];
                if (wrapper && args.length == 1 && isArray(value)) {
                  return wrapper.plant(value).value();
                }
                var index2 = 0, result2 = length ? funcs[index2].apply(this, args) : value;
                while (++index2 < length) {
                  result2 = funcs[index2].call(this, result2);
                }
                return result2;
              };
            });
          }
          function createHybrid(func, bitmask, thisArg, partials, holders, partialsRight, holdersRight, argPos, ary2, arity) {
            var isAry = bitmask & WRAP_ARY_FLAG, isBind = bitmask & WRAP_BIND_FLAG, isBindKey = bitmask & WRAP_BIND_KEY_FLAG, isCurried = bitmask & (WRAP_CURRY_FLAG | WRAP_CURRY_RIGHT_FLAG), isFlip = bitmask & WRAP_FLIP_FLAG, Ctor = isBindKey ? undefined2 : createCtor(func);
            function wrapper() {
              var length = arguments.length, args = Array2(length), index = length;
              while (index--) {
                args[index] = arguments[index];
              }
              if (isCurried) {
                var placeholder = getHolder(wrapper), holdersCount = countHolders(args, placeholder);
              }
              if (partials) {
                args = composeArgs(args, partials, holders, isCurried);
              }
              if (partialsRight) {
                args = composeArgsRight(args, partialsRight, holdersRight, isCurried);
              }
              length -= holdersCount;
              if (isCurried && length < arity) {
                var newHolders = replaceHolders(args, placeholder);
                return createRecurry(
                  func,
                  bitmask,
                  createHybrid,
                  wrapper.placeholder,
                  thisArg,
                  args,
                  newHolders,
                  argPos,
                  ary2,
                  arity - length
                );
              }
              var thisBinding = isBind ? thisArg : this, fn = isBindKey ? thisBinding[func] : func;
              length = args.length;
              if (argPos) {
                args = reorder(args, argPos);
              } else if (isFlip && length > 1) {
                args.reverse();
              }
              if (isAry && ary2 < length) {
                args.length = ary2;
              }
              if (this && this !== root && this instanceof wrapper) {
                fn = Ctor || createCtor(fn);
              }
              return fn.apply(thisBinding, args);
            }
            return wrapper;
          }
          function createInverter(setter, toIteratee) {
            return function(object, iteratee2) {
              return baseInverter(object, setter, toIteratee(iteratee2), {});
            };
          }
          function createMathOperation(operator, defaultValue) {
            return function(value, other) {
              var result2;
              if (value === undefined2 && other === undefined2) {
                return defaultValue;
              }
              if (value !== undefined2) {
                result2 = value;
              }
              if (other !== undefined2) {
                if (result2 === undefined2) {
                  return other;
                }
                if (typeof value == "string" || typeof other == "string") {
                  value = baseToString(value);
                  other = baseToString(other);
                } else {
                  value = baseToNumber(value);
                  other = baseToNumber(other);
                }
                result2 = operator(value, other);
              }
              return result2;
            };
          }
          function createOver(arrayFunc) {
            return flatRest(function(iteratees) {
              iteratees = arrayMap(iteratees, baseUnary(getIteratee()));
              return baseRest(function(args) {
                var thisArg = this;
                return arrayFunc(iteratees, function(iteratee2) {
                  return apply(iteratee2, thisArg, args);
                });
              });
            });
          }
          function createPadding(length, chars) {
            chars = chars === undefined2 ? " " : baseToString(chars);
            var charsLength = chars.length;
            if (charsLength < 2) {
              return charsLength ? baseRepeat(chars, length) : chars;
            }
            var result2 = baseRepeat(chars, nativeCeil(length / stringSize(chars)));
            return hasUnicode(chars) ? castSlice(stringToArray(result2), 0, length).join("") : result2.slice(0, length);
          }
          function createPartial(func, bitmask, thisArg, partials) {
            var isBind = bitmask & WRAP_BIND_FLAG, Ctor = createCtor(func);
            function wrapper() {
              var argsIndex = -1, argsLength = arguments.length, leftIndex = -1, leftLength = partials.length, args = Array2(leftLength + argsLength), fn = this && this !== root && this instanceof wrapper ? Ctor : func;
              while (++leftIndex < leftLength) {
                args[leftIndex] = partials[leftIndex];
              }
              while (argsLength--) {
                args[leftIndex++] = arguments[++argsIndex];
              }
              return apply(fn, isBind ? thisArg : this, args);
            }
            return wrapper;
          }
          function createRange(fromRight) {
            return function(start, end, step) {
              if (step && typeof step != "number" && isIterateeCall(start, end, step)) {
                end = step = undefined2;
              }
              start = toFinite(start);
              if (end === undefined2) {
                end = start;
                start = 0;
              } else {
                end = toFinite(end);
              }
              step = step === undefined2 ? start < end ? 1 : -1 : toFinite(step);
              return baseRange(start, end, step, fromRight);
            };
          }
          function createRelationalOperation(operator) {
            return function(value, other) {
              if (!(typeof value == "string" && typeof other == "string")) {
                value = toNumber2(value);
                other = toNumber2(other);
              }
              return operator(value, other);
            };
          }
          function createRecurry(func, bitmask, wrapFunc, placeholder, thisArg, partials, holders, argPos, ary2, arity) {
            var isCurry = bitmask & WRAP_CURRY_FLAG, newHolders = isCurry ? holders : undefined2, newHoldersRight = isCurry ? undefined2 : holders, newPartials = isCurry ? partials : undefined2, newPartialsRight = isCurry ? undefined2 : partials;
            bitmask |= isCurry ? WRAP_PARTIAL_FLAG : WRAP_PARTIAL_RIGHT_FLAG;
            bitmask &= ~(isCurry ? WRAP_PARTIAL_RIGHT_FLAG : WRAP_PARTIAL_FLAG);
            if (!(bitmask & WRAP_CURRY_BOUND_FLAG)) {
              bitmask &= ~(WRAP_BIND_FLAG | WRAP_BIND_KEY_FLAG);
            }
            var newData = [
              func,
              bitmask,
              thisArg,
              newPartials,
              newHolders,
              newPartialsRight,
              newHoldersRight,
              argPos,
              ary2,
              arity
            ];
            var result2 = wrapFunc.apply(undefined2, newData);
            if (isLaziable(func)) {
              setData(result2, newData);
            }
            result2.placeholder = placeholder;
            return setWrapToString(result2, func, bitmask);
          }
          function createRound(methodName) {
            var func = Math2[methodName];
            return function(number, precision) {
              number = toNumber2(number);
              precision = precision == null ? 0 : nativeMin(toInteger(precision), 292);
              if (precision && nativeIsFinite(number)) {
                var pair = (toString(number) + "e").split("e"), value = func(pair[0] + "e" + (+pair[1] + precision));
                pair = (toString(value) + "e").split("e");
                return +(pair[0] + "e" + (+pair[1] - precision));
              }
              return func(number);
            };
          }
          var createSet = !(Set2 && 1 / setToArray(new Set2([, -0]))[1] == INFINITY) ? noop : function(values3) {
            return new Set2(values3);
          };
          function createToPairs(keysFunc) {
            return function(object) {
              var tag = getTag(object);
              if (tag == mapTag) {
                return mapToArray(object);
              }
              if (tag == setTag) {
                return setToPairs(object);
              }
              return baseToPairs(object, keysFunc(object));
            };
          }
          function createWrap(func, bitmask, thisArg, partials, holders, argPos, ary2, arity) {
            var isBindKey = bitmask & WRAP_BIND_KEY_FLAG;
            if (!isBindKey && typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            var length = partials ? partials.length : 0;
            if (!length) {
              bitmask &= ~(WRAP_PARTIAL_FLAG | WRAP_PARTIAL_RIGHT_FLAG);
              partials = holders = undefined2;
            }
            ary2 = ary2 === undefined2 ? ary2 : nativeMax(toInteger(ary2), 0);
            arity = arity === undefined2 ? arity : toInteger(arity);
            length -= holders ? holders.length : 0;
            if (bitmask & WRAP_PARTIAL_RIGHT_FLAG) {
              var partialsRight = partials, holdersRight = holders;
              partials = holders = undefined2;
            }
            var data = isBindKey ? undefined2 : getData(func);
            var newData = [
              func,
              bitmask,
              thisArg,
              partials,
              holders,
              partialsRight,
              holdersRight,
              argPos,
              ary2,
              arity
            ];
            if (data) {
              mergeData(newData, data);
            }
            func = newData[0];
            bitmask = newData[1];
            thisArg = newData[2];
            partials = newData[3];
            holders = newData[4];
            arity = newData[9] = newData[9] === undefined2 ? isBindKey ? 0 : func.length : nativeMax(newData[9] - length, 0);
            if (!arity && bitmask & (WRAP_CURRY_FLAG | WRAP_CURRY_RIGHT_FLAG)) {
              bitmask &= ~(WRAP_CURRY_FLAG | WRAP_CURRY_RIGHT_FLAG);
            }
            if (!bitmask || bitmask == WRAP_BIND_FLAG) {
              var result2 = createBind(func, bitmask, thisArg);
            } else if (bitmask == WRAP_CURRY_FLAG || bitmask == WRAP_CURRY_RIGHT_FLAG) {
              result2 = createCurry(func, bitmask, arity);
            } else if ((bitmask == WRAP_PARTIAL_FLAG || bitmask == (WRAP_BIND_FLAG | WRAP_PARTIAL_FLAG)) && !holders.length) {
              result2 = createPartial(func, bitmask, thisArg, partials);
            } else {
              result2 = createHybrid.apply(undefined2, newData);
            }
            var setter = data ? baseSetData : setData;
            return setWrapToString(setter(result2, newData), func, bitmask);
          }
          function customDefaultsAssignIn(objValue, srcValue, key, object) {
            if (objValue === undefined2 || eq(objValue, objectProto[key]) && !hasOwnProperty.call(object, key)) {
              return srcValue;
            }
            return objValue;
          }
          function customDefaultsMerge(objValue, srcValue, key, object, source, stack) {
            if (isObject(objValue) && isObject(srcValue)) {
              stack.set(srcValue, objValue);
              baseMerge(objValue, srcValue, undefined2, customDefaultsMerge, stack);
              stack["delete"](srcValue);
            }
            return objValue;
          }
          function customOmitClone(value) {
            return isPlainObject(value) ? undefined2 : value;
          }
          function equalArrays(array, other, bitmask, customizer2, equalFunc, stack) {
            var isPartial = bitmask & COMPARE_PARTIAL_FLAG, arrLength = array.length, othLength = other.length;
            if (arrLength != othLength && !(isPartial && othLength > arrLength)) {
              return false;
            }
            var arrStacked = stack.get(array);
            var othStacked = stack.get(other);
            if (arrStacked && othStacked) {
              return arrStacked == other && othStacked == array;
            }
            var index = -1, result2 = true, seen = bitmask & COMPARE_UNORDERED_FLAG ? new SetCache() : undefined2;
            stack.set(array, other);
            stack.set(other, array);
            while (++index < arrLength) {
              var arrValue = array[index], othValue = other[index];
              if (customizer2) {
                var compared = isPartial ? customizer2(othValue, arrValue, index, other, array, stack) : customizer2(arrValue, othValue, index, array, other, stack);
              }
              if (compared !== undefined2) {
                if (compared) {
                  continue;
                }
                result2 = false;
                break;
              }
              if (seen) {
                if (!arraySome(other, function(othValue2, othIndex) {
                  if (!cacheHas(seen, othIndex) && (arrValue === othValue2 || equalFunc(arrValue, othValue2, bitmask, customizer2, stack))) {
                    return seen.push(othIndex);
                  }
                })) {
                  result2 = false;
                  break;
                }
              } else if (!(arrValue === othValue || equalFunc(arrValue, othValue, bitmask, customizer2, stack))) {
                result2 = false;
                break;
              }
            }
            stack["delete"](array);
            stack["delete"](other);
            return result2;
          }
          function equalByTag(object, other, tag, bitmask, customizer2, equalFunc, stack) {
            switch (tag) {
              case dataViewTag:
                if (object.byteLength != other.byteLength || object.byteOffset != other.byteOffset) {
                  return false;
                }
                object = object.buffer;
                other = other.buffer;
              case arrayBufferTag:
                if (object.byteLength != other.byteLength || !equalFunc(new Uint8Array2(object), new Uint8Array2(other))) {
                  return false;
                }
                return true;
              case boolTag:
              case dateTag:
              case numberTag:
                return eq(+object, +other);
              case errorTag:
                return object.name == other.name && object.message == other.message;
              case regexpTag:
              case stringTag:
                return object == other + "";
              case mapTag:
                var convert = mapToArray;
              case setTag:
                var isPartial = bitmask & COMPARE_PARTIAL_FLAG;
                convert || (convert = setToArray);
                if (object.size != other.size && !isPartial) {
                  return false;
                }
                var stacked = stack.get(object);
                if (stacked) {
                  return stacked == other;
                }
                bitmask |= COMPARE_UNORDERED_FLAG;
                stack.set(object, other);
                var result2 = equalArrays(convert(object), convert(other), bitmask, customizer2, equalFunc, stack);
                stack["delete"](object);
                return result2;
              case symbolTag:
                if (symbolValueOf) {
                  return symbolValueOf.call(object) == symbolValueOf.call(other);
                }
            }
            return false;
          }
          function equalObjects(object, other, bitmask, customizer2, equalFunc, stack) {
            var isPartial = bitmask & COMPARE_PARTIAL_FLAG, objProps = getAllKeys(object), objLength = objProps.length, othProps = getAllKeys(other), othLength = othProps.length;
            if (objLength != othLength && !isPartial) {
              return false;
            }
            var index = objLength;
            while (index--) {
              var key = objProps[index];
              if (!(isPartial ? key in other : hasOwnProperty.call(other, key))) {
                return false;
              }
            }
            var objStacked = stack.get(object);
            var othStacked = stack.get(other);
            if (objStacked && othStacked) {
              return objStacked == other && othStacked == object;
            }
            var result2 = true;
            stack.set(object, other);
            stack.set(other, object);
            var skipCtor = isPartial;
            while (++index < objLength) {
              key = objProps[index];
              var objValue = object[key], othValue = other[key];
              if (customizer2) {
                var compared = isPartial ? customizer2(othValue, objValue, key, other, object, stack) : customizer2(objValue, othValue, key, object, other, stack);
              }
              if (!(compared === undefined2 ? objValue === othValue || equalFunc(objValue, othValue, bitmask, customizer2, stack) : compared)) {
                result2 = false;
                break;
              }
              skipCtor || (skipCtor = key == "constructor");
            }
            if (result2 && !skipCtor) {
              var objCtor = object.constructor, othCtor = other.constructor;
              if (objCtor != othCtor && ("constructor" in object && "constructor" in other) && !(typeof objCtor == "function" && objCtor instanceof objCtor && typeof othCtor == "function" && othCtor instanceof othCtor)) {
                result2 = false;
              }
            }
            stack["delete"](object);
            stack["delete"](other);
            return result2;
          }
          function flatRest(func) {
            return setToString(overRest(func, undefined2, flatten), func + "");
          }
          function getAllKeys(object) {
            return baseGetAllKeys(object, keys, getSymbols);
          }
          function getAllKeysIn(object) {
            return baseGetAllKeys(object, keysIn, getSymbolsIn);
          }
          var getData = !metaMap ? noop : function(func) {
            return metaMap.get(func);
          };
          function getFuncName(func) {
            var result2 = func.name + "", array = realNames[result2], length = hasOwnProperty.call(realNames, result2) ? array.length : 0;
            while (length--) {
              var data = array[length], otherFunc = data.func;
              if (otherFunc == null || otherFunc == func) {
                return data.name;
              }
            }
            return result2;
          }
          function getHolder(func) {
            var object = hasOwnProperty.call(lodash, "placeholder") ? lodash : func;
            return object.placeholder;
          }
          function getIteratee() {
            var result2 = lodash.iteratee || iteratee;
            result2 = result2 === iteratee ? baseIteratee : result2;
            return arguments.length ? result2(arguments[0], arguments[1]) : result2;
          }
          function getMapData(map2, key) {
            var data = map2.__data__;
            return isKeyable(key) ? data[typeof key == "string" ? "string" : "hash"] : data.map;
          }
          function getMatchData(object) {
            var result2 = keys(object), length = result2.length;
            while (length--) {
              var key = result2[length], value = object[key];
              result2[length] = [key, value, isStrictComparable(value)];
            }
            return result2;
          }
          function getNative(object, key) {
            var value = getValue(object, key);
            return baseIsNative(value) ? value : undefined2;
          }
          function getRawTag(value) {
            var isOwn = hasOwnProperty.call(value, symToStringTag), tag = value[symToStringTag];
            try {
              value[symToStringTag] = undefined2;
              var unmasked = true;
            } catch (e) {
            }
            var result2 = nativeObjectToString.call(value);
            if (unmasked) {
              if (isOwn) {
                value[symToStringTag] = tag;
              } else {
                delete value[symToStringTag];
              }
            }
            return result2;
          }
          var getSymbols = !nativeGetSymbols ? stubArray : function(object) {
            if (object == null) {
              return [];
            }
            object = Object2(object);
            return arrayFilter(nativeGetSymbols(object), function(symbol) {
              return propertyIsEnumerable.call(object, symbol);
            });
          };
          var getSymbolsIn = !nativeGetSymbols ? stubArray : function(object) {
            var result2 = [];
            while (object) {
              arrayPush(result2, getSymbols(object));
              object = getPrototype(object);
            }
            return result2;
          };
          var getTag = baseGetTag;
          if (DataView && getTag(new DataView(new ArrayBuffer(1))) != dataViewTag || Map2 && getTag(new Map2()) != mapTag || Promise2 && getTag(Promise2.resolve()) != promiseTag || Set2 && getTag(new Set2()) != setTag || WeakMap2 && getTag(new WeakMap2()) != weakMapTag) {
            getTag = function(value) {
              var result2 = baseGetTag(value), Ctor = result2 == objectTag ? value.constructor : undefined2, ctorString = Ctor ? toSource(Ctor) : "";
              if (ctorString) {
                switch (ctorString) {
                  case dataViewCtorString:
                    return dataViewTag;
                  case mapCtorString:
                    return mapTag;
                  case promiseCtorString:
                    return promiseTag;
                  case setCtorString:
                    return setTag;
                  case weakMapCtorString:
                    return weakMapTag;
                }
              }
              return result2;
            };
          }
          function getView(start, end, transforms) {
            var index = -1, length = transforms.length;
            while (++index < length) {
              var data = transforms[index], size2 = data.size;
              switch (data.type) {
                case "drop":
                  start += size2;
                  break;
                case "dropRight":
                  end -= size2;
                  break;
                case "take":
                  end = nativeMin(end, start + size2);
                  break;
                case "takeRight":
                  start = nativeMax(start, end - size2);
                  break;
              }
            }
            return { "start": start, "end": end };
          }
          function getWrapDetails(source) {
            var match = source.match(reWrapDetails);
            return match ? match[1].split(reSplitDetails) : [];
          }
          function hasPath(object, path, hasFunc) {
            path = castPath(path, object);
            var index = -1, length = path.length, result2 = false;
            while (++index < length) {
              var key = toKey(path[index]);
              if (!(result2 = object != null && hasFunc(object, key))) {
                break;
              }
              object = object[key];
            }
            if (result2 || ++index != length) {
              return result2;
            }
            length = object == null ? 0 : object.length;
            return !!length && isLength(length) && isIndex(key, length) && (isArray(object) || isArguments(object));
          }
          function initCloneArray(array) {
            var length = array.length, result2 = new array.constructor(length);
            if (length && typeof array[0] == "string" && hasOwnProperty.call(array, "index")) {
              result2.index = array.index;
              result2.input = array.input;
            }
            return result2;
          }
          function initCloneObject(object) {
            return typeof object.constructor == "function" && !isPrototype(object) ? baseCreate(getPrototype(object)) : {};
          }
          function initCloneByTag(object, tag, isDeep) {
            var Ctor = object.constructor;
            switch (tag) {
              case arrayBufferTag:
                return cloneArrayBuffer(object);
              case boolTag:
              case dateTag:
                return new Ctor(+object);
              case dataViewTag:
                return cloneDataView(object, isDeep);
              case float32Tag:
              case float64Tag:
              case int8Tag:
              case int16Tag:
              case int32Tag:
              case uint8Tag:
              case uint8ClampedTag:
              case uint16Tag:
              case uint32Tag:
                return cloneTypedArray(object, isDeep);
              case mapTag:
                return new Ctor();
              case numberTag:
              case stringTag:
                return new Ctor(object);
              case regexpTag:
                return cloneRegExp(object);
              case setTag:
                return new Ctor();
              case symbolTag:
                return cloneSymbol(object);
            }
          }
          function insertWrapDetails(source, details) {
            var length = details.length;
            if (!length) {
              return source;
            }
            var lastIndex = length - 1;
            details[lastIndex] = (length > 1 ? "& " : "") + details[lastIndex];
            details = details.join(length > 2 ? ", " : " ");
            return source.replace(reWrapComment, "{\n/* [wrapped with " + details + "] */\n");
          }
          function isFlattenable(value) {
            return isArray(value) || isArguments(value) || !!(spreadableSymbol && value && value[spreadableSymbol]);
          }
          function isIndex(value, length) {
            var type = typeof value;
            length = length == null ? MAX_SAFE_INTEGER : length;
            return !!length && (type == "number" || type != "symbol" && reIsUint.test(value)) && (value > -1 && value % 1 == 0 && value < length);
          }
          function isIterateeCall(value, index, object) {
            if (!isObject(object)) {
              return false;
            }
            var type = typeof index;
            if (type == "number" ? isArrayLike(object) && isIndex(index, object.length) : type == "string" && index in object) {
              return eq(object[index], value);
            }
            return false;
          }
          function isKey(value, object) {
            if (isArray(value)) {
              return false;
            }
            var type = typeof value;
            if (type == "number" || type == "symbol" || type == "boolean" || value == null || isSymbol(value)) {
              return true;
            }
            return reIsPlainProp.test(value) || !reIsDeepProp.test(value) || object != null && value in Object2(object);
          }
          function isKeyable(value) {
            var type = typeof value;
            return type == "string" || type == "number" || type == "symbol" || type == "boolean" ? value !== "__proto__" : value === null;
          }
          function isLaziable(func) {
            var funcName = getFuncName(func), other = lodash[funcName];
            if (typeof other != "function" || !(funcName in LazyWrapper.prototype)) {
              return false;
            }
            if (func === other) {
              return true;
            }
            var data = getData(other);
            return !!data && func === data[0];
          }
          function isMasked(func) {
            return !!maskSrcKey && maskSrcKey in func;
          }
          var isMaskable = coreJsData ? isFunction : stubFalse;
          function isPrototype(value) {
            var Ctor = value && value.constructor, proto = typeof Ctor == "function" && Ctor.prototype || objectProto;
            return value === proto;
          }
          function isStrictComparable(value) {
            return value === value && !isObject(value);
          }
          function matchesStrictComparable(key, srcValue) {
            return function(object) {
              if (object == null) {
                return false;
              }
              return object[key] === srcValue && (srcValue !== undefined2 || key in Object2(object));
            };
          }
          function memoizeCapped(func) {
            var result2 = memoize(func, function(key) {
              if (cache.size === MAX_MEMOIZE_SIZE) {
                cache.clear();
              }
              return key;
            });
            var cache = result2.cache;
            return result2;
          }
          function mergeData(data, source) {
            var bitmask = data[1], srcBitmask = source[1], newBitmask = bitmask | srcBitmask, isCommon = newBitmask < (WRAP_BIND_FLAG | WRAP_BIND_KEY_FLAG | WRAP_ARY_FLAG);
            var isCombo = srcBitmask == WRAP_ARY_FLAG && bitmask == WRAP_CURRY_FLAG || srcBitmask == WRAP_ARY_FLAG && bitmask == WRAP_REARG_FLAG && data[7].length <= source[8] || srcBitmask == (WRAP_ARY_FLAG | WRAP_REARG_FLAG) && source[7].length <= source[8] && bitmask == WRAP_CURRY_FLAG;
            if (!(isCommon || isCombo)) {
              return data;
            }
            if (srcBitmask & WRAP_BIND_FLAG) {
              data[2] = source[2];
              newBitmask |= bitmask & WRAP_BIND_FLAG ? 0 : WRAP_CURRY_BOUND_FLAG;
            }
            var value = source[3];
            if (value) {
              var partials = data[3];
              data[3] = partials ? composeArgs(partials, value, source[4]) : value;
              data[4] = partials ? replaceHolders(data[3], PLACEHOLDER) : source[4];
            }
            value = source[5];
            if (value) {
              partials = data[5];
              data[5] = partials ? composeArgsRight(partials, value, source[6]) : value;
              data[6] = partials ? replaceHolders(data[5], PLACEHOLDER) : source[6];
            }
            value = source[7];
            if (value) {
              data[7] = value;
            }
            if (srcBitmask & WRAP_ARY_FLAG) {
              data[8] = data[8] == null ? source[8] : nativeMin(data[8], source[8]);
            }
            if (data[9] == null) {
              data[9] = source[9];
            }
            data[0] = source[0];
            data[1] = newBitmask;
            return data;
          }
          function nativeKeysIn(object) {
            var result2 = [];
            if (object != null) {
              for (var key in Object2(object)) {
                result2.push(key);
              }
            }
            return result2;
          }
          function objectToString(value) {
            return nativeObjectToString.call(value);
          }
          function overRest(func, start, transform2) {
            start = nativeMax(start === undefined2 ? func.length - 1 : start, 0);
            return function() {
              var args = arguments, index = -1, length = nativeMax(args.length - start, 0), array = Array2(length);
              while (++index < length) {
                array[index] = args[start + index];
              }
              index = -1;
              var otherArgs = Array2(start + 1);
              while (++index < start) {
                otherArgs[index] = args[index];
              }
              otherArgs[start] = transform2(array);
              return apply(func, this, otherArgs);
            };
          }
          function parent(object, path) {
            return path.length < 2 ? object : baseGet(object, baseSlice(path, 0, -1));
          }
          function reorder(array, indexes) {
            var arrLength = array.length, length = nativeMin(indexes.length, arrLength), oldArray = copyArray(array);
            while (length--) {
              var index = indexes[length];
              array[length] = isIndex(index, arrLength) ? oldArray[index] : undefined2;
            }
            return array;
          }
          function safeGet(object, key) {
            if (key === "constructor" && typeof object[key] === "function") {
              return;
            }
            if (key == "__proto__") {
              return;
            }
            return object[key];
          }
          var setData = shortOut(baseSetData);
          var setTimeout = ctxSetTimeout || function(func, wait) {
            return root.setTimeout(func, wait);
          };
          var setToString = shortOut(baseSetToString);
          function setWrapToString(wrapper, reference, bitmask) {
            var source = reference + "";
            return setToString(wrapper, insertWrapDetails(source, updateWrapDetails(getWrapDetails(source), bitmask)));
          }
          function shortOut(func) {
            var count = 0, lastCalled = 0;
            return function() {
              var stamp = nativeNow(), remaining = HOT_SPAN - (stamp - lastCalled);
              lastCalled = stamp;
              if (remaining > 0) {
                if (++count >= HOT_COUNT) {
                  return arguments[0];
                }
              } else {
                count = 0;
              }
              return func.apply(undefined2, arguments);
            };
          }
          function shuffleSelf(array, size2) {
            var index = -1, length = array.length, lastIndex = length - 1;
            size2 = size2 === undefined2 ? length : size2;
            while (++index < size2) {
              var rand = baseRandom(index, lastIndex), value = array[rand];
              array[rand] = array[index];
              array[index] = value;
            }
            array.length = size2;
            return array;
          }
          var stringToPath = memoizeCapped(function(string) {
            var result2 = [];
            if (string.charCodeAt(0) === 46) {
              result2.push("");
            }
            string.replace(rePropName, function(match, number, quote, subString) {
              result2.push(quote ? subString.replace(reEscapeChar, "$1") : number || match);
            });
            return result2;
          });
          function toKey(value) {
            if (typeof value == "string" || isSymbol(value)) {
              return value;
            }
            var result2 = value + "";
            return result2 == "0" && 1 / value == -INFINITY ? "-0" : result2;
          }
          function toSource(func) {
            if (func != null) {
              try {
                return funcToString.call(func);
              } catch (e) {
              }
              try {
                return func + "";
              } catch (e) {
              }
            }
            return "";
          }
          function updateWrapDetails(details, bitmask) {
            arrayEach(wrapFlags, function(pair) {
              var value = "_." + pair[0];
              if (bitmask & pair[1] && !arrayIncludes(details, value)) {
                details.push(value);
              }
            });
            return details.sort();
          }
          function wrapperClone(wrapper) {
            if (wrapper instanceof LazyWrapper) {
              return wrapper.clone();
            }
            var result2 = new LodashWrapper(wrapper.__wrapped__, wrapper.__chain__);
            result2.__actions__ = copyArray(wrapper.__actions__);
            result2.__index__ = wrapper.__index__;
            result2.__values__ = wrapper.__values__;
            return result2;
          }
          function chunk(array, size2, guard) {
            if (guard ? isIterateeCall(array, size2, guard) : size2 === undefined2) {
              size2 = 1;
            } else {
              size2 = nativeMax(toInteger(size2), 0);
            }
            var length = array == null ? 0 : array.length;
            if (!length || size2 < 1) {
              return [];
            }
            var index = 0, resIndex = 0, result2 = Array2(nativeCeil(length / size2));
            while (index < length) {
              result2[resIndex++] = baseSlice(array, index, index += size2);
            }
            return result2;
          }
          function compact2(array) {
            var index = -1, length = array == null ? 0 : array.length, resIndex = 0, result2 = [];
            while (++index < length) {
              var value = array[index];
              if (value) {
                result2[resIndex++] = value;
              }
            }
            return result2;
          }
          function concat() {
            var length = arguments.length;
            if (!length) {
              return [];
            }
            var args = Array2(length - 1), array = arguments[0], index = length;
            while (index--) {
              args[index - 1] = arguments[index];
            }
            return arrayPush(isArray(array) ? copyArray(array) : [array], baseFlatten(args, 1));
          }
          var difference = baseRest(function(array, values3) {
            return isArrayLikeObject(array) ? baseDifference(array, baseFlatten(values3, 1, isArrayLikeObject, true)) : [];
          });
          var differenceBy = baseRest(function(array, values3) {
            var iteratee2 = last(values3);
            if (isArrayLikeObject(iteratee2)) {
              iteratee2 = undefined2;
            }
            return isArrayLikeObject(array) ? baseDifference(array, baseFlatten(values3, 1, isArrayLikeObject, true), getIteratee(iteratee2, 2)) : [];
          });
          var differenceWith = baseRest(function(array, values3) {
            var comparator = last(values3);
            if (isArrayLikeObject(comparator)) {
              comparator = undefined2;
            }
            return isArrayLikeObject(array) ? baseDifference(array, baseFlatten(values3, 1, isArrayLikeObject, true), undefined2, comparator) : [];
          });
          function drop(array, n, guard) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            n = guard || n === undefined2 ? 1 : toInteger(n);
            return baseSlice(array, n < 0 ? 0 : n, length);
          }
          function dropRight(array, n, guard) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            n = guard || n === undefined2 ? 1 : toInteger(n);
            n = length - n;
            return baseSlice(array, 0, n < 0 ? 0 : n);
          }
          function dropRightWhile(array, predicate) {
            return array && array.length ? baseWhile(array, getIteratee(predicate, 3), true, true) : [];
          }
          function dropWhile(array, predicate) {
            return array && array.length ? baseWhile(array, getIteratee(predicate, 3), true) : [];
          }
          function fill(array, value, start, end) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            if (start && typeof start != "number" && isIterateeCall(array, value, start)) {
              start = 0;
              end = length;
            }
            return baseFill(array, value, start, end);
          }
          function findIndex(array, predicate, fromIndex) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return -1;
            }
            var index = fromIndex == null ? 0 : toInteger(fromIndex);
            if (index < 0) {
              index = nativeMax(length + index, 0);
            }
            return baseFindIndex(array, getIteratee(predicate, 3), index);
          }
          function findLastIndex(array, predicate, fromIndex) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return -1;
            }
            var index = length - 1;
            if (fromIndex !== undefined2) {
              index = toInteger(fromIndex);
              index = fromIndex < 0 ? nativeMax(length + index, 0) : nativeMin(index, length - 1);
            }
            return baseFindIndex(array, getIteratee(predicate, 3), index, true);
          }
          function flatten(array) {
            var length = array == null ? 0 : array.length;
            return length ? baseFlatten(array, 1) : [];
          }
          function flattenDeep(array) {
            var length = array == null ? 0 : array.length;
            return length ? baseFlatten(array, INFINITY) : [];
          }
          function flattenDepth(array, depth) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            depth = depth === undefined2 ? 1 : toInteger(depth);
            return baseFlatten(array, depth);
          }
          function fromPairs(pairs) {
            var index = -1, length = pairs == null ? 0 : pairs.length, result2 = {};
            while (++index < length) {
              var pair = pairs[index];
              baseAssignValue(result2, pair[0], pair[1]);
            }
            return result2;
          }
          function head(array) {
            return array && array.length ? array[0] : undefined2;
          }
          function indexOf(array, value, fromIndex) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return -1;
            }
            var index = fromIndex == null ? 0 : toInteger(fromIndex);
            if (index < 0) {
              index = nativeMax(length + index, 0);
            }
            return baseIndexOf(array, value, index);
          }
          function initial(array) {
            var length = array == null ? 0 : array.length;
            return length ? baseSlice(array, 0, -1) : [];
          }
          var intersection = baseRest(function(arrays) {
            var mapped = arrayMap(arrays, castArrayLikeObject);
            return mapped.length && mapped[0] === arrays[0] ? baseIntersection(mapped) : [];
          });
          var intersectionBy = baseRest(function(arrays) {
            var iteratee2 = last(arrays), mapped = arrayMap(arrays, castArrayLikeObject);
            if (iteratee2 === last(mapped)) {
              iteratee2 = undefined2;
            } else {
              mapped.pop();
            }
            return mapped.length && mapped[0] === arrays[0] ? baseIntersection(mapped, getIteratee(iteratee2, 2)) : [];
          });
          var intersectionWith = baseRest(function(arrays) {
            var comparator = last(arrays), mapped = arrayMap(arrays, castArrayLikeObject);
            comparator = typeof comparator == "function" ? comparator : undefined2;
            if (comparator) {
              mapped.pop();
            }
            return mapped.length && mapped[0] === arrays[0] ? baseIntersection(mapped, undefined2, comparator) : [];
          });
          function join(array, separator) {
            return array == null ? "" : nativeJoin.call(array, separator);
          }
          function last(array) {
            var length = array == null ? 0 : array.length;
            return length ? array[length - 1] : undefined2;
          }
          function lastIndexOf(array, value, fromIndex) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return -1;
            }
            var index = length;
            if (fromIndex !== undefined2) {
              index = toInteger(fromIndex);
              index = index < 0 ? nativeMax(length + index, 0) : nativeMin(index, length - 1);
            }
            return value === value ? strictLastIndexOf(array, value, index) : baseFindIndex(array, baseIsNaN, index, true);
          }
          function nth(array, n) {
            return array && array.length ? baseNth(array, toInteger(n)) : undefined2;
          }
          var pull = baseRest(pullAll);
          function pullAll(array, values3) {
            return array && array.length && values3 && values3.length ? basePullAll(array, values3) : array;
          }
          function pullAllBy(array, values3, iteratee2) {
            return array && array.length && values3 && values3.length ? basePullAll(array, values3, getIteratee(iteratee2, 2)) : array;
          }
          function pullAllWith(array, values3, comparator) {
            return array && array.length && values3 && values3.length ? basePullAll(array, values3, undefined2, comparator) : array;
          }
          var pullAt = flatRest(function(array, indexes) {
            var length = array == null ? 0 : array.length, result2 = baseAt(array, indexes);
            basePullAt(array, arrayMap(indexes, function(index) {
              return isIndex(index, length) ? +index : index;
            }).sort(compareAscending));
            return result2;
          });
          function remove(array, predicate) {
            var result2 = [];
            if (!(array && array.length)) {
              return result2;
            }
            var index = -1, indexes = [], length = array.length;
            predicate = getIteratee(predicate, 3);
            while (++index < length) {
              var value = array[index];
              if (predicate(value, index, array)) {
                result2.push(value);
                indexes.push(index);
              }
            }
            basePullAt(array, indexes);
            return result2;
          }
          function reverse(array) {
            return array == null ? array : nativeReverse.call(array);
          }
          function slice(array, start, end) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            if (end && typeof end != "number" && isIterateeCall(array, start, end)) {
              start = 0;
              end = length;
            } else {
              start = start == null ? 0 : toInteger(start);
              end = end === undefined2 ? length : toInteger(end);
            }
            return baseSlice(array, start, end);
          }
          function sortedIndex(array, value) {
            return baseSortedIndex(array, value);
          }
          function sortedIndexBy(array, value, iteratee2) {
            return baseSortedIndexBy(array, value, getIteratee(iteratee2, 2));
          }
          function sortedIndexOf(array, value) {
            var length = array == null ? 0 : array.length;
            if (length) {
              var index = baseSortedIndex(array, value);
              if (index < length && eq(array[index], value)) {
                return index;
              }
            }
            return -1;
          }
          function sortedLastIndex(array, value) {
            return baseSortedIndex(array, value, true);
          }
          function sortedLastIndexBy(array, value, iteratee2) {
            return baseSortedIndexBy(array, value, getIteratee(iteratee2, 2), true);
          }
          function sortedLastIndexOf(array, value) {
            var length = array == null ? 0 : array.length;
            if (length) {
              var index = baseSortedIndex(array, value, true) - 1;
              if (eq(array[index], value)) {
                return index;
              }
            }
            return -1;
          }
          function sortedUniq(array) {
            return array && array.length ? baseSortedUniq(array) : [];
          }
          function sortedUniqBy(array, iteratee2) {
            return array && array.length ? baseSortedUniq(array, getIteratee(iteratee2, 2)) : [];
          }
          function tail(array) {
            var length = array == null ? 0 : array.length;
            return length ? baseSlice(array, 1, length) : [];
          }
          function take(array, n, guard) {
            if (!(array && array.length)) {
              return [];
            }
            n = guard || n === undefined2 ? 1 : toInteger(n);
            return baseSlice(array, 0, n < 0 ? 0 : n);
          }
          function takeRight(array, n, guard) {
            var length = array == null ? 0 : array.length;
            if (!length) {
              return [];
            }
            n = guard || n === undefined2 ? 1 : toInteger(n);
            n = length - n;
            return baseSlice(array, n < 0 ? 0 : n, length);
          }
          function takeRightWhile(array, predicate) {
            return array && array.length ? baseWhile(array, getIteratee(predicate, 3), false, true) : [];
          }
          function takeWhile(array, predicate) {
            return array && array.length ? baseWhile(array, getIteratee(predicate, 3)) : [];
          }
          var union = baseRest(function(arrays) {
            return baseUniq(baseFlatten(arrays, 1, isArrayLikeObject, true));
          });
          var unionBy = baseRest(function(arrays) {
            var iteratee2 = last(arrays);
            if (isArrayLikeObject(iteratee2)) {
              iteratee2 = undefined2;
            }
            return baseUniq(baseFlatten(arrays, 1, isArrayLikeObject, true), getIteratee(iteratee2, 2));
          });
          var unionWith = baseRest(function(arrays) {
            var comparator = last(arrays);
            comparator = typeof comparator == "function" ? comparator : undefined2;
            return baseUniq(baseFlatten(arrays, 1, isArrayLikeObject, true), undefined2, comparator);
          });
          function uniq(array) {
            return array && array.length ? baseUniq(array) : [];
          }
          function uniqBy(array, iteratee2) {
            return array && array.length ? baseUniq(array, getIteratee(iteratee2, 2)) : [];
          }
          function uniqWith(array, comparator) {
            comparator = typeof comparator == "function" ? comparator : undefined2;
            return array && array.length ? baseUniq(array, undefined2, comparator) : [];
          }
          function unzip(array) {
            if (!(array && array.length)) {
              return [];
            }
            var length = 0;
            array = arrayFilter(array, function(group) {
              if (isArrayLikeObject(group)) {
                length = nativeMax(group.length, length);
                return true;
              }
            });
            return baseTimes(length, function(index) {
              return arrayMap(array, baseProperty(index));
            });
          }
          function unzipWith(array, iteratee2) {
            if (!(array && array.length)) {
              return [];
            }
            var result2 = unzip(array);
            if (iteratee2 == null) {
              return result2;
            }
            return arrayMap(result2, function(group) {
              return apply(iteratee2, undefined2, group);
            });
          }
          var without = baseRest(function(array, values3) {
            return isArrayLikeObject(array) ? baseDifference(array, values3) : [];
          });
          var xor = baseRest(function(arrays) {
            return baseXor(arrayFilter(arrays, isArrayLikeObject));
          });
          var xorBy = baseRest(function(arrays) {
            var iteratee2 = last(arrays);
            if (isArrayLikeObject(iteratee2)) {
              iteratee2 = undefined2;
            }
            return baseXor(arrayFilter(arrays, isArrayLikeObject), getIteratee(iteratee2, 2));
          });
          var xorWith = baseRest(function(arrays) {
            var comparator = last(arrays);
            comparator = typeof comparator == "function" ? comparator : undefined2;
            return baseXor(arrayFilter(arrays, isArrayLikeObject), undefined2, comparator);
          });
          var zip = baseRest(unzip);
          function zipObject(props, values3) {
            return baseZipObject(props || [], values3 || [], assignValue);
          }
          function zipObjectDeep(props, values3) {
            return baseZipObject(props || [], values3 || [], baseSet);
          }
          var zipWith = baseRest(function(arrays) {
            var length = arrays.length, iteratee2 = length > 1 ? arrays[length - 1] : undefined2;
            iteratee2 = typeof iteratee2 == "function" ? (arrays.pop(), iteratee2) : undefined2;
            return unzipWith(arrays, iteratee2);
          });
          function chain(value) {
            var result2 = lodash(value);
            result2.__chain__ = true;
            return result2;
          }
          function tap(value, interceptor) {
            interceptor(value);
            return value;
          }
          function thru(value, interceptor) {
            return interceptor(value);
          }
          var wrapperAt = flatRest(function(paths) {
            var length = paths.length, start = length ? paths[0] : 0, value = this.__wrapped__, interceptor = function(object) {
              return baseAt(object, paths);
            };
            if (length > 1 || this.__actions__.length || !(value instanceof LazyWrapper) || !isIndex(start)) {
              return this.thru(interceptor);
            }
            value = value.slice(start, +start + (length ? 1 : 0));
            value.__actions__.push({
              "func": thru,
              "args": [interceptor],
              "thisArg": undefined2
            });
            return new LodashWrapper(value, this.__chain__).thru(function(array) {
              if (length && !array.length) {
                array.push(undefined2);
              }
              return array;
            });
          });
          function wrapperChain() {
            return chain(this);
          }
          function wrapperCommit() {
            return new LodashWrapper(this.value(), this.__chain__);
          }
          function wrapperNext() {
            if (this.__values__ === undefined2) {
              this.__values__ = toArray(this.value());
            }
            var done = this.__index__ >= this.__values__.length, value = done ? undefined2 : this.__values__[this.__index__++];
            return { "done": done, "value": value };
          }
          function wrapperToIterator() {
            return this;
          }
          function wrapperPlant(value) {
            var result2, parent2 = this;
            while (parent2 instanceof baseLodash) {
              var clone2 = wrapperClone(parent2);
              clone2.__index__ = 0;
              clone2.__values__ = undefined2;
              if (result2) {
                previous.__wrapped__ = clone2;
              } else {
                result2 = clone2;
              }
              var previous = clone2;
              parent2 = parent2.__wrapped__;
            }
            previous.__wrapped__ = value;
            return result2;
          }
          function wrapperReverse() {
            var value = this.__wrapped__;
            if (value instanceof LazyWrapper) {
              var wrapped = value;
              if (this.__actions__.length) {
                wrapped = new LazyWrapper(this);
              }
              wrapped = wrapped.reverse();
              wrapped.__actions__.push({
                "func": thru,
                "args": [reverse],
                "thisArg": undefined2
              });
              return new LodashWrapper(wrapped, this.__chain__);
            }
            return this.thru(reverse);
          }
          function wrapperValue() {
            return baseWrapperValue(this.__wrapped__, this.__actions__);
          }
          var countBy = createAggregator(function(result2, value, key) {
            if (hasOwnProperty.call(result2, key)) {
              ++result2[key];
            } else {
              baseAssignValue(result2, key, 1);
            }
          });
          function every(collection, predicate, guard) {
            var func = isArray(collection) ? arrayEvery : baseEvery;
            if (guard && isIterateeCall(collection, predicate, guard)) {
              predicate = undefined2;
            }
            return func(collection, getIteratee(predicate, 3));
          }
          function filter(collection, predicate) {
            var func = isArray(collection) ? arrayFilter : baseFilter;
            return func(collection, getIteratee(predicate, 3));
          }
          var find = createFind(findIndex);
          var findLast = createFind(findLastIndex);
          function flatMap(collection, iteratee2) {
            return baseFlatten(map(collection, iteratee2), 1);
          }
          function flatMapDeep(collection, iteratee2) {
            return baseFlatten(map(collection, iteratee2), INFINITY);
          }
          function flatMapDepth(collection, iteratee2, depth) {
            depth = depth === undefined2 ? 1 : toInteger(depth);
            return baseFlatten(map(collection, iteratee2), depth);
          }
          function forEach(collection, iteratee2) {
            var func = isArray(collection) ? arrayEach : baseEach;
            return func(collection, getIteratee(iteratee2, 3));
          }
          function forEachRight(collection, iteratee2) {
            var func = isArray(collection) ? arrayEachRight : baseEachRight;
            return func(collection, getIteratee(iteratee2, 3));
          }
          var groupBy = createAggregator(function(result2, value, key) {
            if (hasOwnProperty.call(result2, key)) {
              result2[key].push(value);
            } else {
              baseAssignValue(result2, key, [value]);
            }
          });
          function includes(collection, value, fromIndex, guard) {
            collection = isArrayLike(collection) ? collection : values2(collection);
            fromIndex = fromIndex && !guard ? toInteger(fromIndex) : 0;
            var length = collection.length;
            if (fromIndex < 0) {
              fromIndex = nativeMax(length + fromIndex, 0);
            }
            return isString(collection) ? fromIndex <= length && collection.indexOf(value, fromIndex) > -1 : !!length && baseIndexOf(collection, value, fromIndex) > -1;
          }
          var invokeMap = baseRest(function(collection, path, args) {
            var index = -1, isFunc = typeof path == "function", result2 = isArrayLike(collection) ? Array2(collection.length) : [];
            baseEach(collection, function(value) {
              result2[++index] = isFunc ? apply(path, value, args) : baseInvoke(value, path, args);
            });
            return result2;
          });
          var keyBy = createAggregator(function(result2, value, key) {
            baseAssignValue(result2, key, value);
          });
          function map(collection, iteratee2) {
            var func = isArray(collection) ? arrayMap : baseMap;
            return func(collection, getIteratee(iteratee2, 3));
          }
          function orderBy(collection, iteratees, orders, guard) {
            if (collection == null) {
              return [];
            }
            if (!isArray(iteratees)) {
              iteratees = iteratees == null ? [] : [iteratees];
            }
            orders = guard ? undefined2 : orders;
            if (!isArray(orders)) {
              orders = orders == null ? [] : [orders];
            }
            return baseOrderBy(collection, iteratees, orders);
          }
          var partition = createAggregator(function(result2, value, key) {
            result2[key ? 0 : 1].push(value);
          }, function() {
            return [[], []];
          });
          function reduce(collection, iteratee2, accumulator) {
            var func = isArray(collection) ? arrayReduce : baseReduce, initAccum = arguments.length < 3;
            return func(collection, getIteratee(iteratee2, 4), accumulator, initAccum, baseEach);
          }
          function reduceRight(collection, iteratee2, accumulator) {
            var func = isArray(collection) ? arrayReduceRight : baseReduce, initAccum = arguments.length < 3;
            return func(collection, getIteratee(iteratee2, 4), accumulator, initAccum, baseEachRight);
          }
          function reject(collection, predicate) {
            var func = isArray(collection) ? arrayFilter : baseFilter;
            return func(collection, negate(getIteratee(predicate, 3)));
          }
          function sample(collection) {
            var func = isArray(collection) ? arraySample : baseSample;
            return func(collection);
          }
          function sampleSize(collection, n, guard) {
            if (guard ? isIterateeCall(collection, n, guard) : n === undefined2) {
              n = 1;
            } else {
              n = toInteger(n);
            }
            var func = isArray(collection) ? arraySampleSize : baseSampleSize;
            return func(collection, n);
          }
          function shuffle(collection) {
            var func = isArray(collection) ? arrayShuffle : baseShuffle;
            return func(collection);
          }
          function size(collection) {
            if (collection == null) {
              return 0;
            }
            if (isArrayLike(collection)) {
              return isString(collection) ? stringSize(collection) : collection.length;
            }
            var tag = getTag(collection);
            if (tag == mapTag || tag == setTag) {
              return collection.size;
            }
            return baseKeys(collection).length;
          }
          function some(collection, predicate, guard) {
            var func = isArray(collection) ? arraySome : baseSome;
            if (guard && isIterateeCall(collection, predicate, guard)) {
              predicate = undefined2;
            }
            return func(collection, getIteratee(predicate, 3));
          }
          var sortBy = baseRest(function(collection, iteratees) {
            if (collection == null) {
              return [];
            }
            var length = iteratees.length;
            if (length > 1 && isIterateeCall(collection, iteratees[0], iteratees[1])) {
              iteratees = [];
            } else if (length > 2 && isIterateeCall(iteratees[0], iteratees[1], iteratees[2])) {
              iteratees = [iteratees[0]];
            }
            return baseOrderBy(collection, baseFlatten(iteratees, 1), []);
          });
          var now = ctxNow || function() {
            return root.Date.now();
          };
          function after(n, func) {
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            n = toInteger(n);
            return function() {
              if (--n < 1) {
                return func.apply(this, arguments);
              }
            };
          }
          function ary(func, n, guard) {
            n = guard ? undefined2 : n;
            n = func && n == null ? func.length : n;
            return createWrap(func, WRAP_ARY_FLAG, undefined2, undefined2, undefined2, undefined2, n);
          }
          function before(n, func) {
            var result2;
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            n = toInteger(n);
            return function() {
              if (--n > 0) {
                result2 = func.apply(this, arguments);
              }
              if (n <= 1) {
                func = undefined2;
              }
              return result2;
            };
          }
          var bind = baseRest(function(func, thisArg, partials) {
            var bitmask = WRAP_BIND_FLAG;
            if (partials.length) {
              var holders = replaceHolders(partials, getHolder(bind));
              bitmask |= WRAP_PARTIAL_FLAG;
            }
            return createWrap(func, bitmask, thisArg, partials, holders);
          });
          var bindKey = baseRest(function(object, key, partials) {
            var bitmask = WRAP_BIND_FLAG | WRAP_BIND_KEY_FLAG;
            if (partials.length) {
              var holders = replaceHolders(partials, getHolder(bindKey));
              bitmask |= WRAP_PARTIAL_FLAG;
            }
            return createWrap(key, bitmask, object, partials, holders);
          });
          function curry(func, arity, guard) {
            arity = guard ? undefined2 : arity;
            var result2 = createWrap(func, WRAP_CURRY_FLAG, undefined2, undefined2, undefined2, undefined2, undefined2, arity);
            result2.placeholder = curry.placeholder;
            return result2;
          }
          function curryRight(func, arity, guard) {
            arity = guard ? undefined2 : arity;
            var result2 = createWrap(func, WRAP_CURRY_RIGHT_FLAG, undefined2, undefined2, undefined2, undefined2, undefined2, arity);
            result2.placeholder = curryRight.placeholder;
            return result2;
          }
          function debounce(func, wait, options) {
            var lastArgs, lastThis, maxWait, result2, timerId, lastCallTime, lastInvokeTime = 0, leading = false, maxing = false, trailing = true;
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            wait = toNumber2(wait) || 0;
            if (isObject(options)) {
              leading = !!options.leading;
              maxing = "maxWait" in options;
              maxWait = maxing ? nativeMax(toNumber2(options.maxWait) || 0, wait) : maxWait;
              trailing = "trailing" in options ? !!options.trailing : trailing;
            }
            function invokeFunc(time) {
              var args = lastArgs, thisArg = lastThis;
              lastArgs = lastThis = undefined2;
              lastInvokeTime = time;
              result2 = func.apply(thisArg, args);
              return result2;
            }
            function leadingEdge(time) {
              lastInvokeTime = time;
              timerId = setTimeout(timerExpired, wait);
              return leading ? invokeFunc(time) : result2;
            }
            function remainingWait(time) {
              var timeSinceLastCall = time - lastCallTime, timeSinceLastInvoke = time - lastInvokeTime, timeWaiting = wait - timeSinceLastCall;
              return maxing ? nativeMin(timeWaiting, maxWait - timeSinceLastInvoke) : timeWaiting;
            }
            function shouldInvoke(time) {
              var timeSinceLastCall = time - lastCallTime, timeSinceLastInvoke = time - lastInvokeTime;
              return lastCallTime === undefined2 || timeSinceLastCall >= wait || timeSinceLastCall < 0 || maxing && timeSinceLastInvoke >= maxWait;
            }
            function timerExpired() {
              var time = now();
              if (shouldInvoke(time)) {
                return trailingEdge(time);
              }
              timerId = setTimeout(timerExpired, remainingWait(time));
            }
            function trailingEdge(time) {
              timerId = undefined2;
              if (trailing && lastArgs) {
                return invokeFunc(time);
              }
              lastArgs = lastThis = undefined2;
              return result2;
            }
            function cancel() {
              if (timerId !== undefined2) {
                clearTimeout(timerId);
              }
              lastInvokeTime = 0;
              lastArgs = lastCallTime = lastThis = timerId = undefined2;
            }
            function flush() {
              return timerId === undefined2 ? result2 : trailingEdge(now());
            }
            function debounced() {
              var time = now(), isInvoking = shouldInvoke(time);
              lastArgs = arguments;
              lastThis = this;
              lastCallTime = time;
              if (isInvoking) {
                if (timerId === undefined2) {
                  return leadingEdge(lastCallTime);
                }
                if (maxing) {
                  clearTimeout(timerId);
                  timerId = setTimeout(timerExpired, wait);
                  return invokeFunc(lastCallTime);
                }
              }
              if (timerId === undefined2) {
                timerId = setTimeout(timerExpired, wait);
              }
              return result2;
            }
            debounced.cancel = cancel;
            debounced.flush = flush;
            return debounced;
          }
          var defer = baseRest(function(func, args) {
            return baseDelay(func, 1, args);
          });
          var delay = baseRest(function(func, wait, args) {
            return baseDelay(func, toNumber2(wait) || 0, args);
          });
          function flip(func) {
            return createWrap(func, WRAP_FLIP_FLAG);
          }
          function memoize(func, resolver) {
            if (typeof func != "function" || resolver != null && typeof resolver != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            var memoized = function() {
              var args = arguments, key = resolver ? resolver.apply(this, args) : args[0], cache = memoized.cache;
              if (cache.has(key)) {
                return cache.get(key);
              }
              var result2 = func.apply(this, args);
              memoized.cache = cache.set(key, result2) || cache;
              return result2;
            };
            memoized.cache = new (memoize.Cache || MapCache)();
            return memoized;
          }
          memoize.Cache = MapCache;
          function negate(predicate) {
            if (typeof predicate != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            return function() {
              var args = arguments;
              switch (args.length) {
                case 0:
                  return !predicate.call(this);
                case 1:
                  return !predicate.call(this, args[0]);
                case 2:
                  return !predicate.call(this, args[0], args[1]);
                case 3:
                  return !predicate.call(this, args[0], args[1], args[2]);
              }
              return !predicate.apply(this, args);
            };
          }
          function once(func) {
            return before(2, func);
          }
          var overArgs = castRest(function(func, transforms) {
            transforms = transforms.length == 1 && isArray(transforms[0]) ? arrayMap(transforms[0], baseUnary(getIteratee())) : arrayMap(baseFlatten(transforms, 1), baseUnary(getIteratee()));
            var funcsLength = transforms.length;
            return baseRest(function(args) {
              var index = -1, length = nativeMin(args.length, funcsLength);
              while (++index < length) {
                args[index] = transforms[index].call(this, args[index]);
              }
              return apply(func, this, args);
            });
          });
          var partial = baseRest(function(func, partials) {
            var holders = replaceHolders(partials, getHolder(partial));
            return createWrap(func, WRAP_PARTIAL_FLAG, undefined2, partials, holders);
          });
          var partialRight = baseRest(function(func, partials) {
            var holders = replaceHolders(partials, getHolder(partialRight));
            return createWrap(func, WRAP_PARTIAL_RIGHT_FLAG, undefined2, partials, holders);
          });
          var rearg = flatRest(function(func, indexes) {
            return createWrap(func, WRAP_REARG_FLAG, undefined2, undefined2, undefined2, indexes);
          });
          function rest(func, start) {
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            start = start === undefined2 ? start : toInteger(start);
            return baseRest(func, start);
          }
          function spread(func, start) {
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            start = start == null ? 0 : nativeMax(toInteger(start), 0);
            return baseRest(function(args) {
              var array = args[start], otherArgs = castSlice(args, 0, start);
              if (array) {
                arrayPush(otherArgs, array);
              }
              return apply(func, this, otherArgs);
            });
          }
          function throttle(func, wait, options) {
            var leading = true, trailing = true;
            if (typeof func != "function") {
              throw String(FUNC_ERROR_TEXT);
            }
            if (isObject(options)) {
              leading = "leading" in options ? !!options.leading : leading;
              trailing = "trailing" in options ? !!options.trailing : trailing;
            }
            return debounce(func, wait, {
              "leading": leading,
              "maxWait": wait,
              "trailing": trailing
            });
          }
          function unary(func) {
            return ary(func, 1);
          }
          function wrap(value, wrapper) {
            return partial(castFunction(wrapper), value);
          }
          function castArray() {
            if (!arguments.length) {
              return [];
            }
            var value = arguments[0];
            return isArray(value) ? value : [value];
          }
          function clone(value) {
            return baseClone(value, CLONE_SYMBOLS_FLAG);
          }
          function cloneWith(value, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            return baseClone(value, CLONE_SYMBOLS_FLAG, customizer2);
          }
          function cloneDeep2(value) {
            return baseClone(value, CLONE_DEEP_FLAG | CLONE_SYMBOLS_FLAG);
          }
          function cloneDeepWith2(value, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            return baseClone(value, CLONE_DEEP_FLAG | CLONE_SYMBOLS_FLAG, customizer2);
          }
          function conformsTo(object, source) {
            return source == null || baseConformsTo(object, source, keys(source));
          }
          function eq(value, other) {
            return value === other || value !== value && other !== other;
          }
          var gt = createRelationalOperation(baseGt);
          var gte = createRelationalOperation(function(value, other) {
            return value >= other;
          });
          var isArguments = baseIsArguments(/* @__PURE__ */ (function() {
            return arguments;
          })()) ? baseIsArguments : function(value) {
            return isObjectLike(value) && hasOwnProperty.call(value, "callee") && !propertyIsEnumerable.call(value, "callee");
          };
          var isArray = Array2.isArray;
          var isArrayBuffer = nodeIsArrayBuffer ? baseUnary(nodeIsArrayBuffer) : baseIsArrayBuffer;
          function isArrayLike(value) {
            return value != null && isLength(value.length) && !isFunction(value);
          }
          function isArrayLikeObject(value) {
            return isObjectLike(value) && isArrayLike(value);
          }
          function isBoolean(value) {
            return value === true || value === false || isObjectLike(value) && baseGetTag(value) == boolTag;
          }
          var isBuffer = nativeIsBuffer || stubFalse;
          var isDate = nodeIsDate ? baseUnary(nodeIsDate) : baseIsDate;
          function isElement(value) {
            return isObjectLike(value) && value.nodeType === 1 && !isPlainObject(value);
          }
          function isEmpty(value) {
            if (value == null) {
              return true;
            }
            if (isArrayLike(value) && (isArray(value) || typeof value == "string" || typeof value.splice == "function" || isBuffer(value) || isTypedArray(value) || isArguments(value))) {
              return !value.length;
            }
            var tag = getTag(value);
            if (tag == mapTag || tag == setTag) {
              return !value.size;
            }
            if (isPrototype(value)) {
              return !baseKeys(value).length;
            }
            for (var key in value) {
              if (hasOwnProperty.call(value, key)) {
                return false;
              }
            }
            return true;
          }
          function isEqual(value, other) {
            return baseIsEqual(value, other);
          }
          function isEqualWith(value, other, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            var result2 = customizer2 ? customizer2(value, other) : undefined2;
            return result2 === undefined2 ? baseIsEqual(value, other, undefined2, customizer2) : !!result2;
          }
          function isError(value) {
            if (!isObjectLike(value)) {
              return false;
            }
            var tag = baseGetTag(value);
            return tag == errorTag || tag == domExcTag || typeof value.message == "string" && typeof value.name == "string" && !isPlainObject(value);
          }
          function isFinite2(value) {
            return typeof value == "number" && nativeIsFinite(value);
          }
          function isFunction(value) {
            if (!isObject(value)) {
              return false;
            }
            var tag = baseGetTag(value);
            return tag == funcTag || tag == genTag || tag == asyncTag || tag == proxyTag;
          }
          function isInteger(value) {
            return typeof value == "number" && value == toInteger(value);
          }
          function isLength(value) {
            return typeof value == "number" && value > -1 && value % 1 == 0 && value <= MAX_SAFE_INTEGER;
          }
          function isObject(value) {
            var type = typeof value;
            return value != null && (type == "object" || type == "function");
          }
          function isObjectLike(value) {
            return value != null && typeof value == "object";
          }
          var isMap = nodeIsMap ? baseUnary(nodeIsMap) : baseIsMap;
          function isMatch(object, source) {
            return object === source || baseIsMatch(object, source, getMatchData(source));
          }
          function isMatchWith(object, source, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            return baseIsMatch(object, source, getMatchData(source), customizer2);
          }
          function isNaN2(value) {
            return isNumber11(value) && value != +value;
          }
          function isNative(value) {
            if (isMaskable(value)) {
              throw String(CORE_ERROR_TEXT);
            }
            return baseIsNative(value);
          }
          function isNull(value) {
            return value === null;
          }
          function isNil(value) {
            return value == null;
          }
          function isNumber11(value) {
            return typeof value == "number" || isObjectLike(value) && baseGetTag(value) == numberTag;
          }
          function isPlainObject(value) {
            if (!isObjectLike(value) || baseGetTag(value) != objectTag) {
              return false;
            }
            var proto = getPrototype(value);
            if (proto === null) {
              return true;
            }
            var Ctor = hasOwnProperty.call(proto, "constructor") && proto.constructor;
            return typeof Ctor == "function" && Ctor instanceof Ctor && funcToString.call(Ctor) == objectCtorString;
          }
          var isRegExp = nodeIsRegExp ? baseUnary(nodeIsRegExp) : baseIsRegExp;
          function isSafeInteger(value) {
            return isInteger(value) && value >= -MAX_SAFE_INTEGER && value <= MAX_SAFE_INTEGER;
          }
          var isSet = nodeIsSet ? baseUnary(nodeIsSet) : baseIsSet;
          function isString(value) {
            return typeof value == "string" || !isArray(value) && isObjectLike(value) && baseGetTag(value) == stringTag;
          }
          function isSymbol(value) {
            return typeof value == "symbol" || isObjectLike(value) && baseGetTag(value) == symbolTag;
          }
          var isTypedArray = nodeIsTypedArray ? baseUnary(nodeIsTypedArray) : baseIsTypedArray;
          function isUndefined(value) {
            return value === undefined2;
          }
          function isWeakMap(value) {
            return isObjectLike(value) && getTag(value) == weakMapTag;
          }
          function isWeakSet(value) {
            return isObjectLike(value) && baseGetTag(value) == weakSetTag;
          }
          var lt = createRelationalOperation(baseLt);
          var lte = createRelationalOperation(function(value, other) {
            return value <= other;
          });
          function toArray(value) {
            if (!value) {
              return [];
            }
            if (isArrayLike(value)) {
              return isString(value) ? stringToArray(value) : copyArray(value);
            }
            if (symIterator && value[symIterator]) {
              return iteratorToArray(value[symIterator]());
            }
            var tag = getTag(value), func = tag == mapTag ? mapToArray : tag == setTag ? setToArray : values2;
            return func(value);
          }
          function toFinite(value) {
            if (!value) {
              return value === 0 ? value : 0;
            }
            value = toNumber2(value);
            if (value === INFINITY || value === -INFINITY) {
              var sign = value < 0 ? -1 : 1;
              return sign * MAX_INTEGER;
            }
            return value === value ? value : 0;
          }
          function toInteger(value) {
            var result2 = toFinite(value), remainder = result2 % 1;
            return result2 === result2 ? remainder ? result2 - remainder : result2 : 0;
          }
          function toLength(value) {
            return value ? baseClamp(toInteger(value), 0, MAX_ARRAY_LENGTH) : 0;
          }
          function toNumber2(value) {
            if (typeof value == "number") {
              return value;
            }
            if (isSymbol(value)) {
              return NAN;
            }
            if (isObject(value)) {
              var other = typeof value.valueOf == "function" ? value.valueOf() : value;
              value = isObject(other) ? other + "" : other;
            }
            if (typeof value != "string") {
              return value === 0 ? value : +value;
            }
            value = baseTrim(value);
            var isBinary = reIsBinary.test(value);
            return isBinary || reIsOctal.test(value) ? freeParseInt(value.slice(2), isBinary ? 2 : 8) : reIsBadHex.test(value) ? NAN : +value;
          }
          function toPlainObject(value) {
            return copyObject(value, keysIn(value));
          }
          function toSafeInteger(value) {
            return value ? baseClamp(toInteger(value), -MAX_SAFE_INTEGER, MAX_SAFE_INTEGER) : value === 0 ? value : 0;
          }
          function toString(value) {
            return value == null ? "" : baseToString(value);
          }
          var assign = createAssigner(function(object, source) {
            if (isPrototype(source) || isArrayLike(source)) {
              copyObject(source, keys(source), object);
              return;
            }
            for (var key in source) {
              if (hasOwnProperty.call(source, key)) {
                assignValue(object, key, source[key]);
              }
            }
          });
          var assignIn = createAssigner(function(object, source) {
            copyObject(source, keysIn(source), object);
          });
          var assignInWith = createAssigner(function(object, source, srcIndex, customizer2) {
            copyObject(source, keysIn(source), object, customizer2);
          });
          var assignWith = createAssigner(function(object, source, srcIndex, customizer2) {
            copyObject(source, keys(source), object, customizer2);
          });
          var at = flatRest(baseAt);
          function create(prototype, properties) {
            var result2 = baseCreate(prototype);
            return properties == null ? result2 : baseAssign(result2, properties);
          }
          var defaults = baseRest(function(object, sources) {
            object = Object2(object);
            var index = -1;
            var length = sources.length;
            var guard = length > 2 ? sources[2] : undefined2;
            if (guard && isIterateeCall(sources[0], sources[1], guard)) {
              length = 1;
            }
            while (++index < length) {
              var source = sources[index];
              var props = keysIn(source);
              var propsIndex = -1;
              var propsLength = props.length;
              while (++propsIndex < propsLength) {
                var key = props[propsIndex];
                var value = object[key];
                if (value === undefined2 || eq(value, objectProto[key]) && !hasOwnProperty.call(object, key)) {
                  object[key] = source[key];
                }
              }
            }
            return object;
          });
          var defaultsDeep = baseRest(function(args) {
            args.push(undefined2, customDefaultsMerge);
            return apply(mergeWith, undefined2, args);
          });
          function findKey(object, predicate) {
            return baseFindKey(object, getIteratee(predicate, 3), baseForOwn);
          }
          function findLastKey(object, predicate) {
            return baseFindKey(object, getIteratee(predicate, 3), baseForOwnRight);
          }
          function forIn(object, iteratee2) {
            return object == null ? object : baseFor(object, getIteratee(iteratee2, 3), keysIn);
          }
          function forInRight(object, iteratee2) {
            return object == null ? object : baseForRight(object, getIteratee(iteratee2, 3), keysIn);
          }
          function forOwn(object, iteratee2) {
            return object && baseForOwn(object, getIteratee(iteratee2, 3));
          }
          function forOwnRight(object, iteratee2) {
            return object && baseForOwnRight(object, getIteratee(iteratee2, 3));
          }
          function functions(object) {
            return object == null ? [] : baseFunctions(object, keys(object));
          }
          function functionsIn(object) {
            return object == null ? [] : baseFunctions(object, keysIn(object));
          }
          function get(object, path, defaultValue) {
            var result2 = object == null ? undefined2 : baseGet(object, path);
            return result2 === undefined2 ? defaultValue : result2;
          }
          function has(object, path) {
            return object != null && hasPath(object, path, baseHas);
          }
          function hasIn(object, path) {
            return object != null && hasPath(object, path, baseHasIn);
          }
          var invert = createInverter(function(result2, value, key) {
            if (value != null && typeof value.toString != "function") {
              value = nativeObjectToString.call(value);
            }
            result2[value] = key;
          }, constant(identity));
          var invertBy = createInverter(function(result2, value, key) {
            if (value != null && typeof value.toString != "function") {
              value = nativeObjectToString.call(value);
            }
            if (hasOwnProperty.call(result2, value)) {
              result2[value].push(key);
            } else {
              result2[value] = [key];
            }
          }, getIteratee);
          var invoke = baseRest(baseInvoke);
          function keys(object) {
            return isArrayLike(object) ? arrayLikeKeys(object) : baseKeys(object);
          }
          function keysIn(object) {
            return isArrayLike(object) ? arrayLikeKeys(object, true) : baseKeysIn(object);
          }
          function mapKeys(object, iteratee2) {
            var result2 = {};
            iteratee2 = getIteratee(iteratee2, 3);
            baseForOwn(object, function(value, key, object2) {
              baseAssignValue(result2, iteratee2(value, key, object2), value);
            });
            return result2;
          }
          function mapValues(object, iteratee2) {
            var result2 = {};
            iteratee2 = getIteratee(iteratee2, 3);
            baseForOwn(object, function(value, key, object2) {
              baseAssignValue(result2, key, iteratee2(value, key, object2));
            });
            return result2;
          }
          var merge = createAssigner(function(object, source, srcIndex) {
            baseMerge(object, source, srcIndex);
          });
          var mergeWith = createAssigner(function(object, source, srcIndex, customizer2) {
            baseMerge(object, source, srcIndex, customizer2);
          });
          var omit = flatRest(function(object, paths) {
            var result2 = {};
            if (object == null) {
              return result2;
            }
            var isDeep = false;
            paths = arrayMap(paths, function(path) {
              path = castPath(path, object);
              isDeep || (isDeep = path.length > 1);
              return path;
            });
            copyObject(object, getAllKeysIn(object), result2);
            if (isDeep) {
              result2 = baseClone(result2, CLONE_DEEP_FLAG | CLONE_FLAT_FLAG | CLONE_SYMBOLS_FLAG, customOmitClone);
            }
            var length = paths.length;
            while (length--) {
              baseUnset(result2, paths[length]);
            }
            return result2;
          });
          function omitBy(object, predicate) {
            return pickBy(object, negate(getIteratee(predicate)));
          }
          var pick = flatRest(function(object, paths) {
            return object == null ? {} : basePick(object, paths);
          });
          function pickBy(object, predicate) {
            if (object == null) {
              return {};
            }
            var props = arrayMap(getAllKeysIn(object), function(prop) {
              return [prop];
            });
            predicate = getIteratee(predicate);
            return basePickBy(object, props, function(value, path) {
              return predicate(value, path[0]);
            });
          }
          function result(object, path, defaultValue) {
            path = castPath(path, object);
            var index = -1, length = path.length;
            if (!length) {
              length = 1;
              object = undefined2;
            }
            while (++index < length) {
              var value = object == null ? undefined2 : object[toKey(path[index])];
              if (value === undefined2) {
                index = length;
                value = defaultValue;
              }
              object = isFunction(value) ? value.call(object) : value;
            }
            return object;
          }
          function set(object, path, value) {
            return object == null ? object : baseSet(object, path, value);
          }
          function setWith(object, path, value, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            return object == null ? object : baseSet(object, path, value, customizer2);
          }
          var toPairs = createToPairs(keys);
          var toPairsIn = createToPairs(keysIn);
          function transform(object, iteratee2, accumulator) {
            var isArr = isArray(object), isArrLike = isArr || isBuffer(object) || isTypedArray(object);
            iteratee2 = getIteratee(iteratee2, 4);
            if (accumulator == null) {
              var Ctor = object && object.constructor;
              if (isArrLike) {
                accumulator = isArr ? new Ctor() : [];
              } else if (isObject(object)) {
                accumulator = isFunction(Ctor) ? baseCreate(getPrototype(object)) : {};
              } else {
                accumulator = {};
              }
            }
            (isArrLike ? arrayEach : baseForOwn)(object, function(value, index, object2) {
              return iteratee2(accumulator, value, index, object2);
            });
            return accumulator;
          }
          function unset(object, path) {
            return object == null ? true : baseUnset(object, path);
          }
          function update(object, path, updater) {
            return object == null ? object : baseUpdate(object, path, castFunction(updater));
          }
          function updateWith(object, path, updater, customizer2) {
            customizer2 = typeof customizer2 == "function" ? customizer2 : undefined2;
            return object == null ? object : baseUpdate(object, path, castFunction(updater), customizer2);
          }
          function values2(object) {
            return object == null ? [] : baseValues(object, keys(object));
          }
          function valuesIn(object) {
            return object == null ? [] : baseValues(object, keysIn(object));
          }
          function clamp(number, lower, upper) {
            if (upper === undefined2) {
              upper = lower;
              lower = undefined2;
            }
            if (upper !== undefined2) {
              upper = toNumber2(upper);
              upper = upper === upper ? upper : 0;
            }
            if (lower !== undefined2) {
              lower = toNumber2(lower);
              lower = lower === lower ? lower : 0;
            }
            return baseClamp(toNumber2(number), lower, upper);
          }
          function inRange(number, start, end) {
            start = toFinite(start);
            if (end === undefined2) {
              end = start;
              start = 0;
            } else {
              end = toFinite(end);
            }
            number = toNumber2(number);
            return baseInRange(number, start, end);
          }
          function random(lower, upper, floating) {
            if (floating && typeof floating != "boolean" && isIterateeCall(lower, upper, floating)) {
              upper = floating = undefined2;
            }
            if (floating === undefined2) {
              if (typeof upper == "boolean") {
                floating = upper;
                upper = undefined2;
              } else if (typeof lower == "boolean") {
                floating = lower;
                lower = undefined2;
              }
            }
            if (lower === undefined2 && upper === undefined2) {
              lower = 0;
              upper = 1;
            } else {
              lower = toFinite(lower);
              if (upper === undefined2) {
                upper = lower;
                lower = 0;
              } else {
                upper = toFinite(upper);
              }
            }
            if (lower > upper) {
              var temp = lower;
              lower = upper;
              upper = temp;
            }
            if (floating || lower % 1 || upper % 1) {
              var rand = nativeRandom();
              return nativeMin(lower + rand * (upper - lower + freeParseFloat("1e-" + ((rand + "").length - 1))), upper);
            }
            return baseRandom(lower, upper);
          }
          var camelCase = createCompounder(function(result2, word, index) {
            word = word.toLowerCase();
            return result2 + (index ? capitalize(word) : word);
          });
          function capitalize(string) {
            return upperFirst(toString(string).toLowerCase());
          }
          function deburr(string) {
            string = toString(string);
            return string && string.replace(reLatin, deburrLetter).replace(reComboMark, "");
          }
          function endsWith(string, target, position) {
            string = toString(string);
            target = baseToString(target);
            var length = string.length;
            position = position === undefined2 ? length : baseClamp(toInteger(position), 0, length);
            var end = position;
            position -= target.length;
            return position >= 0 && string.slice(position, end) == target;
          }
          function escape(string) {
            string = toString(string);
            return string && reHasUnescapedHtml.test(string) ? string.replace(reUnescapedHtml, escapeHtmlChar) : string;
          }
          function escapeRegExp(string) {
            string = toString(string);
            return string && reHasRegExpChar.test(string) ? string.replace(reRegExpChar, "\\$&") : string;
          }
          var kebabCase = createCompounder(function(result2, word, index) {
            return result2 + (index ? "-" : "") + word.toLowerCase();
          });
          var lowerCase = createCompounder(function(result2, word, index) {
            return result2 + (index ? " " : "") + word.toLowerCase();
          });
          var lowerFirst = createCaseFirst("toLowerCase");
          function pad(string, length, chars) {
            string = toString(string);
            length = toInteger(length);
            var strLength = length ? stringSize(string) : 0;
            if (!length || strLength >= length) {
              return string;
            }
            var mid = (length - strLength) / 2;
            return createPadding(nativeFloor(mid), chars) + string + createPadding(nativeCeil(mid), chars);
          }
          function padEnd(string, length, chars) {
            string = toString(string);
            length = toInteger(length);
            var strLength = length ? stringSize(string) : 0;
            return length && strLength < length ? string + createPadding(length - strLength, chars) : string;
          }
          function padStart(string, length, chars) {
            string = toString(string);
            length = toInteger(length);
            var strLength = length ? stringSize(string) : 0;
            return length && strLength < length ? createPadding(length - strLength, chars) + string : string;
          }
          function parseInt2(string, radix, guard) {
            if (guard || radix == null) {
              radix = 0;
            } else if (radix) {
              radix = +radix;
            }
            return nativeParseInt(toString(string).replace(reTrimStart, ""), radix || 0);
          }
          function repeat(string, n, guard) {
            if (guard ? isIterateeCall(string, n, guard) : n === undefined2) {
              n = 1;
            } else {
              n = toInteger(n);
            }
            return baseRepeat(toString(string), n);
          }
          function replace() {
            var args = arguments, string = toString(args[0]);
            return args.length < 3 ? string : string.replace(args[1], args[2]);
          }
          var snakeCase = createCompounder(function(result2, word, index) {
            return result2 + (index ? "_" : "") + word.toLowerCase();
          });
          function split(string, separator, limit) {
            if (limit && typeof limit != "number" && isIterateeCall(string, separator, limit)) {
              separator = limit = undefined2;
            }
            limit = limit === undefined2 ? MAX_ARRAY_LENGTH : limit >>> 0;
            if (!limit) {
              return [];
            }
            string = toString(string);
            if (string && (typeof separator == "string" || separator != null && !isRegExp(separator))) {
              separator = baseToString(separator);
              if (!separator && hasUnicode(string)) {
                return castSlice(stringToArray(string), 0, limit);
              }
            }
            return string.split(separator, limit);
          }
          var startCase = createCompounder(function(result2, word, index) {
            return result2 + (index ? " " : "") + upperFirst(word);
          });
          function startsWith(string, target, position) {
            string = toString(string);
            position = position == null ? 0 : baseClamp(toInteger(position), 0, string.length);
            target = baseToString(target);
            return string.slice(position, position + target.length) == target;
          }
          function template(string, options, guard) {
            var settings = lodash.templateSettings;
            if (guard && isIterateeCall(string, options, guard)) {
              options = undefined2;
            }
            string = toString(string);
            options = assignWith({}, options, settings, customDefaultsAssignIn);
            var imports = assignWith({}, options.imports, settings.imports, customDefaultsAssignIn), importsKeys = keys(imports), importsValues = baseValues(imports, importsKeys);
            arrayEach(importsKeys, function(key) {
              if (reForbiddenIdentifierChars.test(key)) {
                throw String(INVALID_TEMPL_IMPORTS_ERROR_TEXT);
              }
            });
            var isEscaping, isEvaluating, index = 0, interpolate = options.interpolate || reNoMatch, source = "__p += '";
            var reDelimiters = RegExp2(
              (options.escape || reNoMatch).source + "|" + interpolate.source + "|" + (interpolate === reInterpolate ? reEsTemplate : reNoMatch).source + "|" + (options.evaluate || reNoMatch).source + "|$",
              "g"
            );
            var sourceURL = "//# sourceURL=" + (hasOwnProperty.call(options, "sourceURL") ? (options.sourceURL + "").replace(/\s/g, " ") : "lodash.templateSources[" + ++templateCounter + "]") + "\n";
            string.replace(reDelimiters, function(match, escapeValue, interpolateValue, esTemplateValue, evaluateValue, offset) {
              interpolateValue || (interpolateValue = esTemplateValue);
              source += string.slice(index, offset).replace(reUnescapedString, escapeStringChar);
              if (escapeValue) {
                isEscaping = true;
                source += "' +\n__e(" + escapeValue + ") +\n'";
              }
              if (evaluateValue) {
                isEvaluating = true;
                source += "';\n" + evaluateValue + ";\n__p += '";
              }
              if (interpolateValue) {
                source += "' +\n((__t = (" + interpolateValue + ")) == null ? '' : __t) +\n'";
              }
              index = offset + match.length;
              return match;
            });
            source += "';\n";
            var variable = hasOwnProperty.call(options, "variable") && options.variable;
            if (!variable) {
              source = "with (obj) {\n" + source + "\n}\n";
            } else if (reForbiddenIdentifierChars.test(variable)) {
              throw String(INVALID_TEMPL_VAR_ERROR_TEXT);
            }
            source = (isEvaluating ? source.replace(reEmptyStringLeading, "") : source).replace(reEmptyStringMiddle, "$1").replace(reEmptyStringTrailing, "$1;");
            source = "function(" + (variable || "obj") + ") {\n" + (variable ? "" : "obj || (obj = {});\n") + "var __t, __p = ''" + (isEscaping ? ", __e = _.escape" : "") + (isEvaluating ? ", __j = Array.prototype.join;\nfunction print() { __p += __j.call(arguments, '') }\n" : ";\n") + source + "return __p\n}";
            var result2 = attempt(function() {
              return Function2(importsKeys, sourceURL + "return " + source).apply(undefined2, importsValues);
            });
            result2.source = source;
            if (isError(result2)) {
              throw result2;
            }
            return result2;
          }
          function toLower(value) {
            return toString(value).toLowerCase();
          }
          function toUpper(value) {
            return toString(value).toUpperCase();
          }
          function trim(string, chars, guard) {
            string = toString(string);
            if (string && (guard || chars === undefined2)) {
              return baseTrim(string);
            }
            if (!string || !(chars = baseToString(chars))) {
              return string;
            }
            var strSymbols = stringToArray(string), chrSymbols = stringToArray(chars), start = charsStartIndex(strSymbols, chrSymbols), end = charsEndIndex(strSymbols, chrSymbols) + 1;
            return castSlice(strSymbols, start, end).join("");
          }
          function trimEnd(string, chars, guard) {
            string = toString(string);
            if (string && (guard || chars === undefined2)) {
              return string.slice(0, trimmedEndIndex(string) + 1);
            }
            if (!string || !(chars = baseToString(chars))) {
              return string;
            }
            var strSymbols = stringToArray(string), end = charsEndIndex(strSymbols, stringToArray(chars)) + 1;
            return castSlice(strSymbols, 0, end).join("");
          }
          function trimStart(string, chars, guard) {
            string = toString(string);
            if (string && (guard || chars === undefined2)) {
              return string.replace(reTrimStart, "");
            }
            if (!string || !(chars = baseToString(chars))) {
              return string;
            }
            var strSymbols = stringToArray(string), start = charsStartIndex(strSymbols, stringToArray(chars));
            return castSlice(strSymbols, start).join("");
          }
          function truncate(string, options) {
            var length = DEFAULT_TRUNC_LENGTH, omission = DEFAULT_TRUNC_OMISSION;
            if (isObject(options)) {
              var separator = "separator" in options ? options.separator : separator;
              length = "length" in options ? toInteger(options.length) : length;
              omission = "omission" in options ? baseToString(options.omission) : omission;
            }
            string = toString(string);
            var strLength = string.length;
            if (hasUnicode(string)) {
              var strSymbols = stringToArray(string);
              strLength = strSymbols.length;
            }
            if (length >= strLength) {
              return string;
            }
            var end = length - stringSize(omission);
            if (end < 1) {
              return omission;
            }
            var result2 = strSymbols ? castSlice(strSymbols, 0, end).join("") : string.slice(0, end);
            if (separator === undefined2) {
              return result2 + omission;
            }
            if (strSymbols) {
              end += result2.length - end;
            }
            if (isRegExp(separator)) {
              if (string.slice(end).search(separator)) {
                var match, substring = result2;
                if (!separator.global) {
                  separator = RegExp2(separator.source, toString(reFlags.exec(separator)) + "g");
                }
                separator.lastIndex = 0;
                while (match = separator.exec(substring)) {
                  var newEnd = match.index;
                }
                result2 = result2.slice(0, newEnd === undefined2 ? end : newEnd);
              }
            } else if (string.indexOf(baseToString(separator), end) != end) {
              var index = result2.lastIndexOf(separator);
              if (index > -1) {
                result2 = result2.slice(0, index);
              }
            }
            return result2 + omission;
          }
          function unescape(string) {
            string = toString(string);
            return string && reHasEscapedHtml.test(string) ? string.replace(reEscapedHtml, unescapeHtmlChar) : string;
          }
          var upperCase = createCompounder(function(result2, word, index) {
            return result2 + (index ? " " : "") + word.toUpperCase();
          });
          var upperFirst = createCaseFirst("toUpperCase");
          function words(string, pattern, guard) {
            string = toString(string);
            pattern = guard ? undefined2 : pattern;
            if (pattern === undefined2) {
              return hasUnicodeWord(string) ? unicodeWords(string) : asciiWords(string);
            }
            return string.match(pattern) || [];
          }
          var attempt = baseRest(function(func, args) {
            try {
              return apply(func, undefined2, args);
            } catch (e) {
              return isError(e) ? e : new Error2(e);
            }
          });
          var bindAll = flatRest(function(object, methodNames) {
            arrayEach(methodNames, function(key) {
              key = toKey(key);
              baseAssignValue(object, key, bind(object[key], object));
            });
            return object;
          });
          function cond(pairs) {
            var length = pairs == null ? 0 : pairs.length, toIteratee = getIteratee();
            pairs = !length ? [] : arrayMap(pairs, function(pair) {
              if (typeof pair[1] != "function") {
                throw String(FUNC_ERROR_TEXT);
              }
              return [toIteratee(pair[0]), pair[1]];
            });
            return baseRest(function(args) {
              var index = -1;
              while (++index < length) {
                var pair = pairs[index];
                if (apply(pair[0], this, args)) {
                  return apply(pair[1], this, args);
                }
              }
            });
          }
          function conforms(source) {
            return baseConforms(baseClone(source, CLONE_DEEP_FLAG));
          }
          function constant(value) {
            return function() {
              return value;
            };
          }
          function defaultTo(value, defaultValue) {
            return value == null || value !== value ? defaultValue : value;
          }
          var flow = createFlow();
          var flowRight = createFlow(true);
          function identity(value) {
            return value;
          }
          function iteratee(func) {
            return baseIteratee(typeof func == "function" ? func : baseClone(func, CLONE_DEEP_FLAG));
          }
          function matches(source) {
            return baseMatches(baseClone(source, CLONE_DEEP_FLAG));
          }
          function matchesProperty(path, srcValue) {
            return baseMatchesProperty(path, baseClone(srcValue, CLONE_DEEP_FLAG));
          }
          var method = baseRest(function(path, args) {
            return function(object) {
              return baseInvoke(object, path, args);
            };
          });
          var methodOf = baseRest(function(object, args) {
            return function(path) {
              return baseInvoke(object, path, args);
            };
          });
          function mixin(object, source, options) {
            var props = keys(source), methodNames = baseFunctions(source, props);
            if (options == null && !(isObject(source) && (methodNames.length || !props.length))) {
              options = source;
              source = object;
              object = this;
              methodNames = baseFunctions(source, keys(source));
            }
            var chain2 = !(isObject(options) && "chain" in options) || !!options.chain, isFunc = isFunction(object);
            arrayEach(methodNames, function(methodName) {
              var func = source[methodName];
              object[methodName] = func;
              if (isFunc) {
                object.prototype[methodName] = function() {
                  var chainAll = this.__chain__;
                  if (chain2 || chainAll) {
                    var result2 = object(this.__wrapped__), actions = result2.__actions__ = copyArray(this.__actions__);
                    actions.push({ "func": func, "args": arguments, "thisArg": object });
                    result2.__chain__ = chainAll;
                    return result2;
                  }
                  return func.apply(object, arrayPush([this.value()], arguments));
                };
              }
            });
            return object;
          }
          function noConflict() {
            if (root._ === this) {
              root._ = oldDash;
            }
            return this;
          }
          function noop() {
          }
          function nthArg(n) {
            n = toInteger(n);
            return baseRest(function(args) {
              return baseNth(args, n);
            });
          }
          var over = createOver(arrayMap);
          var overEvery = createOver(arrayEvery);
          var overSome = createOver(arraySome);
          function property(path) {
            return isKey(path) ? baseProperty(toKey(path)) : basePropertyDeep(path);
          }
          function propertyOf(object) {
            return function(path) {
              return object == null ? undefined2 : baseGet(object, path);
            };
          }
          var range = createRange();
          var rangeRight = createRange(true);
          function stubArray() {
            return [];
          }
          function stubFalse() {
            return false;
          }
          function stubObject() {
            return {};
          }
          function stubString() {
            return "";
          }
          function stubTrue() {
            return true;
          }
          function times(n, iteratee2) {
            n = toInteger(n);
            if (n < 1 || n > MAX_SAFE_INTEGER) {
              return [];
            }
            var index = MAX_ARRAY_LENGTH, length = nativeMin(n, MAX_ARRAY_LENGTH);
            iteratee2 = getIteratee(iteratee2);
            n -= MAX_ARRAY_LENGTH;
            var result2 = baseTimes(length, iteratee2);
            while (++index < n) {
              iteratee2(index);
            }
            return result2;
          }
          function toPath(value) {
            if (isArray(value)) {
              return arrayMap(value, toKey);
            }
            return isSymbol(value) ? [value] : copyArray(stringToPath(toString(value)));
          }
          function uniqueId(prefix) {
            var id2 = ++idCounter;
            return toString(prefix) + id2;
          }
          var add = createMathOperation(function(augend, addend) {
            return augend + addend;
          }, 0);
          var ceil = createRound("ceil");
          var divide = createMathOperation(function(dividend, divisor) {
            return dividend / divisor;
          }, 1);
          var floor = createRound("floor");
          function max(array) {
            return array && array.length ? baseExtremum(array, identity, baseGt) : undefined2;
          }
          function maxBy(array, iteratee2) {
            return array && array.length ? baseExtremum(array, getIteratee(iteratee2, 2), baseGt) : undefined2;
          }
          function mean(array) {
            return baseMean(array, identity);
          }
          function meanBy(array, iteratee2) {
            return baseMean(array, getIteratee(iteratee2, 2));
          }
          function min(array) {
            return array && array.length ? baseExtremum(array, identity, baseLt) : undefined2;
          }
          function minBy(array, iteratee2) {
            return array && array.length ? baseExtremum(array, getIteratee(iteratee2, 2), baseLt) : undefined2;
          }
          var multiply = createMathOperation(function(multiplier, multiplicand) {
            return multiplier * multiplicand;
          }, 1);
          var round = createRound("round");
          var subtract = createMathOperation(function(minuend, subtrahend) {
            return minuend - subtrahend;
          }, 0);
          function sum(array) {
            return array && array.length ? baseSum(array, identity) : 0;
          }
          function sumBy(array, iteratee2) {
            return array && array.length ? baseSum(array, getIteratee(iteratee2, 2)) : 0;
          }
          lodash.after = after;
          lodash.ary = ary;
          lodash.assign = assign;
          lodash.assignIn = assignIn;
          lodash.assignInWith = assignInWith;
          lodash.assignWith = assignWith;
          lodash.at = at;
          lodash.before = before;
          lodash.bind = bind;
          lodash.bindAll = bindAll;
          lodash.bindKey = bindKey;
          lodash.castArray = castArray;
          lodash.chain = chain;
          lodash.chunk = chunk;
          lodash.compact = compact2;
          lodash.concat = concat;
          lodash.cond = cond;
          lodash.conforms = conforms;
          lodash.constant = constant;
          lodash.countBy = countBy;
          lodash.create = create;
          lodash.curry = curry;
          lodash.curryRight = curryRight;
          lodash.debounce = debounce;
          lodash.defaults = defaults;
          lodash.defaultsDeep = defaultsDeep;
          lodash.defer = defer;
          lodash.delay = delay;
          lodash.difference = difference;
          lodash.differenceBy = differenceBy;
          lodash.differenceWith = differenceWith;
          lodash.drop = drop;
          lodash.dropRight = dropRight;
          lodash.dropRightWhile = dropRightWhile;
          lodash.dropWhile = dropWhile;
          lodash.fill = fill;
          lodash.filter = filter;
          lodash.flatMap = flatMap;
          lodash.flatMapDeep = flatMapDeep;
          lodash.flatMapDepth = flatMapDepth;
          lodash.flatten = flatten;
          lodash.flattenDeep = flattenDeep;
          lodash.flattenDepth = flattenDepth;
          lodash.flip = flip;
          lodash.flow = flow;
          lodash.flowRight = flowRight;
          lodash.fromPairs = fromPairs;
          lodash.functions = functions;
          lodash.functionsIn = functionsIn;
          lodash.groupBy = groupBy;
          lodash.initial = initial;
          lodash.intersection = intersection;
          lodash.intersectionBy = intersectionBy;
          lodash.intersectionWith = intersectionWith;
          lodash.invert = invert;
          lodash.invertBy = invertBy;
          lodash.invokeMap = invokeMap;
          lodash.iteratee = iteratee;
          lodash.keyBy = keyBy;
          lodash.keys = keys;
          lodash.keysIn = keysIn;
          lodash.map = map;
          lodash.mapKeys = mapKeys;
          lodash.mapValues = mapValues;
          lodash.matches = matches;
          lodash.matchesProperty = matchesProperty;
          lodash.memoize = memoize;
          lodash.merge = merge;
          lodash.mergeWith = mergeWith;
          lodash.method = method;
          lodash.methodOf = methodOf;
          lodash.mixin = mixin;
          lodash.negate = negate;
          lodash.nthArg = nthArg;
          lodash.omit = omit;
          lodash.omitBy = omitBy;
          lodash.once = once;
          lodash.orderBy = orderBy;
          lodash.over = over;
          lodash.overArgs = overArgs;
          lodash.overEvery = overEvery;
          lodash.overSome = overSome;
          lodash.partial = partial;
          lodash.partialRight = partialRight;
          lodash.partition = partition;
          lodash.pick = pick;
          lodash.pickBy = pickBy;
          lodash.property = property;
          lodash.propertyOf = propertyOf;
          lodash.pull = pull;
          lodash.pullAll = pullAll;
          lodash.pullAllBy = pullAllBy;
          lodash.pullAllWith = pullAllWith;
          lodash.pullAt = pullAt;
          lodash.range = range;
          lodash.rangeRight = rangeRight;
          lodash.rearg = rearg;
          lodash.reject = reject;
          lodash.remove = remove;
          lodash.rest = rest;
          lodash.reverse = reverse;
          lodash.sampleSize = sampleSize;
          lodash.set = set;
          lodash.setWith = setWith;
          lodash.shuffle = shuffle;
          lodash.slice = slice;
          lodash.sortBy = sortBy;
          lodash.sortedUniq = sortedUniq;
          lodash.sortedUniqBy = sortedUniqBy;
          lodash.split = split;
          lodash.spread = spread;
          lodash.tail = tail;
          lodash.take = take;
          lodash.takeRight = takeRight;
          lodash.takeRightWhile = takeRightWhile;
          lodash.takeWhile = takeWhile;
          lodash.tap = tap;
          lodash.throttle = throttle;
          lodash.thru = thru;
          lodash.toArray = toArray;
          lodash.toPairs = toPairs;
          lodash.toPairsIn = toPairsIn;
          lodash.toPath = toPath;
          lodash.toPlainObject = toPlainObject;
          lodash.transform = transform;
          lodash.unary = unary;
          lodash.union = union;
          lodash.unionBy = unionBy;
          lodash.unionWith = unionWith;
          lodash.uniq = uniq;
          lodash.uniqBy = uniqBy;
          lodash.uniqWith = uniqWith;
          lodash.unset = unset;
          lodash.unzip = unzip;
          lodash.unzipWith = unzipWith;
          lodash.update = update;
          lodash.updateWith = updateWith;
          lodash.values = values2;
          lodash.valuesIn = valuesIn;
          lodash.without = without;
          lodash.words = words;
          lodash.wrap = wrap;
          lodash.xor = xor;
          lodash.xorBy = xorBy;
          lodash.xorWith = xorWith;
          lodash.zip = zip;
          lodash.zipObject = zipObject;
          lodash.zipObjectDeep = zipObjectDeep;
          lodash.zipWith = zipWith;
          lodash.entries = toPairs;
          lodash.entriesIn = toPairsIn;
          lodash.extend = assignIn;
          lodash.extendWith = assignInWith;
          mixin(lodash, lodash);
          lodash.add = add;
          lodash.attempt = attempt;
          lodash.camelCase = camelCase;
          lodash.capitalize = capitalize;
          lodash.ceil = ceil;
          lodash.clamp = clamp;
          lodash.clone = clone;
          lodash.cloneDeep = cloneDeep2;
          lodash.cloneDeepWith = cloneDeepWith2;
          lodash.cloneWith = cloneWith;
          lodash.conformsTo = conformsTo;
          lodash.deburr = deburr;
          lodash.defaultTo = defaultTo;
          lodash.divide = divide;
          lodash.endsWith = endsWith;
          lodash.eq = eq;
          lodash.escape = escape;
          lodash.escapeRegExp = escapeRegExp;
          lodash.every = every;
          lodash.find = find;
          lodash.findIndex = findIndex;
          lodash.findKey = findKey;
          lodash.findLast = findLast;
          lodash.findLastIndex = findLastIndex;
          lodash.findLastKey = findLastKey;
          lodash.floor = floor;
          lodash.forEach = forEach;
          lodash.forEachRight = forEachRight;
          lodash.forIn = forIn;
          lodash.forInRight = forInRight;
          lodash.forOwn = forOwn;
          lodash.forOwnRight = forOwnRight;
          lodash.get = get;
          lodash.gt = gt;
          lodash.gte = gte;
          lodash.has = has;
          lodash.hasIn = hasIn;
          lodash.head = head;
          lodash.identity = identity;
          lodash.includes = includes;
          lodash.indexOf = indexOf;
          lodash.inRange = inRange;
          lodash.invoke = invoke;
          lodash.isArguments = isArguments;
          lodash.isArray = isArray;
          lodash.isArrayBuffer = isArrayBuffer;
          lodash.isArrayLike = isArrayLike;
          lodash.isArrayLikeObject = isArrayLikeObject;
          lodash.isBoolean = isBoolean;
          lodash.isBuffer = isBuffer;
          lodash.isDate = isDate;
          lodash.isElement = isElement;
          lodash.isEmpty = isEmpty;
          lodash.isEqual = isEqual;
          lodash.isEqualWith = isEqualWith;
          lodash.isError = isError;
          lodash.isFinite = isFinite2;
          lodash.isFunction = isFunction;
          lodash.isInteger = isInteger;
          lodash.isLength = isLength;
          lodash.isMap = isMap;
          lodash.isMatch = isMatch;
          lodash.isMatchWith = isMatchWith;
          lodash.isNaN = isNaN2;
          lodash.isNative = isNative;
          lodash.isNil = isNil;
          lodash.isNull = isNull;
          lodash.isNumber = isNumber11;
          lodash.isObject = isObject;
          lodash.isObjectLike = isObjectLike;
          lodash.isPlainObject = isPlainObject;
          lodash.isRegExp = isRegExp;
          lodash.isSafeInteger = isSafeInteger;
          lodash.isSet = isSet;
          lodash.isString = isString;
          lodash.isSymbol = isSymbol;
          lodash.isTypedArray = isTypedArray;
          lodash.isUndefined = isUndefined;
          lodash.isWeakMap = isWeakMap;
          lodash.isWeakSet = isWeakSet;
          lodash.join = join;
          lodash.kebabCase = kebabCase;
          lodash.last = last;
          lodash.lastIndexOf = lastIndexOf;
          lodash.lowerCase = lowerCase;
          lodash.lowerFirst = lowerFirst;
          lodash.lt = lt;
          lodash.lte = lte;
          lodash.max = max;
          lodash.maxBy = maxBy;
          lodash.mean = mean;
          lodash.meanBy = meanBy;
          lodash.min = min;
          lodash.minBy = minBy;
          lodash.stubArray = stubArray;
          lodash.stubFalse = stubFalse;
          lodash.stubObject = stubObject;
          lodash.stubString = stubString;
          lodash.stubTrue = stubTrue;
          lodash.multiply = multiply;
          lodash.nth = nth;
          lodash.noConflict = noConflict;
          lodash.noop = noop;
          lodash.now = now;
          lodash.pad = pad;
          lodash.padEnd = padEnd;
          lodash.padStart = padStart;
          lodash.parseInt = parseInt2;
          lodash.random = random;
          lodash.reduce = reduce;
          lodash.reduceRight = reduceRight;
          lodash.repeat = repeat;
          lodash.replace = replace;
          lodash.result = result;
          lodash.round = round;
          lodash.runInContext = runInContext2;
          lodash.sample = sample;
          lodash.size = size;
          lodash.snakeCase = snakeCase;
          lodash.some = some;
          lodash.sortedIndex = sortedIndex;
          lodash.sortedIndexBy = sortedIndexBy;
          lodash.sortedIndexOf = sortedIndexOf;
          lodash.sortedLastIndex = sortedLastIndex;
          lodash.sortedLastIndexBy = sortedLastIndexBy;
          lodash.sortedLastIndexOf = sortedLastIndexOf;
          lodash.startCase = startCase;
          lodash.startsWith = startsWith;
          lodash.subtract = subtract;
          lodash.sum = sum;
          lodash.sumBy = sumBy;
          lodash.template = template;
          lodash.times = times;
          lodash.toFinite = toFinite;
          lodash.toInteger = toInteger;
          lodash.toLength = toLength;
          lodash.toLower = toLower;
          lodash.toNumber = toNumber2;
          lodash.toSafeInteger = toSafeInteger;
          lodash.toString = toString;
          lodash.toUpper = toUpper;
          lodash.trim = trim;
          lodash.trimEnd = trimEnd;
          lodash.trimStart = trimStart;
          lodash.truncate = truncate;
          lodash.unescape = unescape;
          lodash.uniqueId = uniqueId;
          lodash.upperCase = upperCase;
          lodash.upperFirst = upperFirst;
          lodash.each = forEach;
          lodash.eachRight = forEachRight;
          lodash.first = head;
          mixin(lodash, (function() {
            var source = {};
            baseForOwn(lodash, function(func, methodName) {
              if (!hasOwnProperty.call(lodash.prototype, methodName)) {
                source[methodName] = func;
              }
            });
            return source;
          })(), { "chain": false });
          lodash.VERSION = VERSION;
          arrayEach(["bind", "bindKey", "curry", "curryRight", "partial", "partialRight"], function(methodName) {
            lodash[methodName].placeholder = lodash;
          });
          arrayEach(["drop", "take"], function(methodName, index) {
            LazyWrapper.prototype[methodName] = function(n) {
              n = n === undefined2 ? 1 : nativeMax(toInteger(n), 0);
              var result2 = this.__filtered__ && !index ? new LazyWrapper(this) : this.clone();
              if (result2.__filtered__) {
                result2.__takeCount__ = nativeMin(n, result2.__takeCount__);
              } else {
                result2.__views__.push({
                  "size": nativeMin(n, MAX_ARRAY_LENGTH),
                  "type": methodName + (result2.__dir__ < 0 ? "Right" : "")
                });
              }
              return result2;
            };
            LazyWrapper.prototype[methodName + "Right"] = function(n) {
              return this.reverse()[methodName](n).reverse();
            };
          });
          arrayEach(["filter", "map", "takeWhile"], function(methodName, index) {
            var type = index + 1, isFilter = type == LAZY_FILTER_FLAG || type == LAZY_WHILE_FLAG;
            LazyWrapper.prototype[methodName] = function(iteratee2) {
              var result2 = this.clone();
              result2.__iteratees__.push({
                "iteratee": getIteratee(iteratee2, 3),
                "type": type
              });
              result2.__filtered__ = result2.__filtered__ || isFilter;
              return result2;
            };
          });
          arrayEach(["head", "last"], function(methodName, index) {
            var takeName = "take" + (index ? "Right" : "");
            LazyWrapper.prototype[methodName] = function() {
              return this[takeName](1).value()[0];
            };
          });
          arrayEach(["initial", "tail"], function(methodName, index) {
            var dropName = "drop" + (index ? "" : "Right");
            LazyWrapper.prototype[methodName] = function() {
              return this.__filtered__ ? new LazyWrapper(this) : this[dropName](1);
            };
          });
          LazyWrapper.prototype.compact = function() {
            return this.filter(identity);
          };
          LazyWrapper.prototype.find = function(predicate) {
            return this.filter(predicate).head();
          };
          LazyWrapper.prototype.findLast = function(predicate) {
            return this.reverse().find(predicate);
          };
          LazyWrapper.prototype.invokeMap = baseRest(function(path, args) {
            if (typeof path == "function") {
              return new LazyWrapper(this);
            }
            return this.map(function(value) {
              return baseInvoke(value, path, args);
            });
          });
          LazyWrapper.prototype.reject = function(predicate) {
            return this.filter(negate(getIteratee(predicate)));
          };
          LazyWrapper.prototype.slice = function(start, end) {
            start = toInteger(start);
            var result2 = this;
            if (result2.__filtered__ && (start > 0 || end < 0)) {
              return new LazyWrapper(result2);
            }
            if (start < 0) {
              result2 = result2.takeRight(-start);
            } else if (start) {
              result2 = result2.drop(start);
            }
            if (end !== undefined2) {
              end = toInteger(end);
              result2 = end < 0 ? result2.dropRight(-end) : result2.take(end - start);
            }
            return result2;
          };
          LazyWrapper.prototype.takeRightWhile = function(predicate) {
            return this.reverse().takeWhile(predicate).reverse();
          };
          LazyWrapper.prototype.toArray = function() {
            return this.take(MAX_ARRAY_LENGTH);
          };
          baseForOwn(LazyWrapper.prototype, function(func, methodName) {
            var checkIteratee = /^(?:filter|find|map|reject)|While$/.test(methodName), isTaker = /^(?:head|last)$/.test(methodName), lodashFunc = lodash[isTaker ? "take" + (methodName == "last" ? "Right" : "") : methodName], retUnwrapped = isTaker || /^find/.test(methodName);
            if (!lodashFunc) {
              return;
            }
            lodash.prototype[methodName] = function() {
              var value = this.__wrapped__, args = isTaker ? [1] : arguments, isLazy = value instanceof LazyWrapper, iteratee2 = args[0], useLazy = isLazy || isArray(value);
              var interceptor = function(value2) {
                var result3 = lodashFunc.apply(lodash, arrayPush([value2], args));
                return isTaker && chainAll ? result3[0] : result3;
              };
              if (useLazy && checkIteratee && typeof iteratee2 == "function" && iteratee2.length != 1) {
                isLazy = useLazy = false;
              }
              var chainAll = this.__chain__, isHybrid = !!this.__actions__.length, isUnwrapped = retUnwrapped && !chainAll, onlyLazy = isLazy && !isHybrid;
              if (!retUnwrapped && useLazy) {
                value = onlyLazy ? value : new LazyWrapper(this);
                var result2 = func.apply(value, args);
                result2.__actions__.push({ "func": thru, "args": [interceptor], "thisArg": undefined2 });
                return new LodashWrapper(result2, chainAll);
              }
              if (isUnwrapped && onlyLazy) {
                return func.apply(this, args);
              }
              result2 = this.thru(interceptor);
              return isUnwrapped ? isTaker ? result2.value()[0] : result2.value() : result2;
            };
          });
          arrayEach(["pop", "push", "shift", "sort", "splice", "unshift"], function(methodName) {
            var func = arrayProto[methodName], chainName = /^(?:push|sort|unshift)$/.test(methodName) ? "tap" : "thru", retUnwrapped = /^(?:pop|shift)$/.test(methodName);
            lodash.prototype[methodName] = function() {
              var args = arguments;
              if (retUnwrapped && !this.__chain__) {
                var value = this.value();
                return func.apply(isArray(value) ? value : [], args);
              }
              return this[chainName](function(value2) {
                return func.apply(isArray(value2) ? value2 : [], args);
              });
            };
          });
          baseForOwn(LazyWrapper.prototype, function(func, methodName) {
            var lodashFunc = lodash[methodName];
            if (lodashFunc) {
              var key = lodashFunc.name + "";
              if (!hasOwnProperty.call(realNames, key)) {
                realNames[key] = [];
              }
              realNames[key].push({ "name": methodName, "func": lodashFunc });
            }
          });
          realNames[createHybrid(undefined2, WRAP_BIND_KEY_FLAG).name] = [{
            "name": "wrapper",
            "func": undefined2
          }];
          LazyWrapper.prototype.clone = lazyClone;
          LazyWrapper.prototype.reverse = lazyReverse;
          LazyWrapper.prototype.value = lazyValue;
          lodash.prototype.at = wrapperAt;
          lodash.prototype.chain = wrapperChain;
          lodash.prototype.commit = wrapperCommit;
          lodash.prototype.next = wrapperNext;
          lodash.prototype.plant = wrapperPlant;
          lodash.prototype.reverse = wrapperReverse;
          lodash.prototype.toJSON = lodash.prototype.valueOf = lodash.prototype.value = wrapperValue;
          lodash.prototype.first = lodash.prototype.head;
          if (symIterator) {
            lodash.prototype[symIterator] = wrapperToIterator;
          }
          return lodash;
        });
        var _ = runInContext();
        if (typeof define == "function" && typeof define.amd == "object" && define.amd) {
          root._ = _;
          define(function() {
            return _;
          });
        } else if (freeModule) {
          (freeModule.exports = _)._ = _;
          freeExports._ = _;
        } else {
          root._ = _;
        }
      }).call(exports);
    }
  });

  // node_modules/lodash/index.js
  var require_lodash2 = __commonJS({
    "node_modules/lodash/index.js"(exports, module) {
      module.exports = require_lodash();
    }
  });

  // stub-empty:application/editor/editorSingleton
  var require_editorSingleton = __commonJS({
    "stub-empty:application/editor/editorSingleton"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:../../../utilities/monomers
  var require_monomers = __commonJS({
    "stub-empty:../../../utilities/monomers"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/monomer
  var require_monomer = __commonJS({
    "stub-empty:application/editor/operations/monomer"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/drawingEntity
  var require_drawingEntity = __commonJS({
    "stub-empty:application/editor/operations/drawingEntity"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/polymerBond
  var require_polymerBond = __commonJS({
    "stub-empty:application/editor/operations/polymerBond"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/monomer/monomerFactory
  var require_monomerFactory = __commonJS({
    "stub-empty:application/editor/operations/monomer/monomerFactory"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/render/renderers/sequence/SequenceRenderer
  var require_SequenceRenderer = __commonJS({
    "stub-empty:application/render/renderers/sequence/SequenceRenderer"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/tools/types
  var require_types = __commonJS({
    "stub-empty:application/editor/tools/types"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/editorSettings
  var require_editorSettings = __commonJS({
    "stub-empty:application/editor/editorSettings"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/modes/snake
  var require_snake = __commonJS({
    "stub-empty:application/editor/operations/modes/snake"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/coreAtom/atom
  var require_atom = __commonJS({
    "stub-empty:application/editor/operations/coreAtom/atom"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/coreBond/bond
  var require_bond = __commonJS({
    "stub-empty:application/editor/operations/coreBond/bond"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/monomerToAtomBond/monomerToAtomBond
  var require_monomerToAtomBond = __commonJS({
    "stub-empty:application/editor/operations/monomerToAtomBond/monomerToAtomBond"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:../../utilities/monomers
  var require_monomers2 = __commonJS({
    "stub-empty:../../utilities/monomers"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/modes
  var require_modes = __commonJS({
    "stub-empty:application/editor/operations/modes"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor
  var require_editor = __commonJS({
    "stub-empty:application/editor"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/coreRxn/rxnArrow
  var require_rxnArrow = __commonJS({
    "stub-empty:application/editor/operations/coreRxn/rxnArrow"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/coreRxn/multitailArrow
  var require_multitailArrow = __commonJS({
    "stub-empty:application/editor/operations/coreRxn/multitailArrow"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:domain/serializers
  var require_serializers = __commonJS({
    "stub-empty:domain/serializers"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/editor/operations/coreRxn/rxnPlus
  var require_rxnPlus = __commonJS({
    "stub-empty:application/editor/operations/coreRxn/rxnPlus"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // stub-empty:application/render/restruct/draftToLexical
  var require_draftToLexical = __commonJS({
    "stub-empty:application/render/restruct/draftToLexical"(exports, module) {
      module.exports = new Proxy({}, { get: () => function() {
      } });
    }
  });

  // entry.ts
  var entry_exports = {};
  __export(entry_exports, {
    Atom: () => Atom2,
    AtomLabel: () => AtomLabel,
    AttachmentPointName: () => AttachmentPointName,
    Bond: () => Bond3,
    Box2Abs: () => Box2Abs,
    CREATE_MONOMER_TOOL_NAME: () => CREATE_MONOMER_TOOL_NAME,
    Command: () => Command,
    CoreAtom: () => Atom,
    DrawingEntitiesManager: () => DrawingEntitiesManager,
    ElementColor: () => ElementColor,
    Elements: () => Elements,
    Entities: () => Entities,
    FunctionalGroupsProvider: () => FunctionalGroupsProvider,
    Generics: () => Generics,
    HalfMonomerSize: () => HalfMonomerSize,
    IMAGE_KEY: () => IMAGE_KEY,
    IMAGE_SERIALIZE_KEY: () => IMAGE_SERIALIZE_KEY,
    KetAmbiguousMonomerTemplateSubType: () => KetAmbiguousMonomerTemplateSubType,
    KetSerializer: () => KetSerializer,
    MONOMER_CONST: () => MONOMER_CONST,
    MULTITAIL_ARROW_KEY: () => MULTITAIL_ARROW_KEY,
    MULTITAIL_ARROW_SERIALIZE_KEY: () => MULTITAIL_ARROW_SERIALIZE_KEY,
    MULTITAIL_ARROW_TOOL_NAME: () => MULTITAIL_ARROW_TOOL_NAME,
    MolSerializer: () => MolSerializer,
    MonomerSize: () => MonomerSize,
    NO_NATURAL_ANALOGUE: () => NO_NATURAL_ANALOGUE,
    Pile: () => Pile,
    Pool: () => Pool,
    QetcherJSWrapper: () => QetcherJSWrapper,
    RNA_DNA_NON_MODIFIED_PART: () => RNA_DNA_NON_MODIFIED_PART,
    RnaDnaBaseNames: () => RnaDnaBaseNames,
    RnaDnaNaturalAnaloguesEnum: () => RnaDnaNaturalAnaloguesEnum,
    STRAND_TYPE: () => STRAND_TYPE,
    SaltsAndSolventsProvider: () => SaltsAndSolventsProvider,
    Scale: () => Scale,
    SdfSerializer: () => SdfSerializer,
    SgContexts: () => SgContexts,
    SnakeLayoutCellWidth: () => SnakeLayoutCellWidth,
    StandardAmbiguousPeptide: () => StandardAmbiguousPeptide,
    StandardAmbiguousRnaBase: () => StandardAmbiguousRnaBase,
    StandardBondLength: () => StandardBondLength,
    StereoValidator: () => StereoValidator,
    Struct: () => Struct,
    Vec2: () => Vec2,
    attachmentPointNames: () => attachmentPointNames,
    fillNaturalAnalogueForPhosphateAndSugar: () => fillNaturalAnalogueForPhosphateAndSugar,
    genericsList: () => genericsList,
    getAttachmentPointLabel: () => getAttachmentPointLabel,
    getAttachmentPointLabelWithBinaryShift: () => getAttachmentPointLabelWithBinaryShift,
    getAttachmentPointNumberFromLabel: () => getAttachmentPointNumberFromLabel,
    getHELMClassByKetMonomerClass: () => getHELMClassByKetMonomerClass,
    getKetRef: () => getKetRef,
    getMonomerTemplateRefFromMonomerItem: () => getMonomerTemplateRefFromMonomerItem,
    getNextFreeAttachmentPoint: () => getNextFreeAttachmentPoint,
    getNodeWithInvertedYCoord: () => getNodeWithInvertedYCoord,
    imageReferencePositionToCursor: () => imageReferencePositionToCursor,
    isSingleRGroupAttachmentPoint: () => isSingleRGroupAttachmentPoint,
    modifyTransformation: () => modifyTransformation,
    multitailArrowReferenceLinesToCursor: () => multitailArrowReferenceLinesToCursor,
    multitailReferencePositionToCursor: () => multitailReferencePositionToCursor,
    peptideAmbiguousSymbols: () => peptideAmbiguousSymbols,
    peptideNaturalAnalogues: () => peptideNaturalAnalogues,
    populateStructWithSelection: () => populateStructWithSelection,
    rnaDnaAmbiguousSymbols: () => rnaDnaAmbiguousSymbols,
    rnaDnaNaturalAnalogues: () => rnaDnaNaturalAnalogues,
    setAmbiguousMonomerPrefix: () => setAmbiguousMonomerPrefix,
    setAmbiguousMonomerTemplatePrefix: () => setAmbiguousMonomerTemplatePrefix,
    setMonomerGroupTemplatePrefix: () => setMonomerGroupTemplatePrefix,
    setMonomerPrefix: () => setMonomerPrefix,
    setMonomerTemplatePrefix: () => setMonomerTemplatePrefix,
    switchIntoChemistryCoordSystem: () => switchIntoChemistryCoordSystem,
    unknownNaturalAnalogues: () => unknownNaturalAnalogues
  });

  // stub-assert:assert
  function assert(c, m) {
    if (!c) throw String(m || "Assertion failed");
  }
  var assert_default = assert;

  // src/core/chem/vec2.ts
  var import_utilities = __toESM(require_utilities());
  function toNumber(value) {
    return typeof value === "number" ? value : parseFloat(value);
  }
  var _Vec2 = class _Vec2 {
    constructor(...args) {
      __publicField(this, "x");
      __publicField(this, "y");
      __publicField(this, "z");
      if (args.length === 0) {
        this.x = 0;
        this.y = 0;
        this.z = 0;
      } else if (arguments.length === 1) {
        const point = args[0];
        this.x = toNumber(point.x || 0);
        this.y = toNumber(point.y || 0);
        this.z = toNumber(point.z || 0);
      } else if (arguments.length === 2) {
        this.x = toNumber(args[0] || 0);
        this.y = toNumber(args[1] || 0);
        this.z = 0;
      } else if (arguments.length === 3) {
        this.x = toNumber(args[0]);
        this.y = toNumber(args[1]);
        this.z = toNumber(args[2]);
      } else {
        throw String("Vec2(): invalid arguments");
      }
    }
    static dist(a, b) {
      return _Vec2.diff(a, b).length();
    }
    static max(v1, v2) {
      return new _Vec2(
        Math.max(v1.x, v2.x),
        Math.max(v1.y, v2.y),
        Math.max(v1.z, v2.z)
      );
    }
    static min(v1, v2) {
      return new _Vec2(
        Math.min(v1.x, v2.x),
        Math.min(v1.y, v2.y),
        Math.min(v1.z, v2.z)
      );
    }
    static sum(v1, v2) {
      return new _Vec2(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
    }
    static dot(v1, v2) {
      return v1.x * v2.x + v1.y * v2.y;
    }
    static cross(v1, v2) {
      return v1.x * v2.y - v1.y * v2.x;
    }
    static angle(v1, v2) {
      return Math.atan2(_Vec2.cross(v1, v2), _Vec2.dot(v1, v2));
    }
    static diff(v1, v2) {
      return new _Vec2(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }
    // assume arguments v1, f1, v2, f2, v3, f3, etc.
    // where v[i] are vectors and f[i] are corresponding coefficients
    static lc(...args) {
      let v = new _Vec2();
      for (let i = 0; i < arguments.length / 2; ++i) {
        v = v.addScaled(args[2 * i], args[2 * i + 1]);
      }
      return v;
    }
    static lc2(v1, f1, v2, f2) {
      return new _Vec2(
        v1.x * f1 + v2.x * f2,
        v1.y * f1 + v2.y * f2,
        v1.z * f1 + v2.z * f2
      );
    }
    static centre(v1, v2) {
      return _Vec2.lc2(v1, 0.5, v2, 0.5);
    }
    static getLinePoint(lineStart, lineEnd, length) {
      const difference = lineStart.sub(lineEnd);
      const distance = difference.length();
      const ratio = length / distance;
      return new _Vec2(
        lineStart.x + difference.x * ratio,
        lineStart.y + difference.y * ratio
      );
    }
    static crossProduct(v1, v2) {
      return v1.x * v2.y - v1.y * v2.x;
    }
    length() {
      return Math.sqrt(this.x * this.x + this.y * this.y);
    }
    equals(v) {
      return this.x === v.x && this.y === v.y && this.z === v.z;
    }
    add(v) {
      return new _Vec2(this.x + v.x, this.y + v.y, this.z + v.z);
    }
    add_(v) {
      this.x += v.x;
      this.y += v.y;
      this.z += v.z;
    }
    get_xy0() {
      return new _Vec2(this.x, this.y);
    }
    sub(v) {
      return new _Vec2(this.x - v.x, this.y - v.y, this.z - v.z);
    }
    scaled(sInitial) {
      const s = isFinite(sInitial) ? sInitial : 1;
      return new _Vec2(this.x * s, this.y * s, this.z * s);
    }
    negated() {
      return new _Vec2(-this.x, -this.y, -this.z);
    }
    yComplement(y1) {
      y1 = y1 || 0;
      return new _Vec2(this.x, y1 - this.y, this.z);
    }
    addScaled(v, f) {
      return new _Vec2(this.x + v.x * f, this.y + v.y * f, this.z + v.z * f);
    }
    normalized() {
      return this.scaled(1 / this.length());
    }
    normalize() {
      const l = this.length();
      if (l < 1e-6) return false;
      this.x /= l;
      this.y /= l;
      return true;
    }
    turnLeft() {
      return new _Vec2(-this.y, this.x, this.z);
    }
    coordStr() {
      return this.x.toString() + " , " + this.y.toString();
    }
    toString() {
      return "(" + this.x.toFixed(2) + "," + this.y.toFixed(2) + ")";
    }
    max(v) {
      assert_default(v != null);
      return _Vec2.max(this, v);
    }
    min(v) {
      return _Vec2.min(this, v);
    }
    ceil() {
      return new _Vec2(Math.ceil(this.x), Math.ceil(this.y), Math.ceil(this.z));
    }
    floor() {
      return new _Vec2(Math.floor(this.x), Math.floor(this.y), Math.floor(this.z));
    }
    rotate(angle) {
      const sin = Math.sin(angle);
      const cos = Math.cos(angle);
      return this.rotateSC(sin, cos);
    }
    rotateSC(sin, cos) {
      assert_default(sin === 0 || !!sin);
      assert_default(cos === 0 || !!cos);
      return new _Vec2(
        this.x * cos - this.y * sin,
        this.x * sin + this.y * cos,
        this.z
      );
    }
    rotateAroundOrigin(angleInDegrees, origin) {
      const angleInRadians = angleInDegrees * Math.PI / 180;
      const offsetX = this.x - origin.x;
      const offsetY = this.y - origin.y;
      const rotatedX = Math.cos(angleInRadians) * offsetX - Math.sin(angleInRadians) * offsetY;
      const rotatedY = Math.sin(angleInRadians) * offsetX + Math.cos(angleInRadians) * offsetY;
      const x = rotatedX + origin.x;
      const y = rotatedY + origin.y;
      return new _Vec2(Number((0, import_utilities.toFixed)(x)), Number((0, import_utilities.toFixed)(y)), this.z || 0);
    }
    isInsidePolygon(points) {
      const { x, y } = this;
      let inside = false;
      for (let i = 0, j = points.length - 1; i < points.length; j = i++) {
        const xi = points[i].x || 0;
        const yi = points[i].y || 0;
        const xj = points[j].x || 0;
        const yj = points[j].y || 0;
        const intersect = yi > y !== yj > y && x < (xj - xi) * (y - yi) / (yj - yi) + xi;
        if (intersect) inside = !inside;
      }
      return inside;
    }
    calculateDistanceToLine(line) {
      const lineVec = _Vec2.diff(line[1], line[0]);
      const pointVec = _Vec2.diff(this, line[0]);
      const lineLength = _Vec2.dist(line[0], line[1]);
      const lineUnitVec = lineVec.normalized();
      const projectionLength = _Vec2.dot(lineUnitVec, pointVec);
      const clampedProjectionLength = Math.max(
        0,
        Math.min(lineLength, projectionLength)
      );
      const closestPoint = _Vec2.sum(
        line[0],
        lineUnitVec.scaled(clampedProjectionLength)
      );
      return _Vec2.dist(closestPoint, this);
    }
    oxAngle() {
      return Math.atan2(this.y, this.x);
    }
    static radiansToDegrees(radians) {
      return radians * (180 / Math.PI);
    }
    static degrees_to_radians(degrees) {
      return degrees * Math.PI / 180;
    }
    static oxAngleForVector(v1, v2) {
      return Math.atan2(v2.y - v1.y, v2.x - v1.x);
    }
    static findSecondPoint(startPoint, lineLength, lineAngleRadians) {
      const cos = Math.cos(lineAngleRadians);
      const sin = Math.sin(lineAngleRadians);
      const deltaX = lineLength * cos;
      const deltaY = lineLength * sin;
      const endPoint = {
        x: startPoint.x + deltaX,
        y: startPoint.y + deltaY
      };
      return endPoint;
    }
  };
  __publicField(_Vec2, "ZERO", new _Vec2(0, 0));
  __publicField(_Vec2, "UNIT", new _Vec2(1, 1));
  var Vec2 = _Vec2;

  // domain/entities/DrawingEntity.ts
  var import_coordinates = __toESM(require_coordinates());
  var nextId = Math.floor(Math.random() * 1e6);
  var DrawingEntity = class {
    constructor(_position = new Vec2(0, 0), config = {
      generateId: true
    }) {
      this._position = _position;
      this.config = config;
      __publicField(this, "selected", false);
      __publicField(this, "hovered", false);
      __publicField(this, "id", 0);
      __publicField(this, "baseRenderer");
      var _a;
      this._position = _position || new Vec2(0, 0);
      if (((_a = this.config) == null ? void 0 : _a.generateId) === true) {
        this.id = nextId++;
      }
    }
    moveRelative(position) {
      this._position = new Vec2(
        this._position.x + position.x,
        this._position.y + position.y
      );
    }
    moveAbsolute(position) {
      this._position = position;
    }
    get position() {
      return this._position;
    }
    turnOnHover() {
      this.hovered = true;
    }
    turnOffHover() {
      this.hovered = false;
    }
    turnOnSelection() {
      this.selected = true;
    }
    turnOffSelection() {
      this.selected = false;
    }
    selectIfLocatedInRectangle(rectangleTopLeftPoint, rectangleBottomRightPoint, isPreviousSelected = false, shiftKey = false) {
      assert_default(this.baseRenderer);
      const prevSelectedValue = this.selected;
      const selectionPoints = this.baseRenderer.selectionPoints || [
        import_coordinates.Coordinates.modelToCanvas(this.center)
      ];
      let isSelected = false;
      selectionPoints.forEach((point) => {
        const locatedInRectangle = rectangleBottomRightPoint.x > point.x && rectangleBottomRightPoint.y > point.y && rectangleTopLeftPoint.x < point.x && rectangleTopLeftPoint.y < point.y;
        isSelected = isSelected || locatedInRectangle;
      });
      if (shiftKey) {
        isSelected = isPreviousSelected || isSelected;
      }
      if (isSelected) {
        this.turnOnSelection();
      } else {
        this.turnOffSelection();
      }
      return prevSelectedValue !== this.selected;
    }
    selectIfLocatedInPolygon(polygonPoints, isPreviousSelected = false, shiftKey = false) {
      assert_default(this.baseRenderer);
      const prevSelectedValue = this.selected;
      const selectionPoints = this.baseRenderer.selectionPoints || [
        import_coordinates.Coordinates.modelToCanvas(this.center)
      ];
      let isSelected = false;
      selectionPoints.forEach((point) => {
        const locatedInPolygon = this.isPointInPolygon(polygonPoints, point);
        isSelected = isSelected || locatedInPolygon;
      });
      if (shiftKey) {
        isSelected = isPreviousSelected || isSelected;
      }
      if (isSelected) {
        this.turnOnSelection();
      } else {
        this.turnOffSelection();
      }
      return prevSelectedValue !== this.selected;
    }
    isPointInPolygon(r, p) {
      const d = new Vec2(0, 1);
      const n = d.rotate(Math.PI / 2);
      let v0 = Vec2.diff(r[r.length - 1], p);
      let n0 = Vec2.dot(n, v0);
      let d0 = Vec2.dot(d, v0);
      let w0 = new Vec2(0, 0);
      let counter = 0;
      const eps = 1e-5;
      let flag1 = false;
      let flag0 = false;
      for (const point of r) {
        const v1 = Vec2.diff(point, p);
        const w1 = Vec2.diff(v1, v0);
        const n1 = Vec2.dot(n, v1);
        const d1 = Vec2.dot(d, v1);
        flag1 = false;
        if (n1 * n0 < 0) {
          if (d1 * d0 > -eps) {
            if (d0 > -eps) flag1 = true;
          } else if ((Math.abs(n0) * Math.abs(d1) - Math.abs(n1) * Math.abs(d0)) * d1 > 0) {
            flag1 = true;
          }
        }
        if (flag1 && flag0 && Vec2.dot(w1, n) * Vec2.dot(w0, n) >= 0) {
          flag1 = false;
        }
        if (flag1) {
          counter++;
        }
        v0 = v1;
        n0 = n1;
        d0 = d1;
        w0 = w1;
        flag0 = flag1;
      }
      return counter % 2 !== 0;
    }
    setBaseRenderer(renderer) {
      this.baseRenderer = renderer;
    }
  };

  // domain/entities/CoreBond.ts
  var Bond = class extends DrawingEntity {
    constructor(firstAtom, secondAtom, bondIdInMicroMode, type = 1 /* Single */, stereo = 0 /* None */, cip = null) {
      super(firstAtom.position);
      this.firstAtom = firstAtom;
      this.secondAtom = secondAtom;
      this.bondIdInMicroMode = bondIdInMicroMode;
      this.type = type;
      this.stereo = stereo;
      this.cip = cip;
      __publicField(this, "endPosition", new Vec2());
      __publicField(this, "renderer");
      this.endPosition = secondAtom.position;
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    get startPosition() {
      return this.position;
    }
    get center() {
      return Vec2.centre(this.startPosition, this.endPosition);
    }
    moveBondStartAbsolute(x, y) {
      this.moveAbsolute(new Vec2(x, y));
    }
    moveBondEndAbsolute(x, y) {
      this.endPosition = new Vec2(x, y);
    }
    moveToLinkedAtoms() {
      const firstAtomCenter = this.firstAtom.position;
      const secondAtomCenter = this.secondAtom.position;
      this.moveBondStartAbsolute(firstAtomCenter.x, firstAtomCenter.y);
      if (secondAtomCenter) {
        this.moveBondEndAbsolute(secondAtomCenter.x, secondAtomCenter.y);
      }
    }
    moveToLinkedEntities() {
      this.moveToLinkedAtoms();
    }
  };

  // domain/constants/elementColor.ts
  var ElementColor = {
    H: "#000000",
    He: "#89a1a1",
    Li: "#bd77ed",
    Be: "#8fbc00",
    B: "#c18989",
    C: "#000000",
    N: "#304ff7",
    O: "#ff0d0d",
    F: "#78bc42",
    Ne: "#80a2af",
    Na: "#ab5cf2",
    Mg: "#6fcd00",
    Al: "#a99393",
    Si: "#b29478",
    P: "#ff8000",
    S: "#c99a19",
    Cl: "#1fd01f",
    Ar: "#69acba",
    K: "#8f40d4",
    Ca: "#38e900",
    Sc: "#999999",
    Ti: "#979a9e",
    V: "#99999e",
    Cr: "#8a99c7",
    Mn: "#9c7ac7",
    Fe: "#e06633",
    Co: "#d37e8e",
    Ni: "#4ece4e",
    Cu: "#c78033",
    Zn: "#7d80b0",
    Ga: "#bc8b8b",
    Ge: "#668f8f",
    As: "#b87ddd",
    Se: "#e59100",
    Br: "#a62929",
    Kr: "#59b1c9",
    Rb: "#702eb0",
    Sr: "#00ff00",
    Y: "#66afaf",
    Zr: "#71abab",
    Nb: "#67aeb4",
    Mo: "#54b5b5",
    Tc: "#3b9e9e",
    Ru: "#248f8f",
    Rh: "#0a7d8c",
    Pd: "#006985",
    Ag: "#9a9a9a",
    Cd: "#b29764",
    In: "#a67573",
    Sn: "#668080",
    Sb: "#9e63b5",
    Te: "#d47a00",
    I: "#940094",
    Xe: "#429eb0",
    Cs: "#57178f",
    Ba: "#00c900",
    La: "#5caed1",
    Ce: "#9d9d7b",
    Pr: "#8ca581",
    Nd: "#84a984",
    Pm: "#71b18a",
    Sm: "#66b68e",
    Eu: "#4ac298",
    Gd: "#37cb9e",
    Tb: "#28d1a4",
    Dy: "#1bd7a8",
    Ho: "#00e98f",
    Er: "#00e675",
    Tm: "#00d452",
    Yb: "#00bf38",
    Lu: "#00ab24",
    Hf: "#47b3ec",
    Ta: "#4da6ff",
    W: "#2194d6",
    Re: "#267dab",
    Os: "#266696",
    Ir: "#175487",
    Pt: "#9898a3",
    Au: "#c19e1c",
    Hg: "#9797ac",
    Tl: "#a6544d",
    Pb: "#575961",
    Bi: "#9e4fb5",
    Po: "#ab5c00",
    At: "#754f45",
    Rn: "#428296",
    Fr: "#420066",
    Ra: "#007d00",
    Ac: "#6aa2ec",
    Th: "#00baff",
    Pa: "#00a1ff",
    U: "#008fff",
    Np: "#0080ff",
    Pu: "#006bff",
    Am: "#545cf2",
    Cm: "#785ce3",
    Bk: "#8a4fe3",
    Cf: "#a136d4",
    Es: "#b31fd4",
    // Need to fix colors for the elements below (c)
    Fm: "#000000",
    Md: "#000000",
    No: "#000000",
    Lr: "#000000",
    Rf: "#47b3ec",
    Db: "#4da6ff",
    Sg: "#2194d6",
    Bh: "#267dab",
    Hs: "#266696",
    Mt: "#175487",
    Ds: "#9898a3",
    Rg: "#c19e1c",
    Cn: "#9797ac",
    Nh: "#000000",
    Fl: "#000000",
    Mc: "#000000",
    Lv: "#000000",
    Ts: "#000000",
    Og: "#000000"
  };

  // domain/constants/elements.ts
  var elementsArray = [
    {
      number: 1,
      label: "H",
      period: 1,
      group: 1,
      title: "Hydrogen",
      state: "gas",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 1.00794
    },
    {
      number: 2,
      label: "He",
      period: 1,
      group: 8,
      title: "Helium",
      state: "gas",
      origin: "primordial",
      type: "noble",
      mass: 4.0026022
    },
    {
      number: 3,
      label: "Li",
      period: 2,
      group: 1,
      title: "Lithium",
      state: "solid",
      origin: "primordial",
      type: "alkali",
      mass: 6.94
    },
    {
      number: 4,
      label: "Be",
      period: 2,
      group: 2,
      title: "Beryllium",
      state: "solid",
      origin: "primordial",
      type: "alkaline-earth",
      mass: 9.01218315
    },
    {
      number: 5,
      label: "B",
      period: 2,
      group: 3,
      title: "Boron",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 10.81
    },
    {
      number: 6,
      label: "C",
      period: 2,
      group: 4,
      title: "Carbon",
      state: "solid",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 12.011
    },
    {
      number: 7,
      label: "N",
      period: 2,
      group: 5,
      title: "Nitrogen",
      state: "gas",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 14.007
    },
    {
      number: 8,
      label: "O",
      period: 2,
      group: 6,
      leftH: true,
      title: "Oxygen",
      state: "gas",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 15.999
    },
    {
      number: 9,
      label: "F",
      period: 2,
      group: 7,
      leftH: true,
      title: "Fluorine",
      state: "gas",
      origin: "primordial",
      type: "halogen",
      mass: 18.9984031636
    },
    {
      number: 10,
      label: "Ne",
      period: 2,
      group: 8,
      title: "Neon",
      state: "gas",
      origin: "primordial",
      type: "noble",
      mass: 20.17976
    },
    {
      number: 11,
      label: "Na",
      period: 3,
      group: 1,
      title: "Sodium",
      state: "solid",
      origin: "primordial",
      type: "alkali",
      mass: 22.989769282
    },
    {
      number: 12,
      label: "Mg",
      period: 3,
      group: 2,
      title: "Magnesium",
      state: "solid",
      origin: "primordial",
      type: "alkaline-earth",
      mass: 24.305
    },
    {
      number: 13,
      label: "Al",
      period: 3,
      group: 3,
      title: "Aluminium",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 26.98153857
    },
    {
      number: 14,
      label: "Si",
      period: 3,
      group: 4,
      title: "Silicon",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 28.085
    },
    {
      number: 15,
      label: "P",
      period: 3,
      group: 5,
      title: "Phosphorus",
      state: "solid",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 30.9737619985
    },
    {
      number: 16,
      label: "S",
      period: 3,
      group: 6,
      leftH: true,
      title: "Sulfur",
      state: "solid",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 32.06
    },
    {
      number: 17,
      label: "Cl",
      period: 3,
      group: 7,
      leftH: true,
      title: "Chlorine",
      state: "gas",
      origin: "primordial",
      type: "halogen",
      mass: 35.45
    },
    {
      number: 18,
      label: "Ar",
      period: 3,
      group: 8,
      title: "Argon",
      state: "gas",
      origin: "primordial",
      type: "noble",
      mass: 39.9481
    },
    {
      number: 19,
      label: "K",
      period: 4,
      group: 1,
      title: "Potassium",
      state: "solid",
      origin: "primordial",
      type: "alkali",
      mass: 39.09831
    },
    {
      number: 20,
      label: "Ca",
      period: 4,
      group: 2,
      title: "Calcium",
      state: "solid",
      origin: "primordial",
      type: "alkaline-earth",
      mass: 40.0784
    },
    {
      number: 21,
      label: "Sc",
      period: 4,
      group: 3,
      title: "Scandium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 44.9559085
    },
    {
      number: 22,
      label: "Ti",
      period: 4,
      group: 4,
      title: "Titanium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 47.8671
    },
    {
      number: 23,
      label: "V",
      period: 4,
      group: 5,
      title: "Vanadium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 50.94151
    },
    {
      number: 24,
      label: "Cr",
      period: 4,
      group: 6,
      title: "Chromium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 51.99616
    },
    {
      number: 25,
      label: "Mn",
      period: 4,
      group: 7,
      title: "Manganese",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 54.9380443
    },
    {
      number: 26,
      label: "Fe",
      period: 4,
      group: 8,
      title: "Iron",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 55.8452
    },
    {
      number: 27,
      label: "Co",
      period: 4,
      group: 8,
      title: "Cobalt",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 58.9331944
    },
    {
      number: 28,
      label: "Ni",
      period: 4,
      group: 8,
      title: "Nickel",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 58.69344
    },
    {
      number: 29,
      label: "Cu",
      period: 4,
      group: 1,
      title: "Copper",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 63.5463
    },
    {
      number: 30,
      label: "Zn",
      period: 4,
      group: 2,
      title: "Zinc",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 65.382
    },
    {
      number: 31,
      label: "Ga",
      period: 4,
      group: 3,
      title: "Gallium",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 69.7231
    },
    {
      number: 32,
      label: "Ge",
      period: 4,
      group: 4,
      title: "Germanium",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 72.6308
    },
    {
      number: 33,
      label: "As",
      period: 4,
      group: 5,
      title: "Arsenic",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 74.9215956
    },
    {
      number: 34,
      label: "Se",
      period: 4,
      group: 6,
      leftH: true,
      title: "Selenium",
      state: "solid",
      origin: "primordial",
      type: "other-nonmetal",
      mass: 78.9718
    },
    {
      number: 35,
      label: "Br",
      period: 4,
      group: 7,
      leftH: true,
      title: "Bromine",
      state: "liquid",
      origin: "primordial",
      type: "halogen",
      mass: 79.904
    },
    {
      number: 36,
      label: "Kr",
      period: 4,
      group: 8,
      title: "Krypton",
      state: "gas",
      origin: "primordial",
      type: "noble",
      mass: 83.7982
    },
    {
      number: 37,
      label: "Rb",
      period: 5,
      group: 1,
      title: "Rubidium",
      state: "solid",
      origin: "primordial",
      type: "alkali",
      mass: 85.46783
    },
    {
      number: 38,
      label: "Sr",
      period: 5,
      group: 2,
      title: "Strontium",
      state: "solid",
      origin: "primordial",
      type: "alkaline-earth",
      mass: 87.621
    },
    {
      number: 39,
      label: "Y",
      period: 5,
      group: 3,
      title: "Yttrium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 88.905842
    },
    {
      number: 40,
      label: "Zr",
      period: 5,
      group: 4,
      title: "Zirconium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 91.2242
    },
    {
      number: 41,
      label: "Nb",
      period: 5,
      group: 5,
      title: "Niobium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 92.906372
    },
    {
      number: 42,
      label: "Mo",
      period: 5,
      group: 6,
      title: "Molybdenum",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 95.951
    },
    {
      number: 43,
      label: "Tc",
      period: 5,
      group: 7,
      title: "Technetium",
      state: "solid",
      origin: "decay",
      type: "transition",
      mass: 98
    },
    {
      number: 44,
      label: "Ru",
      period: 5,
      group: 8,
      title: "Ruthenium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 101.072
    },
    {
      number: 45,
      label: "Rh",
      period: 5,
      group: 8,
      title: "Rhodium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 102.905502
    },
    {
      number: 46,
      label: "Pd",
      period: 5,
      group: 8,
      title: "Palladium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 106.421
    },
    {
      number: 47,
      label: "Ag",
      period: 5,
      group: 1,
      title: "Silver",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 107.86822
    },
    {
      number: 48,
      label: "Cd",
      period: 5,
      group: 2,
      title: "Cadmium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 112.4144
    },
    {
      number: 49,
      label: "In",
      // 49
      period: 5,
      group: 3,
      title: "Indium",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 114.8181
    },
    {
      number: 50,
      label: "Sn",
      period: 5,
      group: 4,
      title: "Tin",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 118.7107
    },
    {
      number: 51,
      label: "Sb",
      period: 5,
      group: 5,
      title: "Antimony",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 121.7601
    },
    {
      number: 52,
      label: "Te",
      period: 5,
      group: 6,
      title: "Tellurium",
      state: "solid",
      origin: "primordial",
      type: "metalloid",
      mass: 127.603
    },
    {
      number: 53,
      label: "I",
      period: 5,
      group: 7,
      leftH: true,
      title: "Iodine",
      state: "solid",
      origin: "primordial",
      type: "halogen",
      mass: 126.904473
    },
    {
      number: 54,
      label: "Xe",
      period: 5,
      group: 8,
      title: "Xenon",
      state: "gas",
      origin: "primordial",
      type: "noble",
      mass: 131.2936
    },
    {
      number: 55,
      label: "Cs",
      period: 6,
      group: 1,
      title: "Caesium",
      state: "solid",
      origin: "primordial",
      type: "alkali",
      mass: 132.905451966
    },
    {
      number: 56,
      label: "Ba",
      period: 6,
      group: 2,
      title: "Barium",
      state: "solid",
      origin: "primordial",
      type: "alkaline-earth",
      mass: 137.3277
    },
    {
      number: 57,
      label: "La",
      period: 6,
      group: 3,
      title: "Lanthanum",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 138.905477
    },
    {
      number: 58,
      label: "Ce",
      period: 6,
      group: 3,
      title: "Cerium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 140.1161
    },
    {
      number: 59,
      label: "Pr",
      period: 6,
      group: 3,
      title: "Praseodymium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 140.907662
    },
    {
      number: 60,
      label: "Nd",
      period: 6,
      group: 3,
      title: "Neodymium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 144.2423
    },
    {
      number: 61,
      label: "Pm",
      period: 6,
      group: 3,
      title: "Promethium",
      state: "solid",
      origin: "decay",
      type: "lanthanide",
      mass: 145
    },
    {
      number: 62,
      label: "Sm",
      period: 6,
      group: 3,
      title: "Samarium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 150.362
    },
    {
      number: 63,
      label: "Eu",
      period: 6,
      group: 3,
      title: "Europium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 151.9641
    },
    {
      number: 64,
      label: "Gd",
      period: 6,
      group: 3,
      title: "Gadolinium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 157.253
    },
    {
      number: 65,
      label: "Tb",
      period: 6,
      group: 3,
      title: "Terbium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 158.925352
    },
    {
      number: 66,
      label: "Dy",
      period: 6,
      group: 3,
      title: "Dysprosium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 162.5001
    },
    {
      number: 67,
      label: "Ho",
      period: 6,
      group: 3,
      title: "Holmium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 164.930332
    },
    {
      number: 68,
      label: "Er",
      period: 6,
      group: 3,
      title: "Erbium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 167.2593
    },
    {
      number: 69,
      label: "Tm",
      period: 6,
      group: 3,
      title: "Thulium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 168.934222
    },
    {
      number: 70,
      label: "Yb",
      period: 6,
      group: 3,
      title: "Ytterbium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 173.0451
    },
    {
      number: 71,
      label: "Lu",
      period: 6,
      group: 3,
      title: "Lutetium",
      state: "solid",
      origin: "primordial",
      type: "lanthanide",
      mass: 174.96681
    },
    {
      number: 72,
      label: "Hf",
      period: 6,
      group: 4,
      title: "Hafnium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 178.492
    },
    {
      number: 73,
      label: "Ta",
      period: 6,
      group: 5,
      title: "Tantalum",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 180.947882
    },
    {
      number: 74,
      label: "W",
      period: 6,
      group: 6,
      title: "Tungsten",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 183.841
    },
    {
      number: 75,
      label: "Re",
      period: 6,
      group: 7,
      title: "Rhenium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 186.2071
    },
    {
      number: 76,
      label: "Os",
      period: 6,
      group: 8,
      title: "Osmium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 190.233
    },
    {
      number: 77,
      label: "Ir",
      period: 6,
      group: 8,
      title: "Iridium",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 192.2173
    },
    {
      number: 78,
      label: "Pt",
      period: 6,
      group: 8,
      title: "Platinum",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 195.0849
    },
    {
      number: 79,
      label: "Au",
      period: 6,
      group: 1,
      title: "Gold",
      state: "solid",
      origin: "primordial",
      type: "transition",
      mass: 196.9665695
    },
    {
      number: 80,
      label: "Hg",
      period: 6,
      group: 2,
      title: "Mercury",
      state: "liquid",
      origin: "primordial",
      type: "transition",
      mass: 200.5923
    },
    {
      number: 81,
      label: "Tl",
      period: 6,
      group: 3,
      title: "Thallium",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 204.38
    },
    {
      number: 82,
      label: "Pb",
      period: 6,
      group: 4,
      title: "Lead",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 207.21
    },
    {
      number: 83,
      label: "Bi",
      period: 6,
      group: 5,
      title: "Bismuth",
      state: "solid",
      origin: "primordial",
      type: "post-transition",
      mass: 208.980401
    },
    {
      number: 84,
      label: "Po",
      period: 6,
      group: 6,
      title: "Polonium",
      state: "solid",
      origin: "decay",
      type: "metalloid",
      mass: 209
    },
    {
      number: 85,
      label: "At",
      period: 6,
      group: 7,
      title: "Astatine",
      state: "solid",
      origin: "decay",
      type: "halogen",
      mass: 210
    },
    {
      number: 86,
      label: "Rn",
      period: 6,
      group: 8,
      title: "Radon",
      state: "gas",
      origin: "decay",
      type: "noble",
      mass: 222
    },
    {
      number: 87,
      label: "Fr",
      period: 7,
      group: 1,
      title: "Francium",
      state: "solid",
      origin: "decay",
      type: "alkali",
      mass: 223
    },
    {
      number: 88,
      label: "Ra",
      period: 7,
      group: 2,
      title: "Radium",
      state: "solid",
      origin: "decay",
      type: "alkaline-earth",
      mass: 226
    },
    {
      number: 89,
      label: "Ac",
      period: 7,
      group: 3,
      title: "Actinium",
      state: "solid",
      origin: "decay",
      type: "transition",
      mass: 227
    },
    {
      number: 90,
      label: "Th",
      period: 7,
      group: 3,
      title: "Thorium",
      state: "solid",
      origin: "primordial",
      type: "actinide",
      mass: 232.03774
    },
    {
      number: 91,
      label: "Pa",
      period: 7,
      group: 3,
      title: "Protactinium",
      state: "solid",
      origin: "decay",
      type: "actinide",
      mass: 231.035882
    },
    {
      number: 92,
      label: "U",
      period: 7,
      group: 3,
      title: "Uranium",
      state: "solid",
      origin: "primordial",
      type: "actinide",
      mass: 238.028913
    },
    {
      number: 93,
      label: "Np",
      period: 7,
      group: 3,
      title: "Neptunium",
      state: "solid",
      origin: "decay",
      type: "actinide",
      mass: 237
    },
    {
      number: 94,
      label: "Pu",
      period: 7,
      group: 3,
      title: "Plutonium",
      state: "solid",
      origin: "decay",
      type: "actinide",
      mass: 244
    },
    {
      number: 95,
      label: "Am",
      period: 7,
      group: 3,
      title: "Americium",
      state: "solid",
      origin: "synthetic",
      type: "actinide",
      mass: 243
    },
    {
      number: 96,
      label: "Cm",
      period: 7,
      group: 3,
      title: "Curium",
      state: "solid",
      origin: "synthetic",
      type: "actinide",
      mass: 247
    },
    {
      number: 97,
      label: "Bk",
      period: 7,
      group: 3,
      title: "Berkelium",
      state: "solid",
      origin: "synthetic",
      type: "actinide",
      mass: 247
    },
    {
      number: 98,
      label: "Cf",
      period: 7,
      group: 3,
      title: "Californium",
      state: "solid",
      origin: "synthetic",
      type: "actinide",
      mass: 251
    },
    {
      number: 99,
      label: "Es",
      period: 7,
      group: 3,
      title: "Einsteinium",
      state: "solid",
      origin: "synthetic",
      type: "actinide",
      mass: 252
    },
    {
      number: 100,
      label: "Fm",
      period: 7,
      group: 3,
      title: "Fermium",
      origin: "synthetic",
      type: "actinide",
      mass: 257
    },
    {
      number: 101,
      label: "Md",
      period: 7,
      group: 3,
      title: "Mendelevium",
      origin: "synthetic",
      type: "actinide",
      mass: 258
    },
    {
      number: 102,
      label: "No",
      period: 7,
      group: 3,
      title: "Nobelium",
      origin: "synthetic",
      type: "actinide",
      mass: 259
    },
    {
      number: 103,
      label: "Lr",
      period: 7,
      group: 3,
      title: "Lawrencium",
      origin: "synthetic",
      type: "actinide",
      mass: 266
    },
    {
      number: 104,
      label: "Rf",
      period: 7,
      group: 4,
      title: "Rutherfordium",
      origin: "synthetic",
      type: "transition",
      mass: 267
    },
    {
      number: 105,
      label: "Db",
      period: 7,
      group: 5,
      title: "Dubnium",
      origin: "synthetic",
      type: "transition",
      mass: 268
    },
    {
      number: 106,
      label: "Sg",
      period: 7,
      group: 6,
      title: "Seaborgium",
      origin: "synthetic",
      type: "transition",
      mass: 269
    },
    {
      number: 107,
      label: "Bh",
      period: 7,
      group: 7,
      title: "Bohrium",
      origin: "synthetic",
      type: "transition",
      mass: 270
    },
    {
      number: 108,
      label: "Hs",
      period: 7,
      group: 8,
      title: "Hassium",
      origin: "synthetic",
      type: "transition",
      mass: 269
    },
    {
      number: 109,
      label: "Mt",
      period: 7,
      group: 8,
      title: "Meitnerium",
      origin: "synthetic",
      type: "transition",
      mass: 278
    },
    {
      number: 110,
      label: "Ds",
      period: 7,
      group: 8,
      title: "Darmstadtium",
      origin: "synthetic",
      type: "transition",
      mass: 281
    },
    {
      number: 111,
      label: "Rg",
      period: 7,
      group: 1,
      title: "Roentgenium",
      origin: "synthetic",
      type: "transition",
      mass: 282
    },
    {
      number: 112,
      label: "Cn",
      period: 7,
      group: 2,
      title: "Copernicium",
      origin: "synthetic",
      type: "transition",
      mass: 285
    },
    {
      number: 113,
      label: "Nh",
      period: 7,
      group: 3,
      title: "Nihonium",
      origin: "synthetic",
      type: "post-transition",
      mass: 286
    },
    {
      number: 114,
      label: "Fl",
      period: 7,
      group: 4,
      title: "Flerovium",
      origin: "synthetic",
      type: "post-transition",
      mass: 289
    },
    {
      number: 115,
      label: "Mc",
      period: 7,
      group: 5,
      title: "Moscovium",
      origin: "synthetic",
      type: "post-transition",
      mass: 289
    },
    {
      number: 116,
      label: "Lv",
      period: 7,
      group: 6,
      title: "Livermorium",
      origin: "synthetic",
      type: "post-transition",
      mass: 293
    },
    {
      number: 117,
      label: "Ts",
      period: 7,
      group: 7,
      title: "Tennessine",
      origin: "synthetic",
      type: "halogen",
      mass: 294
    },
    {
      number: 118,
      label: "Og",
      period: 7,
      group: 8,
      title: "Oganesson",
      origin: "synthetic",
      type: "noble",
      mass: 294
    }
  ];
  var elementsMap = elementsArray.reduce((acc, element) => {
    acc.set(element.label, element);
    acc.set(element.number, element);
    return acc;
  }, /* @__PURE__ */ new Map());
  var Elements = {
    get: (key) => elementsMap.get(key),
    filter: (predicate) => {
      return elementsArray.filter(predicate);
    },
    getAll: () => [...elementsArray]
  };

  // domain/constants/element.types.ts
  var AtomLabel = /* @__PURE__ */ ((AtomLabel3) => {
    AtomLabel3["Ac"] = "Ac";
    AtomLabel3["Ag"] = "Ag";
    AtomLabel3["Al"] = "Al";
    AtomLabel3["Am"] = "Am";
    AtomLabel3["Ar"] = "Ar";
    AtomLabel3["As"] = "As";
    AtomLabel3["At"] = "At";
    AtomLabel3["Au"] = "Au";
    AtomLabel3["B"] = "B";
    AtomLabel3["Ba"] = "Ba";
    AtomLabel3["Be"] = "Be";
    AtomLabel3["Bh"] = "Bh";
    AtomLabel3["Bi"] = "Bi";
    AtomLabel3["Bk"] = "Bk";
    AtomLabel3["Br"] = "Br";
    AtomLabel3["C"] = "C";
    AtomLabel3["Ca"] = "Ca";
    AtomLabel3["Cd"] = "Cd";
    AtomLabel3["Ce"] = "Ce";
    AtomLabel3["Cf"] = "Cf";
    AtomLabel3["Cl"] = "Cl";
    AtomLabel3["Cm"] = "Cm";
    AtomLabel3["Cn"] = "Cn";
    AtomLabel3["Co"] = "Co";
    AtomLabel3["Cr"] = "Cr";
    AtomLabel3["Cs"] = "Cs";
    AtomLabel3["Cu"] = "Cu";
    AtomLabel3["Db"] = "Db";
    AtomLabel3["Ds"] = "Ds";
    AtomLabel3["Dy"] = "Dy";
    AtomLabel3["Er"] = "Er";
    AtomLabel3["Es"] = "Es";
    AtomLabel3["Eu"] = "Eu";
    AtomLabel3["F"] = "F";
    AtomLabel3["Fe"] = "Fe";
    AtomLabel3["Fl"] = "Fl";
    AtomLabel3["Fm"] = "Fm";
    AtomLabel3["Fr"] = "Fr";
    AtomLabel3["Ga"] = "Ga";
    AtomLabel3["Gd"] = "Gd";
    AtomLabel3["Ge"] = "Ge";
    AtomLabel3["H"] = "H";
    AtomLabel3["He"] = "He";
    AtomLabel3["Hf"] = "Hf";
    AtomLabel3["Hg"] = "Hg";
    AtomLabel3["Ho"] = "Ho";
    AtomLabel3["Hs"] = "Hs";
    AtomLabel3["I"] = "I";
    AtomLabel3["In"] = "In";
    AtomLabel3["Ir"] = "Ir";
    AtomLabel3["K"] = "K";
    AtomLabel3["Kr"] = "Kr";
    AtomLabel3["La"] = "La";
    AtomLabel3["Li"] = "Li";
    AtomLabel3["Lr"] = "Lr";
    AtomLabel3["Lu"] = "Lu";
    AtomLabel3["Lv"] = "Lv";
    AtomLabel3["Mc"] = "Mc";
    AtomLabel3["Md"] = "Md";
    AtomLabel3["Mg"] = "Mg";
    AtomLabel3["Mn"] = "Mn";
    AtomLabel3["Mo"] = "Mo";
    AtomLabel3["Mt"] = "Mt";
    AtomLabel3["N"] = "N";
    AtomLabel3["Na"] = "Na";
    AtomLabel3["Nb"] = "Nb";
    AtomLabel3["Nd"] = "Nd";
    AtomLabel3["Ne"] = "Ne";
    AtomLabel3["Nh"] = "Nh";
    AtomLabel3["Ni"] = "Ni";
    AtomLabel3["No"] = "No";
    AtomLabel3["Np"] = "Np";
    AtomLabel3["O"] = "O";
    AtomLabel3["Og"] = "Og";
    AtomLabel3["Os"] = "Os";
    AtomLabel3["P"] = "P";
    AtomLabel3["Pa"] = "Pa";
    AtomLabel3["Pb"] = "Pb";
    AtomLabel3["Pd"] = "Pd";
    AtomLabel3["Pm"] = "Pm";
    AtomLabel3["Po"] = "Po";
    AtomLabel3["Pr"] = "Pr";
    AtomLabel3["Pt"] = "Pt";
    AtomLabel3["Pu"] = "Pu";
    AtomLabel3["Ra"] = "Ra";
    AtomLabel3["Rb"] = "Rb";
    AtomLabel3["Re"] = "Re";
    AtomLabel3["Rf"] = "Rf";
    AtomLabel3["Rg"] = "Rg";
    AtomLabel3["Rh"] = "Rh";
    AtomLabel3["Rn"] = "Rn";
    AtomLabel3["Ru"] = "Ru";
    AtomLabel3["S"] = "S";
    AtomLabel3["Sb"] = "Sb";
    AtomLabel3["Sc"] = "Sc";
    AtomLabel3["Se"] = "Se";
    AtomLabel3["Sg"] = "Sg";
    AtomLabel3["Si"] = "Si";
    AtomLabel3["Sm"] = "Sm";
    AtomLabel3["Sn"] = "Sn";
    AtomLabel3["Sr"] = "Sr";
    AtomLabel3["Ta"] = "Ta";
    AtomLabel3["Tb"] = "Tb";
    AtomLabel3["Tc"] = "Tc";
    AtomLabel3["Te"] = "Te";
    AtomLabel3["Th"] = "Th";
    AtomLabel3["Ti"] = "Ti";
    AtomLabel3["Tl"] = "Tl";
    AtomLabel3["Tm"] = "Tm";
    AtomLabel3["Ts"] = "Ts";
    AtomLabel3["U"] = "U";
    AtomLabel3["V"] = "V";
    AtomLabel3["W"] = "W";
    AtomLabel3["Xe"] = "Xe";
    AtomLabel3["Y"] = "Y";
    AtomLabel3["Yb"] = "Yb";
    AtomLabel3["Zn"] = "Zn";
    AtomLabel3["Zr"] = "Zr";
    AtomLabel3["D"] = "D";
    AtomLabel3["T"] = "T";
    return AtomLabel3;
  })(AtomLabel || {});

  // domain/constants/generics.ts
  var Generics = {
    "atoms-gen": {
      title: "Atom Generics",
      itemSets: [
        {
          displayName: "any atom",
          items: [
            { label: "A", description: "Any atom except hydrogen" },
            { label: "AH", description: "Any atom, including hydrogen" }
          ]
        },
        {
          displayName: "except C or H",
          items: [
            {
              label: "Q",
              description: "Any heteroatom (any atom except C or H)"
            },
            { label: "QH", description: "Any atom except C" }
          ]
        },
        {
          displayName: "any metal",
          items: [
            { label: "M", description: "Any metal" },
            { label: "MH", description: "Any metal or hydrogen" }
          ]
        },
        {
          displayName: "any halogen",
          items: [
            { label: "X", description: "Any halogen" },
            { label: "XH", description: "Any halogen or hydrogen" }
          ]
        }
      ]
    },
    "special-nodes": {
      title: "Special Nodes",
      itemSets: [
        {
          items: [
            { label: "H+", description: "Proton" },
            { label: "D", description: "Deuterium" },
            { label: "T", description: "Tritium" },
            { label: "R", description: "Pseudoatom" },
            { label: "Pol", description: "Polymer Bead" },
            { label: "*", description: "Any atom, including hydrogen" }
          ]
        }
      ]
    },
    "group-gen": {
      title: "Group Generics",
      itemSets: [
        {
          items: [
            {
              label: "G",
              description: "Any group"
            },
            {
              label: "GH",
              description: "Any group or hydrogen"
            }
          ]
        },
        {
          items: [
            {
              label: "G*",
              description: "Any group with a ring closure"
            },
            {
              label: "GH*",
              description: "Any group with a ring closure or hydrogen"
            }
          ]
        }
      ],
      subGroups: {
        "group-acyclic": {
          title: "Acyclic",
          itemSets: [
            {
              items: [
                { label: "ACY", description: "Acyclic group" },
                { label: "ACH", description: "Acyclic group or hydrogen" }
              ]
            }
          ],
          subGroups: {
            "acyclic-carbo": {
              title: "Acyclic Carbo",
              itemSets: [
                {
                  items: [
                    { label: "ABC", description: "Carbocyclic" },
                    { label: "ABH", description: "Carbocyclic of hydrogen" }
                  ]
                },
                {
                  displayName: "alkynyl",
                  items: [
                    { label: "AYL", description: "Alkynyl" },
                    { label: "AYH", description: "Alkynyl or hydrogen" }
                  ]
                },
                {
                  displayName: "alkyl",
                  items: [
                    { label: "ALK", description: "Alkyl" },
                    { label: "ALH", description: "Alkyl or hydrogen" }
                  ]
                },
                {
                  displayName: "alkenyl",
                  items: [
                    { label: "AEL", description: "Alkenyl" },
                    { label: "AEH", description: "Alkenyl or hydrogen" }
                  ]
                }
              ]
            },
            "acyclic-hetero": {
              title: "Acyclic Hetero",
              itemSets: [
                {
                  items: [
                    { label: "AHC", description: "Heteroacyclic" },
                    { label: "AHH", description: "Heterocyclic or hydrogen" }
                  ]
                },
                {
                  items: [
                    { label: "AOX", description: "Alkoxy" },
                    { label: "AOH", description: "Alkoxy or hydrogen" }
                  ]
                }
              ]
            }
          }
        },
        "group-cyclic": {
          title: "Cyclic",
          itemSets: [
            {
              items: [
                { label: "CYC", description: "Cyclic group" },
                { label: "CYH", description: "Cyclic group or hydrogen" }
              ]
            },
            {
              displayName: "no carbon",
              items: [
                {
                  label: "CXX",
                  description: "Cyclic group with no Carbon atoms"
                },
                {
                  label: "CXH",
                  description: "Cyclic group with no Carbon atoms or hydrogen"
                }
              ]
            }
          ],
          subGroups: {
            "cyclic-carbo": {
              title: "Cyclic Carbo",
              itemSets: [
                {
                  items: [
                    { label: "CBC", description: "Carbocyclic derivatives" },
                    {
                      label: "CBH",
                      description: "Carbocyclic derivatives or hydrogen"
                    }
                  ]
                },
                {
                  displayName: "aryl",
                  items: [
                    { label: "ARY", description: "Any aryl group" },
                    { label: "ARH", description: "Any aryl group or hydrogen" }
                  ]
                },
                {
                  displayName: "cycloalkyl",
                  items: [
                    { label: "CAL", description: "Any cycloalkyl group" },
                    {
                      label: "CAH",
                      description: "Any cycloalkyl group or hydrogen"
                    }
                  ]
                },
                {
                  displayName: "cycloalkenyl",
                  items: [
                    { label: "CEL", description: "Any cyloalkenyl group" },
                    {
                      label: "CEH",
                      description: "Any cyloalkenyl group or hydrogen"
                    }
                  ]
                }
              ]
            },
            "cyclic-hetero": {
              title: "Cyclic Hetero",
              itemSets: [
                {
                  items: [
                    { label: "CHC", description: "Heterocyclic group" },
                    {
                      label: "CHH",
                      description: "Heterocyclic group or hydrogen"
                    }
                  ]
                },
                {
                  displayName: "hetero aryl",
                  items: [
                    { label: "HAR", description: "Heteroaryl group" },
                    { label: "HAH", description: "Heteroaryl group or hydrogen" }
                  ]
                }
              ]
            }
          }
        }
      }
    }
  };
  function getGenericsList(generics) {
    var _a;
    if (Array.isArray(generics) && !((_a = generics[0]) == null ? void 0 : _a.items)) {
      return generics.map((item) => item.label);
    } else {
      let result = [];
      for (const subGroup of Object.values(generics)) {
        if (typeof generics === "string") continue;
        result = [...result, ...getGenericsList(subGroup)];
      }
      return result;
    }
  }
  var genericsList = getGenericsList(Generics);

  // domain/constants/image.ts
  var IMAGE_KEY = "images";
  var IMAGE_SERIALIZE_KEY = "image";
  var CURSOR_DIAGONAL_NWSE = "nwse-resize";
  var CURSOR_DIAGONAL_NESW = "nesw-resize";
  var CURSOR_VERTICAL = "ns-resize";
  var CURSOR_HORIZONTAL = "ew-resize";
  var imageReferencePositionToCursor = {
    topLeftPosition: CURSOR_DIAGONAL_NWSE,
    topMiddlePosition: CURSOR_VERTICAL,
    topRightPosition: CURSOR_DIAGONAL_NESW,
    rightMiddlePosition: CURSOR_HORIZONTAL,
    bottomRightPosition: CURSOR_DIAGONAL_NWSE,
    bottomMiddlePosition: CURSOR_VERTICAL,
    bottomLeftPosition: CURSOR_DIAGONAL_NESW,
    leftMiddlePosition: CURSOR_HORIZONTAL
  };

  // domain/constants/multitailArrow.ts
  var MULTITAIL_ARROW_KEY = "multitailArrows";
  var MULTITAIL_ARROW_TOOL_NAME = "reaction-arrow-multitail";
  var MULTITAIL_ARROW_SERIALIZE_KEY = "multi-tailed-arrow";
  var MOVE = "move";
  var CURSOR_RESIZE_VERTICAL = "ns-resize";
  var CURSOR_RESIZE_HORIZONTAL = "ew-resize";
  var multitailReferencePositionToCursor = {
    topTail: CURSOR_RESIZE_HORIZONTAL,
    tails: CURSOR_RESIZE_HORIZONTAL,
    bottomTail: CURSOR_RESIZE_HORIZONTAL,
    topSpine: MOVE,
    bottomSpine: MOVE,
    head: CURSOR_RESIZE_HORIZONTAL
  };
  var multitailArrowReferenceLinesToCursor = {
    topTail: CURSOR_RESIZE_VERTICAL,
    bottomTail: CURSOR_RESIZE_VERTICAL,
    tails: CURSOR_RESIZE_VERTICAL,
    head: CURSOR_RESIZE_VERTICAL,
    spine: MOVE
  };

  // domain/constants/chains.ts
  var STRAND_TYPE = /* @__PURE__ */ ((STRAND_TYPE2) => {
    STRAND_TYPE2["SENSE"] = "sense";
    STRAND_TYPE2["ANTISENSE"] = "antisense";
    return STRAND_TYPE2;
  })(STRAND_TYPE || {});

  // domain/constants/monomers.ts
  var RNA_DNA_NON_MODIFIED_PART = /* @__PURE__ */ ((RNA_DNA_NON_MODIFIED_PART2) => {
    RNA_DNA_NON_MODIFIED_PART2["SUGAR_RNA"] = "R";
    RNA_DNA_NON_MODIFIED_PART2["SUGAR_DNA"] = "dR";
    RNA_DNA_NON_MODIFIED_PART2["PHOSPHATE"] = "P";
    return RNA_DNA_NON_MODIFIED_PART2;
  })(RNA_DNA_NON_MODIFIED_PART || {});
  var RnaDnaNaturalAnaloguesEnum = /* @__PURE__ */ ((RnaDnaNaturalAnaloguesEnum2) => {
    RnaDnaNaturalAnaloguesEnum2["ADENINE"] = "A";
    RnaDnaNaturalAnaloguesEnum2["THYMINE"] = "T";
    RnaDnaNaturalAnaloguesEnum2["GUANINE"] = "G";
    RnaDnaNaturalAnaloguesEnum2["CYTOSINE"] = "C";
    RnaDnaNaturalAnaloguesEnum2["URACIL"] = "U";
    return RnaDnaNaturalAnaloguesEnum2;
  })(RnaDnaNaturalAnaloguesEnum || {});
  var RnaDnaBaseNames = /* @__PURE__ */ ((RnaDnaBaseNames2) => {
    RnaDnaBaseNames2["URACIL"] = "Uracil";
    RnaDnaBaseNames2["THYMINE"] = "Thymine";
    return RnaDnaBaseNames2;
  })(RnaDnaBaseNames || {});
  var StandardAmbiguousRnaBase = /* @__PURE__ */ ((StandardAmbiguousRnaBase2) => {
    StandardAmbiguousRnaBase2["N"] = "N";
    StandardAmbiguousRnaBase2["B"] = "B";
    StandardAmbiguousRnaBase2["V"] = "V";
    StandardAmbiguousRnaBase2["D"] = "D";
    StandardAmbiguousRnaBase2["H"] = "H";
    StandardAmbiguousRnaBase2["K"] = "K";
    StandardAmbiguousRnaBase2["M"] = "M";
    StandardAmbiguousRnaBase2["W"] = "W";
    StandardAmbiguousRnaBase2["Y"] = "Y";
    StandardAmbiguousRnaBase2["R"] = "R";
    StandardAmbiguousRnaBase2["S"] = "S";
    return StandardAmbiguousRnaBase2;
  })(StandardAmbiguousRnaBase || {});
  var StandardAmbiguousPeptide = /* @__PURE__ */ ((StandardAmbiguousPeptide2) => {
    StandardAmbiguousPeptide2["B"] = "B";
    StandardAmbiguousPeptide2["J"] = "J";
    StandardAmbiguousPeptide2["Z"] = "Z";
    StandardAmbiguousPeptide2["X"] = "X";
    return StandardAmbiguousPeptide2;
  })(StandardAmbiguousPeptide || {});
  var rnaDnaNaturalAnalogues = [
    "A" /* ADENINE */,
    "T" /* THYMINE */,
    "G" /* GUANINE */,
    "C" /* CYTOSINE */,
    "U" /* URACIL */
  ];
  var rnaDnaAmbiguousSymbols = [
    "N" /* N */,
    "B" /* B */,
    "V" /* V */,
    "D" /* D */,
    "H" /* H */,
    "K" /* K */,
    "M" /* M */,
    "W" /* W */,
    "Y" /* Y */,
    "R" /* R */,
    "S" /* S */
  ];
  var peptideAmbiguousSymbols = [
    "B" /* B */,
    "J" /* J */,
    "Z" /* Z */,
    "X" /* X */
  ];
  var unknownNaturalAnalogues = [".", "X"];
  var peptideNaturalAnalogues = [
    "A",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "V",
    "U",
    "W",
    "Y"
  ];
  var NO_NATURAL_ANALOGUE = "X";
  var MONOMER_CONST = {
    AMINO_ACID: "AminoAcid",
    PEPTIDE: "PEPTIDE",
    CHEM: "CHEM",
    RNA: "RNA",
    DNA: "DNA",
    MODDNA: "MODDNA",
    R: "R",
    // states for Ribose
    P: "P",
    // states for Phosphate
    SUGAR: "SUGAR",
    BASE: "BASE",
    PHOSPHATE: "PHOSPHATE"
  };
  var CREATE_MONOMER_TOOL_NAME = "create-monomer";
  var MonomerSize = 0.75;
  var HalfMonomerSize = MonomerSize / 2;
  var StandardBondLength = MonomerSize * 2;

  // domain/constants/layout.ts
  var SnakeLayoutCellWidth = 60;

  // domain/constants/sgroups.ts
  var SgContexts = {
    Fragment: "Fragment",
    Multifragment: "Multifragment",
    Bond: "Bond",
    Atom: "Atom",
    Group: "Group"
  };

  // domain/entities/CoreAtom.ts
  var import_lodash = __toESM(require_lodash2());

  // domain/entities/BaseBond.ts
  var BaseBond = class extends DrawingEntity {
    constructor() {
      super(...arguments);
      __publicField(this, "endPosition", new Vec2());
      __publicField(this, "_isOverlappedByMonomer", false);
    }
    get finished() {
      return Boolean(this.firstEndEntity && this.secondEndEntity);
    }
    get center() {
      return Vec2.centre(this.startPosition, this.endPosition);
    }
    moveToLinkedEntities() {
      var _a;
      const firstMonomerCenter = this.firstEndEntity.position;
      const secondMonomerCenter = (_a = this.secondEndEntity) == null ? void 0 : _a.position;
      this.moveBondStartAbsolute(firstMonomerCenter.x, firstMonomerCenter.y);
      if (secondMonomerCenter) {
        this.moveBondEndAbsolute(secondMonomerCenter.x, secondMonomerCenter.y);
      }
    }
    moveBondStartAbsolute(x, y) {
      this.moveAbsolute(new Vec2(x, y));
    }
    moveBondEndAbsolute(x, y) {
      this.endPosition = new Vec2(x, y);
    }
    get startPosition() {
      return this.position;
    }
    getAnotherEntity(monomer) {
      return this.firstEndEntity === monomer ? this.secondEndEntity : this.firstEndEntity;
    }
    get isOverlappedByMonomer() {
      return this._isOverlappedByMonomer;
    }
    set isOverlappedByMonomer(value) {
      this._isOverlappedByMonomer = value;
    }
  };

  // domain/entities/MonomerToAtomBond.ts
  var MonomerToAtomBond = class extends BaseBond {
    constructor(monomer, atom) {
      super();
      this.monomer = monomer;
      this.atom = atom;
      __publicField(this, "renderer");
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    get firstEndEntity() {
      return this.monomer;
    }
    get secondEndEntity() {
      return this.atom;
    }
    get isHorizontal() {
      return false;
    }
    get isVertical() {
      return false;
    }
  };

  // domain/entities/CoreAtom.ts
  var Atom = class extends DrawingEntity {
    constructor(position, monomer, atomIdInMicroMode, label, properties = {}) {
      super(position);
      this.monomer = monomer;
      this.atomIdInMicroMode = atomIdInMicroMode;
      this.label = label;
      this.properties = properties;
      __publicField(this, "bonds", []);
      __publicField(this, "renderer");
    }
    get center() {
      return this.position;
    }
    addBond(bond) {
      if (!this.bonds.includes(bond)) {
        this.bonds.push(bond);
      }
    }
    deleteBond(bondId) {
      this.bonds = this.bonds.filter((bond) => bond.id !== bondId);
    }
    setRenderer(renderer) {
      this.renderer = renderer;
      super.setBaseRenderer(renderer);
    }
    get isCarbon() {
      return this.label === "C" /* C */;
    }
    calculateConnections() {
      let connectionsAmount = 0;
      for (const bond of this.bonds) {
        if (bond instanceof MonomerToAtomBond) {
          connectionsAmount += 1;
        } else {
          switch (bond.type) {
            case 1 /* Single */:
              connectionsAmount += 1;
              break;
            case 2 /* Double */:
              connectionsAmount += 2;
              break;
            case 3 /* Triple */:
              connectionsAmount += 3;
              break;
            case 9 /* Dative */:
            case 10 /* Hydrogen */:
              break;
            case 4 /* Aromatic */:
              if (this.bonds.length === 1) {
                return -1;
              }
              return this.bonds.length;
            default:
              return -1;
          }
        }
      }
      return connectionsAmount;
    }
    get hasAlias() {
      return Boolean(this.properties.alias);
    }
    get hasRadical() {
      return (0, import_lodash.isNumber)(this.properties.radical) && this.properties.radical !== 0;
    }
    get hasCharge() {
      return (0, import_lodash.isNumber)(this.properties.charge) && this.properties.charge !== 0;
    }
    get hasExplicitValence() {
      return (0, import_lodash.isNumber)(this.properties.explicitValence) && this.properties.explicitValence !== -1;
    }
    get hasExplicitIsotope() {
      return (0, import_lodash.isNumber)(this.properties.isotope) && this.properties.isotope >= 0;
    }
    get hasBadValence() {
      const { hydrogenAmount } = this.calculateValence();
      return hydrogenAmount < 0;
    }
    get hasStereoLabel() {
      return Boolean(this.properties.stereoLabel);
    }
    get radicalAmount() {
      switch (this.properties.radical) {
        case 1 /* Single */:
        case 3 /* Triplet */:
          return 2;
        case 2 /* Doublet */:
          return 1;
        default:
          return 0;
      }
    }
    get valenceWithoutHydrogen() {
      var _a;
      const charge = (_a = this.properties.charge) != null ? _a : 0;
      const label = this.label;
      const element = Elements.get(this.label);
      const elementGroupNumber = element == null ? void 0 : element.group;
      const radicalAmount = this.radicalAmount;
      const connectionAmount = this.calculateConnections();
      const absoluteCharge = Math.abs(charge);
      if (elementGroupNumber === 3) {
        if (label === "B" /* B */ || label === "Al" /* Al */ || label === "Ga" /* Ga */ || label === "In" /* In */) {
          if (charge === -1) {
            if (radicalAmount + connectionAmount <= 4) {
              return radicalAmount + connectionAmount;
            }
          }
        }
      } else if (elementGroupNumber === 5) {
        if (label === "N" /* N */ || label === "P" /* P */ || label === "Sb" /* Sb */ || label === "Bi" /* Bi */ || label === "As" /* As */) {
          if (charge === 1 || charge === 2) {
            return radicalAmount + connectionAmount;
          }
        }
      } else if (elementGroupNumber === 6) {
        if (label === "O" /* O */) {
          if (charge >= 1) {
            return radicalAmount + connectionAmount;
          }
        } else if (label === "S" /* S */ || label === "Se" /* Se */ || label === "Po" /* Po */) {
          if (charge === 1) {
            return radicalAmount + connectionAmount;
          }
        }
      } else if (elementGroupNumber === 7) {
        if (label === "Cl" /* Cl */ || label === "Br" /* Br */ || label === "I" /* I */ || label === "At" /* At */) {
          if (charge === 1) {
            return radicalAmount + connectionAmount;
          }
        }
      }
      return radicalAmount + connectionAmount + absoluteCharge;
    }
    calculateValence() {
      var _a;
      if (this.hasExplicitValence) {
        const valence2 = this.properties.explicitValence;
        const hydrogenAmount2 = valence2 - this.valenceWithoutHydrogen;
        return {
          valence: valence2,
          hydrogenAmount: hydrogenAmount2
        };
      }
      const label = this.label;
      const element = Elements.get(label);
      const elementGroupNumber = element == null ? void 0 : element.group;
      const connectionAmount = this.calculateConnections();
      const radicalAmount = this.radicalAmount;
      const charge = (_a = this.properties.charge) != null ? _a : 0;
      const absCharge = Math.abs(charge);
      let valence = connectionAmount;
      let hydrogenAmount = 0;
      if (connectionAmount === -1) {
        return {
          valence,
          hydrogenAmount
        };
      }
      if (elementGroupNumber === void 0) {
        if (label === "D" /* D */ || label === "T" /* T */) {
          valence = 1;
          hydrogenAmount = 1 - radicalAmount - connectionAmount - absCharge;
        }
      } else if (elementGroupNumber === 1) {
        if (label === "H" /* H */ || label === "Li" /* Li */ || label === "Na" /* Na */ || label === "K" /* K */ || label === "Rb" /* Rb */ || label === "Cs" /* Cs */ || label === "Fr" /* Fr */) {
          valence = 1;
          hydrogenAmount = 1 - radicalAmount - connectionAmount - absCharge;
        }
      } else if (elementGroupNumber === 2) {
        if (connectionAmount + radicalAmount + absCharge === 2 || connectionAmount + radicalAmount + absCharge === 0) {
          valence = 2;
        } else hydrogenAmount = -1;
      } else if (elementGroupNumber === 3) {
        if (label === "B" /* B */ || label === "Al" /* Al */ || label === "Ga" /* Ga */ || label === "In" /* In */) {
          if (charge === -1) {
            valence = 4;
            hydrogenAmount = 4 - radicalAmount - connectionAmount;
          } else {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount - absCharge;
          }
        } else if (label === "Tl" /* Tl */) {
          if (charge === -1) {
            if (radicalAmount + connectionAmount <= 2) {
              valence = 2;
              hydrogenAmount = 2 - radicalAmount - connectionAmount;
            } else {
              valence = 4;
              hydrogenAmount = 4 - radicalAmount - connectionAmount;
            }
          } else if (charge === -2) {
            if (radicalAmount + connectionAmount <= 3) {
              valence = 3;
              hydrogenAmount = 3 - radicalAmount - connectionAmount;
            } else {
              valence = 5;
              hydrogenAmount = 5 - radicalAmount - connectionAmount;
            }
          } else if (radicalAmount + connectionAmount + absCharge <= 1) {
            valence = 1;
            hydrogenAmount = 1 - radicalAmount - connectionAmount - absCharge;
          } else {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount - absCharge;
          }
        }
      } else if (elementGroupNumber === 4) {
        if (label === "C" /* C */ || label === "Si" /* Si */ || label === "Ge" /* Ge */) {
          valence = 4;
          hydrogenAmount = 4 - radicalAmount - connectionAmount - absCharge;
        } else if (label === "Sn" /* Sn */ || label === "Pb" /* Pb */) {
          if (connectionAmount + radicalAmount + absCharge <= 2) {
            valence = 2;
            hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
          } else {
            valence = 4;
            hydrogenAmount = 4 - radicalAmount - connectionAmount - absCharge;
          }
        }
      } else if (elementGroupNumber === 5) {
        if (label === "N" /* N */ || label === "P" /* P */) {
          if (charge === 1) {
            valence = 4;
            hydrogenAmount = 4 - radicalAmount - connectionAmount;
          } else if (charge === 2) {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount;
          } else if (radicalAmount + connectionAmount + absCharge <= 3) {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount - absCharge;
          } else {
            valence = 5;
            hydrogenAmount = 5 - radicalAmount - connectionAmount - absCharge;
          }
        } else if (label === "Bi" /* Bi */ || label === "Sb" /* Sb */ || label === "As" /* As */) {
          if (charge === 1) {
            if (radicalAmount + connectionAmount <= 2 && label !== "As" /* As */) {
              valence = 2;
              hydrogenAmount = 2 - radicalAmount - connectionAmount;
            } else {
              valence = 4;
              hydrogenAmount = 4 - radicalAmount - connectionAmount;
            }
          } else if (charge === 2) {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount;
          } else if (radicalAmount + connectionAmount <= 3) {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount - absCharge;
          } else {
            valence = 5;
            hydrogenAmount = 5 - radicalAmount - connectionAmount - absCharge;
          }
        }
      } else if (elementGroupNumber === 6) {
        if (label === "O" /* O */) {
          if (charge >= 1) {
            valence = 3;
            hydrogenAmount = 3 - radicalAmount - connectionAmount;
          } else {
            valence = 2;
            hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
          }
        } else if (label === "S" /* S */ || label === "Se" /* Se */ || label === "Po" /* Po */) {
          if (charge === 1) {
            if (connectionAmount <= 3) {
              valence = 3;
              hydrogenAmount = 3 - radicalAmount - connectionAmount;
            } else {
              valence = 5;
              hydrogenAmount = 5 - radicalAmount - connectionAmount;
            }
          } else if (connectionAmount + radicalAmount + absCharge <= 2) {
            valence = 2;
            hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
          } else if (connectionAmount + radicalAmount + absCharge <= 4) {
            valence = 4;
            hydrogenAmount = 4 - radicalAmount - connectionAmount - absCharge;
          } else {
            valence = 6;
            hydrogenAmount = 6 - radicalAmount - connectionAmount - absCharge;
          }
        } else if (label === "Te" /* Te */) {
          if (charge === -1) {
            if (connectionAmount <= 2) {
              valence = 2;
              hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
            }
          } else if (charge === 0 || charge === 2) {
            if (connectionAmount <= 2) {
              valence = 2;
              hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
            } else if (connectionAmount <= 4) {
              valence = 4;
              hydrogenAmount = 4 - radicalAmount - connectionAmount - absCharge;
            } else if (charge === 0 && connectionAmount <= 6) {
              valence = 6;
              hydrogenAmount = 6 - radicalAmount - connectionAmount - absCharge;
            } else {
              hydrogenAmount = -1;
            }
          }
        }
      } else if (elementGroupNumber === 7) {
        if (label === "F" /* F */) {
          valence = 1;
          hydrogenAmount = 1 - radicalAmount - connectionAmount - absCharge;
        } else if (label === "Cl" /* Cl */ || label === "Br" /* Br */ || label === "I" /* I */ || label === "At" /* At */) {
          if (charge === 1) {
            if (connectionAmount <= 2) {
              valence = 2;
              hydrogenAmount = 2 - radicalAmount - connectionAmount;
            } else if (connectionAmount === 3 || connectionAmount === 5 || connectionAmount >= 7) {
              hydrogenAmount = -1;
            }
          } else if (charge === 0) {
            if (connectionAmount <= 1) {
              valence = 1;
              hydrogenAmount = 1 - radicalAmount - connectionAmount;
            } else if (connectionAmount === 2 || connectionAmount === 4 || connectionAmount === 6) {
              if (radicalAmount === 1) {
                valence = connectionAmount;
              } else {
                hydrogenAmount = -1;
              }
            } else if (connectionAmount > 7) {
              hydrogenAmount = -1;
            }
          }
        }
      } else if (elementGroupNumber === 8) {
        if (label === "Pt" /* Pt */) {
          if (connectionAmount + radicalAmount + absCharge <= 2) {
            valence = 2;
            hydrogenAmount = 2 - radicalAmount - connectionAmount - absCharge;
          } else if (connectionAmount + radicalAmount + absCharge <= 4) {
            valence = 4;
            hydrogenAmount = 4 - radicalAmount - connectionAmount - absCharge;
          } else {
            hydrogenAmount = -1;
          }
        } else if (connectionAmount + radicalAmount + absCharge === 0) {
          valence = 1;
        } else {
          hydrogenAmount = -1;
        }
      }
      return {
        valence,
        hydrogenAmount
      };
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_editorSingleton2 = __toESM(require_editorSingleton());

  // domain/types/monomers.ts
  var AttachmentPointName = /* @__PURE__ */ ((AttachmentPointName2) => {
    AttachmentPointName2["R1"] = "R1";
    AttachmentPointName2["R2"] = "R2";
    AttachmentPointName2["R3"] = "R3";
    AttachmentPointName2["R4"] = "R4";
    AttachmentPointName2["R5"] = "R5";
    AttachmentPointName2["R6"] = "R6";
    AttachmentPointName2["R7"] = "R7";
    AttachmentPointName2["R8"] = "R8";
    AttachmentPointName2["HYDROGEN"] = "hydrogen";
    return AttachmentPointName2;
  })(AttachmentPointName || {});
  var attachmentPointNames = [
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8"
  ];

  // domain/types/entities.ts
  var Entities = /* @__PURE__ */ ((Entities2) => {
    Entities2["Nucleotide"] = "Nucleotide";
    Entities2["Nucleoside"] = "Nucleoside";
    Entities2["Phosphate"] = "Phosphate";
    return Entities2;
  })(Entities || {});

  // domain/types/index.ts
  var KetAmbiguousMonomerTemplateSubType = {};
  var KetMonomerClass = {};

  // domain/entities/Command.ts
  var Command = class _Command {
    constructor() {
      __publicField(this, "operations", []);
      __publicField(this, "inverseOperations", []);
    }
    merge(command) {
      this.operations.push(...command.operations);
      this.inverseOperations.push(...command.inverseOperations);
    }
    addOperation(operation) {
      this.operations.push(operation);
    }
    addInverseOperation(operation) {
      this.inverseOperations.push(operation);
    }
    execute() {
      this.operations.forEach((op) => op());
    }
    invert() {
      const inverted = new _Command();
      inverted.operations = [...this.inverseOperations];
      inverted.inverseOperations = [...this.operations];
      return inverted;
    }
  };

  // domain/helpers/polymerBondMonomerConnections.ts
  var getMonomerClass = (monomer) => {
    var _a, _b, _c;
    return (
      // BaseMonomer does not expose normalized class metadata on its TS contract,
      // so we read it from runtime monomer payload fields used across KET models.
      (_c = (_b = (_a = monomer == null ? void 0 : monomer.monomerItem) == null ? void 0 : _a.props) == null ? void 0 : _b.MonomerClass) != null ? _c : monomer == null ? void 0 : monomer.monomerClass
    );
  };
  var isPolymerBondLike = (bond) => Boolean(
    bond && typeof bond === "object" && "getAnotherMonomer" in bond && !(bond instanceof MonomerToAtomBond)
  );
  var isRnaBaseOrAmbiguousRnaBase = (monomer) => getMonomerClass(monomer) === "Base";
  var isSugarMonomer = (monomer) => getMonomerClass(monomer) === "Sugar";
  var getSugarFromRnaBase = (monomer) => {
    if (!monomer || !isRnaBaseOrAmbiguousRnaBase(monomer)) {
      return void 0;
    }
    const r1PolymerBond = monomer.attachmentPointsToBonds.R1;
    const r1ConnectedMonomer = isPolymerBondLike(r1PolymerBond) ? r1PolymerBond.getAnotherMonomer(monomer) : void 0;
    if (!r1ConnectedMonomer) {
      return void 0;
    }
    const r3PolymerBond = r1ConnectedMonomer.attachmentPointsToBonds.R3;
    const r3ConnectedMonomer = isPolymerBondLike(r3PolymerBond) ? r3PolymerBond.getAnotherMonomer(r1ConnectedMonomer) : void 0;
    return isSugarMonomer(r1ConnectedMonomer) && r3ConnectedMonomer === monomer ? r1ConnectedMonomer : void 0;
  };
  var isMonomerConnectedToR2RnaBase = (monomer) => {
    if (!monomer) {
      return false;
    }
    const r1PolymerBond = monomer.attachmentPointsToBonds.R1;
    if (r1PolymerBond instanceof MonomerToAtomBond) {
      return false;
    }
    const r1ConnectedMonomer = r1PolymerBond == null ? void 0 : r1PolymerBond.getAnotherMonomer(monomer);
    if (!r1ConnectedMonomer) {
      return false;
    }
    const r2PolymerBond = r1ConnectedMonomer.attachmentPointsToBonds.R2;
    return Boolean(
      isRnaBaseOrAmbiguousRnaBase(r1ConnectedMonomer) && getSugarFromRnaBase(r1ConnectedMonomer) && isPolymerBondLike(r2PolymerBond) && r2PolymerBond.getAnotherMonomer(r1ConnectedMonomer) === monomer
    );
  };
  var isBondBetweenSugarAndBaseOfRna = (polymerBond) => polymerBond.firstMonomerAttachmentPoint === "R1" /* R1 */ && isRnaBaseOrAmbiguousRnaBase(polymerBond.firstMonomer) && polymerBond.secondMonomerAttachmentPoint === "R3" /* R3 */ && isSugarMonomer(polymerBond.secondMonomer) || polymerBond.firstMonomerAttachmentPoint === "R3" /* R3 */ && isSugarMonomer(polymerBond.firstMonomer) && polymerBond.secondMonomerAttachmentPoint === "R1" /* R1 */ && isRnaBaseOrAmbiguousRnaBase(polymerBond.secondMonomer);

  // src/core/chem/macromolecules/PolymerBond.ts
  var PolymerBond = class _PolymerBond extends BaseBond {
    constructor(firstMonomer, secondMonomer) {
      super();
      this.firstMonomer = firstMonomer;
      __publicField(this, "secondMonomer");
      __publicField(this, "renderer");
      // Move to renderer
      __publicField(this, "hasAntisenseInRow", false);
      // Move to renderer
      __publicField(this, "nextRowPositionX");
      this.firstMonomer = firstMonomer;
      this.secondMonomer = secondMonomer;
    }
    setFirstMonomer(monomer) {
      this.firstMonomer = monomer;
    }
    setSecondMonomer(monomer) {
      this.secondMonomer = monomer;
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    static get backBoneChainAttachmentPoints() {
      return ["R1" /* R1 */, "R2" /* R2 */];
    }
    get isBackBoneChainConnection() {
      return !this.isSideChainConnection;
    }
    get firstMonomerAttachmentPoint() {
      return this.firstMonomer.getAttachmentPointByBond(this);
    }
    get secondMonomerAttachmentPoint() {
      var _a;
      return (_a = this.secondMonomer) == null ? void 0 : _a.getAttachmentPointByBond(this);
    }
    get isSideChainConnection() {
      const firstMonomerAttachmentPoint = this.firstMonomerAttachmentPoint;
      const secondMonomerAttachmentPoint = this.secondMonomerAttachmentPoint;
      if (!firstMonomerAttachmentPoint || !secondMonomerAttachmentPoint) {
        return false;
      }
      return (!(_PolymerBond.backBoneChainAttachmentPoints.includes(
        firstMonomerAttachmentPoint
      ) && _PolymerBond.backBoneChainAttachmentPoints.includes(
        secondMonomerAttachmentPoint
      )) || isMonomerConnectedToR2RnaBase(this.firstMonomer) && isRnaBaseOrAmbiguousRnaBase(this.secondMonomer) || isMonomerConnectedToR2RnaBase(this.secondMonomer) && isRnaBaseOrAmbiguousRnaBase(this.firstMonomer) || firstMonomerAttachmentPoint === secondMonomerAttachmentPoint) && !isBondBetweenSugarAndBaseOfRna(this);
    }
    get firstEndEntity() {
      return this.firstMonomer;
    }
    get secondEndEntity() {
      return this.secondMonomer;
    }
    getAnotherMonomer(monomer) {
      return super.getAnotherEntity(monomer);
    }
    get isHorizontal() {
      if (!this.secondMonomer) {
        return false;
      }
      return Math.abs(this.firstMonomer.position.y - this.secondMonomer.position.y) < HalfMonomerSize;
    }
    get isVertical() {
      if (!this.secondMonomer) {
        return false;
      }
      return Math.abs(this.firstMonomer.position.x - this.secondMonomer.position.x) < HalfMonomerSize;
    }
  };

  // domain/entities/atomList.ts
  var AtomList = class {
    constructor(params) {
      __publicField(this, "notList");
      __publicField(this, "ids");
      this.notList = params.notList;
      this.ids = params.ids;
    }
    labelList() {
      const labels = [];
      for (const id2 of this.ids) {
        const currenElement = Elements.get(id2);
        if (currenElement) {
          labels.push(currenElement.label);
        }
      }
      return labels;
    }
    label() {
      let label = "[" + this.labelList().join(",") + "]";
      if (this.notList) {
        label = "!" + label;
      }
      return label;
    }
    equals(atomList) {
      const getSortedIds = (ids) => [...ids != null ? ids : []].sort((a, b) => a - b).toString();
      return this.notList === atomList.notList && getSortedIds(this.ids) === getSortedIds(atomList.ids);
    }
  };

  // domain/entities/pile.ts
  var Pile = class _Pile extends Set {
    // TODO: it's used only in dfs.js in one place in some strange way.
    // Should be removed after dfs.js refactoring
    find(predicate) {
      for (const item of this) {
        if (predicate(item)) return item;
      }
      return null;
    }
    equals(setB) {
      return this.isSuperset(setB) && setB.isSuperset(this);
    }
    isSuperset(subset) {
      for (const item of subset) {
        if (!this.has(item)) return false;
      }
      return true;
    }
    filter(expression) {
      return new _Pile(Array.from(this).filter(expression));
    }
    union(setB) {
      const union = new _Pile(this);
      for (const item of setB) union.add(item);
      return union;
    }
    intersection(setB) {
      const thisSet = new _Pile(this);
      return new _Pile([...thisSet].filter((item) => setB.has(item)));
    }
    /**
     * Union multiple sets which have intersections
     * @example ```
     * const setA = new Pile([0, 1])
     * const setB = new Pile([1, 2])
     * const setC = new Pile([2, 3])
     * const setD = new Pile([4, 5])
     * console.log(Pile.unionMultiple([setA, setB, setC, setD]))
     * // [{0, 1, 2, 3}, {4, 5}]
     * ```
     */
    static unionIntersections(sets) {
      let unionized = false;
      const setsToReturn = sets.reduce((prevSets, curSet) => {
        let isCurSetMerged = false;
        const newSets = prevSets.map((set) => {
          const intersec = set.intersection(curSet);
          if (intersec.size > 0) {
            unionized = true;
            isCurSetMerged = true;
            return set.union(curSet);
          }
          return set;
        });
        if (!isCurSetMerged) newSets.push(curSet);
        return newSets;
      }, new Array());
      return unionized ? _Pile.unionIntersections(setsToReturn) : setsToReturn;
    }
  };

  // domain/entities/BaseMicromoleculeEntity.ts
  var INVALID = "invalid";
  var BaseMicromoleculeEntity = class {
    constructor(initiallySelected) {
      __publicField(this, "initiallySelected");
      this.initiallySelected = initiallySelected;
    }
    getInitiallySelected() {
      if (this.initiallySelected === INVALID) {
        throw String(
          "this field is used only for serialization/deserialization"
        );
      }
      return this.initiallySelected;
    }
    setInitiallySelected(value) {
      if (this.initiallySelected === INVALID) {
        throw String(
          "this field is used only for serialization/deserialization"
        );
      }
      this.initiallySelected = value;
    }
    resetInitiallySelected(invalidate) {
      this.initiallySelected = invalidate ? INVALID : void 0;
    }
  };

  // src/core/chem/atom.ts
  var import_lodash3 = __toESM(require_lodash2());

  // domain/entities/box2Abs.ts
  var _Box2Abs_static, isPointOnSegment_fn;
  var _Box2Abs = class _Box2Abs {
    constructor(...args) {
      __publicField(this, "p0");
      __publicField(this, "p1");
      if (args.length === 1 && "min" in args[0] && "max" in args[0]) {
        this.p0 = args[0].min;
        this.p1 = args[0].max;
      }
      if (args.length === 2) {
        this.p0 = args[0];
        this.p1 = args[1];
      } else if (args.length === 4) {
        this.p0 = new Vec2(args[0], args[1]);
        this.p1 = new Vec2(args[2], args[3]);
      } else if (args.length === 0) {
        this.p0 = new Vec2();
        this.p1 = new Vec2();
      } else {
        throw String(
          "Box2Abs constructor only accepts 4 numbers or 2 vectors or no args!"
        );
      }
    }
    toString() {
      return this.p0.toString() + " " + this.p1.toString();
    }
    clone() {
      return new _Box2Abs(this.p0, this.p1);
    }
    extend(lp, rb) {
      rb = rb || lp;
      return new _Box2Abs(this.p0.sub(lp), this.p1.add(rb));
    }
    include(p) {
      assert_default(p != null);
      return new _Box2Abs(this.p0.min(p), this.p1.max(p));
    }
    contains(p, ext = 0) {
      assert_default(p != null);
      return p.x >= this.p0.x - ext && p.x <= this.p1.x + ext && p.y >= this.p0.y - ext && p.y <= this.p1.y + ext;
    }
    translate(d) {
      return new _Box2Abs(this.p0.add(d), this.p1.add(d));
    }
    transform(f, options) {
      assert_default(typeof f === "function");
      return new _Box2Abs(f(this.p0, options), f(this.p1, options));
    }
    sz() {
      return this.p1.sub(this.p0);
    }
    centre() {
      return Vec2.centre(this.p0, this.p1);
    }
    pos() {
      return this.p0;
    }
    hasZeroArea() {
      const size = this.sz();
      return size.x === 0 && size.y === 0;
    }
    static fromRelBox(relBox) {
      return new _Box2Abs(
        relBox.x,
        relBox.y,
        relBox.x + relBox.width,
        relBox.y + relBox.height
      );
    }
    static union(b1, b2) {
      return new _Box2Abs(Vec2.min(b1.p0, b2.p0), Vec2.max(b1.p1, b2.p1));
    }
    static segmentIntersection(a, b, c, d) {
      var _a, _b, _c, _d;
      const dc = (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
      const dd = (a.x - d.x) * (b.y - d.y) - (a.y - d.y) * (b.x - d.x);
      const da = (c.x - a.x) * (d.y - a.y) - (c.y - a.y) * (d.x - a.x);
      const db = (c.x - b.x) * (d.y - b.y) - (c.y - b.y) * (d.x - b.x);
      if (dc === 0 && dd === 0 && da === 0 && db === 0) {
        return __privateMethod(_a = _Box2Abs, _Box2Abs_static, isPointOnSegment_fn).call(_a, a, b, c) || __privateMethod(_b = _Box2Abs, _Box2Abs_static, isPointOnSegment_fn).call(_b, a, b, d) || __privateMethod(_c = _Box2Abs, _Box2Abs_static, isPointOnSegment_fn).call(_c, c, d, a) || __privateMethod(_d = _Box2Abs, _Box2Abs_static, isPointOnSegment_fn).call(_d, c, d, b);
      } else return dc * dd < 0 && da * db < 0;
    }
  };
  _Box2Abs_static = new WeakSet();
  isPointOnSegment_fn = function(segPointA, segPointB, point) {
    const minX = Math.min(segPointA.x, segPointB.x);
    const maxX = Math.max(segPointA.x, segPointB.x);
    const minY = Math.min(segPointA.y, segPointB.y);
    const maxY = Math.max(segPointA.y, segPointB.y);
    return point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY;
  };
  __privateAdd(_Box2Abs, _Box2Abs_static);
  var Box2Abs = _Box2Abs;

  // domain/helpers/scale.ts
  function canvasToModel(point, options) {
    return point.scaled(1 / options.microModeScale);
  }
  function modelToCanvas(vector, options) {
    return vector.scaled(options.microModeScale);
  }
  var Scale = {
    canvasToModel,
    modelToCanvas
  };

  // domain/helpers/stereoValidator.ts
  function isCorrectStereoCenter(bond, beginNeighs, endNeighs, struct) {
    var _a;
    const beginAtom = struct.atoms.get(bond.begin);
    let EndAtomNeigh = NaN;
    if ((endNeighs == null ? void 0 : endNeighs.length) === 2) {
      EndAtomNeigh = endNeighs[0].aid === bond.begin ? endNeighs[1].aid : endNeighs[0].aid;
    }
    if (bond.stereo > 0) {
      if ((endNeighs == null ? void 0 : endNeighs.length) === 1 && (beginNeighs == null ? void 0 : beginNeighs.length) === 2 && Number(beginAtom == null ? void 0 : beginAtom.implicitH) % 2 === 0) {
        return false;
      }
      if ((endNeighs == null ? void 0 : endNeighs.length) === 2 && (beginNeighs == null ? void 0 : beginNeighs.length) === 2 && Number(beginAtom == null ? void 0 : beginAtom.implicitH) % 2 === 0 && ((_a = struct.atomGetNeighbors(EndAtomNeigh)) == null ? void 0 : _a.length) === 1) {
        return false;
      }
      if ((beginNeighs == null ? void 0 : beginNeighs.length) === 1) {
        return false;
      }
      return true;
    } else {
      return false;
    }
  }
  var StereoValidator = {
    isCorrectStereoCenter
  };

  // domain/helpers/functionalGroupsProvider.ts
  var _FunctionalGroupsProvider = class _FunctionalGroupsProvider {
    constructor() {
      __publicField(this, "functionalGroupsList");
      this.functionalGroupsList = [];
    }
    static getInstance() {
      if (!_FunctionalGroupsProvider.instance) {
        _FunctionalGroupsProvider.instance = new _FunctionalGroupsProvider();
      }
      return _FunctionalGroupsProvider.instance;
    }
    getFunctionalGroupsList() {
      return this.functionalGroupsList;
    }
    setFunctionalGroupsList(list) {
      this.functionalGroupsList = list;
    }
    addToFunctionalGroupsList(list) {
      this.functionalGroupsList = [...this.functionalGroupsList, ...list];
    }
  };
  // eslint-disable-next-line no-use-before-define
  __publicField(_FunctionalGroupsProvider, "instance");
  var FunctionalGroupsProvider = _FunctionalGroupsProvider;

  // domain/helpers/saltsAndSolventsProvider.ts
  var _SaltsAndSolventsProvider = class _SaltsAndSolventsProvider {
    constructor() {
      __publicField(this, "saltsAndSolventsList");
      this.saltsAndSolventsList = [];
    }
    static getInstance() {
      if (!_SaltsAndSolventsProvider.instance) {
        _SaltsAndSolventsProvider.instance = new _SaltsAndSolventsProvider();
      }
      return _SaltsAndSolventsProvider.instance;
    }
    getSaltsAndSolventsList() {
      return this.saltsAndSolventsList;
    }
    setSaltsAndSolventsList(list) {
      this.saltsAndSolventsList = list;
    }
  };
  // eslint-disable-next-line no-use-before-define
  __publicField(_SaltsAndSolventsProvider, "instance");
  var SaltsAndSolventsProvider = _SaltsAndSolventsProvider;

  // domain/helpers/attachmentPointCalculations.ts
  var import_coordinates2 = __toESM(require_coordinates());
  function getAttachmentPointLabelWithBinaryShift(attachmentPointNumber) {
    let attachmentPointLabel = "";
    for (let rgi = 0; rgi < 32; rgi++) {
      if (attachmentPointNumber & 1 << rgi) {
        attachmentPointLabel += getAttachmentPointLabel(rgi + 1);
      }
    }
    return attachmentPointLabel;
  }
  function isSingleRGroupAttachmentPoint(rGroupLabel) {
    if (rGroupLabel === 0) return false;
    const unsigned = rGroupLabel >>> 0;
    return (unsigned & unsigned - 1) === 0;
  }
  function getAttachmentPointLabel(attachmentPointNumber) {
    return `R${attachmentPointNumber}`;
  }
  function getAttachmentPointNumberFromLabel(attachmentPointLabel) {
    return Number(attachmentPointLabel.replace("R", ""));
  }
  var getNextFreeAttachmentPoint = (attachmentPoints, skipR1AndR2 = false) => {
    const orderedAttachmentPointNumbers = attachmentPoints.map(getAttachmentPointNumberFromLabel).sort((a, b) => a - b);
    let nextFreeAttachmentPointNumber = skipR1AndR2 ? 3 : 1;
    for (const number of orderedAttachmentPointNumbers) {
      if (number === nextFreeAttachmentPointNumber) {
        nextFreeAttachmentPointNumber++;
      } else {
        break;
      }
    }
    if (nextFreeAttachmentPointNumber > 8) {
      throw String("Cannot assign more than 8 attachment points");
    }
    return getAttachmentPointLabel(nextFreeAttachmentPointNumber);
  };

  // src/core/chem/sgroup.ts
  var import_lodash2 = __toESM(require_lodash2());
  var SGroupBracketParams = class {
    constructor(c, d, w, h) {
      __publicField(this, "c");
      __publicField(this, "d");
      __publicField(this, "n");
      __publicField(this, "w");
      __publicField(this, "h");
      this.c = c;
      this.d = d;
      this.n = d.rotateSC(1, 0);
      this.w = w;
      this.h = h;
    }
  };
  var _SGroup = class _SGroup {
    constructor(type) {
      __publicField(this, "type");
      __publicField(this, "id");
      __publicField(this, "label");
      __publicField(this, "bracketBox");
      __publicField(this, "bracketDirection");
      __publicField(this, "areas");
      __publicField(this, "hover");
      __publicField(this, "hovering");
      __publicField(this, "selected");
      __publicField(this, "selectionPlate");
      __publicField(this, "atoms");
      __publicField(this, "atomSet");
      __publicField(this, "parentAtomSet");
      __publicField(this, "patoms");
      __publicField(this, "allAtoms");
      __publicField(this, "bonds");
      __publicField(this, "xBonds");
      __publicField(this, "neiAtoms");
      __publicField(this, "pp");
      __publicField(this, "data");
      __publicField(this, "dataArea");
      __publicField(this, "functionalGroup");
      __publicField(this, "attachmentPoints");
      this.type = type;
      this.id = -1;
      this.label = -1;
      this.bracketBox = null;
      this.bracketDirection = new Vec2(1, 0);
      this.areas = [];
      this.hover = false;
      this.hovering = null;
      this.selected = false;
      this.selectionPlate = null;
      this.atoms = [];
      this.patoms = [];
      this.bonds = [];
      this.xBonds = [];
      this.neiAtoms = [];
      this.attachmentPoints = [];
      this.pp = null;
      this.data = {
        mul: 1,
        // multiplication count for MUL group
        connectivity: "ht",
        // head-to-head, head-to-tail or either-unknown
        name: "",
        nucleotideComponent: "",
        subscript: "",
        expanded: void 0,
        // data s-group fields
        attached: false,
        absolute: true,
        showUnits: false,
        nCharsToDisplay: -1,
        tagChar: "",
        daspPos: 1,
        fieldType: "F",
        fieldName: "",
        fieldValue: "",
        units: "",
        query: "",
        queryOp: ""
      };
    }
    // TODO: these methods should be overridden
    //      and should only accept valid attributes for each S-group type.
    //      The attributes should be accessed via these methods only and not directly through this.data.
    // stub
    getAttr(attr) {
      return this.data[attr];
    }
    setFunctionalGroup(functionalGroup) {
      this.functionalGroup = functionalGroup;
    }
    // TODO: should be group-specific
    getAttrs() {
      const attrs = {};
      Object.keys(this.data).forEach((attr) => {
        attrs[attr] = this.data[attr];
      });
      return attrs;
    }
    // stub
    setAttr(attr, value) {
      const oldValue = this.data[attr];
      this.data[attr] = value;
      return oldValue;
    }
    // stub
    checkAttr(attr, value) {
      return this.data[attr] === value;
    }
    updateOffset(offset) {
      this.pp = Vec2.sum(this.bracketBox.p1, offset);
    }
    isExpanded() {
      if (_SGroup.isSuperAtom(this)) {
        return Boolean(this.data.expanded);
      } else {
        return true;
      }
    }
    isContracted() {
      return !this.isExpanded();
    }
    calculatePP(struct) {
      var _a;
      let topLeftPoint;
      const isAtomContext = this.data.context === SgContexts.Atom;
      const isBondContent = this.data.context === SgContexts.Bond;
      if (isAtomContext || isBondContent) {
        const contentBoxes = [];
        let contentBB = null;
        this.atoms.forEach((aid) => {
          const atom = struct.atoms.get(aid);
          const pos = new Vec2(atom.pp);
          const ext = new Vec2(0.05 * 3, 0.05 * 3);
          const bba = new Box2Abs(pos, pos).extend(ext, ext);
          contentBoxes.push(bba);
        });
        contentBoxes.forEach((bba) => {
          let bbb = null;
          [bba.p0.x, bba.p1.x].forEach((x) => {
            [bba.p0.y, bba.p1.y].forEach((y) => {
              const v = new Vec2(x, y);
              bbb = !bbb ? new Box2Abs(v, v) : bbb.include(v);
            });
          });
          contentBB = !contentBB ? bbb : Box2Abs.union(contentBB, bbb);
        });
        topLeftPoint = isBondContent ? contentBB.centre() : contentBB.p0;
      } else {
        topLeftPoint = this.bracketBox.p1.add(new Vec2(0.5, 0.5));
      }
      const sgroups = Array.from(struct.sgroups.values());
      for (const _ of sgroups) {
        if (!descriptorIntersects(sgroups, topLeftPoint)) break;
        topLeftPoint = topLeftPoint.add(new Vec2(0, 0.5));
      }
      if (this.data.fieldName === "INDIGO_CIP_DESC") {
        if (this.atoms.length === 1) {
          const sAtom = this.atoms[0];
          const sAtomPP = (_a = struct.atoms.get(sAtom)) == null ? void 0 : _a.pp;
          if (sAtomPP) {
            topLeftPoint = sAtomPP;
          }
        } else {
          topLeftPoint = _SGroup.getMassCentre(struct, this.atoms);
        }
      }
      this.pp = topLeftPoint;
    }
    isGroupAttached(struct) {
      return this.getConnectionPointsCount(struct) >= 1;
    }
    addAttachmentPoint(attachmentPoint, validateUniqueness = true) {
      const isAttachmentPointAlreadyExist = this.attachmentPoints.some(
        ({ atomId, leaveAtomId }) => attachmentPoint.atomId === atomId && attachmentPoint.leaveAtomId === leaveAtomId
      );
      if (isAttachmentPointAlreadyExist && validateUniqueness) {
        throw String(
          "The same attachment point cannot be added to an S-group more than once"
        );
      }
      this.attachmentPoints.push(attachmentPoint);
    }
    addAttachmentPoints(attachmentPoints, validateUniqueness = true) {
      for (const attachmentPoint of attachmentPoints) {
        this.addAttachmentPoint(attachmentPoint, validateUniqueness);
      }
    }
    removeAttachmentPoint(attachmentPoint) {
      const index = this.attachmentPoints.indexOf(attachmentPoint);
      if (index !== -1) {
        this.attachmentPoints.splice(index, 1);
        return true;
      }
      return false;
    }
    getAttachmentPoints() {
      return this.attachmentPoints;
    }
    /**
     * Connection point - is not! the same as Attachment point.
     * Connection point is a fact for the sgroup - is the atom that has connected bond to an external atom.
     * So it doesn't matter how it happens (connection atom).
     * When we talk about "Attachment point" it is a hypothetical, suitable place to connect to sgroup.
     * But there are cases when sgroup doesn't have attachment points but have connection (read from external file)
     */
    getConnectionPointsCount(struct) {
      var _a;
      const connectionAtoms = /* @__PURE__ */ new Set();
      for (const atomId of this.atoms) {
        const neighbors = (_a = struct.atomGetNeighbors(atomId)) != null ? _a : [];
        for (const { aid } of neighbors) {
          if (!this.atoms.includes(aid)) {
            connectionAtoms.add(atomId);
            break;
          }
        }
      }
      return connectionAtoms.size;
    }
    isNotContractible(struct) {
      return this.getConnectionPointsCount(struct) > 1;
    }
    /**
     * Why only one?
     * Currently other parts of application don't support several attachment points for sgroup.
     * So to support it - it's required to refactor almost every peace of code with sgroups.
     *
     *
     * Why return 'undefined' without fallback?
     * If sgroup doesn't have attachment points it can't be attached, (salt and solvents for example).
     */
    getAttachmentAtomId() {
      var _a;
      return (_a = this.attachmentPoints[0]) == null ? void 0 : _a.atomId;
    }
    /**
     * WHY? When group is contracted we need to understand the represent atom to calculate position.
     * It is not always the attachmentPoint!! if no attachment point - use the first atom
     */
    getContractedPosition(struct) {
      var _a;
      let atomId = (_a = this.attachmentPoints[0]) == null ? void 0 : _a.atomId;
      let representAtom = struct.atoms.get(atomId);
      if (!representAtom) {
        let externalConnectionAtom;
        struct.bonds.forEach((bond) => {
          const isBeginAtomInCurrentSgroup = this.atoms.indexOf(bond.begin) !== -1;
          const isEndAtomInCurrentSgroup = this.atoms.indexOf(bond.end) !== -1;
          if (isBeginAtomInCurrentSgroup && !isEndAtomInCurrentSgroup) {
            externalConnectionAtom = bond.begin;
          } else if (isEndAtomInCurrentSgroup && !isBeginAtomInCurrentSgroup) {
            externalConnectionAtom = bond.end;
          }
        });
        atomId = (0, import_lodash2.isNumber)(externalConnectionAtom) ? externalConnectionAtom : this.atoms[0];
        representAtom = struct.atoms.get(atomId);
      }
      assert_default(representAtom != null);
      return { atomId, position: representAtom.pp };
    }
    cloneAttachmentPoints(atomIdMap) {
      return this.attachmentPoints.map((point) => point.clone(atomIdMap));
    }
    get isSuperatomWithoutLabel() {
      return this.type === _SGroup.TYPES.SUP && !this.data.name && !this.data.class;
    }
    get isMonomer() {
      return false;
    }
    static getOffset(sgroup) {
      if (!(sgroup == null ? void 0 : sgroup.pp) || !sgroup.bracketBox) return null;
      return Vec2.diff(sgroup.pp, sgroup.bracketBox.p1);
    }
    static isSaltOrSolvent(moleculeName) {
      const saltsAndSolventsProvider = SaltsAndSolventsProvider.getInstance();
      const saltsAndSolvents = saltsAndSolventsProvider.getSaltsAndSolventsList();
      return saltsAndSolvents.some(
        ({ name, abbreviation }) => name === moleculeName || moleculeName === abbreviation
      );
    }
    static isAtomInSaltOrSolvent(atomId, sgroupsOnCanvas) {
      const onlySaltsOrSolvents = sgroupsOnCanvas.filter(
        (sgroup) => this.isSaltOrSolvent(sgroup.data.name)
      );
      return onlySaltsOrSolvents.some(
        ({ atoms }) => atoms.some((atomIdInSaltOrSolvent) => atomIdInSaltOrSolvent === atomId)
      );
    }
    static isBondInSaltOrSolvent(bondId, sgroupsOnCanvas) {
      const onlySaltsOrSolvents = sgroupsOnCanvas.filter(
        (sgroup) => this.isSaltOrSolvent(sgroup.data.name)
      );
      return onlySaltsOrSolvents.some(
        ({ bonds }) => bonds.some((bondIdInSaltOrSolvent) => bondIdInSaltOrSolvent === bondId)
      );
    }
    static filterAtoms(atoms, map) {
      const newAtoms = [];
      for (const aid of atoms) {
        if (typeof map[aid] !== "number") newAtoms.push(aid);
        else if (map[aid] >= 0) newAtoms.push(map[aid]);
        else newAtoms.push(-1);
      }
      return newAtoms;
    }
    static removeNegative(atoms) {
      const newAtoms = [];
      for (const atom of atoms) {
        if (atom >= 0) newAtoms.push(atom);
      }
      return newAtoms;
    }
    static filter(_mol, sg, atomMap) {
      sg.atoms = _SGroup.removeNegative(_SGroup.filterAtoms(sg.atoms, atomMap));
    }
    static clone(sgroup, aidMap) {
      const cp = new _SGroup(sgroup.type);
      Object.keys(sgroup.data).forEach((field) => {
        cp.data[field] = sgroup.data[field];
      });
      cp.atoms = sgroup.atoms.map((elem) => aidMap.get(elem));
      cp.pp = sgroup.pp;
      cp.bracketBox = sgroup.bracketBox;
      cp.patoms = null;
      cp.bonds = null;
      cp.allAtoms = sgroup.allAtoms;
      cp.data.expanded = sgroup.data.expanded;
      cp.addAttachmentPoints(sgroup.cloneAttachmentPoints(aidMap));
      return cp;
    }
    static addAtom(sgroup, aid, struct) {
      sgroup.atoms.push(aid);
      if (sgroup.isNotContractible(struct)) {
        sgroup.setAttr("expanded", true);
      }
    }
    static removeAtom(sgroup, aid) {
      if (!sgroup) {
        return;
      }
      const index = sgroup.atoms.indexOf(aid);
      if (index !== -1) {
        sgroup.atoms.splice(index, 1);
      }
    }
    static getCrossBonds(mol, parentAtomSet) {
      const crossBonds = {};
      mol.bonds.forEach((bond, bid) => {
        if (parentAtomSet.has(bond.begin) && !parentAtomSet.has(bond.end)) {
          if (!crossBonds[bond.begin]) {
            crossBonds[bond.begin] = [];
          }
          crossBonds[bond.begin].push(bid);
        } else if (parentAtomSet.has(bond.end) && !parentAtomSet.has(bond.begin)) {
          if (!crossBonds[bond.end]) {
            crossBonds[bond.end] = [];
          }
          crossBonds[bond.end].push(bid);
        }
      });
      return crossBonds;
    }
    static bracketPos(sGroup, mol, remol, render) {
      const BORDER_EXT = new Vec2(0.05 * 3, 0.05 * 3);
      const PADDING_VECTOR = !_SGroup.isCOPGroup(sGroup) ? new Vec2(0.2, 0.4) : new Vec2(1.2, 1.2);
      const atoms = sGroup.atoms;
      let braketBox = null;
      const contentBoxes = [];
      const getAtom2 = (aid) => {
        if (remol && render) {
          return remol.atoms.get(aid);
        }
        return mol.atoms.get(aid);
      };
      sGroup.bracketDirection = new Vec2(1, 0);
      atoms.forEach((aid) => {
        const atom = getAtom2(aid);
        if (!atom) return;
        let position;
        let structBoundingBox = null;
        if ("getVBoxObj" in atom && render) {
          structBoundingBox = atom.getVBoxObj(render);
        } else if (atom.pp) {
          position = new Vec2(atom.pp);
          structBoundingBox = new Box2Abs(position, position);
        }
        if (!structBoundingBox) return;
        contentBoxes.push(structBoundingBox.extend(BORDER_EXT, BORDER_EXT));
      });
      contentBoxes.forEach((bba) => {
        braketBox = !braketBox ? bba : Box2Abs.union(braketBox, bba);
      });
      let attachmentPointsVBox = null;
      if (render) {
        attachmentPointsVBox = render.ctab.getRGroupAttachmentPointsVBoxByAtomIds(atoms);
      }
      attachmentPointsVBox = attachmentPointsVBox ? attachmentPointsVBox.extend(BORDER_EXT, BORDER_EXT) : attachmentPointsVBox;
      braketBox = attachmentPointsVBox && braketBox ? Box2Abs.union(braketBox, attachmentPointsVBox) : braketBox;
      if (braketBox) braketBox = braketBox.extend(PADDING_VECTOR, PADDING_VECTOR);
      sGroup.bracketBox = braketBox;
    }
    static getBracketParameters(mol, crossBondsPerAtom, atomSet, bb, d, n) {
      const brackets = [];
      const crossBondsPerAtomValues = Object.values(crossBondsPerAtom);
      const crossBonds = crossBondsPerAtomValues.flat();
      if (crossBonds.length < 2) {
        (function() {
          d = d || new Vec2(1, 0);
          n = n || d.rotateSC(1, 0);
          const bracketWidth = Math.min(0.25, bb.sz().x * 0.3);
          const cl = Vec2.lc2(d, bb.p0.x, n, 0.5 * (bb.p0.y + bb.p1.y));
          const cr = Vec2.lc2(d, bb.p1.x, n, 0.5 * (bb.p0.y + bb.p1.y));
          const bracketHeight = bb.sz().y;
          brackets.push(
            new SGroupBracketParams(cl, d.negated(), bracketWidth, bracketHeight),
            new SGroupBracketParams(cr, d, bracketWidth, bracketHeight)
          );
        })();
      } else if (crossBonds.length === 2 && crossBondsPerAtomValues.length === 2) {
        (function() {
          const b1 = mol.bonds.get(crossBonds[0]);
          const b2 = mol.bonds.get(crossBonds[1]);
          const cl0 = b1.getCenter(mol);
          const cr0 = b2.getCenter(mol);
          const dr = Vec2.diff(cr0, cl0).normalized();
          const dl = dr.negated();
          const bracketWidth = 0.25;
          const bracketHeight = 1.5;
          brackets.push(
            new SGroupBracketParams(
              cl0.addScaled(dl, 0),
              dl,
              bracketWidth,
              bracketHeight
            ),
            new SGroupBracketParams(
              cr0.addScaled(dr, 0),
              dr,
              bracketWidth,
              bracketHeight
            )
          );
        })();
      } else {
        (function() {
          for (const crossBondId of crossBonds) {
            const b = mol.bonds.get(crossBondId);
            const c = b.getCenter(mol);
            const d2 = atomSet.has(b.begin) ? b.getDir(mol) : b.getDir(mol).negated();
            brackets.push(new SGroupBracketParams(c, d2, 0.2, 1));
          }
        })();
      }
      return brackets;
    }
    static getObjBBox(atoms, mol, useCollapsedSgroupsPosition = false) {
      var _a;
      const a0 = (_a = mol.atoms.get(atoms[0])) == null ? void 0 : _a.pp;
      assert_default(a0);
      let bb = new Box2Abs(a0, a0);
      for (const aid of atoms.slice(1)) {
        const atom = mol.atoms.get(aid);
        assert_default(atom);
        const sgroupId = atom.sgs.values().next().value;
        const sgroup = (0, import_lodash2.isNumber)(sgroupId) ? mol.sgroups.get(sgroupId) : void 0;
        const p = useCollapsedSgroupsPosition && sgroup && !sgroup.isExpanded() ? sgroup.getContractedPosition(mol).position : atom.pp;
        bb = bb.include(p);
      }
      return bb;
    }
    static getAtoms(mol, sg) {
      if (sg && !sg.allAtoms) {
        return sg.atoms;
      }
      const atoms = [];
      mol.atoms.forEach((_atom, aid) => {
        atoms.push(aid);
      });
      return atoms;
    }
    static getBonds(mol, sg) {
      const atoms = _SGroup.getAtoms(mol, sg);
      const bonds = [];
      mol.bonds.forEach((bond, bid) => {
        if (atoms.indexOf(bond.begin) >= 0 && atoms.indexOf(bond.end) >= 0) {
          bonds.push(bid);
        }
      });
      return bonds;
    }
    static prepareMulForSaving(sgroup, mol) {
      sgroup.atoms.sort((a, b) => a - b);
      sgroup.atomSet = new Pile(sgroup.atoms);
      sgroup.parentAtomSet = new Pile(sgroup.atomSet);
      const inBonds = [];
      const xBonds = [];
      mol.bonds.forEach((bond, bid) => {
        if (sgroup.parentAtomSet.has(bond.begin) && sgroup.parentAtomSet.has(bond.end)) {
          inBonds.push(bid);
        } else if (sgroup.parentAtomSet.has(bond.begin) || sgroup.parentAtomSet.has(bond.end)) {
          xBonds.push(bid);
        }
      });
      if (xBonds.length !== 0 && xBonds.length !== 2) {
        throw String("Unsupported cross-bonds number");
      }
      let xAtom1 = -1;
      let xAtom2 = -1;
      let crossBond = null;
      if (xBonds.length === 2) {
        const bond1 = mol.bonds.get(xBonds[0]);
        xAtom1 = sgroup.parentAtomSet.has(bond1.begin) ? bond1.begin : bond1.end;
        const bond2 = mol.bonds.get(xBonds[1]);
        xAtom2 = sgroup.parentAtomSet.has(bond2.begin) ? bond2.begin : bond2.end;
        crossBond = bond2;
      }
      let tailAtom = xAtom2;
      const newAtoms = [];
      for (let j = 0; j < sgroup.data.mul - 1; j++) {
        const amap = {};
        sgroup.atoms.forEach((aid) => {
          const atom = mol.atoms.get(aid);
          const aid2 = mol.atoms.add(atom.clone());
          newAtoms.push(aid2);
          sgroup.atomSet.add(aid2);
          amap[aid] = aid2;
        });
        inBonds.forEach((bid) => {
          const bond = mol.bonds.get(bid);
          const newBond = bond.clone();
          newBond.begin = amap[newBond.begin];
          newBond.end = amap[newBond.end];
          mol.bonds.add(newBond);
        });
        if (crossBond !== null) {
          const newCrossBond = crossBond.clone();
          newCrossBond.begin = tailAtom;
          newCrossBond.end = amap[xAtom1];
          mol.bonds.add(newCrossBond);
          tailAtom = amap[xAtom2];
        }
      }
      if (tailAtom >= 0) {
        const xBond2 = mol.bonds.get(xBonds[1]);
        if (xBond2.begin === xAtom2) xBond2.begin = tailAtom;
        else xBond2.end = tailAtom;
      }
      sgroup.bonds = xBonds;
      newAtoms.forEach((aid) => {
        mol.sGroupForest.getPathToRoot(sgroup.id).reverse().forEach((sgid) => {
          mol.atomAddToSGroup(sgid, aid);
        });
      });
    }
    static getMassCentre(mol, atoms) {
      let c = new Vec2();
      for (const atomId of atoms) {
        c = c.addScaled(mol.atoms.get(atomId).pp, 1 / atoms.length);
      }
      return c;
    }
    static isBondInContractedSGroup(bond, sGroups) {
      return [...sGroups.values()].some((sGroupOrReSGroup) => {
        const sGroup = "item" in sGroupOrReSGroup ? sGroupOrReSGroup.item : sGroupOrReSGroup;
        const atomsInSGroup = sGroup == null ? void 0 : sGroup.atoms;
        return (sGroup == null ? void 0 : sGroup.isContracted()) && atomsInSGroup.includes(bond == null ? void 0 : bond.begin) && atomsInSGroup.includes(bond == null ? void 0 : bond.end);
      });
    }
    static isSuperAtom(sGroup) {
      if (!sGroup) {
        return false;
      }
      return (sGroup == null ? void 0 : sGroup.type) === _SGroup.TYPES.SUP;
    }
    static isDataSGroup(sGroup) {
      return sGroup.type === _SGroup.TYPES.DAT;
    }
    static isQuerySGroup(sGroup) {
      return sGroup.type === _SGroup.TYPES.queryComponent;
    }
    static isSRUSGroup(sGroup) {
      return sGroup.type === _SGroup.TYPES.SRU;
    }
    static isMulSGroup(sGroup) {
      return sGroup.type === _SGroup.TYPES.MUL;
    }
    static isCOPGroup(sGroup) {
      return sGroup.type === _SGroup.TYPES.COP;
    }
  };
  __publicField(_SGroup, "TYPES", {
    SUP: "SUP",
    MUL: "MUL",
    SRU: "SRU",
    MON: "MON",
    MER: "MER",
    COP: "COP",
    CRO: "CRO",
    MOD: "MOD",
    GRA: "GRA",
    COM: "COM",
    MIX: "MIX",
    FOR: "FOR",
    DAT: "DAT",
    ANY: "ANY",
    GEN: "GEN",
    queryComponent: "queryComponent",
    nucleotideComponent: "nucleotideComponent"
  });
  __publicField(_SGroup, "isAtomInContractedSGroup", (atom, sGroups) => {
    const contractedSGroup = [];
    sGroups.forEach((sGroupOrReSGroup) => {
      const sGroup = "item" in sGroupOrReSGroup ? sGroupOrReSGroup.item : sGroupOrReSGroup;
      if (sGroup.isContracted()) {
        contractedSGroup.push(sGroup.id);
      }
    });
    return contractedSGroup.some((sg) => atom.sgs.has(sg));
  });
  var SGroup = _SGroup;
  function descriptorIntersects(sgroups, topLeftPoint) {
    return sgroups.some((sg) => {
      if (!sg.pp) return false;
      const sgBottomRightPoint = sg.pp.add(new Vec2(0.5, 0.5));
      const bottomRightPoint = topLeftPoint.add(new Vec2(0.5, 0.5));
      return Box2Abs.segmentIntersection(
        sg.pp,
        sgBottomRightPoint,
        topLeftPoint,
        bottomRightPoint
      );
    });
  }

  // domain/entities/functionalGroup.ts
  var isSaltOrSolvent = (moleculeName) => {
    const saltsAndSolventsProvider = SaltsAndSolventsProvider.getInstance();
    const saltsAndSolvents = saltsAndSolventsProvider.getSaltsAndSolventsList();
    return saltsAndSolvents.some(
      ({ name, abbreviation }) => name === moleculeName || moleculeName === abbreviation
    );
  };
  var getSGroupBonds = (molecule, sgroup) => {
    const atoms = sgroup.allAtoms ? Array.from(molecule.atoms.keys()) : sgroup.atoms;
    const bonds = [];
    molecule.bonds.forEach((bond, bid) => {
      if (atoms.includes(bond.begin) && atoms.includes(bond.end)) {
        bonds.push(bid);
      }
    });
    return bonds;
  };
  var _sgroup;
  var _FunctionalGroup = class _FunctionalGroup {
    constructor(sgroup) {
      __privateAdd(this, _sgroup);
      assert_default(sgroup != null);
      __privateSet(this, _sgroup, sgroup);
      sgroup.setFunctionalGroup(this);
    }
    get name() {
      return __privateGet(this, _sgroup).data.name;
    }
    get relatedSGroupId() {
      return __privateGet(this, _sgroup).id;
    }
    get isExpanded() {
      return __privateGet(this, _sgroup).data.expanded;
    }
    get relatedSGroup() {
      return __privateGet(this, _sgroup);
    }
    static isFunctionalGroup(sgroup) {
      const provider = FunctionalGroupsProvider.getInstance();
      const functionalGroups = provider.getFunctionalGroupsList();
      const {
        data: { name },
        type
      } = sgroup;
      return type === "SUP" && (functionalGroups.some((type2) => type2.name === name) || isSaltOrSolvent(name));
    }
    static atomsInFunctionalGroup(functionalGroups, atom, isNeedCheckForGroups = false) {
      if (functionalGroups.size === 0) {
        return null;
      }
      for (const fg of functionalGroups.values()) {
        const isFunctionalGroup = isNeedCheckForGroups ? this.isFunctionalGroup(fg.relatedSGroup) : true;
        if (isFunctionalGroup && fg.relatedSGroup.atoms.includes(atom))
          return atom;
      }
      return null;
    }
    static bondsInFunctionalGroup(molecule, functionalGroups, bond) {
      if (functionalGroups.size === 0) {
        return null;
      }
      for (const fg of functionalGroups.values()) {
        const bonds = getSGroupBonds(molecule, fg.relatedSGroup);
        if (bonds.includes(bond)) return bond;
      }
      return null;
    }
    static isRGroupAttachmentPointInsideFunctionalGroup(molecule, id2) {
      const rgroupAttachmentPoint = molecule.rgroupAttachmentPoints.get(id2);
      assert_default(rgroupAttachmentPoint != null);
      const attachedAtom = rgroupAttachmentPoint.atomId;
      return _FunctionalGroup.atomsInFunctionalGroup(
        molecule.functionalGroups,
        attachedAtom
      );
    }
    static findFunctionalGroupByAtom(functionalGroups, atomId, isFunctionalGroupReturned) {
      for (const fg of functionalGroups.values()) {
        if (!fg.relatedSGroup.isSuperatomWithoutLabel && fg.relatedSGroup.atoms.includes(atomId))
          return isFunctionalGroupReturned ? fg : fg.relatedSGroupId;
      }
      return null;
    }
    static findFunctionalGroupByBond(molecule, functionalGroups, bondId, isFunctionalGroupReturned) {
      for (const fg of functionalGroups.values()) {
        const bonds = getSGroupBonds(molecule, fg.relatedSGroup);
        if (bondId !== null && !fg.relatedSGroup.isSuperatomWithoutLabel && bonds.includes(bondId)) {
          return isFunctionalGroupReturned ? fg : fg.relatedSGroupId;
        }
      }
      return null;
    }
    static findFunctionalGroupBySGroup(functionalGroups, sGroup) {
      const key = functionalGroups.find(
        (_, functionalGroup) => functionalGroup.relatedSGroupId === (sGroup == null ? void 0 : sGroup.id)
      );
      return key !== null ? functionalGroups.get(key) : void 0;
    }
    static clone(functionalGroup) {
      return new _FunctionalGroup(__privateGet(functionalGroup, _sgroup));
    }
    static isAtomInContractedFunctionalGroup(atom, sgroups, functionalGroups) {
      return [...atom.sgs.values()].some((sgid) => {
        const sgroup = sgroups.get(sgid);
        if (!sgroup) {
          return false;
        }
        return _FunctionalGroup.isContractedFunctionalGroup(
          "item" in sgroup ? sgroup.item : sgroup,
          functionalGroups
        );
      });
    }
    static isBondInContractedFunctionalGroup(bond, sGroups, functionalGroups) {
      return [...sGroups.values()].some((_sGroup) => {
        const sGroup = "item" in _sGroup ? _sGroup == null ? void 0 : _sGroup.item : _sGroup;
        const atomsInSGroup = sGroup == null ? void 0 : sGroup.atoms;
        const isContracted = _FunctionalGroup.isContractedFunctionalGroup(
          sGroup,
          functionalGroups
        );
        return isContracted && atomsInSGroup.includes(bond.begin) && atomsInSGroup.includes(bond.end);
      });
    }
    static isHalfBondInContractedFunctionalGroup(halfBond, struct) {
      const bond = struct.bonds.get(halfBond.bid);
      assert_default(bond != null);
      return this.isBondInContractedFunctionalGroup(
        bond,
        struct.sgroups,
        struct.functionalGroups
      );
    }
    static isContractedFunctionalGroup(sgroup, functionalGroups) {
      let isFunctionalGroup = false;
      let expanded = false;
      if (sgroup instanceof SGroup) {
        if (sgroup.functionalGroup) {
          isFunctionalGroup = true;
          expanded = sgroup.functionalGroup.isExpanded;
        }
      } else {
        functionalGroups.forEach((fg) => {
          if (fg.relatedSGroupId === sgroup) {
            isFunctionalGroup = true;
            expanded = fg.isExpanded;
          }
        });
      }
      return !expanded && isFunctionalGroup;
    }
  };
  _sgroup = new WeakMap();
  var FunctionalGroup = _FunctionalGroup;

  // src/core/chem/atom.ts
  var _Atom = class _Atom extends BaseMicromoleculeEntity {
    constructor(attributes) {
      var _a, _b, _c;
      super(attributes == null ? void 0 : attributes.initiallySelected);
      __publicField(this, "label");
      __publicField(this, "fragment");
      __publicField(this, "atomList");
      __publicField(this, "attachmentPoints");
      __publicField(this, "isotope");
      __publicField(this, "isPreview");
      __publicField(this, "hCount");
      __publicField(this, "radical");
      __publicField(this, "cip");
      __publicField(this, "charge");
      __publicField(this, "explicitValence");
      __publicField(this, "ringBondCount");
      __publicField(this, "queryProperties");
      __publicField(this, "unsaturatedAtom");
      __publicField(this, "substitutionCount");
      __publicField(this, "valence");
      __publicField(this, "implicitH");
      __publicField(this, "implicitHCount");
      __publicField(this, "pp");
      __publicField(this, "neighbors");
      __publicField(this, "sgs");
      __publicField(this, "badConn");
      __publicField(this, "alias");
      __publicField(this, "rglabel");
      __publicField(this, "aam");
      __publicField(this, "invRet");
      __publicField(this, "exactChangeFlag");
      __publicField(this, "rxnFragmentType");
      __publicField(this, "stereoLabel");
      __publicField(this, "stereoParity");
      __publicField(this, "hasImplicitH");
      __publicField(this, "pseudo");
      this.label = attributes.label;
      this.fragment = getValueOrDefault(attributes.fragment, -1);
      this.alias = getValueOrDefault(attributes.alias, _Atom.attrlist.alias);
      this.isotope = getValueOrDefault(attributes.isotope, _Atom.attrlist.isotope);
      this.radical = getValueOrDefault(attributes.radical, _Atom.attrlist.radical);
      this.cip = getValueOrDefault(attributes.cip, _Atom.attrlist.cip);
      this.charge = getValueOrDefault(attributes.charge, _Atom.attrlist.charge);
      this.rglabel = getValueOrDefault(attributes.rglabel, _Atom.attrlist.rglabel);
      this.attachmentPoints = getValueOrDefault(
        attributes.attachmentPoints,
        _Atom.attrlist.attachmentPoints
      );
      this.implicitHCount = getValueOrDefault(attributes.implicitHCount, null);
      this.explicitValence = getValueOrDefault(
        attributes.explicitValence,
        _Atom.attrlist.explicitValence
      );
      this.isPreview = getValueOrDefault(
        attributes.isPreview,
        _Atom.attrlist.isPreview
      );
      this.valence = 0;
      this.implicitH = (_b = (_a = attributes.implicitHCount) != null ? _a : attributes.implicitH) != null ? _b : 0;
      this.pp = attributes.pp ? new Vec2(attributes.pp) : new Vec2();
      this.sgs = new Pile();
      this.ringBondCount = getValueOrDefault(
        attributes.ringBondCount,
        _Atom.attrlist.ringBondCount
      );
      this.substitutionCount = getValueOrDefault(
        attributes.substitutionCount,
        _Atom.attrlist.substitutionCount
      );
      this.unsaturatedAtom = getValueOrDefault(
        attributes.unsaturatedAtom,
        _Atom.attrlist.unsaturatedAtom
      );
      this.hCount = getValueOrDefault(attributes.hCount, _Atom.attrlist.hCount);
      this.queryProperties = {};
      for (const property in _Atom.attrlist.queryProperties) {
        this.queryProperties[property] = getValueOrDefault(
          (_c = attributes.queryProperties) == null ? void 0 : _c[property],
          _Atom.attrlist.queryProperties[property]
        );
      }
      this.aam = getValueOrDefault(attributes.aam, _Atom.attrlist.aam);
      this.invRet = getValueOrDefault(attributes.invRet, _Atom.attrlist.invRet);
      this.exactChangeFlag = getValueOrDefault(
        attributes.exactChangeFlag,
        _Atom.attrlist.exactChangeFlag
      );
      this.rxnFragmentType = getValueOrDefault(attributes.rxnFragmentType, -1);
      this.stereoLabel = getValueOrDefault(
        attributes.stereoLabel,
        _Atom.attrlist.stereoLabel
      );
      this.stereoParity = getValueOrDefault(
        attributes.stereoParity,
        _Atom.attrlist.stereoParity
      );
      this.atomList = attributes.atomList ? new AtomList(attributes.atomList) : null;
      this.neighbors = [];
      this.badConn = false;
      Object.defineProperty(this, "pseudo", {
        enumerable: true,
        get: function() {
          return getPseudo(this.label);
        },
        set: function(value) {
          if (isCorrectPseudo(value)) {
            this.label = value;
          }
        }
      });
    }
    /** @deprecated */
    get attpnt() {
      return this.attachmentPoints;
    }
    get isRGroupAttachmentPointEditDisabled() {
      return this.label === "R#" && this.rglabel !== null;
    }
    /**
     * Trick: used for cloned struct for tooltips, for preview, for templates
     *
     * Why?
     * Currently, tooltips are implemented with removing sgroups (wrong implementation)
     * That's why we need to mark atoms as sgroup attachment points.
     *
     * If we change preview approach to flagged (option for showing sgroups without abbreviation),
     * then we will be able to remove this hack.
     */
    setRGAttachmentPointForDisplayPurpose() {
      this.attachmentPoints = 1 /* FirstSideOnly */;
    }
    static getConnectedBondIds(struct, atomId) {
      const result = [];
      for (const [bondId, bond] of struct.bonds.entries()) {
        if (bond.begin === atomId || bond.end === atomId) {
          result.push(bondId);
        }
      }
      return result;
    }
    static getAttrHash(atom) {
      const attrs = {};
      for (const attr in _Atom.attrlist) {
        if (typeof atom[attr] !== "undefined") attrs[attr] = atom[attr];
      }
      return attrs;
    }
    static attrGetDefault(attr) {
      if (attr in _Atom.attrlist) {
        return _Atom.attrlist[attr];
      }
    }
    static isHeteroAtom(label) {
      return label !== "C" && label !== "H";
    }
    static isInAromatizedRing(struct, atomId) {
      const atom = struct.atoms.get(atomId);
      if (atom && _Atom.isHeteroAtom(atom.label)) {
        for (const [_, loop] of struct.loops) {
          const halfBondIds = loop.hbs;
          if (loop.aromatic) {
            for (const halfBondId of halfBondIds) {
              const halfBond = struct.halfBonds.get(halfBondId);
              if (!halfBond) return false;
              const { begin, end } = halfBond;
              if (begin === atomId || end === atomId) {
                return true;
              }
            }
          }
        }
      }
      return false;
    }
    clone(fidMap) {
      const ret = new _Atom(this);
      const fragmentId = fidMap == null ? void 0 : fidMap.get(this.fragment);
      if (fragmentId !== void 0) {
        ret.fragment = fragmentId;
      }
      return ret;
    }
    isQuery() {
      const { queryProperties } = this;
      const isAnyAtom = this.label === "A";
      const isAnyMetal = this.label === "M" || this.label === "MH";
      const isAnyHalogen = this.label === "X" || this.label === "XH";
      const isAnyGroup = this.label === "G" || this.label === "G*" || this.label === "GH" || this.label === "GH*";
      return Boolean(
        this.substitutionCount !== 0 || this.unsaturatedAtom !== 0 || this.ringBondCount !== 0 || isAnyAtom || isAnyMetal || isAnyHalogen || isAnyGroup || this.hCount !== 0 || this.atomList !== null || Object.values(queryProperties).some((value) => value)
      );
    }
    pureHydrogen() {
      return this.label === "H" && this.isotope === 0;
    }
    isPlainCarbon() {
      return this.label === "C" && this.isotope === null && this.radical === 0 && this.charge === null && this.explicitValence < 0 && this.ringBondCount === 0 && this.substitutionCount === 0 && this.unsaturatedAtom === 0 && this.hCount === 0 && !this.atomList;
    }
    isPseudo() {
      return !this.atomList && !this.rglabel && !Elements.get(this.label);
    }
    hasRxnProps() {
      return !!(this.invRet || this.exactChangeFlag || this.attachmentPoints !== null || this.aam);
    }
    calcValence(connectionCount) {
      var _a;
      const label = this.label;
      const charge = (_a = this.charge) != null ? _a : 0;
      if (this.isQuery() || this.attachmentPoints) {
        this.implicitH = 0;
        return true;
      }
      const element = Elements.get(label);
      const radicalCount = radicalElectrons(this.radical);
      const absCharge = Math.abs(charge);
      const valenceResult = this.calculateValenceResult(element == null ? void 0 : element.group, {
        label,
        charge,
        connectionCount,
        radicalCount,
        absCharge
      });
      if (!valenceResult) {
        return true;
      }
      const hydrogenCount = this.overrideHydrogenCountIfNeeded(
        valenceResult.hydrogenCount
      );
      return this.applyValenceResult(
        valenceResult.valence,
        hydrogenCount,
        connectionCount
      );
    }
    calculateValenceResult(groupno, context) {
      if (groupno === void 0) {
        return this.calculateUndefinedGroupValence(context);
      }
      switch (groupno) {
        case 1:
          return this.calculateGroup1Valence(context);
        case 2:
          return this.calculateGroup2Valence(context);
        case 3:
          return this.calculateGroup3Valence(context);
        case 4:
          return this.calculateGroup4Valence(context);
        case 5:
          return this.calculateGroup5Valence(context);
        case 6:
          return this.calculateGroup6Valence(context);
        case 7:
          return this.calculateGroup7Valence(context);
        case 8:
          return this.calculateGroup8Valence(context);
        default:
          return {
            valence: context.connectionCount,
            hydrogenCount: 0
          };
      }
    }
    calculateUndefinedGroupValence({
      label,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "D" || label === "T") {
        return {
          valence: 1,
          hydrogenCount: 1 - radicalCount - connectionCount - absCharge
        };
      }
      this.implicitH = 0;
      return null;
    }
    calculateGroup1Valence({
      label,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "H" || label === "Li" || label === "Na" || label === "K" || label === "Rb" || label === "Cs" || label === "Fr") {
        return {
          valence: 1,
          hydrogenCount: 1 - radicalCount - connectionCount - absCharge
        };
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup2Valence({
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (connectionCount + radicalCount + absCharge === 2 || connectionCount + radicalCount + absCharge === 0) {
        return {
          valence: 2,
          hydrogenCount: 0
        };
      }
      return { valence: connectionCount, hydrogenCount: -1 };
    }
    calculateGroup3Valence({
      label,
      charge,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "B" || label === "Al" || label === "Ga" || label === "In") {
        if (charge === -1) {
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount
          };
        }
        return {
          valence: 3,
          hydrogenCount: 3 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "Tl") {
        if (charge === -1) {
          if (radicalCount + connectionCount <= 2) {
            return {
              valence: 2,
              hydrogenCount: 2 - radicalCount - connectionCount
            };
          }
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount
          };
        }
        if (charge === -2) {
          if (radicalCount + connectionCount <= 3) {
            return {
              valence: 3,
              hydrogenCount: 3 - radicalCount - connectionCount
            };
          }
          return {
            valence: 5,
            hydrogenCount: 5 - radicalCount - connectionCount
          };
        }
        if (radicalCount + connectionCount + absCharge <= 1) {
          return {
            valence: 1,
            hydrogenCount: 1 - radicalCount - connectionCount - absCharge
          };
        }
        return {
          valence: 3,
          hydrogenCount: 3 - radicalCount - connectionCount - absCharge
        };
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup4Valence({
      label,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "C" || label === "Si" || label === "Ge") {
        return {
          valence: 4,
          hydrogenCount: 4 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "Sn" || label === "Pb") {
        if (connectionCount + radicalCount + absCharge <= 2) {
          return {
            valence: 2,
            hydrogenCount: 2 - radicalCount - connectionCount - absCharge
          };
        }
        return {
          valence: 4,
          hydrogenCount: 4 - radicalCount - connectionCount - absCharge
        };
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup5Valence({
      label,
      charge,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "N" || label === "P") {
        if (charge === 1) {
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount
          };
        }
        if (charge === 2) {
          return {
            valence: 3,
            hydrogenCount: 3 - radicalCount - connectionCount
          };
        }
        if (radicalCount + connectionCount + absCharge <= 3) {
          return {
            valence: 3,
            hydrogenCount: 3 - radicalCount - connectionCount - absCharge
          };
        }
        return {
          valence: 5,
          hydrogenCount: 5 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "Bi" || label === "Sb" || label === "As") {
        if (charge === 1) {
          if (radicalCount + connectionCount <= 2 && label !== "As") {
            return {
              valence: 2,
              hydrogenCount: 2 - radicalCount - connectionCount
            };
          }
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount
          };
        }
        if (charge === 2) {
          return {
            valence: 3,
            hydrogenCount: 3 - radicalCount - connectionCount
          };
        }
        if (radicalCount + connectionCount <= 3) {
          return {
            valence: 3,
            hydrogenCount: 3 - radicalCount - connectionCount - absCharge
          };
        }
        return {
          valence: 5,
          hydrogenCount: 5 - radicalCount - connectionCount - absCharge
        };
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup6Valence({
      label,
      charge,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "O") {
        if (charge >= 1) {
          return {
            valence: 3,
            hydrogenCount: 3 - radicalCount - connectionCount
          };
        }
        return {
          valence: 2,
          hydrogenCount: 2 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "S" || label === "Se" || label === "Po") {
        if (charge === 1) {
          if (connectionCount <= 3) {
            return {
              valence: 3,
              hydrogenCount: 3 - radicalCount - connectionCount
            };
          }
          return {
            valence: 5,
            hydrogenCount: 5 - radicalCount - connectionCount
          };
        }
        if (connectionCount + radicalCount + absCharge <= 2) {
          return {
            valence: 2,
            hydrogenCount: 2 - radicalCount - connectionCount - absCharge
          };
        }
        if (connectionCount + radicalCount + absCharge <= 4) {
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount - absCharge
          };
        }
        return {
          valence: 6,
          hydrogenCount: 6 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "Te") {
        let valence = connectionCount;
        let hydrogenCount = 0;
        if ((charge === -1 || charge === 0 || charge === 2) && connectionCount <= 2) {
          valence = 2;
          hydrogenCount = 2 - radicalCount - connectionCount - absCharge;
        } else if (charge === 0 || charge === 2) {
          if (connectionCount <= 4) {
            valence = 4;
            hydrogenCount = 4 - radicalCount - connectionCount - absCharge;
          } else if (charge === 0 && connectionCount <= 6) {
            valence = 6;
            hydrogenCount = 6 - radicalCount - connectionCount - absCharge;
          } else {
            hydrogenCount = -1;
          }
        }
        return { valence, hydrogenCount };
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup7Valence({
      label,
      charge,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "F") {
        return {
          valence: 1,
          hydrogenCount: 1 - radicalCount - connectionCount - absCharge
        };
      }
      if (label === "Cl" || label === "Br" || label === "I" || label === "At") {
        if (charge === 1) {
          if (connectionCount <= 2) {
            return {
              valence: 2,
              hydrogenCount: 2 - radicalCount - connectionCount
            };
          }
          if (connectionCount === 3 || connectionCount === 5 || connectionCount >= 7) {
            return { valence: connectionCount, hydrogenCount: -1 };
          }
        } else if (charge === 0) {
          if (connectionCount <= 1) {
            return {
              valence: 1,
              hydrogenCount: 1 - radicalCount - connectionCount
            };
          }
          if (connectionCount === 2 || connectionCount === 4 || connectionCount === 6) {
            if (radicalCount === 1) {
              return { valence: connectionCount, hydrogenCount: 0 };
            }
            return { valence: connectionCount, hydrogenCount: -1 };
          }
          if (connectionCount > 7) {
            return { valence: connectionCount, hydrogenCount: -1 };
          }
        }
      }
      return { valence: connectionCount, hydrogenCount: 0 };
    }
    calculateGroup8Valence({
      label,
      connectionCount,
      radicalCount,
      absCharge
    }) {
      if (label === "Pt") {
        if (connectionCount + radicalCount + absCharge <= 2) {
          return {
            valence: 2,
            hydrogenCount: 2 - radicalCount - connectionCount - absCharge
          };
        }
        if (connectionCount + radicalCount + absCharge <= 4) {
          return {
            valence: 4,
            hydrogenCount: 4 - radicalCount - connectionCount - absCharge
          };
        }
        return { valence: connectionCount, hydrogenCount: -1 };
      }
      if (connectionCount + radicalCount + absCharge === 0) {
        return { valence: 1, hydrogenCount: 0 };
      }
      return { valence: connectionCount, hydrogenCount: -1 };
    }
    overrideHydrogenCountIfNeeded(hydrogenCount) {
      if (this.implicitHCount !== null) {
        return this.implicitHCount;
      }
      return hydrogenCount;
    }
    applyValenceResult(valence, hydrogenCount, connectionCount) {
      this.valence = valence;
      this.implicitH = hydrogenCount;
      if (this.implicitH < 0) {
        this.valence = connectionCount;
        this.implicitH = 0;
        this.badConn = true;
        return false;
      }
      return true;
    }
    calcValenceMinusHyd(conn) {
      var _a;
      const charge = (_a = this.charge) != null ? _a : 0;
      const label = this.label;
      const element = Elements.get(this.label);
      if (!element) {
        this.implicitH = 0;
        return 0;
      }
      const groupno = element.group;
      const rad = radicalElectrons(this.radical);
      if (groupno === 3) {
        if (label === "B" || label === "Al" || label === "Ga" || label === "In") {
          if (charge === -1) {
            if (rad + conn <= 4) return rad + conn;
          }
        }
      } else if (groupno === 5) {
        if ((label === "N" || label === "P" || label === "Sb" || label === "Bi" || label === "As") && (charge === 1 || charge === 2)) {
          return rad + conn;
        }
      } else if (groupno === 6) {
        if (label === "O") {
          if (charge >= 1) return rad + conn;
        } else if (label === "S" || label === "Se" || label === "Po") {
          if (charge === 1) return rad + conn;
        }
      } else if (groupno === 7) {
        if (label === "Cl" || label === "Br" || label === "I" || label === "At") {
          if (charge === 1) return rad + conn;
        }
      }
      return rad + conn + Math.abs(charge);
    }
    static getSuperAtomAttachmentPointByAttachmentAtom(struct, atomId, searchBySgroups = false) {
      const sgroup = searchBySgroups ? struct.getGroupFromAtomIdBySgroups(atomId) : struct.getGroupFromAtomId(atomId);
      return sgroup == null ? void 0 : sgroup.getAttachmentPoints().find((attachmentPoint) => attachmentPoint.atomId === atomId);
    }
    static getSuperAtomAttachmentPointByLeavingGroup(structOrSgroup, atomId, searchBySgroups = false) {
      let sgroup;
      if (_Atom.isSGroup(structOrSgroup)) {
        sgroup = structOrSgroup;
      } else if (searchBySgroups) {
        sgroup = structOrSgroup.getGroupFromAtomIdBySgroups(atomId);
      } else {
        sgroup = structOrSgroup.getGroupFromAtomId(atomId);
      }
      return sgroup == null ? void 0 : sgroup.getAttachmentPoints().find((attachmentPoint) => attachmentPoint.leaveAtomId === atomId);
    }
    static isSuperatomLeavingGroupAtom(structOrSgroup, atomId, searchBySgroups = false) {
      if (atomId === void 0) {
        return false;
      }
      return Boolean(
        _Atom.getSuperAtomAttachmentPointByLeavingGroup(
          structOrSgroup,
          atomId,
          searchBySgroups
        )
      );
    }
    static isSuperatomAttachmentAtom(struct, atomId) {
      if (atomId === void 0) {
        return false;
      }
      return Boolean(
        _Atom.getSuperAtomAttachmentPointByAttachmentAtom(struct, atomId)
      );
    }
    static getAttachmentAtomExternalConnections(struct, attachmentAtomId, leavingGroupAtomid, searchBySgroups = false) {
      const bonds = struct.bonds;
      const atomId = (0, import_lodash3.isNumber)(attachmentAtomId) ? attachmentAtomId : leavingGroupAtomid;
      const atom = struct.atoms.get(atomId);
      const attachmentPoint = (0, import_lodash3.isNumber)(attachmentAtomId) ? _Atom.getSuperAtomAttachmentPointByAttachmentAtom(
        struct,
        atomId,
        searchBySgroups
      ) : _Atom.getSuperAtomAttachmentPointByLeavingGroup(
        struct,
        atomId,
        searchBySgroups
      );
      const attachmentPointAtomBonds = attachmentPoint ? bonds.filter(
        (_, bond) => bond.begin === attachmentPoint.atomId && bond.end !== attachmentPoint.leaveAtomId || bond.end === attachmentPoint.atomId && bond.begin !== attachmentPoint.leaveAtomId
      ) : void 0;
      const attachmentAtomExternalConnection = attachmentPointAtomBonds == null ? void 0 : attachmentPointAtomBonds.filter(
        (_, bond) => {
          const beginAtom = struct.atoms.get(bond.begin);
          const endAtom = struct.atoms.get(bond.end);
          const isExternalBondBetweenMonomers = bond.isExternalBondBetweenMonomers(struct);
          return isExternalBondBetweenMonomers || (beginAtom == null ? void 0 : beginAtom.fragment) !== (atom == null ? void 0 : atom.fragment) || (endAtom == null ? void 0 : endAtom.fragment) !== (atom == null ? void 0 : atom.fragment);
        }
      );
      return attachmentAtomExternalConnection;
    }
    static isHiddenLeavingGroupAtom(struct, atomId, searchBySgroups = false, includeAtomsInCollapsedSgroups = false) {
      const atom = struct.atoms.get(atomId);
      if (atom && !includeAtomsInCollapsedSgroups && FunctionalGroup.isAtomInContractedFunctionalGroup(
        atom,
        struct.sgroups,
        struct.functionalGroups
      )) {
        return false;
      }
      const attachmentAtomExternalConnections = _Atom.getAttachmentAtomExternalConnections(
        struct,
        void 0,
        atomId,
        searchBySgroups
      );
      const attachmentPoint = _Atom.getSuperAtomAttachmentPointByLeavingGroup(
        struct,
        atomId
      );
      const sGroup = searchBySgroups ? struct.getGroupFromAtomIdBySgroups(atomId) : struct.getGroupFromAtomId(atomId);
      const isMonomer = sGroup == null ? void 0 : sGroup.isMonomer;
      if (!sGroup || !isMonomer && !(sGroup == null ? void 0 : sGroup.isSuperatomWithoutLabel)) {
        return false;
      }
      return Boolean(
        _Atom.isSuperatomLeavingGroupAtom(struct, atomId, searchBySgroups) && (attachmentAtomExternalConnections == null ? void 0 : attachmentAtomExternalConnections.find(
          (_, bond) => bond.begin === (attachmentPoint == null ? void 0 : attachmentPoint.atomId) ? bond.beginSuperatomAttachmentPointNumber === (attachmentPoint == null ? void 0 : attachmentPoint.attachmentPointNumber) : bond.endSuperatomAttachmentPointNumber === (attachmentPoint == null ? void 0 : attachmentPoint.attachmentPointNumber)
        )) !== null
      );
    }
    static isSGroup(structOrSgroup) {
      return structOrSgroup instanceof SGroup;
    }
  };
  __publicField(_Atom, "PATTERN", {
    RADICAL: {
      NONE: 0,
      SINGLET: 1,
      DOUPLET: 2,
      TRIPLET: 3
    },
    STEREO_PARITY: {
      NONE: 0,
      ODD: 1,
      EVEN: 2,
      EITHER: 3
    }
  });
  // TODO: rename
  __publicField(_Atom, "attrlist", {
    alias: null,
    label: "C",
    isotope: null,
    radical: 0,
    cip: null,
    charge: null,
    explicitValence: -1,
    ringBondCount: 0,
    substitutionCount: 0,
    unsaturatedAtom: 0,
    hCount: 0,
    queryProperties: {
      aromaticity: null,
      ringMembership: null,
      ringSize: null,
      connectivity: null,
      chirality: null,
      customQuery: null
    },
    atomList: null,
    invRet: 0,
    exactChangeFlag: 0,
    rglabel: null,
    attachmentPoints: null,
    aam: 0,
    isPreview: false,
    // enhanced stereo
    stereoLabel: null,
    stereoParity: 0,
    implicitHCount: null
  });
  var Atom2 = _Atom;
  function radicalElectrons(radical) {
    const normalizedRadical = Number(radical);
    if (normalizedRadical === Atom2.PATTERN.RADICAL.DOUPLET) return 1;
    else if (normalizedRadical === Atom2.PATTERN.RADICAL.SINGLET || normalizedRadical === Atom2.PATTERN.RADICAL.TRIPLET) {
      return 2;
    } else {
      return 0;
    }
  }
  function getValueOrDefault(value, defaultValue) {
    return typeof value !== "undefined" ? value : defaultValue;
  }
  function isCorrectPseudo(label) {
    return !Elements.get(label) && label !== "L" && label !== "L#" && label !== "R#";
  }
  function getPseudo(label) {
    return isCorrectPseudo(label) ? label : "";
  }

  // src/core/chem/bond.ts
  var _Bond = class _Bond extends BaseMicromoleculeEntity {
    constructor(attributes) {
      var _a, _b;
      super(attributes.initiallySelected);
      __publicField(this, "begin");
      __publicField(this, "end");
      __publicField(this, "type");
      __publicField(this, "xxx");
      __publicField(this, "stereo");
      __publicField(this, "topology");
      __publicField(this, "reactingCenterStatus");
      __publicField(this, "customQuery");
      __publicField(this, "len");
      __publicField(this, "sb");
      __publicField(this, "sa");
      __publicField(this, "cip");
      __publicField(this, "hb1");
      __publicField(this, "hb2");
      __publicField(this, "angle");
      __publicField(this, "center");
      __publicField(this, "isPreview");
      __publicField(this, "beginSuperatomAttachmentPointNumber");
      __publicField(this, "endSuperatomAttachmentPointNumber");
      __publicField(this, "beginSgroup");
      __publicField(this, "endSgroup");
      this.begin = attributes.begin;
      this.end = attributes.end;
      this.type = attributes.type;
      this.xxx = (_a = attributes.xxx) != null ? _a : "";
      this.stereo = _Bond.PATTERN.STEREO.NONE;
      this.topology = _Bond.PATTERN.TOPOLOGY.EITHER;
      this.customQuery = null;
      this.reactingCenterStatus = 0;
      this.cip = (_b = attributes.cip) != null ? _b : null;
      this.len = 0;
      this.sb = 0;
      this.sa = 0;
      this.angle = 0;
      this.isPreview = false;
      this.beginSuperatomAttachmentPointNumber = attributes.beginSuperatomAttachmentPointNumber;
      this.endSuperatomAttachmentPointNumber = attributes.endSuperatomAttachmentPointNumber;
      if (attributes.stereo) this.stereo = attributes.stereo;
      if (attributes.topology) this.topology = attributes.topology;
      if (attributes.customQuery) {
        this.customQuery = attributes.customQuery;
        this.type = _Bond.PATTERN.TYPE.ANY;
        this.reactingCenterStatus = null;
        this.topology = null;
      }
      if (attributes.reactingCenterStatus) {
        this.reactingCenterStatus = attributes.reactingCenterStatus;
      }
      this.center = new Vec2();
    }
    static getAttrHash(bond) {
      const attrs = {};
      for (const attr in _Bond.attrlist) {
        if (bond[attr] || attr === "stereo") {
          attrs[attr] = bond[attr];
        }
      }
      return attrs;
    }
    static getBondNeighbourIds(struct, bondId) {
      const bond = struct.bonds.get(bondId);
      const { begin, end } = bond;
      const beginBondIds = Atom2.getConnectedBondIds(struct, begin).filter(
        (id2) => id2 !== bondId
      );
      const endBondIds = Atom2.getConnectedBondIds(struct, end).filter(
        (id2) => id2 !== bondId
      );
      return { beginBondIds, endBondIds };
    }
    static getFusingConditions(bond, bondBegin, bondEnd) {
      const { DOUBLE, SINGLE } = this.PATTERN.TYPE;
      const isFusingToDoubleBond = bondBegin.type === SINGLE && bond.type === DOUBLE && bondEnd.type === SINGLE;
      const isFusingToSingleBond = bondBegin.type === DOUBLE && bond.type === SINGLE && bondEnd.type === DOUBLE;
      const isFusingDoubleSingleSingle = bondBegin.type === DOUBLE && bond.type === SINGLE && bondEnd.type === SINGLE;
      const isFusingSingleSingleDouble = bondBegin.type === SINGLE && bond.type === SINGLE && bondEnd.type === DOUBLE;
      const isAllSingle = bondBegin.type === SINGLE && bond.type === SINGLE && bondEnd.type === SINGLE;
      return {
        isFusingToSingleBond,
        isFusingToDoubleBond,
        isFusingDoubleSingleSingle,
        isFusingSingleSingleDouble,
        isAllSingle
      };
    }
    static getBenzeneConnectingBondType(bond, bondBegin, bondEnd) {
      const { DOUBLE, SINGLE } = this.PATTERN.TYPE;
      const { isFusingToSingleBond, isFusingToDoubleBond } = _Bond.getFusingConditions(bond, bondBegin, bondEnd);
      if (isFusingToDoubleBond) {
        return DOUBLE;
      } else if (isFusingToSingleBond) {
        return SINGLE;
      }
      return null;
    }
    static getCyclopentadieneFusingBondType(bond, bondBegin, bondEnd) {
      const { DOUBLE, SINGLE } = this.PATTERN.TYPE;
      const {
        isFusingToSingleBond,
        isFusingToDoubleBond,
        isFusingDoubleSingleSingle,
        isAllSingle
      } = _Bond.getFusingConditions(bond, bondBegin, bondEnd);
      if (isFusingToDoubleBond) {
        return DOUBLE;
      } else if (isFusingToSingleBond || isAllSingle || isFusingDoubleSingleSingle) {
        return SINGLE;
      }
      return null;
    }
    static getCyclopentadieneDoubleBondIndexes(bond, bondBegin, bondEnd) {
      const {
        isFusingToSingleBond,
        isFusingToDoubleBond,
        isFusingDoubleSingleSingle
      } = _Bond.getFusingConditions(bond, bondBegin, bondEnd);
      if (isFusingToSingleBond || isFusingToDoubleBond) {
        return [3];
      }
      if (isFusingDoubleSingleSingle) {
        return [2, 4];
      }
      return [1, 3];
    }
    static attrGetDefault(attr) {
      if (attr in _Bond.attrlist) {
        return _Bond.attrlist[attr];
      }
    }
    isQuery() {
      const TYPES = _Bond.PATTERN.TYPE;
      const QUERY_BOND_TYPES = [
        TYPES.ANY,
        TYPES.SINGLE_OR_DOUBLE,
        TYPES.SINGLE_OR_AROMATIC,
        TYPES.DOUBLE_OR_AROMATIC,
        TYPES.AROMATIC
      ];
      return this.customQuery !== null || QUERY_BOND_TYPES.includes(this.type) || TYPES.SINGLE === this.type && this.stereo === _Bond.PATTERN.STEREO.EITHER;
    }
    hasRxnProps() {
      return !!this.reactingCenterStatus;
    }
    getCenter(struct) {
      const p1 = struct.atoms.get(this.begin).pp;
      const p2 = struct.atoms.get(this.end).pp;
      return Vec2.lc2(p1, 0.5, p2, 0.5);
    }
    getDir(struct) {
      const p1 = struct.atoms.get(this.begin).pp;
      const p2 = struct.atoms.get(this.end).pp;
      return p2.sub(p1).normalized();
    }
    clone(aidMap) {
      const cp = new _Bond(this);
      if (aidMap) {
        cp.begin = aidMap.get(cp.begin);
        cp.end = aidMap.get(cp.end);
      }
      return cp;
    }
    getAttachedSGroups(struct) {
      var _a, _b, _c, _d;
      const sGroupsWithBeginAtom = (_b = (_a = struct.atoms.get(this.begin)) == null ? void 0 : _a.sgs) != null ? _b : new Pile();
      const sGroupsWithEndAtom = (_d = (_c = struct.atoms.get(this.end)) == null ? void 0 : _c.sgs) != null ? _d : new Pile();
      return sGroupsWithBeginAtom == null ? void 0 : sGroupsWithBeginAtom.intersection(sGroupsWithEndAtom);
    }
    isExternalBondBetweenMonomers(struct) {
      if (!struct.isBondFromMacromolecule(this)) {
        return false;
      }
      const sGroup1 = struct.getGroupFromAtomId(this.begin);
      const sGroup2 = struct.getGroupFromAtomId(this.end);
      if (!sGroup1 || !sGroup2) {
        return false;
      }
      return sGroup1 !== sGroup2;
    }
    static isBondToHiddenLeavingGroup(struct, bond, includeAtomsInCollapsedSgroups = false) {
      const beginSuperatomAttachmentPoint = Atom2.getSuperAtomAttachmentPointByLeavingGroup(struct, bond.begin);
      const endSuperatomAttachmentPoint = Atom2.getSuperAtomAttachmentPointByLeavingGroup(struct, bond.end);
      return beginSuperatomAttachmentPoint && Atom2.isHiddenLeavingGroupAtom(
        struct,
        bond.begin,
        false,
        includeAtomsInCollapsedSgroups
      ) && bond.end === beginSuperatomAttachmentPoint.atomId || endSuperatomAttachmentPoint && Atom2.isHiddenLeavingGroupAtom(
        struct,
        bond.end,
        false,
        includeAtomsInCollapsedSgroups
      ) && bond.begin === endSuperatomAttachmentPoint.atomId;
    }
    static isBondToExpandedMonomer(struct, bond) {
      return [...struct.sgroups.values()].some((sgroup) => {
        return (sgroup.atoms.includes(bond.begin) && !sgroup.atoms.includes(bond.end) || sgroup.atoms.includes(bond.end) && !sgroup.atoms.includes(bond.begin)) && sgroup.isExpanded() && sgroup.isMonomer;
      });
    }
  };
  __publicField(_Bond, "PATTERN", {
    TYPE: {
      SINGLE: 1,
      DOUBLE: 2,
      TRIPLE: 3,
      AROMATIC: 4,
      SINGLE_OR_DOUBLE: 5,
      SINGLE_OR_AROMATIC: 6,
      DOUBLE_OR_AROMATIC: 7,
      ANY: 8,
      DATIVE: 9,
      HYDROGEN: 10
    },
    STEREO: {
      NONE: 0,
      UP: 1,
      EITHER: 4,
      DOWN: 6,
      CIS_TRANS: 3
    },
    TOPOLOGY: {
      EITHER: 0,
      RING: 1,
      CHAIN: 2
    },
    REACTING_CENTER: {
      NOT_CENTER: -1,
      UNMARKED: 0,
      CENTER: 1,
      UNCHANGED: 2,
      MADE_OR_BROKEN: 4,
      ORDER_CHANGED: 8,
      MADE_OR_BROKEN_AND_CHANGED: 12
    }
  });
  __publicField(_Bond, "attrlist", {
    type: _Bond.PATTERN.TYPE.SINGLE,
    stereo: _Bond.PATTERN.STEREO.NONE,
    topology: _Bond.PATTERN.TOPOLOGY.EITHER,
    reactingCenterStatus: _Bond.PATTERN.REACTING_CENTER.UNMARKED,
    cip: null,
    customQuery: null
  });
  var Bond3 = _Bond;

  // domain/entities/fixedPrecision.ts
  var _FixedPrecisionCoordinates = class _FixedPrecisionCoordinates {
    constructor(value) {
      __publicField(this, "value");
      this.value = value instanceof _FixedPrecisionCoordinates ? value.value : value;
    }
    static fromFloatingPrecision(value) {
      return new _FixedPrecisionCoordinates(
        Math.round(value * _FixedPrecisionCoordinates.MULTIPLIER)
      );
    }
    add(fixedPrecisionValue) {
      return new _FixedPrecisionCoordinates(
        this.value + fixedPrecisionValue.value
      );
    }
    sub(fixedPrecisionValue) {
      return new _FixedPrecisionCoordinates(
        this.value - fixedPrecisionValue.value
      );
    }
    multiply(value) {
      const isFixedPrecision = value instanceof _FixedPrecisionCoordinates;
      const multiplier = isFixedPrecision ? value.value : value;
      const result = this.value * multiplier;
      return new _FixedPrecisionCoordinates(
        Math.round(
          isFixedPrecision ? result / _FixedPrecisionCoordinates.MULTIPLIER : result
        )
      );
    }
    divide(value) {
      const isFixedPrecision = value instanceof _FixedPrecisionCoordinates;
      const delimiter = isFixedPrecision ? value.value : value;
      const result = this.value / delimiter;
      return new _FixedPrecisionCoordinates(
        Math.round(
          isFixedPrecision ? result * _FixedPrecisionCoordinates.MULTIPLIER : result
        )
      );
    }
    getFloatingPrecision() {
      return this.value / _FixedPrecisionCoordinates.MULTIPLIER;
    }
  };
  __publicField(_FixedPrecisionCoordinates, "MULTIPLIER", 10 ** 5);
  var FixedPrecisionCoordinates = _FixedPrecisionCoordinates;

  // domain/entities/fragment.ts
  function calcStereoFlag(struct, stereoAids) {
    var _a;
    if (!stereoAids || stereoAids.length === 0) return void 0;
    const filteredStereoAtoms = stereoAids.map((aid) => struct.atoms.get(aid)).filter((atom2) => atom2 == null ? void 0 : atom2.stereoLabel);
    if (!filteredStereoAtoms.length) return void 0;
    const atom = filteredStereoAtoms[0];
    const stereoLabel = atom.stereoLabel;
    const hasAnotherLabel = filteredStereoAtoms.some(
      (atom2) => (atom2 == null ? void 0 : atom2.stereoLabel) !== stereoLabel
    );
    let stereoFlag;
    if (hasAnotherLabel) {
      stereoFlag = "MIXED" /* Mixed */;
    } else {
      const label = (_a = stereoLabel.match(/\D+/g)) == null ? void 0 : _a[0];
      switch (label) {
        case "abs" /* Abs */: {
          stereoFlag = "ABS" /* Abs */;
          break;
        }
        case "&" /* And */: {
          stereoFlag = "AND" /* And */;
          break;
        }
        case "or" /* Or */: {
          stereoFlag = "OR" /* Or */;
          break;
        }
        default: {
          throw String(`Unsupported stereo label: ${label}.`);
        }
      }
    }
    return stereoFlag;
  }
  var _enhancedStereoFlag, _stereoAtoms;
  var _Fragment = class _Fragment {
    constructor(stereoAtoms = [], stereoFlagPosition, properties) {
      __privateAdd(this, _enhancedStereoFlag);
      __publicField(this, "stereoFlagPosition");
      __publicField(this, "properties");
      __privateAdd(this, _stereoAtoms);
      if (stereoFlagPosition) {
        this.stereoFlagPosition = new Vec2(stereoFlagPosition);
      }
      if (properties) {
        this.properties = properties;
      }
      __privateSet(this, _stereoAtoms, stereoAtoms);
    }
    get stereoAtoms() {
      return [...__privateGet(this, _stereoAtoms)];
    }
    get enhancedStereoFlag() {
      return __privateGet(this, _enhancedStereoFlag);
    }
    static getDefaultStereoFlagPosition(struct, fragmentId) {
      const fragment = struct.getFragment(fragmentId);
      if (!fragment) return void 0;
      const bb = fragment.getCoordBoundingBox();
      return new Vec2(bb.max.x, bb.min.y - 1);
    }
    clone(aidMap) {
      const stereoAtoms = __privateGet(this, _stereoAtoms).map((aid) => aidMap.get(aid));
      const fr = new _Fragment(
        stereoAtoms,
        this.stereoFlagPosition,
        this.properties
      );
      __privateSet(fr, _enhancedStereoFlag, __privateGet(this, _enhancedStereoFlag));
      return fr;
    }
    updateStereoFlag(struct) {
      __privateSet(this, _enhancedStereoFlag, calcStereoFlag(struct, this.stereoAtoms));
      return __privateGet(this, _enhancedStereoFlag);
    }
    // TODO: split to 'add' and 'remove methods
    updateStereoAtom(struct, aid, frId, isAdd) {
      var _a;
      if (isAdd && !__privateGet(this, _stereoAtoms).includes(aid)) __privateGet(this, _stereoAtoms).push(aid);
      if (!isAdd && (((_a = struct.atoms.get(aid)) == null ? void 0 : _a.fragment) !== frId || !Array.from(struct.bonds.values()).filter(
        (bond) => bond.stereo && bond.type !== Bond3.PATTERN.TYPE.DOUBLE
      ).some((bond) => bond.begin === aid))) {
        __privateSet(this, _stereoAtoms, this.stereoAtoms.filter((item) => item !== aid));
      }
      __privateSet(this, _enhancedStereoFlag, calcStereoFlag(struct, this.stereoAtoms));
    }
    addStereoAtom(atomId) {
      if (!__privateGet(this, _stereoAtoms).includes(atomId)) {
        this.stereoAtoms.push(atomId);
        return true;
      }
      return false;
    }
    deleteStereoAtom(struct, fragmentId, atomId) {
      var _a;
      if (((_a = struct.atoms.get(atomId)) == null ? void 0 : _a.fragment) !== fragmentId || !Array.from(struct.bonds.values()).filter((bond) => bond.stereo && bond.type !== Bond3.PATTERN.TYPE.DOUBLE).some((bond) => bond.begin === atomId)) {
        __privateSet(this, _stereoAtoms, __privateGet(this, _stereoAtoms).filter((item) => item !== atomId));
        return true;
      }
      return false;
    }
  };
  _enhancedStereoFlag = new WeakMap();
  _stereoAtoms = new WeakMap();
  var Fragment = _Fragment;

  // domain/entities/halfBond.ts
  var HalfBond = class {
    constructor(begin, end, bid) {
      __publicField(this, "begin");
      __publicField(this, "end");
      __publicField(this, "bid");
      __publicField(this, "dir");
      __publicField(this, "norm");
      __publicField(this, "ang");
      __publicField(this, "p");
      __publicField(this, "loop");
      __publicField(this, "contra");
      __publicField(this, "next");
      __publicField(this, "leftSin");
      __publicField(this, "leftCos");
      __publicField(this, "leftNeighbor");
      __publicField(this, "rightSin");
      __publicField(this, "rightCos");
      __publicField(this, "rightNeighbor");
      assert_default(arguments.length === 3, "Invalid parameter number.");
      this.begin = begin;
      this.end = end;
      this.bid = bid;
      this.dir = new Vec2();
      this.norm = new Vec2();
      this.ang = 0;
      this.p = new Vec2();
      this.loop = -1;
      this.contra = -1;
      this.next = -1;
      this.leftSin = 0;
      this.leftCos = 0;
      this.leftNeighbor = 0;
      this.rightSin = 0;
      this.rightCos = 0;
      this.rightNeighbor = 0;
    }
  };

  // domain/entities/loop.ts
  var Loop = class {
    constructor(hbs, struct, isConvex) {
      __publicField(this, "hbs");
      __publicField(this, "dblBonds");
      __publicField(this, "aromatic");
      __publicField(this, "convex");
      this.hbs = hbs;
      this.dblBonds = 0;
      this.aromatic = true;
      this.convex = isConvex || false;
      hbs.forEach((hb) => {
        const bond = struct.bonds.get(struct.halfBonds.get(hb).bid);
        if (bond.type !== Bond3.PATTERN.TYPE.AROMATIC) this.aromatic = false;
        if (bond.type === Bond3.PATTERN.TYPE.DOUBLE) this.dblBonds++;
      });
    }
  };

  // domain/entities/rgroup.ts
  var RGroup = class _RGroup {
    constructor(atrributes) {
      __publicField(this, "frags");
      __publicField(this, "resth");
      __publicField(this, "range");
      __publicField(this, "ifthen");
      __publicField(this, "index");
      var _a, _b, _c, _d;
      this.frags = new Pile();
      this.resth = (_a = atrributes == null ? void 0 : atrributes.resth) != null ? _a : false;
      this.range = (_b = atrributes == null ? void 0 : atrributes.range) != null ? _b : "";
      this.ifthen = (_c = atrributes == null ? void 0 : atrributes.ifthen) != null ? _c : 0;
      this.index = (_d = atrributes == null ? void 0 : atrributes.index) != null ? _d : -1;
    }
    static findRGroupByFragment(rgroups, frid) {
      return rgroups.find((_rgid, rgroup) => rgroup.frags.has(frid));
    }
    getAttrs() {
      return {
        resth: this.resth,
        range: this.range,
        ifthen: this.ifthen,
        index: this.index
      };
    }
    clone(fidMap) {
      const ret = new _RGroup(this);
      this.frags.forEach((fid) => {
        if (!fidMap || fidMap.has(fid)) {
          ret.frags.add(fidMap ? fidMap.get(fid) : fid);
        }
      });
      return ret;
    }
  };

  // domain/entities/rgroupAttachmentPoint.ts
  var RGroupAttachmentPoint = class _RGroupAttachmentPoint extends BaseMicromoleculeEntity {
    constructor(atomId, type, initiallySelected) {
      super(initiallySelected);
      __publicField(this, "atomId");
      __publicField(this, "type");
      this.atomId = atomId;
      this.type = type;
    }
    clone(atomToNewAtom) {
      const newAtomId = atomToNewAtom == null ? void 0 : atomToNewAtom.get(this.atomId);
      return new _RGroupAttachmentPoint(
        newAtomId != null ? newAtomId : this.atomId,
        this.type,
        this.initiallySelected
      );
    }
  };

  // domain/entities/rxnArrow.ts
  var RxnArrow = class _RxnArrow extends BaseMicromoleculeEntity {
    constructor(attributes) {
      var _a;
      super(attributes == null ? void 0 : attributes.initiallySelected);
      __publicField(this, "mode");
      __publicField(this, "pos");
      __publicField(this, "height");
      __publicField(this, "arrowId");
      this.pos = [];
      this.arrowId = attributes.arrowId;
      if (attributes.pos) {
        for (let i = 0; i < attributes.pos.length; i++) {
          const currentP = attributes.pos[i];
          this.pos[i] = currentP ? new Vec2(attributes.pos[i]) : new Vec2();
        }
      }
      this.mode = attributes.mode;
      const defaultHeight = 1;
      if (_RxnArrow.isElliptical(this)) {
        this.height = (_a = attributes.height) != null ? _a : defaultHeight;
      }
    }
    static isElliptical(arrow) {
      return [
        "elliptical-arc-arrow-filled-bow" /* EllipticalArcFilledBow */,
        "elliptical-arc-arrow-filled-triangle" /* EllipticalArcFilledTriangle */,
        "elliptical-arc-arrow-open-half-angle" /* EllipticalArcOpenHalfAngle */,
        "elliptical-arc-arrow-open-angle" /* EllipticalArcOpenAngle */
      ].includes(arrow.mode);
    }
    clone() {
      return new _RxnArrow(this);
    }
    center() {
      return Vec2.centre(this.pos[0], this.pos[1]);
    }
  };

  // domain/entities/rxnPlus.ts
  var RxnPlus = class _RxnPlus extends BaseMicromoleculeEntity {
    constructor(attributes) {
      super(attributes == null ? void 0 : attributes.initiallySelected);
      __publicField(this, "pp");
      this.pp = (attributes == null ? void 0 : attributes.pp) ? new Vec2(attributes.pp) : new Vec2();
    }
    clone() {
      return new _RxnPlus(this);
    }
  };

  // domain/entities/sgroupForest.ts
  var import_utilities2 = __toESM(require_utilities());
  var SGroupForest = class {
    constructor() {
      /** node id -> parent id */
      __publicField(this, "parent");
      /** node id -> list of child ids */
      __publicField(this, "children");
      __publicField(this, "atomSets");
      this.parent = /* @__PURE__ */ new Map();
      this.children = /* @__PURE__ */ new Map();
      this.children.set(-1, []);
      this.atomSets = /* @__PURE__ */ new Map();
    }
    /** returns an array or s-group ids in the order of breadth-first search */
    getSGroupsBFS() {
      const order = [];
      const queue = Array.from(this.children.get(-1));
      while (queue.length > 0) {
        const id2 = queue.shift();
        if (typeof id2 !== "number") {
          break;
        }
        const children = this.children.get(id2);
        if (typeof children === "undefined") {
          break;
        }
        children.forEach((id3) => {
          queue.push(id3);
        });
        order.push(id2);
      }
      return order;
    }
    getAtomSetRelations(newId, atoms) {
      const isStrictSuperset = /* @__PURE__ */ new Map();
      const isSubset = /* @__PURE__ */ new Map();
      this.atomSets.delete(newId);
      this.atomSets.forEach((atomSet, id2) => {
        isSubset.set(id2, atomSet.isSuperset(atoms));
        isStrictSuperset.set(
          id2,
          atoms.isSuperset(atomSet) && !atomSet.equals(atoms)
        );
      });
      const parents = Array.from(this.atomSets.keys()).filter((sgid) => {
        if (!isSubset.get(sgid)) {
          return false;
        }
        const childs = this.children.get(sgid);
        return childs && childs.findIndex((childId) => isSubset.get(childId)) < 0;
      });
      const children = Array.from(this.atomSets.keys()).filter(
        (id2) => isStrictSuperset.get(id2) && !isStrictSuperset.get(this.parent.get(id2))
      );
      return {
        children,
        parent: parents.length === 0 ? -1 : parents[0]
      };
    }
    getPathToRoot(sgid) {
      const path = [];
      for (let id2 = sgid; typeof id2 === "number" && id2 >= 0; id2 = this.parent.get(id2)) {
        path.push(id2);
      }
      return path;
    }
    insert({ id: id2, atoms }, parent, children) {
      var _a;
      assert_default(!this.parent.has(id2), "sgid already present in the forest");
      assert_default(!this.children.has(id2), "sgid already present in the forest");
      if (!parent || !children) {
        const guess = this.getAtomSetRelations(id2, new Pile(atoms));
        parent = guess.parent;
        children = guess.children;
      }
      children.forEach((childId) => {
        this.resetParentLink(childId, id2);
      });
      this.children.set(
        id2,
        children.filter((id3) => this.parent.get(id3))
      );
      this.parent.set(id2, parent);
      (_a = this.children.get(parent)) == null ? void 0 : _a.push(id2);
      this.atomSets.set(id2, new Pile(atoms));
      return { parent, children };
    }
    resetParentLink(childId, id2) {
      const parentId = this.parent.get(childId);
      if (typeof parentId === "undefined") {
        return;
      }
      const childs = this.children.get(parentId);
      if (!childs) {
        return;
      }
      const childIndex = childs.indexOf(childId);
      childs.splice(childIndex, 1);
      this.parent.set(childId, id2);
    }
    remove(id2) {
      var _a;
      try {
        assert_default(this.parent.has(id2), "sgid is not in the forest");
        assert_default(this.children.has(id2), "sgid is not in the forest");
      } catch (e) {
        import_utilities2.sketchLogger.error("sgroupForest.ts::SGroupForest::remove", e);
        console.info("error: sgid is not in the forest");
      }
      const parentId = this.parent.get(id2);
      if (typeof parentId === "undefined") return;
      const childs = this.children.get(parentId);
      if (!childs) return;
      (_a = this.children.get(id2)) == null ? void 0 : _a.forEach((childId) => {
        var _a2;
        this.parent.set(childId, parentId);
        (_a2 = this.children.get(parentId)) == null ? void 0 : _a2.push(childId);
      });
      const i = childs.indexOf(id2);
      childs.splice(i, 1);
      this.children.delete(id2);
      this.parent.delete(id2);
      this.atomSets.delete(id2);
    }
  };

  // domain/entities/simpleObject.ts
  var SimpleObject = class _SimpleObject extends BaseMicromoleculeEntity {
    constructor(attributes) {
      var _a;
      super(attributes == null ? void 0 : attributes.initiallySelected);
      __publicField(this, "pos");
      __publicField(this, "mode");
      this.pos = [];
      if (attributes == null ? void 0 : attributes.pos) {
        for (let i = 0; i < attributes.pos.length; i++) {
          const currentP = attributes.pos[i];
          this.pos[i] = currentP ? new Vec2(attributes.pos[i]) : new Vec2();
        }
      }
      this.mode = (_a = attributes == null ? void 0 : attributes.mode) != null ? _a : "line" /* line */;
    }
    clone() {
      return new _SimpleObject(this);
    }
    center() {
      if (this.mode === "rectangle" /* rectangle */) {
        return Vec2.centre(this.pos[0], this.pos[1]);
      }
      return this.pos[0];
    }
  };

  // domain/entities/pool.ts
  var Pool = class _Pool extends Map {
    constructor() {
      super(...arguments);
      __publicField(this, "nextId", 0);
    }
    add(item) {
      const id2 = this.nextId++;
      super.set(id2, item);
      return id2;
    }
    newId() {
      return this.nextId++;
    }
    keyOf(item) {
      for (const [key, value] of this.entries()) {
        if (value === item) return key;
      }
      return null;
    }
    find(predicate) {
      for (const [key, value] of this.entries()) {
        if (predicate(key, value)) return key;
      }
      return null;
    }
    filter(predicate) {
      const result = new _Pool(
        Array.from(this).filter(([key, value]) => predicate(key, value))
      );
      result.nextId = this.nextId;
      return result;
    }
    some(predicate) {
      for (const value of this.values()) {
        if (predicate(value)) {
          return true;
        }
      }
      return false;
    }
    changeInitiallySelectedPropertiesForPool(invalidate) {
      this.forEach((value, key) => {
        if (typeof value.resetInitiallySelected === "function") {
          value.resetInitiallySelected(invalidate);
          this.set(key, value);
        }
      });
    }
    clone() {
      const newPool = new _Pool(this);
      newPool.nextId = this.nextId;
      return newPool;
    }
  };

  // domain/entities/monomerMicromolecule.ts
  var MonomerMicromolecule = class _MonomerMicromolecule extends SGroup {
    constructor(type, monomer) {
      super(type);
      this.monomer = monomer;
      this.data.absolute = false;
      this.data.attached = false;
    }
    get isMonomer() {
      return true;
    }
    getContractedPosition(struct) {
      assert_default(this.pp);
      const sgroupContractedPosition = super.getContractedPosition(struct);
      return { position: this.pp, atomId: sgroupContractedPosition.atomId };
    }
    static clone(monomerMicromolecule, atomIdMap, needCloneAttachmentPoints = false) {
      const monomerMicromoleculeClone = new _MonomerMicromolecule(
        monomerMicromolecule.type,
        monomerMicromolecule.monomer
      );
      monomerMicromoleculeClone.pp = monomerMicromolecule.pp;
      monomerMicromoleculeClone.atoms = atomIdMap ? monomerMicromolecule.atoms.map((elem) => atomIdMap.get(elem)) : monomerMicromolecule.atoms;
      monomerMicromoleculeClone.data.expanded = monomerMicromolecule.isExpanded();
      monomerMicromoleculeClone.data.name = monomerMicromolecule.data.name;
      if (needCloneAttachmentPoints && atomIdMap) {
        monomerMicromoleculeClone.addAttachmentPoints(
          monomerMicromolecule.cloneAttachmentPoints(atomIdMap),
          false
        );
      }
      return monomerMicromoleculeClone;
    }
  };

  // src/core/chem/struct.ts
  var import_lodash4 = __toESM(require_lodash2());

  // domain/helpers/stereo.ts
  function resetStereoAtomIfNotCorrect(stereoAtomsMap, correctAtomIds, atomId) {
    if (!correctAtomIds.includes(atomId)) {
      stereoAtomsMap.set(atomId, {
        stereoParity: Atom2.PATTERN.STEREO_PARITY.NONE,
        stereoLabel: null
      });
    }
  }
  function getStereoAtomsMap(struct, bonds, bond) {
    const stereoAtomsMap = /* @__PURE__ */ new Map();
    const correctAtomIds = [];
    bonds.forEach((bond2) => {
      var _a, _b;
      if (bond2) {
        const beginNeighs = struct.atomGetNeighbors(
          bond2.begin
        );
        const endNeighs = struct.atomGetNeighbors(
          bond2.end
        );
        if (StereoValidator.isCorrectStereoCenter(
          bond2,
          beginNeighs,
          endNeighs,
          struct
        )) {
          const stereoLabel = (_a = struct.atoms.get(bond2.begin)) == null ? void 0 : _a.stereoLabel;
          if (stereoLabel == null || ((_b = stereoAtomsMap.get(bond2.begin)) == null ? void 0 : _b.stereoLabel) == null) {
            stereoAtomsMap.set(bond2.begin, {
              stereoParity: getStereoParity(bond2.stereo),
              stereoLabel: stereoLabel != null ? stereoLabel : null
            });
          }
          correctAtomIds.push(bond2.begin);
        } else {
          resetStereoAtomIfNotCorrect(stereoAtomsMap, correctAtomIds, bond2.begin);
          resetStereoAtomIfNotCorrect(stereoAtomsMap, correctAtomIds, bond2.end);
        }
      }
    });
    if (bond) {
      resetStereoAtomIfNotCorrect(stereoAtomsMap, correctAtomIds, bond.begin);
      resetStereoAtomIfNotCorrect(stereoAtomsMap, correctAtomIds, bond.end);
    }
    return stereoAtomsMap;
  }
  function getStereoParity(stereo) {
    let newAtomParity = null;
    switch (stereo) {
      case Bond3.PATTERN.STEREO.UP:
        newAtomParity = Atom2.PATTERN.STEREO_PARITY.ODD;
        break;
      case Bond3.PATTERN.STEREO.EITHER:
        newAtomParity = Atom2.PATTERN.STEREO_PARITY.EITHER;
        break;
      case Bond3.PATTERN.STEREO.DOWN:
        newAtomParity = Atom2.PATTERN.STEREO_PARITY.EVEN;
        break;
    }
    return newAtomParity;
  }

  // domain/helpers/geometry.ts
  var rotateDelta = (v, center, angle) => {
    let v1 = v.sub(center);
    v1 = v1.rotate(angle);
    v1.add_(center);
    return v1.sub(v);
  };
  var flipPointByCenter = (pointToFlip, center, flipDirection) => {
    const d = new Vec2();
    if (flipDirection === "horizontal") {
      d.x = center.x > pointToFlip.x ? 2 * (center.x - pointToFlip.x) : -2 * (pointToFlip.x - center.x);
    } else {
      d.y = center.y > pointToFlip.y ? 2 * (center.y - pointToFlip.y) : -2 * (pointToFlip.y - center.y);
    }
    return d;
  };

  // domain/helpers/getAttachmentPointStereoBond.ts
  function getAttachmentPointStereoBond(sGroup, sGroupAttachmentPoint) {
    var _a;
    if (!sGroup.isMonomer) {
      return null;
    }
    const monomer = sGroup.monomer;
    if (!(monomer == null ? void 0 : monomer.monomerItem)) {
      return null;
    }
    const monomerStruct = monomer.monomerItem.struct;
    const monomerAttachmentPoints = monomer.monomerItem.attachmentPoints;
    if (!monomerStruct || !monomerAttachmentPoints) {
      return null;
    }
    const attachmentPointNumber = sGroupAttachmentPoint.attachmentPointNumber;
    if (!attachmentPointNumber) {
      return null;
    }
    const attachmentPointLabel = getAttachmentPointLabel(attachmentPointNumber);
    const orderedAttachmentPoints = monomer.listOfAttachmentPoints;
    const attachmentPointIndex = orderedAttachmentPoints.indexOf(attachmentPointLabel);
    if (attachmentPointIndex === -1 || attachmentPointIndex >= monomerAttachmentPoints.length) {
      return null;
    }
    const monomerAttachmentPoint = monomerAttachmentPoints[attachmentPointIndex];
    if (!monomerAttachmentPoint) {
      return null;
    }
    const monomerAttachmentAtomId = monomerAttachmentPoint.attachmentAtom;
    const monomerLeavingGroupAtoms = (_a = monomerAttachmentPoint.leavingGroup) == null ? void 0 : _a.atoms;
    if (monomerAttachmentAtomId === void 0 || !monomerLeavingGroupAtoms || monomerLeavingGroupAtoms.length === 0) {
      return null;
    }
    for (const internalLeavingAtomId of monomerLeavingGroupAtoms) {
      const bondId = monomerStruct.findBondId(
        internalLeavingAtomId,
        monomerAttachmentAtomId
      );
      if (bondId === null) {
        continue;
      }
      const bond = monomerStruct.bonds.get(bondId);
      if (!bond) {
        continue;
      }
      const isSuitableStereoBond = bond.stereo === Bond3.PATTERN.STEREO.UP || bond.stereo === Bond3.PATTERN.STEREO.DOWN;
      if (!isSuitableStereoBond) {
        continue;
      }
      if (bond.begin === monomerAttachmentAtomId) {
        return bond.stereo;
      }
    }
    return null;
  }

  // src/core/chem/struct.ts
  function arrayAddIfMissing(array, item) {
    for (const arrayItem of array) {
      if (arrayItem === item) return false;
    }
    array.push(item);
    return true;
  }
  var Struct = class _Struct {
    constructor() {
      __publicField(this, "atoms");
      __publicField(this, "bonds");
      __publicField(this, "sgroups");
      __publicField(this, "halfBonds");
      __publicField(this, "loops");
      __publicField(this, "isReaction");
      __publicField(this, "rxnArrows");
      __publicField(this, "rxnPluses");
      __publicField(this, "frags");
      __publicField(this, "rgroups");
      __publicField(this, "rgroupAttachmentPoints");
      __publicField(this, "name");
      __publicField(this, "abbreviation");
      __publicField(this, "sGroupForest");
      __publicField(this, "simpleObjects");
      __publicField(this, "texts");
      __publicField(this, "functionalGroups");
      __publicField(this, "highlights");
      __publicField(this, "images", new Pool());
      __publicField(this, "multitailArrows", new Pool());
      __publicField(this, "nextArrowId", 0);
      this.atoms = new Pool();
      this.bonds = new Pool();
      this.sgroups = new Pool();
      this.halfBonds = new Pool();
      this.loops = new Pool();
      this.isReaction = false;
      this.rxnArrows = new Pool();
      this.rxnPluses = new Pool();
      this.frags = new Pool();
      this.rgroups = new Pool();
      this.rgroupAttachmentPoints = new Pool();
      this.name = "";
      this.abbreviation = "";
      this.sGroupForest = new SGroupForest();
      this.simpleObjects = new Pool();
      this.texts = new Pool();
      this.functionalGroups = new Pool();
      this.highlights = new Pool();
    }
    syncNextArrowId(arrowId) {
      this.nextArrowId = Math.max(this.nextArrowId, arrowId + 1);
    }
    ensureArrowId(arrow) {
      var _a;
      const arrowId = (_a = arrow.arrowId) != null ? _a : this.nextArrowId;
      arrow.arrowId = arrowId;
      this.syncNextArrowId(arrowId);
      return arrow;
    }
    addRxnArrow(item) {
      this.ensureArrowId(item);
      return this.rxnArrows.add(item);
    }
    setRxnArrow(id2, item) {
      this.ensureArrowId(item);
      this.rxnArrows.set(id2, item);
    }
    addMultitailArrow(item) {
      this.ensureArrowId(item);
      return this.multitailArrows.add(item);
    }
    setMultitailArrow(id2, item) {
      this.ensureArrowId(item);
      this.multitailArrows.set(id2, item);
    }
    hasRxnProps() {
      var _a;
      return !!((_a = this.atoms.find((_aid, atom) => atom.hasRxnProps())) != null ? _a : this.bonds.find((_bid, bond) => bond.hasRxnProps()));
    }
    hasRxnArrow() {
      return this.rxnArrows.size >= 1;
    }
    hasMultitailArrow() {
      return this.multitailArrows.size >= 1;
    }
    hasRxnPluses() {
      return this.rxnPluses.size > 0;
    }
    isRxn() {
      return this.hasRxnArrow() || this.hasRxnPluses();
    }
    isBlank() {
      return this.atoms.size === 0 && this.rxnArrows.size === 0 && this.rxnPluses.size === 0 && this.simpleObjects.size === 0 && this.texts.size === 0 && this.images.size === 0 && this.multitailArrows.size === 0;
    }
    isSingleGroup() {
      if (!this.sgroups.size || this.sgroups.size > 1) return false;
      const sgroup = this.sgroups.values().next().value;
      return sgroup !== void 0 && this.atoms.size === sgroup.atoms.length;
    }
    clone(atomSet, bondSet, dropRxnSymbols, aidMap, simpleObjectsSet, textsSet, rgroupAttachmentPointSet, imagesSet, multitailArrowsSet, bidMap, needCloneAttachmentPoints = false) {
      const cloneStruct = this.mergeInto(
        new _Struct(),
        atomSet,
        bondSet,
        dropRxnSymbols,
        false,
        aidMap,
        simpleObjectsSet,
        textsSet,
        rgroupAttachmentPointSet,
        imagesSet,
        multitailArrowsSet,
        bidMap,
        needCloneAttachmentPoints
      );
      cloneStruct.findConnectedComponents();
      cloneStruct.setImplicitHydrogen(void 0, true);
      cloneStruct.setStereoLabelsToAtoms();
      cloneStruct.markFragments();
      return cloneStruct;
    }
    getScaffold() {
      const atomSet = new Pile();
      this.atoms.forEach((_atom, aid) => {
        atomSet.add(aid);
      });
      this.rgroups.forEach((rg) => {
        rg.frags.forEach((_fnum, fid) => {
          this.atoms.forEach((atom, aid) => {
            if (atom.fragment === fid) atomSet.delete(aid);
          });
        });
      });
      return this.clone(atomSet);
    }
    getFragmentIds(_fid) {
      const atomSet = new Pile();
      const fid = Array.isArray(_fid) ? _fid : [_fid];
      this.atoms.forEach((atom, aid) => {
        if (fid.includes(atom.fragment)) atomSet.add(aid);
      });
      return atomSet;
    }
    getFragment(fid, aidMap) {
      return this.clone(this.getFragmentIds(fid), null, true, aidMap);
    }
    getFragmentOnly(fid, aidMap) {
      return this.clone(
        this.getFragmentIds(fid),
        null,
        true,
        aidMap,
        new Pile(),
        new Pile(),
        new Pile(),
        new Pile(),
        new Pile()
      );
    }
    mergeInto(cp, atomSet, bondSet, dropRxnSymbols, keepAllRGroups, aidMap, simpleObjectsSet, textsSet, rgroupAttachmentPointSet, imagesSet, multitailArrowsSet, bidMapEntity, needCloneAttachmentPoints = false) {
      const atoms = atomSet != null ? atomSet : new Pile(this.atoms.keys());
      let bonds = bondSet != null ? bondSet : new Pile(this.bonds.keys());
      const simpleObjects = simpleObjectsSet != null ? simpleObjectsSet : new Pile(this.simpleObjects.keys());
      const texts = textsSet != null ? textsSet : new Pile(this.texts.keys());
      const images = imagesSet != null ? imagesSet : new Pile(this.images.keys());
      const multitailArrows = multitailArrowsSet != null ? multitailArrowsSet : new Pile(this.multitailArrows.keys());
      const rgroupAttachmentPoints = rgroupAttachmentPointSet != null ? rgroupAttachmentPointSet : new Pile(this.rgroupAttachmentPoints.keys());
      const aids = aidMap != null ? aidMap : /* @__PURE__ */ new Map();
      const bidMap = bidMapEntity != null ? bidMapEntity : /* @__PURE__ */ new Map();
      bonds = bonds.filter((bid) => {
        const bond = this.bonds.get(bid);
        return atoms.has(bond.begin) && atoms.has(bond.end);
      });
      const fidMask = new Pile();
      this.atoms.forEach((atom, aid) => {
        if (atoms.has(aid)) fidMask.add(atom.fragment);
      });
      const fidMap = /* @__PURE__ */ new Map();
      this.frags.forEach((_frag, fid) => {
        if (fidMask.has(fid)) fidMap.set(fid, cp.frags.add(null));
      });
      const rgroupsIds = [];
      this.rgroups.forEach((rgroup, rgid) => {
        let keepGroup = keepAllRGroups;
        if (!keepGroup) {
          rgroup.frags.forEach((_fnum, fid) => {
            rgroupsIds.push(fid);
            if (fidMask.has(fid)) keepGroup = true;
          });
          if (!keepGroup) return;
        }
        const rg = cp.rgroups.get(rgid);
        if (rg) {
          rgroup.frags.forEach((_fnum, fid) => {
            rgroupsIds.push(fid);
            if (fidMask.has(fid)) rg.frags.add(fidMap.get(fid));
          });
        } else {
          cp.rgroups.set(rgid, rgroup.clone(fidMap));
        }
      });
      this.atoms.forEach((atom, aid) => {
        if (atoms.has(aid) && rgroupsIds.indexOf(atom.fragment) === -1) {
          aids.set(aid, cp.atoms.add(atom.clone(fidMap)));
        }
      });
      this.atoms.forEach((atom, aid) => {
        if (atoms.has(aid) && rgroupsIds.indexOf(atom.fragment) !== -1) {
          aids.set(aid, cp.atoms.add(atom.clone(fidMap)));
        }
      });
      fidMap.forEach((newfid, oldfid) => {
        const fragment = this.frags.get(oldfid);
        if (fragment && fragment instanceof Fragment) {
          cp.frags.set(newfid, this.frags.get(oldfid).clone(aids));
        }
      });
      this.bonds.forEach((bond, bid) => {
        if (bonds.has(bid)) bidMap.set(bid, cp.bonds.add(bond.clone(aids)));
      });
      const sgroupIdMap = {};
      this.sgroups.forEach((sg, sgroupId) => {
        if (sg.atoms.some((aid) => !atoms.has(aid))) return;
        const oldSgroup = sg;
        sg = oldSgroup instanceof MonomerMicromolecule ? MonomerMicromolecule.clone(
          oldSgroup,
          aids,
          needCloneAttachmentPoints
        ) : SGroup.clone(sg, aids);
        const id2 = cp.sgroups.add(sg);
        sg.id = id2;
        sgroupIdMap[sgroupId] = id2;
        sg.atoms.forEach((aid) => {
          const atom = cp.atoms.get(aid);
          if (atom) {
            atom.sgs.add(id2);
          }
        });
        if (sg.type === "DAT") cp.sGroupForest.insert(sg, -1, []);
        else cp.sGroupForest.insert(sg);
      });
      this.functionalGroups.forEach((fg) => {
        if (fg.relatedSGroup.atoms.some((aid) => !atoms.has(aid))) return;
        const sgroup = cp.sgroups.get(sgroupIdMap[fg.relatedSGroupId]);
        fg = sgroup ? new FunctionalGroup(sgroup) : FunctionalGroup.clone(fg);
        cp.functionalGroups.add(fg);
      });
      simpleObjects.forEach((soid) => {
        cp.simpleObjects.add(this.simpleObjects.get(soid).clone());
      });
      texts.forEach((id2) => {
        cp.texts.add(this.texts.get(id2).clone());
      });
      images.forEach((id2) => {
        cp.images.add(this.images.get(id2).clone());
      });
      multitailArrows.forEach((id2) => {
        cp.addMultitailArrow(this.multitailArrows.get(id2).clone());
      });
      rgroupAttachmentPoints.forEach((id2) => {
        const rgroupAttachmentPoint = this.rgroupAttachmentPoints.get(id2);
        assert_default(rgroupAttachmentPoint != null);
        cp.rgroupAttachmentPoints.add(rgroupAttachmentPoint.clone(aids));
      });
      if (!dropRxnSymbols) {
        cp.isReaction = this.isReaction;
        this.rxnArrows.forEach((item) => {
          cp.addRxnArrow(item.clone());
        });
        this.rxnPluses.forEach((item) => {
          cp.rxnPluses.add(item.clone());
        });
      }
      cp.name = this.name;
      return cp;
    }
    // NB: this updates the structure without modifying the corresponding ReStruct.
    //  To be applied to standalone structures only.
    prepareLoopStructure() {
      this.initHalfBonds();
      this.initNeighbors();
      this.updateHalfBonds(Array.from(this.atoms.keys()));
      this.sortNeighbors(Array.from(this.atoms.keys()));
      this.findLoops();
    }
    atomAddToSGroup(sgid, aid) {
      SGroup.addAtom(this.sgroups.get(sgid), aid, this);
      this.atoms.get(aid).sgs.add(sgid);
    }
    calcConn(atom, includeAtomsInCollapsedSgroups = false) {
      let conn = 0;
      for (const neighborId of atom.neighbors) {
        const hb = this.halfBonds.get(neighborId);
        const bond = this.bonds.get(hb.bid);
        if (Bond3.isBondToHiddenLeavingGroup(
          this,
          bond,
          includeAtomsInCollapsedSgroups
        )) {
          continue;
        }
        switch (bond.type) {
          case Bond3.PATTERN.TYPE.SINGLE:
            conn += 1;
            break;
          case Bond3.PATTERN.TYPE.DOUBLE:
            conn += 2;
            break;
          case Bond3.PATTERN.TYPE.TRIPLE:
            conn += 3;
            break;
          case Bond3.PATTERN.TYPE.DATIVE:
          case Bond3.PATTERN.TYPE.HYDROGEN:
            break;
          case Bond3.PATTERN.TYPE.AROMATIC:
            if (atom.neighbors.length === 1) return [-1, true];
            return [atom.neighbors.length, true];
          default:
            return [-1, false];
        }
      }
      return [conn, false];
    }
    findBondId(begin, end) {
      return this.bonds.find(
        (_bid, bond) => bond.begin === begin && bond.end === end || bond.begin === end && bond.end === begin
      );
    }
    initNeighbors() {
      this.atoms.forEach((atom) => {
        atom.neighbors = [];
      });
      this.bonds.forEach((bond) => {
        const a1 = this.atoms.get(bond.begin);
        const a2 = this.atoms.get(bond.end);
        a1.neighbors.push(bond.hb1);
        a2.neighbors.push(bond.hb2);
      });
    }
    bondInitHalfBonds(bid, bond) {
      bond = bond != null ? bond : this.bonds.get(bid);
      bond.hb1 = 2 * bid;
      bond.hb2 = 2 * bid + 1;
      this.halfBonds.set(bond.hb1, new HalfBond(bond.begin, bond.end, bid));
      this.halfBonds.set(bond.hb2, new HalfBond(bond.end, bond.begin, bid));
      const hb1 = this.halfBonds.get(bond.hb1);
      const hb2 = this.halfBonds.get(bond.hb2);
      hb1.contra = bond.hb2;
      hb2.contra = bond.hb1;
    }
    halfBondUpdate(halfBondId) {
      var _a, _b;
      const halfBond = this.halfBonds.get(halfBondId);
      const sgroup1 = this.getGroupFromAtomId(halfBond.begin);
      const sgroup2 = this.getGroupFromAtomId(halfBond.end);
      let startCoords;
      let endCoords;
      if (sgroup1 instanceof MonomerMicromolecule && sgroup1 !== sgroup2) {
        startCoords = sgroup1.isContracted() ? sgroup1.pp : this.atoms.get(halfBond.begin).pp;
      } else if (sgroup1 && sgroup1 !== sgroup2 && sgroup1.isContracted()) {
        startCoords = (_a = sgroup1.getContractedPosition(this).position) != null ? _a : this.atoms.get(halfBond.begin).pp;
      } else {
        startCoords = this.atoms.get(halfBond.begin).pp;
      }
      if (sgroup2 instanceof MonomerMicromolecule && sgroup1 !== sgroup2) {
        endCoords = sgroup2.isContracted() ? sgroup2.pp : this.atoms.get(halfBond.end).pp;
      } else if (sgroup2 && sgroup2 !== sgroup1 && sgroup2.isContracted()) {
        endCoords = (_b = sgroup2.getContractedPosition(this).position) != null ? _b : this.atoms.get(halfBond.end).pp;
      } else {
        endCoords = this.atoms.get(halfBond.end).pp;
      }
      const coordsDifference = Vec2.diff(endCoords, startCoords).normalized();
      halfBond.dir = Vec2.dist(endCoords, startCoords) > 1e-4 ? coordsDifference : new Vec2(1, 0);
      halfBond.norm = halfBond.dir.turnLeft();
      halfBond.ang = halfBond.dir.oxAngle();
      if (halfBond.loop < 0) halfBond.loop = -1;
    }
    initHalfBonds() {
      this.halfBonds.clear();
      this.bonds.forEach((bond, bid) => {
        this.bondInitHalfBonds(bid, bond);
      });
    }
    setHbNext(hbid, next) {
      this.halfBonds.get(this.halfBonds.get(hbid).contra).next = next;
    }
    halfBondSetAngle(hbid, left) {
      const hb = this.halfBonds.get(hbid);
      const hbl = this.halfBonds.get(left);
      hbl.rightCos = Vec2.dot(hbl.dir, hb.dir);
      hb.leftCos = Vec2.dot(hbl.dir, hb.dir);
      hbl.rightSin = Vec2.cross(hbl.dir, hb.dir);
      hb.leftSin = Vec2.cross(hbl.dir, hb.dir);
      hb.leftNeighbor = left;
      hbl.rightNeighbor = hbid;
    }
    atomAddNeighbor(hbid) {
      const hb = this.halfBonds.get(hbid);
      const atom = this.atoms.get(hb.begin);
      let i;
      for (i = 0; i < atom.neighbors.length; ++i) {
        if (this.halfBonds.get(atom.neighbors[i]).ang > hb.ang) break;
      }
      atom.neighbors.splice(i, 0, hbid);
      const ir = atom.neighbors[(i + 1) % atom.neighbors.length];
      const il = atom.neighbors[(i + atom.neighbors.length - 1) % atom.neighbors.length];
      this.setHbNext(il, hbid);
      this.setHbNext(hbid, ir);
      this.halfBondSetAngle(hbid, il);
      this.halfBondSetAngle(ir, hbid);
    }
    atomSortNeighbors(aid) {
      const atom = this.atoms.get(aid);
      const halfBonds = this.halfBonds;
      
      const arr = atom.neighbors;
      const len = arr.length;
      for (let i = 0; i < len; i++) {
        for (let j = 0; j < len - i - 1; j++) {
          if (halfBonds.get(arr[j]).ang > halfBonds.get(arr[j+1]).ang) {
            const temp = arr[j];
            arr[j] = arr[j+1];
            arr[j+1] = temp;
          }
        }
      }
      
      atom.neighbors.forEach((nei, i) => {
        const nextNei = atom.neighbors[(i + 1) % atom.neighbors.length];
        this.halfBonds.get(this.halfBonds.get(nei).contra).next = nextNei;
        this.halfBondSetAngle(nextNei, nei);
      });
    }
    sortNeighbors(list) {
      if (!list) {
        this.atoms.forEach((_atom, aid) => {
          this.atomSortNeighbors(aid);
        });
      } else {
        list.forEach((aid) => {
          this.atomSortNeighbors(aid);
        });
      }
    }
    atomUpdateHalfBonds(atomId) {
      this.atoms.get(atomId).neighbors.forEach((hbid) => {
        this.halfBondUpdate(hbid);
        this.halfBondUpdate(this.halfBonds.get(hbid).contra);
      });
    }
    updateHalfBonds(list) {
      if (!list) {
        this.atoms.forEach((_atom, atomId) => {
          this.atomUpdateHalfBonds(atomId);
        });
      } else {
        list.forEach((atomId) => {
          this.atomUpdateHalfBonds(atomId);
        });
      }
    }
    sGroupsRecalcCrossBonds() {
      this.sgroups.forEach((sg) => {
        sg.xBonds = [];
        sg.neiAtoms = [];
      });
      this.bonds.forEach((bond, bid) => {
        const a1 = this.atoms.get(bond.begin);
        const a2 = this.atoms.get(bond.end);
        a1.sgs.forEach((sgid) => {
          if (!a2.sgs.has(sgid)) {
            const sg = this.sgroups.get(sgid);
            sg.xBonds.push(bid);
            arrayAddIfMissing(sg.neiAtoms, bond.end);
          }
        });
        a2.sgs.forEach((sgid) => {
          if (!a1.sgs.has(sgid)) {
            const sg = this.sgroups.get(sgid);
            sg.xBonds.push(bid);
            arrayAddIfMissing(sg.neiAtoms, bond.begin);
          }
        });
      });
    }
    sGroupDelete(sgid) {
      this.sgroups.get(sgid).atoms.forEach((atom) => {
        this.atoms.get(atom).sgs.delete(sgid);
      });
      this.sGroupForest.remove(sgid);
      this.sgroups.delete(sgid);
    }
    atomSetPos(id2, pp) {
      const item = this.atoms.get(id2);
      item.pp = pp;
    }
    rxnPlusSetPos(id2, pp) {
      const item = this.rxnPluses.get(id2);
      item.pp = pp;
    }
    rxnArrowSetPos(id2, pos) {
      const item = this.rxnArrows.get(id2);
      if (item) {
        item.pos = pos;
      }
    }
    simpleObjectSetPos(id2, pos) {
      const item = this.simpleObjects.get(id2);
      item.pos = pos;
    }
    textSetPosition(id2, position) {
      const item = this.texts.get(id2);
      if (item) {
        item.position = position;
      }
    }
    getCoordBoundingBox(atomSet) {
      let bb = null;
      function extend(pp) {
        if (!bb) {
          bb = {
            min: pp,
            max: pp
          };
        } else if (pp instanceof Array) {
          pp.forEach((vec) => {
            bb.min = Vec2.min(bb.min, vec);
            bb.max = Vec2.max(bb.max, vec);
          });
        } else {
          bb.min = Vec2.min(bb.min, pp);
          bb.max = Vec2.max(bb.max, pp);
        }
      }
      const global2 = !atomSet || atomSet.size === 0;
      this.atoms.forEach((atom, aid) => {
        if (global2 || atomSet.has(aid)) extend(atom.pp);
      });
      if (global2) {
        this.rxnPluses.forEach((item) => {
          extend(item.pp);
        });
        this.rxnArrows.forEach((item) => {
          extend(item.pos);
        });
        this.simpleObjects.forEach((item) => {
          extend(item.pos);
        });
        this.texts.forEach((item) => {
          extend(item.position);
        });
      }
      if (!bb && global2) {
        bb = {
          min: new Vec2(0, 0),
          max: new Vec2(1, 1)
        };
      }
      return bb;
    }
    getCoordBoundingBoxObj() {
      let bb = null;
      function extend(pp) {
        if (!bb) {
          bb = {
            min: new Vec2(pp),
            max: new Vec2(pp)
          };
        } else {
          bb.min = Vec2.min(bb.min, pp);
          bb.max = Vec2.max(bb.max, pp);
        }
      }
      this.atoms.forEach((atom) => {
        extend(atom.pp);
      });
      return bb;
    }
    getBondLengthData() {
      let totalLength = 0;
      let cnt = 0;
      this.bonds.forEach((bond) => {
        totalLength += Vec2.dist(
          this.atoms.get(bond.begin).pp,
          this.atoms.get(bond.end).pp
        );
        cnt++;
      });
      return { cnt, totalLength };
    }
    getAvgBondLength() {
      const bld = this.getBondLengthData();
      return bld.cnt > 0 ? bld.totalLength / bld.cnt : -1;
    }
    getAvgClosestAtomDistance() {
      let totalDist = 0;
      let minDist;
      let dist = 0;
      const keys = Array.from(this.atoms.keys());
      let k;
      let j;
      for (k = 0; k < keys.length; ++k) {
        minDist = -1;
        for (j = 0; j < keys.length; ++j) {
          if (j === k) continue;
          dist = Vec2.dist(
            this.atoms.get(keys[j]).pp,
            this.atoms.get(keys[k]).pp
          );
          if (minDist < 0 || minDist > dist) minDist = dist;
        }
        totalDist += minDist;
      }
      return keys.length > 0 ? totalDist / keys.length : -1;
    }
    checkBondExists(begin, end) {
      const key = this.bonds.find(
        (_bid, bond) => bond.begin === begin && bond.end === end || bond.end === begin && bond.begin === end
      );
      return key !== null;
    }
    findConnectedComponent(firstaid) {
      const list = [firstaid];
      const ids = new Pile();
      while (list.length > 0) {
        const aid = list.pop();
        const atom = this.atoms.get(aid);
        if (this.isAtomFromMacromolecule(aid)) {
          continue;
        }
        ids.add(aid);
        atom.neighbors.forEach((nei) => {
          const neiId = this.halfBonds.get(nei).end;
          if (!ids.has(neiId)) list.push(neiId);
        });
      }
      return ids;
    }
    findConnectedComponents(discardExistingFragments) {
      if (!this.halfBonds.size) {
        this.initHalfBonds();
        this.initNeighbors();
        this.updateHalfBonds(Array.from(this.atoms.keys()));
        this.sortNeighbors(Array.from(this.atoms.keys()));
      }
      let addedAtoms = new Pile();
      const components = [];
      this.atoms.forEach((atom, aid) => {
        if ((discardExistingFragments || atom.fragment < 0) && !addedAtoms.has(aid) && !this.isAtomFromMacromolecule(aid)) {
          const component = this.findConnectedComponent(aid);
          components.push(component);
          addedAtoms = addedAtoms.union(component);
        }
      });
      return components;
    }
    markFragment(idSet, properties) {
      const frag = new Fragment([], void 0, properties);
      const fid = this.frags.add(frag);
      idSet.forEach((aid) => {
        const atom = this.atoms.get(aid);
        if (atom.stereoLabel) frag.updateStereoAtom(this, aid, fid, true);
        atom.fragment = fid;
      });
    }
    clearFragments() {
      this.atoms.forEach((atom) => {
        atom.fragment = -1;
      });
      this.frags.clear();
    }
    markFragments(properties) {
      const components = this.findConnectedComponents();
      components.forEach((comp) => {
        const [firstAtom] = comp;
        const sgroup = this.getGroupFromAtomId(firstAtom);
        if (sgroup instanceof MonomerMicromolecule) {
          return;
        }
        this.markFragment(comp, properties);
      });
    }
    scale(scale) {
      if (scale === 1) return;
      this.atoms.forEach((atom) => {
        atom.pp = atom.pp.scaled(scale);
      });
      this.rxnPluses.forEach((item) => {
        item.pp = item.pp.scaled(scale);
      });
      this.rxnArrows.forEach((item) => {
        item.pos = item.pos.map((p) => p.scaled(scale));
      });
      this.sgroups.forEach((item) => {
        var _a, _b;
        if (item instanceof MonomerMicromolecule) {
          return;
        }
        item.pp = (_b = (_a = item.pp) == null ? void 0 : _a.scaled(scale)) != null ? _b : null;
      });
      this.texts.forEach((item) => {
        item.pos = item.pos.map((p) => p.scaled(scale));
        item.position = item.position.scaled(scale);
      });
      this.simpleObjects.forEach((simpleObjects) => {
        simpleObjects.pos = simpleObjects.pos.map((p) => p.scaled(scale));
      });
      this.images.forEach((image) => image.rescaleSize(scale));
      this.multitailArrows.forEach(
        (multitailArrow) => multitailArrow.rescaleSize(scale)
      );
    }
    rescale() {
      let avg = this.getAvgBondLength();
      if (avg <= 0) {
        return;
      }
      if (avg < 1e-3) avg = 1;
      const scale = 1 / avg;
      this.scale(scale);
    }
    loopHasSelfIntersections(hbs) {
      for (const [i, halfBondId] of hbs.entries()) {
        const hbi = this.halfBonds.get(halfBondId);
        const ai = this.atoms.get(hbi.begin).pp;
        const bi = this.atoms.get(hbi.end).pp;
        const set = new Pile([hbi.begin, hbi.end]);
        for (const hbjId of hbs.slice(i + 2)) {
          const hbj = this.halfBonds.get(hbjId);
          if (set.has(hbj.begin) || set.has(hbj.end)) continue;
          const aj = this.atoms.get(hbj.begin).pp;
          const bj = this.atoms.get(hbj.end).pp;
          if (Box2Abs.segmentIntersection(ai, bi, aj, bj)) return true;
        }
      }
      return false;
    }
    // partition a cycle into simple cycles
    // TODO: [MK] rewrite the detection algorithm to only find simple ones right away?
    partitionLoop(loop) {
      const subloops = [];
      let continueFlag = true;
      while (continueFlag) {
        const atomToHalfBond = {};
        continueFlag = false;
        for (const [index, hbid] of loop.entries()) {
          const aid1 = this.halfBonds.get(hbid).begin;
          const aid2 = this.halfBonds.get(hbid).end;
          if (aid2 in atomToHalfBond) {
            const s = atomToHalfBond[aid2];
            const subloop = loop.slice(s, index + 1);
            subloops.push(subloop);
            if (index < loop.length) {
              loop.splice(s, index - s + 1);
            }
            continueFlag = true;
            break;
          }
          atomToHalfBond[aid1] = index;
        }
        if (!continueFlag) subloops.push(loop);
      }
      return subloops;
    }
    halfBondAngle(hbid1, hbid2) {
      const hba = this.halfBonds.get(hbid1);
      const hbb = this.halfBonds.get(hbid2);
      return Math.atan2(Vec2.cross(hba.dir, hbb.dir), Vec2.dot(hba.dir, hbb.dir));
    }
    loopIsConvex(loop) {
      return loop.every((item, k, loopArr) => {
        const angle = this.halfBondAngle(item, loopArr[(k + 1) % loopArr.length]);
        return angle <= 0;
      });
    }
    // check whether a loop is on the inner or outer side of the polygon
    //  by measuring the total angle between bonds
    loopIsInner(loop) {
      let totalAngle = 2 * Math.PI;
      loop.forEach((hbida, k, loopArr) => {
        const hbidb = loopArr[(k + 1) % loopArr.length];
        const hbb = this.halfBonds.get(hbidb);
        const angle = this.halfBondAngle(hbida, hbidb);
        totalAngle += hbb.contra === hbida ? Math.PI : angle;
      });
      return Math.abs(totalAngle) < Math.PI;
    }
    findLoops() {
      const newLoops = [];
      const bondsToMark = new Pile();
      let hbIdNext, c, loop;
      this.halfBonds.forEach((hb, hbId) => {
        if (hb.loop !== -1) return;
        for (hbIdNext = hbId, c = 0, loop = []; c <= this.halfBonds.size; hbIdNext = this.halfBonds.get(hbIdNext)?.next, ++c) {
          if (hbIdNext === undefined) break;
          if (!(c > 0 && hbIdNext === hbId)) {
            loop.push(hbIdNext);
            continue;
          }
          const subloops = this.partitionLoop(loop);
          subloops.forEach((loop2) => {
            let loopId;
            if (this.loopIsInner(loop2) && !this.loopHasSelfIntersections(loop2)) {
              loopId = Math.min(...loop2);
              this.loops.set(
                loopId,
                new Loop(loop2, this, this.loopIsConvex(loop2))
              );
            } else {
              loopId = -2;
            }
            loop2.forEach((hbid) => {
              this.halfBonds.get(hbid).loop = loopId;
              bondsToMark.add(this.halfBonds.get(hbid).bid);
            });
            if (loopId >= 0) newLoops.push(loopId);
          });
          break;
        }
      });
      return {
        newLoops,
        bondsToMark: (function(p){ var a=[]; p.forEach(function(v){a.push(v)}); return a; })(bondsToMark)
      };
    }
    calcImplicitHydrogen(aid, includeAtomsInCollapsedSgroups = false) {
      var _a;
      if (Atom2.isHiddenLeavingGroupAtom(this, aid)) {
        return;
      }
      const atom = this.atoms.get(aid);
      const charge = (_a = atom.charge) != null ? _a : 0;
      const [conn, isAromatic] = this.calcConn(
        atom,
        includeAtomsInCollapsedSgroups
      );
      let correctConn = conn;
      atom.badConn = false;
      if (isAromatic) {
        if (atom.label === "C" && charge === 0) {
          if (conn === 3) {
            atom.implicitH = -radicalElectrons(atom.radical);
            return;
          }
          if (conn === 2) {
            atom.implicitH = 1 - radicalElectrons(atom.radical);
            return;
          }
        } else if (atom.label === "O" && charge === 0 || atom.label === "N" && charge === 0 && conn === 3 || atom.label === "N" && charge === 1 && conn === 3 || atom.label === "S" && charge === 0 && conn === 3 || !atom.implicitH) {
          atom.implicitH = 0;
          return;
        } else if (!atom.hasImplicitH) {
          correctConn++;
        }
      }
      if (correctConn < 0 || atom.isQuery() || atom.attachmentPoints) {
        atom.implicitH = 0;
        return;
      }
      if (atom.explicitValence >= 0) {
        const elem = Elements.get(atom.label);
        atom.implicitH = elem ? atom.explicitValence - atom.calcValenceMinusHyd(correctConn) : 0;
        if (atom.implicitH < 0) {
          atom.implicitH = 0;
          atom.badConn = true;
        }
      } else {
        atom.calcValence(correctConn);
      }
    }
    setImplicitHydrogen(list, includeAtomsInCollapsedSgroups = false) {
      this.sgroups.forEach((item) => {
        if (item.data.fieldName === "MRV_IMPLICIT_H") {
          this.atoms.get(item.atoms[0]).hasImplicitH = true;
        }
      });
      if (!list) {
        this.atoms.forEach((_atom, aid) => {
          this.calcImplicitHydrogen(aid, includeAtomsInCollapsedSgroups);
        });
      } else {
        list.forEach((aid) => {
          if (this.atoms.get(aid)) {
            this.calcImplicitHydrogen(aid, includeAtomsInCollapsedSgroups);
          }
        });
      }
    }
    setStereoLabelsToAtoms() {
      const stereAtomsMap = getStereoAtomsMap(
        this,
        Array.from(this.bonds.values())
      );
      this.atoms.forEach((atom, id2) => {
        var _a;
        if (((_a = this == null ? void 0 : this.atomGetNeighbors(id2)) == null ? void 0 : _a.length) === 0) {
          atom.stereoLabel = null;
          atom.stereoParity = 0;
        } else {
          const stereoProp = stereAtomsMap.get(id2);
          if (stereoProp) {
            atom.stereoLabel = stereoProp.stereoLabel;
            atom.stereoParity = stereoProp.stereoParity;
          }
        }
      });
    }
    atomGetNeighbors(aid) {
      var _a;
      return (_a = this.atoms.get(aid)) == null ? void 0 : _a.neighbors.map((nei) => {
        const hb = this.halfBonds.get(nei);
        return {
          aid: hb.end,
          bid: hb.bid
        };
      });
    }
    getComponents() {
      const connectedComponents = this.findConnectedComponents(true);
      const barriers = [];
      let arrowPos = null;
      this.rxnArrows.forEach((item) => {
        arrowPos = item.center().x;
      });
      this.rxnPluses.forEach((item) => {
        barriers.push(item.pp.x);
      });
      if (arrowPos !== null) barriers.push(arrowPos);
      barriers.sort((a, b) => a - b);
      const components = [];
      connectedComponents.forEach((component) => {
        var _a;
        const bb = this.getCoordBoundingBox(component);
        const c = Vec2.lc2(bb.min, 0.5, bb.max, 0.5);
        let j = 0;
        while (c.x > barriers[j]) ++j;
        components[j] = (_a = components[j]) != null ? _a : new Pile();
        components[j] = components[j].union(component);
      });
      const reactants = [];
      const products = [];
      components.forEach((component) => {
        if (!component) {
          return;
        }
        const rxnFragmentType = this.defineRxnFragmentTypeForAtomset(
          component,
          arrowPos != null ? arrowPos : 0
        );
        if (rxnFragmentType === 1) reactants.push(component);
        else products.push(component);
      });
      return {
        reactants,
        products
      };
    }
    defineRxnFragmentTypeForAtomset(atomset, arrowpos) {
      const bb = this.getCoordBoundingBox(atomset);
      const c = Vec2.lc2(bb.min, 0.5, bb.max, 0.5);
      return c.x < arrowpos ? 1 : 2;
    }
    getBondFragment(bid) {
      var _a, _b;
      const aid = (_a = this.bonds.get(bid)) == null ? void 0 : _a.begin;
      return aid && ((_b = this.atoms.get(aid)) == null ? void 0 : _b.fragment);
    }
    bindSGroupsToFunctionalGroups() {
      this.sgroups.forEach((sgroup) => {
        if (FunctionalGroup.isFunctionalGroup(sgroup) || SGroup.isSuperAtom(sgroup)) {
          this.functionalGroups.add(new FunctionalGroup(sgroup));
        }
      });
    }
    getGroupIdFromAtomId(atomId) {
      var _a, _b;
      const firstSgroupId = [...(_b = (_a = this.atoms.get(atomId)) == null ? void 0 : _a.sgs.values()) != null ? _b : []][0];
      return (0, import_lodash4.isNumber)(firstSgroupId) ? firstSgroupId : null;
    }
    getGroupIdFromAtomIdBySgroups(atomId) {
      for (const [groupId, sgroup] of Array.from(this.sgroups)) {
        if (sgroup.atoms.includes(atomId)) return groupId;
      }
      return null;
    }
    getGroupFromAtomId(atomId) {
      var _a;
      if (!(0, import_lodash4.isNumber)(atomId)) {
        return void 0;
      }
      const sgroupId = this.getGroupIdFromAtomId(atomId);
      return (0, import_lodash4.isNumber)(sgroupId) ? (_a = this.sgroups) == null ? void 0 : _a.get(sgroupId) : void 0;
    }
    getGroupFromAtomIdBySgroups(atomId) {
      var _a;
      if (!(0, import_lodash4.isNumber)(atomId)) {
        return void 0;
      }
      const sgroupId = this.getGroupIdFromAtomIdBySgroups(atomId);
      return (_a = this.sgroups) == null ? void 0 : _a.get(sgroupId);
    }
    // TODO: simplify if bonds ids ever appear in sgroup
    // ! deprecate
    getGroupIdFromBondId(bondId) {
      const bond = this.bonds.get(bondId);
      if (!bond) return null;
      for (const [groupId, sgroup] of Array.from(this.sgroups)) {
        if (sgroup.atoms.includes(bond.begin) || sgroup.atoms.includes(bond.end)) {
          return groupId;
        }
      }
      return null;
    }
    getGroupFromBondId(atomId) {
      var _a;
      const sgroupId = this.getGroupIdFromBondId(atomId);
      if (!(0, import_lodash4.isNumber)(sgroupId)) {
        return;
      }
      return (_a = this.sgroups) == null ? void 0 : _a.get(sgroupId);
    }
    getGroupsIdsFromBondId(bondId) {
      const bond = this.bonds.get(bondId);
      if (!bond) return [];
      const groupsIds = [];
      for (const [groupId, sgroup] of Array.from(this.sgroups)) {
        if (sgroup.atoms.includes(bond.begin) || sgroup.atoms.includes(bond.end)) {
          groupsIds.push(groupId);
        }
      }
      return groupsIds;
    }
    getBondIdByHalfBond(halfBondId) {
      const halfBond = this.halfBonds.get(halfBondId);
      if (halfBond) {
        return halfBond.bid;
      }
      return void 0;
    }
    /**
     * @returns visibleAtoms = selected atoms
     *                       - atoms in contracted functional groups
     *                       + functional groups's attachment atoms
     */
    getSelectedVisibleAtoms(selection) {
      var _a;
      return ((_a = selection == null ? void 0 : selection.atoms) == null ? void 0 : _a.filter((atomId) => {
        const atom = this.atoms.get(atomId);
        if (!atom) {
          return false;
        }
        const isAtomNotInContractedGroup = !FunctionalGroup.isAtomInContractedFunctionalGroup(
          atom,
          this.sgroups,
          this.functionalGroups
        );
        if (isAtomNotInContractedGroup) {
          return true;
        }
        const groupId = this.getGroupIdFromAtomId(atomId);
        const sgroup = this.sgroups.get(groupId);
        return (sgroup == null ? void 0 : sgroup.getAttachmentAtomId()) === atomId;
      })) || [];
    }
    getRGroupAttachmentPointsByAtomId(atomId) {
      const rgroupAttachmentPoints = this.rgroupAttachmentPoints.filter(
        (_id, attachmentPoint) => attachmentPoint.atomId === atomId
      );
      return [...rgroupAttachmentPoints.keys()];
    }
    isAtomFromMacromolecule(atomId) {
      const sgroup = this.getGroupFromAtomId(atomId);
      return sgroup instanceof MonomerMicromolecule;
    }
    isBondFromMacromolecule(bondOrBondId) {
      const bond = bondOrBondId instanceof Bond3 ? bondOrBondId : this.bonds.get(bondOrBondId);
      assert_default(bond);
      return this.isAtomFromMacromolecule(bond.begin) || this.isAtomFromMacromolecule(bond.end);
    }
    isFunctionalGroupFromMacromolecule(functionalGroupId) {
      const functionalGroup = this.functionalGroups.get(functionalGroupId);
      return (functionalGroup == null ? void 0 : functionalGroup.relatedSGroup) instanceof MonomerMicromolecule;
    }
    isTargetFromMacromolecule(target) {
      return target && (target.map === "functionalGroups" && this.isFunctionalGroupFromMacromolecule(target.id) || target.map === "atoms" && this.isAtomFromMacromolecule(target.id) || target.map === "bonds" && this.isBondFromMacromolecule(target.id));
    }
    disableInitiallySelected() {
      this.atoms.changeInitiallySelectedPropertiesForPool(true);
      this.bonds.changeInitiallySelectedPropertiesForPool(true);
      this.rxnPluses.changeInitiallySelectedPropertiesForPool(true);
      this.rxnArrows.changeInitiallySelectedPropertiesForPool(true);
      this.texts.changeInitiallySelectedPropertiesForPool(true);
    }
    enableInitiallySelected() {
      this.atoms.changeInitiallySelectedPropertiesForPool();
      this.bonds.changeInitiallySelectedPropertiesForPool();
      this.rxnPluses.changeInitiallySelectedPropertiesForPool();
      this.rxnArrows.changeInitiallySelectedPropertiesForPool();
      this.texts.changeInitiallySelectedPropertiesForPool();
    }
    applyMonomersTransformations() {
      const atomToBonds = /* @__PURE__ */ new Map();
      this.bonds.forEach((bond, bondId) => {
        var _a;
        for (const atomId of [bond.begin, bond.end]) {
          const list = (_a = atomToBonds.get(atomId)) != null ? _a : [];
          list.push(bondId);
          atomToBonds.set(atomId, list);
        }
      });
      this.sgroups.forEach((sGroup) => {
        var _a, _b;
        if (!(sGroup instanceof MonomerMicromolecule)) {
          return;
        }
        const center = sGroup.pp;
        if (!center) {
          return;
        }
        const rotateValue = (_a = sGroup.monomer.monomerItem.transformation) == null ? void 0 : _a.rotate;
        if (rotateValue) {
          sGroup.atoms.forEach((atomId) => {
            const atom = this.atoms.get(atomId);
            if (!atom) {
              return;
            }
            atom.pp = atom.pp.add(rotateDelta(atom.pp, center, rotateValue));
          });
        }
        const flipValue = (_b = sGroup.monomer.monomerItem.transformation) == null ? void 0 : _b.flip;
        if (flipValue) {
          sGroup.atoms.forEach((atomId) => {
            const atom = this.atoms.get(atomId);
            if (!atom) {
              return;
            }
            atom.pp = atom.pp.add(flipPointByCenter(atom.pp, center, flipValue));
          });
          const sGroupBonds = new Set(
            sGroup.atoms.flatMap((atomId) => atomToBonds.get(atomId))
          );
          sGroupBonds.forEach((bondId) => {
            const bond = this.bonds.get(bondId);
            if (!bond || bond.type !== Bond3.PATTERN.TYPE.SINGLE) {
              return;
            }
            if (bond.stereo === Bond3.PATTERN.STEREO.UP || bond.stereo === Bond3.PATTERN.STEREO.DOWN) {
              bond.stereo = bond.stereo === Bond3.PATTERN.STEREO.UP ? Bond3.PATTERN.STEREO.DOWN : Bond3.PATTERN.STEREO.UP;
            }
          });
        }
      });
    }
    applyStereoBondsToExpandedMonomers() {
      const expandedMonomers = [];
      this.sgroups.forEach((sgroup) => {
        if (sgroup instanceof MonomerMicromolecule && sgroup.isExpanded()) {
          expandedMonomers.push(sgroup);
        }
      });
      if (expandedMonomers.length < 2) {
        return;
      }
      for (let i = 0; i < expandedMonomers.length; i++) {
        const firstMonomer = expandedMonomers[i];
        const firstMonomerAtoms = new Set(SGroup.getAtoms(this, firstMonomer));
        const firstMonomerAttachmentPoints = firstMonomer.getAttachmentPoints();
        for (let j = i + 1; j < expandedMonomers.length; j++) {
          const secondMonomer = expandedMonomers[j];
          const secondMonomerAtoms = new Set(
            SGroup.getAtoms(this, secondMonomer)
          );
          const secondMonomerAttachmentPoints = secondMonomer.getAttachmentPoints();
          this.bonds.forEach((bond, bondId) => {
            const firstMonomerHasBondBegin = firstMonomerAtoms.has(bond.begin);
            const firstMonomerHasBondEnd = firstMonomerAtoms.has(bond.end);
            const secondMonomerHasBondBegin = secondMonomerAtoms.has(bond.begin);
            const secondMonomerHasBondEnd = secondMonomerAtoms.has(bond.end);
            const isBondConnectinBothMonomers = firstMonomerHasBondBegin && secondMonomerHasBondEnd || firstMonomerHasBondEnd && secondMonomerHasBondBegin;
            if (!isBondConnectinBothMonomers) {
              return;
            }
            const firstMonomerAtom = firstMonomerHasBondBegin ? bond.begin : bond.end;
            const secondMonomerAtom = secondMonomerHasBondBegin ? bond.begin : bond.end;
            const firstMonomerAttachmentPointInConnection = firstMonomerAttachmentPoints.find(
              (attachmentPoint) => attachmentPoint.atomId === firstMonomerAtom
            );
            const secondMonomerAttachmentPointInConnection = secondMonomerAttachmentPoints.find(
              (attachmentPoint) => attachmentPoint.atomId === secondMonomerAtom
            );
            if (!firstMonomerAttachmentPointInConnection || !secondMonomerAttachmentPointInConnection) {
              return;
            }
            const firstMonomerAttachmentPointBondStereo = getAttachmentPointStereoBond(
              firstMonomer,
              firstMonomerAttachmentPointInConnection
            );
            const secondMonomerAttachmentPointBondStereo = getAttachmentPointStereoBond(
              secondMonomer,
              secondMonomerAttachmentPointInConnection
            );
            const firstMonomerHasStereoBondOnAttachmentPoint = firstMonomerAttachmentPointBondStereo !== null && firstMonomerAttachmentPointBondStereo !== Bond3.PATTERN.STEREO.NONE;
            const secondMonomerHasStereoBondOnAttachmentPoint = secondMonomerAttachmentPointBondStereo !== null && secondMonomerAttachmentPointBondStereo !== Bond3.PATTERN.STEREO.NONE;
            if (firstMonomerHasStereoBondOnAttachmentPoint && !secondMonomerHasStereoBondOnAttachmentPoint) {
              if (bond.begin !== firstMonomerAtom) {
                this.flipBondAndSetStereo(
                  bondId,
                  bond,
                  firstMonomerAttachmentPointBondStereo
                );
              } else {
                bond.stereo = firstMonomerAttachmentPointBondStereo;
              }
            } else if (!firstMonomerHasStereoBondOnAttachmentPoint && secondMonomerHasStereoBondOnAttachmentPoint) {
              if (bond.begin !== secondMonomerAtom) {
                this.flipBondAndSetStereo(
                  bondId,
                  bond,
                  secondMonomerAttachmentPointBondStereo
                );
              } else {
                bond.stereo = secondMonomerAttachmentPointBondStereo;
              }
            } else if (firstMonomerHasStereoBondOnAttachmentPoint && secondMonomerHasStereoBondOnAttachmentPoint) {
              bond.stereo = Bond3.PATTERN.STEREO.NONE;
            }
          });
        }
      }
    }
    flipBondAndSetStereo(bondId, bond, stereo) {
      this.bonds.delete(bondId);
      const newBond = new Bond3(__spreadProps(__spreadValues({}, bond), {
        begin: bond.end,
        end: bond.begin,
        stereo,
        beginSuperatomAttachmentPointNumber: bond.endSuperatomAttachmentPointNumber,
        endSuperatomAttachmentPointNumber: bond.beginSuperatomAttachmentPointNumber
      }));
      this.bonds.set(bondId, newBond);
      this.bondInitHalfBonds(bondId);
      const newBondObj = this.bonds.get(bondId);
      if ((newBondObj == null ? void 0 : newBondObj.hb1) && (newBondObj == null ? void 0 : newBondObj.hb2)) {
        this.atomAddNeighbor(newBondObj.hb1);
        this.atomAddNeighbor(newBondObj.hb2);
      }
    }
  };

  // domain/entities/text.ts
  function preparePositions(positions) {
    if (!(positions == null ? void 0 : positions.length)) {
      return [new Vec2(), new Vec2(), new Vec2(), new Vec2()];
    }
    return positions.map((position) => new Vec2(position));
  }
  var Text = class _Text extends BaseMicromoleculeEntity {
    constructor(attributes) {
      var _a;
      super(attributes == null ? void 0 : attributes.initiallySelected);
      /**
       * Stringified Lexical editor state. Parsable JSON with a `root` node.
       */
      __publicField(this, "content");
      __publicField(this, "position");
      __publicField(this, "pos");
      this.pos = preparePositions(attributes == null ? void 0 : attributes.pos);
      this.content = (_a = attributes == null ? void 0 : attributes.content) != null ? _a : "";
      this.position = (attributes == null ? void 0 : attributes.position) ? new Vec2(attributes.position) : new Vec2();
    }
    setPos(coords) {
      this.pos = coords != null ? coords : [];
    }
    clone() {
      return new _Text(this);
    }
  };

  // src/core/io/ket/helpers.ts
  var import_lodash5 = __toESM(require_lodash2());

  // stub-ket-types:application/formatters/types/ket
  var KetTemplateType = {
    MONOMER_TEMPLATE: "monomerTemplate",
    AMBIGUOUS_MONOMER_TEMPLATE: "ambiguousMonomerTemplate",
    MONOMER_GROUP_TEMPLATE: "monomerGroupTemplate"
  };

  // domain/entities/HydrogenBond.ts
  var HydrogenBond = class extends BaseBond {
    constructor(firstMonomer, secondMonomer) {
      super();
      this.firstMonomer = firstMonomer;
      __publicField(this, "secondMonomer");
      __publicField(this, "renderer");
      this.firstMonomer = firstMonomer;
      this.secondMonomer = secondMonomer;
    }
    setFirstMonomer(monomer) {
      this.firstMonomer = monomer;
    }
    setSecondMonomer(monomer) {
      this.secondMonomer = monomer;
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    get isBackBoneChainConnection() {
      return false;
    }
    get firstMonomerAttachmentPoint() {
      return this.firstMonomer.getAttachmentPointByBond(this);
    }
    get secondMonomerAttachmentPoint() {
      var _a;
      return (_a = this.secondMonomer) == null ? void 0 : _a.getAttachmentPointByBond(this);
    }
    get isSideChainConnection() {
      return true;
    }
    get firstEndEntity() {
      return this.firstMonomer;
    }
    get secondEndEntity() {
      return this.secondMonomer;
    }
    getAnotherMonomer(monomer) {
      return super.getAnotherEntity(monomer);
    }
    get isHorizontal() {
      return false;
    }
    get isVertical() {
      return false;
    }
  };

  // domain/helpers/monomerItem.ts
  function isMonomerItemSugar(monomer) {
    return monomer.props.MonomerClass === "Sugar" /* Sugar */ || monomer.props.MonomerType === MONOMER_CONST.RNA && monomer.props.MonomerNaturalAnalogCode === MONOMER_CONST.R;
  }
  function isMonomerItemPhosphate(monomer) {
    return monomer.props.MonomerClass === "Phosphate" /* Phosphate */ || monomer.props.MonomerType === MONOMER_CONST.RNA && monomer.props.MonomerNaturalAnalogCode === MONOMER_CONST.P;
  }

  // domain/helpers/monomers.ts
  var isAmbiguousMonomerEntity = (monomer) => {
    var _a;
    const ambiguousMonomer = monomer;
    return Boolean(
      ((_a = ambiguousMonomer == null ? void 0 : ambiguousMonomer.monomerItem) == null ? void 0 : _a.isAmbiguous) && (ambiguousMonomer == null ? void 0 : ambiguousMonomer.monomerClass)
    );
  };
  var getMonomerClass2 = (monomer) => {
    var _a, _b, _c;
    const monomerLike = monomer;
    return (_c = (_b = (_a = monomerLike == null ? void 0 : monomerLike.monomerItem) == null ? void 0 : _a.props) == null ? void 0 : _b.MonomerClass) != null ? _c : monomerLike == null ? void 0 : monomerLike.monomerClass;
  };
  var isMonomerOfClass = (monomer, monomerClass) => {
    if (getMonomerClass2(monomer) === monomerClass) return true;
    if (monomerClass === "Sugar" /* Sugar */) return Boolean(monomer == null ? void 0 : monomer.isSugar);
    if (monomerClass === "Phosphate" /* Phosphate */)
      return Boolean(monomer == null ? void 0 : monomer.isPhosphate);
    return false;
  };
  var CHAIN_MONOMER_TYPE_TO_CLASS = {
    Peptide: "AminoAcid" /* AminoAcid */,
    Phosphate: "Phosphate" /* Phosphate */,
    Sugar: "Sugar" /* Sugar */,
    UnsplitNucleotide: "RNA" /* RNA */
  };
  function getMonomerUniqueKey(monomer) {
    return `${monomer.props.MonomerName}___${monomer.props.Name}`;
  }
  function isMonomerConnectedToR2RnaBase2(monomer) {
    if (!monomer) {
      return false;
    }
    const R1PolymerBond = monomer.attachmentPointsToBonds.R1;
    if (R1PolymerBond instanceof MonomerToAtomBond) {
      return false;
    }
    const R1ConnectedMonomer = R1PolymerBond == null ? void 0 : R1PolymerBond.getAnotherMonomer(monomer);
    const r2PolymerBond = R1ConnectedMonomer == null ? void 0 : R1ConnectedMonomer.attachmentPointsToBonds.R2;
    return isRnaBaseOrAmbiguousRnaBase2(R1ConnectedMonomer) && getSugarFromRnaBase2(R1ConnectedMonomer) && r2PolymerBond instanceof PolymerBond && (r2PolymerBond == null ? void 0 : r2PolymerBond.getAnotherMonomer(R1ConnectedMonomer)) === monomer;
  }
  function getPreviousMonomerInChain(monomer) {
    const r1PolymerBond = monomer.attachmentPointsToBonds.R1;
    const previousMonomer = r1PolymerBond instanceof PolymerBond ? r1PolymerBond == null ? void 0 : r1PolymerBond.getAnotherMonomer(monomer) : void 0;
    if (!previousMonomer || !(r1PolymerBond instanceof PolymerBond)) {
      return void 0;
    }
    return previousMonomer && previousMonomer.getAttachmentPointByBond(r1PolymerBond) === "R2" /* R2 */ ? previousMonomer : void 0;
  }
  function getNextMonomerInChain(monomer, firstMonomer) {
    var _a;
    if (!monomer) return void 0;
    const r2PolymerBond = monomer.attachmentPointsToBonds.R2;
    const nextMonomer = r2PolymerBond instanceof PolymerBond ? (_a = r2PolymerBond == null ? void 0 : r2PolymerBond.getAnotherMonomer) == null ? void 0 : _a.call(r2PolymerBond, monomer) : void 0;
    if (!nextMonomer || nextMonomer === firstMonomer && r2PolymerBond || isMonomerConnectedToR2RnaBase2(nextMonomer))
      return void 0;
    return r2PolymerBond && (nextMonomer == null ? void 0 : nextMonomer.getAttachmentPointByBond(r2PolymerBond)) === "R1" /* R1 */ ? nextMonomer : void 0;
  }
  function getRnaBaseFromSugar(monomer) {
    if (!monomer || !isMonomerOfClass(monomer, "Sugar" /* Sugar */))
      return void 0;
    const r3PolymerBond = monomer.attachmentPointsToBonds.R3;
    const r3ConnectedMonomer = r3PolymerBond instanceof PolymerBond ? r3PolymerBond == null ? void 0 : r3PolymerBond.getAnotherMonomer(monomer) : void 0;
    if (!r3ConnectedMonomer) {
      return void 0;
    }
    const r1PolymerBondOfConnectedMonomer = r3ConnectedMonomer == null ? void 0 : r3ConnectedMonomer.attachmentPointsToBonds.R1;
    const r1ConnectedMonomer = r1PolymerBondOfConnectedMonomer instanceof PolymerBond ? r1PolymerBondOfConnectedMonomer == null ? void 0 : r1PolymerBondOfConnectedMonomer.getAnotherMonomer(r3ConnectedMonomer) : void 0;
    return isRnaBaseOrAmbiguousRnaBase2(r3ConnectedMonomer) && r1ConnectedMonomer === monomer ? r3ConnectedMonomer : void 0;
  }
  function getSugarFromRnaBase2(monomer) {
    if (!monomer || !isRnaBaseOrAmbiguousRnaBase2(monomer)) return void 0;
    const r1PolymerBond = monomer.attachmentPointsToBonds.R1;
    const r1ConnectedMonomer = r1PolymerBond instanceof PolymerBond ? r1PolymerBond == null ? void 0 : r1PolymerBond.getAnotherMonomer(monomer) : void 0;
    if (!r1ConnectedMonomer) {
      return void 0;
    }
    const r3PolymerBondOfConnectedMonomer = r1ConnectedMonomer == null ? void 0 : r1ConnectedMonomer.attachmentPointsToBonds.R3;
    const r3ConnectedMonomer = r3PolymerBondOfConnectedMonomer instanceof PolymerBond ? r3PolymerBondOfConnectedMonomer == null ? void 0 : r3PolymerBondOfConnectedMonomer.getAnotherMonomer(r1ConnectedMonomer) : void 0;
    return isMonomerOfClass(r1ConnectedMonomer, "Sugar" /* Sugar */) && r3ConnectedMonomer === monomer ? r1ConnectedMonomer : void 0;
  }
  function getPhosphateFromSugar(monomer) {
    if (!monomer) return void 0;
    const nextMonomerInChain = getNextMonomerInChain(monomer);
    return isMonomerOfClass(nextMonomerInChain, "Phosphate" /* Phosphate */) ? nextMonomerInChain : void 0;
  }
  function isValidNucleotide(sugar, firstMonomerInCyclicChain) {
    if (!getRnaBaseFromSugar(sugar)) {
      return false;
    }
    const phosphate = getPhosphateFromSugar(sugar);
    if (!phosphate || phosphate === firstMonomerInCyclicChain) {
      return false;
    }
    const nextMonomerAfterPhosphate = getNextMonomerInChain(phosphate);
    return !!nextMonomerAfterPhosphate;
  }
  function isValidNucleoside(sugar, firstMonomerInCyclicChain) {
    if (!getRnaBaseFromSugar(sugar)) {
      return false;
    }
    const phosphate = getPhosphateFromSugar(sugar);
    if (!phosphate || phosphate === firstMonomerInCyclicChain) {
      return true;
    }
    const nextMonomerAfterPhosphate = getNextMonomerInChain(phosphate);
    return !nextMonomerAfterPhosphate;
  }
  function isAmbiguousMonomerLibraryItem(monomer) {
    return Boolean(monomer && monomer.isAmbiguous);
  }
  function isRnaBaseOrAmbiguousRnaBase2(monomer) {
    return isMonomerOfClass(monomer, "Base" /* Base */) || isAmbiguousMonomerEntity(monomer) && monomer.monomerClass === "Base" /* Base */;
  }
  function isPhosphateOrAmbiguousPhosphate(monomer) {
    return isMonomerOfClass(monomer, "Phosphate" /* Phosphate */) || isAmbiguousMonomerEntity(monomer) && monomer.monomerClass === "Phosphate" /* Phosphate */;
  }
  function isSugarOrAmbiguousSugar(monomer) {
    return isMonomerOfClass(monomer, "Sugar" /* Sugar */) || isAmbiguousMonomerEntity(monomer) && monomer.monomerClass === "Sugar" /* Sugar */;
  }
  function isRnaBaseApplicableForAntisense(monomer) {
    return isMonomerOfClass(monomer, "RNA" /* RNA */) || isRnaBaseOrAmbiguousRnaBase2(monomer) && Boolean(getSugarFromRnaBase2(monomer));
  }

  // src/core/io/ket/helpers.ts
  var customizer = (value) => {
    if (typeof value === "object" && value.y) {
      const clonedValue = (0, import_lodash5.cloneDeep)(value);
      clonedValue.y = -clonedValue.y;
      return clonedValue;
    }
  };
  var getNodeWithInvertedYCoord = (node) => (0, import_lodash5.cloneDeepWith)(node, customizer);
  var setMonomerTemplatePrefix = (templateName) => `monomerTemplate-${templateName}`;
  var setMonomerPrefix = (monomerId) => `monomer${monomerId}`;
  var setMonomerGroupTemplatePrefix = (templateName) => `${KetTemplateType.MONOMER_GROUP_TEMPLATE}-${templateName}`;
  var setAmbiguousMonomerTemplatePrefix = (templateName) => `ambiguousMonomerTemplate-${templateName}`;
  var setAmbiguousMonomerPrefix = (monomerId) => `ambiguousMonomer${monomerId}`;
  var getKetRef = (entityId) => {
    return { $ref: entityId };
  };
  var getMonomerTemplateRefFromMonomerItem = (monomerItem) => {
    const { props } = monomerItem;
    if (props.id) {
      return setMonomerTemplatePrefix(props.id);
    }
    return setMonomerTemplatePrefix(getMonomerUniqueKey(monomerItem));
  };
  var getHELMClassByKetMonomerClass = (monomerClass) => {
    if (monomerClass === "AminoAcid" /* AminoAcid */) {
      return MONOMER_CONST.PEPTIDE;
    }
    if (monomerClass === "CHEM" /* CHEM */) {
      return MONOMER_CONST.CHEM;
    }
    return MONOMER_CONST.RNA;
  };
  var fillNaturalAnalogueForPhosphateAndSugar = (naturalAnalogue, monomerClass) => {
    if (naturalAnalogue !== "") {
      return naturalAnalogue;
    }
    if (monomerClass === "Sugar" /* Sugar */) {
      return "R" /* SUGAR_RNA */;
    }
    if (monomerClass === "Phosphate" /* Phosphate */) {
      return "P" /* PHOSPHATE */;
    }
    return naturalAnalogue;
  };
  var rotateCoordAxisBy180Degrees = (position, axis) => {
    const rotatedPosition = {
      x: position.x,
      y: position.y,
      z: position.z
    };
    rotatedPosition[axis] = -rotatedPosition[axis];
    return new Vec2(rotatedPosition.x, rotatedPosition.y, rotatedPosition.z);
  };
  var switchIntoChemistryCoordSystem = (position) => {
    return rotateCoordAxisBy180Degrees(position, "y" /* y */);
  };
  var modifyTransformation = (transformation) => {
    const { rotate } = transformation;
    const newTransformation = (0, import_lodash5.cloneDeep)(transformation);
    if (rotate) {
      newTransformation.rotate = -rotate;
    }
    return newTransformation;
  };
  var populateStructWithSelection = (populatedStruct, selection, resetSelection = false) => {
    if (!selection) {
      return populatedStruct;
    }
    Object.keys(selection).forEach((entity) => {
      var _a;
      const selectedEntities = selection[entity];
      (_a = populatedStruct[entity]) == null ? void 0 : _a.forEach((value, key) => {
        if (typeof value.setInitiallySelected === "function") {
          if (resetSelection) {
            value.setInitiallySelected(
              selectedEntities.includes(key) || void 0
            );
          } else if (selectedEntities.includes(key)) {
            value.setInitiallySelected(true);
          }
        }
      });
    });
    return populatedStruct;
  };

  // domain/entities/image.ts
  var Image = class _Image extends BaseMicromoleculeEntity {
    constructor(bitmap, _center, halfSize) {
      super();
      this.bitmap = bitmap;
      this._center = _center;
      this.halfSize = halfSize;
    }
    getTopLeftPosition() {
      return this._center.sub(this.halfSize);
    }
    getTopRightPosition() {
      return new Vec2(
        this._center.x + this.halfSize.x,
        this._center.y - this.halfSize.y
      );
    }
    getBottomRightPosition() {
      return this._center.add(this.halfSize);
    }
    getBottomLeftPosition() {
      return new Vec2(
        this._center.x - this.halfSize.x,
        this._center.y + this.halfSize.y
      );
    }
    getCornerPositions() {
      return [
        this.getTopLeftPosition(),
        this.getTopRightPosition(),
        this.getBottomRightPosition(),
        this.getBottomLeftPosition()
      ];
    }
    getReferencePositions() {
      const [
        topLeftPosition,
        topRightPosition,
        bottomRightPosition,
        bottomLeftPosition
      ] = this.getCornerPositions();
      return {
        topLeftPosition,
        topMiddlePosition: Vec2.centre(topLeftPosition, topRightPosition),
        topRightPosition,
        rightMiddlePosition: Vec2.centre(topRightPosition, bottomRightPosition),
        bottomRightPosition,
        bottomMiddlePosition: Vec2.centre(
          bottomLeftPosition,
          bottomRightPosition
        ),
        bottomLeftPosition,
        leftMiddlePosition: Vec2.centre(topLeftPosition, bottomLeftPosition)
      };
    }
    clone() {
      return new _Image(
        this.bitmap,
        new Vec2(this._center),
        new Vec2(this.halfSize)
      );
    }
    addPositionOffset(offset) {
      this._center = this._center.add(offset);
    }
    resize(topLeftPosition, bottomRightPosition) {
      this._center = Vec2.centre(topLeftPosition, bottomRightPosition);
      const halfSize = Vec2.diff(bottomRightPosition, topLeftPosition).scaled(
        0.5
      );
      this.halfSize = new Vec2(Math.abs(halfSize.x), Math.abs(halfSize.y));
    }
    rescaleSize(scale) {
      this.halfSize = this.halfSize.scaled(scale);
    }
    center() {
      return this._center;
    }
    toKetNode() {
      var _a;
      const topLeftCorner = this.getTopLeftPosition();
      const base64Data = this.bitmap.replace(/^.*;base64,/, "");
      const format = (_a = /^data:(image\/.*);base64,/.exec(this.bitmap)) == null ? void 0 : _a[1];
      return {
        type: IMAGE_SERIALIZE_KEY,
        center: getNodeWithInvertedYCoord(this._center),
        format,
        boundingBox: __spreadProps(__spreadValues({}, getNodeWithInvertedYCoord(topLeftCorner)), {
          width: this.halfSize.x * 2,
          height: this.halfSize.y * 2
        }),
        data: base64Data,
        selected: this.getInitiallySelected()
      };
    }
    static fromKetNode(ketFileNode) {
      const _a = getNodeWithInvertedYCoord(
        ketFileNode.boundingBox
      ), { width, height } = _a, point = __objRest(_a, ["width", "height"]);
      const halfSize = new Vec2(width / 2, height / 2);
      const topLeftCorner = new Vec2(point);
      const center = topLeftCorner.add(halfSize);
      const imageSrc = `data:${ketFileNode.format};base64,${ketFileNode.data}`;
      const image = new _Image(imageSrc, center, halfSize);
      image.setInitiallySelected(ketFileNode.selected);
      return image;
    }
  };

  // domain/entities/multitailArrow.ts
  var _MultitailArrow = class _MultitailArrow extends BaseMicromoleculeEntity {
    constructor(spineTopX, spineTopY, height, headOffsetX, headOffsetY, tailLength, tailsYOffset, arrowId) {
      super();
      this.spineTopX = spineTopX;
      this.spineTopY = spineTopY;
      this.height = height;
      this.headOffsetX = headOffsetX;
      this.headOffsetY = headOffsetY;
      this.tailLength = tailLength;
      this.tailsYOffset = tailsYOffset;
      __publicField(this, "arrowId");
      this.arrowId = arrowId;
    }
    static canAddTail(distance) {
      return distance >= _MultitailArrow.MIN_TAIL_DISTANCE.multiply(2).getFloatingPrecision();
    }
    static fromTwoPoints(topLeft, bottomRight) {
      const center = Vec2.centre(topLeft, bottomRight);
      const tailLength = FixedPrecisionCoordinates.fromFloatingPrecision(
        Math.max(
          (bottomRight.x - topLeft.x) / 3,
          _MultitailArrow.MIN_TAIL_LENGTH.getFloatingPrecision()
        )
      );
      const topSpineX = FixedPrecisionCoordinates.fromFloatingPrecision(
        topLeft.x
      ).add(tailLength);
      const topSpineY = FixedPrecisionCoordinates.fromFloatingPrecision(
        topLeft.y
      );
      const height = FixedPrecisionCoordinates.fromFloatingPrecision(
        Math.max(
          bottomRight.y - topLeft.y,
          _MultitailArrow.MIN_HEIGHT.getFloatingPrecision()
        )
      );
      const headOffsetX = FixedPrecisionCoordinates.fromFloatingPrecision(
        bottomRight.x
      ).sub(topSpineX);
      const headOffsetY = FixedPrecisionCoordinates.fromFloatingPrecision(
        center.y
      ).sub(topSpineY);
      return new _MultitailArrow(
        topSpineX,
        topSpineY,
        height,
        headOffsetX,
        headOffsetY,
        tailLength,
        new Pool()
      );
    }
    static validateKetNode(ketFileData) {
      var _a, _b;
      const { head, spine, tails } = ketFileData;
      const [spineStart, spineEnd] = spine.pos;
      const spineStartX = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineStart.x
      );
      const spineStartY = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineStart.y
      );
      const spineEndX = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineEnd.x
      );
      const spineEndY = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineEnd.y
      );
      const headX = FixedPrecisionCoordinates.fromFloatingPrecision(
        head.position.x
      );
      const headY = FixedPrecisionCoordinates.fromFloatingPrecision(
        head.position.y
      );
      const tailsFixedPrecision = tails.pos.map((tail) => ({
        x: FixedPrecisionCoordinates.fromFloatingPrecision(tail.x),
        y: FixedPrecisionCoordinates.fromFloatingPrecision(tail.y)
      }));
      tailsFixedPrecision.sort((a, b) => b.y.value - a.y.value);
      if (spineStartX.value !== spineEndX.value || spineStartY.value < spineEndY.sub(_MultitailArrow.KET_MIN_DISTANCE).value) {
        return "INCORRECT_SPINE" /* INCORRECT_SPINE */;
      }
      if (headX.value < spineStartX.add(_MultitailArrow.KET_MIN_DISTANCE).value || headY.sub(_MultitailArrow.KET_MIN_DISTANCE).value < spineEndY.value || headY.add(_MultitailArrow.KET_MIN_DISTANCE).value > spineStartY.value) {
        return "INCORRECT_HEAD" /* INCORRECT_HEAD */;
      }
      if (((_a = tailsFixedPrecision.at(0)) == null ? void 0 : _a.y.value) !== spineStartY.value || ((_b = tailsFixedPrecision.at(-1)) == null ? void 0 : _b.y.value) !== spineEndY.value) {
        return "INCORRECT_TAILS" /* INCORRECT_TAILS */;
      }
      const firstTailX = tailsFixedPrecision[0].x;
      if (firstTailX.value > spineStartX.sub(_MultitailArrow.KET_MIN_DISTANCE).value) {
        return "INCORRECT_TAILS" /* INCORRECT_TAILS */;
      }
      const result = tailsFixedPrecision.every((tail, index, allTails) => {
        if (index > 0 && allTails[index - 1].y.value < tail.y.add(_MultitailArrow.KET_MIN_DISTANCE).value) {
          return false;
        }
        return tail.x.value === firstTailX.value && tail.y.value >= spineEndY.value && tail.y.value <= spineStartY.value;
      });
      return !result ? "INCORRECT_TAILS" /* INCORRECT_TAILS */ : null;
    }
    static getConstructorParamsFromKetNode(ketFileNode) {
      const data = getNodeWithInvertedYCoord(
        ketFileNode.data
      );
      const [spineStart, spineEnd] = data.spine.pos;
      const head = data.head.position;
      const spineTopX = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineStart.x
      );
      const spineTopY = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineStart.y
      );
      const height = FixedPrecisionCoordinates.fromFloatingPrecision(
        spineEnd.y
      ).sub(spineTopY);
      const headOffsetX = FixedPrecisionCoordinates.fromFloatingPrecision(
        head.x
      ).sub(spineTopX);
      const headOffsetY = FixedPrecisionCoordinates.fromFloatingPrecision(
        head.y
      ).sub(spineTopY);
      const tailsYOffset = new Pool();
      const tails = [...data.tails.pos];
      tails.sort((a, b) => a.y - b.y);
      const tailsLength = spineTopX.sub(
        FixedPrecisionCoordinates.fromFloatingPrecision(tails[0].x)
      );
      tails.slice(1, -1).forEach((tail) => {
        tailsYOffset.add(
          FixedPrecisionCoordinates.fromFloatingPrecision(tail.y).sub(spineTopY)
        );
      });
      return {
        spineTopX,
        spineTopY,
        height,
        headOffsetX,
        headOffsetY,
        tailsLength,
        tailsYOffset
      };
    }
    static fromKetNode(ketFileNode) {
      const {
        spineTopX,
        spineTopY,
        height,
        headOffsetX,
        headOffsetY,
        tailsLength,
        tailsYOffset
      } = _MultitailArrow.getConstructorParamsFromKetNode(ketFileNode);
      return new _MultitailArrow(
        spineTopX,
        spineTopY,
        height,
        headOffsetX,
        headOffsetY,
        tailsLength,
        tailsYOffset
      );
    }
    static fromFloatingPointCoordinates(spineTop, height, headOffset, tailLength, tailsYOffset) {
      const tailsYOffsetFixedPrecision = tailsYOffset.clone();
      tailsYOffsetFixedPrecision.forEach((item, key, map) => {
        const pool = map;
        pool.set(key, FixedPrecisionCoordinates.fromFloatingPrecision(item));
      });
      return new _MultitailArrow(
        FixedPrecisionCoordinates.fromFloatingPrecision(spineTop.x),
        FixedPrecisionCoordinates.fromFloatingPrecision(spineTop.y),
        FixedPrecisionCoordinates.fromFloatingPrecision(height),
        FixedPrecisionCoordinates.fromFloatingPrecision(headOffset.x),
        FixedPrecisionCoordinates.fromFloatingPrecision(headOffset.y),
        FixedPrecisionCoordinates.fromFloatingPrecision(tailLength),
        tailsYOffsetFixedPrecision
      );
    }
    static getReferencePositions(spineTopX, spineTopY, height, headOffsetX, headOffsetY, tailLength, tailsYOffset) {
      const tailX = spineTopX.sub(tailLength);
      const bottomY = spineTopY.add(height);
      const tails = new Pool();
      tailsYOffset.forEach((tailYOffset, key) => {
        tails.set(
          key,
          new Vec2(
            tailX.getFloatingPrecision(),
            spineTopY.add(tailYOffset).getFloatingPrecision()
          )
        );
      });
      return {
        head: new Vec2(
          spineTopX.add(headOffsetX).getFloatingPrecision(),
          spineTopY.add(headOffsetY).getFloatingPrecision()
        ),
        topTail: new Vec2(
          tailX.getFloatingPrecision(),
          spineTopY.getFloatingPrecision()
        ),
        bottomTail: new Vec2(
          tailX.getFloatingPrecision(),
          bottomY.getFloatingPrecision()
        ),
        topSpine: new Vec2(
          spineTopX.getFloatingPrecision(),
          spineTopY.getFloatingPrecision()
        ),
        bottomSpine: new Vec2(
          spineTopX.getFloatingPrecision(),
          bottomY.getFloatingPrecision()
        ),
        tails
      };
    }
    getReferencePositions() {
      return _MultitailArrow.getReferencePositions(
        this.spineTopX,
        this.spineTopY,
        this.height,
        this.headOffsetX,
        this.headOffsetY,
        this.tailLength,
        this.tailsYOffset
      );
    }
    getReferencePositionsArray() {
      const _a = this.getReferencePositions(), { tails } = _a, positions = __objRest(_a, ["tails"]);
      return Object.values(positions).concat(Array.from(tails.values()));
    }
    getReferenceLines(referencePositions) {
      const spineX = referencePositions.topSpine.x;
      const headSpinePosition = new Vec2(spineX, referencePositions.head.y);
      const tails = new Pool();
      referencePositions.tails.forEach((tail, key) => {
        tails.set(key, [tail, new Vec2(spineX, tail.y)]);
      });
      return {
        topTail: [referencePositions.topTail, referencePositions.topSpine],
        bottomTail: [
          referencePositions.bottomTail,
          referencePositions.bottomSpine
        ],
        spine: [referencePositions.topSpine, referencePositions.bottomSpine],
        head: [headSpinePosition, referencePositions.head],
        tails
      };
    }
    getTailsDistance(tailsYOffsets) {
      const allTailsOffsets = tailsYOffsets.concat([
        new FixedPrecisionCoordinates(0),
        this.height
      ]);
      allTailsOffsets.sort((a, b) => a.sub(b).getFloatingPrecision());
      return allTailsOffsets.reduce(
        (acc, item, index, array) => {
          if (index === 0) {
            return acc;
          }
          const distance = item.sub(array[index - 1]);
          const centerFloatingPoint = item.sub(distance.divide(2));
          return acc.concat({
            distance: distance.getFloatingPrecision(),
            center: centerFloatingPoint.getFloatingPrecision()
          });
        },
        []
      );
    }
    getTailsMaxDistance() {
      return this.getTailsDistance(Array.from(this.tailsYOffset.values())).reduce(
        (acc, item) => {
          return item.distance > acc.distance ? item : acc;
        },
        { distance: 0, center: 0 }
      );
    }
    getTailCoordinate(id2) {
      return this.tailsYOffset.get(id2);
    }
    addTail(id2, coordinate) {
      if (typeof id2 === "number" && coordinate) {
        this.tailsYOffset.set(id2, coordinate);
        return id2;
      }
      const { center, distance } = this.getTailsMaxDistance();
      if (!_MultitailArrow.canAddTail(distance)) {
        throw String("Cannot add tail because no minimal distance found");
      }
      const centerFixedPrecision = FixedPrecisionCoordinates.fromFloatingPrecision(center);
      if (typeof id2 === "number") {
        this.tailsYOffset.set(id2, centerFixedPrecision);
        return id2;
      } else {
        return this.tailsYOffset.add(centerFixedPrecision);
      }
    }
    removeTail(id2) {
      this.tailsYOffset.delete(id2);
    }
    center() {
      return Vec2.centre(
        new Vec2(
          this.spineTopX.sub(this.tailLength).getFloatingPrecision(),
          this.spineTopY.getFloatingPrecision()
        ),
        new Vec2(
          this.spineTopX.add(this.headOffsetX).getFloatingPrecision(),
          this.spineTopY.add(this.height).getFloatingPrecision()
        )
      );
    }
    clone() {
      return new _MultitailArrow(
        this.spineTopX,
        this.spineTopY,
        this.height,
        this.headOffsetX,
        this.headOffsetY,
        this.tailLength,
        this.tailsYOffset.clone(),
        this.arrowId
      );
    }
    rescaleSize(scale) {
      this.spineTopX = this.spineTopX.multiply(scale);
      this.spineTopY = this.spineTopY.multiply(scale);
      this.headOffsetX = this.headOffsetX.multiply(scale);
      this.headOffsetY = this.headOffsetY.multiply(scale);
      this.height = this.height.multiply(scale);
      this.tailLength = this.tailLength.multiply(scale);
      this.tailsYOffset.forEach((item, index) => {
        this.tailsYOffset.set(index, item.multiply(scale));
      });
    }
    resizeHead(offset) {
      const fixedPrecisionOffset = FixedPrecisionCoordinates.fromFloatingPrecision(offset);
      const headOffsetX = new FixedPrecisionCoordinates(
        Math.max(
          this.headOffsetX.add(fixedPrecisionOffset).value,
          _MultitailArrow.MIN_HEAD_LENGTH.value
        )
      );
      const realOffset = headOffsetX.sub(this.headOffsetX);
      this.headOffsetX = headOffsetX;
      return realOffset.getFloatingPrecision();
    }
    moveHead(offset) {
      const fixedPrecisionOffset = FixedPrecisionCoordinates.fromFloatingPrecision(offset);
      const headOffsetY = new FixedPrecisionCoordinates(
        Math.min(
          Math.max(
            _MultitailArrow.MIN_TOP_BOTTOM_OFFSET.value,
            this.headOffsetY.add(fixedPrecisionOffset).value
          ),
          this.height.sub(_MultitailArrow.MIN_TOP_BOTTOM_OFFSET).value
        )
      );
      const realOffset = headOffsetY.sub(this.headOffsetY);
      this.headOffsetY = headOffsetY;
      return realOffset.getFloatingPrecision();
    }
    resizeTails(offset) {
      const fixedPrecisionOffset = FixedPrecisionCoordinates.fromFloatingPrecision(offset);
      const updatedLength = new FixedPrecisionCoordinates(
        Math.max(
          this.tailLength.sub(fixedPrecisionOffset).value,
          _MultitailArrow.MIN_TAIL_LENGTH.value
        )
      );
      const realOffset = this.tailLength.sub(updatedLength);
      this.tailLength = updatedLength;
      return realOffset.getFloatingPrecision();
    }
    normalizeTailPosition(proposedPosition, tailId) {
      const proposedPositionFloatingPrecision = proposedPosition.getFloatingPrecision();
      const getDistanceToTailDistance = (tailDistance) => Math.abs(tailDistance.center - proposedPositionFloatingPrecision) - tailDistance.distance / 2;
      const tailsWithoutCurrent = Array.from(this.tailsYOffset.entries()).filter(([key]) => key !== tailId).map(([_, value]) => value);
      const tailDistances = this.getTailsDistance(tailsWithoutCurrent).filter(
        (item) => _MultitailArrow.canAddTail(item.distance)
      );
      tailDistances.sort(
        (a, b) => getDistanceToTailDistance(a) - getDistanceToTailDistance(b)
      );
      const tailMinDistance = tailDistances.at(0);
      if (!tailMinDistance) {
        return null;
      }
      const positionCenter = FixedPrecisionCoordinates.fromFloatingPrecision(
        tailMinDistance.center
      );
      const positionDistance = FixedPrecisionCoordinates.fromFloatingPrecision(
        tailMinDistance.distance
      );
      const maxDistanceFromCenter = positionDistance.divide(2).sub(_MultitailArrow.MIN_TAIL_DISTANCE);
      if (Math.abs(positionCenter.sub(proposedPosition).value) >= maxDistanceFromCenter.value) {
        const distanceFromCenter = positionCenter.value > proposedPosition.value ? maxDistanceFromCenter.multiply(-1) : maxDistanceFromCenter;
        return positionCenter.add(distanceFromCenter);
      }
      return proposedPosition;
    }
    moveTail(offset, second, normalize) {
      const offsetFixedPrecision = FixedPrecisionCoordinates.fromFloatingPrecision(offset);
      const minHeight = new FixedPrecisionCoordinates(
        Math.max(
          _MultitailArrow.MIN_TAIL_DISTANCE.multiply(this.tailsYOffset.size + 1).value,
          _MultitailArrow.MIN_HEIGHT.value
        )
      );
      const tailsOffset = Array.from(this.tailsYOffset.values());
      tailsOffset.sort((a, b) => a.value - b.value);
      const lastTail = tailsOffset.at(-1) || new FixedPrecisionCoordinates(0);
      const firstTail = tailsOffset.at(0) || new FixedPrecisionCoordinates(Infinity);
      const closestTopLimit = new FixedPrecisionCoordinates(
        Math.min(
          firstTail.sub(_MultitailArrow.MIN_TAIL_DISTANCE).value,
          this.headOffsetY.sub(_MultitailArrow.MIN_TOP_BOTTOM_OFFSET).value
        )
      );
      const closestBottomLimit = new FixedPrecisionCoordinates(
        Math.max(
          lastTail.add(_MultitailArrow.MIN_TAIL_DISTANCE).value,
          this.headOffsetY.add(_MultitailArrow.MIN_TOP_BOTTOM_OFFSET).value
        )
      );
      if (typeof second === "number") {
        const originalValue = this.tailsYOffset.get(
          second
        );
        let updatedHeight = new FixedPrecisionCoordinates(
          Math.max(
            _MultitailArrow.MIN_TAIL_DISTANCE.value,
            Math.min(
              originalValue.add(offsetFixedPrecision).value,
              this.height.sub(_MultitailArrow.MIN_TAIL_DISTANCE).value
            )
          )
        );
        if (normalize) {
          const result = this.normalizeTailPosition(updatedHeight, second);
          if (result === null) {
            return originalValue.getFloatingPrecision();
          }
          updatedHeight = result;
        }
        const realOffset = updatedHeight.sub(originalValue);
        this.tailsYOffset.set(second, updatedHeight);
        return realOffset.getFloatingPrecision();
      } else if (second === _MultitailArrow.BOTTOM_TAIL_NAME) {
        const updatedHeight = new FixedPrecisionCoordinates(
          Math.max(
            minHeight.value,
            this.height.add(offsetFixedPrecision).value,
            closestBottomLimit.value
          )
        );
        const realOffset = updatedHeight.sub(this.height);
        this.height = updatedHeight;
        return realOffset.getFloatingPrecision();
      } else {
        const realOffset = new FixedPrecisionCoordinates(
          Math.min(
            offsetFixedPrecision.value,
            closestTopLimit.value,
            this.height.sub(minHeight).value
          )
        );
        if (realOffset.value !== 0) {
          this.spineTopY = this.spineTopY.add(realOffset);
          this.headOffsetY = this.headOffsetY.sub(realOffset);
          this.height = this.height.sub(realOffset);
          const updatedTails = this.tailsYOffset.clone();
          updatedTails.forEach((item, key) => {
            updatedTails.set(key, item.sub(realOffset));
          });
          this.tailsYOffset = updatedTails;
        }
        return realOffset.getFloatingPrecision();
      }
    }
    move(offset) {
      this.spineTopX = this.spineTopX.add(
        FixedPrecisionCoordinates.fromFloatingPrecision(offset.x)
      );
      this.spineTopY = this.spineTopY.add(
        FixedPrecisionCoordinates.fromFloatingPrecision(offset.y)
      );
    }
    static getParametersForKetNode(spineTopX, spineTopY, headOffsetX, headOffsetY, tailLength, tailsYOffset, height, center, isInitiallySelected) {
      const head = new Vec2(
        spineTopX.add(headOffsetX).getFloatingPrecision(),
        spineTopY.add(headOffsetY).getFloatingPrecision()
      );
      const bottomY = spineTopY.add(height);
      const spine = [
        new Vec2(
          spineTopX.getFloatingPrecision(),
          spineTopY.getFloatingPrecision()
        ),
        new Vec2(
          spineTopX.getFloatingPrecision(),
          bottomY.getFloatingPrecision()
        )
      ];
      const tailX = spineTopX.sub(tailLength);
      const nonBorderTails = Array.from(tailsYOffset.values()).map(
        (yOffset) => spineTopY.add(yOffset)
      );
      const convertTail = (y) => new Vec2(tailX.getFloatingPrecision(), y.getFloatingPrecision());
      const tails = [spineTopY].concat(nonBorderTails).concat(bottomY).map(convertTail);
      return {
        type: MULTITAIL_ARROW_SERIALIZE_KEY,
        center,
        selected: isInitiallySelected,
        data: getNodeWithInvertedYCoord({
          head: {
            position: head
          },
          spine: {
            pos: spine
          },
          tails: {
            pos: tails
          },
          zOrder: 0
        })
      };
    }
    toKetNode() {
      return _MultitailArrow.getParametersForKetNode(
        this.spineTopX,
        this.spineTopY,
        this.headOffsetX,
        this.headOffsetY,
        this.tailLength,
        this.tailsYOffset,
        this.height,
        this.center(),
        this.getInitiallySelected()
      );
    }
  };
  __publicField(_MultitailArrow, "KET_MIN_DISTANCE", FixedPrecisionCoordinates.fromFloatingPrecision(0.01));
  __publicField(_MultitailArrow, "MIN_TAIL_DISTANCE", FixedPrecisionCoordinates.fromFloatingPrecision(0.35));
  __publicField(_MultitailArrow, "MIN_HEAD_LENGTH", FixedPrecisionCoordinates.fromFloatingPrecision(0.5));
  __publicField(_MultitailArrow, "MIN_TAIL_LENGTH", FixedPrecisionCoordinates.fromFloatingPrecision(0.4));
  __publicField(_MultitailArrow, "MIN_TOP_BOTTOM_OFFSET", FixedPrecisionCoordinates.fromFloatingPrecision(0.15));
  __publicField(_MultitailArrow, "MIN_HEIGHT", FixedPrecisionCoordinates.fromFloatingPrecision(0.5));
  __publicField(_MultitailArrow, "TOP_TAIL_NAME", "topTail");
  __publicField(_MultitailArrow, "BOTTOM_TAIL_NAME", "bottomTail");
  __publicField(_MultitailArrow, "TAILS_NAME", "tails");
  var MultitailArrow = _MultitailArrow;

  // domain/entities/sGroupAttachmentPoint.ts
  var SGroupAttachmentPoint = class _SGroupAttachmentPoint {
    constructor(atomId, leaveAtomId, attachmentId, attachmentPointNumber) {
      /**
       * This is the index of the atom in the S-group that serves as the attachment point.
       */
      __publicField(this, "atomId");
      /**
       * This is the index of the atom that is being replaced or removed at the attachment point
       * when the S-group is connected to another structure.
       * If no atom is being replaced, this value should be set to zero.
       *
       * NOTE: The logic is not supported in the current implementation of Sketch.
       * Only reading from file and saving to file.
       */
      __publicField(this, "leaveAtomId");
      /**
       * 2 character attachment identifier (for example, H or T for head/tail).
       * No validation of any kind is performed, and ‘ ’ is allowed.
       * ISIS/Desktop uses the first character as the ID of the leaving group
       * to attach if the bond between ooo and iii is deleted, and uses the second character
       * to indicate the sequence polarity: l for left, r for right, and x for none (a crosslink).
       *
       * NOTE: The logic is not supported in the current implementation of Sketch.
       * Only reading from file and saving to file.
       */
      __publicField(this, "attachmentId");
      // Field attachmentPointNumber is used for superatom attachment points
      // which are used for connection between molecule and monomer.
      __publicField(this, "attachmentPointNumber");
      this.atomId = atomId;
      this.leaveAtomId = leaveAtomId;
      this.attachmentId = attachmentId;
      this.attachmentPointNumber = attachmentPointNumber;
    }
    clone(atomIdMap) {
      const newAtomId = atomIdMap.get(this.atomId);
      assert_default(newAtomId != null);
      const newLeaveAtomId = atomIdMap.get(this.leaveAtomId);
      return new _SGroupAttachmentPoint(
        newAtomId,
        newLeaveAtomId,
        this.attachmentId,
        this.attachmentPointNumber
      );
    }
    /**
     * Trick: used for cloned struct for tooltips, for preview, for templates
     *
     * Why?
     * Currently, tooltips are implemented with removing sgroups (wrong implementation)
     * That's why we need to mark atoms as sgroup attachment points.
     *
     * If we change preview approach to flagged (option for showing sgroups without abbreviation),
     * then we will be able to remove this hack.
     */
    convertToRGroupAttachmentPointForDisplayPurpose(attachedAtomId) {
      return new RGroupAttachmentPoint(attachedAtomId, "primary");
    }
  };

  // src/core/chem/macromolecules/BaseMonomer.ts
  var import_lodash6 = __toESM(require_lodash2());
  var BaseMonomer = class _BaseMonomer extends DrawingEntity {
    constructor(monomerItem, _position, config) {
      var _a;
      super(_position, config);
      __publicField(this, "renderer");
      __publicField(this, "attachmentPointsToBonds", {});
      __publicField(this, "chosenFirstAttachmentPointForBond");
      __publicField(this, "potentialSecondAttachmentPointForBond");
      __publicField(this, "chosenSecondAttachmentPointForBond");
      __publicField(this, "potentialAttachmentPointsToBonds", {});
      __publicField(this, "attachmentPointsVisible", false);
      __publicField(this, "monomerItem");
      __publicField(this, "hydrogenBonds", []);
      this.monomerItem = __spreadValues({}, monomerItem);
      this.monomerItem.expanded = monomerItem.expanded;
      this.recalculateAttachmentPoints();
      this.monomerItem.attachmentPoints = (_a = this.monomerItem.attachmentPoints) != null ? _a : this.getMonomerDefinitionAttachmentPoints();
      this.chosenFirstAttachmentPointForBond = null;
      this.potentialSecondAttachmentPointForBond = null;
      this.chosenSecondAttachmentPointForBond = null;
    }
    get label() {
      return this.monomerItem.label;
    }
    get center() {
      return this.position;
    }
    get listOfAttachmentPoints() {
      const maxAttachmentPointNumber = this.getMaxAttachmentPointNumber();
      const attachmentPointList = [];
      for (let i = 1; i <= maxAttachmentPointNumber; i++) {
        const attachmentPointLabel = getAttachmentPointLabel(i);
        if (attachmentPointLabel in this.attachmentPointsToBonds) {
          attachmentPointList.push(attachmentPointLabel);
        }
      }
      return attachmentPointList;
    }
    turnOnAttachmentPointsVisibility() {
      this.attachmentPointsVisible = true;
    }
    turnOffAttachmentPointsVisibility() {
      this.attachmentPointsVisible = false;
    }
    setChosenFirstAttachmentPoint(attachmentPoint) {
      this.chosenFirstAttachmentPointForBond = attachmentPoint;
    }
    setChosenSecondAttachmentPoint(attachmentPoint) {
      this.chosenSecondAttachmentPointForBond = attachmentPoint;
    }
    setPotentialSecondAttachmentPoint(attachmentPoint) {
      this.potentialSecondAttachmentPointForBond = attachmentPoint;
    }
    setPotentialBond(attachmentPoint, potentialBond) {
      if (potentialBond instanceof HydrogenBond) {
        this.hydrogenBonds.push(potentialBond);
        return;
      }
      if (attachmentPoint !== void 0) {
        this.potentialAttachmentPointsToBonds[attachmentPoint] = potentialBond;
      }
    }
    getAttachmentPointByBond(bond) {
      if (bond instanceof HydrogenBond) {
        return this.hydrogenBonds.find((hydrogenBond) => hydrogenBond === bond) ? "hydrogen" /* HYDROGEN */ : void 0;
      }
      for (const attachmentPointName in this.attachmentPointsToBonds) {
        if (this.attachmentPointsToBonds[attachmentPointName] === bond) {
          return attachmentPointName;
        }
      }
      return void 0;
    }
    getPotentialAttachmentPointByBond(bond) {
      for (const attachmentPointName in this.potentialAttachmentPointsToBonds) {
        if (this.potentialAttachmentPointsToBonds[attachmentPointName] === bond) {
          return attachmentPointName;
        }
      }
      return void 0;
    }
    get firstFreeAttachmentPoint() {
      const maxAttachmentPointNumber = this.getMaxAttachmentPointNumber();
      for (let i = 1; i <= maxAttachmentPointNumber; i++) {
        const attachmentPoint = `R${i}`;
        if (this.hasAttachmentPoint(attachmentPoint) && this.attachmentPointsToBonds[attachmentPoint] === null) {
          return attachmentPoint;
        }
      }
      return void 0;
    }
    getMaxAttachmentPointNumber() {
      let maxAttachmentPointNumber = 1;
      for (const attachmentPoint in this.attachmentPointsToBonds) {
        const match = /R(\d+)/.exec(attachmentPoint);
        if (match) {
          const pointNumber = parseInt(match[1]);
          if (!isNaN(pointNumber) && pointNumber > maxAttachmentPointNumber) {
            maxAttachmentPointNumber = pointNumber;
          }
        }
      }
      return maxAttachmentPointNumber;
    }
    get R1AttachmentPoint() {
      if (this.attachmentPointsToBonds.R1 === null) {
        return "R1" /* R1 */;
      }
      return void 0;
    }
    get R2AttachmentPoint() {
      if (this.attachmentPointsToBonds.R2 === null) {
        return "R2" /* R2 */;
      }
      return void 0;
    }
    get hasFreeAttachmentPoint() {
      return Boolean(this.firstFreeAttachmentPoint);
    }
    isAttachmentPointExistAndFree(attachmentPoint) {
      return this.hasAttachmentPoint(attachmentPoint) && !this.isAttachmentPointUsed(attachmentPoint);
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    forEachBond(callback) {
      for (const attachmentPointName in this.attachmentPointsToBonds) {
        if (this.attachmentPointsToBonds[attachmentPointName]) {
          callback(
            this.attachmentPointsToBonds[attachmentPointName],
            attachmentPointName
          );
        }
      }
      this.hydrogenBonds.forEach((hydrogenBond) => {
        callback(hydrogenBond, "hydrogen" /* HYDROGEN */);
      });
    }
    setBond(attachmentPointName, bond) {
      if (!(bond instanceof HydrogenBond)) {
        this.attachmentPointsToBonds[attachmentPointName] = bond;
        return;
      }
      if (!this.hydrogenBonds.includes(bond)) {
        this.hydrogenBonds.push(bond);
      }
    }
    unsetBond(attachmentPointName, bondToDelete) {
      if (bondToDelete instanceof HydrogenBond) {
        this.hydrogenBonds = this.hydrogenBonds.filter(
          (bond) => bond !== bondToDelete
        );
        return;
      }
      if (attachmentPointName) {
        this.attachmentPointsToBonds[attachmentPointName] = null;
      }
    }
    get covalentBonds() {
      return (0, import_lodash6.compact)((0, import_lodash6.values)(this.attachmentPointsToBonds));
    }
    get polymerBonds() {
      return this.covalentBonds.filter(
        (bond) => bond instanceof PolymerBond
      );
    }
    get monomerToAtomBonds() {
      return this.bonds.filter(
        (bond) => bond instanceof MonomerToAtomBond
      );
    }
    get bonds() {
      return [...this.covalentBonds, ...this.hydrogenBonds];
    }
    get bondsSortedByLength() {
      const bonds = [...this.bonds];
      return bonds.sort((firstBond, secondBond) => {
        var _a, _b;
        if (!firstBond.secondEndEntity || !secondBond.secondEndEntity) {
          return 0;
        }
        const firstLength = Vec2.diff(
          firstBond.firstEndEntity.position,
          (_a = firstBond.secondEndEntity) == null ? void 0 : _a.position
        ).length();
        const secondLength = Vec2.diff(
          secondBond.firstEndEntity.position,
          (_b = secondBond.secondEndEntity) == null ? void 0 : _b.position
        ).length();
        return firstLength - secondLength;
      });
    }
    get polymerBondsSortedByLength() {
      return this.bondsSortedByLength.filter(
        (bond) => !(bond instanceof MonomerToAtomBond)
      );
    }
    get hasBonds() {
      let hasBonds = false;
      for (const bondName in this.attachmentPointsToBonds) {
        if (this.attachmentPointsToBonds[bondName]) {
          hasBonds = true;
        }
      }
      return hasBonds || this.hydrogenBonds.length > 0;
    }
    hasHydrogenBondWithMonomer(monomer) {
      return this.hydrogenBonds.find(
        (bond) => bond.firstMonomer === monomer || bond.secondMonomer === monomer
      );
    }
    hasPotentialBonds() {
      return Object.values(this.potentialAttachmentPointsToBonds).some(
        (bond) => !!bond
      );
    }
    getPotentialBond(attachmentPointName) {
      return this.potentialAttachmentPointsToBonds[attachmentPointName];
    }
    removeBond(polymerBond) {
      const attachmentPointName = this.getAttachmentPointByBond(polymerBond);
      if (!attachmentPointName) return;
      this.unsetBond(attachmentPointName);
    }
    removePotentialBonds(clearSelectedPoints = false) {
      if (clearSelectedPoints) {
        this.chosenFirstAttachmentPointForBond = null;
        this.chosenSecondAttachmentPointForBond = null;
        this.potentialSecondAttachmentPointForBond = null;
      }
      for (const attachmentPointName in this.potentialAttachmentPointsToBonds) {
        this.potentialAttachmentPointsToBonds[attachmentPointName] = null;
      }
    }
    get availableAttachmentPointForBondEnd() {
      if (this.chosenSecondAttachmentPointForBond) {
        return this.chosenSecondAttachmentPointForBond;
      }
      return this.firstFreeAttachmentPoint;
    }
    hasAttachmentPoint(attachmentPointName) {
      return this.attachmentPointsToBonds[attachmentPointName] !== void 0;
    }
    get isPhosphate() {
      return isMonomerItemPhosphate(this.monomerItem);
    }
    get isSugar() {
      return isMonomerItemSugar(this.monomerItem);
    }
    get usedAttachmentPointsNamesList() {
      const list = [];
      this.listOfAttachmentPoints.forEach((attachmentPointName) => {
        if (this.isAttachmentPointUsed(attachmentPointName)) {
          list.push(attachmentPointName);
        }
      });
      return list;
    }
    get unUsedAttachmentPointsNamesList() {
      const list = [];
      this.listOfAttachmentPoints.forEach((attachmentPointName) => {
        if (!this.isAttachmentPointUsed(attachmentPointName)) {
          list.push(attachmentPointName);
        }
      });
      return list;
    }
    getBondByAttachmentPoint(attachmentPointName) {
      return this.attachmentPointsToBonds[attachmentPointName];
    }
    getPotentialBondByAttachmentPoint(attachmentPointName) {
      return this.potentialAttachmentPointsToBonds[attachmentPointName];
    }
    isAttachmentPointUsed(attachmentPointName) {
      return Boolean(this.getBondByAttachmentPoint(attachmentPointName));
    }
    isAttachmentPointPotentiallyUsed(attachmentPointName) {
      return Boolean(this.getPotentialBondByAttachmentPoint(attachmentPointName));
    }
    getAttachmentPointDict() {
      if (this.monomerItem.attachmentPoints) {
        const { attachmentPointDictionary } = _BaseMonomer.getAttachmentPointDictFromMonomerDefinition(
          this.monomerItem.attachmentPoints
        );
        return attachmentPointDictionary;
      } else {
        return this.getAttachmentPointDictFromAtoms();
      }
    }
    static getAttachmentPointDictFromMonomerDefinition(attachmentPoints) {
      const attachmentPointDictionary = {};
      const attachmentPointsList = [];
      attachmentPoints.forEach((attachmentPoint, attachmentPointIndex) => {
        var _a;
        const attachmentPointNumber = attachmentPointIndex + 1;
        let calculatedAttachmentPointNumber;
        if (attachmentPoint.type) {
          if (attachmentPoint.type === "left") {
            calculatedAttachmentPointNumber = 1;
          } else if (attachmentPoint.type === "right") {
            calculatedAttachmentPointNumber = 2;
          } else if (attachmentPoint.type === "side") {
            calculatedAttachmentPointNumber = attachmentPointNumber + ("R1" in attachmentPointDictionary ? 0 : 1) + ("R2" in attachmentPointDictionary ? 0 : 1);
          } else {
            calculatedAttachmentPointNumber = attachmentPointNumber;
          }
        } else {
          calculatedAttachmentPointNumber = attachmentPointNumber;
        }
        const calculatedLabel = (_a = attachmentPoint.label) != null ? _a : `R${calculatedAttachmentPointNumber}`;
        attachmentPointDictionary[calculatedLabel] = null;
        attachmentPointsList.push(calculatedLabel);
      });
      return { attachmentPointDictionary, attachmentPointsList };
    }
    get attachmentPointNumberToType() {
      return {
        1: "left",
        2: "right",
        moreThanTwo: "side"
      };
    }
    getMonomerDefinitionAttachmentPoints() {
      const monomerDefinitionAttachmentPoints = [];
      this.superatomAttachmentPoints.forEach((superatomAttachmentPoint) => {
        var _a;
        if (!(0, import_lodash6.isNumber)(superatomAttachmentPoint.attachmentPointNumber)) {
          return;
        }
        const bondsToLeavingGroupAtom = this.monomerItem.struct.bonds.filter(
          (_, bond) => {
            return bond.begin === superatomAttachmentPoint.leaveAtomId || bond.end === superatomAttachmentPoint.leaveAtomId;
          }
        );
        if (bondsToLeavingGroupAtom.size > 1) {
          return;
        }
        monomerDefinitionAttachmentPoints.push({
          attachmentAtom: superatomAttachmentPoint.atomId,
          leavingGroup: {
            atoms: superatomAttachmentPoint.leaveAtomId === 0 || superatomAttachmentPoint.leaveAtomId ? [superatomAttachmentPoint.leaveAtomId] : []
          },
          type: (_a = this.attachmentPointNumberToType[superatomAttachmentPoint.attachmentPointNumber]) != null ? _a : this.attachmentPointNumberToType.moreThanTwo
        });
      });
      return monomerDefinitionAttachmentPoints;
    }
    get superatomAttachmentPoints() {
      var _a;
      const struct = this.monomerItem.struct;
      const superatomWithoutLabel = (_a = struct.sgroups.filter((_, sgroup) => sgroup.isSuperatomWithoutLabel)) == null ? void 0 : _a.get(0);
      if (!superatomWithoutLabel) {
        return [];
      }
      return superatomWithoutLabel.getAttachmentPoints();
    }
    getAttachmentPointDictFromAtoms() {
      const attachmentPointNameToBond = {};
      this.superatomAttachmentPoints.forEach((superatomAttachmentPoint) => {
        if (!(0, import_lodash6.isNumber)(superatomAttachmentPoint.attachmentPointNumber)) {
          return;
        }
        const label = getAttachmentPointLabel(
          superatomAttachmentPoint.attachmentPointNumber
        );
        const leavingGroupAtomId = superatomAttachmentPoint.leaveAtomId;
        const bondsToLeavingGroupAtom = this.monomerItem.struct.bonds.filter(
          (_, bond) => {
            return bond.begin === leavingGroupAtomId || bond.end === leavingGroupAtomId;
          }
        );
        if (bondsToLeavingGroupAtom.size > 1) {
          return;
        }
        attachmentPointNameToBond[label] = null;
      });
      return attachmentPointNameToBond;
    }
    get startBondAttachmentPoint() {
      if (this.chosenFirstAttachmentPointForBond) {
        return this.chosenFirstAttachmentPointForBond;
      }
      if (this.attachmentPointsToBonds.R2 === null) {
        return "R2" /* R2 */;
      }
      if (this.attachmentPointsToBonds.R1 === null) {
        return "R1" /* R1 */;
      }
      return this.firstFreeAttachmentPoint;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return this.SubChainConstructor !== monomerToChain.SubChainConstructor;
    }
    get isModification() {
      const naturalAnalogThreeLettersCode = this.monomerItem.props.MonomerNaturalAnalogThreeLettersCode;
      const naturalAnalogCode = this.monomerItem.props.MonomerNaturalAnalogCode;
      const namesToCompareNaturalAnalog = [
        this.label,
        this.monomerItem.props.MonomerName
      ];
      const naturalAnaloguesToCompare = [
        ...naturalAnalogThreeLettersCode ? [naturalAnalogThreeLettersCode] : [],
        naturalAnalogCode
      ];
      return namesToCompareNaturalAnalog.every((nameToCompare) => {
        if (naturalAnaloguesToCompare.includes(nameToCompare)) {
          return false;
        }
        const nameWithoutAsterisk = nameToCompare.replace(/\*$/, "");
        return !naturalAnaloguesToCompare.includes(nameWithoutAsterisk);
      });
    }
    get sideConnections() {
      const sideConnections = [];
      this.forEachBond((bond) => {
        if (!(bond instanceof MonomerToAtomBond) && bond.isSideChainConnection) {
          sideConnections.push(bond);
        }
      });
      return sideConnections;
    }
    get monomerCaps() {
      return this.monomerItem.props.MonomerCaps;
    }
    recalculateAttachmentPoints() {
      const oldAttachmentPointsToBonds = this.attachmentPointsToBonds;
      this.attachmentPointsToBonds = this.getAttachmentPointDict();
      for (const attachmentPointName in this.attachmentPointsToBonds) {
        if (oldAttachmentPointsToBonds[attachmentPointName]) {
          this.attachmentPointsToBonds[attachmentPointName] = oldAttachmentPointsToBonds[attachmentPointName];
        }
      }
      this.potentialAttachmentPointsToBonds = this.getAttachmentPointDict();
    }
  };

  // src/core/chem/macromolecules/monomer-chains/BaseSubChain.ts
  var BaseSubChain = class {
    constructor() {
      __publicField(this, "nodes", []);
      __publicField(this, "bonds", []);
      // TODO this flag is needed to track changes made to bonds in SequenceRenderer
      // Added to minimize impact on existing code
      // See TODO in Chain.ts get bonds() for proper implementation
      __publicField(this, "modified", true);
    }
    get lastNode() {
      return this.nodes[this.nodes.length - 1];
    }
    get firstNode() {
      return this.nodes[0];
    }
    add(node) {
      this.nodes.push(node);
      this.modified = true;
    }
    addBond(bond) {
      this.bonds.push(bond);
      this.modified = true;
    }
    get length() {
      return this.nodes.length;
    }
  };

  // src/core/chem/macromolecules/monomer-chains/PeptideSubChain.ts
  var PeptideSubChain = class extends BaseSubChain {
  };

  // src/core/chem/macromolecules/Peptide.ts
  var Peptide = class extends BaseMonomer {
    getValidSourcePoint(secondMonomer) {
      if (this.chosenFirstAttachmentPointForBond) {
        return this.chosenFirstAttachmentPointForBond;
      }
      if (this.unUsedAttachmentPointsNamesList.length === 1) {
        return this.unUsedAttachmentPointsNamesList[0];
      }
      if (secondMonomer == null ? void 0 : secondMonomer.potentialSecondAttachmentPointForBond) {
        if ((secondMonomer == null ? void 0 : secondMonomer.potentialSecondAttachmentPointForBond) === "R1" /* R1 */ && this.isAttachmentPointExistAndFree("R2" /* R2 */)) {
          return "R2" /* R2 */;
        }
        if ((secondMonomer == null ? void 0 : secondMonomer.potentialSecondAttachmentPointForBond) === "R2" /* R2 */ && this.isAttachmentPointExistAndFree("R1" /* R1 */)) {
          return "R1" /* R1 */;
        }
        return;
      }
      if ((!secondMonomer || secondMonomer.isAttachmentPointExistAndFree("R1" /* R1 */)) && this.isAttachmentPointExistAndFree("R2" /* R2 */)) {
        return "R2" /* R2 */;
      }
      if (this.isAttachmentPointExistAndFree("R1" /* R1 */) && (secondMonomer == null ? void 0 : secondMonomer.isAttachmentPointExistAndFree("R2" /* R2 */))) {
        return "R1" /* R1 */;
      }
      return void 0;
    }
    getValidTargetPoint(firstMonomer) {
      if (this.potentialSecondAttachmentPointForBond) {
        return this.potentialSecondAttachmentPointForBond;
      }
      if (this.unUsedAttachmentPointsNamesList.length === 1) {
        return this.unUsedAttachmentPointsNamesList[0];
      }
      if (firstMonomer == null ? void 0 : firstMonomer.chosenFirstAttachmentPointForBond) {
        if ((firstMonomer == null ? void 0 : firstMonomer.chosenFirstAttachmentPointForBond) === "R1" /* R1 */ && this.isAttachmentPointExistAndFree("R2" /* R2 */)) {
          return "R2" /* R2 */;
        }
        if ((firstMonomer == null ? void 0 : firstMonomer.chosenFirstAttachmentPointForBond) === "R2" /* R2 */ && this.isAttachmentPointExistAndFree("R1" /* R1 */)) {
          return "R1" /* R1 */;
        }
        return;
      }
      if (this.isAttachmentPointExistAndFree("R1" /* R1 */) && firstMonomer.isAttachmentPointExistAndFree("R2" /* R2 */)) {
        return "R1" /* R1 */;
      }
      if (firstMonomer.isAttachmentPointExistAndFree("R1" /* R1 */) && this.isAttachmentPointExistAndFree("R2" /* R2 */)) {
        return "R2" /* R2 */;
      }
      return void 0;
    }
    get SubChainConstructor() {
      return PeptideSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![PeptideSubChain].includes(monomerToChain.SubChainConstructor);
    }
  };

  // src/core/chem/macromolecules/monomer-chains/ChemSubChain.ts
  var ChemSubChain = class extends BaseSubChain {
  };

  // domain/entities/Chem.ts
  var Chem = class extends BaseMonomer {
    getValidSourcePoint(monomer) {
      return Peptide.prototype.getValidSourcePoint.call(this, monomer);
    }
    getValidTargetPoint(monomer) {
      return Peptide.prototype.getValidTargetPoint.call(this, monomer);
    }
    get SubChainConstructor() {
      return ChemSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![ChemSubChain].includes(monomerToChain.SubChainConstructor);
    }
  };

  // src/core/chem/macromolecules/monomer-chains/RnaSubChain.ts
  var RnaSubChain = class extends BaseSubChain {
  };

  // src/core/chem/macromolecules/monomer-chains/PhosphateSubChain.ts
  var PhosphateSubChain = class extends BaseSubChain {
  };

  // src/core/chem/macromolecules/Sugar.ts
  var Sugar = class _Sugar extends BaseMonomer {
    getValidSourcePoint(secondMonomer) {
      if (!secondMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return _Sugar.getValidPoint(
        this,
        secondMonomer,
        secondMonomer.potentialSecondAttachmentPointForBond
      );
    }
    getValidTargetPoint(firstMonomer) {
      if (!firstMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return _Sugar.getValidPoint(
        this,
        firstMonomer,
        firstMonomer.chosenFirstAttachmentPointForBond
      );
    }
    static getValidPoint(self2, otherMonomer, potentialPointOnOther) {
      if (self2.chosenFirstAttachmentPointForBond) {
        return self2.chosenFirstAttachmentPointForBond;
      }
      if (self2.potentialSecondAttachmentPointForBond) {
        return self2.potentialSecondAttachmentPointForBond;
      }
      if (self2.unUsedAttachmentPointsNamesList.length === 1) {
        return self2.unUsedAttachmentPointsNamesList[0];
      }
      if (!isPhosphateOrAmbiguousPhosphate(otherMonomer) && !isRnaBaseOrAmbiguousRnaBase2(otherMonomer)) {
        return;
      }
      if (isRnaBaseOrAmbiguousRnaBase2(otherMonomer)) {
        if (self2.isAttachmentPointExistAndFree("R3" /* R3 */)) {
          return "R3" /* R3 */;
        } else return;
      }
      if (potentialPointOnOther) {
        if (potentialPointOnOther === "R1" /* R1 */ && self2.isAttachmentPointExistAndFree("R2" /* R2 */)) {
          return "R2" /* R2 */;
        } else if (potentialPointOnOther !== "R1" /* R1 */ && self2.isAttachmentPointExistAndFree("R1" /* R1 */)) {
          return "R1" /* R1 */;
        } else {
          return;
        }
      }
      if (otherMonomer.isAttachmentPointExistAndFree("R1" /* R1 */) && self2.isAttachmentPointExistAndFree("R2" /* R2 */)) {
        return "R2" /* R2 */;
      }
      if (otherMonomer.isAttachmentPointExistAndFree("R2" /* R2 */) && self2.isAttachmentPointExistAndFree("R1" /* R1 */)) {
        return "R1" /* R1 */;
      }
      if (!otherMonomer.isAttachmentPointExistAndFree("R1" /* R1 */) && self2.isAttachmentPointExistAndFree("R1" /* R1 */)) {
        return "R1" /* R1 */;
      }
      return void 0;
    }
    get SubChainConstructor() {
      return RnaSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![PhosphateSubChain, RnaSubChain].includes(
        monomerToChain.SubChainConstructor
      );
    }
    get isPartOfRNA() {
      const r3PolymerBond = this.attachmentPointsToBonds.R3;
      return r3PolymerBond instanceof PolymerBond && isRnaBaseOrAmbiguousRnaBase2(r3PolymerBond == null ? void 0 : r3PolymerBond.getAnotherMonomer(this));
    }
  };

  // src/core/chem/macromolecules/RNABase.ts
  var RNABase = class extends BaseMonomer {
    getValidSourcePoint() {
      if (this.chosenFirstAttachmentPointForBond) {
        return this.chosenFirstAttachmentPointForBond;
      }
      return this.firstFreeAttachmentPoint;
    }
    getValidTargetPoint() {
      if (this.potentialSecondAttachmentPointForBond) {
        return this.potentialSecondAttachmentPointForBond;
      }
      return this.firstFreeAttachmentPoint;
    }
    get SubChainConstructor() {
      return ChemSubChain;
    }
    get sideConnections() {
      const sideConnections = [];
      this.forEachBond((polymerBond, attachmentPointName) => {
        if (!(polymerBond instanceof MonomerToAtomBond) && (attachmentPointName !== "R1" /* R1 */ || !getSugarFromRnaBase2(this))) {
          sideConnections.push(polymerBond);
        }
      });
      return sideConnections;
    }
  };

  // src/core/chem/macromolecules/Phosphate.ts
  var Phosphate = class _Phosphate extends BaseMonomer {
    constructor(monomerItem, _position) {
      super(monomerItem, _position);
    }
    getValidSourcePoint(secondMonomer) {
      if (!secondMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return _Phosphate.getValidPoint(
        this,
        secondMonomer,
        secondMonomer.potentialSecondAttachmentPointForBond
      );
    }
    getValidTargetPoint(firstMonomer) {
      if (!firstMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return _Phosphate.getValidPoint(
        this,
        firstMonomer,
        firstMonomer.chosenFirstAttachmentPointForBond
      );
    }
    static getValidPoint(self2, otherMonomer, potentialPointOnOther) {
      if (self2.chosenFirstAttachmentPointForBond) {
        return self2.chosenFirstAttachmentPointForBond;
      }
      if (self2.potentialSecondAttachmentPointForBond) {
        return self2.potentialSecondAttachmentPointForBond;
      }
      if (self2.unUsedAttachmentPointsNamesList.length === 1) {
        return self2.unUsedAttachmentPointsNamesList[0];
      }
      if (!isSugarOrAmbiguousSugar(otherMonomer)) {
        return;
      }
      if (potentialPointOnOther) {
        if (potentialPointOnOther === "R2" /* R2 */ && self2.isAttachmentPointExistAndFree("R1" /* R1 */)) {
          return "R1" /* R1 */;
        } else if (potentialPointOnOther !== "R2" /* R2 */ && self2.isAttachmentPointExistAndFree("R2" /* R2 */)) {
          return "R2" /* R2 */;
        } else {
          return;
        }
      }
      if (otherMonomer.isAttachmentPointExistAndFree("R2" /* R2 */) && self2.isAttachmentPointExistAndFree("R1" /* R1 */)) {
        return "R1" /* R1 */;
      }
      if (!otherMonomer.isAttachmentPointExistAndFree("R2" /* R2 */) && self2.isAttachmentPointExistAndFree("R2" /* R2 */)) {
        return "R2" /* R2 */;
      }
      return void 0;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![PhosphateSubChain, RnaSubChain].includes(
        monomerToChain.SubChainConstructor
      );
    }
    get SubChainConstructor() {
      return PhosphateSubChain;
    }
  };

  // src/core/chem/macromolecules/types/editor_shims.ts
  var Coordinates2 = class {
  };
  var resolveMonomerClass = () => {
  };
  var provideEditorInstance2 = () => ({});

  // src/core/chem/macromolecules/UnsplitNucleotide.ts
  var UnsplitNucleotide = class extends BaseMonomer {
    getValidSourcePoint(monomer) {
      return Peptide.prototype.getValidSourcePoint.call(this, monomer);
    }
    getValidTargetPoint(monomer) {
      return Peptide.prototype.getValidTargetPoint.call(this, monomer);
    }
    get SubChainConstructor() {
      return RnaSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![RnaSubChain].includes(monomerToChain.SubChainConstructor);
    }
  };

  // src/core/chem/macromolecules/AmbiguousMonomer.ts
  var DEFAULT_VARIANT_MONOMER_LABEL = "%";
  var MONOMER_CLASS_TO_CONSTRUCTOR = {
    [KetMonomerClass.CHEM]: Chem,
    [KetMonomerClass.AminoAcid]: Peptide,
    [KetMonomerClass.Phosphate]: Phosphate,
    [KetMonomerClass.Sugar]: Sugar,
    [KetMonomerClass.Base]: RNABase,
    [KetMonomerClass.RNA]: UnsplitNucleotide
  };
  var AmbiguousMonomer = class _AmbiguousMonomer extends BaseMonomer {
    constructor(variantMonomerItem, position, generateId = true) {
      var _a;
      const variantMonomerLabel = variantMonomerItem.subtype === KetAmbiguousMonomerTemplateSubType.MIXTURE || ((_a = variantMonomerItem.label) == null ? void 0 : _a.length) > 1 ? DEFAULT_VARIANT_MONOMER_LABEL : variantMonomerItem.label;
      super(
        {
          label: variantMonomerLabel,
          props: {
            MonomerNaturalAnalogCode: "",
            MonomerName: variantMonomerLabel,
            Name: variantMonomerLabel
          },
          attachmentPoints: _AmbiguousMonomer.getAttachmentPoints(
            variantMonomerItem.monomers
          ),
          struct: new Struct(),
          isAmbiguous: true
        },
        position,
        { generateId }
      );
      this.variantMonomerItem = variantMonomerItem;
      __publicField(this, "monomers");
      __publicField(this, "monomerClass");
      __publicField(this, "subtype");
      this.monomers = variantMonomerItem.monomers;
      this.monomerClass = _AmbiguousMonomer.getMonomerClass(
        variantMonomerItem.monomers
      );
      this.subtype = variantMonomerItem.subtype;
    }
    static getMonomerClass(monomers) {
      const monomerClass = resolveMonomerClass(monomers[0].monomerItem);
      const containDifferentMonomerTypes = monomers.some((monomer) => {
        const monomerClassToCompare = resolveMonomerClass(monomer.monomerItem);
        return monomerClass !== monomerClassToCompare;
      });
      if (containDifferentMonomerTypes) {
        return KetMonomerClass.CHEM;
      }
      return monomerClass;
    }
    static getAttachmentPoints(monomers) {
      const monomersAttachmentPoints = monomers.map(
        (monomer) => monomer.listOfAttachmentPoints
      );
      const possibleAttachmentPoints = monomersAttachmentPoints.flat();
      const attachmentPoints = possibleAttachmentPoints.filter(
        (attachmentPointName) => {
          return monomersAttachmentPoints.every(
            (monomerAttachmentPoints) => monomerAttachmentPoints.includes(attachmentPointName)
          );
        }
      );
      return attachmentPoints.map((attachmentPointName) => {
        return {
          label: attachmentPointName,
          leavingGroup: {
            atoms: []
          },
          attachmentAtom: -1
        };
      });
    }
    get monomerCaps() {
      let monomerCaps;
      this.monomers.forEach((monomer) => {
        if (monomer.monomerItem.props.MonomerCaps) {
          if (!monomerCaps) {
            monomerCaps = __spreadValues({}, monomer.monomerItem.props.MonomerCaps);
          } else {
            for (const [attachmentPointName, label] of Object.entries(
              monomer.monomerItem.props.MonomerCaps
            )) {
              if (!monomerCaps[attachmentPointName]) {
                delete monomerCaps[attachmentPointName];
              } else if (monomerCaps[attachmentPointName] !== label) {
                monomerCaps[attachmentPointName] = "";
              }
            }
          }
        }
      });
      return monomerCaps;
    }
    getValidSourcePoint(_secondMonomer) {
      return MONOMER_CLASS_TO_CONSTRUCTOR[this.monomerClass].prototype.getValidSourcePoint.call(this, _secondMonomer);
    }
    getValidTargetPoint(_firstMonomer) {
      return MONOMER_CLASS_TO_CONSTRUCTOR[this.monomerClass].prototype.getValidTargetPoint.call(this, _firstMonomer);
    }
    get SubChainConstructor() {
      const monomerClassToSubchainConstructor = {
        [KetMonomerClass.CHEM]: ChemSubChain,
        [KetMonomerClass.AminoAcid]: PeptideSubChain,
        [KetMonomerClass.RNA]: RnaSubChain,
        [KetMonomerClass.DNA]: RnaSubChain
      };
      return monomerClassToSubchainConstructor[this.monomerClass] || ChemSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return MONOMER_CLASS_TO_CONSTRUCTOR[this.monomerClass].prototype.isMonomerTypeDifferentForChaining.call(this, monomerToChain);
    }
  };

  // domain/helpers/rna.ts
  function getRnaPartLibraryItem(editor, libraryItemLabel, monomerClass, isDna = false) {
    return editor.monomersLibrary.find((libraryItem) => {
      if (isAmbiguousMonomerLibraryItem(libraryItem)) {
        if (monomerClass && AmbiguousMonomer.getMonomerClass(libraryItem.monomers) !== monomerClass) {
          return false;
        }
        if (libraryItem.label !== libraryItemLabel) {
          return false;
        }
        return libraryItem.options.every(
          (option) => isDna ? option.templateId.includes("Thymine" /* THYMINE */) || !option.templateId.includes("Uracil" /* URACIL */) : option.templateId.includes("Uracil" /* URACIL */) || !option.templateId.includes("Thymine" /* THYMINE */)
        );
      }
      return (!monomerClass || libraryItem.props.MonomerClass === monomerClass) && libraryItem.props.MonomerName === libraryItemLabel;
    });
  }

  // src/core/chem/macromolecules/types/render_shims.ts
  var getMonomerSize = () => {
    return { width: 0, height: 0 };
  };

  // src/core/chem/macromolecules/Nucleoside.ts
  var Nucleoside = class _Nucleoside {
    constructor(sugar, rnaBase) {
      this.sugar = sugar;
      this.rnaBase = rnaBase;
      __publicField(this, "monomersCache", []);
      this.monomersCache = [sugar, rnaBase];
    }
    static fromSugar(sugar, needValidation = true) {
      if (needValidation) {
        assert_default(
          isValidNucleoside(sugar),
          "Created nucleoside is not valid. Please check nucleotide parts connections."
        );
        const isNucleotide = isValidNucleotide(sugar);
        assert_default(!isNucleotide, "Created nucleoside is nucleotide.");
      }
      return new _Nucleoside(sugar, getRnaBaseFromSugar(sugar));
    }
    static createOnCanvas(rnaBaseName, position, sugarName = "R" /* SUGAR_RNA */, isAntisense = false) {
      const editor = provideEditorInstance2();
      const isDnaSugar = sugarName === "dR" /* SUGAR_DNA */;
      const rnaBaseLibraryItem = getRnaPartLibraryItem(
        editor,
        rnaBaseName,
        "Base" /* Base */,
        isDnaSugar
      );
      const sugarLibraryItem = getRnaPartLibraryItem(
        editor,
        sugarName,
        "Sugar" /* Sugar */
      );
      assert_default(sugarLibraryItem);
      assert_default(rnaBaseLibraryItem);
      const topLeftItemPosition = position;
      const bottomItemPosition = position.add(
        Coordinates2.canvasToModel(
          new Vec2(0, SnakeLayoutCellWidth + getMonomerSize().height)
        )
      );
      const modelChanges = new Command();
      modelChanges.merge(
        editor.drawingEntitiesManager.addMonomer(
          __spreadProps(__spreadValues({}, sugarLibraryItem), { isAntisense }),
          isAntisense ? bottomItemPosition : topLeftItemPosition
        )
      );
      modelChanges.merge(
        editor.drawingEntitiesManager.addMonomer(
          __spreadProps(__spreadValues({}, rnaBaseLibraryItem), { isAntisense }),
          isAntisense ? topLeftItemPosition : bottomItemPosition
        )
      );
      const sugar = modelChanges.operations[0].monomer;
      const rnaBase = modelChanges.operations[1].monomer;
      modelChanges.merge(
        editor.drawingEntitiesManager.createPolymerBond(
          sugar,
          rnaBase,
          "R3" /* R3 */,
          "R1" /* R1 */
        )
      );
      return { modelChanges, node: _Nucleoside.fromSugar(sugar, false) };
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return this.sugar.isMonomerTypeDifferentForChaining(monomerToChain);
    }
    get SubChainConstructor() {
      return this.sugar.SubChainConstructor;
    }
    get monomer() {
      return this.sugar;
    }
    get monomers() {
      return this.monomersCache;
    }
    get firstMonomerInNode() {
      return this.sugar;
    }
    get lastMonomerInNode() {
      return this.sugar;
    }
    get renderer() {
      return this.monomer.renderer;
    }
    get modified() {
      const isNotLastNode = !!getNextMonomerInChain(this.sugar);
      return this.rnaBase.isModification || this.sugar.isModification || isNotLastNode;
    }
  };

  // src/core/chem/macromolecules/Nucleotide.ts
  var Nucleotide = class _Nucleotide {
    constructor(sugar, rnaBase, phosphate) {
      this.sugar = sugar;
      this.rnaBase = rnaBase;
      this.phosphate = phosphate;
      __publicField(this, "monomersCache", []);
      this.monomersCache = [sugar, rnaBase, phosphate];
    }
    toString() {
      return `sugar: ${this.sugar.constructor.name}, rnaBase: ${this.rnaBase.constructor.name}, phosphate: ${this.phosphate.constructor.name}`;
    }
    static fromSugar(sugar, needValidation = true) {
      if (needValidation) {
        assert_default(
          isValidNucleotide(sugar),
          "Nucleotide is not valid. Please check nucleotide parts connections."
        );
        const isNucleoside = isValidNucleoside(sugar);
        assert_default(
          !isNucleoside,
          "Nucleotide is nucleoside because it is a last sugar+base of rna chain"
        );
      }
      return new _Nucleotide(
        sugar,
        getRnaBaseFromSugar(sugar),
        getPhosphateFromSugar(sugar)
      );
    }
    static createOnCanvas(rnaBaseName, position, sugarName = "R" /* SUGAR_RNA */) {
      const editor = provideEditorInstance2();
      const isDnaSugar = sugarName === "dR" /* SUGAR_DNA */;
      const rnaBaseLibraryItem = getRnaPartLibraryItem(
        editor,
        rnaBaseName,
        "Base" /* Base */,
        isDnaSugar
      );
      const phosphateLibraryItem = getRnaPartLibraryItem(
        editor,
        "P" /* PHOSPHATE */
      );
      const sugarLibraryItem = getRnaPartLibraryItem(
        editor,
        sugarName,
        "Sugar" /* Sugar */
      );
      assert_default(sugarLibraryItem);
      assert_default(rnaBaseLibraryItem);
      assert_default(phosphateLibraryItem);
      const topLeftItemPosition = position;
      const bottomItemPosition = position.add(
        Coordinates2.canvasToModel(
          new Vec2(0, SnakeLayoutCellWidth + getMonomerSize().height)
        )
      );
      const { command: modelChanges, monomers } = editor.drawingEntitiesManager.addRnaPreset({
        sugar: sugarLibraryItem,
        sugarPosition: topLeftItemPosition,
        rnaBase: rnaBaseLibraryItem,
        rnaBasePosition: bottomItemPosition,
        phosphate: phosphateLibraryItem,
        phosphatePosition: topLeftItemPosition.add(
          Coordinates2.canvasToModel(new Vec2(SnakeLayoutCellWidth, 0))
        )
      });
      const sugar = monomers.find((monomer) => monomer instanceof Sugar);
      return { modelChanges, node: _Nucleotide.fromSugar(sugar, false) };
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return this.sugar.isMonomerTypeDifferentForChaining(monomerToChain);
    }
    get SubChainConstructor() {
      return this.sugar.SubChainConstructor;
    }
    get monomer() {
      return this.firstMonomerInNode;
    }
    get monomers() {
      return this.monomersCache;
    }
    get firstMonomerInNode() {
      return this.isFiveEndPhosphate ? this.phosphate : this.sugar;
    }
    get lastMonomerInNode() {
      return this.isFiveEndPhosphate ? this.sugar : this.phosphate;
    }
    get renderer() {
      return this.monomer.renderer;
    }
    get modified() {
      return this.rnaBase.isModification || this.sugar.isModification || this.phosphate.isModification;
    }
    get isFiveEndPhosphate() {
      var _a, _b;
      return ((_a = this.sugar.attachmentPointsToBonds.R1) == null ? void 0 : _a.getAnotherEntity(this.sugar)) === this.phosphate && ((_b = this.phosphate.attachmentPointsToBonds.R2) == null ? void 0 : _b.getAnotherEntity(
        this.phosphate
      )) === this.sugar;
    }
  };

  // src/core/chem/macromolecules/UnresolvedMonomer.ts
  var UnresolvedMonomer = class extends BaseMonomer {
    getValidSourcePoint(secondMonomer) {
      if (!secondMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return Peptide.prototype.getValidSourcePoint.call(this, secondMonomer);
    }
    getValidTargetPoint(firstMonomer) {
      if (!firstMonomer) {
        return this.firstFreeAttachmentPoint;
      }
      return Peptide.prototype.getValidTargetPoint.call(this, firstMonomer);
    }
    get SubChainConstructor() {
      return ChemSubChain;
    }
    isMonomerTypeDifferentForChaining(monomerToChain) {
      return ![PeptideSubChain, ChemSubChain].includes(
        monomerToChain.SubChainConstructor
      );
    }
  };

  // domain/entities/MonomerSequenceNode.ts
  var MonomerSequenceNode = class {
    constructor(monomer) {
      this.monomer = monomer;
      __publicField(this, "monomersCache", []);
      this.monomersCache = [monomer];
    }
    get SubChainConstructor() {
      return this.monomer.SubChainConstructor;
    }
    get firstMonomerInNode() {
      return this.monomer;
    }
    get lastMonomerInNode() {
      return this.monomer;
    }
    get monomers() {
      return this.monomersCache;
    }
    get renderer() {
      return this.monomer.renderer;
    }
    get modified() {
      return this.monomer.isModification;
    }
  };

  // src/core/chem/macromolecules/monomer-chains/EmptySubChain.ts
  var EmptySubChain = class extends BaseSubChain {
  };

  // domain/entities/EmptyMonomer.ts
  function getEmptyMonomerItem() {
    return {
      label: "",
      struct: new Struct(),
      props: {
        MonomerNaturalAnalogCode: "",
        MonomerName: "",
        Name: ""
      }
    };
  }
  var EmptyMonomer = class extends BaseMonomer {
    constructor() {
      super(getEmptyMonomerItem(), void 0, { generateId: false });
    }
    getValidSourcePoint() {
      return void 0;
    }
    getValidTargetPoint() {
      return void 0;
    }
    get SubChainConstructor() {
      return EmptySubChain;
    }
    isMonomerTypeDifferentForChaining() {
      return true;
    }
  };

  // domain/entities/EmptySequenceNode.ts
  var EmptySequenceNode = class {
    constructor() {
      __publicField(this, "renderer");
      __publicField(this, "monomer", new EmptyMonomer());
      // when iterating over large amount of nodes, this saves a lot of GC time
      __publicField(this, "monomersCache", [this.monomer]);
    }
    get SubChainConstructor() {
      return EmptySubChain;
    }
    get firstMonomerInNode() {
      return this.monomer;
    }
    get lastMonomerInNode() {
      return this.monomer;
    }
    get hovered() {
      return false;
    }
    get selected() {
      return false;
    }
    get monomerItem() {
      return { props: { MonomerNaturalAnalogCode: null } };
    }
    get monomers() {
      return this.monomersCache;
    }
    setRenderer(renderer) {
      this.renderer = renderer;
      this.monomer.setRenderer(renderer);
    }
    get modified() {
      return false;
    }
  };

  // domain/entities/LinkerSequenceNode.ts
  var LinkerSequenceNode = class _LinkerSequenceNode {
    constructor(monomer) {
      this.monomer = monomer;
    }
    get SubChainConstructor() {
      return this.monomer.SubChainConstructor;
    }
    get firstMonomerInNode() {
      return this.monomer;
    }
    get lastMonomerInNode() {
      return this.monomers[this.monomers.length - 1];
    }
    get monomers() {
      const monomers = [this.firstMonomerInNode];
      const firstMonomer = this.firstMonomerInNode;
      let nextMonomer = getNextMonomerInChain(this.firstMonomerInNode);
      while (_LinkerSequenceNode.isValidPartForLinker(nextMonomer)) {
        monomers.push(nextMonomer);
        nextMonomer = getNextMonomerInChain(nextMonomer, firstMonomer);
      }
      return monomers;
    }
    get renderer() {
      return this.monomer.renderer;
    }
    get modified() {
      return false;
    }
    static isValidPartForLinker(monomer) {
      return monomer instanceof Chem || monomer instanceof Phosphate || monomer instanceof RNABase || monomer instanceof Sugar && !isValidNucleotide(monomer) && !isValidNucleoside(monomer) || monomer instanceof AmbiguousMonomer && (monomer.monomerClass === "CHEM" /* CHEM */ || monomer.monomerClass === "Sugar" /* Sugar */ || monomer.monomerClass === "Phosphate" /* Phosphate */ || monomer.monomerClass === "Base" /* Base */);
    }
    static isPartOfLinker(monomer) {
      if (!monomer) {
        return false;
      }
      const previousMonomerInChain = getPreviousMonomerInChain(monomer);
      const nextMonomerInChain = getNextMonomerInChain(monomer);
      return _LinkerSequenceNode.isValidPartForLinker(monomer) && (_LinkerSequenceNode.isValidPartForLinker(previousMonomerInChain) || _LinkerSequenceNode.isValidPartForLinker(nextMonomerInChain));
    }
  };

  // domain/entities/AmbiguousMonomerSequenceNode.ts
  var AmbiguousMonomerSequenceNode = class {
    constructor(monomer) {
      this.monomer = monomer;
    }
    get SubChainConstructor() {
      return this.monomer.SubChainConstructor;
    }
    get firstMonomerInNode() {
      return this.monomer;
    }
    get lastMonomerInNode() {
      return this.monomer;
    }
    get monomers() {
      return [this.monomer];
    }
    get renderer() {
      return this.monomer.renderer;
    }
    get modified() {
      return this.monomer.isModification;
    }
  };

  // src/core/chem/macromolecules/monomer-chains/Chain.ts
  var id = 0;
  var Chain = class _Chain {
    constructor(firstMonomer, isCyclic) {
      __publicField(this, "subChains", []);
      __publicField(this, "firstMonomer");
      __publicField(this, "isCyclic", false);
      __publicField(this, "id");
      __publicField(this, "nodesChanged", true);
      __publicField(this, "nodesCache", []);
      __publicField(this, "monomersCache", []);
      __publicField(this, "bondsCache", []);
      this.id = id++;
      if (firstMonomer) {
        this.firstMonomer = firstMonomer;
        this.fillSubChains(firstMonomer);
      }
      if (isCyclic) {
        this.isCyclic = isCyclic;
      }
    }
    recalculateNodes() {
      if (this.nodesChanged || this.subChains.some((subChain) => subChain.modified)) {
        this.nodesCache = this.subChains.flatMap((subChain) => subChain.nodes);
        this.monomersCache = this.nodesCache.flatMap((node) => node.monomers);
        this.bondsCache = this.subChains.flatMap((subChain) => subChain.bonds);
        this.nodesChanged = false;
        this.subChains.forEach((subChain) => {
          subChain.modified = false;
        });
      }
    }
    createSubChainIfNeed(monomer) {
      var _a;
      const needCreateNewSubchain = !((_a = this.lastNode) == null ? void 0 : _a.monomer) || monomer.isMonomerTypeDifferentForChaining(this.lastNode.monomer);
      if (needCreateNewSubchain) {
        this.subChains.push(new monomer.SubChainConstructor());
      }
    }
    tryAddAsNucleosideOrNucleotide(sugar) {
      if (isValidNucleoside(sugar, this.firstMonomer)) {
        this.lastSubChain.add(Nucleoside.fromSugar(sugar, false));
        return true;
      }
      if (isValidNucleotide(sugar, this.firstMonomer)) {
        this.lastSubChain.add(Nucleotide.fromSugar(sugar, false));
        return true;
      }
      return false;
    }
    addAmbiguousMonomer(monomer) {
      if (monomer.monomerClass === "Sugar" /* Sugar */) {
        if (this.tryAddAsNucleosideOrNucleotide(monomer)) {
          return;
        }
      }
      if (LinkerSequenceNode.isPartOfLinker(monomer)) {
        this.lastSubChain.add(new LinkerSequenceNode(monomer));
      } else {
        this.lastSubChain.add(new AmbiguousMonomerSequenceNode(monomer));
      }
    }
    add(monomer) {
      this.nodesChanged = true;
      this.createSubChainIfNeed(monomer);
      if (monomer instanceof Peptide || monomer instanceof UnsplitNucleotide || monomer instanceof UnresolvedMonomer) {
        this.lastSubChain.add(new MonomerSequenceNode(monomer));
        return;
      }
      if (monomer instanceof AmbiguousMonomer) {
        this.addAmbiguousMonomer(monomer);
        return;
      }
      if (monomer instanceof Sugar) {
        if (this.tryAddAsNucleosideOrNucleotide(monomer)) {
          return;
        }
      }
      const nextMonomer = getNextMonomerInChain(monomer);
      const isNextMonomerNucleosideOrNucleotideOrPeptide = () => {
        const isNucleosideOrNucleotide = nextMonomer instanceof Sugar && (isValidNucleotide(nextMonomer) || isValidNucleoside(nextMonomer));
        return isNucleosideOrNucleotide || nextMonomer instanceof Peptide;
      };
      if (monomer instanceof Phosphate && (!this.lastNode || this.lastNode instanceof Nucleoside || this.lastNode.lastMonomerInNode instanceof UnsplitNucleotide) && (!nextMonomer || isNextMonomerNucleosideOrNucleotideOrPeptide())) {
        this.lastSubChain.add(new MonomerSequenceNode(monomer));
        return;
      }
      this.lastSubChain.add(new LinkerSequenceNode(monomer));
    }
    addNode(node) {
      this.createSubChainIfNeed(node.monomer);
      this.lastSubChain.add(node);
      this.nodesChanged = true;
      return this;
    }
    fillSubChains(monomer) {
      var _a;
      if (!monomer) return;
      this.add(monomer);
      this.fillSubChains(
        getNextMonomerInChain(
          (_a = this.lastNode) == null ? void 0 : _a.lastMonomerInNode,
          this.firstMonomer
        )
      );
    }
    get lastSubChain() {
      return this.subChains[this.subChains.length - 1];
    }
    get nodes() {
      this.recalculateNodes();
      return this.nodesCache;
    }
    get lastNode() {
      var _a;
      return (_a = this.lastSubChain) == null ? void 0 : _a.lastNode;
    }
    get lastNonEmptyNode() {
      if (this.lastNode instanceof EmptySequenceNode) {
        const nodes = this.nodes;
        return nodes[nodes.length - 2];
      } else {
        return this.lastNode;
      }
    }
    get firstSubChain() {
      return this.subChains[0];
    }
    get firstNode() {
      var _a;
      return (_a = this.firstSubChain) == null ? void 0 : _a.firstNode;
    }
    get length() {
      let length = 0;
      this.subChains.forEach((subChain) => {
        length += subChain.length;
      });
      return length;
    }
    get isEmpty() {
      return this.subChains.length === 1 && this.subChains[0].nodes.length === 1 && this.subChains[0].nodes[0] instanceof EmptySequenceNode;
    }
    get isAntisense() {
      return this.nodes.some((node) => node.monomer.monomerItem.isAntisense);
    }
    forEachNode(callback) {
      let nodeIndex = 0;
      this.subChains.forEach((subChain) => {
        subChain.nodes.forEach((node) => {
          callback({ node, subChain, nodeIndex });
          nodeIndex++;
        });
      });
    }
    forEachNodeReversed(callback) {
      let nodeIndex = this.length - 1;
      for (let i = this.subChains.length - 1; i >= 0; i--) {
        for (let j = this.subChains[i].nodes.length - 1; j >= 0; j--) {
          callback({
            node: this.subChains[i].nodes[j],
            subChain: this.subChains[i],
            nodeIndex
          });
          nodeIndex--;
        }
      }
    }
    static createChainWithEmptyNode() {
      const emptyChain = new _Chain();
      const emptySequenceNode = new EmptySequenceNode();
      const emptySubChain = new EmptySubChain();
      emptySubChain.add(emptySequenceNode);
      emptyChain.subChains.push(emptySubChain);
      return { emptyChain, emptySubChain, emptySequenceNode };
    }
    get isNewSequenceChain() {
      return this.length === 1 && this.firstNode instanceof EmptySequenceNode;
    }
    get monomers() {
      this.recalculateNodes();
      return this.monomersCache;
    }
    // TODO: Currently the only place where bonds are pushed is in SequenceModeRenderer thus it doesn't provide correct data. Collect all bonds in `fromMonomers` method
    get bonds() {
      this.recalculateNodes();
      return this.bondsCache;
    }
  };

  // src/core/chem/macromolecules/monomer-chains/ChainsCollection.ts
  var import_monomers18 = __toESM(require_monomers());
  var ChainsCollection = class _ChainsCollection {
    constructor() {
      __publicField(this, "chains", []);
    }
    get monomerToChain() {
      const monomerToChain = /* @__PURE__ */ new Map();
      this.chains.forEach((chain) => {
        chain.forEachNode(({ node }) => {
          node.monomers.forEach((monomer) => {
            monomerToChain.set(monomer, chain);
          });
        });
      });
      return monomerToChain;
    }
    get monomerToNode() {
      const monomerToNode = /* @__PURE__ */ new Map();
      this.forEachNode(({ node }) => {
        node.monomers.forEach((monomer) => {
          monomerToNode.set(monomer, node);
        });
      });
      return monomerToNode;
    }
    rearrange() {
      this.chains.sort((chain1, chain2) => {
        var _a, _b, _c, _d;
        const X_COORDINATE_REDUCTION_FACTOR = 0.01;
        if (((_a = chain2.firstNode) == null ? void 0 : _a.monomer.position.x) * X_COORDINATE_REDUCTION_FACTOR + ((_b = chain2.firstNode) == null ? void 0 : _b.monomer.position.y) > ((_c = chain1.firstNode) == null ? void 0 : _c.monomer.position.x) * X_COORDINATE_REDUCTION_FACTOR + ((_d = chain1.firstNode) == null ? void 0 : _d.monomer.position.y)) {
          return -1;
        } else {
          return 1;
        }
      });
      const reorderedChains = /* @__PURE__ */ new Set();
      const monomerToChain = this.monomerToChain;
      this.chains.forEach((chain) => {
        reorderedChains.add(chain);
        chain.forEachNode(({ node }) => {
          node.monomers.forEach((monomer) => {
            const sideConnections = monomer.sideConnections;
            if (sideConnections.length) {
              sideConnections.forEach((sideConnection) => {
                const anotherMonomer = sideConnection.getAnotherMonomer(monomer);
                const anotherChain = anotherMonomer && monomerToChain.get(anotherMonomer);
                if (anotherChain && !reorderedChains.has(anotherChain)) {
                  reorderedChains.add(anotherChain);
                }
              });
            }
          });
        });
      });
      this.chains = [...reorderedChains.values()];
      this.reorderChainsPutSenseChainOrderInAccordanceAntisenseConnection();
    }
    add(chain) {
      this.chains.push(chain);
      return this;
    }
    static fromMonomers(monomers) {
      const chainsCollection = new _ChainsCollection();
      const filteredMonomers = monomers.filter(
        (monomer) => !monomer.monomerItem.props.isMicromoleculeFragment || (0, import_monomers18.isMonomerSgroupWithAttachmentPoints)(monomer)
      );
      if (filteredMonomers.length === 0) {
        return chainsCollection;
      }
      const [firstMonomersInRegularChains, firstMonomersInCycledChains] = this.getFirstMonomersInChains(filteredMonomers);
      firstMonomersInRegularChains.forEach((monomer) => {
        chainsCollection.add(new Chain(monomer));
      });
      firstMonomersInCycledChains.forEach((monomer) => {
        chainsCollection.add(new Chain(monomer, !!1 /* CYCLED */));
      });
      const firstMonomersInMiddleOfChains = this.getFirstMonomersInMiddleOfChains(filteredMonomers);
      if (firstMonomersInMiddleOfChains.length) {
        firstMonomersInMiddleOfChains.forEach(
          (firstMonomerInMiddleOfChain) => {
            chainsCollection.add(new Chain(firstMonomerInMiddleOfChain));
          }
        );
      }
      return chainsCollection;
    }
    static getFirstMonomersInChains(monomers, MonomerTypes = [
      Peptide,
      Chem,
      Phosphate,
      Sugar,
      RNABase,
      UnresolvedMonomer,
      UnsplitNucleotide,
      AmbiguousMonomer
    ]) {
      const monomersList = monomers.filter(
        (monomer) => MonomerTypes.some((MonomerType) => monomer instanceof MonomerType)
      );
      const firstMonomersInChains = [];
      const firstMonomersInRegularChains = this.getFirstMonomersInRegularChains(monomersList);
      const firstMonomersInCycledChains = this.getFirstMonomersInCycledChains(monomersList);
      firstMonomersInChains.push(
        firstMonomersInRegularChains,
        firstMonomersInCycledChains
      );
      return firstMonomersInChains;
    }
    static getFirstMonomersInMiddleOfChains(monomers) {
      const initialMonomersSet = new Set(monomers);
      const handledMonomers = /* @__PURE__ */ new Set();
      const firstMonomersInMiddleOfChains = [];
      monomers.forEach((monomer) => {
        if (handledMonomers.has(monomer)) {
          return;
        }
        handledMonomers.add(monomer);
        let previousMonomerInChain = getPreviousMonomerInChain(monomer);
        while (previousMonomerInChain && !handledMonomers.has(previousMonomerInChain) && !initialMonomersSet.has(previousMonomerInChain)) {
          const previousMonomer = getPreviousMonomerInChain(
            previousMonomerInChain
          );
          handledMonomers.add(previousMonomerInChain);
          if (!previousMonomer) {
            firstMonomersInMiddleOfChains.push(previousMonomerInChain);
          } else {
            previousMonomerInChain = previousMonomer;
          }
        }
      });
      return firstMonomersInMiddleOfChains;
    }
    get firstNode() {
      var _a, _b;
      return (_b = (_a = this.chains[0]) == null ? void 0 : _a.subChains[0]) == null ? void 0 : _b.nodes[0];
    }
    static getFirstMonomersInRegularChains(monomersList) {
      const firstMonomersInRegularChains = monomersList.filter((monomer) => {
        const R1PolymerBond = monomer.attachmentPointsToBonds.R1;
        if (R1PolymerBond instanceof MonomerToAtomBond) {
          return true;
        }
        const isFirstMonomerWithR2R1connection = !R1PolymerBond || R1PolymerBond.isSideChainConnection;
        const R1ConnectedMonomer = R1PolymerBond == null ? void 0 : R1PolymerBond.getAnotherMonomer(monomer);
        const isRnaBaseConnectedToSugar = isRnaBaseOrAmbiguousRnaBase2(monomer) && R1ConnectedMonomer instanceof Sugar && getRnaBaseFromSugar(R1ConnectedMonomer) === monomer;
        return (isFirstMonomerWithR2R1connection || isMonomerConnectedToR2RnaBase2(monomer)) && !isRnaBaseConnectedToSugar;
      });
      return firstMonomersInRegularChains;
    }
    static getFirstMonomersInCycledChains(monomersList) {
      const handledMonomers = /* @__PURE__ */ new Set();
      const cyclicChains = [];
      monomersList.forEach((monomer) => {
        if (handledMonomers.has(monomer)) {
          return;
        }
        const monomersInSameChain = /* @__PURE__ */ new Set();
        monomersInSameChain.add(monomer);
        handledMonomers.add(monomer);
        let nextMonomerInChain = getNextMonomerInChain(monomer);
        while (nextMonomerInChain && !handledMonomers.has(nextMonomerInChain)) {
          monomersInSameChain.add(nextMonomerInChain);
          handledMonomers.add(nextMonomerInChain);
          nextMonomerInChain = getNextMonomerInChain(nextMonomerInChain);
        }
        if (monomer === nextMonomerInChain) {
          cyclicChains.push(Array.from(monomersInSameChain));
        }
      });
      const firstMonomersOfCycledChainsSet = cyclicChains.map(
        (cyclicChain) => this.getMonomerWithLowerCoordsFromMonomerList(cyclicChain)
      );
      return firstMonomersOfCycledChainsSet;
    }
    static getMonomerWithLowerCoordsFromMonomerList(monomerList) {
      const monomerListShallowCopy = monomerList.slice();
      monomerListShallowCopy.sort((monomer1, monomer2) => {
        if (monomer2.position.x + monomer2.position.y > monomer1.position.x + monomer1.position.y) {
          return -1;
        } else {
          return 1;
        }
      });
      const monomerWithLowerCoords = monomerListShallowCopy[0];
      return monomerWithLowerCoords;
    }
    get lastNode() {
      return this.chains[0].lastSubChain.lastNode;
    }
    get length() {
      return this.chains.reduce((length, chain) => length + chain.length, 0);
    }
    forEachNode(forEachCallback) {
      let nodeIndexOverall = 0;
      this.chains.forEach((chain, chainIndex) => {
        chain.subChains.forEach((subChain, subChainIndex) => {
          subChain.nodes.forEach((node, nodeIndex) => {
            forEachCallback({
              chainIndex,
              subChainIndex,
              nodeIndex,
              nodeIndexOverall,
              node,
              subChain,
              chain
            });
            nodeIndexOverall++;
          });
        });
      });
    }
    getFirstComplimentaryMonomer(monomer) {
      const hydrogenBond = monomer.hydrogenBonds[0];
      if (hydrogenBond) {
        return {
          monomer,
          complimentaryMonomer: hydrogenBond.getAnotherMonomer(monomer)
        };
      }
      return void 0;
    }
    findCycledComplimentaryChains(chain, startChain, previousChain, visitedChains = /* @__PURE__ */ new Set()) {
      if (visitedChains.has(chain)) {
        return [];
      }
      visitedChains.add(chain);
      const complimentaryChainsWithData = this.getComplimentaryChainsWithData(chain);
      if (complimentaryChainsWithData.length === 0) {
        return [];
      }
      const complimentaryChainGoesToStartChain = complimentaryChainsWithData.find(
        ({ complimentaryChain }) => complimentaryChain !== previousChain && complimentaryChain === startChain
      );
      if (complimentaryChainGoesToStartChain) {
        return [chain];
      } else {
        return complimentaryChainsWithData.reduce(
          (acc, { complimentaryChain }) => {
            if (complimentaryChain === startChain || complimentaryChain === previousChain) {
              return acc;
            }
            return [
              ...acc,
              ...this.findCycledComplimentaryChains(
                complimentaryChain,
                startChain,
                chain,
                visitedChains
              )
            ];
          },
          []
        );
      }
    }
    getComplimentaryChainIfNucleotide(node, monomerToChain, monomerToNode) {
      let complimentaryChain;
      let complimentaryNode;
      for (const monomerToCheck of node.monomers) {
        const { monomer, complimentaryMonomer } = this.getFirstComplimentaryMonomer(monomerToCheck) || {};
        const complimentaryNodeOrUndefined = complimentaryMonomer && monomerToNode.get(complimentaryMonomer);
        const complimentaryChainOrUndefined = complimentaryMonomer && monomerToChain.get(complimentaryMonomer);
        if (!complimentaryNodeOrUndefined || !complimentaryChainOrUndefined) {
          continue;
        }
        const isRnaMonomer = isRnaBaseApplicableForAntisense(monomer);
        const isRnaComplimentaryMonomer = isRnaBaseApplicableForAntisense(complimentaryMonomer);
        if (!isRnaMonomer || !isRnaComplimentaryMonomer) {
          continue;
        }
        return {
          complimentaryChain: complimentaryChainOrUndefined,
          complimentaryNode: complimentaryNodeOrUndefined
        };
      }
      return { complimentaryChain, complimentaryNode };
    }
    reorderChainsPutSenseChainOrderInAccordanceAntisenseConnection() {
      const handledChain = /* @__PURE__ */ new Set();
      const monomerToChain = this.monomerToChain;
      const monomerToNode = this.monomerToNode;
      const reorderedSenseForSequentialAntisenseChains = new Array(
        this.chains.length
      );
      this.chains.forEach((chain) => {
        if (!handledChain.has(chain)) {
          reorderedSenseForSequentialAntisenseChains[handledChain.size] = chain;
          handledChain.add(chain);
        }
        if (chain.isAntisense) {
          return;
        }
        chain.forEachNode(({ node: sNode }) => {
          var _a;
          const {
            complimentaryChain: antisenseChain,
            complimentaryNode: antisenseNode
          } = (_a = this.getComplimentaryChainIfNucleotide(
            sNode,
            monomerToChain,
            monomerToNode
          )) != null ? _a : {};
          if (!antisenseChain) {
            return;
          }
          let isFindCur = false;
          antisenseChain.forEachNode(({ node: aNode }) => {
            var _a2;
            if (aNode === antisenseNode) {
              isFindCur = true;
            }
            if (!isFindCur) {
              const { complimentaryChain: anotherSenseChain } = (_a2 = this.getComplimentaryChainIfNucleotide(
                aNode,
                monomerToChain,
                monomerToNode
              )) != null ? _a2 : {};
              if (anotherSenseChain && !handledChain.has(anotherSenseChain)) {
                const curChainIdx = reorderedSenseForSequentialAntisenseChains.findIndex(
                  (v) => v === chain
                );
                let last = anotherSenseChain;
                for (let i = curChainIdx; i < reorderedSenseForSequentialAntisenseChains.length; i++) {
                  const tmp = reorderedSenseForSequentialAntisenseChains[i];
                  reorderedSenseForSequentialAntisenseChains[i] = last;
                  last = tmp;
                }
                handledChain.add(anotherSenseChain);
              }
            }
          });
        });
      });
      this.chains = [...reorderedSenseForSequentialAntisenseChains];
    }
    // for example
    // 1 x x x
    //   |
    // 2 x x
    //     |
    // 3 x x x
    // 4 x x
    //     |
    // 5 x x
    // in the picture we have 5 chains, if we pass number 1 it return 1, 2 and 3, if pass 5, return 4, 5
    getAllChainsWithConnectionInBlock(c) {
      const chains = [{ group: 0, chain: c }];
      const cycledComplimentaryChains = new Set(
        this.findCycledComplimentaryChains(c, c)
      );
      const res = [{ group: 0, chain: c }];
      const handledChains = /* @__PURE__ */ new Set([c]);
      const monomerToChain = this.monomerToChain;
      const monomerToNode = this.monomerToNode;
      while (chains.length) {
        const { group, chain } = chains.pop();
        const chainNodes = chain.nodes;
        chain.forEachNode(({ node, nodeIndex }) => {
          var _a, _b;
          const { complimentaryChain, complimentaryNode } = (_a = this.getComplimentaryChainIfNucleotide(
            node,
            monomerToChain,
            monomerToNode
          )) != null ? _a : {};
          if (!complimentaryChain || !complimentaryNode || handledChains.has(complimentaryChain) || cycledComplimentaryChains.has(complimentaryChain)) {
            return;
          }
          const complimentaryChainNodes = complimentaryChain.nodes;
          const firstComplimentaryNodeIndex = complimentaryChainNodes.indexOf(complimentaryNode);
          let hasIntersection = false;
          for (let i = firstComplimentaryNodeIndex; i < complimentaryChainNodes.length; i++) {
            const potentialNextComplimentaryNode = complimentaryChainNodes[i];
            const {
              complimentaryNode: nextComplimentaryNode,
              complimentaryChain: nextComplimentaryNodeChain
            } = (_b = this.getComplimentaryChainIfNucleotide(
              potentialNextComplimentaryNode,
              monomerToChain,
              monomerToNode
            )) != null ? _b : {};
            if (nextComplimentaryNode && nextComplimentaryNodeChain === chain && chainNodes.indexOf(nextComplimentaryNode) > nodeIndex) {
              hasIntersection = true;
              break;
            }
          }
          handledChains.add(complimentaryChain);
          if (hasIntersection) {
            return;
          }
          const el = { chain: complimentaryChain, group: Number(!group) };
          chains.push(el);
          res.push(el);
        });
      }
      return res;
    }
    getComplimentaryChainsWithData(chain) {
      const complimentaryChainsWithData = [];
      const handledChains = /* @__PURE__ */ new Set();
      const monomerToNode = this.monomerToNode;
      const monomerToChain = this.monomerToChain;
      chain.forEachNode(({ node, nodeIndex }) => {
        node.monomers.forEach((monomer) => {
          const { complimentaryMonomer } = this.getFirstComplimentaryMonomer(monomer) || {};
          const complimentaryNode = complimentaryMonomer && monomerToNode.get(complimentaryMonomer);
          const complimentaryChain = complimentaryMonomer && monomerToChain.get(complimentaryMonomer);
          if (!complimentaryNode || !complimentaryChain || handledChains.has(complimentaryChain)) {
            return;
          }
          handledChains.add(complimentaryChain);
          complimentaryChainsWithData.push({
            complimentaryChain,
            chain,
            firstConnectedNode: node,
            firstConnectedComplimentaryNode: complimentaryNode,
            chainIdxConnection: nodeIndex
          });
        });
      });
      return complimentaryChainsWithData;
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_monomer = __toESM(require_monomer());
  var import_drawingEntity = __toESM(require_drawingEntity());
  var import_polymerBond = __toESM(require_polymerBond());
  var import_monomerFactory = __toESM(require_monomerFactory());
  var import_coordinates3 = __toESM(require_coordinates());
  var import_SequenceRenderer = __toESM(require_SequenceRenderer());
  var import_types15 = __toESM(require_types());
  var import_editorSettings = __toESM(require_editorSettings());

  // domain/entities/canvas-matrix/Matrix.ts
  var Matrix = class {
    constructor() {
      __publicField(this, "matrix");
      this.matrix = [];
    }
    get(x, y) {
      if (!this.matrix[x]) {
        return void 0;
      }
      return this.matrix[x][y];
    }
    getRow(x) {
      return this.matrix[x];
    }
    set(x, y, value) {
      if (!this.matrix[x]) {
        this.matrix[x] = [];
      }
      this.matrix[x][y] = value;
    }
    get height() {
      return this.matrix.length;
    }
    get width() {
      return this.matrix.reduce((max, row) => Math.max(max, row.length), 0);
    }
    forEach(callback) {
      var _a;
      for (let x = 0; x < this.matrix.length; x++) {
        for (let y = 0; y < ((_a = this.matrix[x]) == null ? void 0 : _a.length); y++) {
          const value = this.matrix[x][y];
          if (value) {
            callback(value, x, y);
          }
        }
      }
    }
    forEachRightToLeft(callback) {
      var _a;
      for (let x = this.matrix.length - 1; x >= 0; x--) {
        for (let y = ((_a = this.matrix[x]) == null ? void 0 : _a.length) - 1; y >= 0; y--) {
          const value = this.matrix[x][y];
          if (value) {
            callback(value, x, y);
          }
        }
      }
    }
    forEachBottomToTop(callback) {
      var _a;
      for (let y = ((_a = this.matrix[0]) == null ? void 0 : _a.length) - 1; y >= 0; y--) {
        for (let x = this.matrix.length - 1; x >= 0; x--) {
          const value = this.matrix[x][y];
          if (value) {
            callback(value, x, y);
          }
        }
      }
    }
  };

  // domain/entities/canvas-matrix/Connection.ts
  var Connection = class {
    constructor(connectedNode, direction, isVertical, polymerBond, xOffset, yOffset) {
      this.connectedNode = connectedNode;
      this.direction = direction;
      this.isVertical = isVertical;
      this.polymerBond = polymerBond;
      this.xOffset = xOffset;
      this.yOffset = yOffset;
    }
  };

  // domain/entities/canvas-matrix/Cell.ts
  var Cell = class {
    constructor(node, connections, x, y, monomer) {
      this.node = node;
      this.connections = connections;
      this.x = x;
      this.y = y;
      this.monomer = monomer;
    }
  };

  // domain/entities/canvas-matrix/CanvasMatrix.ts
  var import_lodash7 = __toESM(require_lodash2());
  var CanvasMatrix = class {
    constructor(chainsCollection, matrixConfig = {
      initialMatrix: new Matrix()
    }) {
      this.chainsCollection = chainsCollection;
      this.matrixConfig = matrixConfig;
      __publicField(this, "matrix");
      __publicField(this, "initialMatrixWidth");
      __publicField(this, "monomerToCell", /* @__PURE__ */ new Map());
      __publicField(this, "polymerBondToCells", /* @__PURE__ */ new Map());
      __publicField(this, "polymerBondToConnections", /* @__PURE__ */ new Map());
      this.matrix = new Matrix();
      this.initialMatrixWidth = this.matrixConfig.initialMatrix.width;
      this.fillCells();
    }
    fillConnectionsOffset(direction, increaseOffset = (connection, increaseValue) => {
      if ((0, import_lodash7.isNumber)(increaseValue)) {
        connection.xOffset = increaseValue;
      } else {
        connection.xOffset++;
      }
    }, getOffset = (connection) => connection.xOffset) {
      const currentConnections = /* @__PURE__ */ new Map();
      let iterationMethod;
      if (direction === 180) {
        iterationMethod = this.matrix.forEach.bind(this.matrix);
      } else if (direction === 0) {
        iterationMethod = this.matrix.forEachRightToLeft.bind(this.matrix);
      } else {
        iterationMethod = this.matrix.forEachBottomToTop.bind(this.matrix);
      }
      iterationMethod((cell) => {
        const biggestOffsetInCell = cell.connections.reduce(
          (biggestOffset, connection) => {
            return getOffset(connection) > biggestOffset ? getOffset(connection) : biggestOffset;
          },
          0
        );
        cell.connections.forEach((connection) => {
          if (connection.direction !== direction || connection.connectedNode) {
            return;
          }
          if (!currentConnections.has(connection.polymerBond)) {
            const polymerBondConnections = this.polymerBondToConnections.get(
              connection.polymerBond
            );
            polymerBondConnections == null ? void 0 : polymerBondConnections.forEach(
              (polymerBondConnection) => {
                increaseOffset(polymerBondConnection, biggestOffsetInCell);
              }
            );
            currentConnections.set(
              connection.polymerBond,
              new Set(polymerBondConnections)
            );
          }
        });
        cell.connections.forEach((connection) => {
          if (!connection.connectedNode || connection.direction !== direction && !currentConnections.has(connection.polymerBond)) {
            return;
          }
          if (currentConnections.has(connection.polymerBond)) {
            currentConnections.delete(connection.polymerBond);
            currentConnections.forEach((connections) => {
              Array.from(connections.values()).forEach(
                (currentConnection) => {
                  increaseOffset(currentConnection);
                }
              );
            });
          } else {
            currentConnections.set(
              connection.polymerBond,
              new Set(this.polymerBondToConnections.get(connection.polymerBond))
            );
          }
        });
        if (cell.x === 0 && direction !== 90 || cell.y === 0 && direction === 90) {
          currentConnections.clear();
        }
        Array.from(currentConnections.keys()).forEach((polymerBond) => {
          const polymerBondConnections = this.polymerBondToConnections.get(polymerBond);
          if (polymerBondConnections == null ? void 0 : polymerBondConnections.every(
            (connection) => !cell.connections.includes(connection)
          )) {
            currentConnections.delete(polymerBond);
          }
        });
      });
    }
    fillRightConnectionsOffset() {
      const direction = 0;
      const handledConnections = /* @__PURE__ */ new Set();
      this.matrix.forEach((cell) => {
        const biggestOffsetInCell = cell.connections.reduce(
          (biggestOffset, connection) => {
            return connection.xOffset > biggestOffset ? connection.xOffset : biggestOffset;
          },
          0
        );
        cell.connections.forEach((connection) => {
          if (connection.direction !== direction) {
            return;
          }
          if (connection.xOffset <= biggestOffsetInCell) {
            const polymerBondConnections = this.polymerBondToConnections.get(
              connection.polymerBond
            );
            polymerBondConnections == null ? void 0 : polymerBondConnections.forEach(
              (polymerBondConnection) => {
                polymerBondConnection.xOffset = biggestOffsetInCell;
              }
            );
            handledConnections.add(connection.polymerBond);
          }
        });
      });
      handledConnections.forEach((polymerBond) => {
        const polymerBondConnections = this.polymerBondToConnections.get(polymerBond);
        polymerBondConnections == null ? void 0 : polymerBondConnections.forEach((polymerBondConnection) => {
          if (polymerBondConnection.direction !== direction) {
            return;
          }
          polymerBondConnection.xOffset++;
        });
      });
    }
    fillCells() {
      for (let rowNumber = 0; rowNumber < this.matrixConfig.initialMatrix.height; rowNumber++) {
        for (let columnNumber = 0; columnNumber < this.initialMatrixWidth; columnNumber++) {
          const initialMatrixCell = this.matrixConfig.initialMatrix.get(
            rowNumber,
            columnNumber
          );
          if (!initialMatrixCell) {
            this.matrix.set(
              rowNumber,
              columnNumber,
              new Cell(null, [], columnNumber, rowNumber)
            );
            continue;
          }
          const cell = new Cell(
            initialMatrixCell.node,
            [],
            columnNumber,
            rowNumber,
            initialMatrixCell.monomer
          );
          this.matrix.set(rowNumber, columnNumber, cell);
          if (initialMatrixCell.monomer) {
            this.monomerToCell.set(initialMatrixCell.monomer, cell);
          }
        }
      }
      const monomerToNode = this.chainsCollection.monomerToNode;
      const handledConnections = /* @__PURE__ */ new Set();
      this.matrix.forEach((cell) => {
        const monomer = cell.monomer;
        monomer == null ? void 0 : monomer.forEachBond((polymerBond) => {
          var _a, _b, _c, _d, _e, _f;
          if (polymerBond instanceof MonomerToAtomBond) {
            return;
          }
          if ((polymerBond.isSideChainConnection || polymerBond.isOverlappedByMonomer) && !handledConnections.has(polymerBond)) {
            const anotherMonomer = polymerBond.getAnotherMonomer(
              monomer
            );
            const connectedNode = monomerToNode.get(
              anotherMonomer
            );
            const connectedCell = this.monomerToCell.get(anotherMonomer);
            if (!connectedCell) {
              return;
            }
            const xDistance = connectedCell.x - cell.x;
            const yDistance = connectedCell.y - cell.y;
            const xDirection = xDistance > 0 ? 0 : 180;
            const yDirection = yDistance > 0 ? 90 : 270;
            let xDistanceAbsolute = Math.abs(xDistance);
            let yDistanceAbsolute = Math.abs(yDistance);
            const isVertical = xDistanceAbsolute === 0;
            let connection = new Connection(
              connectedNode,
              isVertical ? 90 : xDirection,
              isVertical,
              polymerBond,
              0,
              0
            );
            cell.connections.push(connection);
            this.polymerBondToCells.set(polymerBond, [cell]);
            this.polymerBondToConnections.set(polymerBond, [connection]);
            let nextCellX = cell.x;
            let nextCellY = cell.y;
            while (xDistanceAbsolute > 1) {
              nextCellX += Math.sign(xDistance);
              const nextCellToHandle = this.matrix.get(
                nextCellY,
                nextCellX
              );
              connection = new Connection(
                null,
                xDirection,
                isVertical,
                polymerBond,
                0,
                0
              );
              nextCellToHandle.connections.push(connection);
              (_a = this.polymerBondToCells.get(polymerBond)) == null ? void 0 : _a.push(nextCellToHandle);
              (_b = this.polymerBondToConnections.get(polymerBond)) == null ? void 0 : _b.push(connection);
              xDistanceAbsolute--;
            }
            while (yDistanceAbsolute > 1) {
              nextCellY += Math.sign(yDistance);
              const nextCellToHandle = this.matrix.get(
                nextCellY,
                nextCellX
              );
              connection = new Connection(
                null,
                yDirection,
                isVertical,
                polymerBond,
                0,
                0
              );
              nextCellToHandle.connections.push(connection);
              (_c = this.polymerBondToCells.get(polymerBond)) == null ? void 0 : _c.push(nextCellToHandle);
              (_d = this.polymerBondToConnections.get(polymerBond)) == null ? void 0 : _d.push(connection);
              yDistanceAbsolute--;
            }
            nextCellX += Math.sign(xDistance);
            nextCellY += Math.sign(yDistance);
            const lastCellToHandle = this.matrix.get(
              nextCellY,
              nextCellX
            );
            connection = new Connection(
              connectedNode,
              isVertical ? yDirection : { x: xDistance === 0 ? 0 : xDirection, y: yDirection },
              isVertical,
              polymerBond,
              0,
              0
            );
            lastCellToHandle.connections.push(connection);
            (_e = this.polymerBondToCells.get(polymerBond)) == null ? void 0 : _e.push(lastCellToHandle);
            (_f = this.polymerBondToConnections.get(polymerBond)) == null ? void 0 : _f.push(connection);
            handledConnections.add(polymerBond);
          }
        });
      });
      this.fillConnectionsOffset(180);
      this.fillRightConnectionsOffset();
      this.fillConnectionsOffset(0);
      this.fillConnectionsOffset(
        90,
        (connection, increaseValue) => {
          if ((0, import_lodash7.isNumber)(increaseValue)) {
            connection.yOffset = increaseValue;
          } else {
            connection.yOffset++;
          }
        },
        (connection) => connection.yOffset
      );
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_snake = __toESM(require_snake());
  var import_atom5 = __toESM(require_atom());
  var import_bond6 = __toESM(require_bond());
  var import_monomerToAtomBond = __toESM(require_monomerToAtomBond());
  var import_monomers21 = __toESM(require_monomers2());
  var import_modes = __toESM(require_modes());

  // domain/entities/snake-layout-model/SnakeLayoutModel.ts
  var import_editorSingleton = __toESM(require_editorSingleton());

  // domain/entities/snake-layout-model/SingleMonomerSnakeLayoutNode.ts
  var SingleMonomerSnakeLayoutNode = class {
    constructor(monomer) {
      this.monomer = monomer;
    }
    get monomers() {
      return [this.monomer];
    }
  };

  // domain/entities/snake-layout-model/SugarWithBaseSnakeLayoutNode.ts
  var SugarWithBaseSnakeLayoutNode = class {
    constructor(sugar, base) {
      this.sugar = sugar;
      this.base = base;
    }
    get monomers() {
      return [this.sugar, this.base];
    }
  };

  // domain/entities/snake-layout-model/SnakeLayoutModel.ts
  var import_lodash8 = __toESM(require_lodash2());
  var import_editor = __toESM(require_editor());
  var import_utilities3 = __toESM(require_utilities());

  // domain/entities/snake-layout-model/MoleculeSnakeLayoutNode.ts
  var MoleculeSnakeLayoutNode = class {
    constructor(molecule) {
      this.molecule = molecule;
    }
  };

  // domain/entities/snake-layout-model/EmptySnakeLayoutNode.ts
  var EmptySnakeLayoutNode = class {
    constructor() {
      __publicField(this, "kind", "empty");
    }
  };

  // domain/entities/snake-layout-model/types.ts
  function isTwoStrandedSnakeLayoutNode(node) {
    return !(node instanceof MoleculeSnakeLayoutNode) && !(node instanceof EmptySnakeLayoutNode);
  }

  // domain/entities/snake-layout-model/SnakeLayoutModelChain.ts
  var SnakeLayoutModelChain = class {
    constructor() {
      __publicField(this, "rows", []);
    }
    get lastRow() {
      return this.rows[this.rows.length - 1];
    }
    get lastNode() {
      return this.lastRow.snakeLayoutModelItems[this.lastRow.snakeLayoutModelItems.length - 1];
    }
    get firstRow() {
      return this.rows[0];
    }
    get firstNode() {
      return this.firstRow.snakeLayoutModelItems[0];
    }
    get nodes() {
      return this.rows.reduce(
        (acc, row) => acc.concat(row.snakeLayoutModelItems),
        []
      );
    }
    get length() {
      return this.nodes.length;
    }
    get rowsLength() {
      return this.rows.length;
    }
    addRow(row) {
      this.rows.push(row);
    }
    forEachNode(callback) {
      let nodeIndexInChain = 0;
      this.rows.forEach((row) => {
        row.snakeLayoutModelItems.forEach((node) => {
          callback(node, nodeIndexInChain);
          nodeIndexInChain++;
        });
      });
    }
    forEachRow(callback) {
      this.rows.forEach((row, rowIndex) => {
        callback(row, rowIndex);
      });
    }
  };

  // domain/entities/snake-layout-model/SnakeLayoutModel.ts
  var SnakeLayoutModel = class {
    constructor(chainsCollection, drawingEntitiesManager, needFillMolecules = true) {
      __publicField(this, "nodes", []);
      __publicField(this, "chains", []);
      __publicField(this, "monomerToTwoStrandedSnakeLayoutNode", /* @__PURE__ */ new Map());
      this.fillNodes(chainsCollection);
      this.fillChains();
      if (needFillMolecules) {
        this.fillMolecules(drawingEntitiesManager);
      }
    }
    addNode(snakeLayoutNode, chain) {
      const twoStrandedSnakeLayoutNode = {
        senseNode: snakeLayoutNode,
        chain
      };
      this.nodes.push(twoStrandedSnakeLayoutNode);
      snakeLayoutNode.monomers.forEach((monomer) => {
        this.monomerToTwoStrandedSnakeLayoutNode.set(
          monomer,
          twoStrandedSnakeLayoutNode
        );
      });
    }
    getSnakeLayoutNodesFromChainNode(node, isAntisense = false) {
      const nodes = [];
      if (node instanceof Nucleotide) {
        if (isAntisense) {
          nodes.push(new SingleMonomerSnakeLayoutNode(node.phosphate));
          nodes.push(new SugarWithBaseSnakeLayoutNode(node.sugar, node.rnaBase));
        } else {
          nodes.push(new SugarWithBaseSnakeLayoutNode(node.sugar, node.rnaBase));
          nodes.push(new SingleMonomerSnakeLayoutNode(node.phosphate));
        }
      } else if (node instanceof Nucleoside) {
        nodes.push(new SugarWithBaseSnakeLayoutNode(node.sugar, node.rnaBase));
      } else if (node instanceof LinkerSequenceNode) {
        if (isAntisense) {
          node.monomers.reverse().forEach((monomer) => {
            nodes.push(new SingleMonomerSnakeLayoutNode(monomer));
          });
        } else {
          node.monomers.forEach((monomer) => {
            nodes.push(new SingleMonomerSnakeLayoutNode(monomer));
          });
        }
      } else {
        nodes.push(new SingleMonomerSnakeLayoutNode(node.monomer));
      }
      return nodes;
    }
    fillSenseNodes(chainsCollection) {
      chainsCollection.chains.forEach((chain) => {
        if (chain.isAntisense) {
          return;
        }
        chain.forEachNode(({ node }) => {
          const snakeLayoutNodes = this.getSnakeLayoutNodesFromChainNode(node);
          snakeLayoutNodes.forEach((snakeLayoutNode) => {
            this.addNode(snakeLayoutNode, chain);
          });
        });
      });
    }
    fillAntisenseNodes(chainsCollection) {
      const handledChainNodes = /* @__PURE__ */ new Set();
      const monomerToChain = chainsCollection.monomerToChain;
      const editor = (0, import_editorSingleton.provideEditorInstance)();
      chainsCollection.chains.forEach((chain) => {
        var _a, _b, _c;
        if (!chain.isAntisense) {
          return;
        }
        let nodesBeforeHydrogenConnectionToBase = [];
        let lastTwoStrandedNodeWithHydrogenBond;
        chain.forEachNodeReversed(({ node }) => {
          if (handledChainNodes.has(node)) {
            return;
          }
          const snakeLayoutNodes = this.getSnakeLayoutNodesFromChainNode(
            node,
            true
          );
          snakeLayoutNodes.forEach((snakeLayoutNode) => {
            var _a2, _b2;
            const senseMonomersConnectedByHydrogenBond = snakeLayoutNode.monomers.reduce((foundMonomersInNode, monomer) => {
              return [
                ...foundMonomersInNode,
                ...monomer.hydrogenBonds.reduce(
                  (foundMonomersConnectedHydrogenBonds, hydrogenBond) => {
                    var _a3, _b3;
                    const monomerConnectedByHydrogenBond = hydrogenBond.getAnotherMonomer(monomer);
                    return monomerConnectedByHydrogenBond && (isRnaBaseApplicableForAntisense(
                      monomerConnectedByHydrogenBond
                    ) && isRnaBaseApplicableForAntisense(monomer) || ((_a3 = editor.drawingEntitiesManager.antisenseMonomerToSenseChain.get(
                      monomer
                    )) == null ? void 0 : _a3.firstMonomer) === ((_b3 = monomerToChain.get(monomerConnectedByHydrogenBond)) == null ? void 0 : _b3.firstMonomer)) ? [
                      ...foundMonomersConnectedHydrogenBonds,
                      monomerConnectedByHydrogenBond
                    ] : foundMonomersConnectedHydrogenBonds;
                  },
                  []
                )
              ];
            }, []);
            const firstSenseMonomerConnectedByHydrogenBond = senseMonomersConnectedByHydrogenBond[0];
            const twoStrandedSnakeLayoutNode = firstSenseMonomerConnectedByHydrogenBond ? this.monomerToTwoStrandedSnakeLayoutNode.get(
              firstSenseMonomerConnectedByHydrogenBond
            ) : void 0;
            let twoStrandedSnakeLayoutNodeIndex = this.nodes.findIndex((node2) => {
              return node2 === twoStrandedSnakeLayoutNode;
            });
            const lastTwoStrandedNodeWithHydrogenBondIndex = this.nodes.findIndex(
              (node2) => {
                return node2 === lastTwoStrandedNodeWithHydrogenBond;
              }
            );
            if (firstSenseMonomerConnectedByHydrogenBond && (!(0, import_lodash8.isNumber)(lastTwoStrandedNodeWithHydrogenBondIndex) || twoStrandedSnakeLayoutNodeIndex > lastTwoStrandedNodeWithHydrogenBondIndex)) {
              nodesBeforeHydrogenConnectionToBase.push(snakeLayoutNode);
              lastTwoStrandedNodeWithHydrogenBond = this.nodes[twoStrandedSnakeLayoutNodeIndex];
              for (let i = 0; i < nodesBeforeHydrogenConnectionToBase.length; i++) {
                twoStrandedSnakeLayoutNodeIndex = this.nodes.findIndex((node2) => {
                  return node2 === twoStrandedSnakeLayoutNode;
                });
                const currentTwoStrandedSnakeLayoutNodeIndex = twoStrandedSnakeLayoutNodeIndex - i;
                const currentTwoStrandedSnakeLayoutNode = this.nodes[currentTwoStrandedSnakeLayoutNodeIndex];
                const currentNodeBeforeHydrogenConnectionToBase = nodesBeforeHydrogenConnectionToBase[nodesBeforeHydrogenConnectionToBase.length - 1 - i];
                const firstMonomerInLastTwoStrandedNodeWithHydrogenBond = (_a2 = lastTwoStrandedNodeWithHydrogenBond == null ? void 0 : lastTwoStrandedNodeWithHydrogenBond.senseNode) == null ? void 0 : _a2.monomers[0];
                const firstMonomerInCurrentTwoStrandedSnakeLayoutNode = (_b2 = currentTwoStrandedSnakeLayoutNode == null ? void 0 : currentTwoStrandedSnakeLayoutNode.senseNode) == null ? void 0 : _b2.monomers[0];
                const isNodeInSameChain = firstMonomerInLastTwoStrandedNodeWithHydrogenBond && firstMonomerInCurrentTwoStrandedSnakeLayoutNode && monomerToChain.get(
                  firstMonomerInLastTwoStrandedNodeWithHydrogenBond
                ) === monomerToChain.get(
                  firstMonomerInCurrentTwoStrandedSnakeLayoutNode
                );
                if (currentTwoStrandedSnakeLayoutNode && !currentTwoStrandedSnakeLayoutNode.antisenseNode && isNodeInSameChain) {
                  currentTwoStrandedSnakeLayoutNode.antisenseNode = currentNodeBeforeHydrogenConnectionToBase;
                } else if (currentTwoStrandedSnakeLayoutNodeIndex < 0) {
                  this.nodes.unshift({
                    antisenseNode: currentNodeBeforeHydrogenConnectionToBase,
                    chain: lastTwoStrandedNodeWithHydrogenBond.chain
                  });
                } else {
                  this.nodes.splice(
                    currentTwoStrandedSnakeLayoutNodeIndex + 1,
                    0,
                    {
                      antisenseNode: currentNodeBeforeHydrogenConnectionToBase,
                      chain: lastTwoStrandedNodeWithHydrogenBond.chain
                    }
                  );
                }
              }
              nodesBeforeHydrogenConnectionToBase = [];
            } else {
              nodesBeforeHydrogenConnectionToBase.push(snakeLayoutNode);
            }
          });
          handledChainNodes.add(node);
        });
        if (nodesBeforeHydrogenConnectionToBase.length && lastTwoStrandedNodeWithHydrogenBond) {
          for (let i = 0; i < nodesBeforeHydrogenConnectionToBase.length; i++) {
            const lastTwoStrandedNodeWithHydrogenBondIndex = this.nodes.findIndex(
              (node) => {
                return node === lastTwoStrandedNodeWithHydrogenBond;
              }
            );
            const currentTwoStrandedSnakeLayoutNodeIndex = lastTwoStrandedNodeWithHydrogenBondIndex + 1 + i;
            const currentTwoStrandedSnakeLayoutNode = this.nodes[currentTwoStrandedSnakeLayoutNodeIndex];
            const currentAntisenseSnakeLayoutNode = nodesBeforeHydrogenConnectionToBase[i];
            const firstMonomerInLastTwoStrandedNodeWithHydrogenBond = (_a = lastTwoStrandedNodeWithHydrogenBond == null ? void 0 : lastTwoStrandedNodeWithHydrogenBond.senseNode) == null ? void 0 : _a.monomers[0];
            const firstMonomerInCurrentTwoStrandedSnakeLayoutNode = (_b = currentTwoStrandedSnakeLayoutNode == null ? void 0 : currentTwoStrandedSnakeLayoutNode.senseNode) == null ? void 0 : _b.monomers[0];
            const isNodeInSameChain = firstMonomerInLastTwoStrandedNodeWithHydrogenBond && firstMonomerInCurrentTwoStrandedSnakeLayoutNode && monomerToChain.get(
              firstMonomerInLastTwoStrandedNodeWithHydrogenBond
            ) === monomerToChain.get(
              firstMonomerInCurrentTwoStrandedSnakeLayoutNode
            );
            const hasAnotherAntisenseConnection = (_c = currentTwoStrandedSnakeLayoutNode == null ? void 0 : currentTwoStrandedSnakeLayoutNode.senseNode) == null ? void 0 : _c.monomers.some(
              (monomer) => {
                return monomer instanceof RNABase && monomer.hydrogenBonds.length !== 0;
              }
            );
            if (currentTwoStrandedSnakeLayoutNode && isNodeInSameChain && !hasAnotherAntisenseConnection) {
              currentTwoStrandedSnakeLayoutNode.antisenseNode = currentAntisenseSnakeLayoutNode;
            } else if (currentTwoStrandedSnakeLayoutNode && (!isNodeInSameChain || hasAnotherAntisenseConnection)) {
              this.nodes.splice(currentTwoStrandedSnakeLayoutNodeIndex, 0, {
                antisenseNode: currentAntisenseSnakeLayoutNode,
                chain: lastTwoStrandedNodeWithHydrogenBond.chain
              });
            } else {
              this.nodes.push({
                antisenseNode: currentAntisenseSnakeLayoutNode,
                chain: lastTwoStrandedNodeWithHydrogenBond.chain
              });
            }
          }
          lastTwoStrandedNodeWithHydrogenBond = void 0;
        }
      });
    }
    fillNodes(chainsCollection) {
      this.fillSenseNodes(chainsCollection);
      this.fillAntisenseNodes(chainsCollection);
    }
    forEachNode(callback) {
      this.nodes.forEach(callback);
    }
    forEachChain(callback) {
      this.chains.forEach(callback);
    }
    fillChains() {
      const lineLength = import_utilities3.SettingsManager.editorLineLength["snake-layout-mode"];
      let currentIndexInSequenceModelChain = 0;
      let currentSequenceModelChain = new SnakeLayoutModelChain();
      let currentSequenceModelRow = {
        snakeLayoutModelItems: []
      };
      let previousSenseNodeChain;
      this.nodes.forEach((sequenceModelItem) => {
        const currentSenseChain = sequenceModelItem.chain;
        if (previousSenseNodeChain !== currentSenseChain) {
          currentSequenceModelChain = new SnakeLayoutModelChain();
          this.chains.push(currentSequenceModelChain);
          currentIndexInSequenceModelChain = 0;
        }
        if (currentIndexInSequenceModelChain % lineLength === 0) {
          currentSequenceModelRow = {
            snakeLayoutModelItems: []
          };
          currentSequenceModelChain.addRow(currentSequenceModelRow);
        }
        currentSequenceModelRow.snakeLayoutModelItems.push(sequenceModelItem);
        previousSenseNodeChain = currentSenseChain;
        currentIndexInSequenceModelChain++;
      });
    }
    fillMolecules(drawingEntitiesManager) {
      const lineLength = import_utilities3.SettingsManager.editorLineLength["snake-layout-mode"];
      const handledMonomerConnectedToMolecules = /* @__PURE__ */ new Set();
      const handledMolecules = [];
      this.chains.forEach((chain, chainIndex) => {
        const newChain = new SnakeLayoutModelChain();
        chain.forEachRow((row, rowIndex) => {
          newChain.addRow(row);
          const nodeIndexToMolecules = /* @__PURE__ */ new Map();
          const isLastRowOfLastChain = chainIndex === this.chains.length - 1 && rowIndex === chain.rowsLength - 1;
          row.snakeLayoutModelItems.forEach((node, nodeIndex) => {
            var _a, _b, _c, _d;
            if (!isTwoStrandedSnakeLayoutNode(node)) {
              return;
            }
            const monomers = [
              ...(_b = (_a = node.senseNode) == null ? void 0 : _a.monomers) != null ? _b : [],
              ...(_d = (_c = node.antisenseNode) == null ? void 0 : _c.monomers) != null ? _d : []
            ];
            monomers.forEach((monomer) => {
              monomer.monomerToAtomBonds.forEach((monomerToAtomBond) => {
                var _a2;
                const molecule = drawingEntitiesManager.getConnectedMolecule(
                  monomerToAtomBond.atom,
                  [Atom]
                );
                if (!handledMonomerConnectedToMolecules.has(
                  monomerToAtomBond.atom.monomer
                )) {
                  handledMonomerConnectedToMolecules.add(
                    monomerToAtomBond.atom.monomer
                  );
                }
                if (!nodeIndexToMolecules.has(nodeIndex)) {
                  nodeIndexToMolecules.set(nodeIndex, []);
                }
                const isHandledMolecule = molecule.find(
                  (atom) => handledMolecules.find(
                    (atomConnectedToMonomer) => atomConnectedToMonomer.id === atom.id
                  )
                );
                if (!isHandledMolecule) {
                  handledMolecules.push(molecule[0]);
                  (_a2 = nodeIndexToMolecules.get(nodeIndex)) == null ? void 0 : _a2.push(molecule);
                }
              });
            });
          });
          if (isLastRowOfLastChain) {
            drawingEntitiesManager.atoms.forEach((atom) => {
              var _a;
              if (handledMonomerConnectedToMolecules.has(atom.monomer)) {
                return;
              }
              const molecule = drawingEntitiesManager.getConnectedMolecule(atom, [
                Atom
              ]);
              handledMonomerConnectedToMolecules.add(atom.monomer);
              if (!nodeIndexToMolecules.has(row.snakeLayoutModelItems.length)) {
                nodeIndexToMolecules.set(row.snakeLayoutModelItems.length, []);
              }
              (_a = nodeIndexToMolecules.get(row.snakeLayoutModelItems.length)) == null ? void 0 : _a.push(molecule);
            });
          }
          const editorSettings = (0, import_editor.provideEditorSettings)();
          const cellSizeInAngstroms = SnakeLayoutCellWidth / editorSettings.macroModeScale;
          let currentRowToHandle = {
            snakeLayoutModelItems: []
          };
          let nextCellIndexToFill = 0;
          let emptyRowsToAdd = 0;
          nodeIndexToMolecules.forEach((molecules) => {
            molecules.forEach((molecule) => {
              const moleculeBbox = DrawingEntitiesManager.getStructureBbox(molecule);
              const cellsNeededHorizontally = Math.ceil(
                (moleculeBbox.width + cellSizeInAngstroms / 2) / cellSizeInAngstroms
              );
              const cellsNeededVertically = Math.ceil(
                (moleculeBbox.height + cellSizeInAngstroms / 2) / cellSizeInAngstroms
              );
              const isThereEnoughSpaceInCurrentRow = nextCellIndexToFill + cellsNeededHorizontally <= lineLength;
              const freeCellsInCurrentRow = lineLength - nextCellIndexToFill;
              if (!isThereEnoughSpaceInCurrentRow) {
                for (let i = 0; i < freeCellsInCurrentRow; i++) {
                  currentRowToHandle.snakeLayoutModelItems.push(
                    new EmptySnakeLayoutNode()
                  );
                }
                newChain.addRow(currentRowToHandle);
                currentRowToHandle = { snakeLayoutModelItems: [] };
                nextCellIndexToFill = 0;
                for (let i = 0; i < emptyRowsToAdd; i++) {
                  newChain.addRow({
                    snakeLayoutModelItems: row.snakeLayoutModelItems.map(
                      (_) => new EmptySnakeLayoutNode()
                    )
                  });
                }
                emptyRowsToAdd = 0;
              }
              currentRowToHandle.snakeLayoutModelItems.push(
                new MoleculeSnakeLayoutNode(molecule)
              );
              for (let i = 1; i < cellsNeededHorizontally; i++) {
                currentRowToHandle.snakeLayoutModelItems.push(
                  new EmptySnakeLayoutNode()
                );
              }
              nextCellIndexToFill += cellsNeededHorizontally;
              emptyRowsToAdd = Math.max(
                emptyRowsToAdd,
                cellsNeededVertically - 1
              );
            });
          });
          if (currentRowToHandle.snakeLayoutModelItems.length) {
            for (let i = currentRowToHandle.snakeLayoutModelItems.length; i < lineLength; i++) {
              currentRowToHandle.snakeLayoutModelItems.push(
                new EmptySnakeLayoutNode()
              );
            }
            newChain.addRow(currentRowToHandle);
          }
          if (emptyRowsToAdd > 0) {
            for (let i = 0; i < emptyRowsToAdd; i++) {
              newChain.addRow({
                snakeLayoutModelItems: row.snakeLayoutModelItems.map(
                  (_) => new EmptySnakeLayoutNode()
                )
              });
            }
          }
        });
        this.chains.splice(chainIndex, 1, newChain);
      });
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_utilities4 = __toESM(require_utilities());
  var import_rxnArrow = __toESM(require_rxnArrow());

  // domain/entities/CoreRxnArrow.ts
  var RxnArrow2 = class extends DrawingEntity {
    constructor(type, startEndPosition, height, initiallySelected) {
      super();
      this.type = type;
      this.startEndPosition = startEndPosition;
      this.height = height;
      this.initiallySelected = initiallySelected;
      __publicField(this, "renderer");
      __publicField(this, "arrowId");
    }
    get startPosition() {
      return this.startEndPosition[0];
    }
    set startPosition(newStartPosition) {
      this.startEndPosition[0] = newStartPosition;
    }
    get endPosition() {
      return this.startEndPosition[1];
    }
    set endPosition(newEndPosition) {
      this.startEndPosition[1] = newEndPosition;
    }
    get center() {
      return new Vec2(
        (this.startPosition.x + this.endPosition.x) / 2,
        (this.startPosition.y + this.endPosition.y) / 2
      );
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    moveRelative(delta) {
      this.startPosition = this.startPosition.add(delta);
      this.endPosition = this.endPosition.add(delta);
    }
    moveAbsolute(position) {
      const delta = Vec2.diff(position, this.startPosition);
      this.moveRelative(delta);
    }
  };

  // domain/entities/CoreMultitailArrow.ts
  var MultitailArrow2 = class _MultitailArrow2 extends DrawingEntity {
    constructor(spineTopX, spineTopY, height, headOffsetX, headOffsetY, tailLength, tailsYOffset) {
      super();
      this.spineTopX = spineTopX;
      this.spineTopY = spineTopY;
      this.height = height;
      this.headOffsetX = headOffsetX;
      this.headOffsetY = headOffsetY;
      this.tailLength = tailLength;
      this.tailsYOffset = tailsYOffset;
      __publicField(this, "renderer");
      __publicField(this, "arrowId");
    }
    get center() {
      return Vec2.centre(
        new Vec2(
          this.spineTopX.sub(this.tailLength).getFloatingPrecision(),
          this.spineTopY.getFloatingPrecision()
        ),
        new Vec2(
          this.spineTopX.add(this.headOffsetX).getFloatingPrecision(),
          this.spineTopY.add(this.height).getFloatingPrecision()
        )
      );
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
    moveRelative(delta) {
      this.spineTopX = this.spineTopX.add(
        FixedPrecisionCoordinates.fromFloatingPrecision(delta.x)
      );
      this.spineTopY = this.spineTopY.add(
        FixedPrecisionCoordinates.fromFloatingPrecision(delta.y)
      );
    }
    moveAbsolute(position) {
      const delta = Vec2.diff(
        position,
        new Vec2(this.spineTopX.value, this.spineTopY.value)
      );
      this.moveRelative(delta);
    }
    static fromKet(multitailArrowKetNode) {
      const {
        spineTopX,
        spineTopY,
        height,
        headOffsetX,
        headOffsetY,
        tailsLength,
        tailsYOffset
      } = MultitailArrow.getConstructorParamsFromKetNode(
        multitailArrowKetNode
      );
      return new _MultitailArrow2(
        spineTopX,
        spineTopY,
        height,
        headOffsetX,
        headOffsetY,
        tailsLength,
        tailsYOffset
      );
    }
    toKetNode() {
      return MultitailArrow.getParametersForKetNode(
        this.spineTopX,
        this.spineTopY,
        this.headOffsetX,
        this.headOffsetY,
        this.tailLength,
        this.tailsYOffset,
        this.height,
        this.center,
        false
      );
    }
    getReferencePositions() {
      return MultitailArrow.getReferencePositions(
        this.spineTopX,
        this.spineTopY,
        this.height,
        this.headOffsetX,
        this.headOffsetY,
        this.tailLength,
        this.tailsYOffset
      );
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_multitailArrow2 = __toESM(require_multitailArrow());
  var import_serializers = __toESM(require_serializers());

  // domain/entities/CoreRxnPlus.ts
  var RxnPlus2 = class extends DrawingEntity {
    constructor(position, initiallySelected) {
      super(position);
      this.initiallySelected = initiallySelected;
      __publicField(this, "renderer");
    }
    get center() {
      return this.position;
    }
    setRenderer(renderer) {
      super.setBaseRenderer(renderer);
      this.renderer = renderer;
    }
  };

  // domain/entities/DrawingEntitiesManager.ts
  var import_rxnPlus = __toESM(require_rxnPlus());
  var VERTICAL_DISTANCE_FROM_ROW_WITHOUT_RNA = SnakeLayoutCellWidth;
  var VERTICAL_OFFSET_FROM_ROW_WITH_RNA = 142;
  var SNAKE_LAYOUT_Y_OFFSET_BETWEEN_CHAINS = SnakeLayoutCellWidth * 2 + 30;
  var MONOMER_START_X_POSITION = 20 + SnakeLayoutCellWidth / 2;
  var MONOMER_START_Y_POSITION = 20 + SnakeLayoutCellWidth / 2;
  var DrawingEntitiesManager = class _DrawingEntitiesManager {
    constructor() {
      __publicField(this, "monomers", /* @__PURE__ */ new Map());
      __publicField(this, "polymerBonds", /* @__PURE__ */ new Map());
      __publicField(this, "bondsMonomersOverlaps", /* @__PURE__ */ new Map());
      __publicField(this, "atoms", /* @__PURE__ */ new Map());
      __publicField(this, "bonds", /* @__PURE__ */ new Map());
      __publicField(this, "monomerToAtomBonds", /* @__PURE__ */ new Map());
      __publicField(this, "rxnArrows", /* @__PURE__ */ new Map());
      __publicField(this, "multitailArrows", /* @__PURE__ */ new Map());
      __publicField(this, "rxnPluses", /* @__PURE__ */ new Map());
      __publicField(this, "micromoleculesHiddenEntities", new Struct());
      __publicField(this, "canvasMatrix");
      __publicField(this, "snakeLayoutMatrix");
      __publicField(this, "antisenseMonomerToSenseChain", /* @__PURE__ */ new Map());
      __publicField(this, "nextArrowId", 0);
      __publicField(this, "addRnaPresetFromNode", (node, connections) => {
        const command = new Command();
        const sugarMonomer = node.monomers.find(
          (monomer) => monomer instanceof Sugar
        );
        const phosphateMonomer = node.monomers.find(
          (monomer) => monomer instanceof Phosphate
        );
        const rnaBaseMonomer = node.monomers.find(
          (monomer) => isRnaBaseOrAmbiguousRnaBase2(monomer)
        );
        const monomers = [rnaBaseMonomer, sugarMonomer, phosphateMonomer].filter(
          (monomer) => monomer !== void 0
        );
        monomers.forEach((monomer, monomerIndex) => {
          const monomerAddOperation = monomer instanceof AmbiguousMonomer ? new import_monomer.MonomerAddOperation(
            this.addAmbiguousMonomerChangeModel.bind(
              this,
              monomer.variantMonomerItem,
              monomer.position,
              monomer
            ),
            this.deleteMonomerChangeModel.bind(this)
          ) : new import_monomer.MonomerAddOperation(
            this.addMonomerChangeModel.bind(
              this,
              monomer.monomerItem,
              monomer.position,
              monomer
            ),
            this.deleteMonomerChangeModel.bind(this)
          );
          command.addOperation(monomerAddOperation);
          if (monomerIndex > 0) {
            const previousMonomer = monomers[monomerIndex - 1];
            const connectionTemplate = this.findGroupTemplateConnection(
              connections || [],
              previousMonomer.monomerItem,
              monomer.monomerItem
            );
            let attPointStart;
            let attPointEnd;
            if (connectionTemplate) {
              const isEndpoint1 = connectionTemplate.endpoint1.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(previousMonomer.monomerItem);
              attPointStart = isEndpoint1 ? connectionTemplate.endpoint1.attachmentPointId : connectionTemplate.endpoint2.attachmentPointId;
              attPointEnd = isEndpoint1 ? connectionTemplate.endpoint2.attachmentPointId : connectionTemplate.endpoint1.attachmentPointId;
            } else {
              attPointStart = previousMonomer.getValidSourcePoint(monomer);
              attPointEnd = monomer.getValidSourcePoint(previousMonomer);
            }
            assert_default(attPointStart);
            assert_default(attPointEnd);
            const operation = new import_polymerBond.PolymerBondFinishCreationOperation(
              (polymerBond) => this.finishPolymerBondCreationModelChange(
                previousMonomer,
                monomer,
                attPointStart,
                attPointEnd,
                import_types15.MACROMOLECULES_BOND_TYPES.SINGLE,
                polymerBond
              ),
              this.deletePolymerBondChangeModel.bind(this)
            );
            command.addOperation(operation);
          }
        });
        return command;
      });
    }
    ensureArrowId(arrow) {
      var _a;
      const arrowId = (_a = arrow.arrowId) != null ? _a : this.nextArrowId;
      arrow.arrowId = arrowId;
      this.nextArrowId = Math.max(this.nextArrowId, arrowId + 1);
      return arrow;
    }
    resetArrowIdCounter() {
      this.nextArrowId = 0;
    }
    static normalizeInitiallySelected(initiallySelected) {
      return typeof initiallySelected === "boolean" ? initiallySelected : void 0;
    }
    get bottomRightMonomerPosition() {
      let position = null;
      this.monomers.forEach((monomer) => {
        if (!position || monomer.position.x + monomer.position.y > position.x + position.y) {
          position = monomer.position;
        }
      });
      return position != null ? position : new Vec2(0, 0, 0);
    }
    get bottomLeftMonomerPosition() {
      const bbox = _DrawingEntitiesManager.getStructureBbox(this.monomersArray);
      return new Vec2(bbox.left, bbox.bottom);
    }
    get selectedEntitiesArr() {
      const selectedEntities = [];
      this.allEntities.forEach(([, drawingEntity]) => {
        if (drawingEntity.selected) {
          selectedEntities.push(drawingEntity);
        }
      });
      return selectedEntities;
    }
    get selectedEntities() {
      return this.allEntities.filter(
        ([, drawingEntity]) => drawingEntity.selected
      );
    }
    get selectedMonomers() {
      return this.monomersArray.filter((monomer) => monomer.selected);
    }
    get selectedMicromoleculeEntities() {
      return this.selectedEntitiesArr.filter(
        (entity) => !(entity instanceof BaseMonomer || entity instanceof PolymerBond || entity instanceof HydrogenBond)
      );
    }
    get externalConnectionsToSelection() {
      const connectedMonomers = [];
      this.selectedMonomers.forEach((monomer) => {
        monomer.bonds.forEach((bond) => {
          if (!(bond instanceof PolymerBond || bond instanceof HydrogenBond) || !bond.secondMonomer) {
            return;
          }
          if (bond.firstMonomer === monomer && !bond.secondMonomer.selected) {
            connectedMonomers.push({
              monomerFromSelection: monomer,
              monomerConnectedToSelection: bond.secondMonomer,
              bond
            });
          } else if (bond.secondMonomer === monomer && !bond.firstMonomer.selected) {
            connectedMonomers.push({
              monomerFromSelection: monomer,
              monomerConnectedToSelection: bond.firstMonomer,
              bond
            });
          }
        });
      });
      return connectedMonomers;
    }
    get allEntities() {
      return [
        ...this.monomers,
        ...this.polymerBonds,
        ...this.monomerToAtomBonds,
        ...this.atoms,
        ...this.bonds,
        ...this.rxnArrows,
        ...this.multitailArrows,
        ...this.rxnPluses
      ];
    }
    get allEntitiesArray() {
      return this.allEntities.map(([, drawingEntity]) => drawingEntity);
    }
    get hasDrawingEntities() {
      return this.allEntities.length !== 0;
    }
    get hasMonomers() {
      const monomers = [...this.monomers.values()].filter(
        (monomer) => !monomer.monomerItem.props.isMicromoleculeFragment || (0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(monomer)
      );
      return monomers.length !== 0;
    }
    get allBondsToMonomers() {
      return [
        ...this.polymerBonds,
        ...this.monomerToAtomBonds
      ];
    }
    deleteSelectedEntities() {
      const mergedCommand = new Command();
      this.selectedEntities.forEach(([, drawingEntity]) => {
        const command = this.deleteDrawingEntity(drawingEntity);
        mergedCommand.merge(command);
      });
      return mergedCommand;
    }
    deleteAllEntities() {
      const mergedCommand = new Command();
      this.allEntities.forEach(([, drawingEntity]) => {
        const command = this.deleteDrawingEntity(drawingEntity, false);
        mergedCommand.merge(command);
      });
      this.clearMicromoleculesHiddenEntities();
      this.resetArrowIdCounter();
      return mergedCommand;
    }
    addMonomerChangeModel(monomerItem, position, _monomer) {
      if (_monomer) {
        this.monomers.set(_monomer.id, _monomer);
        return _monomer;
      }
      const newMonomer = this.createMonomer(monomerItem, position);
      newMonomer.moveAbsolute(position);
      this.monomers.set(newMonomer.id, newMonomer);
      return newMonomer;
    }
    createMonomer(monomerItem, position, generateId = true) {
      if (isAmbiguousMonomerLibraryItem(monomerItem)) {
        return new AmbiguousMonomer(monomerItem, position, generateId);
      } else {
        const [Monomer] = (0, import_monomerFactory.monomerFactory)(monomerItem);
        return new Monomer(monomerItem, position, { generateId });
      }
    }
    updateMonomerItem(monomer, monomerItemNew) {
      const initialMonomer = this.monomers.get(monomer.id);
      if (!initialMonomer) return monomer;
      initialMonomer.monomerItem = Object.isFrozen(monomerItemNew) ? __spreadValues({}, monomerItemNew) : monomerItemNew;
      initialMonomer.recalculateAttachmentPoints();
      this.monomers.set(monomer.id, initialMonomer);
      return initialMonomer;
    }
    addMonomer(monomerItem, position, _monomer) {
      const command = new Command();
      let addMonomerChangeModelCallback = this.addMonomerChangeModel.bind(
        this,
        monomerItem,
        position
      );
      if (_monomer) {
        addMonomerChangeModelCallback = addMonomerChangeModelCallback.bind(
          this,
          _monomer
        );
      }
      const operation = new import_monomer.MonomerAddOperation(
        addMonomerChangeModelCallback,
        this.deleteMonomerChangeModel.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    deleteDrawingEntity(drawingEntity, needToDeleteConnectedEntities = true, force = false) {
      if (drawingEntity instanceof BaseMonomer) {
        return this.deleteMonomer(
          drawingEntity,
          needToDeleteConnectedEntities,
          force
        );
      } else if (drawingEntity instanceof PolymerBond || drawingEntity instanceof HydrogenBond) {
        return this.deletePolymerBond(drawingEntity);
      } else if (drawingEntity instanceof MonomerToAtomBond) {
        return this.deleteMonomerToAtomBond(drawingEntity);
      } else if (drawingEntity instanceof Bond) {
        return this.deleteBond(drawingEntity);
      } else if (drawingEntity instanceof Atom) {
        return this.deleteAtom(drawingEntity, needToDeleteConnectedEntities);
      } else if (drawingEntity instanceof RxnArrow2) {
        return this.deleteRxnArrow(drawingEntity);
      } else if (drawingEntity instanceof MultitailArrow2) {
        return this.deleteMultitailArrow(drawingEntity);
      } else if (drawingEntity instanceof RxnPlus2) {
        return this.deleteRxnPlus(drawingEntity);
      } else {
        return new Command();
      }
    }
    selectDrawingEntity(drawingEntity) {
      const command = this.unselectAllDrawingEntities();
      drawingEntity.turnOnSelection();
      command.merge(this.createDrawingEntitySelectionCommand(drawingEntity));
      return command;
    }
    selectDrawingEntitiesModelChange(drawingEntity) {
      drawingEntity.turnOnSelection();
    }
    selectDrawingEntities(drawingEntities) {
      const command = this.unselectAllDrawingEntities();
      drawingEntities.forEach((drawingEntity) => {
        drawingEntity.turnOnSelection();
        const operation = new import_drawingEntity.DrawingEntitySelectOperation(
          drawingEntity,
          this.selectDrawingEntitiesModelChange.bind(this, drawingEntity)
        );
        command.addOperation(operation);
      });
      return command;
    }
    createDrawingEntitySelectionCommand(drawingEntity) {
      const command = new Command();
      const selectionCommand = new import_drawingEntity.DrawingEntitySelectOperation(drawingEntity);
      command.addOperation(selectionCommand);
      return command;
    }
    unselectAllDrawingEntities() {
      const command = new Command();
      this.allEntities.forEach(([, drawingEntity]) => {
        if (drawingEntity.selected) {
          command.merge(this.unselectDrawingEntity(drawingEntity));
        }
      });
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      editor.events.selectEntities.dispatch(
        this.selectedEntities.map((entity) => entity[1])
      );
      return command;
    }
    unselectDrawingEntity(drawingEntity) {
      const command = new Command();
      drawingEntity.turnOffSelection();
      command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(drawingEntity));
      return command;
    }
    selectAllDrawingEntities() {
      const command = new Command();
      this.allEntities.forEach(([, drawingEntity]) => {
        if (!drawingEntity.selected) {
          drawingEntity.turnOnSelection();
          const operation = new import_drawingEntity.DrawingEntitySelectOperation(drawingEntity);
          command.addOperation(operation);
        }
      });
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      editor.events.selectEntities.dispatch(
        this.selectedEntities.map((entity) => entity[1])
      );
      return command;
    }
    addDrawingEntitiesToSelection(drawingEntities) {
      const command = new Command();
      drawingEntities.forEach((drawingEntity) => {
        if (drawingEntity.selected) {
          drawingEntity.turnOffSelection();
        } else {
          drawingEntity.turnOnSelection();
        }
        command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(drawingEntity));
      });
      return command;
    }
    moveDrawingEntityModelChange(drawingEntity, offset) {
      if (drawingEntity instanceof PolymerBond || drawingEntity instanceof HydrogenBond) {
        drawingEntity.moveToLinkedEntities();
        drawingEntity.isOverlappedByMonomer = this.checkBondForOverlapsByMonomers(drawingEntity);
      } else if (drawingEntity instanceof Bond) {
        drawingEntity.moveToLinkedAtoms();
      } else if (drawingEntity instanceof MonomerToAtomBond) {
        drawingEntity.moveToLinkedEntities();
      } else {
        assert_default(offset);
        drawingEntity.moveRelative(offset);
        if (drawingEntity instanceof BaseMonomer && (0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          this.moveChemAtomsPoint(drawingEntity, offset);
        }
      }
      return drawingEntity;
    }
    moveChemAtomsPoint(drawingEntity, offset) {
      if (drawingEntity.monomerItem.props.isMicromoleculeFragment && offset) {
        drawingEntity.monomerItem.struct.atoms.forEach((atom) => {
          atom.pp.add_(offset);
        });
        drawingEntity.monomerItem.struct.sgroups.forEach((sgroup) => {
          var _a;
          (_a = sgroup.pp) == null ? void 0 : _a.add_(offset);
        });
      }
    }
    moveSelectedDrawingEntities(partOfMovementOffset, fullMovementOffset) {
      const command = new Command();
      [
        ...this.atoms.values(),
        ...this.monomers.values(),
        ...this.rxnArrows.values(),
        ...this.multitailArrows.values(),
        ...this.rxnPluses.values()
      ].forEach((drawingEntity) => {
        if (drawingEntity instanceof BaseMonomer && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        if (drawingEntity.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              partOfMovementOffset,
              fullMovementOffset
            )
          );
        }
      });
      this.polymerBonds.forEach((drawingEntity) => {
        var _a;
        if (drawingEntity.selected || drawingEntity.firstMonomer.selected || ((_a = drawingEntity.secondMonomer) == null ? void 0 : _a.selected)) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              partOfMovementOffset,
              fullMovementOffset
            )
          );
        }
      });
      this.monomerToAtomBonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.monomer.selected || drawingEntity.atom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              partOfMovementOffset,
              fullMovementOffset
            )
          );
        }
      });
      this.bonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.firstAtom.selected || drawingEntity.secondAtom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              partOfMovementOffset,
              fullMovementOffset
            )
          );
        }
      });
      return command;
    }
    rotateSelectedDrawingEntities(center, angleInDegrees, isPartialRotation = true) {
      const command = new Command();
      [
        ...this.atoms.values(),
        ...this.monomers.values(),
        ...this.rxnArrows.values(),
        ...this.multitailArrows.values(),
        ...this.rxnPluses.values()
      ].forEach((drawingEntity) => {
        if (drawingEntity instanceof BaseMonomer && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        if (drawingEntity.selected) {
          const newPosition = drawingEntity.position.rotateAroundOrigin(
            angleInDegrees,
            center
          );
          const positionDelta = newPosition.sub(drawingEntity.position);
          if (isPartialRotation) {
            command.merge(
              this.createDrawingEntityMovingCommand(drawingEntity, positionDelta)
            );
          } else {
            command.merge(
              this.createDrawingEntityMovingCommand(
                drawingEntity,
                positionDelta,
                positionDelta
              )
            );
          }
        }
      });
      this.polymerBonds.forEach((drawingEntity) => {
        var _a;
        if (drawingEntity.selected || drawingEntity.firstMonomer.selected || ((_a = drawingEntity.secondMonomer) == null ? void 0 : _a.selected)) {
          command.addOperation(this.movePolymerBond(drawingEntity));
        }
      });
      this.monomerToAtomBonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.monomer.selected || drawingEntity.atom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              new Vec2(0, 0),
              new Vec2(0, 0)
            )
          );
        }
      });
      this.bonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.firstAtom.selected || drawingEntity.secondAtom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              new Vec2(0, 0),
              new Vec2(0, 0)
            )
          );
        }
      });
      return command;
    }
    flipSelectedDrawingEntities(flipDirection) {
      const command = new Command();
      const center = this.getSelectedEntitiesCenter();
      const zeroOffset = new Vec2(0, 0);
      if (!center) {
        return command;
      }
      [
        ...this.atoms.values(),
        ...this.monomers.values(),
        ...this.rxnArrows.values(),
        ...this.multitailArrows.values(),
        ...this.rxnPluses.values()
      ].forEach((drawingEntity) => {
        if (drawingEntity instanceof BaseMonomer && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        if (drawingEntity.selected) {
          let newPosition;
          if (flipDirection === "horizontal") {
            newPosition = new Vec2(
              center.x - (drawingEntity.position.x - center.x),
              drawingEntity.position.y
            );
          } else {
            newPosition = new Vec2(
              drawingEntity.position.x,
              center.y - (drawingEntity.position.y - center.y)
            );
          }
          const positionDelta = newPosition.sub(drawingEntity.position);
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              positionDelta,
              positionDelta
            )
          );
        }
      });
      this.polymerBonds.forEach((drawingEntity) => {
        var _a;
        if (drawingEntity.selected || drawingEntity.firstMonomer.selected || ((_a = drawingEntity.secondMonomer) == null ? void 0 : _a.selected)) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      this.monomerToAtomBonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.monomer.selected || drawingEntity.atom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      this.bonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.firstAtom.selected || drawingEntity.secondAtom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      return command;
    }
    getSelectedEntitiesBoundingBox() {
      const selectedEntities = this.selectedEntitiesArr;
      if (selectedEntities.length === 0) {
        return null;
      }
      return _DrawingEntitiesManager.getStructureBbox(selectedEntities);
    }
    getSelectedEntitiesCenter() {
      const bbox = this.getSelectedEntitiesBoundingBox();
      if (!bbox) {
        return null;
      }
      return new Vec2(bbox.left + bbox.width / 2, bbox.top + bbox.height / 2);
    }
    createDrawingEntityMovingCommand(drawingEntity, partOfMovementOffset, fullMovementOffset) {
      const command = new Command();
      const movingCommand = new import_drawingEntity.DrawingEntityMoveOperation(
        this.moveDrawingEntityModelChange.bind(
          this,
          drawingEntity,
          partOfMovementOffset
        ),
        this.moveDrawingEntityModelChange.bind(
          this,
          drawingEntity,
          fullMovementOffset ? fullMovementOffset.negated() : partOfMovementOffset.negated()
        ),
        this.moveDrawingEntityModelChange.bind(
          this,
          drawingEntity,
          fullMovementOffset || partOfMovementOffset
        ),
        drawingEntity
      );
      command.addOperation(movingCommand);
      return command;
    }
    createDrawingEntityRedrawCommand(drawingEntityRedrawModelChange, invertDrawingEntityRedrawModelChange) {
      const command = new Command();
      const redrawCommand = new import_drawingEntity.DrawingEntityRedrawOperation(
        drawingEntityRedrawModelChange,
        invertDrawingEntityRedrawModelChange
      );
      command.addOperation(redrawCommand);
      return command;
    }
    deleteMonomerChangeModel(monomer) {
      this.monomers.delete(monomer.id);
    }
    deleteMonomer(monomer, needToDeleteConnectedBonds = true, force = false) {
      const command = new Command();
      if (monomer instanceof EmptyMonomer) {
        return command;
      }
      const operation = new import_monomer.MonomerDeleteOperation(
        monomer,
        this.addMonomerChangeModel.bind(
          this,
          monomer.monomerItem,
          monomer.position
        ),
        this.deleteMonomerChangeModel.bind(this)
      );
      command.addOperation(operation);
      if (needToDeleteConnectedBonds && monomer.hasBonds) {
        monomer.forEachBond((bond) => {
          if (bond.selected && !force) return;
          if (bond instanceof PolymerBond || bond instanceof HydrogenBond) {
            bond.turnOnSelection();
            command.merge(this.deletePolymerBond(bond));
          } else {
            command.merge(this.deleteMonomerToAtomBond(bond));
          }
        });
      }
      return command;
    }
    modifyMonomerItem(monomer, monomerItemNew) {
      const command = new Command();
      const operation = new import_monomer.MonomerItemModifyOperation(
        monomer,
        this.updateMonomerItem.bind(this, monomer, monomerItemNew),
        this.updateMonomerItem.bind(this, monomer, monomer.monomerItem)
      );
      command.addOperation(operation);
      return command;
    }
    selectIfLocatedInRectangle(rectangleTopLeftPoint, rectangleBottomRightPoint, previousSelectedEntities, shiftKey = false) {
      const command = new Command();
      this.allEntities.forEach(([, drawingEntity]) => {
        if (drawingEntity instanceof Chem && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        const isPreviousSelected = previousSelectedEntities.find(
          ([, entity]) => entity === drawingEntity
        );
        let isValueChanged;
        const editor = (0, import_editorSingleton2.provideEditorInstance)();
        if (editor.mode.modeName === "sequence-layout-mode" && drawingEntity instanceof PolymerBond) {
          isValueChanged = this.checkBondSelectionForSequenceMode(drawingEntity);
        } else {
          isValueChanged = drawingEntity.selectIfLocatedInRectangle(
            rectangleTopLeftPoint,
            rectangleBottomRightPoint,
            !!isPreviousSelected,
            shiftKey
          );
        }
        if (isValueChanged) {
          const selectionCommand = this.createDrawingEntitySelectionCommand(drawingEntity);
          command.merge(selectionCommand);
        }
      });
      return command;
    }
    selectIfLocatedInPolygon(polygonPoints, previousSelectedEntities, shiftKey = false) {
      const command = new Command();
      this.allEntities.forEach(([, drawingEntity]) => {
        if (drawingEntity instanceof Chem && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        const isPreviousSelected = previousSelectedEntities.find(
          ([, entity]) => entity === drawingEntity
        );
        let isValueChanged;
        const editor = (0, import_editorSingleton2.provideEditorInstance)();
        if (editor.mode.modeName === "sequence-layout-mode" && drawingEntity instanceof PolymerBond) {
          isValueChanged = this.checkBondSelectionForSequenceMode(drawingEntity);
        } else {
          isValueChanged = drawingEntity.selectIfLocatedInPolygon(
            polygonPoints,
            !!isPreviousSelected,
            shiftKey
          );
        }
        if (isValueChanged) {
          const selectionCommand = this.createDrawingEntitySelectionCommand(drawingEntity);
          command.merge(selectionCommand);
        }
      });
      return command;
    }
    checkBondSelectionForSequenceMode(bond) {
      var _a;
      const prevSelectedValue = bond.selected;
      if (bond.firstMonomer.selected && ((_a = bond.secondMonomer) == null ? void 0 : _a.selected)) {
        bond.turnOnSelection();
      } else {
        bond.turnOffSelection();
      }
      return prevSelectedValue !== bond.selected;
    }
    startPolymerBondCreationChangeModel(firstMonomer, startPosition, endPosition, bondType = import_types15.MACROMOLECULES_BOND_TYPES.SINGLE, _polymerBond) {
      if (_polymerBond) {
        this.polymerBonds.set(_polymerBond.id, _polymerBond);
        return _polymerBond;
      }
      const polymerBond = bondType === import_types15.MACROMOLECULES_BOND_TYPES.HYDROGEN ? new HydrogenBond(firstMonomer) : new PolymerBond(firstMonomer);
      this.polymerBonds.set(polymerBond.id, polymerBond);
      if (firstMonomer.chosenFirstAttachmentPointForBond) {
        const startBondAttachmentPoint = firstMonomer.startBondAttachmentPoint;
        firstMonomer.setBond(startBondAttachmentPoint, polymerBond);
        firstMonomer.setPotentialBond(startBondAttachmentPoint, polymerBond);
      }
      polymerBond.moveBondStartAbsolute(startPosition.x, startPosition.y);
      polymerBond.moveBondEndAbsolute(endPosition.x, endPosition.y);
      return polymerBond;
    }
    startPolymerBondCreation(firstMonomer, startPosition, endPosition, bondType) {
      const command = new Command();
      const operation = new import_polymerBond.PolymerBondAddOperation(
        this.startPolymerBondCreationChangeModel.bind(
          this,
          firstMonomer,
          startPosition,
          endPosition,
          bondType
        ),
        this.deletePolymerBondChangeModel.bind(this)
      );
      command.addOperation(operation);
      return { command, polymerBond: operation.polymerBond };
    }
    deletePolymerBondChangeModel(polymerBond) {
      var _a, _b, _c, _d;
      if (this.polymerBonds.get(polymerBond.id) !== polymerBond) {
        return;
      }
      this.polymerBonds.delete(polymerBond.id);
      const firstMonomerAttachmentPoint = polymerBond.firstMonomer.getAttachmentPointByBond(polymerBond);
      const secondMonomerAttachmentPoint = (_a = polymerBond.secondMonomer) == null ? void 0 : _a.getAttachmentPointByBond(polymerBond);
      polymerBond.firstMonomer.removePotentialBonds();
      (_b = polymerBond.secondMonomer) == null ? void 0 : _b.removePotentialBonds();
      polymerBond.firstMonomer.turnOffSelection();
      (_c = polymerBond.secondMonomer) == null ? void 0 : _c.turnOffSelection();
      if (firstMonomerAttachmentPoint || polymerBond instanceof HydrogenBond) {
        polymerBond.firstMonomer.unsetBond(
          firstMonomerAttachmentPoint,
          polymerBond
        );
      }
      if (secondMonomerAttachmentPoint || polymerBond instanceof HydrogenBond) {
        (_d = polymerBond.secondMonomer) == null ? void 0 : _d.unsetBond(
          secondMonomerAttachmentPoint,
          polymerBond
        );
      }
    }
    deletePolymerBond(polymerBond) {
      var _a;
      const command = new Command();
      const firstAttachmentPoint = polymerBond.firstMonomer.getAttachmentPointByBond(
        polymerBond
      );
      const secondAttachmentPoint = (_a = polymerBond.secondMonomer) == null ? void 0 : _a.getAttachmentPointByBond(
        polymerBond
      );
      const operation = new import_polymerBond.PolymerBondDeleteOperation(
        polymerBond,
        this.deletePolymerBondChangeModel.bind(this, polymerBond),
        (_polymerBond) => this.finishPolymerBondCreationModelChange(
          polymerBond.firstMonomer,
          polymerBond.secondMonomer,
          firstAttachmentPoint,
          secondAttachmentPoint,
          import_types15.MACROMOLECULES_BOND_TYPES.SINGLE,
          _polymerBond
        )
      );
      command.addOperation(operation);
      return command;
    }
    cancelPolymerBondCreation(polymerBond, secondMonomer) {
      this.polymerBonds.delete(polymerBond.id);
      const command = new Command();
      polymerBond.firstMonomer.removeBond(polymerBond);
      polymerBond.firstMonomer.removePotentialBonds(true);
      polymerBond.firstMonomer.turnOffSelection();
      polymerBond.firstMonomer.turnOffHover();
      polymerBond.firstMonomer.turnOffAttachmentPointsVisibility();
      secondMonomer == null ? void 0 : secondMonomer.turnOffSelection();
      secondMonomer == null ? void 0 : secondMonomer.turnOffHover();
      secondMonomer == null ? void 0 : secondMonomer.turnOffAttachmentPointsVisibility();
      const operation = new import_polymerBond.PolymerBondCancelCreationOperation(
        polymerBond,
        secondMonomer
      );
      command.addOperation(operation);
      return command;
    }
    movePolymerBond(polymerBond, position) {
      const command = new Command();
      if (position) {
        polymerBond.moveBondEndAbsolute(position.x, position.y);
      } else {
        polymerBond.moveToLinkedEntities();
      }
      const operation = new import_polymerBond.PolymerBondMoveOperation(polymerBond);
      command.addOperation(operation);
      return command;
    }
    finishPolymerBondCreationModelChange(firstMonomer, secondMonomer, firstMonomerAttachmentPoint, secondMonomerAttachmentPoint, bondType, _polymerBond) {
      var _a;
      if (_polymerBond) {
        this.polymerBonds.set(_polymerBond.id, _polymerBond);
        firstMonomer.setBond(firstMonomerAttachmentPoint, _polymerBond);
        secondMonomer.setBond(secondMonomerAttachmentPoint, _polymerBond);
        return _polymerBond;
      }
      const isHydrogenBond = bondType === import_types15.MACROMOLECULES_BOND_TYPES.HYDROGEN;
      const polymerBond = isHydrogenBond ? new HydrogenBond(firstMonomer) : new PolymerBond(firstMonomer);
      this.polymerBonds.set(polymerBond.id, polymerBond);
      polymerBond.setSecondMonomer(secondMonomer);
      polymerBond.firstMonomer.setBond(firstMonomerAttachmentPoint, polymerBond);
      assert_default(polymerBond.secondMonomer);
      polymerBond.secondMonomer.setBond(
        secondMonomerAttachmentPoint,
        polymerBond
      );
      polymerBond.firstMonomer.removePotentialBonds(true);
      polymerBond.secondMonomer.removePotentialBonds(true);
      polymerBond.firstMonomer.setChosenFirstAttachmentPoint(null);
      (_a = polymerBond.secondMonomer) == null ? void 0 : _a.setChosenSecondAttachmentPoint(null);
      polymerBond.moveToLinkedEntities();
      polymerBond.firstMonomer.turnOffSelection();
      polymerBond.firstMonomer.turnOffHover();
      polymerBond.firstMonomer.turnOffAttachmentPointsVisibility();
      polymerBond.secondMonomer.turnOffSelection();
      polymerBond.secondMonomer.turnOffHover();
      polymerBond.secondMonomer.turnOffAttachmentPointsVisibility();
      polymerBond.turnOffHover();
      return polymerBond;
    }
    finishPolymerBondCreation(polymerBond, secondMonomer, firstMonomerAttachmentPoint, secondMonomerAttachmentPoint, bondType = import_types15.MACROMOLECULES_BOND_TYPES.SINGLE) {
      const command = new Command();
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      const firstMonomer = polymerBond.firstMonomer;
      this.polymerBonds.delete(polymerBond.id);
      const operation = new import_polymerBond.PolymerBondFinishCreationOperation(
        (polymerBond2) => this.finishPolymerBondCreationModelChange(
          firstMonomer,
          secondMonomer,
          firstMonomerAttachmentPoint,
          secondMonomerAttachmentPoint,
          bondType,
          polymerBond2
        ),
        this.deletePolymerBondChangeModel.bind(this)
      );
      command.addOperation(operation);
      if (editor.mode.modeName === "snake-layout-mode") {
        command.merge(this.recalculateCanvasMatrix());
      }
      command.merge(this.recalculateAntisenseChains());
      return command;
    }
    createPolymerBond(firstMonomer, secondMonomer, firstMonomerAttachmentPoint, secondMonomerAttachmentPoint, bondType = import_types15.MACROMOLECULES_BOND_TYPES.SINGLE) {
      const command = new Command();
      const operation = new import_polymerBond.PolymerBondFinishCreationOperation(
        (polymerBond) => this.finishPolymerBondCreationModelChange(
          firstMonomer,
          secondMonomer,
          firstMonomerAttachmentPoint,
          secondMonomerAttachmentPoint,
          bondType,
          polymerBond
        ),
        this.deletePolymerBondChangeModel.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    intendToStartBondCreation(monomer) {
      const command = new Command();
      monomer.turnOnHover();
      monomer.turnOnAttachmentPointsVisibility();
      const operation = new import_monomer.MonomerHoverOperation(monomer, true);
      command.addOperation(operation);
      return command;
    }
    intendToStartAttachmenPointBondCreation(monomer, attachmentPointName) {
      const command = new Command();
      monomer.turnOnHover();
      monomer.turnOnAttachmentPointsVisibility();
      const operation = new import_monomer.AttachmentPointHoverOperation(
        monomer,
        attachmentPointName
      );
      command.addOperation(operation);
      return command;
    }
    intendToFinishBondCreation(monomer, bond, shouldCalculateBonds) {
      const command = new Command();
      monomer.turnOnHover();
      monomer.turnOnAttachmentPointsVisibility();
      if (shouldCalculateBonds) {
        bond.firstMonomer.removePotentialBonds();
        monomer.removePotentialBonds();
        const firstMonomerValidSourcePoint = bond.firstMonomer.getValidSourcePoint(monomer);
        const secondMonomerValidTargetPoint = monomer.getValidTargetPoint(
          bond.firstMonomer
        );
        bond.firstMonomer.setPotentialBond(firstMonomerValidSourcePoint, bond);
        monomer.setPotentialBond(secondMonomerValidTargetPoint, bond);
      }
      const connectFirstMonomerOperation = new import_monomer.MonomerHoverOperation(
        bond.firstMonomer,
        true
      );
      const connectSecondMonomerOperation = new import_monomer.MonomerHoverOperation(
        monomer,
        true
      );
      command.addOperation(connectFirstMonomerOperation);
      command.addOperation(connectSecondMonomerOperation);
      return command;
    }
    intendToFinishAttachmenPointBondCreation(monomer, bond, attachmentPointName, shouldCalculateBonds) {
      const command = new Command();
      monomer.turnOnHover();
      monomer.turnOnAttachmentPointsVisibility();
      if (monomer.isAttachmentPointUsed(attachmentPointName)) {
        const operation = new import_monomer.MonomerHoverOperation(monomer, true);
        command.addOperation(operation);
        return command;
      }
      if (attachmentPointName) {
        monomer.setPotentialSecondAttachmentPoint(attachmentPointName);
        monomer.setPotentialBond(attachmentPointName, bond);
      }
      if (shouldCalculateBonds) {
        bond.firstMonomer.removePotentialBonds();
        monomer.removePotentialBonds();
        const firstMonomerValidSourcePoint = bond.firstMonomer.getValidSourcePoint(monomer);
        const secondMonomerValidTargetPoint = monomer.getValidTargetPoint(
          bond.firstMonomer
        );
        bond.firstMonomer.setPotentialBond(firstMonomerValidSourcePoint, bond);
        monomer.setPotentialBond(secondMonomerValidTargetPoint, bond);
      }
      const connectFirstMonomerOperation = new import_monomer.MonomerHoverOperation(
        bond.firstMonomer,
        true
      );
      const connectSecondMonomerOperation = new import_monomer.AttachmentPointHoverOperation(
        monomer,
        attachmentPointName
      );
      command.addOperation(connectFirstMonomerOperation);
      command.addOperation(connectSecondMonomerOperation);
      return command;
    }
    cancelIntentionToFinishBondCreation(monomer, polymerBond) {
      const command = new Command();
      monomer.turnOffHover();
      monomer.turnOffAttachmentPointsVisibility();
      monomer.setPotentialSecondAttachmentPoint(null);
      monomer.removePotentialBonds();
      const operation = new import_monomer.MonomerHoverOperation(monomer, true);
      command.addOperation(operation);
      if (polymerBond && !polymerBond.firstMonomer.chosenFirstAttachmentPointForBond) {
        polymerBond.firstMonomer.removePotentialBonds();
        const operation2 = new import_monomer.MonomerHoverOperation(
          polymerBond.firstMonomer,
          true
        );
        command.addOperation(operation2);
      }
      return command;
    }
    intendToSelectDrawingEntity(drawingEntity) {
      const command = new Command();
      drawingEntity.turnOnHover();
      const operation = new import_drawingEntity.DrawingEntityHoverOperation(drawingEntity);
      command.addOperation(operation);
      return command;
    }
    intendToSelectAllConnectedDrawingEntities(startEntity) {
      const command = new Command();
      this.visitAllConnectedEntities(startEntity, (drawingEntity) => {
        drawingEntity.turnOnHover();
        const operation = new import_drawingEntity.DrawingEntityHoverOperation(drawingEntity);
        command.addOperation(operation);
      });
      return command;
    }
    cancelIntentionToSelectDrawingEntity(drawingEntity) {
      const command = new Command();
      drawingEntity.turnOffHover();
      const operation = new import_drawingEntity.DrawingEntityHoverOperation(drawingEntity);
      command.addOperation(operation);
      return command;
    }
    cancelIntentionToSelectAllConnectedDrawingEntities(startEntity) {
      const command = new Command();
      this.visitAllConnectedEntities(startEntity, (drawingEntity) => {
        drawingEntity.turnOffHover();
        const operation = new import_drawingEntity.DrawingEntityHoverOperation(drawingEntity);
        command.addOperation(operation);
      });
      return command;
    }
    showPolymerBondInformation(polymerBond) {
      const command = new Command();
      polymerBond.turnOnHover();
      polymerBond.firstMonomer.turnOnHover();
      assert_default(polymerBond.secondMonomer);
      polymerBond.secondMonomer.turnOnHover();
      if (!(polymerBond instanceof HydrogenBond)) {
        polymerBond.firstMonomer.turnOnAttachmentPointsVisibility();
        polymerBond.secondMonomer.turnOnAttachmentPointsVisibility();
      }
      const operation = new import_polymerBond.PolymerBondShowInfoOperation(polymerBond);
      command.addOperation(operation);
      return command;
    }
    hidePolymerBondInformation(polymerBond) {
      const command = new Command();
      polymerBond.turnOffHover();
      polymerBond.firstMonomer.turnOffHover();
      polymerBond.firstMonomer.turnOffAttachmentPointsVisibility();
      assert_default(polymerBond.secondMonomer);
      polymerBond.secondMonomer.turnOffHover();
      polymerBond.secondMonomer.turnOffAttachmentPointsVisibility();
      const operation = new import_polymerBond.PolymerBondShowInfoOperation(polymerBond);
      command.addOperation(operation);
      return command;
    }
    hideAllMonomersHoverAndAttachmentPoints() {
      const command = new Command();
      this.monomers.forEach((monomer) => {
        monomer.turnOffHover();
        monomer.turnOffAttachmentPointsVisibility();
        const operation = new import_monomer.MonomerHoverOperation(monomer, true);
        command.addOperation(operation);
      });
      return command;
    }
    findGroupTemplateConnection(connections, monomer1, monomer2) {
      return monomer2 && (connections == null ? void 0 : connections.find((connection) => {
        return connection.endpoint1.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(monomer1) && connection.endpoint2.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(monomer2) || connection.endpoint2.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(monomer1) && connection.endpoint1.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(monomer2);
      }));
    }
    addRnaPreset({
      sugar,
      sugarPosition: _sugarPosition,
      phosphate,
      phosphatePosition: _phosphatePosition,
      rnaBase,
      rnaBasePosition: _rnaBasePosition,
      connections
    }) {
      const sugarPhosphateConnectionTemplate = this.findGroupTemplateConnection(
        connections || [],
        sugar,
        phosphate
      );
      const isFivePrimePhosphate = (sugarPhosphateConnectionTemplate == null ? void 0 : sugarPhosphateConnectionTemplate.endpoint1.templateId) === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(sugar) ? (sugarPhosphateConnectionTemplate == null ? void 0 : sugarPhosphateConnectionTemplate.endpoint1.attachmentPointId) === "R1" /* R1 */ : (sugarPhosphateConnectionTemplate == null ? void 0 : sugarPhosphateConnectionTemplate.endpoint2.attachmentPointId) === "R1" /* R1 */;
      const sugarPosition = isFivePrimePhosphate && _phosphatePosition ? _phosphatePosition : _sugarPosition;
      const phosphatePosition = isFivePrimePhosphate ? _sugarPosition : _phosphatePosition;
      const rnaBasePosition = isFivePrimePhosphate && sugarPosition && _rnaBasePosition ? new Vec2(sugarPosition.x, _rnaBasePosition.y) : _rnaBasePosition;
      const command = new Command();
      const monomersToAdd = [];
      if (rnaBase && rnaBasePosition) {
        monomersToAdd.push([rnaBase, rnaBasePosition]);
      }
      monomersToAdd.push([sugar, sugarPosition]);
      if (phosphate && phosphatePosition) {
        monomersToAdd.push([phosphate, phosphatePosition]);
      }
      const monomers = [];
      monomersToAdd.forEach(([monomerItem, monomerPosition], monomerIndex) => {
        const monomerAddOperation = new import_monomer.MonomerAddOperation(
          this.addMonomerChangeModel.bind(this, monomerItem, monomerPosition),
          this.deleteMonomerChangeModel.bind(this)
        );
        const monomer = monomerAddOperation.monomer;
        monomers.push(monomer);
        command.addOperation(monomerAddOperation);
        if (monomerIndex > 0) {
          const previousMonomer = monomers[monomerIndex - 1];
          const connectionTemplate = this.findGroupTemplateConnection(
            connections || [],
            previousMonomer.monomerItem,
            monomer.monomerItem
          );
          let attPointStart;
          let attPointEnd;
          if (connectionTemplate) {
            const isEndpoint1 = connectionTemplate.endpoint1.templateId === (0, import_serializers.getMonomerTemplateRefFromMonomerItem)(previousMonomer.monomerItem);
            attPointStart = isEndpoint1 ? connectionTemplate.endpoint1.attachmentPointId : connectionTemplate.endpoint2.attachmentPointId;
            attPointEnd = isEndpoint1 ? connectionTemplate.endpoint2.attachmentPointId : connectionTemplate.endpoint1.attachmentPointId;
          } else {
            attPointStart = previousMonomer.getValidSourcePoint(monomer);
            attPointEnd = monomer.getValidSourcePoint(previousMonomer);
          }
          assert_default(attPointStart);
          assert_default(attPointEnd);
          const operation = new import_polymerBond.PolymerBondFinishCreationOperation(
            (polymerBond) => this.finishPolymerBondCreationModelChange(
              previousMonomer,
              monomer,
              attPointStart,
              attPointEnd,
              import_types15.MACROMOLECULES_BOND_TYPES.SINGLE,
              polymerBond
            ),
            this.deletePolymerBondChangeModel.bind(this)
          );
          command.addOperation(operation);
        }
      });
      return { command, monomers };
    }
    rearrangeChainModelChange(monomer, newPosition) {
      if ((0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(monomer)) {
        const offset = newPosition.sub(monomer.position);
        this.moveChemAtomsPoint(monomer, offset);
      }
      monomer.moveAbsolute(newPosition);
      return monomer;
    }
    addRnaOperations(command, oldMonomerPosition, newPosition, monomer) {
      if (!monomer || !oldMonomerPosition || !newPosition) {
        return;
      }
      const operation = new import_monomer.MonomerMoveOperation(
        this.rearrangeChainModelChange.bind(
          this,
          monomer,
          import_coordinates3.Coordinates.canvasToModel(newPosition)
        ),
        this.rearrangeChainModelChange.bind(this, monomer, oldMonomerPosition)
      );
      command.addOperation(operation);
    }
    recalculateCanvasMatrixModelChange(snakeLayoutMatrix, _chainsCollection) {
      if (!snakeLayoutMatrix || snakeLayoutMatrix.height === 0) {
        return;
      }
      const chainsCollection = _chainsCollection || ChainsCollection.fromMonomers(Array.from(this.monomers.values()));
      if (!_chainsCollection) {
        chainsCollection.rearrange();
      }
      this.canvasMatrix = new CanvasMatrix(chainsCollection, {
        initialMatrix: snakeLayoutMatrix
      });
      return this.redrawBonds();
    }
    recalculateCanvasMatrix(chainsCollection, previousSnakeLayoutMatrix) {
      const command = new Command();
      command.addOperation(
        new import_snake.RecalculateCanvasMatrixOperation(
          this.recalculateCanvasMatrixModelChange.bind(
            this,
            this.snakeLayoutMatrix,
            chainsCollection
          ),
          this.recalculateCanvasMatrixModelChange.bind(
            this,
            previousSnakeLayoutMatrix,
            chainsCollection
          )
        )
      );
      return command;
    }
    calculateSnakeLayoutMatrix(chainsCollection) {
      const snakeLayoutMatrix = new Matrix();
      const monomersGroupedByY = /* @__PURE__ */ new Map();
      const monomerToNode = chainsCollection.monomerToNode;
      this.monomers.forEach((monomer) => {
        const x = Number(monomer.position.x.toFixed());
        const y = Number(monomer.position.y.toFixed());
        if (!monomersGroupedByY.has(y)) {
          monomersGroupedByY.set(y, /* @__PURE__ */ new Map());
        }
        const monomersGroupedByX = monomersGroupedByY.get(y);
        monomersGroupedByX == null ? void 0 : monomersGroupedByX.set(x, monomer);
      });
      const sortedGroupedMonomers = [...monomersGroupedByY.entries()].map(
        ([y, groupedByX]) => {
          const groupedByYArray = [
            y,
            [...groupedByX.entries()]
          ];
          return groupedByYArray;
        }
      );
      sortedGroupedMonomers.sort((a, b) => a[0] - b[0]);
      sortedGroupedMonomers.forEach(([y, groupedByY], index) => {
        groupedByY.sort((a, b) => Number(a[0]) - Number(b[0]));
        sortedGroupedMonomers[index] = [y, groupedByY];
      });
      const monomerXToIndexInMatrix = {};
      const allXPositions = /* @__PURE__ */ new Set();
      sortedGroupedMonomers.forEach(([, groupedByX]) => {
        groupedByX.forEach(([x]) => {
          allXPositions.add(x);
        });
      });
      const sortedXPositions = [...allXPositions].sort((a, b) => a - b);
      sortedXPositions.forEach((x, index) => {
        monomerXToIndexInMatrix[x] = index;
      });
      sortedGroupedMonomers.forEach(([, groupedByX], indexY) => {
        groupedByX.forEach(([x, monomer]) => {
          snakeLayoutMatrix.set(
            Number(indexY),
            Number(monomerXToIndexInMatrix[x]),
            new Cell(
              monomerToNode.get(monomer),
              [],
              Number(indexY),
              Number(monomerXToIndexInMatrix[x]),
              monomer
            )
          );
        });
      });
      return snakeLayoutMatrix;
    }
    rearrangeSingleMonomerSnakeLayoutNode(snakeLayoutNode, newPosition, rearrangedMonomersSet, needRepositionMonomers = true) {
      var _a;
      const command = new Command();
      if (needRepositionMonomers) {
        this.addRnaOperations(
          command,
          snakeLayoutNode.monomer.position,
          newPosition,
          snakeLayoutNode.monomer
        );
      }
      rearrangedMonomersSet.add((_a = snakeLayoutNode.monomer) == null ? void 0 : _a.id);
      return command;
    }
    rearrangeSugarWithBaseSnakeLayoutNode(snakeLayoutNode, newSugarPosition, rearrangedMonomersSet, needRepositionMonomers = true, isAntisense = false) {
      var _a, _b;
      const command = new Command();
      if (needRepositionMonomers) {
        this.addRnaOperations(
          command,
          snakeLayoutNode.sugar.position,
          newSugarPosition,
          snakeLayoutNode.sugar
        );
        this.addRnaOperations(
          command,
          (_a = snakeLayoutNode.base) == null ? void 0 : _a.position,
          new Vec2(
            newSugarPosition.x,
            newSugarPosition.y + (isAntisense ? -1 : 1) * SnakeLayoutCellWidth
          ),
          snakeLayoutNode.base
        );
      }
      rearrangedMonomersSet.add(snakeLayoutNode.sugar.id);
      rearrangedMonomersSet.add((_b = snakeLayoutNode.base) == null ? void 0 : _b.id);
      return command;
    }
    applySnakeLayout(isSnakeMode, needRedrawBonds = true, needRepositionMonomers = true, needRecalculateOldAntisense = true, needRepositionMolecules = true) {
      if (this.monomers.size === 0) {
        return new Command();
      }
      const previousSnakeLayoutMatrix = this.snakeLayoutMatrix;
      const command = new Command();
      let chainsCollection;
      command.merge(this.recalculateAntisenseChains(needRecalculateOldAntisense));
      if (isSnakeMode) {
        const editor = (0, import_editorSingleton2.provideEditorInstance)();
        const editorSettings = (0, import_editorSettings.provideEditorSettings)();
        const canvasWidth = editor.canvas.width.baseVal.value;
        const cellWidthInAngstroms = SnakeLayoutCellWidth / editorSettings.macroModeScale;
        const lineLengthFromSettings = import_utilities4.SettingsManager.editorLineLength["snake-layout-mode"];
        const lineLengthFromCanvasWidth = Math.floor(
          (canvasWidth - SnakeLayoutCellWidth) / SnakeLayoutCellWidth
        );
        if (lineLengthFromSettings === 0) {
          import_utilities4.SettingsManager.editorLineLength = {
            "snake-layout-mode": lineLengthFromCanvasWidth
          };
        }
        const rearrangedMonomersSet = /* @__PURE__ */ new Set();
        let lastPosition = new Vec2({
          x: MONOMER_START_X_POSITION,
          y: MONOMER_START_Y_POSITION
        });
        chainsCollection = ChainsCollection.fromMonomers([
          ...this.monomers.values()
        ]);
        chainsCollection.rearrange();
        const snakeLayoutModel = new SnakeLayoutModel(
          chainsCollection,
          this,
          needRepositionMolecules
        );
        let hasAntisenseInRow = false;
        let hasRnaInRow = false;
        let previousSenseNode;
        let previousAntisenseNode;
        let newSenseNodePosition = lastPosition;
        snakeLayoutModel.forEachChain((chain) => {
          chain.forEachRow((row) => {
            var _a;
            const firstNodeInRow = row.snakeLayoutModelItems[0];
            if (hasAntisenseInRow && isTwoStrandedSnakeLayoutNode(firstNodeInRow)) {
              const r1BondToPreviousMonomer = (_a = firstNodeInRow.senseNode) == null ? void 0 : _a.monomers[0].attachmentPointsToBonds.R1;
              if (r1BondToPreviousMonomer instanceof PolymerBond) {
                r1BondToPreviousMonomer.hasAntisenseInRow = true;
              }
              const r2BondFromPreviousSenseNode = previousSenseNode == null ? void 0 : previousSenseNode.monomers[0].attachmentPointsToBonds.R2;
              const r1BondFromPreviousAntisenseNode = previousAntisenseNode == null ? void 0 : previousAntisenseNode.monomers[0].attachmentPointsToBonds.R1;
              if (r2BondFromPreviousSenseNode instanceof PolymerBond) {
                r2BondFromPreviousSenseNode.nextRowPositionX = newSenseNodePosition.x;
              }
              if (r1BondFromPreviousAntisenseNode instanceof PolymerBond) {
                r1BondFromPreviousAntisenseNode.nextRowPositionX = newSenseNodePosition.x;
              }
            }
            hasAntisenseInRow = false;
            hasRnaInRow = false;
            row.snakeLayoutModelItems.forEach((twoStrandedSnakeLayoutNode) => {
              if (twoStrandedSnakeLayoutNode instanceof MoleculeSnakeLayoutNode) {
                const moleculeBbox = _DrawingEntitiesManager.getStructureBbox(
                  twoStrandedSnakeLayoutNode.molecule
                );
                const offset = Vec2.diff(
                  import_coordinates3.Coordinates.canvasToModel(newSenseNodePosition),
                  new Vec2(
                    moleculeBbox.left + cellWidthInAngstroms / 4,
                    moleculeBbox.top + cellWidthInAngstroms / 4
                  )
                );
                twoStrandedSnakeLayoutNode.molecule.forEach((atom) => {
                  command.merge(
                    this.createDrawingEntityMovingCommand(atom, offset)
                  );
                });
              } else if (isTwoStrandedSnakeLayoutNode(twoStrandedSnakeLayoutNode)) {
                const senseNode = twoStrandedSnakeLayoutNode.senseNode;
                const antisenseNode = twoStrandedSnakeLayoutNode.antisenseNode;
                if (senseNode) {
                  if (senseNode instanceof SugarWithBaseSnakeLayoutNode) {
                    command.merge(
                      this.rearrangeSugarWithBaseSnakeLayoutNode(
                        senseNode,
                        newSenseNodePosition,
                        rearrangedMonomersSet,
                        needRepositionMonomers
                      )
                    );
                    hasRnaInRow = true;
                  } else if (senseNode instanceof SingleMonomerSnakeLayoutNode) {
                    command.merge(
                      this.rearrangeSingleMonomerSnakeLayoutNode(
                        senseNode,
                        newSenseNodePosition,
                        rearrangedMonomersSet,
                        needRepositionMonomers
                      )
                    );
                  }
                }
                if (antisenseNode) {
                  if (antisenseNode instanceof SugarWithBaseSnakeLayoutNode) {
                    command.merge(
                      this.rearrangeSugarWithBaseSnakeLayoutNode(
                        antisenseNode,
                        new Vec2(
                          newSenseNodePosition.x,
                          newSenseNodePosition.y + SnakeLayoutCellWidth * 3
                        ),
                        rearrangedMonomersSet,
                        needRepositionMonomers,
                        true
                      )
                    );
                    hasRnaInRow = true;
                  } else if (antisenseNode instanceof SingleMonomerSnakeLayoutNode) {
                    command.merge(
                      this.rearrangeSingleMonomerSnakeLayoutNode(
                        antisenseNode,
                        new Vec2(
                          newSenseNodePosition.x,
                          newSenseNodePosition.y + SnakeLayoutCellWidth * 3
                        ),
                        rearrangedMonomersSet,
                        needRepositionMonomers
                      )
                    );
                  }
                  hasAntisenseInRow = true;
                }
                previousSenseNode = senseNode || previousSenseNode;
                previousAntisenseNode = antisenseNode || previousAntisenseNode;
              }
              lastPosition = newSenseNodePosition;
              newSenseNodePosition = new Vec2(
                lastPosition.x + SnakeLayoutCellWidth,
                lastPosition.y
              );
            });
            newSenseNodePosition = new Vec2(
              MONOMER_START_X_POSITION,
              lastPosition.y + (hasRnaInRow || hasAntisenseInRow ? VERTICAL_OFFSET_FROM_ROW_WITH_RNA : VERTICAL_DISTANCE_FROM_ROW_WITHOUT_RNA) + (hasAntisenseInRow ? SNAKE_LAYOUT_Y_OFFSET_BETWEEN_CHAINS : 0)
            );
          });
        });
        const snakeLayoutMatrix = this.calculateSnakeLayoutMatrix(chainsCollection);
        this.snakeLayoutMatrix = snakeLayoutMatrix;
        command.merge(
          this.recalculateCanvasMatrix(
            chainsCollection,
            previousSnakeLayoutMatrix
          )
        );
      }
      if (needRedrawBonds) {
        command.merge(this.redrawBonds());
      }
      return command;
    }
    redrawBondsModelChange(bond, startPosition, endPosition) {
      if (bond instanceof MonomerToAtomBond) {
        bond.moveToLinkedEntities();
        return bond;
      }
      if (startPosition && endPosition) {
        bond.moveBondStartAbsolute(startPosition.x, startPosition.y);
        bond.moveBondEndAbsolute(endPosition.x, endPosition.y);
      } else {
        bond.moveToLinkedEntities();
      }
      return bond;
    }
    redrawBonds() {
      const command = new Command();
      [
        ...this.polymerBonds.values(),
        ...this.monomerToAtomBonds.values(),
        ...this.bonds.values()
      ].forEach((polymerBond) => {
        command.merge(
          this.createDrawingEntityRedrawCommand(
            this.redrawBondsModelChange.bind(this, polymerBond),
            this.redrawBondsModelChange.bind(
              this,
              polymerBond,
              polymerBond.startPosition,
              polymerBond.endPosition
            )
          )
        );
      });
      return command;
    }
    isNucleosideAndPhosphateConnectedAsNucleotide(nucleoside, phosphate) {
      if (!(nucleoside instanceof Nucleoside) || !(phosphate instanceof Phosphate))
        return false;
      const r2Bond = nucleoside.sugar.attachmentPointsToBonds.R2;
      return !(r2Bond instanceof MonomerToAtomBond) && (r2Bond == null ? void 0 : r2Bond.secondMonomer) === phosphate;
    }
    setMicromoleculesHiddenEntities(struct) {
      struct.mergeInto(this.micromoleculesHiddenEntities);
      this.micromoleculesHiddenEntities.atoms = new Pool();
      this.micromoleculesHiddenEntities.bonds = new Pool();
      this.micromoleculesHiddenEntities.halfBonds = new Pool();
      this.micromoleculesHiddenEntities.sgroups = new Pool();
      this.micromoleculesHiddenEntities.functionalGroups = new Pool();
      this.micromoleculesHiddenEntities.sGroupForest = new SGroupForest();
      this.micromoleculesHiddenEntities.frags = new Pool();
      this.micromoleculesHiddenEntities.rxnArrows = new Pool();
      this.micromoleculesHiddenEntities.rxnPluses = new Pool();
      this.micromoleculesHiddenEntities.multitailArrows = new Pool();
    }
    clearMicromoleculesHiddenEntities() {
      this.micromoleculesHiddenEntities = new Struct();
    }
    mergeInto(targetDrawingEntitiesManager) {
      const command = new Command();
      const monomerToNewMonomer = /* @__PURE__ */ new Map();
      const atomToNewAtom = /* @__PURE__ */ new Map();
      const mergedDrawingEntities = new _DrawingEntitiesManager();
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      const viewModel = editor.viewModel;
      this.monomers.forEach((monomer) => {
        const monomerAddCommand = monomer instanceof AmbiguousMonomer ? targetDrawingEntitiesManager.addAmbiguousMonomer(
          __spreadValues({}, monomer.variantMonomerItem),
          monomer.position
        ) : targetDrawingEntitiesManager.addMonomer(
          monomer.monomerItem,
          monomer.position
        );
        command.merge(monomerAddCommand);
        const addedMonomer = monomerAddCommand.operations[0].monomer;
        mergedDrawingEntities.monomers.set(addedMonomer.id, addedMonomer);
        monomerToNewMonomer.set(
          monomer,
          monomerAddCommand.operations[0].monomer
        );
      });
      this.polymerBonds.forEach((polymerBond) => {
        assert_default(polymerBond.secondMonomer);
        const polymerBondCreateCommand = targetDrawingEntitiesManager.createPolymerBond(
          monomerToNewMonomer.get(polymerBond.firstMonomer),
          monomerToNewMonomer.get(polymerBond.secondMonomer),
          polymerBond.firstMonomer.getAttachmentPointByBond(
            polymerBond
          ),
          polymerBond.secondMonomer.getAttachmentPointByBond(
            polymerBond
          ),
          polymerBond instanceof HydrogenBond ? import_types15.MACROMOLECULES_BOND_TYPES.HYDROGEN : import_types15.MACROMOLECULES_BOND_TYPES.SINGLE
        );
        command.merge(polymerBondCreateCommand);
        const addedPolymerBond = polymerBondCreateCommand.operations[0].polymerBond;
        mergedDrawingEntities.polymerBonds.set(
          addedPolymerBond.id,
          addedPolymerBond
        );
      });
      this.atoms.forEach((atom) => {
        const atomAddCommand = targetDrawingEntitiesManager.addAtom(
          atom.position,
          monomerToNewMonomer.get(atom.monomer),
          atom.atomIdInMicroMode,
          atom.label,
          atom.properties
        );
        const addedAtom = atomAddCommand.operations[0].atom;
        command.merge(atomAddCommand);
        mergedDrawingEntities.atoms.set(addedAtom.id, addedAtom);
        atomToNewAtom.set(atom, addedAtom);
      });
      this.bonds.forEach((bond) => {
        const newFirstAtom = atomToNewAtom.get(bond.firstAtom);
        const newSecondAtom = atomToNewAtom.get(bond.secondAtom);
        if (!newFirstAtom || !newSecondAtom) {
          return;
        }
        const bondAddCommand = targetDrawingEntitiesManager.addBond(
          newFirstAtom,
          newSecondAtom,
          bond.type,
          bond.stereo,
          bond.bondIdInMicroMode,
          bond.cip
        );
        const addedBond = bondAddCommand.operations[0].bond;
        command.merge(bondAddCommand);
        mergedDrawingEntities.bonds.set(addedBond.id, addedBond);
      });
      viewModel.initialize([...targetDrawingEntitiesManager.bonds.values()]);
      this.monomerToAtomBonds.forEach((monomerToAtomBond) => {
        const bondAddCommand = targetDrawingEntitiesManager.addMonomerToAtomBond(
          monomerToNewMonomer.get(monomerToAtomBond.monomer),
          atomToNewAtom.get(monomerToAtomBond.atom),
          monomerToAtomBond.monomer.getAttachmentPointByBond(
            monomerToAtomBond
          )
        );
        const addedBond = bondAddCommand.operations[0].monomerToAtomBond;
        command.merge(bondAddCommand);
        mergedDrawingEntities.monomerToAtomBonds.set(addedBond.id, addedBond);
      });
      this.rxnArrows.forEach((rxnArrow) => {
        const rxnArrowAddCommand = targetDrawingEntitiesManager.addRxnArrow(
          rxnArrow.type,
          rxnArrow.startEndPosition,
          rxnArrow.height,
          rxnArrow.initiallySelected,
          rxnArrow.arrowId
        );
        const addedRxnArrow = rxnArrowAddCommand.operations[0].rxnArrow;
        command.merge(rxnArrowAddCommand);
        mergedDrawingEntities.rxnArrows.set(addedRxnArrow.id, addedRxnArrow);
      });
      this.multitailArrows.forEach((multitailArrow) => {
        const arrowAddCommand = targetDrawingEntitiesManager.addMultitailArrow(
          multitailArrow.toKetNode(),
          multitailArrow.arrowId
        );
        const addedArrow = arrowAddCommand.operations[0].multitailArrow;
        command.merge(arrowAddCommand);
        mergedDrawingEntities.multitailArrows.set(addedArrow.id, addedArrow);
      });
      this.rxnPluses.forEach((rxnPlus) => {
        const plusAddCommand = targetDrawingEntitiesManager.addRxnPlus(
          rxnPlus.position
        );
        const addedPlus = plusAddCommand.operations[0].rxnPlus;
        command.merge(plusAddCommand);
        mergedDrawingEntities.rxnPluses.set(addedPlus.id, addedPlus);
      });
      this.micromoleculesHiddenEntities.mergeInto(
        targetDrawingEntitiesManager.micromoleculesHiddenEntities
      );
      return { command, mergedDrawingEntities };
    }
    filterSelection() {
      const filteredDrawingEntitiesManager = new _DrawingEntitiesManager();
      this.selectedEntities.forEach(([, entity]) => {
        var _a, _b;
        if (entity instanceof BaseMonomer) {
          filteredDrawingEntitiesManager.addMonomerChangeModel(
            entity.monomerItem,
            entity.position,
            entity
          );
        } else if (entity instanceof Atom) {
          filteredDrawingEntitiesManager.addMonomerChangeModel(
            entity.monomer.monomerItem,
            entity.monomer.position,
            entity.monomer
          );
        } else if (entity instanceof PolymerBond && entity.secondMonomer) {
          const firstAttachmentPoint = entity.firstMonomer.getAttachmentPointByBond(entity);
          const secondAttachmentPoint = (_a = entity.secondMonomer) == null ? void 0 : _a.getAttachmentPointByBond(entity);
          if (firstAttachmentPoint && secondAttachmentPoint && entity.firstMonomer.selected && ((_b = entity.secondMonomer) == null ? void 0 : _b.selected)) {
            filteredDrawingEntitiesManager.finishPolymerBondCreationModelChange(
              entity.firstMonomer,
              entity.secondMonomer,
              firstAttachmentPoint,
              secondAttachmentPoint,
              void 0,
              entity
            );
          }
        } else if (entity instanceof HydrogenBond && entity.secondMonomer) {
          filteredDrawingEntitiesManager.finishPolymerBondCreationModelChange(
            entity.firstMonomer,
            entity.secondMonomer,
            "hydrogen" /* HYDROGEN */,
            "hydrogen" /* HYDROGEN */,
            import_types15.MACROMOLECULES_BOND_TYPES.HYDROGEN,
            entity
          );
        } else if (entity instanceof MonomerToAtomBond) {
          filteredDrawingEntitiesManager.addMonomerToAtomBondChangeModel(
            entity.monomer,
            entity.atom,
            entity.monomer.getAttachmentPointByBond(
              entity
            ),
            entity
          );
        } else if (entity instanceof Bond) {
          filteredDrawingEntitiesManager.addBondChangeModel(
            entity.firstAtom,
            entity.secondAtom,
            entity.type,
            entity.stereo,
            entity.bondIdInMicroMode,
            entity,
            entity.cip
          );
        } else if (entity instanceof RxnArrow2) {
          filteredDrawingEntitiesManager.addRxnArrowModelChange(
            entity.type,
            entity.startEndPosition,
            entity.height,
            entity.initiallySelected,
            void 0,
            entity
          );
        } else if (entity instanceof MultitailArrow2) {
          filteredDrawingEntitiesManager.addMultitailArrowArrowModelChange(
            entity.toKetNode(),
            void 0,
            entity
          );
        } else if (entity instanceof RxnPlus2) {
          filteredDrawingEntitiesManager.addRxnPlusModelChange(
            entity.position,
            entity.initiallySelected,
            entity
          );
        }
      });
      return filteredDrawingEntitiesManager;
    }
    centerMacroStructure() {
      const centerPointOfModel = import_coordinates3.Coordinates.canvasToModel(
        this.getCurrentCenterPointOfCanvas()
      );
      const structCenter = this.getMacroStructureCenter();
      const offset = Vec2.diff(centerPointOfModel, structCenter);
      this.allEntities.forEach(([, entity]) => {
        this.moveDrawingEntityModelChange(entity, offset);
      });
    }
    getCurrentCenterPointOfCanvas() {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      const originalCenterPointOfCanvas = new Vec2(
        editor.canvasOffset.width / 2,
        editor.canvasOffset.height / 2
      );
      return import_coordinates3.Coordinates.viewToCanvas(originalCenterPointOfCanvas);
    }
    getMacroStructureCenter() {
      let xmin = 1e50;
      let ymin = xmin;
      let xmax = -xmin;
      let ymax = -ymin;
      this.monomers.forEach((monomer) => {
        xmin = Math.min(xmin, monomer.position.x);
        ymin = Math.min(ymin, monomer.position.y);
        xmax = Math.max(xmax, monomer.position.x);
        ymax = Math.max(ymax, monomer.position.y);
      });
      this.polymerBonds.forEach((bond) => {
        xmin = Math.min(xmin, bond.position.x);
        ymin = Math.min(ymin, bond.position.y);
        xmax = Math.max(xmax, bond.position.x);
        ymax = Math.max(ymax, bond.position.y);
      });
      return new Vec2((xmin + xmax) / 2, (ymin + ymax) / 2);
    }
    rerenderMolecules() {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      this.atoms.forEach((atom) => {
        editor.renderersContainer.deleteAtom(atom);
        editor.renderersContainer.addAtom(atom);
      });
      this.bonds.forEach((bond) => {
        editor.renderersContainer.deleteBond(bond);
        editor.renderersContainer.addBond(bond);
      });
      this.rxnArrows.forEach((rxnArrow) => {
        editor.renderersContainer.deleteRxnArrow(rxnArrow);
        editor.renderersContainer.addRxnArrow(rxnArrow);
      });
      this.multitailArrows.forEach((multitailArrow) => {
        editor.renderersContainer.deleteMultitailArrow(multitailArrow);
        editor.renderersContainer.addMultitailArrow(multitailArrow);
      });
      this.rxnPluses.forEach((rxnPlus) => {
        editor.renderersContainer.deleteRxnPlus(rxnPlus);
        editor.renderersContainer.addRxnPlus(rxnPlus);
      });
    }
    applyMonomersSequenceLayout() {
      const chainsCollection = ChainsCollection.fromMonomers([
        ...this.monomers.values()
      ]);
      chainsCollection.rearrange();
      this.rerenderMolecules();
      import_SequenceRenderer.SequenceRenderer.show(chainsCollection);
      return chainsCollection;
    }
    clearCanvas() {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      this.monomers.forEach((monomer) => {
        editor.renderersContainer.deleteMonomer(monomer);
      });
      this.polymerBonds.forEach((polymerBond) => {
        editor.renderersContainer.deletePolymerBond(polymerBond);
      });
      this.monomerToAtomBonds.forEach((monomerToAtomBond) => {
        editor.renderersContainer.deleteMonomerToAtomBond(monomerToAtomBond);
      });
      this.atoms.forEach((atom) => {
        editor.renderersContainer.deleteAtom(atom);
      });
      this.bonds.forEach((bond) => {
        editor.renderersContainer.deleteBond(bond);
      });
      this.rxnArrows.forEach((bond) => {
        editor.renderersContainer.deleteRxnArrow(bond);
      });
      this.multitailArrows.forEach((bond) => {
        editor.renderersContainer.deleteMultitailArrow(bond);
      });
      this.rxnPluses.forEach((rxnPlus) => {
        editor.renderersContainer.deleteRxnPlus(rxnPlus);
      });
      import_SequenceRenderer.SequenceRenderer.clear();
    }
    applyFlexLayoutMode(needRedrawBonds = false) {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      const command = new Command();
      if (needRedrawBonds) {
        command.merge(this.redrawBonds());
      }
      this.detectBondsOverlappedByMonomers();
      this.monomers.forEach((monomer) => {
        editor.renderersContainer.deleteMonomer(monomer);
        editor.renderersContainer.addMonomer(monomer);
      });
      this.polymerBonds.forEach((polymerBond) => {
        editor.renderersContainer.deletePolymerBond(polymerBond);
        editor.renderersContainer.addPolymerBond(polymerBond);
      });
      this.rerenderMolecules();
      this.monomerToAtomBonds.forEach((monomerToAtomBond) => {
        editor.renderersContainer.deleteMonomerToAtomBond(monomerToAtomBond);
        editor.renderersContainer.addMonomerToAtomBond(monomerToAtomBond);
      });
      return command;
    }
    rerenderBondsOverlappedByMonomers() {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      if (editor.mode.modeName === "sequence-layout-mode") {
        return;
      }
      const monomersToCheck = this.selectedEntities.filter(([, entity]) => entity instanceof BaseMonomer).map(([, entity]) => entity);
      const outstandingBonds = this.polymerBondsArray.filter(
        (polymerBond) => monomersToCheck.some(
          (monomer) => polymerBond.firstMonomer !== monomer && polymerBond.secondMonomer !== monomer
        )
      );
      outstandingBonds.forEach((polymerBond) => {
        const previousIsOverlappedByMonomer = polymerBond.isOverlappedByMonomer;
        polymerBond.isOverlappedByMonomer = this.checkBondForOverlapsByMonomers(
          polymerBond,
          monomersToCheck
        );
        if (polymerBond.isOverlappedByMonomer !== previousIsOverlappedByMonomer) {
          editor.renderersContainer.deletePolymerBond(polymerBond, false, false);
          editor.renderersContainer.addPolymerBond(polymerBond, false);
        }
      });
    }
    getAllSelectedEntitiesForEntities(drawingEntities) {
      const command = new Command();
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      editor.events.selectEntities.dispatch(drawingEntities);
      drawingEntities.forEach((monomer) => monomer.turnOnSelection());
      const newDrawingEntities = drawingEntities.reduce(
        (selectedDrawingEntities, drawingEntity) => {
          const res = editor.drawingEntitiesManager.getAllSelectedEntitiesForSingleEntity(
            drawingEntity,
            true,
            selectedDrawingEntities
          );
          res.drawingEntities.forEach(
            (entity) => command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(entity))
          );
          return selectedDrawingEntities.concat(res.drawingEntities);
        },
        []
      );
      return { command, drawingEntities: newDrawingEntities };
    }
    getAllSelectedEntitiesForSingleEntity(drawingEntity, needToSelectConnectedBonds = true, selectedDrawingEntities) {
      const command = new Command();
      command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(drawingEntity));
      drawingEntity.turnOnSelection();
      let drawingEntities = [drawingEntity];
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      if (editor.mode.modeName !== "sequence-layout-mode" || drawingEntity instanceof PolymerBond) {
        return { command, drawingEntities };
      }
      if (drawingEntity instanceof Sugar && drawingEntity.isPartOfRNA) {
        const sugar = drawingEntity;
        if (isValidNucleoside(sugar)) {
          const nucleoside = Nucleoside.fromSugar(sugar);
          drawingEntities = nucleoside.monomers;
        } else if (isValidNucleotide(sugar)) {
          const nucleotide = Nucleotide.fromSugar(sugar);
          drawingEntities = nucleotide.monomers;
        }
        drawingEntities.forEach((entity) => {
          if (!(entity instanceof Sugar)) {
            entity.turnOnSelection();
            command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(entity));
          }
        });
      }
      drawingEntities.forEach((entity) => {
        const monomer = entity;
        if (needToSelectConnectedBonds && monomer.hasBonds) {
          monomer.forEachBond((polymerBond) => {
            var _a;
            if (!(selectedDrawingEntities == null ? void 0 : selectedDrawingEntities.includes(polymerBond)) && !drawingEntities.includes(polymerBond) && polymerBond instanceof PolymerBond && ((_a = polymerBond.getAnotherMonomer(monomer)) == null ? void 0 : _a.selected)) {
              drawingEntities.push(polymerBond);
              polymerBond.turnOnSelection();
              command.addOperation(new import_drawingEntity.DrawingEntitySelectOperation(polymerBond));
            }
          });
        }
      });
      return { command, drawingEntities };
    }
    validateIfApplicableForFasta() {
      if (this.monomers.size === 0 && this.micromoleculesHiddenEntities.atoms.size > 0) {
        return false;
      }
      const monomerTypes = /* @__PURE__ */ new Set();
      let isValid = true;
      this.monomers.forEach((monomer) => {
        let monomerType = monomer.monomerItem.props.MonomerType;
        if (monomer instanceof AmbiguousMonomer) {
          monomerType = monomer.monomerClass === "CHEM" /* CHEM */ ? MONOMER_CONST.CHEM : monomer.monomers[0].monomerItem.props.MonomerType;
        }
        monomerTypes.add(monomerType);
        if (monomerType === MONOMER_CONST.CHEM || monomerTypes.size > 1) {
          isValid = false;
        }
      });
      return isValid;
    }
    moveMonomer(monomer, position) {
      const oldMonomerPosition = monomer.position;
      const command = new Command();
      const operation = new import_monomer.MonomerMoveOperation(
        this.rearrangeChainModelChange.bind(this, monomer, position),
        this.rearrangeChainModelChange.bind(this, monomer, oldMonomerPosition)
      );
      command.addOperation(operation);
      return command;
    }
    removeHoverForAllMonomers() {
      const command = new Command();
      this.monomers.forEach((monomer) => {
        if (!monomer.hovered) {
          return;
        }
        monomer.turnOffHover();
        monomer.turnOffAttachmentPointsVisibility();
        command.addOperation(new import_monomer.MonomerHoverOperation(monomer, true));
      });
      return command;
    }
    reconnectPolymerBondModelChange(polymerBond, {
      newFirstMonomerAttachmentPoint,
      newSecondMonomerAttachmentPoint,
      initialFirstMonomerAttachmentPoint,
      initialSecondMonomerAttachmentPoint
    }) {
      var _a, _b;
      polymerBond.firstMonomer.unsetBond(initialFirstMonomerAttachmentPoint);
      (_a = polymerBond.secondMonomer) == null ? void 0 : _a.unsetBond(initialSecondMonomerAttachmentPoint);
      polymerBond.firstMonomer.setBond(
        newFirstMonomerAttachmentPoint,
        polymerBond
      );
      (_b = polymerBond.secondMonomer) == null ? void 0 : _b.setBond(
        newSecondMonomerAttachmentPoint,
        polymerBond
      );
      return polymerBond;
    }
    reconnectPolymerBond(polymerBond, newFirstMonomerAttachmentPoint, newSecondMonomerAttachmentPoint, initialFirstMonomerAttachmentPoint, initialSecondMonomerAttachmentPoint) {
      const command = new Command();
      command.addOperation(
        new import_polymerBond.ReconnectPolymerBondOperation(
          this.reconnectPolymerBondModelChange.bind(this, polymerBond, {
            newFirstMonomerAttachmentPoint,
            newSecondMonomerAttachmentPoint,
            initialFirstMonomerAttachmentPoint,
            initialSecondMonomerAttachmentPoint
          }),
          this.reconnectPolymerBondModelChange.bind(this, polymerBond, {
            newFirstMonomerAttachmentPoint: initialFirstMonomerAttachmentPoint,
            newSecondMonomerAttachmentPoint: initialSecondMonomerAttachmentPoint,
            initialFirstMonomerAttachmentPoint: newFirstMonomerAttachmentPoint,
            initialSecondMonomerAttachmentPoint: newSecondMonomerAttachmentPoint
          })
        )
      );
      return command;
    }
    addAmbiguousMonomerChangeModel(variantMonomerItem, position, _monomer) {
      if (_monomer) {
        this.monomers.set(_monomer.id, _monomer);
        return _monomer;
      }
      const monomer = new AmbiguousMonomer(variantMonomerItem, position);
      this.monomers.set(monomer.id, monomer);
      return monomer;
    }
    addAmbiguousMonomer(ambiguousMonomerItem, position) {
      const command = new Command();
      const operation = new import_monomer.MonomerAddOperation(
        this.addAmbiguousMonomerChangeModel.bind(
          this,
          ambiguousMonomerItem,
          position
        ),
        this.deleteMonomerChangeModel.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    addAtomChangeModel(position, monomer, atomIdInMicroMode, label, properties, _atom) {
      if (_atom) {
        this.atoms.set(_atom.id, _atom);
        return _atom;
      }
      const atom = new Atom(
        position,
        monomer,
        atomIdInMicroMode,
        label,
        properties
      );
      this.atoms.set(atom.id, atom);
      return atom;
    }
    addAtom(position, monomer, atomIdInMicroMode, label, properties) {
      const command = new Command();
      const atomAddOperation = new import_atom5.AtomAddOperation(
        (atom) => this.addAtomChangeModel(
          position,
          monomer,
          atomIdInMicroMode,
          label,
          properties,
          atom
        ),
        this.deleteAtomChangeModel.bind(this)
      );
      command.addOperation(atomAddOperation);
      return command;
    }
    deleteAtomChangeModel(atom) {
      this.atoms.delete(atom.id);
      return atom;
    }
    deleteAtom(atom, needToDeleteConnectedEntities = true) {
      const command = new Command();
      command.addOperation(
        new import_atom5.AtomDeleteOperation(
          atom,
          this.deleteAtomChangeModel.bind(this, atom),
          () => this.addAtomChangeModel(
            atom.position,
            atom.monomer,
            atom.atomIdInMicroMode,
            atom.label,
            atom.properties,
            atom
          )
        )
      );
      if (needToDeleteConnectedEntities) {
        atom.bonds.forEach((bond) => {
          if (bond.selected) {
            return;
          }
          if (bond instanceof Bond) {
            command.merge(this.deleteBond(bond));
          }
        });
        this.monomerToAtomBonds.forEach((monomerToAtomBond) => {
          if (monomerToAtomBond.atom === atom && !monomerToAtomBond.selected) {
            command.merge(this.deleteMonomerToAtomBond(monomerToAtomBond));
          }
        });
      }
      return command;
    }
    addBondChangeModel(firstAtom, secondAtom, type, stereo, bondIdInMicroMode, _bond, cip) {
      if (_bond) {
        this.bonds.set(_bond.id, _bond);
        return _bond;
      }
      const bond = new Bond(
        firstAtom,
        secondAtom,
        bondIdInMicroMode,
        type,
        stereo,
        cip
      );
      this.bonds.set(bond.id, bond);
      firstAtom.addBond(bond);
      secondAtom.addBond(bond);
      return bond;
    }
    addBond(firstAtom, secondAtom, type, stereo, bondIdInMicroMode, cip) {
      const command = new Command();
      const bondAddOperation = new import_bond6.BondAddOperation(
        (bond) => this.addBondChangeModel(
          firstAtom,
          secondAtom,
          type,
          stereo,
          bondIdInMicroMode,
          bond,
          cip
        ),
        (bond) => this.deleteBondChangeModel(bond)
      );
      command.addOperation(bondAddOperation);
      return command;
    }
    deleteBondChangeModel(bond) {
      this.bonds.delete(bond.id);
      const firstAtom = bond.firstAtom;
      const secondAtom = bond.secondAtom;
      [firstAtom, secondAtom].forEach((atom) => {
        atom.deleteBond(bond.id);
      });
      return bond;
    }
    deleteBond(bond, needToDeleteDisconnectedAtoms = true) {
      const command = new Command();
      command.addOperation(
        new import_bond6.BondDeleteOperation(
          bond,
          this.deleteBondChangeModel.bind(this, bond),
          (bond2) => this.addBondChangeModel(
            bond2.firstAtom,
            bond2.secondAtom,
            bond2.type,
            bond2.stereo,
            bond2.bondIdInMicroMode,
            bond2
          )
        )
      );
      const firstAtom = bond.firstAtom;
      const secondAtom = bond.secondAtom;
      [firstAtom, secondAtom].forEach((atom) => {
        atom.deleteBond(bond.id);
        if (!needToDeleteDisconnectedAtoms || !atom.bonds.every((atomBond) => atomBond instanceof MonomerToAtomBond)) {
          return;
        }
        this.monomerToAtomBonds.forEach((monomerToAtomBond) => {
          if (monomerToAtomBond.atom !== atom || monomerToAtomBond.selected) {
            return;
          }
          command.merge(this.deleteAtom(atom, true));
        });
      });
      return command;
    }
    addMonomerToAtomBondChangeModel(monomer, atom, attachmentPoint, _monomerToAtomBond) {
      if (_monomerToAtomBond) {
        this.monomerToAtomBonds.set(_monomerToAtomBond.id, _monomerToAtomBond);
        monomer.setBond(attachmentPoint, _monomerToAtomBond);
        atom.addBond(_monomerToAtomBond);
        return _monomerToAtomBond;
      }
      const monomerToAtomBond = new MonomerToAtomBond(monomer, atom);
      atom.addBond(monomerToAtomBond);
      this.monomerToAtomBonds.set(monomerToAtomBond.id, monomerToAtomBond);
      monomerToAtomBond.moveToLinkedEntities();
      monomer.setBond(attachmentPoint, monomerToAtomBond);
      monomer.turnOffAttachmentPointsVisibility();
      monomer.turnOffHover();
      return monomerToAtomBond;
    }
    deleteMonomerToAtomBondChangeModel(monomerAtomBond) {
      const attachmentPointName = monomerAtomBond.monomer.getAttachmentPointByBond(monomerAtomBond);
      if (attachmentPointName) {
        monomerAtomBond.monomer.unsetBond(attachmentPointName);
      }
      this.monomerToAtomBonds.delete(monomerAtomBond.id);
      monomerAtomBond.atom.deleteBond(monomerAtomBond.id);
      return monomerAtomBond;
    }
    deleteMonomerToAtomBond(monomerAtomBond) {
      const command = new Command();
      command.addOperation(
        new import_monomerToAtomBond.MonomerToAtomBondDeleteOperation(
          monomerAtomBond,
          this.deleteMonomerToAtomBondChangeModel.bind(this, monomerAtomBond),
          this.addMonomerToAtomBondChangeModel.bind(
            this,
            monomerAtomBond.monomer,
            monomerAtomBond.atom,
            monomerAtomBond.monomer.getAttachmentPointByBond(
              monomerAtomBond
            )
          )
        )
      );
      return command;
    }
    addMonomerToAtomBond(monomer, atom, attachmentPoint) {
      const command = new Command();
      const monomerAddToAtomBondOperation = new import_monomerToAtomBond.MonomerToAtomBondAddOperation(
        this.addMonomerToAtomBondChangeModel.bind(
          this,
          monomer,
          atom,
          attachmentPoint
        ),
        this.deleteMonomerToAtomBondChangeModel.bind(this)
      );
      command.addOperation(monomerAddToAtomBondOperation);
      return command;
    }
    // TODO create separate class for BoundingBox
    static getStructureBbox(drawingEntities) {
      let left = 0;
      let right = 0;
      let top = 0;
      let bottom = 0;
      drawingEntities.forEach((drawingEntity) => {
        const monomerPosition = drawingEntity.position;
        left = left ? Math.min(left, monomerPosition.x) : monomerPosition.x;
        right = right ? Math.max(right, monomerPosition.x) : monomerPosition.x;
        top = top ? Math.min(top, monomerPosition.y) : monomerPosition.y;
        bottom = bottom ? Math.max(bottom, monomerPosition.y) : monomerPosition.y;
      });
      return {
        left,
        right,
        top,
        bottom,
        width: right - left,
        height: bottom - top
      };
    }
    static antisenseChainBasesMap(isDnaAntisense) {
      const antisenseMap = {
        ["A" /* ADENINE */]: "U" /* URACIL */,
        ["C" /* CYTOSINE */]: "G" /* GUANINE */,
        ["G" /* GUANINE */]: "C" /* CYTOSINE */,
        ["T" /* THYMINE */]: "A" /* ADENINE */,
        ["U" /* URACIL */]: "A" /* ADENINE */,
        ["N" /* N */]: "N" /* N */,
        ["B" /* B */]: "V" /* V */,
        ["D" /* D */]: "H" /* H */,
        ["H" /* H */]: "D" /* D */,
        ["K" /* K */]: "M" /* M */,
        ["W" /* W */]: "W" /* W */,
        ["Y" /* Y */]: "R" /* R */,
        ["M" /* M */]: "K" /* K */,
        ["R" /* R */]: "Y" /* Y */,
        ["S" /* S */]: "S" /* S */,
        ["V" /* V */]: "B" /* B */
      };
      if (isDnaAntisense) {
        antisenseMap["A" /* ADENINE */] = "T" /* THYMINE */;
      }
      return antisenseMap;
    }
    markMonomerAsAntisense(monomer) {
      const command = new Command();
      command.merge(
        this.modifyMonomerItem(monomer, __spreadProps(__spreadValues({}, monomer.monomerItem), {
          isSense: false,
          isAntisense: true
        }))
      );
      return command;
    }
    markMonomerAsSense(monomer) {
      const command = new Command();
      command.merge(
        this.modifyMonomerItem(monomer, __spreadProps(__spreadValues({}, monomer.monomerItem), {
          isSense: true,
          isAntisense: false
        }))
      );
      return command;
    }
    recalculateAntisenseChains(needRecalculateOldAntisense = true) {
      const command = new Command();
      const chainsCollection = ChainsCollection.fromMonomers([
        ...this.monomers.values()
      ]);
      const handledChains = /* @__PURE__ */ new Set();
      if (needRecalculateOldAntisense) {
        this.monomers.forEach((monomer) => {
          command.merge(
            this.modifyMonomerItem(monomer, __spreadProps(__spreadValues({}, monomer.monomerItem), {
              isAntisense: false,
              isSense: false
            }))
          );
        });
        this.antisenseMonomerToSenseChain = /* @__PURE__ */ new Map();
      }
      chainsCollection.chains.forEach((chain) => {
        if (handledChains.has(chain)) {
          return;
        }
        if (!needRecalculateOldAntisense) {
          const isAntisenseChain = chain.monomers.some(
            (monomer) => monomer.monomerItem.isAntisense
          );
          const isSenseChain = chain.monomers.some(
            (monomer) => monomer.monomerItem.isSense
          );
          if (isSenseChain) {
            chain.monomers.forEach((monomer) => {
              command.merge(this.markMonomerAsSense(monomer));
            });
            return;
          }
          if (isAntisenseChain) {
            chain.monomers.forEach((monomer) => {
              command.merge(this.markMonomerAsAntisense(monomer));
            });
            return;
          }
        }
        let senseChain;
        const chainsToCheck = chainsCollection.getAllChainsWithConnectionInBlock(chain);
        const chainToMonomers = /* @__PURE__ */ new Map();
        chainsToCheck.forEach((chainToCheck) => {
          chainToMonomers.set(chainToCheck, chainToCheck.chain.monomers);
        });
        const largestChainsMonomersAmount = Math.max(
          ...[...chainToMonomers.values()].map((monomers) => monomers.length)
        );
        const largestChains = [...chainToMonomers.entries()].filter(
          ([, monomers]) => monomers.length === largestChainsMonomersAmount
        );
        if (largestChains.length === 1) {
          senseChain = largestChains[0][0];
        } else {
          const chainsToCenters = /* @__PURE__ */ new Map();
          const chainsToComplimentaryChainsAmount = /* @__PURE__ */ new Map();
          largestChains.forEach(([chainToCheck]) => {
            const complimentayChains = chainsCollection.getComplimentaryChainsWithData(chainToCheck.chain);
            chainsToComplimentaryChainsAmount.set(
              chainToCheck,
              complimentayChains.length
            );
          });
          largestChains.forEach(([chainToCheck, monomers]) => {
            const chainBbox = _DrawingEntitiesManager.getStructureBbox(monomers);
            chainsToCenters.set(
              chainToCheck,
              new Vec2(
                chainBbox.left + chainBbox.width / 2,
                chainBbox.top + chainBbox.height / 2
              )
            );
          });
          const chainsToCenterArray = [...chainsToCenters.entries()];
          const chainWithLowestCenter = chainsToCenterArray.reduce(
            ([previousChain, previousChainCenter], [chainToCheck, center]) => {
              return center.y < previousChainCenter.y ? [chainToCheck, center] : [previousChain, previousChainCenter];
            },
            chainsToCenterArray[0]
          );
          const chainsToComplimentaryChainsAmountArray = [
            ...chainsToComplimentaryChainsAmount.entries()
          ];
          const chainWithMoreComplimentaryChains = chainsToComplimentaryChainsAmountArray.reduce(
            ([previousChain, previousChainComplimentaryChainsAmount], [chainToCheck, complimentaryChainsAmount]) => {
              return complimentaryChainsAmount > previousChainComplimentaryChainsAmount ? [chainToCheck, complimentaryChainsAmount] : [previousChain, previousChainComplimentaryChainsAmount];
            },
            chainsToComplimentaryChainsAmountArray[0]
          );
          senseChain = chainsToComplimentaryChainsAmount.size === 1 ? chainWithMoreComplimentaryChains[0] : chainWithLowestCenter[0];
        }
        const { group: senseGroup } = senseChain;
        chainsToCheck.forEach(({ chain: chain2, group }) => {
          handledChains.add(chain2);
          if (group === senseGroup) {
            chain2.monomers.forEach((monomer) => {
              command.merge(this.markMonomerAsSense(monomer));
            });
          } else {
            chain2.monomers.forEach((monomer) => {
              command.merge(this.markMonomerAsAntisense(monomer));
              this.antisenseMonomerToSenseChain.set(monomer, senseChain.chain);
            });
          }
        });
      });
      return command;
    }
    get hasAntisenseChains() {
      return [...this.monomers.values()].some(
        (monomer) => monomer.monomerItem.isAntisense
      );
    }
    static getAntisenseBaseLabel(rnaBaseMonomerOrLabel, isDnaAntisense) {
      let baseLabelKey;
      if (typeof rnaBaseMonomerOrLabel === "string") {
        baseLabelKey = rnaBaseMonomerOrLabel;
      } else if (rnaBaseMonomerOrLabel instanceof AmbiguousMonomer) {
        baseLabelKey = rnaBaseMonomerOrLabel.monomerItem.label;
      } else {
        baseLabelKey = rnaBaseMonomerOrLabel.monomerItem.props.MonomerNaturalAnalogCode;
      }
      return _DrawingEntitiesManager.antisenseChainBasesMap(isDnaAntisense)[baseLabelKey];
    }
    static createAntisenseNode(node, isDnaAntisense, needAddPhosphate = false) {
      const antisenseBaseLabel = _DrawingEntitiesManager.getAntisenseBaseLabel(
        node.rnaBase,
        isDnaAntisense
      );
      if (!antisenseBaseLabel) {
        return;
      }
      const sugarName = isDnaAntisense ? "dR" /* SUGAR_DNA */ : "R" /* SUGAR_RNA */;
      return (needAddPhosphate ? Nucleotide : Nucleoside).createOnCanvas(
        antisenseBaseLabel,
        node.monomer.position.add(new Vec2(0, 3)),
        sugarName
      );
    }
    createAntisenseChain(isDnaAntisense) {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      const command = new Command();
      const selectedMonomers = this.selectedEntities.filter(([, drawingEntity]) => drawingEntity instanceof BaseMonomer).map(([, monomer]) => monomer);
      const chainsCollection = ChainsCollection.fromMonomers(selectedMonomers);
      const chainsForAntisenseCreation = chainsCollection.chains.filter(
        (chain) => {
          return chain.subChains.some(
            (subChain) => subChain.nodes.some(
              (node) => (node instanceof Nucleotide || node instanceof Nucleoside) && Boolean(
                _DrawingEntitiesManager.getAntisenseBaseLabel(
                  node.rnaBase,
                  isDnaAntisense
                )
              ) && node.monomer.selected
            )
          );
        }
      );
      const selectedPiecesInChains = [];
      chainsForAntisenseCreation.forEach((chain) => {
        let selectedPiece = [];
        let hasRnaInPiece = false;
        chain.nodes.forEach((node) => {
          const hasSelectedMonomerInNode = node.monomers.some(
            (monomer) => monomer.selected
          );
          if (!hasSelectedMonomerInNode) {
            if (hasRnaInPiece) {
              selectedPiecesInChains.push(selectedPiece);
            }
            selectedPiece = [];
            hasRnaInPiece = false;
          } else {
            selectedPiece.push(node);
          }
          if (node instanceof Nucleoside || node instanceof Nucleotide) {
            hasRnaInPiece = true;
          }
        });
        if (hasRnaInPiece) {
          selectedPiecesInChains.push(selectedPiece);
        }
        selectedPiece = [];
        hasRnaInPiece = false;
      });
      let lastAddedNode;
      let lastAddedMonomer;
      selectedPiecesInChains.forEach((selectedPiece) => {
        [...selectedPiece].reverse().forEach((nodeToHandle) => {
          const senseNode = nodeToHandle instanceof Nucleotide && nodeToHandle.phosphate.selected && !nodeToHandle.monomer.selected ? new MonomerSequenceNode(nodeToHandle.phosphate) : nodeToHandle;
          if (!senseNode.monomer.selected) {
            lastAddedMonomer = void 0;
            lastAddedNode = void 0;
            return;
          }
          if (senseNode instanceof Nucleotide || senseNode instanceof Nucleoside) {
            const antisenseNodeCreationResult = _DrawingEntitiesManager.createAntisenseNode(
              senseNode,
              isDnaAntisense,
              false
            );
            if (!antisenseNodeCreationResult) {
              return;
            }
            const { modelChanges: addNucleotideCommand, node: addedNode } = antisenseNodeCreationResult;
            command.merge(addNucleotideCommand);
            let addedPhosphate;
            if (senseNode instanceof Nucleotide && senseNode.phosphate.selected) {
              const phosphateLibraryItem = getRnaPartLibraryItem(
                editor,
                "P" /* PHOSPHATE */
              );
              if (!phosphateLibraryItem) {
                import_utilities4.sketchLogger.warn(
                  "Phosphate is not found in monomers library. Skipping phosphate addition."
                );
                return;
              }
              const monomerAddCommand = this.addMonomer(
                phosphateLibraryItem,
                senseNode.phosphate.position.add(new Vec2(0, 3))
              );
              addedPhosphate = monomerAddCommand.operations[0].monomer;
              command.merge(monomerAddCommand);
              command.merge(
                this.createPolymerBond(
                  addedPhosphate,
                  addedNode.firstMonomerInNode,
                  "R2" /* R2 */,
                  "R1" /* R1 */
                )
              );
            }
            if (lastAddedNode) {
              command.merge(
                this.createPolymerBond(
                  lastAddedMonomer || lastAddedNode.lastMonomerInNode,
                  addedPhosphate || addedNode.firstMonomerInNode,
                  "R2" /* R2 */,
                  "R1" /* R1 */
                )
              );
            }
            command.merge(
              this.createPolymerBond(
                senseNode.rnaBase,
                addedNode.rnaBase,
                "hydrogen" /* HYDROGEN */,
                "hydrogen" /* HYDROGEN */,
                import_types15.MACROMOLECULES_BOND_TYPES.HYDROGEN
              )
            );
            lastAddedMonomer = void 0;
            lastAddedNode = addedNode;
          } else {
            lastAddedMonomer = lastAddedMonomer || (lastAddedNode == null ? void 0 : lastAddedNode.lastMonomerInNode);
            [...senseNode.monomers].reverse().forEach((monomer) => {
              if (!monomer.selected) {
                lastAddedMonomer = void 0;
                lastAddedNode = void 0;
                return;
              }
              if (!monomer.hasAttachmentPoint("R2" /* R2 */)) {
                editor.events.error.dispatch(
                  `Monomer ${monomer.label} does not have attachment point R2. Antisense was not created for this monomer.`
                );
                return;
              }
              if (lastAddedMonomer && !lastAddedMonomer.hasAttachmentPoint("R1" /* R1 */)) {
                editor.events.error.dispatch(
                  `Monomer ${lastAddedMonomer.label} does not have attachment point R1. Antisense was not created for this monomer.`
                );
                return;
              }
              const isModifiedPhosphate = monomer instanceof Phosphate && monomer.isModification;
              const isAmbiguousMonomer = monomer instanceof AmbiguousMonomer;
              let antisenseMonomerItem = monomer.monomerItem;
              if (isModifiedPhosphate || isAmbiguousMonomer) {
                const nonModifiedPhosphateItem = getRnaPartLibraryItem(
                  editor,
                  "P" /* PHOSPHATE */
                );
                if (nonModifiedPhosphateItem) {
                  antisenseMonomerItem = nonModifiedPhosphateItem;
                } else if (isAmbiguousMonomer) {
                  antisenseMonomerItem = monomer.variantMonomerItem;
                }
              }
              const monomerAddCommand = this.addMonomer(
                antisenseMonomerItem,
                monomer.position.add(new Vec2(0, 4.25))
              );
              const addedMonomer = monomerAddCommand.operations[0].monomer;
              command.merge(monomerAddCommand);
              if (lastAddedMonomer) {
                command.merge(
                  this.createPolymerBond(
                    lastAddedMonomer,
                    addedMonomer,
                    "R2" /* R2 */,
                    "R1" /* R1 */
                  )
                );
              }
              lastAddedNode = senseNode;
              lastAddedMonomer = addedMonomer;
            });
          }
        });
        lastAddedNode = void 0;
        lastAddedMonomer = void 0;
      });
      command.merge(this.applySnakeLayout(true, true));
      if (editor.mode.modeName === "sequence-layout-mode") {
        command.addOperation(new import_modes.ReinitializeModeOperation());
      }
      command.setUndoOperationsByPriority();
      return command;
    }
    get monomersArray() {
      return [...this.monomers.values()];
    }
    get polymerBondsArray() {
      return [...this.polymerBonds.values()];
    }
    get molecules() {
      return this.monomersArray.filter((monomer) => {
        return monomer.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(monomer);
      });
    }
    checkBondForOverlapsByMonomers(polymerBond, monomers) {
      const editor = (0, import_editorSingleton2.provideEditorInstance)();
      if (!editor || editor.mode.modeName === "sequence-layout-mode") {
        return false;
      }
      const secondMonomer = polymerBond.secondMonomer;
      if (!secondMonomer) {
        return false;
      }
      if (!polymerBond.isHorizontal && !polymerBond.isVertical) {
        return false;
      }
      const monomersToUse = monomers != null ? monomers : this.monomersArray;
      if (monomersToUse.length > 500) {
        return false;
      }
      const previousOverlap = this.bondsMonomersOverlaps.get(polymerBond.id);
      const monomersToUseWithPreviousOverlap = previousOverlap ? [previousOverlap, ...monomersToUse] : monomersToUse;
      const overlappingMonomer = monomersToUseWithPreviousOverlap.find(
        (monomer) => {
          if (monomer.id === polymerBond.firstMonomer.id || monomer.id === secondMonomer.id) {
            return false;
          }
          const distanceFromMonomerToLine = monomer.center.calculateDistanceToLine([
            polymerBond.firstMonomer.center,
            secondMonomer.center
          ]);
          return distanceFromMonomerToLine < HalfMonomerSize;
        }
      );
      if (overlappingMonomer) {
        this.bondsMonomersOverlaps.set(polymerBond.id, overlappingMonomer);
      }
      return Boolean(overlappingMonomer);
    }
    detectBondsOverlappedByMonomers(polymerBonds) {
      const bondsToCheck = polymerBonds != null ? polymerBonds : this.polymerBondsArray;
      bondsToCheck.forEach((polymerBond) => {
        polymerBond.isOverlappedByMonomer = this.checkBondForOverlapsByMonomers(polymerBond);
      });
    }
    deleteRxnArrowModelChange(rxnArrow) {
      this.rxnArrows.delete(rxnArrow.id);
    }
    addRxnArrowModelChange(type, position, height, initiallySelected, arrowId, _arrow) {
      if (_arrow) {
        this.ensureArrowId(_arrow);
        this.rxnArrows.set(_arrow.id, _arrow);
        return _arrow;
      }
      const rxnArrow = new RxnArrow2(
        type,
        position,
        height,
        _DrawingEntitiesManager.normalizeInitiallySelected(initiallySelected)
      );
      rxnArrow.arrowId = arrowId;
      this.ensureArrowId(rxnArrow);
      this.rxnArrows.set(rxnArrow.id, rxnArrow);
      return rxnArrow;
    }
    addRxnArrow(type, position, height, initiallySelected, arrowId) {
      const command = new Command();
      const operation = new import_rxnArrow.RxnArrowAddOperation(
        (arrow) => this.addRxnArrowModelChange(
          type,
          position,
          height,
          initiallySelected,
          arrowId,
          arrow
        ),
        this.deleteRxnArrowModelChange.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    deleteRxnArrow(rxnArrow) {
      const command = new Command();
      const operation = new import_rxnArrow.RxnArrowDeleteOperation(
        rxnArrow,
        this.deleteRxnArrowModelChange.bind(this),
        (arrow) => this.addRxnArrowModelChange(
          rxnArrow.type,
          rxnArrow.startEndPosition,
          rxnArrow.height,
          rxnArrow.initiallySelected,
          rxnArrow.arrowId,
          arrow
        )
      );
      command.addOperation(operation);
      return command;
    }
    deleteMultitailArrowModelChange(multitailArrow) {
      this.multitailArrows.delete(multitailArrow.id);
    }
    addMultitailArrowArrowModelChange(multitailArrowKetNode, arrowId, _arrow) {
      if (_arrow) {
        this.ensureArrowId(_arrow);
        this.multitailArrows.set(_arrow.id, _arrow);
        return _arrow;
      }
      const multitailArrow = MultitailArrow2.fromKet(multitailArrowKetNode);
      multitailArrow.arrowId = arrowId;
      this.ensureArrowId(multitailArrow);
      this.multitailArrows.set(multitailArrow.id, multitailArrow);
      return multitailArrow;
    }
    addMultitailArrow(multitailArrowKetNode, arrowId) {
      const command = new Command();
      const operation = new import_multitailArrow2.MultitailArrowAddOperation(
        (arrow) => this.addMultitailArrowArrowModelChange(
          multitailArrowKetNode,
          arrowId,
          arrow
        ),
        this.deleteMultitailArrowModelChange.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    deleteMultitailArrow(multitailArrow) {
      const command = new Command();
      const operation = new import_multitailArrow2.MultitailArrowDeleteOperation(
        multitailArrow,
        this.deleteMultitailArrowModelChange.bind(this),
        (arrow) => this.addMultitailArrowArrowModelChange(
          multitailArrow.toKetNode(),
          multitailArrow.arrowId,
          arrow
        )
      );
      command.addOperation(operation);
      return command;
    }
    deleteRxnPlusModelChange(rxnPlus) {
      this.rxnPluses.delete(rxnPlus.id);
    }
    addRxnPlusModelChange(position, initiallySelected, _rxnPlus) {
      if (_rxnPlus) {
        this.rxnPluses.set(_rxnPlus.id, _rxnPlus);
        return _rxnPlus;
      }
      const rxnPlus = new RxnPlus2(
        position,
        _DrawingEntitiesManager.normalizeInitiallySelected(initiallySelected)
      );
      this.rxnPluses.set(rxnPlus.id, rxnPlus);
      return rxnPlus;
    }
    addRxnPlus(position, initiallySelected) {
      const command = new Command();
      const operation = new import_rxnPlus.RxnPlusAddOperation(
        this.addRxnPlusModelChange.bind(this, position, initiallySelected),
        this.deleteRxnPlusModelChange.bind(this)
      );
      command.addOperation(operation);
      return command;
    }
    deleteRxnPlus(rxnPlus) {
      const command = new Command();
      const operation = new import_rxnPlus.RxnPlusDeleteOperation(
        rxnPlus,
        this.deleteRxnPlusModelChange.bind(this),
        this.addRxnPlusModelChange.bind(
          this,
          rxnPlus.position,
          rxnPlus.initiallySelected
        )
      );
      command.addOperation(operation);
      return command;
    }
    selectAllConnectedEntities(startEntity) {
      const command = new Command();
      const process = (entity) => {
        entity.selected = true;
        command.merge(this.createDrawingEntitySelectionCommand(entity));
      };
      this.visitAllConnectedEntities(startEntity, process);
      return command;
    }
    visitAllConnectedEntities(startEntity, process) {
      const queue = [startEntity];
      const visited = /* @__PURE__ */ new Set();
      while (queue.length > 0) {
        const current = queue.shift();
        if (!current || visited.has(current.id)) continue;
        process(current);
        visited.add(current.id);
        if (current instanceof BaseMonomer) {
          queue.push(...current.hydrogenBonds, ...current.bonds);
        } else if (current instanceof HydrogenBond) {
          queue.push(
            current.firstEndEntity,
            ...current.secondEndEntity ? [current.secondEndEntity] : []
          );
        } else if (current instanceof PolymerBond) {
          queue.push(current.firstMonomer);
          if (current.secondMonomer) queue.push(current.secondMonomer);
        } else if (current instanceof MonomerToAtomBond) {
          queue.push(current.monomer, current.atom);
        } else if (current instanceof Bond) {
          queue.push(current.firstAtom, current.secondAtom);
        } else if (current instanceof Atom) {
          queue.push(...current.bonds);
        }
      }
    }
    getConnectedMolecule(startEntity, entitiesToReturn = [Atom, Bond]) {
      const connectedMoleculeMonomers = [];
      const queue = [startEntity];
      const visited = /* @__PURE__ */ new Set();
      while (queue.length > 0) {
        const current = queue.shift();
        if (!current || visited.has(current.id)) continue;
        visited.add(current.id);
        if (current instanceof Bond) {
          queue.push(current.firstAtom, current.secondAtom);
          if (entitiesToReturn.includes(Bond)) {
            connectedMoleculeMonomers.push(current);
          }
        } else if (current instanceof Atom) {
          queue.push(...current.bonds);
          if (entitiesToReturn.includes(Atom)) {
            connectedMoleculeMonomers.push(current);
          }
        }
      }
      return connectedMoleculeMonomers;
    }
    createRotationHistoryCommand(initialPositions) {
      const command = new Command();
      const zeroOffset = new Vec2(0, 0);
      [
        ...this.atoms.values(),
        ...this.monomers.values(),
        ...this.rxnArrows.values(),
        ...this.multitailArrows.values(),
        ...this.rxnPluses.values()
      ].forEach((drawingEntity) => {
        if (drawingEntity instanceof BaseMonomer && drawingEntity.monomerItem.props.isMicromoleculeFragment && !(0, import_monomers21.isMonomerSgroupWithAttachmentPoints)(drawingEntity)) {
          return;
        }
        if (!drawingEntity.selected) {
          return;
        }
        const initialPosition = initialPositions.get(drawingEntity.id);
        if (!initialPosition) {
          return;
        }
        const delta = drawingEntity.position.sub(initialPosition);
        if (delta.length() === 0) {
          return;
        }
        command.merge(
          this.createDrawingEntityMovingCommand(drawingEntity, zeroOffset, delta)
        );
      });
      this.polymerBonds.forEach((drawingEntity) => {
        var _a;
        if (drawingEntity.selected || drawingEntity.firstMonomer.selected || ((_a = drawingEntity.secondMonomer) == null ? void 0 : _a.selected)) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      this.monomerToAtomBonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.monomer.selected || drawingEntity.atom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      this.bonds.forEach((drawingEntity) => {
        if (drawingEntity.selected || drawingEntity.firstAtom.selected || drawingEntity.secondAtom.selected) {
          command.merge(
            this.createDrawingEntityMovingCommand(
              drawingEntity,
              zeroOffset,
              zeroOffset
            )
          );
        }
      });
      return command;
    }
  };

  // src/core/io/ket/toKet/rxnToKet.ts
  function arrowToKet(arrowNode) {
    return {
      type: "arrow",
      data: getNodeWithInvertedYCoord(arrowNode.data),
      selected: arrowNode.selected
    };
  }
  function plusToKet(plusNode) {
    const coord = plusNode.center;
    return {
      type: "plus",
      location: [coord.x, -coord.y, coord.z],
      prop: plusNode.data,
      selected: plusNode.selected
    };
  }

  // src/core/io/ket/toKet/headerToKet.ts
  var import_utilities5 = __toESM(require_utilities());
  function headerToKet(struct) {
    const header = {};
    (0, import_utilities5.ifDef)(header, "moleculeName", struct.name, "");
    (0, import_utilities5.ifDef)(header, "creatorProgram", null, "");
    (0, import_utilities5.ifDef)(header, "comment", null, "");
    return Object.keys(header).length !== 0 ? header : null;
  }

  // src/core/io/ket/toKet/moleculeToKet.ts
  var import_utilities6 = __toESM(require_utilities());
  var import_lodash9 = __toESM(require_lodash2());
  function fromRlabel(rg) {
    const res = [];
    let rgi;
    let val;
    for (rgi = 0; rgi < 32; rgi++) {
      if (rg & 1 << rgi) {
        val = rgi + 1;
        res.push(val);
      }
    }
    return res;
  }
  function moleculeToKet(struct, monomer) {
    const body = {
      atoms: Array.from(struct.atoms.values()).map((atom) => {
        if (atom.label === "R#" && !monomer) return rglabelToKet(atom);
        return atomToKet(atom, monomer);
      })
    };
    if (struct.bonds.size !== 0) {
      body.bonds = Array.from(struct.bonds.values()).map(bondToKet);
    }
    if (struct.sgroups.size !== 0) {
      body.sgroups = Array.from(struct.sgroups.values()).map(
        (sGroup) => sgroupToKet(struct, sGroup)
      );
    }
    const fragment = struct.frags.get(0);
    if (fragment) {
      (0, import_utilities6.ifDef)(body, "stereoFlagPosition", fragment.stereoFlagPosition, null);
      if (fragment.properties) {
        body.properties = fragment.properties;
      }
    }
    return __spreadValues({
      type: "molecule"
    }, body);
  }
  function atomToKet(source, monomer) {
    var _a;
    const result = {};
    if (source.label !== "L#") {
      (0, import_utilities6.ifDef)(
        result,
        "label",
        source.label === "R#" && monomer ? (_a = monomer.monomerItem.props.MonomerCaps) == null ? void 0 : _a[getAttachmentPointLabelWithBinaryShift(source.rglabel)] : source.label
      );
      (0, import_utilities6.ifDef)(result, "mapping", parseInt(source.aam), 0);
    } else if (source.atomList) {
      result.type = "atom-list";
      (0, import_utilities6.ifDef)(result, "elements", source.atomList.labelList());
      (0, import_utilities6.ifDef)(result, "notList", source.atomList.notList, false);
    }
    (0, import_utilities6.ifDef)(result, "alias", source.alias);
    const position = switchIntoChemistryCoordSystem(
      new Vec2(source.pp.x, source.pp.y, source.pp.z)
    );
    (0, import_utilities6.ifDef)(result, "location", [position.x, position.y, position.z]);
    (0, import_utilities6.ifDef)(result, "charge", source.charge);
    (0, import_utilities6.ifDef)(result, "explicitValence", source.explicitValence, -1);
    (0, import_utilities6.ifDef)(result, "isotope", source.isotope);
    (0, import_utilities6.ifDef)(result, "radical", source.radical, 0);
    (0, import_utilities6.ifDef)(result, "attachmentPoints", source.attachmentPoints, 0);
    (0, import_utilities6.ifDef)(result, "cip", source.cip, "");
    (0, import_utilities6.ifDef)(result, "selected", source.getInitiallySelected());
    (0, import_utilities6.ifDef)(result, "stereoLabel", source.stereoLabel, null);
    (0, import_utilities6.ifDef)(result, "stereoParity", source.stereoCare, 0);
    (0, import_utilities6.ifDef)(result, "weight", source.weight, 0);
    (0, import_utilities6.ifDef)(result, "ringBondCount", source.ringBondCount, 0);
    (0, import_utilities6.ifDef)(result, "substitutionCount", source.substitutionCount, 0);
    (0, import_utilities6.ifDef)(result, "unsaturatedAtom", !!source.unsaturatedAtom, false);
    (0, import_utilities6.ifDef)(result, "hCount", source.hCount, 0);
    if (Object.values(source.queryProperties).some((property) => property !== null)) {
      result.queryProperties = {};
      Object.keys(source.queryProperties).forEach((name) => {
        (0, import_utilities6.ifDef)(result.queryProperties, name, source.queryProperties[name]);
      });
    }
    (0, import_utilities6.ifDef)(result, "invRet", source.invRet, 0);
    (0, import_utilities6.ifDef)(result, "exactChangeFlag", !!source.exactChangeFlag, false);
    (0, import_utilities6.ifDef)(result, "implicitHCount", source.implicitHCount);
    return result;
  }
  function rglabelToKet(source) {
    const result = {
      type: "rg-label"
    };
    const position = switchIntoChemistryCoordSystem(
      new Vec2(source.pp.x, source.pp.y, source.pp.z)
    );
    (0, import_utilities6.ifDef)(result, "location", [position.x, position.y, position.z]);
    (0, import_utilities6.ifDef)(result, "attachmentPoints", source.attachmentPoints, 0);
    const refsToRGroups = fromRlabel(source.rglabel).map(
      (rgnumber) => `rg-${rgnumber}`
    );
    (0, import_utilities6.ifDef)(result, "$refs", refsToRGroups);
    (0, import_utilities6.ifDef)(result, "selected", source.getInitiallySelected());
    return result;
  }
  function bondToKet(source) {
    const result = {};
    if (source.customQuery) {
      (0, import_utilities6.ifDef)(result, "atoms", [source.begin, source.end]);
      (0, import_utilities6.ifDef)(result, "customQuery", source.customQuery);
    } else {
      (0, import_utilities6.ifDef)(result, "type", source.type);
      (0, import_utilities6.ifDef)(result, "atoms", [source.begin, source.end]);
      (0, import_utilities6.ifDef)(result, "stereo", source.stereo, 0);
      (0, import_utilities6.ifDef)(result, "topology", source.topology, 0);
      (0, import_utilities6.ifDef)(result, "center", source.reactingCenterStatus, 0);
      (0, import_utilities6.ifDef)(result, "cip", source.cip, "");
    }
    (0, import_utilities6.ifDef)(result, "selected", source.getInitiallySelected());
    return result;
  }
  function sgroupToKet(struct, source) {
    const result = {};
    (0, import_utilities6.ifDef)(result, "type", source.type);
    (0, import_utilities6.ifDef)(result, "atoms", source.atoms);
    switch (source.type) {
      case "MUL": {
        (0, import_utilities6.ifDef)(result, "mul", source.data.mul || 1);
        break;
      }
      case "SRU": {
        (0, import_utilities6.ifDef)(result, "subscript", source.data.subscript || "n");
        (0, import_utilities6.ifDef)(
          result,
          "connectivity",
          source.data.connectivity.toUpperCase() || "HT"
        );
        break;
      }
      case "COP": {
        (0, import_utilities6.ifDef)(
          result,
          "subtype",
          source.data.subtype ? source.data.subtype.toUpperCase() : null
        );
        (0, import_utilities6.ifDef)(
          result,
          "connectivity",
          source.data.connectivity.toUpperCase() || "HT"
        );
        break;
      }
      case "SUP": {
        (0, import_utilities6.ifDef)(result, "name", source.data.name || "");
        (0, import_utilities6.ifDef)(result, "expanded", source.data.expanded);
        (0, import_utilities6.ifDef)(result, "id", source.id);
        (0, import_utilities6.ifDef)(result, "class", source.data.class);
        (0, import_utilities6.ifDef)(
          result,
          "attachmentPoints",
          source.getAttachmentPoints().map(sgroupAttachmentPointToKet),
          []
        );
        break;
      }
      case "DAT": {
        const data = source.data;
        (0, import_utilities6.ifDef)(result, "placement", data.absolute, true);
        (0, import_utilities6.ifDef)(result, "display", data.attached, false);
        (0, import_utilities6.ifDef)(result, "context", data.context);
        (0, import_utilities6.ifDef)(result, "fieldName", data.fieldName);
        (0, import_utilities6.ifDef)(result, "fieldData", data.fieldValue);
        (0, import_utilities6.ifDef)(result, "bonds", SGroup.getBonds(struct, source));
        break;
      }
      case "GEN":
      case "queryComponent":
      default:
        break;
    }
    return result;
  }
  function sgroupAttachmentPointToKet(source) {
    const result = {};
    (0, import_utilities6.ifDef)(result, "attachmentAtom", source.atomId);
    (0, import_utilities6.ifDef)(result, "leavingAtom", source.leaveAtomId);
    (0, import_utilities6.ifDef)(
      result,
      "attachmentId",
      (0, import_lodash9.isNumber)(source.attachmentPointNumber) ? source.attachmentPointNumber.toString() : source.attachmentId
    );
    return result;
  }

  // src/core/io/ket/fromKet/moleculeToStruct.ts
  var import_utilities7 = __toESM(require_utilities());

  // src/core/io/ket/fromKet/mergeFragmentsToStruct.ts
  function mergeFragmentsToStruct(ketItem, struct) {
    let atomsOffset = 0;
    if (ketItem.fragments) {
      ketItem.fragments.forEach((fragment) => {
        var _a, _b, _c, _d;
        (_a = fragment.atoms) == null ? void 0 : _a.forEach((atom) => struct.atoms.add(atomToStruct(atom)));
        (_b = fragment.bonds) == null ? void 0 : _b.forEach(
          (bond) => struct.bonds.add(bondToStruct(bond, atomsOffset))
        );
        atomsOffset += (_d = (_c = fragment.atoms) == null ? void 0 : _c.length) != null ? _d : 0;
      });
    }
    return struct;
  }

  // src/core/io/ket/fromKet/moleculeToStruct.ts
  function toRlabel(values2) {
    let res = 0;
    values2.forEach((val) => {
      const rgi = val - 1;
      res |= 1 << rgi;
    });
    return res;
  }
  function moleculeToStruct(ketItem) {
    const struct = mergeFragmentsToStruct(ketItem, new Struct());
    if (ketItem.atoms) {
      ketItem.atoms.forEach((atom) => {
        let atomId = null;
        if (atom.type === "rg-label") {
          atomId = struct.atoms.add(rglabelToStruct(atom));
        }
        if (!atom.type || atom.type === "atom-list") {
          atomId = struct.atoms.add(atomToStruct(atom));
        }
        if (atomId !== null) {
          addRGroupAttachmentPointsToStruct(
            struct,
            atomId,
            atom.attachmentPoints,
            atom.selected
          );
        }
      });
    }
    if (ketItem.bonds) {
      ketItem.bonds.forEach((bond) => struct.bonds.add(bondToStruct(bond)));
    }
    if (ketItem.sgroups) {
      ketItem.sgroups.forEach((sgroupData) => {
        const sgroup = sgroupToStruct(sgroupData);
        const id2 = struct.sgroups.add(sgroup);
        sgroup.id = id2;
      });
    }
    struct.initHalfBonds();
    struct.initNeighbors();
    struct.markFragments(ketItem.properties);
    struct.bindSGroupsToFunctionalGroups();
    return struct;
  }
  function atomToStruct(source) {
    const params = {};
    const queryAttribute = [
      "aromaticity",
      "ringMembership",
      "connectivity",
      "ringSize",
      "chirality",
      "customQuery"
    ];
    if (source.type === "atom-list") {
      params.label = "L#";
      const ids = source.elements.map((el) => {
        var _a;
        return (_a = Elements.get(el)) == null ? void 0 : _a.number;
      }).filter((id2) => id2);
      (0, import_utilities7.ifDef)(params, "atomList", {
        ids,
        notList: source.notList
      });
    } else {
      (0, import_utilities7.ifDef)(params, "label", source.label);
      (0, import_utilities7.ifDef)(params, "aam", source.mapping);
    }
    (0, import_utilities7.ifDef)(params, "alias", source.alias);
    (0, import_utilities7.ifDef)(params, "pp", {
      x: source.location[0],
      y: -source.location[1],
      z: source.location[2] || 0
    });
    (0, import_utilities7.ifDef)(params, "charge", source.charge);
    (0, import_utilities7.ifDef)(params, "explicitValence", source.explicitValence);
    (0, import_utilities7.ifDef)(params, "isotope", source.isotope);
    (0, import_utilities7.ifDef)(params, "radical", source.radical);
    (0, import_utilities7.ifDef)(params, "cip", source.cip);
    (0, import_utilities7.ifDef)(params, "attachmentPoints", source.attachmentPoints);
    (0, import_utilities7.ifDef)(params, "stereoLabel", source.stereoLabel);
    (0, import_utilities7.ifDef)(params, "stereoParity", source.stereoParity);
    (0, import_utilities7.ifDef)(params, "weight", source.weight);
    (0, import_utilities7.ifDef)(params, "ringBondCount", source.ringBondCount);
    (0, import_utilities7.ifDef)(params, "substitutionCount", source.substitutionCount);
    (0, import_utilities7.ifDef)(params, "unsaturatedAtom", Number(Boolean(source.unsaturatedAtom)));
    (0, import_utilities7.ifDef)(params, "hCount", source.hCount);
    if (source.queryProperties && Object.values(source.queryProperties).some((property) => property !== null)) {
      params.queryProperties = {};
      queryAttribute.forEach((attributeName) => {
        (0, import_utilities7.ifDef)(
          params.queryProperties,
          attributeName,
          source.queryProperties[attributeName]
        );
      });
    }
    (0, import_utilities7.ifDef)(params, "invRet", source.invRet);
    (0, import_utilities7.ifDef)(params, "exactChangeFlag", Number(Boolean(source.exactChangeFlag)));
    (0, import_utilities7.ifDef)(params, "implicitHCount", source.implicitHCount);
    const newAtom = new Atom2(params);
    newAtom.setInitiallySelected(source.selected);
    return newAtom;
  }
  function rglabelToStruct(source) {
    const params = {};
    params.label = "R#";
    (0, import_utilities7.ifDef)(params, "pp", {
      x: source.location[0],
      y: -source.location[1],
      z: source.location[2] || 0
    });
    (0, import_utilities7.ifDef)(params, "attachmentPoints", source.attachmentPoints);
    const rglabel = toRlabel(source.$refs.map((el) => parseInt(el.slice(3))));
    (0, import_utilities7.ifDef)(params, "rglabel", rglabel);
    const newAtom = new Atom2(params);
    newAtom.setInitiallySelected(source.selected);
    return newAtom;
  }
  function addRGroupAttachmentPointsToStruct(struct, attachedAtomId, attachmentPoints, initiallySelected) {
    const rgroupAttachmentPoints = [];
    if (attachmentPoints === 1 /* FirstSideOnly */) {
      rgroupAttachmentPoints.push(
        new RGroupAttachmentPoint(attachedAtomId, "primary", initiallySelected)
      );
    } else if (attachmentPoints === 2 /* SecondSideOnly */) {
      rgroupAttachmentPoints.push(
        new RGroupAttachmentPoint(attachedAtomId, "secondary", initiallySelected)
      );
    } else if (attachmentPoints === 3 /* BothSides */) {
      rgroupAttachmentPoints.push(
        new RGroupAttachmentPoint(attachedAtomId, "primary", initiallySelected)
      );
      rgroupAttachmentPoints.push(
        new RGroupAttachmentPoint(attachedAtomId, "secondary", initiallySelected)
      );
    }
    rgroupAttachmentPoints.forEach((rgroupAttachmentPoint) => {
      struct.rgroupAttachmentPoints.add(rgroupAttachmentPoint);
    });
  }
  function bondToStruct(source, atomOffset = 0) {
    const params = {};
    (0, import_utilities7.ifDef)(params, "type", source.type);
    (0, import_utilities7.ifDef)(params, "topology", source.topology);
    (0, import_utilities7.ifDef)(params, "reactingCenterStatus", source.center);
    (0, import_utilities7.ifDef)(params, "stereo", source.stereo);
    (0, import_utilities7.ifDef)(params, "cip", source.cip);
    (0, import_utilities7.ifDef)(params, "customQuery", source.customQuery);
    (0, import_utilities7.ifDef)(params, "begin", source.atoms[0] + atomOffset);
    (0, import_utilities7.ifDef)(params, "end", source.atoms[1] + atomOffset);
    (0, import_utilities7.ifDef)(params, "initiallySelected", source.selected);
    const newBond = new Bond3(params);
    newBond.setInitiallySelected(source.selected);
    return newBond;
  }
  function sgroupToStruct(source) {
    var _a;
    const sgroup = new SGroup(source.type);
    (0, import_utilities7.ifDef)(sgroup, "atoms", source.atoms);
    switch (source.type) {
      case "MUL": {
        (0, import_utilities7.ifDef)(sgroup.data, "mul", source.mul);
        break;
      }
      case "SRU": {
        (0, import_utilities7.ifDef)(sgroup.data, "subscript", source.subscript);
        (0, import_utilities7.ifDef)(sgroup.data, "connectivity", source.connectivity.toLowerCase());
        break;
      }
      case "COP": {
        (0, import_utilities7.ifDef)(sgroup.data, "subtype", source.subtype);
        (0, import_utilities7.ifDef)(sgroup.data, "connectivity", source.connectivity.toLowerCase());
        break;
      }
      case "SUP": {
        (0, import_utilities7.ifDef)(sgroup.data, "name", source.name);
        (0, import_utilities7.ifDef)(sgroup.data, "expanded", source.expanded);
        (0, import_utilities7.ifDef)(sgroup.data, "class", source.class);
        (0, import_utilities7.ifDef)(sgroup, "id", source.id);
        (_a = source.attachmentPoints) == null ? void 0 : _a.forEach(
          (sourceAttachmentPoint, sourceAttachmentPointIndex) => {
            sgroup.addAttachmentPoint(
              sgroupAttachmentPointToStruct(
                sourceAttachmentPoint,
                sourceAttachmentPointIndex + 1
              )
            );
          }
        );
        break;
      }
      case "DAT": {
        (0, import_utilities7.ifDef)(sgroup.data, "absolute", source.placement);
        (0, import_utilities7.ifDef)(sgroup.data, "attached", source.display);
        (0, import_utilities7.ifDef)(sgroup.data, "context", source.context);
        (0, import_utilities7.ifDef)(sgroup.data, "fieldName", source.fieldName);
        (0, import_utilities7.ifDef)(sgroup.data, "fieldValue", source.fieldData);
        break;
      }
      case "GEN":
      default:
        break;
    }
    return sgroup;
  }
  function sgroupAttachmentPointToStruct(source, attachmentPointNumber) {
    const atomId = source.attachmentAtom;
    const leavingAtomId = source.leavingAtom;
    const attachmentId = source.attachmentId;
    return new SGroupAttachmentPoint(
      atomId,
      leavingAtomId,
      attachmentId,
      attachmentId && !isNaN(Number(attachmentId)) ? Number(attachmentId) : attachmentPointNumber
    );
  }

  // src/core/io/ket/toKet/prepare.ts
  function prepareStructForKet(struct) {
    const ketNodes = [];
    const rgFrags = /* @__PURE__ */ new Set();
    for (const [rgnumber, rgroup] of struct.rgroups.entries()) {
      rgroup.frags.forEach((frid) => rgFrags.add(frid));
      const fragsAtoms = Array.from(rgroup.frags.values()).reduce(
        (res, frid) => res.union(struct.getFragmentIds(frid)),
        new Pile()
      );
      ketNodes.push({
        type: "rgroup",
        fragment: struct.clone(fragsAtoms),
        center: getFragmentCenter(struct, fragsAtoms),
        data: { rgnumber, rgroup }
      });
    }
    const filteredFragmentIds = Array.from(struct.frags.keys()).filter(
      (fid) => !rgFrags.has(fid)
    );
    addMolecules(ketNodes, filteredFragmentIds, struct);
    struct.rxnArrows.forEach((item) => {
      ketNodes.push({
        type: "arrow",
        center: item.pos[0],
        data: {
          mode: item.mode,
          pos: item.pos,
          height: item.height
        },
        selected: item.getInitiallySelected()
      });
    });
    struct.rxnPluses.forEach((item) => {
      ketNodes.push({
        type: "plus",
        center: item.pp,
        data: {},
        selected: item.getInitiallySelected()
      });
    });
    struct.simpleObjects.forEach((item) => {
      ketNodes.push({
        type: "simpleObject",
        center: item.pos[0],
        data: {
          mode: item.mode,
          pos: item.pos
        },
        selected: item.getInitiallySelected()
      });
    });
    struct.texts.forEach((item) => {
      ketNodes.push({
        type: "text",
        center: item.position,
        data: {
          content: item.content,
          position: item.position,
          pos: item.pos
        },
        selected: item.getInitiallySelected()
      });
    });
    struct.images.forEach((image) => {
      ketNodes.push(image.toKetNode());
    });
    struct.multitailArrows.forEach((multitailArrow) => {
      ketNodes.push(multitailArrow.toKetNode());
    });
    ketNodes.forEach((ketNode) => {
      if (ketNode.fragment) {
        const sgroups = Array.from(ketNode.fragment.sgroups.values());
        const filteredSGroups = sgroups.filter(
          (sg) => sg.atoms.every((atom) => atom !== void 0)
        );
        const filteredSGroupsMap = new Pool();
        filteredSGroups.forEach((sg, index) => {
          filteredSGroupsMap.set(index, sg);
        });
        ketNode.fragment.sgroups = filteredSGroupsMap;
      }
    });
    return ketNodes;
  }
  function getFragmentCenter(struct, atomSet) {
    const bb = struct.getCoordBoundingBox(atomSet);
    return Vec2.centre(bb.min, bb.max);
  }
  function addMolecules(ketNodes, fragmentIds, struct) {
    const sGroupFragmentsMap = generateSGroupFragmentsMap(
      ketNodes,
      fragmentIds,
      struct
    );
    const mergedFragments = Pile.unionIntersections(
      Array.from(sGroupFragmentsMap.values())
    );
    mergedFragments.forEach((fragments) => {
      let atomSet = new Pile();
      fragments.forEach((fragmentId) => {
        atomSet = atomSet.union(struct.getFragmentIds(fragmentId));
      });
      ketNodes.push({
        type: "molecule",
        fragment: struct.clone(atomSet),
        center: getFragmentCenter(struct, atomSet)
      });
    });
  }
  function generateSGroupFragmentsMap(ketNodes, fragmentIds, struct) {
    const sGroupFragmentsMap = /* @__PURE__ */ new Map();
    fragmentIds.forEach((fragmentId) => {
      const atomsInFragment = struct.getFragmentIds(fragmentId);
      let hasAtomInSGroup = false;
      atomsInFragment.forEach((atomId) => {
        var _a;
        (_a = struct.atoms.get(atomId)) == null ? void 0 : _a.sgs.forEach((sGroupId) => {
          hasAtomInSGroup = true;
          const fragmentSet = sGroupFragmentsMap.get(sGroupId);
          if (fragmentSet) {
            fragmentSet.add(fragmentId);
          } else {
            sGroupFragmentsMap.set(sGroupId, new Pile([fragmentId]));
          }
        });
      });
      if (!hasAtomInSGroup) {
        ketNodes.push({
          type: "molecule",
          fragment: struct.clone(atomsInFragment),
          center: getFragmentCenter(struct, atomsInFragment)
        });
      }
    });
    return sGroupFragmentsMap;
  }

  // src/core/io/ket/toKet/rgroupToKet.ts
  var import_utilities8 = __toESM(require_utilities());
  function rgroupToKet(struct, data) {
    const body = __spreadValues({
      rlogic: rgroupLogicToKet(data.rgnumber, data.rgroup)
    }, moleculeToKet(struct));
    return __spreadProps(__spreadValues({}, body), {
      type: "rgroup"
    });
  }
  function rgroupLogicToKet(rgnumber, rglogic) {
    const result = {};
    (0, import_utilities8.ifDef)(result, "number", rgnumber);
    (0, import_utilities8.ifDef)(result, "range", rglogic.range, "");
    (0, import_utilities8.ifDef)(result, "resth", rglogic.resth, false);
    (0, import_utilities8.ifDef)(result, "ifthen", rglogic.ifthen, 0);
    return result;
  }

  // src/core/io/ket/fromKet/rgroupToStruct.ts
  var import_utilities9 = __toESM(require_utilities());
  function rgroupToStruct(ketItem) {
    const struct = moleculeToStruct(ketItem);
    const rgroup = rgroupLogicToStruct(ketItem.rlogic);
    struct.frags.forEach((_value, key) => {
      rgroup.frags.add(key);
    });
    if (ketItem.rlogic) struct.rgroups.set(ketItem.rlogic.number, rgroup);
    return struct;
  }
  function rgroupLogicToStruct(rglogic) {
    const params = {};
    (0, import_utilities9.ifDef)(params, "range", rglogic.range);
    (0, import_utilities9.ifDef)(params, "resth", rglogic.resth);
    (0, import_utilities9.ifDef)(params, "ifthen", rglogic.ifthen);
    return new RGroup(params);
  }

  // src/core/io/ket/fromKet/rxnToStruct.ts
  function rxnToStruct(ketItem, struct) {
    if (ketItem.type === "arrow") {
      const arrow = new RxnArrow(getNodeWithInvertedYCoord(ketItem.data));
      arrow.setInitiallySelected(ketItem.selected);
      struct.addRxnArrow(arrow);
    } else {
      const plus = new RxnPlus({
        pp: {
          x: ketItem.location[0],
          y: -ketItem.location[1],
          z: ketItem.location[2]
        }
      });
      plus.setInitiallySelected(ketItem.selected);
      struct.rxnPluses.add(plus);
    }
    return struct;
  }

  // src/core/io/ket/toKet/simpleObjectToKet.ts
  function simpleObjectToKet(simpleObjectNode) {
    return {
      type: "simpleObject",
      data: getNodeWithInvertedYCoord(simpleObjectNode.data),
      selected: simpleObjectNode.selected
    };
  }

  // src/core/io/ket/fromKet/simpleObjectToStruct.ts
  function simpleObjectToStruct(ketItem, struct) {
    const object = ketItem.data.mode === "circle" ? circleToEllipse(ketItem) : ketItem.data;
    const simpleObject = new SimpleObject(getNodeWithInvertedYCoord(object));
    simpleObject.setInitiallySelected(ketItem.selected);
    struct.simpleObjects.add(simpleObject);
    return struct;
  }
  function circleToEllipse(ketItem) {
    const radius = Vec2.dist(ketItem.data.pos[1], ketItem.data.pos[0]);
    const pos0 = ketItem.data.pos[0];
    return {
      mode: "ellipse" /* ellipse */,
      pos: [
        {
          x: pos0.x - Math.abs(radius),
          y: pos0.y - Math.abs(radius),
          z: pos0.z - Math.abs(radius)
        },
        {
          x: pos0.x + Math.abs(radius),
          y: pos0.y + Math.abs(radius),
          z: pos0.z + Math.abs(radius)
        }
      ]
    };
  }

  // src/core/io/ket/toKet/textToKet.ts
  var IS_BOLD = 1;
  var IS_ITALIC = 2;
  var IS_SUBSCRIPT = 32;
  var IS_SUPERSCRIPT = 64;
  function applyFontStyleOverrides(target, child) {
    var _a;
    const format = (_a = child.format) != null ? _a : 0;
    if (format & IS_BOLD) target.bold = true;
    if (format & IS_ITALIC) target.italic = true;
    if (format & IS_SUPERSCRIPT) target.superscript = true;
    if (format & IS_SUBSCRIPT) target.subscript = true;
    const font = {};
    let hasFont = false;
    if (child.font !== void 0) {
      font.family = child.font;
      hasFont = true;
    }
    if (child.style) {
      const fontSizeMatch = /font-size:\s*(\d+(?:\.\d+)?)px/.exec(child.style);
      if (fontSizeMatch) {
        font.size = parseFloat(fontSizeMatch[1]);
        hasFont = true;
      }
    }
    if (hasFont) {
      target.font = font;
    }
    if (child.style) {
      const colorMatch = /(?:^|;)\s*color:\s*(#[0-9A-Fa-f]+)/.exec(child.style);
      if (colorMatch) {
        target.color = colorMatch[1];
      }
    }
  }
  function textToKet(textNode) {
    const convertToKET20Text = (source) => {
      const pos = source.pos;
      const x = pos[0].x;
      const y = pos[0].y;
      const width = pos[2].x - pos[0].x;
      const height = Math.abs(pos[1].y - pos[0].y);
      const textContent = JSON.parse(source.content);
      const root = textContent.root;
      if (!root) {
        return {
          type: "text",
          data: source
        };
      }
      const ketText = {
        type: "text",
        boundingBox: { x, y, width, height },
        paragraphs: []
      };
      if (root.alignment !== void 0) ketText.alignment = root.alignment;
      if (root.indent !== void 0) ketText.indent = root.indent;
      if (root.font !== void 0) ketText.font = root.font;
      if (root.color !== void 0) ketText.color = root.color;
      if (root.bold !== void 0) ketText.bold = root.bold;
      if (root.italic !== void 0) ketText.italic = root.italic;
      if (root.superscript !== void 0) ketText.superscript = root.superscript;
      if (root.subscript !== void 0) ketText.subscript = root.subscript;
      ketText.paragraphs = (root.children || []).map(
        (paragraph) => {
          const paraObj = { parts: [] };
          if (paragraph.alignment !== void 0)
            paraObj.alignment = paragraph.alignment;
          if (paragraph.indent !== void 0) paraObj.indent = paragraph.indent;
          if (paragraph.font !== void 0) paraObj.font = paragraph.font;
          if (paragraph.color !== void 0) paraObj.color = paragraph.color;
          if (paragraph.bold !== void 0) paraObj.bold = paragraph.bold;
          if (paragraph.italic !== void 0) paraObj.italic = paragraph.italic;
          if (paragraph.superscript !== void 0)
            paraObj.superscript = paragraph.superscript;
          if (paragraph.subscript !== void 0)
            paraObj.subscript = paragraph.subscript;
          paraObj.parts = (paragraph.children || []).map((child) => {
            if (child.type !== "text" || child.text === void 0) return null;
            const part = { text: child.text };
            applyFontStyleOverrides(part, child);
            return part;
          }).filter((p) => Boolean(p));
          return paraObj;
        }
      );
      return ketText;
    };
    return __spreadValues({
      selected: textNode.selected
    }, convertToKET20Text(getNodeWithInvertedYCoord(textNode.data)));
  }

  // src/core/io/ket/fromKet/textToStruct.ts
  var import_draftToLexical = __toESM(require_draftToLexical());
  var IS_BOLD2 = 1;
  var IS_ITALIC2 = 2;
  var IS_SUBSCRIPT2 = 32;
  var IS_SUPERSCRIPT2 = 64;
  function convertKetV2ToInternal(ketText) {
    const { boundingBox, paragraphs } = ketText;
    const { x, y, z, width, height } = boundingBox;
    const pos = [
      { x, y, z },
      { x, y: y + height, z },
      { x: x + width, y: y + height, z },
      { x: x + width, y, z }
    ];
    const lexicalRoot = {
      root: {
        children: paragraphs.map((para) => {
          const paragraphNode = {
            children: (para.parts || []).map((part) => {
              var _a, _b;
              let format = 0;
              if (part.bold) format |= IS_BOLD2;
              if (part.italic) format |= IS_ITALIC2;
              if (part.subscript) format |= IS_SUBSCRIPT2;
              if (part.superscript) format |= IS_SUPERSCRIPT2;
              const textNode = {
                detail: 0,
                format,
                mode: "normal",
                style: "",
                text: part.text,
                type: "text",
                version: 1
              };
              const styles = [];
              if ((_a = part.font) == null ? void 0 : _a.size) {
                styles.push(`font-size: ${part.font.size}px`);
              }
              if (part.color) {
                styles.push(`color: ${part.color}`);
              }
              if (styles.length > 0) {
                textNode.style = styles.join("; ");
              }
              if ((_b = part.font) == null ? void 0 : _b.family) {
                textNode.font = part.font.family;
              }
              return textNode;
            }),
            direction: "ltr",
            format: "",
            indent: 0,
            type: "paragraph",
            version: 1,
            textFormat: 0,
            textStyle: ""
          };
          if (para.alignment) {
            paragraphNode.format = para.alignment;
          }
          return paragraphNode;
        }),
        direction: "ltr",
        format: "",
        indent: 0,
        type: "root",
        version: 1
      }
    };
    return {
      position: { x, y, z },
      pos,
      content: JSON.stringify(lexicalRoot)
    };
  }
  function isKetV2Format(ketItem) {
    return ketItem && ketItem.boundingBox !== void 0 && ketItem.paragraphs !== void 0;
  }
  function textToStruct(ketItem, struct) {
    let node;
    if (isKetV2Format(ketItem)) {
      const internal = convertKetV2ToInternal(ketItem);
      node = getNodeWithInvertedYCoord(internal);
    } else {
      node = getNodeWithInvertedYCoord(ketItem.data);
      if (node && node.content) {
        try {
          const parsed = typeof node.content === "string" ? JSON.parse(node.content) : node.content;
          if (parsed && Array.isArray(parsed.blocks)) {
            const lexical = (0, import_draftToLexical.convertDraftToLexical)(parsed);
            node.content = JSON.stringify(lexical);
          }
        } catch (e) {
        }
      }
    }
    const text = new Text(node);
    text.setInitiallySelected(ketItem.selected);
    struct.texts.add(text);
    return struct;
  }

  // src/core/io/ket/types/macromolecules.ts
  var MACROMOLECULES_BOND_TYPES2 = {};
  var KetConnectionType = {};
  var KetNodeType = {};
  var KetTemplateType2 = {};
  var MacromoleculesConverter = class {
  };

  // src/core/io/ket/fromKet/monomerTemplateUtils.ts
  var import_lodash10 = __toESM(require_lodash2());

  // src/core/io/ket/fromKet/imageToStruct.ts
  function imageToStruct(ketItem, struct) {
    struct.images.add(Image.fromKetNode(ketItem));
    return struct;
  }

  // src/core/io/ket/fromKet/multitailArrowToStruct.ts
  function multitailArrowToStruct(ketItem, struct) {
    struct.addMultitailArrow(MultitailArrow.fromKetNode(ketItem));
    return struct;
  }

  // src/core/io/ket/fromKet/monomerTemplateUtils.ts
  function parseNode(node, struct) {
    const type = node.type;
    switch (type) {
      case "arrow":
      case "plus": {
        rxnToStruct(node, struct);
        break;
      }
      case "simpleObject": {
        simpleObjectToStruct(node, struct);
        break;
      }
      case "molecule": {
        const currentStruct = moleculeToStruct(node);
        if (node.stereoFlagPosition) {
          const fragment = currentStruct.frags.get(0);
          if (fragment) {
            fragment.stereoFlagPosition = new Vec2(node.stereoFlagPosition);
          }
        }
        currentStruct.mergeInto(struct);
        break;
      }
      case "rgroup": {
        rgroupToStruct(node).mergeInto(struct);
        break;
      }
      case "text": {
        textToStruct(node, struct);
        break;
      }
      case MULTITAIL_ARROW_SERIALIZE_KEY: {
        multitailArrowToStruct(node, struct);
        break;
      }
      case IMAGE_SERIALIZE_KEY: {
        imageToStruct(node, struct);
        break;
      }
      default:
        break;
    }
  }
  function fillMonomerTemplateStruct(ket) {
    var _a, _b;
    const resultingStruct = new Struct();
    const nodes = ket.root.nodes;
    Object.keys(nodes).forEach((i) => {
      if (nodes[i].type) parseNode(nodes[i], resultingStruct);
      else if (nodes[i].$ref) parseNode(ket[nodes[i].$ref], resultingStruct);
    });
    resultingStruct.name = (_b = (_a = ket.header) == null ? void 0 : _a.moleculeName) != null ? _b : "";
    return resultingStruct;
  }
  function normalizeTemplateAttachmentPoints(template) {
    const attachmentPointsDict = template.attachmentPointsDict;
    if (!attachmentPointsDict) {
      return template.attachmentPoints;
    }
    return Object.entries(attachmentPointsDict).map(([key, attachmentPoint]) => {
      var _a;
      let normalizedLabel;
      if (attachmentPoint.type === "left") {
        normalizedLabel = "R1" /* R1 */;
      } else if (attachmentPoint.type === "right") {
        normalizedLabel = "R2" /* R2 */;
      } else {
        normalizedLabel = void 0;
      }
      return __spreadValues(__spreadProps(__spreadValues({}, attachmentPoint), {
        label: (_a = attachmentPoint.label) != null ? _a : key
      }), normalizedLabel ? { type: attachmentPoint.type } : {});
    });
  }
  function getTemplateAttachmentPoints(template) {
    var _a;
    const attachmentPoints = (_a = normalizeTemplateAttachmentPoints(template)) != null ? _a : [];
    return template.unresolved ? attachmentPoints.map((_, index) => {
      return {
        attachmentAtom: index,
        leavingGroup: {
          atoms: []
        }
      };
    }) : attachmentPoints;
  }
  function convertMonomerTemplateToStruct(template) {
    var _a;
    const attachmentPoints = (_a = getTemplateAttachmentPoints(template)) != null ? _a : [];
    return fillMonomerTemplateStruct({
      root: {
        nodes: [{ $ref: "mol0" }]
      },
      mol0: __spreadProps(__spreadValues({}, template), {
        type: "molecule",
        atoms: template.unresolved ? attachmentPoints == null ? void 0 : attachmentPoints.map((_, index) => {
          return {
            label: "C",
            location: [index, index, index]
          };
        }) : template.atoms,
        bonds: template.unresolved ? attachmentPoints == null ? void 0 : attachmentPoints.map((_, index) => {
          if (index === attachmentPoints.length - 1) {
            return {
              type: 1,
              atoms: [0, attachmentPoints.length - 1]
            };
          }
          return {
            type: 1,
            atoms: [index, index + 1]
          };
        }) : template.bonds,
        attachmentPoints
      }),
      header: {
        moleculeName: template.fullName
      }
    });
  }
  function fillStructRgLabelsByMonomerTemplate(template, monomerItem) {
    if (monomerItem.props.unresolved) {
      return;
    }
    const attachmentPoints = getTemplateAttachmentPoints(template);
    const { attachmentPointsList } = BaseMonomer.getAttachmentPointDictFromMonomerDefinition(attachmentPoints);
    attachmentPoints == null ? void 0 : attachmentPoints.forEach((attachmentPoint, attachmentPointIndex) => {
      var _a;
      const firstAtomInLeavingGroup = (_a = attachmentPoint.leavingGroup) == null ? void 0 : _a.atoms[0];
      const leavingGroupAtom = monomerItem.struct.atoms.get(
        (0, import_lodash10.isNumber)(firstAtomInLeavingGroup) ? firstAtomInLeavingGroup : attachmentPoint.attachmentAtom
      );
      assert_default(leavingGroupAtom);
      leavingGroupAtom.rglabel = (0 | 1 << Number(
        (attachmentPoint.label ? attachmentPoint.label : attachmentPointsList[attachmentPointIndex]).replace("R", "")
      ) - 1).toString();
      assert_default(monomerItem.props.MonomerCaps);
      monomerItem.props.MonomerCaps[getAttachmentPointLabelWithBinaryShift(Number(leavingGroupAtom.rglabel))] = leavingGroupAtom.label;
    });
  }

  // src/core/io/ket/fromKet/monomerToDrawingEntity.ts
  function templateToMonomerProps(template) {
    var _a, _b, _c, _d, _e, _f, _g;
    return __spreadValues(__spreadValues(__spreadValues({
      id: template.id,
      Name: (_c = (_b = (_a = template.fullName) != null ? _a : template.name) != null ? _b : template.alias) != null ? _c : template.id,
      MonomerNaturalAnalogCode: (_d = template.naturalAnalogShort) != null ? _d : "",
      MonomerNaturalAnalogThreeLettersCode: (_e = template.naturalAnalog) != null ? _e : "",
      MonomerName: (_g = (_f = template.name) != null ? _f : template.alias) != null ? _g : template.id,
      MonomerFullName: template.fullName,
      MonomerType: template.classHELM,
      MonomerClass: template.class,
      MonomerCaps: {},
      idtAliases: template.idtAliases,
      unresolved: template.unresolved,
      modificationTypes: template.modificationTypes
    }, template.aliasHELM ? { aliasHELM: template.aliasHELM } : {}), template.aliasAxoLabs ? { aliasAxoLabs: template.aliasAxoLabs } : {}), template.hidden ? { hidden: template.hidden } : {});
  }
  function monomerToDrawingEntity(node, template, struct, drawingEntitiesManager) {
    const position = switchIntoChemistryCoordSystem(
      new Vec2(node.position.x, node.position.y)
    );
    const { alias, id: id2 } = template;
    const { seqid, expanded, transformation } = node;
    return drawingEntitiesManager.addMonomer(
      __spreadValues(__spreadValues({
        struct,
        label: alias != null ? alias : id2,
        colorScheme: void 0,
        favorite: false,
        props: templateToMonomerProps(template),
        attachmentPoints: getTemplateAttachmentPoints(template),
        seqId: seqid
      }, expanded !== void 0 && {
        expanded
      }), transformation !== void 0 && {
        transformation: modifyTransformation(transformation)
      }),
      position
    );
  }
  function createMonomersForVariantMonomer(variantMonomerTemplate, parsedFileContent, monomerFactory2) {
    const monomerTemplates = variantMonomerTemplate.options.map((option) => {
      return parsedFileContent[setMonomerTemplatePrefix(option.templateId)];
    });
    const monomers = monomerTemplates.map((monomerTemplate) => {
      const monomerItem = {
        label: monomerTemplate.alias,
        expanded: false,
        struct: convertMonomerTemplateToStruct(monomerTemplate),
        props: templateToMonomerProps(monomerTemplate),
        attachmentPoints: getTemplateAttachmentPoints(monomerTemplate)
      };
      const [MonomerConstructor] = monomerFactory2(monomerItem);
      fillStructRgLabelsByMonomerTemplate(monomerTemplate, monomerItem);
      return new MonomerConstructor(monomerItem, void 0, {
        generateId: false
      });
    });
    return monomers;
  }
  function variantMonomerToDrawingEntity(drawingEntitiesManager, node, template, parsedFileContent, monomerFactory2) {
    const position = switchIntoChemistryCoordSystem(
      new Vec2(node.position.x, node.position.y)
    );
    const monomers = createMonomersForVariantMonomer(
      template,
      parsedFileContent,
      monomerFactory2
    );
    return drawingEntitiesManager.addAmbiguousMonomer(
      {
        monomers,
        id: template.id,
        subtype: template.subtype,
        label: node.alias,
        options: template.options,
        idtAliases: template.idtAliases,
        isAmbiguous: true,
        transformation: node.transformation
      },
      position
    );
  }

  // src/core/io/ket/fromKet/polymerBondToDrawingEntity.ts
  var import_editorSingleton3 = __toESM(require_editorSingleton());
  function polymerBondToDrawingEntity(connection, drawingEntitiesManager, atomIdMap, superatomMonomerToUsedAttachmentPoint, firstMonomer, secondMonomer) {
    var _a, _b, _c, _d, _e, _f, _g, _h;
    const command = new Command();
    const firstAttachmentPoint = (_c = connection.endpoint1.attachmentPointId) != null ? _c : getAttachmentPointLabel(
      (_b = (_a = firstMonomer.monomerItem.struct.sgroups.get(0)) == null ? void 0 : _a.getAttachmentPoints().find(
        (attachmentPoint) => {
          var _a2;
          return attachmentPoint.atomId === atomIdMap.get(Number(connection.endpoint1.atomId)) && !((_a2 = superatomMonomerToUsedAttachmentPoint.get(firstMonomer)) == null ? void 0 : _a2.has(
            getAttachmentPointLabel(
              attachmentPoint.attachmentPointNumber
            )
          ));
        }
      )) == null ? void 0 : _b.attachmentPointNumber
    );
    const secondAttachmentPoint = (_f = connection.endpoint2.attachmentPointId) != null ? _f : getAttachmentPointLabel(
      (_e = (_d = secondMonomer.monomerItem.struct.sgroups.get(0)) == null ? void 0 : _d.getAttachmentPoints().find(
        (attachmentPoint) => {
          var _a2;
          return attachmentPoint.atomId === atomIdMap.get(Number(connection.endpoint2.atomId)) && !((_a2 = superatomMonomerToUsedAttachmentPoint.get(secondMonomer)) == null ? void 0 : _a2.has(
            getAttachmentPointLabel(
              attachmentPoint.attachmentPointNumber
            )
          ));
        }
      )) == null ? void 0 : _e.attachmentPointNumber
    );
    if (!firstMonomer.isAttachmentPointExistAndFree(
      firstAttachmentPoint
    ) || !secondMonomer.isAttachmentPointExistAndFree(
      secondAttachmentPoint
    )) {
      const editor = (0, import_editorSingleton3.provideEditorInstance)();
      editor.events.error.dispatch(
        "There is no free attachment point for bond creation."
      );
      return new Command();
    }
    if (!superatomMonomerToUsedAttachmentPoint.get(firstMonomer)) {
      superatomMonomerToUsedAttachmentPoint.set(firstMonomer, /* @__PURE__ */ new Set());
    }
    if (!superatomMonomerToUsedAttachmentPoint.get(secondMonomer)) {
      superatomMonomerToUsedAttachmentPoint.set(secondMonomer, /* @__PURE__ */ new Set());
    }
    (_g = superatomMonomerToUsedAttachmentPoint.get(firstMonomer)) == null ? void 0 : _g.add(firstAttachmentPoint);
    (_h = superatomMonomerToUsedAttachmentPoint.get(secondMonomer)) == null ? void 0 : _h.add(secondAttachmentPoint);
    command.merge(
      drawingEntitiesManager.createPolymerBond(
        firstMonomer,
        secondMonomer,
        firstAttachmentPoint,
        secondAttachmentPoint
      )
    );
    return command;
  }

  // src/core/io/ket/ketSerializer.ts
  var import_utilities10 = __toESM(require_utilities());

  // src/core/io/ket/compiledSchema.ts
  function compiledSchema(ket) {
    return true;
  }

  // src/core/io/ket/multitailArrowsValidator.ts
  var validateMultitailArrows = (json) => {
    const nodes = json.root.nodes;
    return nodes.every((node) => {
      if (node.type === MULTITAIL_ARROW_SERIALIZE_KEY) {
        const result = MultitailArrow.validateKetNode(
          node.data
        );
        if (result !== null) {
          console.info(result);
          return null;
        }
      }
      return true;
    });
  };

  // src/core/io/ket/validate.ts
  function validate(ket) {
    const result = compiledSchema(ket);
    return result ? validateMultitailArrows(ket) : result;
  }

  // src/core/io/ket/ketSerializer.ts
  var import_lodash11 = __toESM(require_lodash2());

  // src/core/io/ket/toKet/imageToKet.ts
  function imageToKet(imageNode) {
    return {
      type: IMAGE_SERIALIZE_KEY,
      format: imageNode.format,
      boundingBox: imageNode.boundingBox,
      data: imageNode.data,
      selected: imageNode.selected
    };
  }

  // src/core/io/ket/toKet/multitailArrowToKet.ts
  function multitailArrowToKet(node) {
    return {
      type: MULTITAIL_ARROW_SERIALIZE_KEY,
      data: node.data,
      selected: node.selected
    };
  }

  // src/core/io/ket/ketSerializer.ts
  var import_monomers24 = __toESM(require_monomers());
  function parseNode2(node, struct) {
    const type = node.type;
    switch (type) {
      case "arrow":
      case "plus": {
        rxnToStruct(node, struct);
        break;
      }
      case "simpleObject": {
        simpleObjectToStruct(node, struct);
        break;
      }
      case "molecule": {
        const currentStruct = moleculeToStruct(node);
        if (node.stereoFlagPosition) {
          const fragment = currentStruct.frags.get(0);
          fragment.stereoFlagPosition = new Vec2(node.stereoFlagPosition);
        }
        currentStruct.mergeInto(struct);
        break;
      }
      case "rgroup": {
        rgroupToStruct(node).mergeInto(struct);
        break;
      }
      case "text": {
        textToStruct(node, struct);
        break;
      }
      case MULTITAIL_ARROW_SERIALIZE_KEY: {
        multitailArrowToStruct(node, struct);
        break;
      }
      case IMAGE_SERIALIZE_KEY: {
        imageToStruct(node, struct);
        break;
      }
      default:
        break;
    }
  }
  var _KetSerializer = class _KetSerializer {
    static setMonomerFactory(factory) {
      _KetSerializer._monomerFactory = factory;
    }
    static getMonomerFactory() {
      if (!_KetSerializer._monomerFactory) {
        throw String(
          "KetSerializer: monomerFactory has not been initialized. Call KetSerializer.setMonomerFactory() before using serializer features that require it."
        );
      }
      return _KetSerializer._monomerFactory;
    }
    deserializeMicromolecules(content) {
      const ket = JSON.parse(content);
      if (!validate(ket)) {
        throw String("Cannot deserialize input JSON.");
      }
      return _KetSerializer.fillStruct(ket);
    }
    static fillStruct(ket) {
      var _a, _b;
      const resultingStruct = new Struct();
      const nodes = ket.root.nodes;
      Object.keys(nodes).forEach((i) => {
        if (nodes[i].type) parseNode2(nodes[i], resultingStruct);
        else if (nodes[i].$ref) parseNode2(ket[nodes[i].$ref], resultingStruct);
      });
      resultingStruct.name = (_b = (_a = ket.header) == null ? void 0 : _a.moleculeName) != null ? _b : "";
      return resultingStruct;
    }
    serializeMicromolecules(struct, monomer) {
      const result = {
        root: {
          nodes: []
        }
      };
      const header = headerToKet(struct);
      if (header) result.header = header;
      const ketNodes = prepareStructForKet(struct);
      let moleculeId = 0;
      ketNodes.forEach((item) => {
        switch (item.type) {
          case "molecule": {
            result.root.nodes.push({ $ref: `mol${moleculeId}` });
            result[`mol${moleculeId++}`] = moleculeToKet(item.fragment, monomer);
            break;
          }
          case "rgroup": {
            result.root.nodes.push({ $ref: `rg${item.data.rgnumber}` });
            result[`rg${item.data.rgnumber}`] = rgroupToKet(
              item.fragment,
              item.data
            );
            break;
          }
          case "plus": {
            result.root.nodes.push(plusToKet(item));
            break;
          }
          case "arrow": {
            result.root.nodes.push(arrowToKet(item));
            break;
          }
          case "simpleObject": {
            result.root.nodes.push(simpleObjectToKet(item));
            break;
          }
          case "text": {
            result.root.nodes.push(textToKet(item));
            break;
          }
          case IMAGE_SERIALIZE_KEY: {
            result.root.nodes.push(imageToKet(item));
            break;
          }
          case MULTITAIL_ARROW_SERIALIZE_KEY:
            result.root.nodes.push(multitailArrowToKet(item));
            break;
          default:
            break;
        }
      });
      return JSON.stringify(__spreadValues({ ket_version: "2.0.0" }, result), null, 4);
    }
    validateMonomerNodeTemplate(node, parsedFileContent, editor) {
      const template = parsedFileContent[setMonomerTemplatePrefix(node.templateId)];
      if (!template) {
        editor.events.error.dispatch("Error during file parsing");
        return true;
      }
      return false;
    }
    validateConnectionTypeAndEndpoints(connection, editor) {
      if (connection.connectionType !== KetConnectionType.SINGLE && connection.connectionType !== KetConnectionType.HYDROGEN) {
        editor.events.error.dispatch("Error during file parsing");
        return true;
      }
      return false;
    }
    parseAndValidateMacromolecules(fileContent) {
      var _a;
      const editor = provideEditorInstance();
      let parsedFileContent;
      try {
        parsedFileContent = JSON.parse(fileContent);
      } catch (e) {
        import_utilities10.sketchLogger.error(
          "ketSerializer.ts::KetSerializer::parseAndValidateMacromolecules",
          e
        );
        return { error: true };
      }
      let error = false;
      parsedFileContent.root.nodes.forEach((node) => {
        const nodeDefinition = parsedFileContent[node.$ref];
        if ((nodeDefinition == null ? void 0 : nodeDefinition.type) === "monomer") {
          error = this.validateMonomerNodeTemplate(
            nodeDefinition,
            parsedFileContent,
            editor
          );
        }
      });
      if (error) {
        return { error: true };
      }
      (_a = parsedFileContent.root.connections) == null ? void 0 : _a.forEach(
        (connection) => {
          this.validateConnectionTypeAndEndpoints(connection, editor);
        }
      );
      return {
        error,
        parsedFileContent
      };
    }
    deserializeToStruct(fileContent) {
      const struct = new Struct();
      const deserializedContent = this.deserializeToDrawingEntities(fileContent);
      assert_default(deserializedContent);
      MacromoleculesConverter.convertDrawingEntitiesToStruct(
        deserializedContent == null ? void 0 : deserializedContent.drawingEntitiesManager,
        struct
      );
      return struct;
    }
    filterMacromoleculesContent(parsedFileContent) {
      var _a;
      const fileContentForMicromolecules = __spreadProps(__spreadValues({}, parsedFileContent), {
        root: {
          nodes: parsedFileContent.root.nodes.filter((node) => {
            const nodeDefinition = parsedFileContent[node.$ref];
            return (nodeDefinition == null ? void 0 : nodeDefinition.type) !== KetNodeType.MONOMER && (nodeDefinition == null ? void 0 : nodeDefinition.type) !== KetNodeType.AMBIGUOUS_MONOMER;
          })
        }
      });
      parsedFileContent.root.nodes.forEach((node) => {
        const nodeDefinition = parsedFileContent[node.$ref];
        if ((nodeDefinition == null ? void 0 : nodeDefinition.type) === KetNodeType.MONOMER || (nodeDefinition == null ? void 0 : nodeDefinition.type) === KetNodeType.AMBIGUOUS_MONOMER) {
          fileContentForMicromolecules[node.$ref] = void 0;
        }
      });
      (_a = parsedFileContent.root.templates) == null ? void 0 : _a.forEach((template) => {
        fileContentForMicromolecules[template.$ref] = void 0;
      });
      Object.entries(
        fileContentForMicromolecules
      ).forEach(([key, value]) => {
        if ((value == null ? void 0 : value.type) === KetTemplateType2.AMBIGUOUS_MONOMER_TEMPLATE) {
          fileContentForMicromolecules[key] = void 0;
        }
      });
      return fileContentForMicromolecules;
    }
    static getTemplateAttachmentPoints(template) {
      return getTemplateAttachmentPoints(template);
    }
    static enrichTemplateWithLibraryData(template) {
      var _a;
      if (template.idtAliases && template.aliasAxoLabs) return;
      const library = (_a = provideEditorInstance()) == null ? void 0 : _a.monomersLibraryParsedJson;
      if (!library) return;
      const libraryTemplate = library[setMonomerTemplatePrefix(template.id)];
      if (!libraryTemplate) return;
      if (!template.idtAliases && libraryTemplate.idtAliases) {
        template.idtAliases = libraryTemplate.idtAliases;
      }
      if (!template.aliasAxoLabs && libraryTemplate.aliasAxoLabs) {
        template.aliasAxoLabs = libraryTemplate.aliasAxoLabs;
      }
    }
    static convertMonomerTemplateToStruct(template) {
      return convertMonomerTemplateToStruct(template);
    }
    convertMonomerTemplateToLibraryItem(template) {
      var _a;
      const monomerLibraryItem = {
        label: (_a = template.alias) != null ? _a : template.id,
        struct: _KetSerializer.convertMonomerTemplateToStruct(template),
        props: templateToMonomerProps(template),
        attachmentPoints: _KetSerializer.getTemplateAttachmentPoints(template)
      };
      _KetSerializer.fillStructRgLabelsByMonomerTemplate(
        template,
        monomerLibraryItem
      );
      return monomerLibraryItem;
    }
    static fillStructRgLabelsByMonomerTemplate(template, monomerItem) {
      return fillStructRgLabelsByMonomerTemplate(template, monomerItem);
    }
    deserializeToDrawingEntities(fileContent) {
      var _a;
      const { error: hasValidationErrors, parsedFileContent } = this.parseAndValidateMacromolecules(fileContent);
      if (hasValidationErrors || !parsedFileContent) return;
      const command = new Command();
      const drawingEntitiesManager = new DrawingEntitiesManager();
      const monomerIdsMap = {};
      parsedFileContent.root.nodes.forEach((node) => {
        const nodeDefinition = parsedFileContent[node.$ref];
        switch (nodeDefinition == null ? void 0 : nodeDefinition.type) {
          case KetNodeType.MONOMER: {
            const template = parsedFileContent[setMonomerTemplatePrefix(nodeDefinition.templateId)];
            assert_default(template);
            _KetSerializer.enrichTemplateWithLibraryData(template);
            const struct = _KetSerializer.convertMonomerTemplateToStruct(template);
            const monomerAdditionCommand = monomerToDrawingEntity(
              nodeDefinition,
              template,
              struct,
              drawingEntitiesManager
            );
            const monomer = monomerAdditionCommand.operations[0].monomer;
            monomerIdsMap[node.$ref] = monomer == null ? void 0 : monomer.id;
            _KetSerializer.fillStructRgLabelsByMonomerTemplate(
              template,
              monomer.monomerItem
            );
            command.merge(monomerAdditionCommand);
            break;
          }
          case KetNodeType.AMBIGUOUS_MONOMER: {
            const template = parsedFileContent[setAmbiguousMonomerTemplatePrefix(nodeDefinition.templateId)];
            assert_default(template);
            const monomerAdditionCommand = variantMonomerToDrawingEntity(
              drawingEntitiesManager,
              nodeDefinition,
              template,
              parsedFileContent,
              _KetSerializer.getMonomerFactory()
            );
            const monomer = monomerAdditionCommand.operations[0].monomer;
            monomerIdsMap[node.$ref] = monomer == null ? void 0 : monomer.id;
            command.merge(monomerAdditionCommand);
            break;
          }
          default:
            break;
        }
      });
      const fileContentForMicromolecules = this.filterMacromoleculesContent(parsedFileContent);
      const deserializedMicromolecules = this.deserializeMicromolecules(
        JSON.stringify(fileContentForMicromolecules)
      );
      const structToDrawingEntitiesConversionResult = MacromoleculesConverter.convertStructToDrawingEntities(
        deserializedMicromolecules,
        drawingEntitiesManager
      );
      const localAtomIdToGlobalAtomId = /* @__PURE__ */ new Map();
      command.merge(structToDrawingEntitiesConversionResult.modelChanges);
      structToDrawingEntitiesConversionResult.fragmentIdToMonomer.forEach(
        (monomer, fragmentId) => {
          monomerIdsMap[`mol${fragmentId}`] = monomer.id;
        }
      );
      structToDrawingEntitiesConversionResult.fragmentIdToAtomIdMap.forEach(
        (_atomIdsMap) => {
          _atomIdsMap.forEach((globalAtomId, localAtomId) => {
            localAtomIdToGlobalAtomId.set(localAtomId, globalAtomId);
          });
        }
      );
      const superatomMonomerToUsedAttachmentPoint = /* @__PURE__ */ new Map();
      (_a = parsedFileContent.root.connections) == null ? void 0 : _a.forEach((connection) => {
        var _a2, _b, _c, _d;
        switch (connection.connectionType) {
          case KetConnectionType.SINGLE: {
            const firstMonomer = drawingEntitiesManager.monomers.get(
              Number(
                monomerIdsMap[(_a2 = connection.endpoint1.monomerId) != null ? _a2 : connection.endpoint1.moleculeId]
              )
            );
            const secondMonomer = drawingEntitiesManager.monomers.get(
              Number(
                monomerIdsMap[(_b = connection.endpoint2.monomerId) != null ? _b : connection.endpoint2.moleculeId]
              )
            );
            if (!firstMonomer || !secondMonomer) {
              return;
            }
            if (!(0, import_monomers24.isMonomerSgroupWithAttachmentPoints)(firstMonomer) && !(0, import_monomers24.isMonomerSgroupWithAttachmentPoints)(secondMonomer) && (firstMonomer.monomerItem.props.isMicromoleculeFragment || secondMonomer.monomerItem.props.isMicromoleculeFragment)) {
              const atomId = Number(
                (_c = connection.endpoint1.atomId) != null ? _c : connection.endpoint2.atomId
              );
              const atom = MacromoleculesConverter.findAtomByMicromoleculeAtomId(
                drawingEntitiesManager,
                atomId,
                firstMonomer.monomerItem.props.isMicromoleculeFragment ? firstMonomer : secondMonomer
              );
              const attachmentPointName = (_d = connection.endpoint1.attachmentPointId) != null ? _d : connection.endpoint2.attachmentPointId;
              if (!atom || !attachmentPointName) {
                return;
              }
              const bondAdditionCommand = drawingEntitiesManager.addMonomerToAtomBond(
                firstMonomer.monomerItem.props.isMicromoleculeFragment ? secondMonomer : firstMonomer,
                atom,
                attachmentPointName
              );
              command.merge(bondAdditionCommand);
            } else {
              const bondAdditionCommand = polymerBondToDrawingEntity(
                connection,
                drawingEntitiesManager,
                localAtomIdToGlobalAtomId,
                superatomMonomerToUsedAttachmentPoint,
                firstMonomer,
                secondMonomer
              );
              command.merge(bondAdditionCommand);
            }
            break;
          }
          case KetConnectionType.HYDROGEN: {
            const firstMonomer = drawingEntitiesManager.monomers.get(
              Number(monomerIdsMap[connection.endpoint1.monomerId])
            );
            const secondMonomer = drawingEntitiesManager.monomers.get(
              Number(monomerIdsMap[connection.endpoint2.monomerId])
            );
            if (!firstMonomer || !secondMonomer) {
              return;
            }
            command.merge(
              drawingEntitiesManager.createPolymerBond(
                firstMonomer,
                secondMonomer,
                "hydrogen" /* HYDROGEN */,
                "hydrogen" /* HYDROGEN */,
                MACROMOLECULES_BOND_TYPES2.HYDROGEN
              )
            );
            break;
          }
          default:
            break;
        }
      });
      return { modelChanges: command, drawingEntitiesManager };
    }
    deserialize(fileContent) {
      return this.deserializeToStruct(fileContent);
    }
    getConnectionMonomerEndpoint(monomer, polymerBond, monomerIdMap) {
      const monomerId = monomerIdMap.get(monomer.id);
      return {
        monomerId: setMonomerPrefix((0, import_lodash11.isNumber)(monomerId) ? monomerId : monomer.id),
        attachmentPointId: polymerBond instanceof HydrogenBond ? void 0 : monomer.getAttachmentPointByBond(polymerBond)
      };
    }
    getConnectionMoleculeEndpoint(monomer, polymerBond, monomerToAtomIdMap, struct) {
      var _a;
      const { attachmentAtomId, globalAttachmentAtomId } = MacromoleculesConverter.findAttachmentPointAtom(
        polymerBond,
        monomer,
        monomerToAtomIdMap
      );
      return {
        moleculeId: `mol${(_a = struct.atoms.get(globalAttachmentAtomId)) == null ? void 0 : _a.fragment}`,
        atomId: `${attachmentAtomId}`
      };
    }
    serializeMonomerTemplate(templateId, monomer, fileContent) {
      var _a;
      const [, , monomerClass] = _KetSerializer.getMonomerFactory()(
        monomer.monomerItem
      );
      const templateNameWithPrefix = setMonomerTemplatePrefix(templateId);
      if (fileContent[templateNameWithPrefix]) {
        return;
      }
      fileContent[templateNameWithPrefix] = __spreadProps(__spreadValues({}, JSON.parse(
        this.serializeMicromolecules(monomer.monomerItem.struct, monomer)
      ).mol0), {
        type: "monomerTemplate",
        class: (_a = monomer.monomerItem.props.MonomerClass) != null ? _a : monomerClass,
        classHELM: monomer.monomerItem.props.MonomerType,
        id: templateId,
        fullName: monomer.monomerItem.props.Name,
        alias: monomer.monomerItem.label,
        aliasHELM: monomer.monomerItem.props.aliasHELM,
        aliasAxoLabs: monomer.monomerItem.props.aliasAxoLabs,
        attachmentPoints: monomer.monomerItem.attachmentPoints,
        idtAliases: monomer.monomerItem.props.idtAliases,
        unresolved: monomer.monomerItem.props.unresolved ? true : void 0,
        modificationTypes: monomer.monomerItem.props.modificationTypes
      });
      if (monomer.monomerItem.props.MonomerType !== "CHEM") {
        fileContent[templateNameWithPrefix].naturalAnalogShort = monomer.monomerItem.props.MonomerNaturalAnalogCode;
      }
      fileContent.root.templates.push(getKetRef(templateNameWithPrefix));
    }
    serializeVariantMonomerTemplate(templateId, variantMonomer, fileContent) {
      const templateNameWithPrefix = setAmbiguousMonomerTemplatePrefix(templateId);
      if (fileContent[templateNameWithPrefix]) {
        return;
      }
      fileContent[templateNameWithPrefix] = {
        type: "ambiguousMonomerTemplate",
        id: templateId,
        alias: variantMonomer.label,
        idtAliases: variantMonomer.variantMonomerItem.idtAliases,
        subtype: variantMonomer.variantMonomerItem.subtype,
        options: variantMonomer.variantMonomerItem.options
      };
      fileContent.root.templates.push(getKetRef(templateNameWithPrefix));
      variantMonomer.monomers.forEach((monomer) => {
        var _a;
        const monomerTemplateId = (_a = monomer.monomerItem.props.id) != null ? _a : getMonomerUniqueKey(monomer.monomerItem);
        this.serializeMonomerTemplate(monomerTemplateId, monomer, fileContent);
      });
    }
    serializeMacromolecules(struct, drawingEntitiesManager, needSetSelection = false) {
      const fileContent = {
        root: {
          nodes: [],
          connections: [],
          templates: []
        }
      };
      const monomerToAtomIdMap = /* @__PURE__ */ new Map();
      const monomerToBondIdMap = /* @__PURE__ */ new Map();
      const moleculesSelection = {
        atoms: [],
        bonds: []
      };
      const monomerIdMap = /* @__PURE__ */ new Map();
      let nextMonomerId = 0;
      drawingEntitiesManager.monomers.forEach((monomer) => {
        var _a;
        const monomerItem = monomer.monomerItem;
        if (monomer instanceof Chem && monomerItem.props.isMicromoleculeFragment) {
          const atomIdMap = /* @__PURE__ */ new Map();
          const bondIdMap = /* @__PURE__ */ new Map();
          monomerItem.struct.mergeInto(
            struct,
            null,
            null,
            false,
            false,
            atomIdMap,
            null,
            null,
            null,
            null,
            null,
            bondIdMap
          );
          monomerToAtomIdMap.set(monomer, atomIdMap);
          monomerToBondIdMap.set(monomer, bondIdMap);
        } else {
          let templateId;
          const monomerKey = setMonomerPrefix(nextMonomerId);
          const position = switchIntoChemistryCoordSystem(
            new Vec2(monomer.position.x, monomer.position.y)
          );
          monomerIdMap.set(monomer.id, nextMonomerId);
          if (monomer instanceof AmbiguousMonomer) {
            const ambiguousMonomerItem = monomer.variantMonomerItem;
            templateId = ambiguousMonomerItem.subtype + "_" + ambiguousMonomerItem.options.reduce(
              (templateId2, option) => {
                var _a2, _b;
                return templateId2 + "_" + option.templateId + "_" + ((_b = (_a2 = option.probability) != null ? _a2 : option.ratio) != null ? _b : "");
              },
              ""
            );
          } else {
            templateId = (_a = monomerItem.props.id) != null ? _a : getMonomerUniqueKey(monomerItem);
          }
          const { seqId, expanded, transformation } = monomerItem;
          const isExpandedDefined = expanded !== void 0;
          const isTransformationDefined = transformation !== void 0 && Object.keys(transformation).length > 0;
          fileContent[monomerKey] = __spreadProps(__spreadValues(__spreadValues({
            type: monomer instanceof AmbiguousMonomer ? KetNodeType.AMBIGUOUS_MONOMER : KetNodeType.MONOMER,
            id: nextMonomerId.toString(),
            position: {
              x: position.x,
              y: position.y
            },
            alias: monomer.label,
            templateId,
            seqid: seqId
          }, isExpandedDefined && {
            expanded
          }), isTransformationDefined && {
            transformation: modifyTransformation(transformation)
          }), {
            selected: needSetSelection && monomer.selected || void 0
          });
          fileContent.root.nodes.push(getKetRef(monomerKey));
          nextMonomerId++;
          if (monomer instanceof AmbiguousMonomer) {
            this.serializeVariantMonomerTemplate(
              templateId,
              monomer,
              fileContent
            );
          } else {
            this.serializeMonomerTemplate(templateId, monomer, fileContent);
          }
        }
      });
      drawingEntitiesManager.polymerBonds.forEach((polymerBond) => {
        assert_default(polymerBond.secondMonomer);
        fileContent.root.connections.push({
          connectionType: polymerBond instanceof HydrogenBond ? KetConnectionType.HYDROGEN : KetConnectionType.SINGLE,
          endpoint1: polymerBond.firstMonomer.monomerItem.props.isMicromoleculeFragment ? this.getConnectionMoleculeEndpoint(
            polymerBond.firstMonomer,
            polymerBond,
            monomerToAtomIdMap,
            struct
          ) : this.getConnectionMonomerEndpoint(
            polymerBond.firstMonomer,
            polymerBond,
            monomerIdMap
          ),
          endpoint2: polymerBond.secondMonomer.monomerItem.props.isMicromoleculeFragment ? this.getConnectionMoleculeEndpoint(
            polymerBond.secondMonomer,
            polymerBond,
            monomerToAtomIdMap,
            struct
          ) : this.getConnectionMonomerEndpoint(
            polymerBond.secondMonomer,
            polymerBond,
            monomerIdMap
          ),
          selected: needSetSelection && polymerBond.selected || void 0
        });
      });
      drawingEntitiesManager.monomerToAtomBonds.forEach((monomerToAtomBond) => {
        var _a;
        const monomer = monomerToAtomBond.monomer;
        const atomIdMap = monomerToAtomIdMap.get(monomerToAtomBond.atom.monomer);
        const globalAtomId = atomIdMap == null ? void 0 : atomIdMap.get(
          monomerToAtomBond.atom.atomIdInMicroMode
        );
        const monomerId = monomerIdMap.get(monomer.id);
        if (!(0, import_lodash11.isNumber)(globalAtomId) || !(0, import_lodash11.isNumber)(monomerId)) {
          return;
        }
        fileContent.root.connections.push({
          connectionType: KetConnectionType.SINGLE,
          endpoint1: {
            monomerId: setMonomerPrefix(monomerId),
            attachmentPointId: monomerToAtomBond.monomer.getAttachmentPointByBond(
              monomerToAtomBond
            )
          },
          endpoint2: {
            moleculeId: `mol${(_a = struct.atoms.get(globalAtomId)) == null ? void 0 : _a.fragment}`,
            atomId: String(monomerToAtomBond.atom.atomIdInMicroMode)
          },
          selected: needSetSelection && monomerToAtomBond.selected || void 0
        });
      });
      if (needSetSelection) {
        drawingEntitiesManager.atoms.forEach((atom) => {
          if (atom.selected) {
            const atomIdMap = monomerToAtomIdMap.get(atom.monomer);
            const globalAtomId = atomIdMap == null ? void 0 : atomIdMap.get(atom.atomIdInMicroMode);
            if ((0, import_lodash11.isNumber)(globalAtomId)) {
              moleculesSelection.atoms.push(globalAtomId);
            }
          }
        });
        drawingEntitiesManager.bonds.forEach((bond) => {
          if (bond.selected) {
            const bondIdMap = monomerToBondIdMap.get(bond.firstAtom.monomer);
            const globalBondId = bondIdMap == null ? void 0 : bondIdMap.get(bond.bondIdInMicroMode);
            if ((0, import_lodash11.isNumber)(globalBondId)) {
              moleculesSelection.bonds.push(globalBondId);
            }
          }
        });
      }
      drawingEntitiesManager.rxnArrows.forEach((rxnArrow) => {
        const arrow = new RxnArrow({
          mode: rxnArrow.type,
          pos: [rxnArrow.startPosition, rxnArrow.endPosition],
          height: rxnArrow.height,
          initiallySelected: rxnArrow.initiallySelected,
          arrowId: rxnArrow.arrowId
        });
        struct.addRxnArrow(arrow);
      });
      drawingEntitiesManager.multitailArrows.forEach((multitailArrow) => {
        const arrow = MultitailArrow.fromKetNode(
          multitailArrow.toKetNode()
        );
        arrow.arrowId = multitailArrow.arrowId;
        struct.addMultitailArrow(arrow);
      });
      drawingEntitiesManager.rxnPluses.forEach((rxnPlus) => {
        const micromoleculeRxnPlus = new RxnPlus({
          pp: rxnPlus.position,
          initiallySelected: rxnPlus.initiallySelected
        });
        struct.rxnPluses.add(micromoleculeRxnPlus);
      });
      drawingEntitiesManager.micromoleculesHiddenEntities.mergeInto(struct);
      return {
        serializedMacromolecules: fileContent,
        micromoleculesStruct: struct,
        moleculesSelection
      };
    }
    static removeLeavingGroupsFromConnectedAtoms(_struct) {
      const struct = _struct.clone();
      struct.atoms.forEach((_atom, atomId) => {
        if (Atom2.isHiddenLeavingGroupAtom(struct, atomId, false, true)) {
          struct.atoms.delete(atomId);
        }
      });
      struct.bonds.forEach((bond, bondId) => {
        if (Bond3.isBondToHiddenLeavingGroup(struct, bond)) {
          struct.bonds.delete(bondId);
        }
      });
      struct.sgroups.forEach((sgroup) => {
        const attachmentPoints = sgroup.getAttachmentPoints();
        const attachmentPointsToReplace = /* @__PURE__ */ new Map();
        attachmentPoints.forEach((attachmentPoint) => {
          if ((0, import_lodash11.isNumber)(attachmentPoint.leaveAtomId) && Atom2.isHiddenLeavingGroupAtom(
            struct,
            attachmentPoint.leaveAtomId,
            true,
            true
          )) {
            const attachmentPointClone = new SGroupAttachmentPoint(
              attachmentPoint.atomId,
              void 0,
              attachmentPoint.attachmentId,
              attachmentPoint.attachmentPointNumber
            );
            attachmentPointsToReplace.set(attachmentPoint, attachmentPointClone);
            sgroup.atoms.splice(
              sgroup.atoms.indexOf(attachmentPoint.leaveAtomId),
              1
            );
          }
        });
        attachmentPointsToReplace.forEach(
          (attachmentPointToAdd, attachmentPointToDelete) => {
            sgroup.removeAttachmentPoint(attachmentPointToDelete);
            sgroup.addAttachmentPoint(attachmentPointToAdd, false);
          }
        );
      });
      return struct;
    }
    serialize(_struct, drawingEntitiesManager = new DrawingEntitiesManager(), selection, isBeautified = true, needSetSelectionToMacromolecules = false) {
      const struct = _KetSerializer.removeLeavingGroupsFromConnectedAtoms(_struct);
      struct.enableInitiallySelected();
      const populatedStruct = populateStructWithSelection(
        struct,
        selection,
        true
      );
      MacromoleculesConverter.convertStructToDrawingEntities(
        populatedStruct,
        drawingEntitiesManager
      );
      const {
        serializedMacromolecules,
        micromoleculesStruct,
        moleculesSelection
      } = this.serializeMacromolecules(
        new Struct(),
        drawingEntitiesManager,
        needSetSelectionToMacromolecules
      );
      if (selection === void 0) {
        micromoleculesStruct.enableInitiallySelected();
      }
      if (needSetSelectionToMacromolecules) {
        populateStructWithSelection(micromoleculesStruct, moleculesSelection);
      }
      const serializedMicromoleculesStruct = JSON.parse(
        this.serializeMicromolecules(micromoleculesStruct)
      );
      micromoleculesStruct.disableInitiallySelected();
      const fileContent = __spreadValues(__spreadValues({}, serializedMicromoleculesStruct), serializedMacromolecules);
      fileContent.root.nodes = [
        ...serializedMacromolecules.root.nodes,
        ...serializedMicromoleculesStruct.root.nodes
      ];
      return JSON.stringify(
        fileContent,
        null,
        isBeautified ? 4 : void 0
      );
    }
    convertMonomersLibrary(monomersLibrary) {
      const library = [];
      monomersLibrary.root.templates.forEach((templateRef) => {
        var _a;
        const template = monomersLibrary[templateRef.$ref];
        if (!template) {
          import_utilities10.sketchLogger.error(
            `There is a ref for monomer template ${templateRef.$ref}, but template definition is not found`
          );
          return;
        }
        switch (template.type) {
          case KetTemplateType2.MONOMER_TEMPLATE: {
            library.push(
              this.convertMonomerTemplateToLibraryItem(
                template
              )
            );
            break;
          }
          case KetTemplateType2.AMBIGUOUS_MONOMER_TEMPLATE: {
            const variantMonomerTemplate = template;
            const variantMonomerLibraryItem = {
              id: variantMonomerTemplate.id,
              label: (_a = variantMonomerTemplate.alias) != null ? _a : "%",
              idtAliases: variantMonomerTemplate.idtAliases,
              isAmbiguous: true,
              monomers: createMonomersForVariantMonomer(
                variantMonomerTemplate,
                monomersLibrary,
                _KetSerializer.getMonomerFactory()
              ),
              options: variantMonomerTemplate.options,
              subtype: variantMonomerTemplate.subtype
            };
            library.push(variantMonomerLibraryItem);
            break;
          }
        }
      });
      return library;
    }
  };
  __publicField(_KetSerializer, "_monomerFactory", null);
  var KetSerializer = _KetSerializer;

  // src/core/io/mol/utils.js
  function paddedNum(number, width, precision) {
    const parsedNumber = parseFloat(number);
    const numStr = parsedNumber.toFixed(precision || 0).replace(",", ".");
    if (numStr.length > width) throw String("number does not fit");
    return numStr.padStart(width);
  }
  function parseDecimalInt(str) {
    const val = parseInt(str, 10);
    return isNaN(val) ? 0 : val;
  }
  function partitionLine(str, parts, withspace) {
    const res = [];
    for (let i = 0, shift = 0; i < parts.length; ++i) {
      res.push(str.slice(shift, shift + parts[i]));
      if (withspace) shift++;
      shift += parts[i];
    }
    return res;
  }
  function partitionLineFixed(str, itemLength, withspace) {
    const res = [];
    const step = withspace ? itemLength + 1 : itemLength;
    let shift = 0;
    while (shift < str.length) {
      res.push(str.slice(shift, shift + itemLength));
      shift += step;
    }
    return res;
  }
  var fmtInfo = {
    bondTypeMap: {
      1: Bond3.PATTERN.TYPE.SINGLE,
      2: Bond3.PATTERN.TYPE.DOUBLE,
      3: Bond3.PATTERN.TYPE.TRIPLE,
      4: Bond3.PATTERN.TYPE.AROMATIC,
      5: Bond3.PATTERN.TYPE.SINGLE_OR_DOUBLE,
      6: Bond3.PATTERN.TYPE.SINGLE_OR_AROMATIC,
      7: Bond3.PATTERN.TYPE.DOUBLE_OR_AROMATIC,
      8: Bond3.PATTERN.TYPE.ANY,
      9: Bond3.PATTERN.TYPE.DATIVE,
      10: Bond3.PATTERN.TYPE.HYDROGEN
    },
    bondStereoMap: {
      0: Bond3.PATTERN.STEREO.NONE,
      1: Bond3.PATTERN.STEREO.UP,
      4: Bond3.PATTERN.STEREO.EITHER,
      6: Bond3.PATTERN.STEREO.DOWN,
      3: Bond3.PATTERN.STEREO.CIS_TRANS
    },
    v30bondStereoMap: {
      0: Bond3.PATTERN.STEREO.NONE,
      1: Bond3.PATTERN.STEREO.UP,
      2: Bond3.PATTERN.STEREO.EITHER,
      3: Bond3.PATTERN.STEREO.DOWN
    },
    bondTopologyMap: {
      0: Bond3.PATTERN.TOPOLOGY.EITHER,
      1: Bond3.PATTERN.TOPOLOGY.RING,
      2: Bond3.PATTERN.TOPOLOGY.CHAIN
    },
    countsLinePartition: [3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6],
    atomLinePartition: [10, 10, 10, 1, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3],
    bondLinePartition: [3, 3, 3, 3, 3, 3, 3],
    atomListHeaderPartition: [3, 1, 1, 4, 1, 1],
    atomListHeaderLength: 11,
    // = atomListHeaderPartition.reduce(function(a,b) { return a + b; }, 0)
    atomListHeaderItemLength: 4,
    chargeMap: [null, 3, 2, 1, null, -1, -2, -3],
    valenceMap: [void 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0],
    implicitHydrogenMap: [void 0, 0, 1, 2, 3, 4],
    v30atomPropMap: {
      CHG: "charge",
      RAD: "radical",
      MASS: "isotope",
      VAL: "explicitValence",
      HCOUNT: "hCount",
      INVRET: "invRet",
      SUBST: "substitutionCount",
      UNSAT: "unsaturatedAtom",
      RBCNT: "ringBondCount"
    },
    rxnItemsPartition: [3, 3, 3]
  };
  var FRAGMENT = {
    NONE: 0,
    REACTANT: 1,
    PRODUCT: 2,
    AGENT: 3
  };
  var SHOULD_RESCALE_MOLECULES = true;
  function calculateAverageBondLength(mols) {
    const bondLengthData = { cnt: 0, totalLength: 0 };
    for (const mol of mols) {
      const bondLengthDataMol = mol.getBondLengthData();
      bondLengthData.cnt += bondLengthDataMol.cnt;
      bondLengthData.totalLength += bondLengthDataMol.totalLength;
    }
    return bondLengthData.cnt === 0 ? 1 : bondLengthData.totalLength / bondLengthData.cnt;
  }
  function rescaleMolecules(mols) {
    const avgBondLength = calculateAverageBondLength(mols);
    const scaleFactor = 1 / avgBondLength;
    for (const mol of mols) {
      mol.scale(scaleFactor);
    }
  }
  function getFragmentType(index, nReactants, nProducts) {
    if (index < nReactants) {
      return FRAGMENT.REACTANT;
    } else if (index < nReactants + nProducts) {
      return FRAGMENT.PRODUCT;
    } else {
      return FRAGMENT.AGENT;
    }
  }
  function categorizeMolecules(mols, nReactants, nProducts) {
    const bbReact = [];
    const bbAgent = [];
    const bbProd = [];
    const molReact = [];
    const molAgent = [];
    const molProd = [];
    for (let j = 0; j < mols.length; ++j) {
      const mol = mols[j];
      const bb = mol.getCoordBoundingBoxObj();
      if (!bb) continue;
      const fragmentType = getFragmentType(j, nReactants, nProducts);
      if (fragmentType === FRAGMENT.REACTANT) {
        bbReact.push(bb);
        molReact.push(mol);
      } else if (fragmentType === FRAGMENT.AGENT) {
        bbAgent.push(bb);
        molAgent.push(mol);
      } else if (fragmentType === FRAGMENT.PRODUCT) {
        bbProd.push(bb);
        molProd.push(mol);
      }
      mol.atoms.forEach((atom) => {
        atom.rxnFragmentType = fragmentType;
      });
    }
    return { bbReact, bbAgent, bbProd, molReact, molAgent, molProd };
  }
  function shiftMol(ret, mol, bb, xorig, over) {
    const d = new Vec2(
      xorig - bb.min.x,
      over ? 1 - bb.min.y : -(bb.min.y + bb.max.y) / 2
    );
    mol.atoms.forEach((atom) => {
      atom.pp.add_(d);
    });
    mol.sgroups.forEach((item) => {
      if (item.pp) item.pp.add_(d);
    });
    bb.min.add_(d);
    bb.max.add_(d);
    mol.mergeInto(ret);
    return bb.max.x - bb.min.x;
  }
  function layoutReactionFragments(ret, molReact, bbReact, molAgent, bbAgent, molProd, bbProd) {
    let xorig = 0;
    for (let j = 0; j < molReact.length; ++j) {
      xorig += shiftMol(ret, molReact[j], bbReact[j], xorig, false) + 2;
    }
    xorig += 2;
    for (let j = 0; j < molAgent.length; ++j) {
      xorig += shiftMol(ret, molAgent[j], bbAgent[j], xorig, true) + 2;
    }
    xorig += 2;
    for (let j = 0; j < molProd.length; ++j) {
      xorig += shiftMol(ret, molProd[j], bbProd[j], xorig, false) + 2;
    }
  }
  function mergeWithoutLayout(ret, molReact, molAgent, molProd) {
    for (const mol of molReact) mol.mergeInto(ret);
    for (const mol of molAgent) mol.mergeInto(ret);
    for (const mol of molProd) mol.mergeInto(ret);
  }
  function addPlusSigns(ret, boundingBoxes) {
    for (let j = 0; j < boundingBoxes.length - 1; ++j) {
      const bb1 = boundingBoxes[j];
      const bb2 = boundingBoxes[j + 1];
      const x = (bb1.max.x + bb2.min.x) / 2;
      const y = (bb1.max.y + bb1.min.y + bb2.max.y + bb2.min.y) / 4;
      ret.rxnPluses.add(new RxnPlus({ pp: new Vec2(x, y) }));
    }
  }
  function aggregateBoundingBoxes(boundingBoxes) {
    if (boundingBoxes.length === 0) return null;
    const bbAll = {
      max: new Vec2(boundingBoxes[0].max),
      min: new Vec2(boundingBoxes[0].min)
    };
    for (let j = 1; j < boundingBoxes.length; ++j) {
      bbAll.max = Vec2.max(bbAll.max, boundingBoxes[j].max);
      bbAll.min = Vec2.min(bbAll.min, boundingBoxes[j].min);
    }
    return bbAll;
  }
  function createReactionArrow(bb1, bb2) {
    const defaultArrowLength = 2;
    const defaultOffset = 3;
    if (!bb1 && !bb2) {
      return new RxnArrow({
        mode: "open-angle",
        pos: [new Vec2(0, 0), new Vec2(defaultArrowLength, 0)]
      });
    }
    let v1 = bb1 ? new Vec2(bb1.max.x, (bb1.max.y + bb1.min.y) / 2) : null;
    let v2 = bb2 ? new Vec2(bb2.min.x, (bb2.max.y + bb2.min.y) / 2) : null;
    if (!v1) v1 = new Vec2(v2.x - defaultOffset, v2.y);
    if (!v2) v2 = new Vec2(v1.x + defaultOffset, v1.y);
    const arrowCenter = Vec2.lc2(v1, 0.5, v2, 0.5);
    const arrowStart = new Vec2(
      arrowCenter.x - 0.5 * defaultArrowLength,
      arrowCenter.y,
      arrowCenter.z
    );
    const arrowEnd = new Vec2(
      arrowCenter.x + 0.5 * defaultArrowLength,
      arrowCenter.y,
      arrowCenter.z
    );
    return new RxnArrow({
      mode: "open-angle",
      pos: [arrowStart, arrowEnd]
    });
  }
  function rxnMerge(mols, nReactants, nProducts, nAgents, shouldReactionRelayout) {
    const ret = new Struct();
    if (SHOULD_RESCALE_MOLECULES) {
      rescaleMolecules(mols);
    }
    const { bbReact, bbAgent, bbProd, molReact, molAgent, molProd } = categorizeMolecules(mols, nReactants, nProducts);
    if (shouldReactionRelayout) {
      layoutReactionFragments(
        ret,
        molReact,
        bbReact,
        molAgent,
        bbAgent,
        molProd,
        bbProd
      );
    } else {
      mergeWithoutLayout(ret, molReact, molAgent, molProd);
    }
    addPlusSigns(ret, bbReact);
    addPlusSigns(ret, bbProd);
    const bbReactAll = aggregateBoundingBoxes(bbReact);
    const bbProdAll = aggregateBoundingBoxes(bbProd);
    const arrow = createReactionArrow(bbReactAll, bbProdAll);
    ret.addRxnArrow(arrow);
    ret.isReaction = true;
    return ret;
  }
  function rgMerge(scaffold, rgroups) {
    const ret = new Struct();
    scaffold.mergeInto(ret, null, null, false, true);
    Object.keys(rgroups).forEach((id2) => {
      const rgid = parseInt(id2, 10);
      for (const ctab of rgroups[rgid]) {
        ctab.rgroups.set(rgid, new RGroup());
        const frag = new Fragment();
        const frid = ctab.frags.add(frag);
        ctab.rgroups.get(rgid).frags.add(frid);
        ctab.atoms.forEach((atom) => {
          atom.fragment = frid;
        });
        ctab.mergeInto(ret);
      }
    });
    return ret;
  }
  var utils_default = {
    fmtInfo,
    paddedNum,
    parseDecimalInt,
    partitionLine,
    partitionLineFixed,
    rxnMerge,
    rgMerge
  };

  // src/core/io/mol/parseSGroup.ts
  function readKeyValuePairs(str, valueString) {
    const ret = new Pool();
    const partition = utils_default.partitionLineFixed(str, 3, true);
    const count = utils_default.parseDecimalInt(partition[0]);
    for (let i = 0; i < count; ++i) {
      const key = utils_default.parseDecimalInt(partition[2 * i + 1]) - 1;
      const value = valueString ? partition[2 * i + 2].trim() : utils_default.parseDecimalInt(partition[2 * i + 2]);
      ret.set(key, value);
    }
    return ret;
  }
  function readKeyMultiValuePairs(str, valueString) {
    const ret = [];
    const partition = utils_default.partitionLineFixed(str, 3, true);
    const count = utils_default.parseDecimalInt(partition[0]);
    for (let i = 0; i < count; ++i) {
      ret.push([
        /* eslint-disable no-mixed-operators */
        utils_default.parseDecimalInt(partition[2 * i + 1]) - 1,
        valueString ? partition[2 * i + 2].trim() : utils_default.parseDecimalInt(partition[2 * i + 2])
        /* eslint-enable no-mixed-operators */
      ]);
    }
    return ret;
  }
  function postLoadMul(sgroup, mol, atomMap) {
    if (!mol || !atomMap) return;
    sgroup.data.mul = sgroup.data.subscript - 0;
    const atomReductionMap = {};
    sgroup.atoms = SGroup.filterAtoms(sgroup.atoms, atomMap);
    sgroup.patoms = SGroup.filterAtoms(sgroup.patoms, atomMap);
    for (let k = 1; k < sgroup.data.mul; ++k) {
      for (let m = 0; m < sgroup.patoms.length; ++m) {
        const raid = sgroup.atoms[k * sgroup.patoms.length + m];
        if (raid < 0) continue;
        if (sgroup.patoms[m] < 0) throw String("parent atom missing");
        atomReductionMap[raid] = sgroup.patoms[m];
      }
    }
    sgroup.patoms = SGroup.removeNegative(sgroup.patoms);
    const patomsMap = identityMap(sgroup.patoms);
    const bondsToRemove = [];
    mol.bonds.forEach((bond, bid) => {
      const beginIn = bond.begin in atomReductionMap;
      const endIn = bond.end in atomReductionMap;
      const endInPatoms = bond.end in patomsMap;
      const beginInPatoms = bond.begin in patomsMap;
      if (beginIn && endIn || beginIn && endInPatoms || endIn && beginInPatoms) {
        bondsToRemove.push(bid);
      } else if (beginIn) bond.begin = atomReductionMap[bond.begin];
      else if (endIn) bond.end = atomReductionMap[bond.end];
    });
    for (const bondId of bondsToRemove) {
      mol.bonds.delete(bondId);
    }
    for (const a in atomReductionMap) {
      mol.atoms.delete(+a);
      atomMap[a] = -1;
    }
    sgroup.atoms = sgroup.patoms;
    sgroup.patoms = null;
  }
  function postLoadSru(sgroup) {
    sgroup.data.connectivity = (sgroup.data.connectivity || "EU").trim().toLowerCase();
    sgroup.data.subtype = (sgroup.data.subtype || "").trim().toLowerCase();
  }
  function postLoadSup(sgroup) {
    sgroup.data.name = (sgroup.data.subscript || "").trim();
    sgroup.data.subscript = "";
  }
  function postLoadGen(sgroup, _mol, _atomMap) {
    sgroup.data.connectivity = (sgroup.data.connectivity || "eu").trim().toLowerCase();
    sgroup.data.subtype = (sgroup.data.subtype || "").trim().toLowerCase();
  }
  function postLoadDat(sgroup, mol) {
    if (!sgroup.data.absolute && mol) {
      if (!sgroup.pp) {
        throw String("SGroup pp is not set");
      }
      sgroup.pp = sgroup.pp.add(SGroup.getMassCentre(mol, sgroup.atoms));
    }
  }
  function postLoadMon(_sgroup2) {
  }
  function postLoadMer(_sgroup2) {
  }
  function postLoadCop(sgroup) {
    sgroup.data.connectivity = (sgroup.data.connectivity || "eu").trim().toLowerCase();
    sgroup.data.subtype = (sgroup.data.subtype || "").trim().toLowerCase();
  }
  function postLoadCro(_sgroup2) {
  }
  function postLoadMod(_sgroup2) {
  }
  function postLoadGra(_sgroup2) {
  }
  function postLoadCom(_sgroup2) {
  }
  function postLoadMix(_sgroup2) {
  }
  function postLoadFor(_sgroup2) {
  }
  function postLoadAny(_sgroup2) {
  }
  var postLoadMap = {
    SUP: postLoadSup,
    MUL: postLoadMul,
    SRU: postLoadSru,
    MON: postLoadMon,
    MER: postLoadMer,
    COP: postLoadCop,
    CRO: postLoadCro,
    MOD: postLoadMod,
    GRA: postLoadGra,
    COM: postLoadCom,
    MIX: postLoadMix,
    FOR: postLoadFor,
    DAT: postLoadDat,
    ANY: postLoadAny,
    GEN: postLoadGen
  };
  var allowedSGroupTypes = new Set(Object.keys(postLoadMap));
  function loadSGroup(mol, sg, atomMap) {
    sg.id = mol.sgroups.add(sg);
    if (allowedSGroupTypes.has(sg.type)) {
      const handler = postLoadMap[sg.type];
      if (typeof handler === "function") {
        handler(sg, mol, atomMap);
      }
    }
    for (const atomId of sg.atoms) {
      const atom = mol.atoms.get(atomId);
      if (atom) atom.sgs.add(sg.id);
    }
    if (sg.type === "DAT") mol.sGroupForest.insert(sg, -1, []);
    else mol.sGroupForest.insert(sg);
    return sg.id;
  }
  function initSGroup(sGroups, propData) {
    const kv = readKeyValuePairs(propData, true);
    for (const [key, type] of kv) {
      const sg = new SGroup(type);
      sg.number = key;
      sGroups[key] = sg;
    }
  }
  function applySGroupProp(sGroups, propName, propData, numeric, core) {
    const kv = readKeyValuePairs(propData, !numeric);
    for (const key of kv.keys()) {
      (core ? sGroups[key] : sGroups[key].data)[propName] = kv.get(key);
    }
  }
  function applySGroupArrayProp(sGroups, propName, propData, shift) {
    const sid = utils_default.parseDecimalInt(propData.slice(1, 4)) - 1;
    const num = utils_default.parseDecimalInt(propData.slice(4, 8));
    let part = toIntArray(utils_default.partitionLineFixed(propData.slice(8), 3, true));
    if (part.length !== num) throw String("File format invalid");
    if (shift) part = part.map((v) => v + shift);
    sGroups[sid][propName] = sGroups[sid][propName].concat(part);
  }
  function applyDataSGroupName(sg, name) {
    sg.data.fieldName = name;
  }
  function applyDataSGroupExpand(sg, expanded) {
    sg.data.expanded = expanded;
  }
  function applyDataSGroupQuery(sg, query) {
    sg.data.query = query;
  }
  function applyDataSGroupQueryOp(sg, queryOp) {
    sg.data.queryOp = queryOp;
  }
  function applyDataSGroupDesc(sGroups, propData) {
    const split = utils_default.partitionLine(propData, [4, 31, 2, 20, 2, 3], false);
    const id2 = utils_default.parseDecimalInt(split[0]) - 1;
    const fieldName = split[1].trim();
    const fieldType = split[2].trim();
    const units = split[3].trim();
    const query = split[4].trim();
    const queryOp = split[5].trim();
    const sGroup = sGroups[id2];
    sGroup.data.fieldType = fieldType;
    sGroup.data.fieldName = fieldName;
    sGroup.data.units = units;
    sGroup.data.query = query;
    sGroup.data.queryOp = queryOp;
  }
  function applyDataSGroupInfo(sg, propData) {
    const split = utils_default.partitionLine(
      propData,
      [
        10,
        10,
        4,
        1,
        1,
        1,
        3,
        3,
        3,
        3,
        2,
        3,
        2
      ],
      false
    );
    const x = parseFloat(split[0]);
    const y = parseFloat(split[1]);
    const attached = split[3].trim() === "A";
    const absolute = split[4].trim() === "A";
    const showUnits = split[5].trim() === "U";
    const nCharsRaw = split[7].trim();
    const nCharsToDisplay = nCharsRaw === "ALL" ? -1 : utils_default.parseDecimalInt(nCharsRaw);
    const tagChar = split[10].trim();
    const daspPos = utils_default.parseDecimalInt(split[11].trim());
    sg.pp = new Vec2(x, -y);
    sg.data.attached = attached;
    sg.data.absolute = absolute;
    sg.data.showUnits = showUnits;
    sg.data.nCharsToDisplay = nCharsToDisplay;
    sg.data.tagChar = tagChar;
    sg.data.daspPos = daspPos;
  }
  function applyDataSGroupInfoLine(sGroups, propData) {
    const id2 = utils_default.parseDecimalInt(propData.substr(0, 4)) - 1;
    const sg = sGroups[id2];
    applyDataSGroupInfo(sg, propData.substr(5));
  }
  function applyDataSGroupData(sg, data, finalize) {
    sg.data.fieldValue = (sg.data.fieldValue || "") + data;
    if (finalize) {
      sg.data.fieldValue = trimRight(sg.data.fieldValue);
      if (sg.data.fieldValue.startsWith('"') && sg.data.fieldValue.endsWith('"')) {
        sg.data.fieldValue = sg.data.fieldValue.substr(
          1,
          sg.data.fieldValue.length - 2
        );
      }
    }
  }
  function applyDataSGroupDataLine(sGroups, propData, finalize) {
    const id2 = utils_default.parseDecimalInt(propData.substr(0, 5)) - 1;
    const data = propData.substr(5);
    const sg = sGroups[id2];
    applyDataSGroupData(sg, data, finalize);
  }
  function toIntArray(strArray) {
    const ret = [];
    for (let j = 0; j < strArray.length; ++j) {
      ret[j] = utils_default.parseDecimalInt(strArray[j]);
    }
    return ret;
  }
  function trimRight(str) {
    return str ? str.replace(/\s+$/, '') : str;
  }
  function identityMap(array) {
    const map = {};
    for (const item of array) map[item] = item;
    return map;
  }
  function parseSGroupSAPLineV2000(ctabString) {
    const [, sss, nn6] = utils_default.partitionLine(
      ctabString.slice(0, 7),
      [1, 3, 3],
      false
    );
    const chunksNumberInLine = utils_default.parseDecimalInt(nn6);
    assert_default(chunksNumberInLine <= 6);
    const sGroupId = utils_default.parseDecimalInt(sss) - 1;
    const attachmentPointsStr = ctabString.slice(7);
    const attachmentPoints = [];
    for (let i = 0; i < chunksNumberInLine; i++) {
      const CHUNK_SIZE = 11;
      const stringForParse = attachmentPointsStr.slice(i * CHUNK_SIZE);
      const CHUNK_PARTS_LENGTHS = [1, 3, 1, 3, 1, 2];
      const [, iii, , ooo, , cc] = utils_default.partitionLine(
        stringForParse,
        CHUNK_PARTS_LENGTHS,
        false
      );
      const atomId = utils_default.parseDecimalInt(iii) - 1;
      assert_default(atomId >= 0);
      const leaveAtomParsedId = utils_default.parseDecimalInt(ooo);
      const leaveAtomId = leaveAtomParsedId > 0 ? leaveAtomParsedId - 1 : void 0;
      attachmentPoints.push(new SGroupAttachmentPoint(atomId, leaveAtomId, cc));
    }
    return { sGroupId, attachmentPoints };
  }
  var parseSGroup_default = {
    readKeyValuePairs,
    readKeyMultiValuePairs,
    loadSGroup,
    initSGroup,
    applySGroupProp,
    applySGroupArrayProp,
    applyDataSGroupName,
    applyDataSGroupQuery,
    applyDataSGroupQueryOp,
    applyDataSGroupDesc,
    applyDataSGroupInfo,
    applyDataSGroupData,
    applyDataSGroupInfoLine,
    applyDataSGroupDataLine,
    applyDataSGroupExpand,
    parseSGroupSAPLineV2000
  };

  // src/core/io/mol/v2000.ts
  var loadRGroupFragments = true;
  function parseAtomLine(atomLine) {
    const atomSplit = utils_default.partitionLine(
      atomLine,
      utils_default.fmtInfo.atomLinePartition
    );
    const params = {
      // generic
      pp: new Vec2(
        parseFloat(atomSplit[0]),
        -parseFloat(atomSplit[1]),
        parseFloat(atomSplit[2])
      ),
      label: atomSplit[4].trim(),
      explicitValence: utils_default.fmtInfo.valenceMap[utils_default.parseDecimalInt(atomSplit[10])],
      // obsolete
      massDifference: utils_default.parseDecimalInt(atomSplit[5]),
      charge: utils_default.fmtInfo.chargeMap[utils_default.parseDecimalInt(atomSplit[6])],
      // query
      hCount: utils_default.parseDecimalInt(atomSplit[8]),
      stereoCare: utils_default.parseDecimalInt(atomSplit[9]) !== 0,
      // reaction
      aam: utils_default.parseDecimalInt(atomSplit[14]),
      invRet: utils_default.parseDecimalInt(atomSplit[15]),
      // reaction query
      exactChangeFlag: utils_default.parseDecimalInt(atomSplit[16])
    };
    return new Atom2(params);
  }
  function parseBondLine(bondLine) {
    const bondSplit = utils_default.partitionLine(
      bondLine,
      utils_default.fmtInfo.bondLinePartition
    );
    const params = {
      begin: utils_default.parseDecimalInt(bondSplit[0]) - 1,
      end: utils_default.parseDecimalInt(bondSplit[1]) - 1,
      type: utils_default.fmtInfo.bondTypeMap[utils_default.parseDecimalInt(bondSplit[2])],
      stereo: utils_default.fmtInfo.bondStereoMap[utils_default.parseDecimalInt(bondSplit[3])],
      xxx: bondSplit[4],
      topology: utils_default.fmtInfo.bondTopologyMap[utils_default.parseDecimalInt(bondSplit[5])],
      reactingCenterStatus: utils_default.parseDecimalInt(bondSplit[6])
    };
    return new Bond3(params);
  }
  function parseAtomListLine(atomListLine) {
    const split = utils_default.partitionLine(
      atomListLine,
      utils_default.fmtInfo.atomListHeaderPartition
    );
    const number = utils_default.parseDecimalInt(split[0]) - 1;
    const notList = split[2].trim() === "T";
    const count = utils_default.parseDecimalInt(split[4].trim());
    const ids = atomListLine.slice(utils_default.fmtInfo.atomListHeaderLength);
    const list = [];
    const itemLength = utils_default.fmtInfo.atomListHeaderItemLength;
    for (let i = 0; i < count; ++i) {
      list[i] = utils_default.parseDecimalInt(
        ids.slice(i * itemLength, (i + 1) * itemLength - 1)
      );
    }
    return {
      aid: number,
      atomList: new AtomList({
        notList,
        ids: list
      })
    };
  }
  function handleAliasProperty(line, ctabLines, shift, props) {
    const propValue = ctabLines[shift];
    const isPseudo = /'.+'/.test(propValue);
    const propType = isPseudo ? "pseudo" : "alias";
    if (!props.get(propType)) {
      props.set(propType, new Pool());
    }
    const aliasPool = props.get(propType);
    if (aliasPool) {
      aliasPool.set(utils_default.parseDecimalInt(line.slice(3)) - 1, propValue);
    }
  }
  function handleSimpleAtomProperty(propName, propertyData, props) {
    if (!props.get(propName)) {
      props.set(propName, parseSGroup_default.readKeyValuePairs(propertyData, false));
    }
  }
  function handleSubstitutionProperty(propertyData, props) {
    if (!props.get("substitutionCount")) {
      props.set("substitutionCount", new Pool());
    }
    const subLabels = props.get("substitutionCount");
    if (!subLabels) {
      return;
    }
    const arrs = parseSGroup_default.readKeyMultiValuePairs(propertyData, false);
    for (const a2r of arrs) {
      subLabels.set(a2r[0], a2r[1]);
    }
  }
  function handleRGroupProperty(propertyData, props) {
    if (!props.get("rglabel")) {
      props.set("rglabel", new Pool());
    }
    const rglabels = props.get("rglabel");
    if (!rglabels) {
      return;
    }
    const a2rs = parseSGroup_default.readKeyMultiValuePairs(propertyData, false);
    for (const a2r of a2rs) {
      const rg = Number(a2r[1]);
      rglabels.set(a2r[0], (rglabels.get(a2r[0]) || 0) | 1 << rg - 1);
    }
  }
  function handleRGroupLogic(propertyData, rLogic) {
    const data = propertyData.slice(4);
    const rgid = utils_default.parseDecimalInt(data.slice(0, 3).trim());
    const iii = utils_default.parseDecimalInt(data.slice(4, 7).trim());
    const hhh = utils_default.parseDecimalInt(data.slice(8, 11).trim());
    const ooo = data.slice(12).trim();
    const logic = {
      resth: hhh === 1,
      range: ooo
    };
    if (iii > 0) {
      logic.ifthen = iii;
    }
    rLogic[rgid] = logic;
  }
  function handleAtomListProperty(propertyData, props) {
    const pool = parsePropertyLineAtomList(
      utils_default.partitionLine(propertyData, [1, 3, 3, 1, 1, 1]),
      utils_default.partitionLineFixed(propertyData.slice(10), 4, false)
    );
    if (!props.get("atomList")) {
      props.set("atomList", new Pool());
    }
    if (!props.get("label")) {
      props.set("label", new Pool());
    }
    const labelPool = props.get("label");
    const atomListPool = props.get("atomList");
    if (!labelPool || !atomListPool) {
      return;
    }
    pool.forEach((atomList, aid) => {
      labelPool.set(aid, "L#");
      atomListPool.set(aid, atomList);
    });
  }
  function handleSGroupDataProperty(type, propertyData, sGroups) {
    const sid = utils_default.parseDecimalInt(propertyData.slice(0, 4)) - 1;
    const value = propertyData.slice(4).trim();
    if (type === "SMT") {
      sGroups[sid].data.subscript = value;
    } else {
      sGroups[sid].data.class = value;
    }
  }
  function handleSGroupExpandedProperty(propertyData, sGroups) {
    const expandedSGroups = propertyData.slice(7).trim().split("   ");
    expandedSGroups.forEach((eg) => {
      const sGroupId = Number(eg) - 1;
      sGroups[sGroupId].data.expanded = true;
    });
  }
  function handleSGroupAttachmentProperty(propertyData, sGroups) {
    const { sGroupId, attachmentPoints } = parseSGroup_default.parseSGroupSAPLineV2000(propertyData);
    attachmentPoints.forEach((attachmentPoint) => {
      sGroups[sGroupId].addAttachmentPoint(attachmentPoint);
    });
  }
  function processMPropertyLine(type, propertyData, props, sGroups, rLogic) {
    if (type === "END") {
      return true;
    }
    if (type === "CHG") {
      handleSimpleAtomProperty("charge", propertyData, props);
    } else if (type === "RAD") {
      handleSimpleAtomProperty("radical", propertyData, props);
    } else if (type === "ISO") {
      handleSimpleAtomProperty("isotope", propertyData, props);
    } else if (type === "RBC") {
      handleSimpleAtomProperty("ringBondCount", propertyData, props);
    } else if (type === "SUB") {
      handleSubstitutionProperty(propertyData, props);
    } else if (type === "UNS") {
      handleSimpleAtomProperty("unsaturatedAtom", propertyData, props);
    } else if (type === "RGP") {
      handleRGroupProperty(propertyData, props);
    } else if (type === "LOG") {
      handleRGroupLogic(propertyData, rLogic);
    } else if (type === "APO") {
      handleSimpleAtomProperty("attachmentPoints", propertyData, props);
    } else if (type === "ALS") {
      handleAtomListProperty(propertyData, props);
    } else if (type === "STY") {
      parseSGroup_default.initSGroup(sGroups, propertyData);
    } else if (type === "SST") {
      parseSGroup_default.applySGroupProp(sGroups, "subtype", propertyData);
    } else if (type === "SLB") {
      parseSGroup_default.applySGroupProp(sGroups, "label", propertyData, true);
    } else if (type === "SPL") {
      parseSGroup_default.applySGroupProp(sGroups, "parent", propertyData, true, true);
    } else if (type === "SCN") {
      parseSGroup_default.applySGroupProp(sGroups, "connectivity", propertyData);
    } else if (type === "SAL") {
      parseSGroup_default.applySGroupArrayProp(sGroups, "atoms", propertyData, -1);
    } else if (type === "SBL") {
      parseSGroup_default.applySGroupArrayProp(sGroups, "bonds", propertyData, -1);
    } else if (type === "SPA") {
      parseSGroup_default.applySGroupArrayProp(sGroups, "patoms", propertyData, -1);
    } else if (type === "SMT" || type === "SCL") {
      handleSGroupDataProperty(type, propertyData, sGroups);
    } else if (type === "SDT") {
      parseSGroup_default.applyDataSGroupDesc(sGroups, propertyData);
    } else if (type === "SDD") {
      parseSGroup_default.applyDataSGroupInfoLine(sGroups, propertyData);
    } else if (type === "SCD") {
      parseSGroup_default.applyDataSGroupDataLine(sGroups, propertyData, false);
    } else if (type === "SED") {
      parseSGroup_default.applyDataSGroupDataLine(sGroups, propertyData, true);
    } else if (type === "SDS") {
      handleSGroupExpandedProperty(propertyData, sGroups);
    } else if (type === "SAP") {
      handleSGroupAttachmentProperty(propertyData, sGroups);
    }
    return false;
  }
  function parsePropertyLines(_ctab, ctabLines, shift, end, sGroups, rLogic) {
    const props = /* @__PURE__ */ new Map();
    while (shift < end) {
      const line = ctabLines[shift];
      if (line.startsWith("A")) {
        handleAliasProperty(line, ctabLines, ++shift, props);
      } else if (line.startsWith("M")) {
        const type = line.slice(3, 6);
        const propertyData = line.slice(6);
        const shouldBreak = processMPropertyLine(
          type,
          propertyData,
          props,
          sGroups,
          rLogic
        );
        if (shouldBreak) {
          break;
        }
      }
      ++shift;
    }
    return props;
  }
  function applyAtomProp(atoms, values2, propId) {
    values2.forEach((propVal, aid) => {
      const atom = atoms.get(aid);
      if (atom) {
        atom[propId] = propVal;
      }
    });
  }
  function createRGroupAttachmentPointsFromAtoms(struct) {
    struct.atoms.forEach((atom, atomId) => {
      if (!atom.attachmentPoints) {
        return;
      }
      const attachmentPoints = atom.attachmentPoints;
      if (attachmentPoints === 1 /* FirstSideOnly */) {
        struct.rgroupAttachmentPoints.add(
          new RGroupAttachmentPoint(atomId, "primary")
        );
      } else if (attachmentPoints === 2 /* SecondSideOnly */) {
        struct.rgroupAttachmentPoints.add(
          new RGroupAttachmentPoint(atomId, "secondary")
        );
      } else if (attachmentPoints === 3 /* BothSides */) {
        struct.rgroupAttachmentPoints.add(
          new RGroupAttachmentPoint(atomId, "primary")
        );
        struct.rgroupAttachmentPoints.add(
          new RGroupAttachmentPoint(atomId, "secondary")
        );
      }
    });
  }
  function parseCTabV2000(ctabLines, countsSplit, ignoreChiralFlag) {
    const ctab = new Struct();
    let i;
    const atomCount = utils_default.parseDecimalInt(countsSplit[0]);
    const bondCount = utils_default.parseDecimalInt(countsSplit[1]);
    const atomListCount = utils_default.parseDecimalInt(countsSplit[2]);
    const isAbs = utils_default.parseDecimalInt(countsSplit[4]) === 1 || ignoreChiralFlag;
    const isAnd = utils_default.parseDecimalInt(countsSplit[4]) === 0 && !ignoreChiralFlag;
    const stextLinesCount = utils_default.parseDecimalInt(countsSplit[5]);
    const propertyLinesCount = utils_default.parseDecimalInt(countsSplit[10]);
    let shift = 0;
    const atomLines = ctabLines.slice(shift, shift + atomCount);
    shift += atomCount;
    const bondLines = ctabLines.slice(shift, shift + bondCount);
    shift += bondCount;
    const atomListLines = ctabLines.slice(shift, shift + atomListCount);
    shift += atomListCount + stextLinesCount;
    const atoms = atomLines.map(parseAtomLine);
    atoms.forEach((atom) => ctab.atoms.add(atom));
    const bonds = bondLines.map(parseBondLine);
    bonds.forEach((bond) => {
      const beginAtom = ctab.atoms.get(bond.begin);
      if (beginAtom) {
        if (bond.stereo && isAbs) {
          beginAtom.stereoLabel = "abs" /* Abs */;
        }
        if (bond.stereo && isAnd) {
          beginAtom.stereoLabel = `${"&" /* And */}1`;
        }
      }
      ctab.bonds.add(bond);
    });
    const atomLists = atomListLines.map(parseAtomListLine);
    atomLists.forEach((pair) => {
      const atom = ctab.atoms.get(pair.aid);
      if (!atom) {
        throw String("Atom index out of range for atom list");
      }
      atom.atomList = pair.atomList;
      atom.label = "L#";
    });
    const sGroups = {};
    const rLogic = {};
    const props = parsePropertyLines(
      ctab,
      ctabLines,
      shift,
      Math.min(ctabLines.length, shift + propertyLinesCount),
      sGroups,
      rLogic
    );
    props.forEach((values2, propId) => {
      applyAtomProp(ctab.atoms, values2, propId);
    });
    const atomMap = {};
    let sid;
    for (sid in sGroups) {
      const sg = sGroups[sid];
      if (sg.type === "DAT" && sg.atoms.length === 0) {
        const parent = sGroups[sid].parent;
        if (parent >= 0) {
          const psg = sGroups[parent - 1];
          if (psg.type === "GEN") sg.atoms = [].slice.call(psg.atoms);
        }
      }
    }
    for (sid in sGroups) parseSGroup_default.loadSGroup(ctab, sGroups[sid], atomMap);
    const emptyGroups = [];
    for (sid in sGroups) {
      SGroup.filter(ctab, sGroups[sid], atomMap);
      if (sGroups[sid].atoms.length === 0 && !sGroups[sid].allAtoms) {
        emptyGroups.push(+sid);
      }
    }
    for (i = 0; i < emptyGroups.length; ++i) {
      ctab.sGroupForest.remove(emptyGroups[i]);
      ctab.sgroups.delete(emptyGroups[i]);
    }
    for (const id2 in rLogic) {
      const rgid = parseInt(id2, 10);
      ctab.rgroups.set(rgid, new RGroup(rLogic[rgid]));
    }
    createRGroupAttachmentPointsFromAtoms(ctab);
    return ctab;
  }
  function parseRg2000(ctabLines, ignoreChiralFlag) {
    ctabLines = ctabLines.slice(7);
    if (ctabLines[0].trim() !== "$CTAB") throw String("RGFile format invalid");
    let i = 1;
    while (!ctabLines[i].startsWith("$")) i++;
    if (ctabLines[i].trim() !== "$END CTAB") {
      throw String("RGFile format invalid");
    }
    const coreLines = ctabLines.slice(1, i);
    ctabLines = ctabLines.slice(i + 1);
    const fragmentLines = {};
    while (true) {
      if (ctabLines.length === 0) throw String("Unexpected end of file");
      let line = ctabLines[0].trim();
      if (line === "$END MOL") {
        ctabLines = ctabLines.slice(1);
        break;
      }
      if (line !== "$RGP") throw String("RGFile format invalid");
      const rgid = parseInt(ctabLines[1].trim(), 10);
      fragmentLines[rgid] = [];
      ctabLines = ctabLines.slice(2);
      while (true) {
        if (ctabLines.length === 0) throw String("Unexpected end of file");
        line = ctabLines[0].trim();
        if (line === "$END RGP") {
          ctabLines = ctabLines.slice(1);
          break;
        }
        if (line !== "$CTAB") throw String("RGFile format invalid");
        i = 1;
        while (!ctabLines[i].startsWith("$")) i++;
        if (ctabLines[i].trim() !== "$END CTAB") {
          throw String("RGFile format invalid");
        }
        fragmentLines[rgid].push(ctabLines.slice(1, i));
        ctabLines = ctabLines.slice(i + 1);
      }
    }
    const core = parseCTab(coreLines, ignoreChiralFlag);
    const frag = {};
    if (loadRGroupFragments) {
      for (const strId in fragmentLines) {
        const id2 = parseInt(strId, 10);
        frag[id2] = [];
        for (const fragmentLine of fragmentLines[id2]) {
          frag[id2].push(parseCTab(fragmentLine, ignoreChiralFlag));
        }
      }
    }
    return utils_default.rgMerge(core, frag);
  }
  function parseRxn2000(ctabLines, shouldReactionRelayout, ignoreChiralFlag) {
    ctabLines = ctabLines.slice(4);
    const countsSplit = utils_default.partitionLine(
      ctabLines[0],
      utils_default.fmtInfo.rxnItemsPartition
    );
    const nReactants = countsSplit[0] - 0;
    const nProducts = countsSplit[1] - 0;
    const nAgents = countsSplit[2] - 0;
    ctabLines = ctabLines.slice(1);
    const mols = [];
    while (ctabLines.length > 0 && ctabLines[0].startsWith("$MOL")) {
      ctabLines = ctabLines.slice(1);
      let n = 0;
      while (n < ctabLines.length && !ctabLines[n].startsWith("$MOL")) n++;
      const lines = ctabLines.slice(0, n);
      let struct;
      if (lines[0].startsWith("$MDL")) {
        struct = parseRg2000(lines, ignoreChiralFlag);
      } else {
        struct = parseCTab(lines.slice(3), ignoreChiralFlag);
        struct.name = lines[0].trim();
      }
      mols.push(struct);
      ctabLines = ctabLines.slice(n);
    }
    return utils_default.rxnMerge(
      mols,
      nReactants,
      nProducts,
      nAgents,
      shouldReactionRelayout
    );
  }
  function parseCTab(ctabLines, ignoreChiralFlag) {
    const countsSplit = utils_default.partitionLine(
      ctabLines[0],
      utils_default.fmtInfo.countsLinePartition
    );
    ctabLines = ctabLines.slice(1);
    return parseCTabV2000(ctabLines, countsSplit, ignoreChiralFlag);
  }
  function labelsListToIds(labels) {
    const ids = [];
    for (const label of labels) {
      const element = Elements.get(label.trim());
      if (element) {
        ids.push(element.number);
      }
    }
    return ids;
  }
  function parsePropertyLineAtomList(hdr, lst) {
    const aid = utils_default.parseDecimalInt(hdr[1]) - 1;
    const count = utils_default.parseDecimalInt(hdr[2]);
    const notList = hdr[4].trim() === "T";
    const ids = labelsListToIds(lst.slice(0, count));
    const ret = new Pool();
    ret.set(
      aid,
      new AtomList({
        notList,
        ids
      })
    );
    return ret;
  }
  var v2000_default = {
    parseCTabV2000,
    parseRg2000,
    parseRxn2000
  };

  // src/core/io/mol/v3000.ts
  function parseAtomLineV3000(line) {
    let subsplit, key, value, i;
    const split = spacebarsplit(line);
    const params = {
      pp: new Vec2(
        parseFloat(split[2]),
        -parseFloat(split[3]),
        parseFloat(split[4])
      ),
      aam: split[5].trim()
    };
    let label = split[1].trim();
    if (label.startsWith('"') && label.endsWith('"')) {
      label = label.slice(1, -1);
    }
    if (label.endsWith("]")) {
      label = label.slice(0, -1);
      const atomListParams = {};
      atomListParams.notList = false;
      const matchNotListInfo = label.match(/NOT ?\[/);
      if (matchNotListInfo) {
        atomListParams.notList = true;
        const [matchedSubstr] = matchNotListInfo;
        label = label.slice(matchedSubstr.length);
      } else if (!label.startsWith("[")) {
        throw String("Error: atom list expected, found '" + label + "'");
      } else {
        label = label.slice(1);
      }
      atomListParams.ids = labelsListToIds2(label.split(","));
      params.atomList = new AtomList(
        atomListParams
      );
      params.label = "L#";
    } else {
      params.label = label;
    }
    split.splice(0, 6);
    for (i = 0; i < split.length; ++i) {
      subsplit = splitonce(split[i], "=");
      key = subsplit[0];
      value = subsplit[1];
      if (key in utils_default.fmtInfo.v30atomPropMap) {
        let ival = utils_default.parseDecimalInt(value);
        if (key === "VAL") {
          if (ival === 0) continue;
          if (ival === -1) ival = 0;
        }
        params[utils_default.fmtInfo.v30atomPropMap[key]] = ival;
      } else if (key === "RGROUPS") {
        value = value.trim().slice(1, -1);
        const rgrsplit = value.split(" ").slice(1);
        params.rglabel = 0;
        for (const rgrValue of rgrsplit) {
          params.rglabel = params.rglabel | 1 << Number(rgrValue) - 1;
        }
      } else if (key === "ATTCHPT") {
        params.attpnt = value.trim() - 0;
      }
    }
    return new Atom2(params);
  }
  function parseBondLineV3000(line) {
    let subsplit, key, value, i;
    const split = spacebarsplit(line);
    const params = {
      begin: utils_default.parseDecimalInt(split[2]) - 1,
      end: utils_default.parseDecimalInt(split[3]) - 1,
      type: utils_default.fmtInfo.bondTypeMap[utils_default.parseDecimalInt(split[1])]
    };
    split.splice(0, 4);
    for (i = 0; i < split.length; ++i) {
      subsplit = splitonce(split[i], "=");
      key = subsplit[0];
      value = subsplit[1];
      if (key === "CFG") {
        params.stereo = utils_default.fmtInfo.v30bondStereoMap[utils_default.parseDecimalInt(value)];
        if (params.type === Bond3.PATTERN.TYPE.DOUBLE && params.stereo === Bond3.PATTERN.STEREO.EITHER) {
          params.stereo = Bond3.PATTERN.STEREO.CIS_TRANS;
        }
      } else if (key === "TOPO") {
        params.topology = utils_default.fmtInfo.bondTopologyMap[utils_default.parseDecimalInt(value)];
      } else if (key === "RXCTR") {
        params.reactingCenterStatus = utils_default.parseDecimalInt(value);
      } else if (key === "STBOX") {
        params.stereoCare = utils_default.parseDecimalInt(value);
      }
    }
    return new Bond3(params);
  }
  function v3000parseCollection(_ctab, ctabLines, shift) {
    shift++;
    while (ctabLines[shift].trim() !== "M  V30 END COLLECTION") shift++;
    shift++;
    return shift;
  }
  function v3000parseSGroup(ctab, ctabLines, sgroups, atomMap, shift) {
    let line = "";
    shift++;
    while (shift < ctabLines.length) {
      line = stripV30(ctabLines[shift++]).trim();
      if (line.trim() === "END SGROUP") return shift;
      while (line.endsWith("-")) {
        line = (line.slice(0, -1) + stripV30(ctabLines[shift++])).trim();
      }
      const split = splitSGroupDef(line);
      const type = split[1];
      const sg = new SGroup(type);
      sg.number = Number(split[0]);
      sg.type = type;
      sg.label = Number(split[2]);
      sgroups[sg.number] = sg;
      const props = {};
      for (const splitItem of split.slice(3)) {
        const subsplit = splitonce(splitItem, "=");
        if (subsplit.length !== 2) {
          throw String(
            "A record of form AAA=BBB or AAA=(...) expected, got '" + splitItem + "'"
          );
        }
        const name = subsplit[0];
        if (!(name in props)) props[name] = [];
        props[name].push(subsplit[1]);
      }
      sg.atoms = parseBracedNumberList(props.ATOMS[0], -1);
      if (props.PATOMS) {
        sg.patoms = parseBracedNumberList(props.PATOMS[0], -1);
      }
      sg.bonds = props.BONDS ? parseBracedNumberList(props.BONDS[0], -1) : [];
      const brkxyzStrs = props.BRKXYZ;
      sg.brkxyz = [];
      if (brkxyzStrs) {
        for (const brkxyzStr of brkxyzStrs) {
          sg.brkxyz.push(parseBracedNumberList(brkxyzStr));
        }
      }
      if (props.MULT) {
        sg.data.subscript = Number(props.MULT[0]);
      }
      if (props.LABEL) sg.data.subscript = props.LABEL[0].trim();
      if (props.CONNECT) {
        sg.data.connectivity = props.CONNECT[0].toLowerCase();
      }
      if (props.FIELDDISP) {
        parseSGroup_default.applyDataSGroupInfo(sg, stripQuotes(props.FIELDDISP[0]));
      }
      if (props.FIELDDATA) {
        parseSGroup_default.applyDataSGroupData(sg, props.FIELDDATA[0], true);
      }
      if (props.FIELDNAME) {
        parseSGroup_default.applyDataSGroupName(sg, props.FIELDNAME[0]);
      }
      if (props.QUERYTYPE) {
        parseSGroup_default.applyDataSGroupQuery(sg, props.QUERYTYPE[0]);
      }
      if (props.QUERYOP) parseSGroup_default.applyDataSGroupQueryOp(sg, props.QUERYOP[0]);
      parseSGroup_default.loadSGroup(ctab, sg, atomMap);
      if (props.ESTATE) {
        parseSGroup_default.applyDataSGroupExpand(sg, props.ESTATE[0] === "E");
      }
    }
    throw String("S-group declaration incomplete.");
  }
  function parseCTabV3000(ctabLines, norgroups) {
    const ctab = new Struct();
    let shift = 0;
    if (ctabLines[shift++].trim() !== "M  V30 BEGIN CTAB") {
      throw String("CTAB V3000 invalid");
    }
    if (!ctabLines[shift].startsWith("M  V30 COUNTS")) {
      throw String("CTAB V3000 invalid");
    }
    const vals = ctabLines[shift].slice(14).split(" ");
    const isAbs = utils_default.parseDecimalInt(vals[4]) === 1;
    shift++;
    if (ctabLines[shift].trim() === "M  V30 BEGIN ATOM") {
      shift++;
      let line;
      while (shift < ctabLines.length) {
        line = stripV30(ctabLines[shift++]).trim();
        if (line === "END ATOM") break;
        while (line.charAt(line.length - 1) === "-") {
          line = (line.substring(0, line.length - 1) + stripV30(ctabLines[shift++])).trim();
        }
        ctab.atoms.add(parseAtomLineV3000(line));
      }
      if (ctabLines[shift].trim() === "M  V30 BEGIN BOND") {
        shift++;
        while (shift < ctabLines.length) {
          line = stripV30(ctabLines[shift++]).trim();
          if (line === "END BOND") break;
          while (line.charAt(line.length - 1) === "-") {
            line = (line.substring(0, line.length - 1) + stripV30(ctabLines[shift++])).trim();
          }
          const bond = parseBondLineV3000(line);
          if (bond.stereo && isAbs) {
            const beginAtom = ctab.atoms.get(bond.begin);
            if (beginAtom) {
              beginAtom.stereoLabel = "abs";
            }
          }
          ctab.bonds.add(bond);
        }
      }
      const sgroups = {};
      const atomMap = {};
      while (ctabLines[shift].trim() !== "M  V30 END CTAB") {
        if (ctabLines[shift].trim() === "M  V30 BEGIN COLLECTION") {
          shift = v3000parseCollection(ctab, ctabLines, shift);
        } else if (ctabLines[shift].trim() === "M  V30 BEGIN SGROUP") {
          shift = v3000parseSGroup(ctab, ctabLines, sgroups, atomMap, shift);
        } else throw String("CTAB V3000 invalid");
      }
    }
    if (ctabLines[shift++].trim() !== "M  V30 END CTAB") {
      throw String("CTAB V3000 invalid");
    }
    if (!norgroups) readRGroups3000(ctab, ctabLines.slice(shift));
    return ctab;
  }
  function readRGroups3000(ctab, ctabLines) {
    const rfrags = {};
    const rLogic = {};
    let shift = 0;
    while (shift < ctabLines.length && ctabLines[shift].search("M  V30 BEGIN RGROUP") === 0) {
      const id2 = ctabLines[shift++].split(" ").pop();
      rfrags[id2] = [];
      rLogic[id2] = {};
      while (true) {
        let line = ctabLines[shift].trim();
        if (line.search("M  V30 RLOGIC") === 0) {
          line = line.slice(13);
          const rlsplit = line.trim().split(/\s+/g);
          const iii = utils_default.parseDecimalInt(rlsplit[0]);
          const hhh = utils_default.parseDecimalInt(rlsplit[1]);
          const ooo = rlsplit.slice(2).join(" ");
          const logic = {};
          if (iii > 0) {
            logic.ifthen = iii;
          }
          logic.resth = hhh === 1;
          logic.range = ooo;
          rLogic[id2] = logic;
          shift++;
          continue;
        }
        if (line !== "M  V30 BEGIN CTAB") throw String("CTAB V3000 invalid");
        let i;
        for (i = 0; i < ctabLines.length; ++i) {
          if (ctabLines[shift + i].trim() === "M  V30 END CTAB") break;
        }
        const lines = ctabLines.slice(shift, shift + i + 1);
        const rfrag = parseCTabV3000(lines, true);
        rfrags[id2].push(rfrag);
        shift = shift + i + 1;
        if (ctabLines[shift].trim() === "M  V30 END RGROUP") {
          shift++;
          break;
        }
      }
    }
    Object.keys(rfrags).forEach((rgid) => {
      rfrags[rgid].forEach((rg) => {
        var _a;
        const rgidNum = Number(rgid);
        rg.rgroups.set(rgidNum, new RGroup(rLogic[rgid]));
        const frid = rg.frags.add(new Fragment());
        (_a = rg.rgroups.get(rgidNum)) == null ? void 0 : _a.frags.add(frid);
        rg.atoms.forEach((atom) => {
          atom.fragment = frid;
        });
        rg.mergeInto(ctab);
      });
    });
  }
  function parseRxn3000(ctabLines, shouldReactionRelayout) {
    ctabLines = ctabLines.slice(4);
    const countsSplit = ctabLines[0].split(/\s+/g).slice(3);
    const nReactants = Number(countsSplit[0]);
    const nProducts = Number(countsSplit[1]);
    const nAgents = countsSplit.length > 2 ? Number(countsSplit[2]) : 0;
    function findCtabEnd(i2) {
      for (let j = i2; j < ctabLines.length; ++j) {
        if (ctabLines[j].trim() === "M  V30 END CTAB") {
          return j;
        }
      }
      console.error("CTab format invalid");
      return i2;
    }
    function findRGroupEnd(i2) {
      for (let j = i2; j < ctabLines.length; ++j) {
        if (ctabLines[j].trim() === "M  V30 END RGROUP") {
          return j;
        }
      }
      console.error("CTab format invalid");
      return i2;
    }
    const molLinesReactants = [];
    const molLinesProducts = [];
    const molLinesAgents = [];
    let current = null;
    const rGroups = [];
    let i = 0;
    while (i < ctabLines.length) {
      const line = ctabLines[i].trim();
      if (line.startsWith("M  V30 COUNTS")) {
      } else if (line === "M  END") {
        break;
      } else if (line === "M  V30 BEGIN PRODUCT") {
        current = molLinesProducts;
      } else if (line === "M  V30 END PRODUCT") {
        current = null;
      } else if (line === "M  V30 BEGIN REACTANT") {
        current = molLinesReactants;
      } else if (line === "M  V30 END REACTANT") {
        current = null;
      } else if (line === "M  V30 BEGIN AGENT") {
        current = molLinesAgents;
      } else if (line === "M  V30 END AGENT") {
        current = null;
      } else if (line.startsWith("M  V30 BEGIN RGROUP")) {
        const j = findRGroupEnd(i);
        rGroups.push(ctabLines.slice(i, j + 1));
        i = j + 1;
        continue;
      } else if (line === "M  V30 BEGIN CTAB") {
        const j = findCtabEnd(i);
        current == null ? void 0 : current.push(ctabLines.slice(i, j + 1));
        i = j + 1;
        continue;
      } else {
        throw String("line unrecognized: " + line);
      }
      i++;
    }
    const mols = [];
    const molLines = molLinesReactants.concat(molLinesProducts).concat(molLinesAgents);
    for (const molLine of molLines) {
      const mol = parseCTabV3000(molLine, countsSplit);
      mols.push(mol);
    }
    const ctab = utils_default.rxnMerge(
      mols,
      nReactants,
      nProducts,
      nAgents,
      shouldReactionRelayout
    );
    readRGroups3000(
      ctab,
      (function(array) {
        let res = [];
        for (const item of array) {
          res = res.concat(item);
        }
        return res;
      })(rGroups)
    );
    return ctab;
  }
  function spacebarsplit(line) {
    const split = [];
    let bracketEquality = 0;
    let currentIndex = 0;
    let firstSliceIndex = -1;
    let quoted = false;
    while (currentIndex < line.length) {
      const currentSymbol = line[currentIndex];
      if (line.slice(currentIndex, currentIndex + 3) === "NOT") {
        const closingBracketIndex = line.indexOf("]");
        split.push(line.slice(currentIndex, closingBracketIndex + 1));
        currentIndex = closingBracketIndex + 1;
        firstSliceIndex = currentIndex;
      } else if (currentSymbol === "(") bracketEquality += 1;
      else if (currentSymbol === ")") bracketEquality -= 1;
      else if (currentSymbol === '"') quoted = !quoted;
      else if (!quoted && line[currentIndex] === " " && bracketEquality === 0) {
        if (currentIndex > firstSliceIndex + 1) {
          split.push(line.slice(firstSliceIndex + 1, currentIndex));
        }
        firstSliceIndex = currentIndex;
      }
      currentIndex += 1;
    }
    if (currentIndex > firstSliceIndex + 1) {
      split.push(line.slice(firstSliceIndex + 1, currentIndex));
    }
    return split;
  }
  function stripQuotes(str) {
    if (str.startsWith('"') && str.endsWith('"')) {
      return str.slice(1, -1);
    }
    return str;
  }
  function splitonce(line, delim) {
    const p = line.indexOf(delim);
    return [line.slice(0, p), line.slice(p + 1)];
  }
  function splitSGroupDef(line) {
    const split = [];
    let braceBalance = 0;
    let quoted = false;
    let i = 0;
    while (i < line.length) {
      const c = line.charAt(i);
      if (c === '"') {
        quoted = !quoted;
      } else if (!quoted) {
        if (c === "(") {
          braceBalance++;
        } else if (c === ")") {
          braceBalance--;
        } else if (c === " " && braceBalance === 0) {
          split.push(line.slice(0, i));
          line = line.slice(i + 1).trim();
          i = 0;
          continue;
        }
      }
      i++;
    }
    if (braceBalance !== 0) {
      throw String("Brace balance broken. S-group properies invalid!");
    }
    if (line.length > 0) split.push(line.trim());
    return split;
  }
  function parseBracedNumberList(line, shift) {
    if (!line) return null;
    const list = [];
    line = line.trim();
    line = line.slice(1, -1);
    const split = line.split(" ");
    shift = shift || 0;
    for (const splitItem of split.slice(1)) {
      const value = parseInt(splitItem);
      if (!isNaN(value)) {
        list.push(value + shift);
      }
    }
    return list;
  }
  function stripV30(line) {
    if (!line.startsWith("M  V30 ")) throw String("Prefix invalid");
    return line.slice(7);
  }
  function labelsListToIds2(labels) {
    const ids = [];
    for (const label of labels) {
      const element = Elements.get(label.trim());
      if (element) {
        ids.push(element.number);
      }
    }
    return ids;
  }
  var v3000_default = {
    parseCTabV3000,
    readRGroups3000,
    parseRxn3000
  };

  // src/core/io/mol/common.ts
  function getAtom(mol, id2) {
    const atom = mol.atoms.get(id2);
    if (!atom) {
      throw String(`Atom ${id2} not found`);
    }
    return atom;
  }
  var loadRGroupFragments2 = true;
  function parseMol(ctabLines, ignoreChiralFlag) {
    if (ctabLines[0].search("\\$MDL") === 0) {
      const struct2 = v2000_default.parseRg2000(ctabLines, ignoreChiralFlag);
      struct2.name = ctabLines[3].trim();
      return struct2;
    }
    const struct = parseCTab2(ctabLines.slice(3), ignoreChiralFlag);
    struct.name = ctabLines[0].trim();
    return struct;
  }
  function parseCTab2(ctabLines, ignoreChiralFlag) {
    const countsSplit = partitionLine2(
      ctabLines[0],
      utils_default.fmtInfo.countsLinePartition
    );
    const version = countsSplit[11].trim();
    ctabLines = ctabLines.slice(1);
    if (version === "V2000") {
      return v2000_default.parseCTabV2000(ctabLines, countsSplit, ignoreChiralFlag);
    }
    if (version === "V3000") {
      return v3000_default.parseCTabV3000(ctabLines, !loadRGroupFragments2);
    } else {
      throw String("Molfile version unknown: " + version);
    }
  }
  function parseRxn(ctabLines, shouldReactionRelayout, ignoreChiralFlag) {
    const split = ctabLines[0].trim().split(" ");
    if (split.length > 1 && split[1] === "V3000") {
      return v3000_default.parseRxn3000(ctabLines, shouldReactionRelayout);
    }
    const struct = v2000_default.parseRxn2000(
      ctabLines,
      shouldReactionRelayout,
      ignoreChiralFlag
    );
    struct.name = ctabLines[1].trim();
    return struct;
  }
  var prepareForSaving = {
    MUL: SGroup.prepareMulForSaving,
    SRU: prepareSruForSaving,
    SUP: prepareSupForSaving,
    DAT: prepareDatForSaving,
    GEN: prepareGenForSaving,
    COP: prepareCopForSaving,
    queryComponent: prepareQueryComponentForSaving
  };
  function prepareSruForSaving(sgroup, mol) {
    const xBonds = [];
    mol.bonds.forEach((bond, bid) => {
      const a1 = getAtom(mol, bond.begin);
      const a2 = getAtom(mol, bond.end);
      if (a1.sgs.has(sgroup.id) && !a2.sgs.has(sgroup.id) || a2.sgs.has(sgroup.id) && !a1.sgs.has(sgroup.id)) {
        xBonds.push(bid);
      }
    });
    if (xBonds.length !== 0 && xBonds.length !== 2) {
      const error = new Error(
        "Unsupported cross-bonds number"
      );
      error.id = sgroup.id;
      error["error-type"] = "cross-bond-number";
      throw String(error);
    }
    sgroup.bonds = xBonds;
  }
  function prepareCopForSaving(sgroup, mol) {
    const xBonds = [];
    mol.bonds.forEach((bond, bid) => {
      const a1 = getAtom(mol, bond.begin);
      const a2 = getAtom(mol, bond.end);
      if (a1.sgs.has(sgroup.id) && !a2.sgs.has(sgroup.id) || a2.sgs.has(sgroup.id) && !a1.sgs.has(sgroup.id)) {
        xBonds.push(bid);
      }
    });
    if (xBonds.length !== 0 && xBonds.length !== 2) {
      const error = new Error(
        "Unsupported cross-bonds number"
      );
      error.id = sgroup.id;
      error["error-type"] = "cross-bond-number";
      throw String(error);
    }
    sgroup.bonds = xBonds;
  }
  function prepareSupForSaving(sgroup, mol) {
    const xBonds = [];
    mol.bonds.forEach((bond, bid) => {
      const a1 = getAtom(mol, bond.begin);
      const a2 = getAtom(mol, bond.end);
      if (a1.sgs.has(sgroup.id) && !a2.sgs.has(sgroup.id) || a2.sgs.has(sgroup.id) && !a1.sgs.has(sgroup.id)) {
        xBonds.push(bid);
      }
    });
    sgroup.bonds = xBonds;
  }
  function prepareGenForSaving(_sgroup2, _mol) {
  }
  function prepareQueryComponentForSaving(_sgroup2, _mol) {
  }
  function prepareDatForSaving(sgroup, mol) {
    sgroup.atoms = SGroup.getAtoms(mol, sgroup);
  }
  var saveToMolfile = {
    MUL: saveMulToMolfile,
    SRU: saveSruToMolfile,
    COP: saveCopToMolfile,
    SUP: saveSupToMolfile,
    DAT: saveDatToMolfile,
    GEN: saveGenToMolfile
  };
  function saveMulToMolfile(sgroup, mol, sgMap, atomMap, bondMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    let lines = [];
    lines = lines.concat(
      makeAtomBondLines(
        "SAL",
        idstr,
        Array.from(sgroup.atomSet.values()),
        atomMap
      )
    );
    lines = lines.concat(
      makeAtomBondLines(
        "SPA",
        idstr,
        Array.from(sgroup.parentAtomSet.values()),
        atomMap
      )
    );
    lines = lines.concat(makeAtomBondLines("SBL", idstr, sgroup.bonds, bondMap));
    const smtLine = "M  SMT " + idstr + " " + sgroup.data.mul;
    lines.push(smtLine);
    lines = lines.concat(bracketsToMolfile(mol, sgroup, idstr));
    return lines.join("\n");
  }
  function saveSruToMolfile(sgroup, mol, sgMap, atomMap, bondMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    let lines = [];
    lines = lines.concat(makeAtomBondLines("SAL", idstr, sgroup.atoms, atomMap));
    lines = lines.concat(makeAtomBondLines("SBL", idstr, sgroup.bonds, bondMap));
    lines = lines.concat(bracketsToMolfile(mol, sgroup, idstr));
    return lines.join("\n");
  }
  function saveCopToMolfile(sgroup, mol, sgMap, atomMap, bondMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    let lines = [];
    lines = lines.concat(makeAtomBondLines("SAL", idstr, sgroup.atoms, atomMap));
    lines = lines.concat(makeAtomBondLines("SBL", idstr, sgroup.bonds, bondMap));
    lines = lines.concat(bracketsToMolfile(mol, sgroup, idstr));
    return lines.join("\n");
  }
  function saveSupToMolfile(sgroup, _mol, sgMap, atomMap, bondMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    let lines = [];
    lines = lines.concat(makeAtomBondLines("SAL", idstr, sgroup.atoms, atomMap));
    lines = lines.concat(makeAtomBondLines("SBL", idstr, sgroup.bonds, bondMap));
    let sgroupName;
    if (sgroup instanceof MonomerMicromolecule) {
      sgroupName = sgroup.monomer.label;
    } else if (sgroup.data.name && sgroup.data.name !== "") {
      sgroupName = sgroup.data.name;
    }
    if (sgroupName) {
      lines.push("M  SMT " + idstr + " " + sgroupName);
    }
    if (sgroup.data.class) {
      lines.push("M  SCL " + idstr + " " + sgroup.data.class);
    }
    return lines.join("\n");
  }
  function saveDatToMolfile(sgroup, mol, sgMap, atomMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    const data = sgroup.data;
    if (!sgroup.pp) {
      throw String("SGroup pp is not set");
    }
    let pp = sgroup.pp;
    if (!data.absolute) pp = pp.sub(SGroup.getMassCentre(mol, sgroup.atoms));
    let lines = [];
    lines = lines.concat(makeAtomBondLines("SAL", idstr, sgroup.atoms, atomMap));
    let sdtLine = "M  SDT " + idstr + " " + (data.fieldName || "").padEnd(30) + (data.fieldType || "").padStart(2) + (data.units || "").padEnd(20) + (data.query || "").padStart(2);
    if (data.queryOp) {
      sdtLine += data.queryOp.padEnd(80 - 65);
    }
    lines.push(sdtLine);
    const sddLine = "M  SDD " + idstr + " " + utils_default.paddedNum(pp.x, 10, 4) + utils_default.paddedNum(-pp.y, 10, 4) + "    " + // ' eee'
    (data.attached ? "A" : "D") + // f
    (data.absolute ? "A" : "R") + // g
    (data.showUnits ? "U" : " ") + // h
    "   " + //  i
    (data.nCharnCharsToDisplay >= 0 ? utils_default.paddedNum(data.nCharnCharsToDisplay, 3) : "ALL") + // jjj
    "  1   " + // 'kkk ll '
    (data.tagChar || " ") + // m
    "  " + utils_default.paddedNum(data.daspPos, 1) + // n
    "  ";
    lines.push(sddLine);
    const val = normalizeNewlines(data.fieldValue).replace(/\n*$/, "");
    const charsPerLine = 69;
    val.split("\n").forEach((chars) => {
      while (chars.length > charsPerLine) {
        lines.push("M  SCD " + idstr + " " + chars.slice(0, charsPerLine));
        chars = chars.slice(charsPerLine);
      }
      lines.push("M  SED " + idstr + " " + chars);
    });
    return lines.join("\n");
  }
  function saveGenToMolfile(sgroup, mol, sgMap, atomMap, bondMap) {
    const idstr = (sgMap[sgroup.id] + "").padStart(3);
    let lines = [];
    lines = lines.concat(makeAtomBondLines("SAL", idstr, sgroup.atoms, atomMap));
    lines = lines.concat(makeAtomBondLines("SBL", idstr, sgroup.bonds, bondMap));
    lines = lines.concat(bracketsToMolfile(mol, sgroup, idstr));
    return lines.join("\n");
  }
  function makeAtomBondLines(prefix, idstr, ids, map) {
    if (!ids) return [];
    const lines = [];
    for (let i = 0; i < Math.floor((ids.length + 14) / 15); ++i) {
      const rem = Math.min(ids.length - 15 * i, 15);
      let salLine = "M  " + prefix + " " + idstr + " " + utils_default.paddedNum(rem, 2);
      for (let j = 0; j < rem; ++j) {
        salLine += " " + utils_default.paddedNum(map[ids[i * 15 + j]], 3);
      }
      lines.push(salLine);
    }
    return lines;
  }
  function bracketsToMolfile(mol, sg, idstr) {
    const atomSet = new Pile(sg.atoms);
    const crossBonds = SGroup.getCrossBonds(mol, atomSet);
    SGroup.bracketPos(sg, mol);
    const bb = sg.bracketBox;
    const d = sg.bracketDirection;
    const n = d.rotateSC(1, 0);
    const brackets = SGroup.getBracketParameters(
      mol,
      crossBonds,
      atomSet,
      bb,
      d,
      n
    );
    const lines = [];
    for (const bracket of brackets) {
      const a0 = bracket.c.addScaled(bracket.n, -0.5 * bracket.h).yComplement(0);
      const a1 = bracket.c.addScaled(bracket.n, 0.5 * bracket.h).yComplement(0);
      let line = "M  SDI " + idstr + utils_default.paddedNum(4, 3);
      const coord = [a0.x, a0.y, a1.x, a1.y];
      for (const coordValue of coord) {
        line += utils_default.paddedNum(coordValue, 10, 4);
      }
      lines.push(line);
    }
    return lines;
  }
  var nlRe = /\r\n|[\n\r]/g;
  function normalizeNewlines(str) {
    return str.replace(nlRe, "\n");
  }
  function partitionLine2(str, parts, withspace) {
    const res = [];
    for (let i = 0, shift = 0; i < parts.length; ++i) {
      res.push(str.slice(shift, shift + parts[i]));
      if (withspace) shift++;
      shift += parts[i];
    }
    return res;
  }
  var common_default = {
    parseCTab: parseCTab2,
    parseMol,
    parseRxn,
    prepareForSaving,
    saveToMolfile
  };

  // src/core/io/mol/molfile.ts
  var import_utilities11 = __toESM(require_utilities());
  var END_V2000 = "2D 1   1.00000     0.00000     0";
  var Molfile = class _Molfile {
    constructor() {
      __publicField(this, "molecule");
      __publicField(this, "molfile");
      __publicField(this, "reaction");
      __publicField(this, "mapping");
      __publicField(this, "bondMapping");
      this.molecule = null;
      this.molfile = null;
      this.reaction = false;
      this.mapping = {};
      this.bondMapping = {};
    }
    parseCTFile(props) {
      const { molfileLines, shouldReactionRelayout, ignoreChiralFlag } = props;
      let ret;
      if (molfileLines[0].search("\\$RXN") === 0) {
        ret = common_default.parseRxn(
          molfileLines,
          shouldReactionRelayout,
          ignoreChiralFlag
        );
      } else {
        ret = common_default.parseMol(molfileLines, ignoreChiralFlag);
      }
      ret.initHalfBonds();
      ret.initNeighbors();
      ret.bindSGroupsToFunctionalGroups();
      ret.markFragments();
      return ret;
    }
    prepareSGroups(skipErrors, preserveIndigoDesc) {
      const mol = this.molecule;
      if (!mol) return;
      const toRemove = [];
      let errors = 0;
      mol.sGroupForest.getSGroupsBFS().reverse().forEach((id2) => {
        const sgroup = mol.sgroups.get(id2);
        let errorIgnore = false;
        try {
          common_default.prepareForSaving[sgroup.type](sgroup, mol);
        } catch (e) {
          import_utilities11.sketchLogger.error("molfile.ts::Molfile::prepareSGroups", e);
          if (!skipErrors || typeof e.id !== "number") {
            console.warn(`Error: ${e.message}`);
          }
          errorIgnore = true;
        }
        if (errorIgnore || !preserveIndigoDesc && /^INDIGO_.+_DESC$/i.test(sgroup.data.fieldName)) {
          errors += +errorIgnore;
          toRemove.push(sgroup.id);
        }
      }, this);
      if (errors) {
        console.warn(
          "Warning: " + errors + " invalid S-groups were detected. They will be omitted."
        );
      }
      for (const sgroupId of toRemove) {
        mol == null ? void 0 : mol.sGroupDelete(sgroupId);
      }
    }
    getCTab(molecule, rgroups) {
      this.molecule = molecule.clone();
      this.prepareSGroups(false, false);
      this.molfile = "";
      this.writeCTab2000(rgroups);
      return this.molfile;
    }
    saveMolecule(molecule, skipSGroupErrors, norgroups, preserveIndigoDesc) {
      this.reaction = molecule.hasRxnArrow();
      this.molfile = "" + molecule.name;
      if (this.reaction) {
        if (molecule.rgroups.size > 0) {
          throw String(
            "Reactions with r-groups are not supported at the moment"
          );
        }
        const components = molecule.getComponents();
        const reactants = components.reactants;
        const products = components.products;
        const all = reactants.concat(products);
        this.molfile = "$RXN\n" + molecule.name + "\n\n\n" + utils_default.paddedNum(reactants.length, 3) + utils_default.paddedNum(products.length, 3) + utils_default.paddedNum(0, 3) + "\n";
        for (const component of all) {
          const saver = new _Molfile();
          const submol = molecule.clone(component, null, true);
          const molfile = saver.saveMolecule(submol, false, true);
          this.molfile += "$MOL\n" + molfile;
        }
        return this.molfile;
      }
      if (molecule.rgroups.size > 0) {
        if (norgroups) {
          molecule = molecule.getScaffold();
        } else {
          const scaffold = new _Molfile().getCTab(
            molecule.getScaffold(),
            molecule.rgroups
          );
          this.molfile = "$MDL  REV  1\n$MOL\n$HDR\n" + molecule.name + "\n\n\n$END HDR\n";
          this.molfile += "$CTAB\n" + scaffold + "$END CTAB\n";
          molecule.rgroups.forEach((rg, rgid) => {
            this.molfile += "$RGP\n";
            this.writePaddedNumber(rgid, 3);
            this.molfile += "\n";
            rg.frags.forEach((fid) => {
              const group = new _Molfile().getCTab(molecule.getFragment(fid));
              this.molfile += "$CTAB\n" + group + "$END CTAB\n";
            });
            this.molfile += "$END RGP\n";
          });
          this.molfile += "$END MOL\n";
          return this.molfile;
        }
      }
      this.molecule = molecule.clone();
      this.prepareSGroups(skipSGroupErrors, preserveIndigoDesc);
      this.writeHeader();
      this.writeCTab2000();
      return this.molfile;
    }
    writeHeader() {
      const date = /* @__PURE__ */ new Date();
      this.writeCR();
      this.writeWhiteSpace(2);
      this.write("sketch");
      this.writeWhiteSpace();
      this.writeCR(
        (date.getMonth() + 1 + "").padStart(2) + (date.getDate() + "").padStart(2) + (date.getFullYear() % 100 + "").padStart(2) + (date.getHours() + "").padStart(2) + (date.getMinutes() + "").padStart(2) + END_V2000
      );
      this.writeCR();
    }
    write(str) {
      this.molfile += str;
    }
    writeCR(str) {
      if (arguments.length === 0) {
        str = "";
      }
      this.molfile += str + "\n";
    }
    writeWhiteSpace(length = 0) {
      if (arguments.length === 0) {
        length = 1;
      }
      this.write(" ".repeat(Math.max(length, 0)));
    }
    writePadded(str, width) {
      this.write(str);
      this.writeWhiteSpace(width - str.length);
    }
    writePaddedNumber(number, width) {
      const str = (number - 0).toString();
      this.writeWhiteSpace(width - str.length);
      this.write(str);
    }
    writePaddedFloat(number, width, precision) {
      this.write(utils_default.paddedNum(number, width, precision));
    }
    writeCTab2000Header() {
      this.writePaddedNumber(this.molecule.atoms.size, 3);
      this.writePaddedNumber(this.molecule.bonds.size, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      const isAbsFlag = Array.from(this.molecule.frags.values()).some(
        (fr) => fr ? fr.enhancedStereoFlag === "ABS" /* Abs */ : false
      );
      this.writePaddedNumber(isAbsFlag ? 1 : 0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(999, 3);
      this.writeCR(" V2000");
    }
    writeCTab2000(rgroups) {
      const molecule = this.molecule;
      if (!molecule) return;
      this.writeCTab2000Header();
      this.mapping = {};
      let i = 1;
      const atomsIds = [];
      const atomsProps = [];
      molecule.atoms.forEach((atom, id2) => {
        let label = atom.label;
        if (atom.atomList != null) {
          label = "L";
          atomsIds.push(id2);
        } else if (atom.pseudo) {
          if (atom.pseudo.length > 3) {
            label = "A";
            atomsProps.push({ id: id2, value: `'${atom.pseudo}'` });
          }
        } else if (atom.alias) {
          atomsProps.push({ id: id2, value: atom.alias });
        } else if (!Elements.get(atom.label) && ["A", "Q", "X", "*", "R#"].indexOf(atom.label) === -1) {
          label = "C";
          atomsProps.push({ id: id2, value: atom.label });
        }
        this.writeAtom(atom, label);
        this.mapping[id2] = i++;
      }, this);
      this.bondMapping = {};
      i = 1;
      molecule.bonds.forEach((bond, id2) => {
        this.bondMapping[id2] = i++;
        this.writeBond(bond);
      }, this);
      while (atomsProps.length > 0) {
        this.writeAtomProps(atomsProps[0]);
        atomsProps.splice(0, 1);
      }
      const chargeList = [];
      const isotopeList = [];
      const radicalList = [];
      const rglabelList = [];
      const rglogicList = [];
      const aplabelList = [];
      const rbcountList = [];
      const unsaturatedList = [];
      const substcountList = [];
      molecule.atoms.forEach((atom, id2) => {
        if (atom.charge !== 0 && atom.charge !== null) {
          chargeList.push([id2, atom.charge]);
        }
        if (atom.isotope !== 0 && atom.isotope !== null) {
          isotopeList.push([id2, atom.isotope]);
        }
        if (atom.radical !== 0) {
          radicalList.push([id2, atom.radical]);
        }
        if (atom.rglabel != null && atom.label === "R#") {
          for (let rgi = 0; rgi < 32; rgi++) {
            if (atom.rglabel & 1 << rgi) {
              rglabelList.push([id2, rgi + 1]);
            }
          }
        }
        if (atom.attachmentPoints != null) {
          aplabelList.push([id2, atom.attachmentPoints]);
        }
        if (atom.ringBondCount !== 0) {
          rbcountList.push([id2, atom.ringBondCount]);
        }
        if (atom.substitutionCount !== 0) {
          substcountList.push([id2, atom.substitutionCount]);
        }
        if (atom.unsaturatedAtom !== 0) {
          unsaturatedList.push([id2, atom.unsaturatedAtom]);
        }
      });
      if (rgroups) {
        rgroups.forEach((rg, rgid) => {
          if (rg.resth || rg.ifthen > 0 || rg.range.length > 0) {
            const line = "  1 " + utils_default.paddedNum(rgid, 3) + " " + utils_default.paddedNum(rg.ifthen, 3) + " " + utils_default.paddedNum(rg.resth ? 1 : 0, 3) + "   " + rg.range;
            rglogicList.push(line);
          }
        });
      }
      this.writeAtomPropList("M  CHG", chargeList);
      this.writeAtomPropList("M  ISO", isotopeList);
      this.writeAtomPropList("M  RAD", radicalList);
      this.writeAtomPropList("M  RGP", rglabelList);
      for (const logic of rglogicList) {
        this.write("M  LOG" + logic + "\n");
      }
      this.writeAtomPropList("M  APO", aplabelList);
      this.writeAtomPropList("M  RBC", rbcountList);
      this.writeAtomPropList("M  SUB", substcountList);
      this.writeAtomPropList("M  UNS", unsaturatedList);
      if (atomsIds.length > 0) {
        for (const atomId of atomsIds) {
          const atomList = molecule.atoms.get(atomId).atomList;
          this.write("M  ALS");
          this.writePaddedNumber(atomId + 1, 4);
          this.writePaddedNumber(atomList.ids.length, 3);
          this.writeWhiteSpace();
          this.write(atomList.notList ? "T" : "F");
          const labelList = atomList.labelList();
          for (const label of labelList) {
            this.writeWhiteSpace();
            this.writePadded(label, 3);
          }
          this.writeWhiteSpace();
          this.writeCR();
        }
      }
      const sgmap = {};
      let cnt = 1;
      const sgmapback = {};
      const sgorder = molecule.sGroupForest.getSGroupsBFS();
      sgorder.forEach((id2) => {
        sgmapback[cnt] = id2;
        sgmap[id2] = cnt++;
      });
      for (const sGroupIdInCTab of Array.from(
        { length: cnt - 1 },
        (_, index) => index + 1
      )) {
        const id2 = sgmapback[sGroupIdInCTab];
        const sgroup = molecule.sgroups.get(id2);
        if (SGroup.isQuerySGroup(sgroup)) {
          console.warn("Query group does not support in mol format");
          continue;
        }
        this.write("M  STY");
        this.writePaddedNumber(1, 3);
        this.writeWhiteSpace(1);
        this.writePaddedNumber(sGroupIdInCTab, 3);
        this.writeWhiteSpace(1);
        this.writePadded(sgroup.type, 3);
        this.writeCR();
        if (sgroup.type === "COP" && sgroup.data.subtype) {
          this.write("M  SST");
          this.writePaddedNumber(1, 3);
          this.writeWhiteSpace(1);
          this.writePaddedNumber(sGroupIdInCTab, 3);
          this.writeWhiteSpace(1);
          this.writePadded(sgroup.data.subtype.toUpperCase(), 3);
          this.writeCR();
        }
        this.write("M  SLB");
        this.writePaddedNumber(1, 3);
        this.writeWhiteSpace(1);
        this.writePaddedNumber(sGroupIdInCTab, 3);
        this.writeWhiteSpace(1);
        this.writePaddedNumber(sGroupIdInCTab, 3);
        this.writeCR();
        const parentId = molecule.sGroupForest.parent.get(id2);
        if (parentId >= 0) {
          this.write("M  SPL");
          this.writePaddedNumber(1, 3);
          this.writeWhiteSpace(1);
          this.writePaddedNumber(sGroupIdInCTab, 3);
          this.writeWhiteSpace(1);
          this.writePaddedNumber(sgmap[parentId], 3);
          this.writeCR();
        }
        if (["SRU", "COP"].includes(sgroup.type) && sgroup.data.connectivity) {
          const connectivity = ` ${sGroupIdInCTab.toString().padStart(3)} ${(sgroup.data.connectivity || "").padEnd(3)}`;
          this.write("M  SCN");
          this.writePaddedNumber(1, 3);
          this.write(connectivity.toUpperCase());
          this.writeCR();
        }
        if (sgroup.type === "SRU") {
          this.write("M  SMT ");
          this.writePaddedNumber(sGroupIdInCTab, 3);
          this.writeWhiteSpace();
          this.write(sgroup.data.subscript || "n");
          this.writeCR();
        }
        sgroup.getAttachmentPoints().forEach((attachmentPoint) => {
          this.writeSGroupAttachmentPointLine(sGroupIdInCTab, attachmentPoint);
        });
        this.writeCR(
          common_default.saveToMolfile[sgroup.type](
            sgroup,
            molecule,
            sgmap,
            this.mapping,
            this.bondMapping
          )
        );
      }
      const expandedGroups = [];
      molecule.sgroups.forEach((sg) => {
        if (sg.isExpanded() && !SGroup.isQuerySGroup(sg))
          expandedGroups.push(sg.id + 1);
      });
      if (expandedGroups.length) {
        const expandedGroupsLine = `M  SDS EXP  ${expandedGroups.length}   ${expandedGroups.join("   ")}`;
        this.writeCR(expandedGroupsLine);
      }
      this.writeCR("M  END");
    }
    writeAtom(atom, atomLabel) {
      this.writePaddedFloat(atom.pp.x, 10, 4);
      this.writePaddedFloat(-atom.pp.y, 10, 4);
      this.writePaddedFloat(atom.pp.z, 10, 4);
      this.writeWhiteSpace();
      this.writePadded(atomLabel, 3);
      this.writePaddedNumber(0, 2);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      if (typeof atom.hCount === "undefined") {
        atom.hCount = 0;
      }
      this.writePaddedNumber(atom.hCount, 3);
      if (typeof atom.stereoCare === "undefined") {
        atom.stereoCare = 0;
      }
      this.writePaddedNumber(atom.stereoCare, 3);
      let number;
      if (atom.explicitValence < 0) {
        number = 0;
      } else if (atom.explicitValence === 0) {
        number = 15;
      } else {
        number = atom.explicitValence;
      }
      this.writePaddedNumber(number, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      this.writePaddedNumber(0, 3);
      if (typeof atom.aam === "undefined") {
        atom.aam = 0;
      }
      this.writePaddedNumber(atom.aam, 3);
      if (typeof atom.invRet === "undefined") {
        atom.invRet = 0;
      }
      this.writePaddedNumber(atom.invRet, 3);
      if (typeof atom.exactChangeFlag === "undefined") {
        atom.exactChangeFlag = 0;
      }
      this.writePaddedNumber(atom.exactChangeFlag, 3);
      this.writeCR();
    }
    writeBond(bond) {
      this.writePaddedNumber(this.mapping[bond.begin], 3);
      this.writePaddedNumber(this.mapping[bond.end], 3);
      this.writePaddedNumber(bond.type, 3);
      if (typeof bond.stereo === "undefined") {
        bond.stereo = 0;
      }
      this.writePaddedNumber(bond.stereo, 3);
      this.writePadded(bond.xxx, 3);
      if (typeof bond.topology === "undefined") {
        bond.topology = 0;
      }
      this.writePaddedNumber(bond.topology, 3);
      if (typeof bond.reactingCenterStatus === "undefined") {
        bond.reactingCenterStatus = 0;
      }
      this.writePaddedNumber(bond.reactingCenterStatus, 3);
      this.writeCR();
    }
    writeAtomProps(props) {
      this.write("A  ");
      this.writePaddedNumber(props.id + 1, 3);
      this.writeCR();
      this.writeCR(props.value);
    }
    writeAtomPropList(propId, values2) {
      while (values2.length > 0) {
        const part = [];
        while (values2.length > 0 && part.length < 8) {
          part.push(values2[0]);
          values2.splice(0, 1);
        }
        this.write(propId);
        this.writePaddedNumber(part.length, 3);
        part.forEach((value) => {
          this.writeWhiteSpace();
          this.writePaddedNumber(this.mapping[value[0]], 3);
          this.writeWhiteSpace();
          this.writePaddedNumber(value[1], 3);
        });
        this.writeCR();
      }
    }
    writeSGroupAttachmentPointLine(sgroupId, attachmentPoint) {
      var _a;
      this.write(`M  SAP`);
      this.writeWhiteSpace(1);
      this.writePaddedNumber(sgroupId, 3);
      this.writePaddedNumber(1, 3);
      this.writeWhiteSpace(1);
      const atomId = this.mapping[attachmentPoint.atomId];
      this.writePaddedNumber(atomId, 3);
      this.writeWhiteSpace(1);
      const leaveAtomId = (_a = this.mapping[attachmentPoint.leaveAtomId]) != null ? _a : 0;
      this.writePaddedNumber(leaveAtomId, 3);
      this.writeWhiteSpace(1);
      const attachmentId = attachmentPoint.attachmentId ? attachmentPoint.attachmentId.slice(0, 2) : "  ";
      this.writePadded(attachmentId, 2);
      this.writeCR();
    }
  };

  // src/core/io/mol/molSerializer.ts
  var import_utilities12 = __toESM(require_utilities());
  var _MolSerializer = class _MolSerializer {
    constructor(options) {
      __publicField(this, "options");
      this.options = __spreadValues(__spreadValues({}, _MolSerializer.DefaultOptions), options);
    }
    deserialize(content) {
      const molfile = new Molfile();
      const lines = content == null ? void 0 : content.split(/\r\n|[\n\r]/g);
      const parseCTFileParams = {
        molfileLines: lines,
        shouldReactionRelayout: this.options.reactionRelayout,
        ignoreChiralFlag: this.options.ignoreChiralFlag
      };
      try {
        return molfile.parseCTFile(parseCTFileParams);
      } catch (e) {
        console.error("molSerializer::MolSerializer::deserialize", e.message);
        if (this.options.badHeaderRecover) {
          try {
            return molfile.parseCTFile(__spreadProps(__spreadValues({}, parseCTFileParams), {
              molfileLines: lines.slice(1)
            }));
          } catch (e1) {
            console.error("molSerializer::MolSerializer::deserialize", e1.message);
          }
          try {
            return molfile.parseCTFile(__spreadProps(__spreadValues({}, parseCTFileParams), {
              molfileLines: [""].concat(lines)
            }));
          } catch (e2) {
            console.error("molSerializer::MolSerializer::deserialize", e2.message);
          }
        }
        return null;
      }
    }
    serialize(_struct) {
      const struct = KetSerializer.removeLeavingGroupsFromConnectedAtoms(_struct);
      return new Molfile().saveMolecule(
        struct,
        this.options.ignoreErrors,
        this.options.noRgroups,
        this.options.preserveIndigoDesc
      );
    }
  };
  __publicField(_MolSerializer, "DefaultOptions", {
    badHeaderRecover: false,
    ignoreErrors: false,
    noRgroups: false,
    preserveIndigoDesc: false,
    reactionRelayout: false
  });
  var MolSerializer = _MolSerializer;

  // src/core/io/sdf/sdfSerializer.ts
  var DelimeterRegex = /^[^]+?\$\$\$\$$/gm;
  var SdfSerializer = class {
    constructor(options) {
      __publicField(this, "molSerializerOptions");
      this.molSerializerOptions = options;
    }
    deserialize(content) {
      const result = [];
      const molSerializer = new MolSerializer(this.molSerializerOptions);
      let m = DelimeterRegex.exec(content);
      while (m !== null) {
        const chunk = m[0].replace(/\r/g, "").trim();
        const end = chunk.indexOf("M  END");
        if (end !== -1) {
          const propChunks = chunk.substr(end + 7).trim().split(/^$\n?/m);
          const struct = molSerializer.deserialize(chunk.substring(0, end + 6));
          const props = propChunks.reduce(
            (acc, pc) => {
              const m2 = /^> [ \d]*<(\S+)>/.exec(pc);
              if (m2) {
                const field = m2[1];
                const valueArr = pc.split("\n").slice(1, -1);
                let value = "";
                if (valueArr.length > 1) {
                  value = valueArr.join(",");
                } else {
                  value = pc.split("\n")[1].trim();
                }
                acc[field] = Number.isFinite(value) ? +value : value.toString();
              }
              return acc;
            },
            {}
          );
          result.push({ struct, props });
        }
        m = DelimeterRegex.exec(content);
      }
      return result;
    }
    serialize(sdfItems) {
      const molSerializer = new MolSerializer(this.molSerializerOptions);
      return sdfItems.reduce((res, item) => {
        res += molSerializer.serialize(item.struct);
        Object.keys(item.props).forEach((prop) => {
          res += `> <${prop}>
`;
          res += `${item.props[prop]}

`;
        });
        return `${res}$$$$
`;
      }, "");
    }
  };

  // entry.ts
  var QetcherJSWrapper = class {
    constructor() {
      this.manager = new DrawingEntitiesManager();
    }
    addMonomer(monomerProps, x, y) {
      monomerProps.struct = monomerProps.struct || {
        bonds: { filter: () => ({ size: 0 }) },
        sgroups: { filter: () => ({ get: () => null }) }
      };
      monomerProps.props = monomerProps.props || {
        MonomerName: monomerProps.label || "Unknown",
        MonomerType: "Peptide",
        MonomerCaps: {}
      };
      const vec = new Vec2(x, y);
      const monomer = new Peptide(monomerProps, vec);
      this.manager.addMonomerChangeModel(monomerProps, vec, monomer);
    }
    serializeStruct() {
      const result = [];
      this.manager.monomers.forEach((monomer, id2) => {
        result.push({
          type: "monomer",
          id: id2,
          x: monomer.position.x,
          y: monomer.position.y,
          label: monomer.monomerItem.label
        });
      });
      return result;
    }
  };
  return __toCommonJS(entry_exports);
})();
/*! Bundled license information:

lodash/lodash.js:
  (**
   * @license
   * Lodash <https://lodash.com/>
   * Copyright OpenJS Foundation and other contributors <https://openjsf.org/>
   * Released under MIT license <https://lodash.com/license>
   * Based on Underscore.js 1.8.3 <http://underscorejs.org/LICENSE>
   * Copyright Jeremy Ashkenas, DocumentCloud and Investigative Reporters & Editors
   *)
*/
//# sourceMappingURL=chem-core.js.map
