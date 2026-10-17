/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Access to the module memory: strings, lists & variants in the layout of qsp.h (32-bit pointers)

import {
  BufferPtr,
  CharsPtr,
  ErrorPtr,
  IntPtr,
  Ptr,
  PtrPtr,
  QspErrorData,
  QspListItem,
  QspObjectItem,
  QspTuple,
  QspVariableKey,
  QspVariableValue,
  QspWasmModule,
  StringPtr,
  VariantPtr,
} from './qsp-wasm-module';

export const POINTER_SIZE = 4;
const CHAR_SIZE = 2; // UTF-16 code units, the same as JS strings
const VARIANT_SIZE = POINTER_SIZE * 3; // QSPVariant: 2-word union + type byte, padded

// value types from qsp.h
const QSP_TYPE_UNDEF = 0;
const QSP_TYPE_TUPLE = 1;
const QSP_TYPE_NUM = 2;
const QSP_TYPE_BOOL = 3;
const QSP_TYPE_STR = 4;
const QSP_TYPE_CODE = 5;
const QSP_TYPE_VARREF = 6;

export function allocPointer(module: QspWasmModule): IntPtr {
  return module._malloc(POINTER_SIZE) as number as IntPtr;
}

function allocPtrPointer(module: QspWasmModule): PtrPtr {
  return module._malloc(POINTER_SIZE) as number as PtrPtr;
}

export function allocStringPointer(module: QspWasmModule): StringPtr {
  return module._malloc(POINTER_SIZE * 2) as number as StringPtr;
}

export function allocVariantPointer(module: QspWasmModule): VariantPtr {
  return module._malloc(VARIANT_SIZE) as number as VariantPtr;
}

function variantAt(items: Ptr, index: number): VariantPtr {
  return (items + index * VARIANT_SIZE) as number as VariantPtr;
}

export function allocErrorInfoPointer(module: QspWasmModule): ErrorPtr {
  return module._malloc(POINTER_SIZE * 10) as number as ErrorPtr;
}

export function freePointer(module: QspWasmModule, ptr: number): void {
  module._free(ptr);
}

export function movePointer<T extends number>(ptr: T, times = 1): T {
  return (ptr + POINTER_SIZE * times) as T;
}

export function readInt(module: QspWasmModule, ptr: number): number {
  return module.HEAP32[ptr >> 2];
}

export function derefPointer(module: QspWasmModule, ptr: number): Ptr {
  return module.HEAPU32[ptr >> 2] as Ptr;
}

function readByte(module: QspWasmModule, ptr: number): number {
  return module.HEAP8[ptr];
}

// QSPString { Str, End }
export function readString(module: QspWasmModule, ptr: StringPtr): string {
  const start = derefPointer(module, ptr);
  if (!start) return '';
  const end = derefPointer(module, movePointer(ptr));
  return readChars(module, start, (end - start) / CHAR_SIZE);
}

// Allocates a UTF-16 string terminated with 0
export function writeString(module: QspWasmModule, value: string): CharsPtr {
  const ptr = module._malloc((value.length + 1) * CHAR_SIZE) as number as CharsPtr;
  writeChars(module, value, ptr);
  return ptr;
}

// The callbacks of the helpers below run exports that execute game code and may return a promise
// (see ccall), the buffers live until it settles

// Reads a string allocated by the module and frees it
export async function withAllocatedStringRead(
  module: QspWasmModule,
  callback: (ptr: StringPtr) => unknown,
): Promise<string> {
  const ptr = allocStringPointer(module);
  await callback(ptr);
  const result = readString(module, ptr);
  freePointer(module, derefPointer(module, ptr));
  freePointer(module, ptr);
  return result;
}

export async function withStringWrite<T>(
  module: QspWasmModule,
  value: string,
  callback: (ptr: CharsPtr) => T | Promise<T>,
): Promise<T> {
  const ptr = writeString(module, value);
  const result = await callback(ptr);
  freePointer(module, ptr);
  return result;
}

// Lists allocated by the module: the callback returns the count and writes the list to the pointer
function withAllocatedListRead<T>(
  module: QspWasmModule,
  callback: (list: PtrPtr) => number,
  read: (module: QspWasmModule, list: Ptr, count: number) => T[],
): T[] {
  const listPtr = allocPtrPointer(module);
  const count = callback(listPtr);
  const list = derefPointer(module, listPtr);
  const items = read(module, list, count);
  freePointer(module, list);
  freePointer(module, listPtr);
  return items;
}

