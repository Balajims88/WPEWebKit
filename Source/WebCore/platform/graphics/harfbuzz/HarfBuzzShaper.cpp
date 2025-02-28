/*
 * Copyright (c) 2012 Google Inc. All rights reserved.
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

#include "config.h"
#include "HarfBuzzShaper.h"

#include "FontCascade.h"
#include "HarfBuzzFace.h"
#include "SurrogatePairAwareTextIterator.h"
#include <hb-icu.h>
#include <unicode/normlzr.h>
#include <unicode/uchar.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/StringView.h>

namespace WebCore {

template<typename T>
class HarfBuzzScopedPtr {
public:
    typedef void (*DestroyFunction)(T*);

    HarfBuzzScopedPtr(T* ptr, DestroyFunction destroy)
        : m_ptr(ptr)
        , m_destroy(destroy)
    {
        ASSERT(m_destroy);
    }
    ~HarfBuzzScopedPtr()
    {
        if (m_ptr)
            (*m_destroy)(m_ptr);
    }

    T* get() { return m_ptr; }
private:
    T* m_ptr;
    DestroyFunction m_destroy;
};

static inline float harfBuzzPositionToFloat(hb_position_t value)
{
    return static_cast<float>(value) / (1 << 16);
}

HarfBuzzShaper::HarfBuzzRun::HarfBuzzRun(const Font* fontData, unsigned startIndex, unsigned numCharacters, TextDirection direction, hb_script_t script)
    : m_fontData(fontData)
    , m_startIndex(startIndex)
    , m_numCharacters(numCharacters)
    , m_direction(direction)
    , m_script(script)
{
}

void HarfBuzzShaper::HarfBuzzRun::applyShapeResult(hb_buffer_t* harfBuzzBuffer)
{
    m_numGlyphs = hb_buffer_get_length(harfBuzzBuffer);
    if (!m_numGlyphs) {
        // HarfBuzzShaper::fillGlyphBuffer gets offsets()[0]
        m_offsets.resize(1);
        return;
    }
    m_glyphs.resize(m_numGlyphs);
    m_advances.resize(m_numGlyphs);
    m_glyphToCharacterIndexes.resize(m_numGlyphs);
    m_offsets.resize(m_numGlyphs);
}

void HarfBuzzShaper::HarfBuzzRun::setGlyphAndPositions(unsigned index, uint16_t glyphId, float advance, float offsetX, float offsetY)
{
    m_glyphs[index] = glyphId;
    m_advances[index] = advance;
    m_offsets[index] = FloatPoint(offsetX, offsetY);
}

unsigned HarfBuzzShaper::HarfBuzzRun::characterIndexForXPosition(float targetX, bool includePartialGlyphs)
{
    ASSERT(targetX <= m_width);
    float currentX = 0;
    unsigned glyphIndex = 0;
    float characterWidth = 0;
    unsigned characterIndex = 0;
    unsigned previousCharacterIndex = m_numCharacters;

    do {
        characterIndex = m_glyphToCharacterIndexes[glyphIndex];
        characterWidth = m_advances[glyphIndex];
        while (glyphIndex < m_numGlyphs - 1 && m_glyphToCharacterIndexes[glyphIndex + 1] == characterIndex)
            characterWidth += m_advances[++glyphIndex];

        if ((includePartialGlyphs && (targetX <= currentX + characterWidth / 2.0))
            || (!includePartialGlyphs && (targetX < currentX + characterWidth)))
            return rtl() ? previousCharacterIndex : characterIndex;

        if ((includePartialGlyphs && (targetX > (currentX + characterWidth / 2.0) && targetX < currentX + characterWidth))
            || (!includePartialGlyphs && (targetX >= currentX && targetX < currentX + characterWidth)))
            break;

        currentX += characterWidth;
        ++glyphIndex;
        previousCharacterIndex = characterIndex;
    } while (glyphIndex < m_numGlyphs);

    return rtl() ? characterIndex : glyphIndex < m_numGlyphs - 1 ? m_glyphToCharacterIndexes[glyphIndex + 1] : m_numCharacters;
}

float HarfBuzzShaper::HarfBuzzRun::xPositionForOffset(unsigned offset)
{
    ASSERT(offset < m_numCharacters);
    if (!m_numGlyphs)
        return 0;

    unsigned glyphIndex = 0;
    float position = 0;
    if (rtl()) {
        while (glyphIndex < m_numGlyphs && m_glyphToCharacterIndexes[glyphIndex] > offset) {
            position += m_advances[glyphIndex];
            ++glyphIndex;
        }
        // For RTL, we need to return the right side boundary of the character.
        // Add advance of glyphs which are part of the character.
        while (glyphIndex < m_numGlyphs - 1 && m_glyphToCharacterIndexes[glyphIndex] == m_glyphToCharacterIndexes[glyphIndex + 1]) {
            position += m_advances[glyphIndex];
            ++glyphIndex;
        }
        position += m_advances[glyphIndex];
    } else {
        while (glyphIndex < m_numGlyphs && m_glyphToCharacterIndexes[glyphIndex] < offset) {
            position += m_advances[glyphIndex];
            ++glyphIndex;
        }
    }
    return position;
}

HarfBuzzShaper::HarfBuzzShaper(const FontCascade* font, const TextRun& run)
    : m_font(font)
    , m_normalizedBufferLength(0)
    , m_run(run)
    , m_wordSpacingAdjustment(font->wordSpacing())
    , m_padding(0)
    , m_padPerWordBreak(0)
    , m_padError(0)
    , m_letterSpacing(font->letterSpacing())
{
    setNormalizedBuffer();
    setPadding(m_run.expansion());
    setFontFeatures();
}

HarfBuzzShaper::~HarfBuzzShaper()
{
}

static void normalizeSpacesAndMirrorChars(const UChar* source, UChar* destination, unsigned length, HarfBuzzShaper::NormalizeMode normalizeMode)
{
    unsigned position = 0;
    bool error = false;
    // Iterate characters in source and mirror character if needed.
    while (position < length) {
        UChar32 character;
        unsigned nextPosition = position;
        U16_NEXT(source, nextPosition, length, character);
        // Don't normalize tabs as they are not treated as spaces for word-end
        if (FontCascade::treatAsSpace(character) && character != '\t')
            character = ' ';
        else if (FontCascade::treatAsZeroWidthSpaceInComplexScript(character))
            character = zeroWidthSpace;
        else if (normalizeMode == HarfBuzzShaper::NormalizeMirrorChars)
            character = u_charMirror(character);
        U16_APPEND(destination, position, length, character, error);
        ASSERT_UNUSED(error, !error);
        position = nextPosition;
    }
}

void HarfBuzzShaper::setNormalizedBuffer(NormalizeMode normalizeMode)
{
    // Normalize the text run in three ways:
    // 1) Convert the |originalRun| to NFC normalized form if combining diacritical marks
    // (U+0300..) are used in the run. This conversion is necessary since most OpenType
    // fonts (e.g., Arial) don't have substitution rules for the diacritical marks in
    // their GSUB tables.
    //
    // Note that we don't use the icu::Normalizer::isNormalized(UNORM_NFC) API here since
    // the API returns FALSE (= not normalized) for complex runs that don't require NFC
    // normalization (e.g., Arabic text). Unless the run contains the diacritical marks,
    // HarfBuzz will do the same thing for us using the GSUB table.
    // 2) Convert spacing characters into plain spaces, as some fonts will provide glyphs
    // for characters like '\n' otherwise.
    // 3) Convert mirrored characters such as parenthesis for rtl text.

    // Convert to NFC form if the text has diacritical marks.
    icu::UnicodeString normalizedString;
    UErrorCode error = U_ZERO_ERROR;

    const UChar* runCharacters;
    String stringFor8BitRun;
    if (m_run.is8Bit()) {
        stringFor8BitRun = String::make16BitFrom8BitSource(m_run.characters8(), m_run.length());
        runCharacters = stringFor8BitRun.characters16();
    } else
        runCharacters = m_run.characters16();

    for (unsigned i = 0; i < m_run.length(); ++i) {
        UChar ch = runCharacters[i];
        if (::ublock_getCode(ch) == UBLOCK_COMBINING_DIACRITICAL_MARKS) {
            icu::Normalizer::normalize(icu::UnicodeString(runCharacters,
                m_run.length()), UNORM_NFC, 0 /* no options */,
                normalizedString, error);
            if (U_FAILURE(error))
                normalizedString.remove();
            break;
        }
    }

    const UChar* sourceText;
    if (normalizedString.isEmpty()) {
        m_normalizedBufferLength = m_run.length();
        sourceText = runCharacters;
    } else {
        m_normalizedBufferLength = normalizedString.length();
        sourceText = normalizedString.getBuffer();
    }

    m_normalizedBuffer = std::make_unique<UChar[]>(m_normalizedBufferLength + 1);
    normalizeSpacesAndMirrorChars(sourceText, m_normalizedBuffer.get(), m_normalizedBufferLength, normalizeMode);
}

