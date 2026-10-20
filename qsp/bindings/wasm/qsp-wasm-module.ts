/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// JS side of the WebAssembly binding: types of the module exports (qsp_wasm.h)
// and of the host object the engine calls (qsp-wasm-host.js)

type Brand<T> = T & { readonly brand: unique symbol };
export type Ptr = Brand<number>; // any pointer
export type CharsPtr = Brand<number>; // UTF-16 string terminated with 0
export type StringPtr = Brand<number>; // QSPString { Str, End }
export type VariantPtr = Brand<number>; // QSPVariant
export type ErrorPtr = Brand<number>; // QSPErrorInfo
export type IntPtr = Brand<number>;
export type BufferPtr = Brand<number>;
export type Bool = 0 | 1;

export enum QspWindow {
  MAIN = 1 << 0,
  VARS = 1 << 1,
  ACTS = 1 << 2,
  OBJS = 1 << 3,
  INPUT = 1 << 4,
  VIEW = 1 << 5,
  ALL = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5),
}

// Values (qsp.h): undefined, tuple, number (incl. bool), string (incl. code & varref)
export type QspVariableValue = string | number | QspTuple | undefined;
export type QspTuple = Array<QspVariableValue>;
// A key of an array item: a string, a tuple or a number (position of the item)
export type QspVariableKey = string | number | QspTuple;

export interface QspListItem {
  name: string;
  image: string;
}

export interface QspObjectItem {
  name: string;
  title: string;
  image: string;
}

// A code entry (lines the engine joins are one entry) and its first line, as errors number it
export interface QspLineInfo {
  line: number;
  code: string;
}

export interface QspErrorData {
  errorCode: number;
  description: string;
  location: string;
  actionIndex: number;
  line: number;
  localLine: number;
  lineSrc: string;
}

// The host object (module.qspHost), missing methods are ignored. A returned promise suspends the
// engine until it settles, unless it comes from a nested call, see qsp-wasm-host.js.
export interface QspWasmHost {
  onDebug?(code: string): void | Promise<void>;
  onSetTimer?(msecs: number): void | Promise<void>;
  onRefreshInt?(isForced: boolean, isNewLoc: boolean): void | Promise<void>;
  onSetInputStrText?(text: string): void | Promise<void>;
  onShowMessage?(text: string): void | Promise<void>;
  onShowPicture?(file: string): void | Promise<void>;
  onShowWindow?(type: QspWindow, toShow: boolean): void | Promise<void>;
  onPlayFile?(file: string, volume: number): void | Promise<void>;
  onIsPlayingFile?(file: string): boolean | Promise<boolean>;
  onCloseFile?(file: string): void | Promise<void>;
  onSystem?(cmd: string): void | Promise<void>;
  onSleep?(msecs: number): void | Promise<void>;
  onGetMSCount?(): number | Promise<number>;
  onOpenGame?(file: string, isNewGame: boolean): void | Promise<void>;
  onInitGame?(isNewGame: boolean): void | Promise<void>;
  onOpenGameStatus?(file: string): void | Promise<void>;
  onSaveGameStatus?(file: string): void | Promise<void>;
  onInputBox?(text: string): string | Promise<string>;
  onShowMenu?(items: Ptr, count: number): number | Promise<number>; // QSPListItem[count]
  onVersion?(param: string): string | Promise<string>;
}

// Names of the exports without the underscore, for ccall
type Unprefixed<K> = K extends `_${infer N}` ? N : never;
export type QspExportName = Unprefixed<Extract<keyof QspWasmModule, `_QSP${string}`>>;

// The exports write to the memory the JS side allocates and frees
export interface QspWasmModule {
  HEAP8: Int8Array;
  HEAPU8: Uint8Array;
  HEAPU16: Uint16Array;
  HEAP32: Int32Array;
  HEAPU32: Uint32Array;
  _malloc(size: number): Ptr;
  _free(ptr: number): void;

