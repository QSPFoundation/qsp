/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Heap access: strings, lists & variants in the layout of qsp.h (32-bit pointers)

import {
  BufferPtr,
  CharsPtr,
  ErrorPtr,
  IntPtr,
  Ptr,
  QspErrorData,
  QspLineInfo,
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
const STRING_SIZE = POINTER_SIZE * 2; // QSPString { Str, End }
const LIST_ITEM_SIZE = STRING_SIZE * 2; // QSPListItem
const OBJECT_ITEM_SIZE = STRING_SIZE * 3; // QSPObjectItem
const LINE_INFO_SIZE = POINTER_SIZE + STRING_SIZE; // QSPLineInfo { LineNum, Line }

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

export function writeString(module: QspWasmModule, value: string): CharsPtr {
  const ptr = module._malloc((value.length + 1) * CHAR_SIZE) as number as CharsPtr;
  writeChars(module, value, ptr);
  return ptr;
}

// The helpers run exports that execute game code: the buffers live until the call settles

// The export writes at most maxLength characters (a longer string is cut)
export async function withCharsRead(
  module: QspWasmModule,
  maxLength: number,
  callback: (buffer: CharsPtr, size: number) => Promise<number>,
): Promise<string> {
  const buffer = module._malloc((maxLength + 1) * CHAR_SIZE) as number as CharsPtr;
  module.HEAPU16[buffer >> 1] = 0;
  await callback(buffer, maxLength + 1);
  const codes = module.HEAPU16;
  let length = 0;
  while (length < maxLength && codes[(buffer >> 1) + length]) ++length;
  const result = readChars(module, buffer, length);
  freePointer(module, buffer);
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

// Reusable memory for the lists the exports write to, grows when a list doesn't fit.
// Shared by the calls that only read the state: no game code can run while a list is read.
export interface ListBuffer {
  ptr: Ptr;
  size: number; // in bytes
}

export function allocListBuffer(module: QspWasmModule, size = 4096): ListBuffer {
  return { ptr: module._malloc(size), size };
}

export function freeListBuffer(module: QspWasmModule, buffer: ListBuffer): void {
  freePointer(module, buffer.ptr);
}

// The call is repeated with a bigger buffer if the list did not fit
function readList<T>(
  module: QspWasmModule,
  buffer: ListBuffer,
  itemSize: number,
  callback: (items: Ptr, bufSize: number) => number,
  read: (module: QspWasmModule, items: Ptr, count: number) => T[],
): T[] {
  let capacity = Math.floor(buffer.size / itemSize);
  let count = callback(buffer.ptr, capacity);
  if (count > capacity) {
    freePointer(module, buffer.ptr);
    buffer.size = Math.max(count * itemSize, buffer.size * 2);
    buffer.ptr = module._malloc(buffer.size);
    capacity = Math.floor(buffer.size / itemSize);
    count = callback(buffer.ptr, capacity);
  }
  return count > 0 ? read(module, buffer.ptr, count) : [];
}

export function withListRead(
  module: QspWasmModule,
  buffer: ListBuffer,
  callback: (items: Ptr, bufSize: number) => number,
): QspListItem[] {
  return readList(module, buffer, LIST_ITEM_SIZE, callback, readListItems);
}

export function withObjectRead(
  module: QspWasmModule,
  buffer: ListBuffer,
  callback: (items: Ptr, bufSize: number) => number,
): QspObjectItem[] {
  return readList(module, buffer, OBJECT_ITEM_SIZE, callback, readObjectItems);
}

export function withStringListRead(
  module: QspWasmModule,
  buffer: ListBuffer,
  callback: (items: Ptr, bufSize: number) => number,
): string[] {
  return readList(module, buffer, STRING_SIZE, callback, readStringList);
}

export function withLinesRead(
  module: QspWasmModule,
  buffer: ListBuffer,
  callback: (lines: Ptr, bufSize: number) => number,
): QspLineInfo[] {
  return readList(module, buffer, LINE_INFO_SIZE, callback, readLines);
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

// Too small a buffer fails with the required size, then the call is repeated; size 0 is an error
export async function withBufferRead(
  module: QspWasmModule,
  callback: (buffer: BufferPtr, size: IntPtr) => Promise<number>,
  initialSize = 64 * 1024,
): Promise<ArrayBuffer | null> {
  const sizePtr = allocPointer(module);
  let size = initialSize;
  let buffer = module._malloc(size) as number as BufferPtr;
  let data: ArrayBuffer | null = null;
  for (;;) {
    module.HEAP32[sizePtr >> 2] = size;
    const isSaved = await callback(buffer, sizePtr);
    size = readInt(module, sizePtr);
    if (isSaved) {
      data = module.HEAPU8.slice(buffer, buffer + size).buffer;
      break;
    }
    if (!size) break;
    freePointer(module, buffer);
    buffer = module._malloc(size) as number as BufferPtr;
  }
  freePointer(module, buffer);
  freePointer(module, sizePtr);
  return data;
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

// QSPLineInfo { LineNum, Line }
export function readLines(module: QspWasmModule, listPtr: Ptr, count: number): QspLineInfo[] {
  const list: QspLineInfo[] = [];
  let ptr = listPtr as number as StringPtr;
  for (let i = 0; i < count; ++i) {
    const line = readInt(module, ptr);
    const code = readString(module, movePointer(ptr) as number as StringPtr);
    list.push({ line, code });
    ptr = (ptr + LINE_INFO_SIZE) as StringPtr;
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

// Strings and tuple items are allocated, freeVariantData frees them
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

// Short strings are decoded faster by hand
function readChars(module: QspWasmModule, ptr: number, count: number): string {
  if (utf16Decoder && count > 16) {
    return utf16Decoder.decode(module.HEAPU8.subarray(ptr, ptr + count * CHAR_SIZE));
  }
  const codes = module.HEAPU16;
  let str = '';
  for (let i = ptr >> 1, end = i + count; i < end; ++i) str += String.fromCharCode(codes[i]);
  return str;
}

function writeChars(module: QspWasmModule, str: string, ptr: number): void {
  const codes = module.HEAPU16;
  let i = ptr >> 1;
  for (let k = 0; k < str.length; ++k) codes[i++] = str.charCodeAt(k);
  codes[i] = 0;
}