bool HarfBuzzShaper::isWordEnd(unsigned index)
{
    // This could refer a high-surrogate, but should work.
    return index && isCodepointSpace(m_normalizedBuffer[index]);
}

int HarfBuzzShaper::determineWordBreakSpacing()
{
    int wordBreakSpacing = m_wordSpacingAdjustment;

    if (m_padding > 0) {
        int toPad = roundf(m_padPerWordBreak + m_padError);
        m_padError += m_padPerWordBreak - toPad;

        if (m_padding < toPad)
            toPad = m_padding;
        m_padding -= toPad;
        wordBreakSpacing += toPad;
    }
    return wordBreakSpacing;
}

// setPadding sets a number of pixels to be distributed across the TextRun.
// WebKit uses this to justify text.
void HarfBuzzShaper::setPadding(int padding)
{
    m_padding = padding;
    m_padError = 0;
    if (!m_padding)
        return;

    // If we have padding to distribute, then we try to give an equal
    // amount to each space. The last space gets the smaller amount, if
    // any.
    unsigned numWordEnds = 0;

    for (unsigned i = 0; i < m_normalizedBufferLength; i++) {
        if (isWordEnd(i))
            numWordEnds++;
    }

    if (numWordEnds)
        m_padPerWordBreak = m_padding / numWordEnds;
    else
        m_padPerWordBreak = 0;
}

