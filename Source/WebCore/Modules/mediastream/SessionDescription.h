/*
 * Copyright (C) 2015 Ericsson AB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Ericsson nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#ifndef SessionDescription_h
#define SessionDescription_h

#if ENABLE(MEDIA_STREAM)

#include <wtf/RefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class MediaEndpointConfiguration;

class SessionDescription : public RefCounted<SessionDescription> {
public:
    enum class Type {
        Offer = 1,
        Pranswer = 2,
        Answer = 3,
        Rollback = 4
    };

    static RefPtr<SessionDescription> create(Type type, RefPtr<MediaEndpointConfiguration>&& configuration)
    {
        return adoptRef(new SessionDescription(type, WTFMove(configuration)));
    }
    virtual ~SessionDescription() { }

    Type type() const { return m_type; }
    MediaEndpointConfiguration* configuration() const { return m_configuration.get(); }

    bool isLaterThan(SessionDescription* other) { return !other || configuration()->sessionVersion() > other->configuration()->sessionVersion(); }

private:
    SessionDescription(Type type, RefPtr<MediaEndpointConfiguration>&& configuration)
        : m_type(type)
        , m_configuration(configuration)
    { }

    Type m_type;
    RefPtr<MediaEndpointConfiguration> m_configuration;
};

} // namespace WebCore

#endif // ENABLE(MEDIA_STREAM)

#endif // SessionDescription_h