  qspHost?: QspWasmHost;
  // Resolves with the result when the engine finishes the call, even if a host promise suspended it
  ccall(
    name: QspExportName,
    returnType: 'number',
    argTypes: 'number'[],
    args: number[],
    opts: { async: true },
  ): Promise<number>;

  _QSPInit(): void;
  _QSPTerminate(): void;
  _QSPGetVersion(res: StringPtr): void;
  _QSPGetCompiledDateTime(res: StringPtr): void;

  _QSPEnableDebugMode(isDebug: Bool): void;
  _QSPGetCurStateData(loc: StringPtr, actIndex: IntPtr, lineNum: IntPtr): void;
  _QSPGetLocationNames(locNames: StringPtr, bufSize: number): number;
  _QSPGetLocationDesc(name: CharsPtr, res: StringPtr): void;
  // -1 if there is no location
  _QSPGetLocationActions(name: CharsPtr, actions: Ptr, bufSize: number): number;
  _QSPGetLocationActionCode(
    name: CharsPtr,
    index: number,
    lines: Ptr,
    bufSize: number,
  ): number;
  _QSPGetLocationCode(name: CharsPtr, lines: Ptr, bufSize: number): number;
  _QSPGetActionCode(index: number, lines: Ptr, bufSize: number): number; // a current action

  _QSPGetCurLocation(res: StringPtr): void;

  _QSPGetMainDesc(res: StringPtr): void;
  _QSPGetVarsDesc(res: StringPtr): void;

  _QSPGetActions(items: Ptr, bufSize: number): number;
  _QSPSetSelActionIndex(index: number, toRefreshUI: Bool): Bool;
  _QSPExecuteSelActionCode(toRefreshUI: Bool): Bool;
  _QSPGetSelActionIndex(): number;

  _QSPGetObjects(items: Ptr, bufSize: number): number;
  _QSPSetSelObjectIndex(index: number, toRefreshUI: Bool): Bool;
  _QSPGetSelObjectIndex(): number;

  _QSPGetWindowsChangedState(): number;
  _QSPShowWindow(type: number, toShow: Bool): void;

  _QSPGetVarValuesCount(name: CharsPtr, count: IntPtr): Bool;
  _QSPGetVarIndexByKey(name: CharsPtr, key: VariantPtr, index: IntPtr): Bool; // index -1 if none
  _QSPGetVarValue(name: CharsPtr, index: number, value: VariantPtr | 0, key: VariantPtr | 0): Bool;

  _QSPExecString(code: CharsPtr, toRefreshUI: Bool): Bool;
  _QSPCalculateStrExpression(
    expression: CharsPtr,
    buf: CharsPtr,
    bufSize: number,
    toRefreshUI: Bool,
  ): Bool;
  _QSPCalculateNumExpression(expression: CharsPtr, res: IntPtr, toRefreshUI: Bool): Bool;
  _QSPExecLocationCode(name: CharsPtr, toRefreshUI: Bool): Bool;
  _QSPExecCounter(toRefreshUI: Bool): Bool;
  _QSPExecUserInput(text: CharsPtr, toRefreshUI: Bool): Bool;

  _QSPGetLastErrorData(res: ErrorPtr): void;
  _QSPGetErrorDesc(errorNum: number, res: StringPtr): void;

  _QSPLoadGameWorldFromData(data: BufferPtr, size: number, isNewGame: Bool): Bool;
  _QSPSaveGameAsData(buf: BufferPtr, size: IntPtr, toRefreshUI: Bool): Bool; // size: in/out
  _QSPOpenSavedGameFromData(data: BufferPtr, size: number, toRefreshUI: Bool): Bool;
  _QSPRestartGame(toRefreshUI: Bool): Bool;

  _QSPRunChecks(): void;
}

export interface QspModuleOptions {
  wasmBinary?: ArrayBuffer | ArrayBufferView;
  qspHost?: QspWasmHost;
}
