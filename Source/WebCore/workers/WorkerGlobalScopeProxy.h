/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "MessagePortChannel.h"
#include <runtime/RuntimeFlags.h>
#include <wtf/MonotonicTime.h>

namespace PAL {
class SessionID;
}

namespace WebCore {

class ContentSecurityPolicyResponseHeaders;
class URL;
class Worker;

// A proxy to talk to the worker context.
class WorkerGlobalScopeProxy {
public:
    static WorkerGlobalScopeProxy& create(Worker&);

    virtual void startWorkerGlobalScope(const URL& scriptURL, const String& userAgent, const String& sourceCode, const ContentSecurityPolicyResponseHeaders&, bool shouldBypassMainWorldContentSecurityPolicy, MonotonicTime timeOrigin, JSC::RuntimeFlags, PAL::SessionID) = 0;
    virtual void terminateWorkerGlobalScope() = 0;
    virtual void postMessageToWorkerGlobalScope(RefPtr<SerializedScriptValue>&&, std::unique_ptr<MessagePortChannelArray>) = 0;
    virtual bool hasPendingActivity() const = 0;
    virtual void workerObjectDestroyed() = 0;
    virtual void notifyNetworkStateChange(bool isOnline) = 0;

protected:
    virtual ~WorkerGlobalScopeProxy() { }
};

} // namespace WebCore
