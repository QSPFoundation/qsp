/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Host calls (wasm_callbacks.c): Module.qspHost[name](...), missing methods are ignored.
// A returned promise suspends the engine (Asyncify) until it settles, the running export then
// completes later (call it with ccall(..., {async: true})). Only the outermost host call can
// suspend, a promise from a nested one (the host calling the API) is ignored.

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
  qspWasmHostInputBox: (text, bytes, buffer, maxLen) =>
    qspHostCall('onInputBox', () => [UTF16ToString(text, bytes)], (res) =>
      qspWriteString(res, buffer, maxLen),
    ),
  qspWasmHostShowMenu: (items, count) =>
    qspHostCall('onShowMenu', () => [items, count], (res) => (Number.isInteger(res) ? res : -1)),
  qspWasmHostVersion: (param, bytes, buffer, maxLen) =>
    qspHostCall('onVersion', () => [UTF16ToString(param, bytes)], (res) =>
      qspWriteString(res, buffer, maxLen),
    ),
};

for (const name of Object.keys(hostCalls)) {
  hostCalls[`${name}__deps`] = ['$qspHostCall', '$qspWriteString', '$UTF16ToString'];
  hostCalls[`${name}__async`] = true;
}

addToLibrary({
  ...hostCalls,

  $qspHostDepth: 0, // host calls in progress

  // getArgs decodes the arguments only if the host has the method
  $qspHostCall__deps: ['$Asyncify', '$qspHostDepth'],
  $qspHostCall: (name, getArgs, convert) =>
    Asyncify.handleSleep((wakeUp) => {
      const host = Module['qspHost'];
      const done = (res) => {
        let value = 0;
        --qspHostDepth;
        try {
          if (convert) value = convert(res);
        } catch (e) {
          console.error(e);
        }
        wakeUp(value);
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

  // A longer string is cut, no answer (null, undefined) leaves the buffer empty
  $qspWriteString__deps: ['$stringToUTF16'],
  $qspWriteString: (str, buffer, maxLen) => {
    if (str != null) stringToUTF16(String(str), buffer, (maxLen + 1) * 2);
  },
});