export function withListRead(
  module: QspWasmModule,
  callback: (list: PtrPtr) => number,
): QspListItem[] {
  return withAllocatedListRead(module, callback, readListItems);
}

export function withObjectRead(
  module: QspWasmModule,
  callback: (list: PtrPtr) => number,
): QspObjectItem[] {
  return withAllocatedListRead(module, callback, readObjectItems);
}

export function withStringListRead(
  module: QspWasmModule,
  callback: (list: PtrPtr) => number,
): string[] {
  return withAllocatedListRead(module, callback, readStringList);
}

export async function withBufferWrite<T>(
  module: QspWasmModule,
  data: ArrayBuffer,
  callback: (ptr: BufferPtr, size: number) => T | Promise<T>,
): Promise<T> {
  const bytes = new Uint8Array(data);
  const ptr = module._malloc(bytes.length) as number as BufferPtr;
  module.HEAPU8.set(bytes, ptr);
  const result = await callback(ptr, bytes.length);
  freePointer(module, ptr);
  return result;
}

// Buffers allocated by the module: the callback writes the buffer & its size to the pointers
export async function withBufferRead(
  module: QspWasmModule,
  callback: (buffer: PtrPtr, size: IntPtr) => unknown,
): Promise<ArrayBuffer | null> {
  const bufferPtr = allocPtrPointer(module);
  const sizePtr = allocPointer(module);
  await callback(bufferPtr, sizePtr);
  const buffer = derefPointer(module, bufferPtr);
  const size = readInt(module, sizePtr);
  freePointer(module, sizePtr);
  freePointer(module, bufferPtr);
  if (!buffer) return null;
  const data = module.HEAPU8.slice(buffer, buffer + size);
  freePointer(module, buffer);
  return data.buffer;
}

// QSPListItem { Name, Image }
export function readListItems(module: QspWasmModule, listPtr: Ptr, count: number): QspListItem[] {
  const list: QspListItem[] = [];
  let ptr = listPtr as number as StringPtr;
  for (let i = 0; i < count; ++i) {
    const name = readString(module, ptr);
    ptr = movePointer(ptr, 2);
    const image = readString(module, ptr);
    ptr = movePointer(ptr, 2);
    list.push({ name, image });
  }
  return list;
}

// QSPObjectItem { Name, Title, Image }
export function readObjectItems(
  module: QspWasmModule,
  listPtr: Ptr,
  count: number,
): QspObjectItem[] {
  const list: QspObjectItem[] = [];
  let ptr = listPtr as number as StringPtr;
  for (let i = 0; i < count; ++i) {
    const name = readString(module, ptr);
    ptr = movePointer(ptr, 2);
    const title = readString(module, ptr);
    ptr = movePointer(ptr, 2);
    const image = readString(module, ptr);
    ptr = movePointer(ptr, 2);
    list.push({ name, title, image });
  }
  return list;
}

export function readStringList(module: QspWasmModule, listPtr: Ptr, count: number): string[] {
  const list: string[] = [];
  let ptr = listPtr as number as StringPtr;
  for (let i = 0; i < count; ++i) {
    list.push(readString(module, ptr));
    ptr = movePointer(ptr, 2);
  }
  return list;
}

// QSPErrorInfo { ErrorNum, ErrorDesc, LocName, ActIndex, TopLineNum, IntLineNum, IntLine }
export function readError(module: QspWasmModule, errorPtr: ErrorPtr): QspErrorData {
  return {
    errorCode: readInt(module, errorPtr),
    description: readString(module, movePointer(errorPtr) as number as StringPtr),
    location: readString(module, movePointer(errorPtr, 3) as number as StringPtr),
    actionIndex: readInt(module, movePointer(errorPtr, 5)),
    line: readInt(module, movePointer(errorPtr, 6)),
    localLine: readInt(module, movePointer(errorPtr, 7)),
    lineSrc: readString(module, movePointer(errorPtr, 8) as number as StringPtr),
  };
}