void HarfBuzzShaper::setFontFeatures()
{
    const auto& description = m_font->fontDescription();
    if (description.orientation() == Vertical) {
        static hb_feature_t vert = { HarfBuzzFace::vertTag, 1, 0, static_cast<unsigned>(-1) };
        static hb_feature_t vrt2 = { HarfBuzzFace::vrt2Tag, 1, 0, static_cast<unsigned>(-1) };
        m_features.append(vert);
        m_features.append(vrt2);
    }

    hb_feature_t kerning = { HarfBuzzFace::kernTag, 0, 0, static_cast<unsigned>(-1) };
    switch (description.kerning()) {
    case Kerning::Normal:
        kerning.value = 1;
        m_features.append(kerning);
        break;
    case Kerning::NoShift:
        kerning.value = 0;
        m_features.append(kerning);
        break;
    case Kerning::Auto:
        break;
    default:
        ASSERT_NOT_REACHED();
    }

    const FontFeatureSettings& settings = description.featureSettings();

    unsigned numFeatures = settings.size();
    for (unsigned i = 0; i < numFeatures; ++i) {
        hb_feature_t feature;
        auto& tag = settings[i].tag();
        feature.tag = HB_TAG(tag[0], tag[1], tag[2], tag[3]);
        feature.value = settings[i].value();
        feature.start = 0;
        feature.end = static_cast<unsigned>(-1);
        m_features.append(feature);
    }
}

bool HarfBuzzShaper::shape(GlyphBuffer* glyphBuffer, std::optional<unsigned> from, std::optional<unsigned> to)
{
    if (!collectHarfBuzzRuns())
        return false;

    m_totalWidth = 0;
    // WebKit doesn't set direction when calulating widths. Leave the direction setting to
    // HarfBuzz when we are calculating widths (except when directionalOverride() is set).
    if (!shapeHarfBuzzRuns(glyphBuffer || m_run.directionalOverride()))
        return false;
    m_totalWidth = roundf(m_totalWidth);

    if (glyphBuffer && !fillGlyphBuffer(glyphBuffer, from.value_or(0), to.value_or(m_run.length())))
        return false;

    return true;
}

FloatPoint HarfBuzzShaper::adjustStartPoint(const FloatPoint& point)
{
    return point + m_startOffset;
}

