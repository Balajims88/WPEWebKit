/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "JSCell.h"
#include "Operations.h"
#include "PropertyNameArray.h"
#include "Structure.h"

namespace JSC {

class JSPropertyNameEnumerator final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static JSPropertyNameEnumerator* create(VM&);
    static JSPropertyNameEnumerator* create(VM&, Structure*, uint32_t, uint32_t, PropertyNameArray&&);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

    JSString* propertyNameAtIndex(uint32_t index) const
    {
        if (index >= m_propertyNames.size())
            return nullptr;
        return m_propertyNames[index].get();
    }

    StructureChain* cachedPrototypeChain() const { return m_prototypeChain.get(); }
    void setCachedPrototypeChain(VM& vm, StructureChain* prototypeChain) { return m_prototypeChain.set(vm, this, prototypeChain); }

    Structure* cachedStructure(VM& vm) const
    {
        if (!m_cachedStructureID)
            return nullptr;
        return vm.heap.structureIDTable().get(m_cachedStructureID);
    }
    StructureID cachedStructureID() const { return m_cachedStructureID; }
    uint32_t indexedLength() const { return m_indexedLength; }
    uint32_t endStructurePropertyIndex() const { return m_endStructurePropertyIndex; }
    uint32_t endGenericPropertyIndex() const { return m_endGenericPropertyIndex; }
    uint32_t cachedInlineCapacity() const { return m_cachedInlineCapacity; }
    static ptrdiff_t cachedStructureIDOffset() { return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_cachedStructureID); }
    static ptrdiff_t indexedLengthOffset() { return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_indexedLength); }
    static ptrdiff_t endStructurePropertyIndexOffset() { return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_endStructurePropertyIndex); }
    static ptrdiff_t endGenericPropertyIndexOffset() { return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_endGenericPropertyIndex); }
    static ptrdiff_t cachedInlineCapacityOffset() { return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_cachedInlineCapacity); }
    static ptrdiff_t cachedPropertyNamesVectorOffset()
    {
        return OBJECT_OFFSETOF(JSPropertyNameEnumerator, m_propertyNames) + Vector<WriteBarrier<JSString>>::dataMemoryOffset();
    }

    static void visitChildren(JSCell*, SlotVisitor&);

private:
    JSPropertyNameEnumerator(VM&, StructureID, uint32_t);
    void finishCreation(VM&, uint32_t, uint32_t, RefPtr<PropertyNameArrayData>&&);

    Vector<WriteBarrier<JSString>> m_propertyNames;
    StructureID m_cachedStructureID;
    WriteBarrier<StructureChain> m_prototypeChain;
    uint32_t m_indexedLength;
    uint32_t m_endStructurePropertyIndex;
    uint32_t m_endGenericPropertyIndex;
    uint32_t m_cachedInlineCapacity;
};

inline JSPropertyNameEnumerator* propertyNameEnumerator(ExecState* exec, JSObject* base)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    uint32_t indexedLength = base->methodTable(vm)->getEnumerableLength(exec, base);

    JSPropertyNameEnumerator* enumerator = nullptr;

    Structure* structure = base->structure(vm);
    if (!indexedLength
        && (enumerator = structure->cachedPropertyNameEnumerator())
        && enumerator->cachedPrototypeChain() == structure->prototypeChain(exec))
        return enumerator;

    uint32_t numberStructureProperties = 0;

    PropertyNameArray propertyNames(&vm, PropertyNameMode::Strings, PrivateSymbolMode::Exclude);

    if (structure->canAccessPropertiesQuicklyForEnumeration() && indexedLength == base->getArrayLength()) {
        base->methodTable(vm)->getStructurePropertyNames(base, exec, propertyNames, EnumerationMode());
        scope.assertNoException();

        numberStructureProperties = propertyNames.size();

        base->methodTable(vm)->getGenericPropertyNames(base, exec, propertyNames, EnumerationMode());
        scope.assertNoException();
    } else {
        // Generic property names vector contains all indexed property names.
        // So disable indexed property enumeration phase by setting |indexedLength| to 0.
        indexedLength = 0;
        base->methodTable(vm)->getPropertyNames(base, exec, propertyNames, EnumerationMode());
        RETURN_IF_EXCEPTION(scope, nullptr);
    }

    ASSERT(propertyNames.size() < UINT32_MAX);

    normalizePrototypeChain(exec, structure);

    enumerator = JSPropertyNameEnumerator::create(vm, structure, indexedLength, numberStructureProperties, WTFMove(propertyNames));
    enumerator->setCachedPrototypeChain(vm, structure->prototypeChain(exec));
    if (!indexedLength && structure->canCachePropertyNameEnumerator())
        structure->setCachedPropertyNameEnumerator(vm, enumerator);
    return enumerator;
}

} // namespace JSC