export function withErrorRead(
  module: QspWasmModule,
  callback: (ptr: ErrorPtr) => void,
): QspErrorData {
  const ptr = allocErrorInfoPointer(module);
  callback(ptr);
  const error = readError(module, ptr);
  freePointer(module, ptr);
  return error;
}

// Variants: the stored value as is, undefined for QSP_TYPE_UNDEF
export function readVariant(module: QspWasmModule, variant: VariantPtr): QspVariableValue {
  switch (readByte(module, movePointer(variant, 2))) {
    case QSP_TYPE_TUPLE:
      return readTuple(module, variant);
    case QSP_TYPE_NUM:
    case QSP_TYPE_BOOL:
      return readInt(module, variant);
    case QSP_TYPE_STR:
    case QSP_TYPE_CODE:
    case QSP_TYPE_VARREF:
      return readString(module, variant as number as StringPtr);
    case QSP_TYPE_UNDEF:
    default:
      return undefined;
  }
}

export function withVariantWrite<T>(
  module: QspWasmModule,
  value: QspVariableKey,
  callback: (ptr: VariantPtr) => T,
): T {
  const ptr = allocVariantPointer(module);
  writeVariant(module, ptr, value);
  const result = callback(ptr);
  freeVariantData(module, ptr);
  freePointer(module, ptr);
  return result;
}

function readTuple(module: QspWasmModule, variant: VariantPtr): QspTuple {
  const values: QspTuple = [];
  const list = derefPointer(module, variant);
  const count = readInt(module, movePointer(variant));
  for (let i = 0; i < count; ++i) {
    values.push(readVariant(module, variantAt(list, i)));
  }
  return values;
}

// Writes a variant in the layout readVariant expects, strings and tuple items are allocated
function writeVariant(module: QspWasmModule, variant: VariantPtr, value: QspVariableValue): void {
  module.HEAPU8.fill(0, variant, variant + VARIANT_SIZE); // undefined
  if (value === undefined) return;
  if (typeof value === 'number') {
    module.HEAP32[variant >> 2] = value;
    module.HEAP8[variant + POINTER_SIZE * 2] = QSP_TYPE_NUM;
  } else if (typeof value === 'string') {
    const length = value.length * CHAR_SIZE;
    const str = module._malloc(length + CHAR_SIZE);
    writeChars(module, value, str);
    module.HEAPU32[variant >> 2] = str;
    module.HEAPU32[(variant >> 2) + 1] = str + length;
    module.HEAP8[variant + POINTER_SIZE * 2] = QSP_TYPE_STR;
  } else {
    const items = module._malloc(VARIANT_SIZE * value.length);
    for (let i = 0; i < value.length; ++i) {
      writeVariant(module, variantAt(items, i), value[i]);
    }
    module.HEAPU32[variant >> 2] = items;
    module.HEAP32[(variant >> 2) + 1] = value.length;
    module.HEAP8[variant + POINTER_SIZE * 2] = QSP_TYPE_TUPLE;
  }
}

function freeVariantData(module: QspWasmModule, variant: VariantPtr): void {
  switch (readByte(module, movePointer(variant, 2))) {
    case QSP_TYPE_STR:
      freePointer(module, derefPointer(module, variant));
      break;
    case QSP_TYPE_TUPLE: {
      const items = derefPointer(module, variant);
      const count = readInt(module, movePointer(variant));
      for (let i = 0; i < count; ++i) {
        freeVariantData(module, variantAt(items, i));
      }
      freePointer(module, items);
      break;
    }
  }
}

const utf16Decoder = typeof TextDecoder === 'undefined' ? undefined : new TextDecoder('utf-16le');

// Reads count code units, short strings are faster to decode by hand
function readChars(module: QspWasmModule, ptr: number, count: number): string {
  if (utf16Decoder && count > 16) {
    return utf16Decoder.decode(module.HEAPU8.subarray(ptr, ptr + count * CHAR_SIZE));
  }
  const codes = module.HEAPU16;
  let str = '';
  for (let i = ptr >> 1, end = i + count; i < end; ++i) str += String.fromCharCode(codes[i]);
  return str;
}

// Writes the string and the terminator
function writeChars(module: QspWasmModule, str: string, ptr: number): void {
  const codes = module.HEAPU16;
  let i = ptr >> 1;
  for (let k = 0; k < str.length; ++k) codes[i++] = str.charCodeAt(k);
  codes[i] = 0;
}