static bool scriptsAreCompatibleForCharacters(UScriptCode script, UScriptCode previousScript, UChar32 character, UChar32 previousCharacter)
{
    if (script == previousScript)
        return true;

    if (script == USCRIPT_INHERITED || previousScript == USCRIPT_COMMON)
        return true;

    if (script == USCRIPT_COMMON) {
        // §5.1 Handling Characters with the Common Script Property.
        // Programs must resolve any of the special Script property values, such as Common,
        // based on the context of the surrounding characters. A simple heuristic uses the
        // script of the preceding character, which works well in many cases.
        // http://www.unicode.org/reports/tr24/#Common.
        //
        // FIXME: cover all other cases mentioned in the spec (ie. brackets or quotation marks).
        // https://bugs.webkit.org/show_bug.cgi?id=177003.
        //
        // We use a slightly more conservative heuristic than the one proposed in the spec,
        // using the script of the previous character only if both are ASCII.
        if (isASCII(character) && isASCII(previousCharacter))
            return true;
    }

    return uscript_hasScript(character, previousScript);
}

bool HarfBuzzShaper::collectHarfBuzzRuns()
{
    const UChar* normalizedBufferEnd = m_normalizedBuffer.get() + m_normalizedBufferLength;
    SurrogatePairAwareTextIterator iterator(m_normalizedBuffer.get(), 0, m_normalizedBufferLength, m_normalizedBufferLength);
    UChar32 character;
    unsigned clusterLength = 0;
    unsigned startIndexOfCurrentRun = 0;
    if (!iterator.consume(character, clusterLength))
        return false;

    const Font* nextFontData = m_font->glyphDataForCharacter(character, false).font;
    UErrorCode errorCode = U_ZERO_ERROR;
    UScriptCode nextScript = uscript_getScript(character, &errorCode);
    if (U_FAILURE(errorCode))
        return false;

    do {
        const UChar* currentCharacterPosition = iterator.characters();
        const Font* currentFontData = nextFontData;
        if (!currentFontData)
            currentFontData = &m_font->primaryFont();
        UScriptCode currentScript = nextScript;
        UChar32 previousCharacter = character;

        for (iterator.advance(clusterLength); iterator.consume(character, clusterLength); iterator.advance(clusterLength)) {
            if (FontCascade::treatAsZeroWidthSpace(character))
                continue;

            if (U_GET_GC_MASK(character) & U_GC_M_MASK) {
                unsigned markLength = clusterLength;
                const UChar* markCharactersEnd = iterator.characters() + clusterLength;
                while (markCharactersEnd < normalizedBufferEnd) {
                    UChar32 nextCharacter;
                    unsigned nextCharacterLength = 0;
                    U16_NEXT(markCharactersEnd, nextCharacterLength, normalizedBufferEnd - markCharactersEnd, nextCharacter);
                    if (!(U_GET_GC_MASK(nextCharacter) & U_GC_M_MASK))
                        break;
                    markLength += nextCharacterLength;
                    markCharactersEnd += nextCharacterLength;
                }

                if (currentFontData->canRenderCombiningCharacterSequence(currentCharacterPosition, markCharactersEnd - currentCharacterPosition)) {
                    clusterLength = markLength;
                    continue;
                }
                nextFontData = m_font->glyphDataForCharacter(character, false).font;
            } else
                nextFontData = m_font->glyphDataForCharacter(character, false).font;

            nextScript = uscript_getScript(character, &errorCode);
            if (U_FAILURE(errorCode))
                return false;

            if (nextFontData != currentFontData)
                break;

            if (!scriptsAreCompatibleForCharacters(nextScript, currentScript, character, previousCharacter))
                break;

            if (nextScript == USCRIPT_INHERITED || nextScript == USCRIPT_COMMON)
                nextScript = currentScript;

            currentCharacterPosition = iterator.characters();
            previousCharacter = character;
        }
        unsigned numCharactersOfCurrentRun = iterator.currentIndex() - startIndexOfCurrentRun;
        hb_script_t script = hb_icu_script_to_script(currentScript);
        m_harfBuzzRuns.append(std::make_unique<HarfBuzzRun>(currentFontData, startIndexOfCurrentRun, numCharactersOfCurrentRun, m_run.direction(), script));
        currentFontData = nextFontData;
        startIndexOfCurrentRun = iterator.currentIndex();
    } while (iterator.consume(character, clusterLength));

    return !m_harfBuzzRuns.isEmpty();
}

