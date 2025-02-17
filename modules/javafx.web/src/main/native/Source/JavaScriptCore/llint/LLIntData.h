/*
 * Copyright (C) 2011-2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "JSCJSValue.h"
#include "MacroAssemblerCodeRef.h"
#include "Opcode.h"

namespace JSC {

class VM;
struct Instruction;

#if ENABLE(C_LOOP)
typedef OpcodeID LLIntCode;
#else
typedef void (*LLIntCode)();
#endif

namespace LLInt {

extern "C" JS_EXPORT_PRIVATE Opcode g_opcodeMap[numOpcodeIDs];
extern "C" JS_EXPORT_PRIVATE Opcode g_opcodeMapWide[numOpcodeIDs];

class Data {

public:
    static void performAssertions(VM&);

private:
    static uint8_t s_exceptionInstructions[maxOpcodeLength + 1];

    friend void initialize();

    friend Instruction* exceptionInstructions();
    friend Opcode* opcodeMap();
    friend Opcode* opcodeMapWide();
    friend Opcode getOpcode(OpcodeID);
    friend Opcode getOpcodeWide(OpcodeID);
    template<PtrTag tag> friend MacroAssemblerCodePtr<tag> getCodePtr(OpcodeID);
    template<PtrTag tag> friend MacroAssemblerCodePtr<tag> getWideCodePtr(OpcodeID);
    template<PtrTag tag> friend MacroAssemblerCodeRef<tag> getCodeRef(OpcodeID);
};

void initialize();

inline Instruction* exceptionInstructions()
{
    return reinterpret_cast<Instruction*>(Data::s_exceptionInstructions);
}

inline Opcode* opcodeMap()
{
    return g_opcodeMap;
}

inline Opcode* opcodeMapWide()
{
    return g_opcodeMapWide;
}

inline Opcode getOpcode(OpcodeID id)
{
#if ENABLE(COMPUTED_GOTO_OPCODES)
    return g_opcodeMap[id];
#else
    return static_cast<Opcode>(id);
#endif
}

inline Opcode getOpcodeWide(OpcodeID id)
{
#if ENABLE(COMPUTED_GOTO_OPCODES)
    return g_opcodeMapWide[id];
#else
    UNUSED_PARAM(id);
    RELEASE_ASSERT_NOT_REACHED();
#endif
}

template<PtrTag tag>
ALWAYS_INLINE MacroAssemblerCodePtr<tag> getCodePtr(OpcodeID opcodeID)
{
    void* address = reinterpret_cast<void*>(getOpcode(opcodeID));
    address = retagCodePtr<BytecodePtrTag, tag>(address);
    return MacroAssemblerCodePtr<tag>::createFromExecutableAddress(address);
}

template<PtrTag tag>
ALWAYS_INLINE MacroAssemblerCodePtr<tag> getWideCodePtr(OpcodeID opcodeID)
{
    void* address = reinterpret_cast<void*>(getOpcodeWide(opcodeID));
    address = retagCodePtr<BytecodePtrTag, tag>(address);
    return MacroAssemblerCodePtr<tag>::createFromExecutableAddress(address);
}

template<PtrTag tag>
ALWAYS_INLINE MacroAssemblerCodeRef<tag> getCodeRef(OpcodeID opcodeID)
{
    return MacroAssemblerCodeRef<tag>::createSelfManagedCodeRef(getCodePtr<tag>(opcodeID));
}

#if ENABLE(JIT)
template<PtrTag tag>
ALWAYS_INLINE LLIntCode getCodeFunctionPtr(OpcodeID opcodeID)
{
    ASSERT(opcodeID >= NUMBER_OF_BYTECODE_IDS);
#if COMPILER(MSVC)
    return reinterpret_cast<LLIntCode>(getCodePtr<tag>(opcodeID).executableAddress());
#else
    return reinterpret_cast<LLIntCode>(getCodePtr<tag>(opcodeID).template executableAddress());
#endif
}

#else
ALWAYS_INLINE void* getCodePtr(OpcodeID id)
{
    return reinterpret_cast<void*>(getOpcode(id));
}

ALWAYS_INLINE void* getWideCodePtr(OpcodeID id)
{
    return reinterpret_cast<void*>(getOpcodeWide(id));
}
#endif

ALWAYS_INLINE void* getCodePtr(JSC::EncodedJSValue glueHelper())
{
    return bitwise_cast<void*>(glueHelper);
}

} } // namespace JSC::LLInt
