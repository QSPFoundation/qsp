/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Host calls of the WebAssembly binding (see wasm_callbacks.c), linked with --js-library.
// The JS side attaches the host object to the module (Module.qspHost), missing methods are ignored.
// A method can return a promise, then the engine is suspended (Asyncify) until it settles and the
// running export completes later (call such exports with ccall(..., {async: true})). Only the
// outermost host call can suspend, a promise returned from a nested call (a host method calling
// the API, an API call while another one is suspended) is ignored.

const hostCalls = {
  qspWasmHostDebug: (str, bytes) => qspHostCall('onDebug', () => [UTF16ToString(str, bytes)]),
  qspWasmHostSetTimer: (msecs) => qspHostCall('onSetTimer', () => [msecs]),
  qspWasmHostRefreshInt: (isForced, isNewLoc) =>
    qspHostCall('onRefreshInt', () => [!!isForced, !!isNewLoc]),
  qspWasmHostSetInputStrText: (text, bytes) =>
    qspHostCall('onSetInputStrText', () => [UTF16ToString(text, bytes)]),
  qspWasmHostShowMessage: (text, bytes) =>
    qspHostCall('onShowMessage', () => [UTF16ToString(text, bytes)]),
  qspWasmHostShowPicture: (file, bytes) =>
    qspHostCall('onShowPicture', () => [UTF16ToString(file, bytes)]),
  qspWasmHostShowWindow: (type, toShow) => qspHostCall('onShowWindow', () => [type, !!toShow]),
  qspWasmHostPlayFile: (file, bytes, volume) =>
    qspHostCall('onPlayFile', () => [UTF16ToString(file, bytes), volume]),
  qspWasmHostIsPlayingFile: (file, bytes) =>
    qspHostCall('onIsPlayingFile', () => [UTF16ToString(file, bytes)], (res) => (res ? 1 : 0)),
  qspWasmHostCloseFile: (file, bytes) =>
    qspHostCall('onCloseFile', () => [UTF16ToString(file, bytes)]),
  qspWasmHostSystem: (cmd, bytes) => qspHostCall('onSystem', () => [UTF16ToString(cmd, bytes)]),
  qspWasmHostSleep: (msecs) => qspHostCall('onSleep', () => [msecs]),
  qspWasmHostGetMSCount: () => qspHostCall('onGetMSCount', () => [], (res) => res | 0),
  qspWasmHostOpenGame: (file, bytes, isNewGame) =>
    qspHostCall('onOpenGame', () => [UTF16ToString(file, bytes), !!isNewGame]),
  qspWasmHostInitGame: (isNewGame) => qspHostCall('onInitGame', () => [!!isNewGame]),
  qspWasmHostOpenGameStatus: (file, bytes) =>
    qspHostCall('onOpenGameStatus', () => [UTF16ToString(file, bytes)]),
  qspWasmHostSaveGameStatus: (file, bytes) =>
    qspHostCall('onSaveGameStatus', () => [UTF16ToString(file, bytes)]),
  qspWasmHostInputBox: (text, bytes) =>
    qspHostCall('onInputBox', () => [UTF16ToString(text, bytes)], qspAllocString),
  qspWasmHostShowMenu: (items, count) =>
    qspHostCall('onShowMenu', () => [items, count], (res) => (Number.isInteger(res) ? res : -1)),
  qspWasmHostVersion: (param, bytes) =>
    qspHostCall('onVersion', () => [UTF16ToString(param, bytes)], qspAllocString),
};

for (const name of Object.keys(hostCalls)) {
  hostCalls[`${name}__deps`] = ['$qspHostCall', '$qspAllocString', '$UTF16ToString'];
  hostCalls[`${name}__async`] = true; // can suspend the engine (ASYNCIFY_IMPORTS)
}

addToLibrary({
  ...hostCalls,

  $qspHostDepth: 0, // host calls in progress

  // Calls Module.qspHost[name](...getArgs()), the result (converted to the C value) is returned
  // to the engine; the arguments are prepared once, not again when the engine resumes
  $qspHostCall__deps: ['$Asyncify', '$qspHostDepth'],
  $qspHostCall: (name, getArgs, convert) =>
    Asyncify.handleSleep((wakeUp) => {
      const host = Module['qspHost'];
      const done = (res) => {
        --qspHostDepth;
        wakeUp(convert ? convert(res) : 0);
      };
      let res;
      ++qspHostDepth;
      if (host && host[name]) {
        try {
          res = host[name](...getArgs());
        } catch (e) {
          console.error(e);
        }
      }
      if (!(res && typeof res.then === 'function')) return done(res);
      if (qspHostDepth > 1) {
        console.error(`QSP: the promise returned by ${name} is ignored, the engine can't wait`);
        res.then(undefined, (e) => console.error(e));
        return done(undefined);
      }
      res.then(done, (e) => {
        console.error(e);
        done(undefined);
      });
    }),

  // Strings returned by the host are allocated with malloc & owned by the engine, 0 if there's none
  $qspAllocString__deps: ['$lengthBytesUTF16', '$stringToUTF16', 'malloc'],
  $qspAllocString: (str) => {
    if (str == null) return 0;
    str = String(str);
    const size = lengthBytesUTF16(str) + 2;
    const ptr = _malloc(size);
    stringToUTF16(str, ptr, size);
    return ptr;
  },
});
