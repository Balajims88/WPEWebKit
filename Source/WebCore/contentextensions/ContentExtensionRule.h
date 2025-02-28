/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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

#if ENABLE(CONTENT_EXTENSIONS)

#include "ContentExtensionActions.h"
#include "ResourceLoadInfo.h"
#include <wtf/text/WTFString.h>

namespace WebCore {

namespace ContentExtensions {

// A ContentExtensionRule is the smallest unit in a ContentExtension.
//
// It is composed of a trigger and an action. The trigger defines on what kind of content this extension should apply.
// The action defines what to perform on that content.

struct Trigger {
    String urlFilter;
    bool urlFilterIsCaseSensitive { false };
    bool topURLConditionIsCaseSensitive { false };
    ResourceFlags flags { 0 };
    Vector<String> conditions;
    enum class ConditionType {
        None,
        IfDomain,
        UnlessDomain,
        IfTopURL,
        UnlessTopURL,
    };
    ConditionType conditionType { ConditionType::None };

    ~Trigger()
    {
        ASSERT(conditions.isEmpty() == (conditionType == ConditionType::None));
        if (topURLConditionIsCaseSensitive)
            ASSERT(conditionType == ConditionType::IfTopURL || conditionType == ConditionType::UnlessTopURL);
    }

    bool isEmpty() const
    {
        return urlFilter.isEmpty()
            && !urlFilterIsCaseSensitive
            && !flags
            && conditions.isEmpty()
            && conditionType == ConditionType::None;
    }

    bool operator==(const Trigger& other) const
    {
        return urlFilter == other.urlFilter
            && urlFilterIsCaseSensitive == other.urlFilterIsCaseSensitive
            && flags == other.flags
            && conditions == other.conditions
            && conditionType == other.conditionType;
    }
};

struct TriggerHash {
    static unsigned hash(const Trigger& trigger)
    {
        unsigned hash = trigger.urlFilterIsCaseSensitive ? 10619863 : 40960001;
        if (!trigger.urlFilter.isNull())
            hash ^= StringHash::hash(trigger.urlFilter);
        hash = WTF::pairIntHash(hash, DefaultHash<ResourceFlags>::Hash::hash(trigger.flags));

        for (const String& condition : trigger.conditions)
            hash ^= StringHash::hash(condition);

        hash ^= 1 << static_cast<unsigned>(trigger.conditionType);
        return hash;
    }

    static bool equal(const Trigger& a, const Trigger& b)
    {
        return a == b;
    }

    static const bool safeToCompareToEmptyOrDeleted = false;
};

struct TriggerHashTraits : public WTF::CustomHashTraits<Trigger> {
    static const bool emptyValueIsZero = false;
    static const bool hasIsEmptyValueFunction = true;

    static void constructDeletedValue(Trigger& trigger)
    {
        new (NotNull, std::addressof(trigger.urlFilter)) String(WTF::HashTableDeletedValue);
    }

    static bool isDeletedValue(const Trigger& trigger)
    {
        return trigger.urlFilter.isHashTableDeletedValue();
    }

    static Trigger emptyValue()
    {
        return Trigger();
    }

    static bool isEmptyValue(const Trigger& trigger)
    {
        return trigger.isEmpty();
    }
};

struct Action {
    Action(ActionType type, const String& stringArgument, uint32_t actionID = std::numeric_limits<uint32_t>::max())
        : m_type(type)
        , m_actionID(actionID)
        , m_stringArgument(stringArgument)
    {
        ASSERT(type == ActionType::CSSDisplayNoneSelector);
    }

    Action(ActionType type, uint32_t actionID = std::numeric_limits<uint32_t>::max())
        : m_type(type)
        , m_actionID(actionID)
    {
        ASSERT(type != ActionType::CSSDisplayNoneSelector);
    }

    bool operator==(const Action& other) const
    {
        return m_type == other.m_type
            && m_extensionIdentifier == other.m_extensionIdentifier
            && m_actionID == other.m_actionID
            && m_stringArgument == other.m_stringArgument;
    }

    static Action deserialize(const SerializedActionByte* actions, const uint32_t actionsLength, uint32_t location);
    static ActionType deserializeType(const SerializedActionByte* actions, const uint32_t actionsLength, uint32_t location);
    static uint32_t serializedLength(const SerializedActionByte* actions, const uint32_t actionsLength, uint32_t location);

    void setExtensionIdentifier(const String& extensionIdentifier) { m_extensionIdentifier = extensionIdentifier; }
    const String& extensionIdentifier() const { return m_extensionIdentifier; }
    ActionType type() const { return m_type; }
    uint32_t actionID() const { return m_actionID; }
    const String& stringArgument() const { return m_stringArgument; }

private:
    String m_extensionIdentifier;
    ActionType m_type;
    uint32_t m_actionID;
    String m_stringArgument;
};
    
class ContentExtensionRule {
public:
    ContentExtensionRule(Trigger&&, Action&&);

    const Trigger& trigger() const { return m_trigger; }
    const Action& action() const { return m_action; }

private:
    Trigger m_trigger;
    Action m_action;
};

} // namespace ContentExtensions
} // namespace WebCore

#endif // ENABLE(CONTENT_EXTENSIONS)
