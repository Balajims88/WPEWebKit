/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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

#include "config.h"
#include "JSImageData.h"

#include "JSDOMConvertBufferSource.h"
#include "JSDOMWrapperCache.h"
#include <heap/HeapInlines.h>
#include <runtime/IdentifierInlines.h>
#include <runtime/JSObjectInlines.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/WTFString.h>

using namespace JSC;

namespace WebCore {

JSValue toJSNewlyCreated(ExecState* state, JSDOMGlobalObject* globalObject, Ref<ImageData>&& imageData)
{
    VM& vm = state->vm();
    auto* data = imageData->data();
    auto* wrapper = createWrapper<ImageData>(globalObject, WTFMove(imageData));
    Identifier dataName = Identifier::fromString(state, "data");
    wrapper->putDirect(vm, dataName, toJS(state, globalObject, data), PropertyAttribute::DontDelete | PropertyAttribute::ReadOnly);
    // FIXME: Adopt reportExtraMemoryVisited, and switch to reportExtraMemoryAllocated.
    // https://bugs.webkit.org/show_bug.cgi?id=142595
    vm.heap.deprecatedReportExtraMemory(data->length());
    
    return wrapper;
}

JSValue toJS(ExecState* state, JSDOMGlobalObject* globalObject, ImageData& imageData)
{
    return wrap(state, globalObject, imageData);
}

}