bool HarfBuzzShaper::shapeHarfBuzzRuns(bool shouldSetDirection)
{
    HarfBuzzScopedPtr<hb_buffer_t> harfBuzzBuffer(hb_buffer_create(), hb_buffer_destroy);

    hb_buffer_set_unicode_funcs(harfBuzzBuffer.get(), hb_icu_get_unicode_funcs());

    for (unsigned i = 0; i < m_harfBuzzRuns.size(); ++i) {
        unsigned runIndex = m_run.rtl() ? m_harfBuzzRuns.size() - i - 1 : i;
        HarfBuzzRun* currentRun = m_harfBuzzRuns[runIndex].get();
        const Font* currentFontData = currentRun->fontData();

        hb_buffer_set_script(harfBuzzBuffer.get(), currentRun->script());
        if (shouldSetDirection)
            hb_buffer_set_direction(harfBuzzBuffer.get(), currentRun->rtl() ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
        else
            // Leaving direction to HarfBuzz to guess is *really* bad, but will do for now.
            hb_buffer_guess_segment_properties(harfBuzzBuffer.get());

        // Add a space as pre-context to the buffer. This prevents showing dotted-circle
        // for combining marks at the beginning of runs.
        static const uint16_t preContext = ' ';
        hb_buffer_add_utf16(harfBuzzBuffer.get(), &preContext, 1, 1, 0);

        if (m_font->isSmallCaps() && u_islower(m_normalizedBuffer[currentRun->startIndex()])) {
            String upperText = String(m_normalizedBuffer.get() + currentRun->startIndex(), currentRun->numCharacters()).convertToUppercaseWithoutLocale();
            currentFontData = m_font->glyphDataForCharacter(upperText[0], false, SmallCapsVariant).font;
            const UChar* characters = StringView(upperText).upconvertedCharacters();
            hb_buffer_add_utf16(harfBuzzBuffer.get(), reinterpret_cast<const uint16_t*>(characters), currentRun->numCharacters(), 0, currentRun->numCharacters());
        } else
            hb_buffer_add_utf16(harfBuzzBuffer.get(), reinterpret_cast<const uint16_t*>(m_normalizedBuffer.get() + currentRun->startIndex()), currentRun->numCharacters(), 0, currentRun->numCharacters());

        FontPlatformData* platformData = const_cast<FontPlatformData*>(&currentFontData->platformData());
        HarfBuzzFace* face = platformData->harfBuzzFace();
        if (!face)
            return false;

        if (m_font->fontDescription().orientation() == Vertical)
            face->setScriptForVerticalGlyphSubstitution(harfBuzzBuffer.get());

        HarfBuzzScopedPtr<hb_font_t> harfBuzzFont(face->createFont(), hb_font_destroy);

        hb_shape(harfBuzzFont.get(), harfBuzzBuffer.get(), m_features.isEmpty() ? 0 : m_features.data(), m_features.size());

        currentRun->applyShapeResult(harfBuzzBuffer.get());
        setGlyphPositionsForHarfBuzzRun(currentRun, harfBuzzBuffer.get());

        hb_buffer_reset(harfBuzzBuffer.get());
    }

    return true;
}

void HarfBuzzShaper::setGlyphPositionsForHarfBuzzRun(HarfBuzzRun* currentRun, hb_buffer_t* harfBuzzBuffer)
{
    const Font* currentFontData = currentRun->fontData();
    hb_glyph_info_t* glyphInfos = hb_buffer_get_glyph_infos(harfBuzzBuffer, 0);
    hb_glyph_position_t* glyphPositions = hb_buffer_get_glyph_positions(harfBuzzBuffer, 0);

    unsigned numGlyphs = currentRun->numGlyphs();
    uint16_t* glyphToCharacterIndexes = currentRun->glyphToCharacterIndexes();
    float totalAdvance = 0;

    // HarfBuzz returns the shaping result in visual order. We need not to flip for RTL.
    for (size_t i = 0; i < numGlyphs; ++i) {
        bool runEnd = i + 1 == numGlyphs;
        uint16_t glyph = glyphInfos[i].codepoint;
        float offsetX = harfBuzzPositionToFloat(glyphPositions[i].x_offset);
        float offsetY = -harfBuzzPositionToFloat(glyphPositions[i].y_offset);
        float advance = harfBuzzPositionToFloat(glyphPositions[i].x_advance);

        unsigned currentCharacterIndex = currentRun->startIndex() + glyphInfos[i].cluster;
        bool isClusterEnd = runEnd || glyphInfos[i].cluster != glyphInfos[i + 1].cluster;
        float spacing = 0;

        glyphToCharacterIndexes[i] = glyphInfos[i].cluster;

        if (isClusterEnd && !FontCascade::treatAsZeroWidthSpace(m_normalizedBuffer[currentCharacterIndex]))
            spacing += m_letterSpacing;

        if (isClusterEnd && isWordEnd(currentCharacterIndex))
            spacing += determineWordBreakSpacing();

        if (currentFontData->isZeroWidthSpaceGlyph(glyph)) {
            currentRun->setGlyphAndPositions(i, glyph, 0, 0, 0);
            continue;
        }

        advance += spacing;
        if (m_run.rtl()) {
            // In RTL, spacing should be added to left side of glyphs.
            offsetX += spacing;
            if (!isClusterEnd)
                offsetX += m_letterSpacing;
        }

        currentRun->setGlyphAndPositions(i, glyph, advance, offsetX, offsetY);

        totalAdvance += advance;
    }
    currentRun->setWidth(totalAdvance > 0.0 ? totalAdvance : 0.0);
    m_totalWidth += currentRun->width();
}

void HarfBuzzShaper::fillGlyphBufferFromHarfBuzzRun(GlyphBuffer* glyphBuffer, unsigned from, unsigned to, HarfBuzzRun* currentRun, const FloatPoint& firstOffsetOfNextRun)
{
    FloatPoint* offsets = currentRun->offsets();
    uint16_t* glyphs = currentRun->glyphs();
    float* advances = currentRun->advances();
    unsigned numGlyphs = currentRun->numGlyphs();
    uint16_t* glyphToCharacterIndexes = currentRun->glyphToCharacterIndexes();

    for (unsigned i = 0; i < numGlyphs; ++i) {
        uint16_t currentCharacterIndex = currentRun->startIndex() + glyphToCharacterIndexes[i];
        if (currentCharacterIndex < from) {
            if (m_run.rtl())
                break;
            continue;
        }
        if (currentCharacterIndex >= to) {
            if (m_run.rtl())
                continue;
            break;
        }
        const FloatPoint& currentOffset = offsets[i];
        const FloatPoint& nextOffset = (i == numGlyphs - 1) ? firstOffsetOfNextRun : offsets[i + 1];
        float glyphAdvanceX = advances[i] + nextOffset.x() - currentOffset.x();
        float glyphAdvanceY = nextOffset.y() - currentOffset.y();
        if (m_run.rtl()) {
            if (currentCharacterIndex > m_run.length())
                m_startOffset.move(glyphAdvanceX, glyphAdvanceY);
            else
                glyphBuffer->add(glyphs[i], currentRun->fontData(), createGlyphBufferAdvance(glyphAdvanceX, glyphAdvanceY), currentCharacterIndex);
        } else {
            if (currentCharacterIndex < m_run.length())
                glyphBuffer->add(glyphs[i], currentRun->fontData(), createGlyphBufferAdvance(glyphAdvanceX, glyphAdvanceY), currentCharacterIndex);
        }
    }
}

bool HarfBuzzShaper::fillGlyphBuffer(GlyphBuffer* glyphBuffer, unsigned from, unsigned to)
{
    unsigned numRuns = m_harfBuzzRuns.size();
    if (m_run.rtl()) {
        m_startOffset = m_harfBuzzRuns.last()->offsets()[0];
        for (int runIndex = numRuns - 1; runIndex >= 0; --runIndex) {
            HarfBuzzRun* currentRun = m_harfBuzzRuns[runIndex].get();
            auto runStartIndex = currentRun->startIndex();
            auto runEndIndex = std::max<unsigned>(0, runStartIndex + currentRun->numCharacters() - 1);
            if ((from >= runStartIndex && from <= runEndIndex) || (to >= runStartIndex && to <= runEndIndex)
                || (from < runEndIndex && to > runStartIndex)) {
                FloatPoint firstOffsetOfNextRun = !runIndex ? FloatPoint() : m_harfBuzzRuns[runIndex - 1]->offsets()[0];
                fillGlyphBufferFromHarfBuzzRun(glyphBuffer, from, to, currentRun, firstOffsetOfNextRun);
            }
        }
    } else {
        m_startOffset = m_harfBuzzRuns.first()->offsets()[0];
        for (unsigned runIndex = 0; runIndex < numRuns; ++runIndex) {
            HarfBuzzRun* currentRun = m_harfBuzzRuns[runIndex].get();
            auto runStartIndex = currentRun->startIndex();
            auto runEndIndex = std::max<unsigned>(0, runStartIndex + currentRun->numCharacters() - 1);
            if ((from >= runStartIndex && from <= runEndIndex) || (to >= runStartIndex && to <= runEndIndex)
                || (from < runStartIndex && to > runEndIndex)) {
                FloatPoint firstOffsetOfNextRun = runIndex == numRuns - 1 ? FloatPoint() : m_harfBuzzRuns[runIndex + 1]->offsets()[0];
                fillGlyphBufferFromHarfBuzzRun(glyphBuffer, from, to, currentRun, firstOffsetOfNextRun);
            }
        }
    }
    return glyphBuffer->size();
}

int HarfBuzzShaper::offsetForPosition(float targetX, bool includePartialGlyphs)
{
    int charactersSoFar = 0;
    float currentX = 0;

    if (m_run.rtl()) {
        charactersSoFar = m_normalizedBufferLength;
        for (int i = m_harfBuzzRuns.size() - 1; i >= 0; --i) {
            charactersSoFar -= m_harfBuzzRuns[i]->numCharacters();
            float nextX = currentX + m_harfBuzzRuns[i]->width();
            float offsetForRun = targetX - currentX;
            if (offsetForRun >= 0 && offsetForRun <= m_harfBuzzRuns[i]->width()) {
                // The x value in question is within this script run.
                const unsigned index = m_harfBuzzRuns[i]->characterIndexForXPosition(offsetForRun, includePartialGlyphs);
                return charactersSoFar + index;
            }
            currentX = nextX;
        }
    } else {
        for (unsigned i = 0; i < m_harfBuzzRuns.size(); ++i) {
            float nextX = currentX + m_harfBuzzRuns[i]->width();
            float offsetForRun = targetX - currentX;
            if (offsetForRun >= 0 && offsetForRun <= m_harfBuzzRuns[i]->width()) {
                const unsigned index = m_harfBuzzRuns[i]->characterIndexForXPosition(offsetForRun, includePartialGlyphs);
                return charactersSoFar + index;
            }
            charactersSoFar += m_harfBuzzRuns[i]->numCharacters();
            currentX = nextX;
        }
    }

    return charactersSoFar;
}

FloatRect HarfBuzzShaper::selectionRect(const FloatPoint& point, int height, unsigned from, unsigned to)
{
    float currentX = 0;
    float fromX = 0;
    float toX = 0;
    bool foundFromX = false;
    bool foundToX = false;

    std::optional<unsigned> fromIndex = from;
    std::optional<unsigned> toIndex = to;

    if (m_run.rtl())
        currentX = m_totalWidth;
    for (unsigned i = 0; i < m_harfBuzzRuns.size(); ++i) {
        if (m_run.rtl())
            currentX -= m_harfBuzzRuns[i]->width();
        unsigned numCharacters = m_harfBuzzRuns[i]->numCharacters();
        if (!foundFromX && fromIndex.value() < numCharacters) {
            fromX = m_harfBuzzRuns[i]->xPositionForOffset(fromIndex.value()) + currentX;
            foundFromX = true;
        } else {
            if (fromIndex && fromIndex.value() >= numCharacters)
                fromIndex.value() -= numCharacters;
            else
                fromIndex = std::nullopt;
        }

        if (!foundToX && toIndex.value() < numCharacters) {
            toX = m_harfBuzzRuns[i]->xPositionForOffset(toIndex.value()) + currentX;
            foundToX = true;
        } else {
            if (toIndex && toIndex.value() >= numCharacters)
                toIndex.value() -= numCharacters;
            else
                toIndex = std::nullopt;
        }

        if (foundFromX && foundToX)
            break;
        if (!m_run.rtl())
            currentX += m_harfBuzzRuns[i]->width();
    }

    // The position in question might be just after the text.
    if (!foundFromX)
        fromX = 0;
    if (!foundToX)
        toX = m_run.rtl() ? 0 : m_totalWidth;

    // Using floorf() and roundf() as the same as mac port.
    if (fromX < toX)
        return FloatRect(floorf(point.x() + fromX), point.y(), roundf(toX - fromX), height);
    return FloatRect(floorf(point.x() + toX), point.y(), roundf(fromX - toX), height);
}

} // namespace WebCore
