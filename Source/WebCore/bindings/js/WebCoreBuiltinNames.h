/*
 *  Copyright (c) 2015, Canon Inc. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1.  Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *  2.  Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *  3.  Neither the name of Canon Inc. nor the names of
 *      its contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY CANON INC. AND ITS CONTRIBUTORS "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL CANON INC. AND ITS CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <builtins/BuiltinUtils.h>

namespace WebCore {

#define WEBCORE_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(macro) \
    macro(Animation) \
    macro(AnimationEffectReadOnly) \
    macro(AnimationTimeline) \
    macro(ApplePaySession) \
    macro(AttachmentElement) \
    macro(Audio) \
    macro(Cache) \
    macro(CacheStorage) \
    macro(Credential) \
    macro(CredentialsContainer) \
    macro(CustomElementRegistry) \
    macro(DataTransferItem) \
    macro(DataTransferItemList) \
    macro(DocumentTimeline) \
    macro(FederatedCredential) \
    macro(FileSystem) \
    macro(FileSystemDirectoryEntry) \
    macro(FileSystemDirectoryReader) \
    macro(FileSystemEntry) \
    macro(FileSystemFileEntry) \
    macro(Gamepad) \
    macro(GamepadButton) \
    macro(GamepadEvent) \
    macro(HTMLAttachmentElement) \
    macro(HTMLAudioElement) \
    macro(HTMLSlotElement) \
    macro(Headers) \
    macro(IDBCursor) \
    macro(IDBCursorWithValue) \
    macro(IDBDatabase) \
    macro(IDBFactory) \
    macro(IDBIndex) \
    macro(IDBKeyRange) \
    macro(IDBObjectStore) \
    macro(IDBOpenDBRequest) \
    macro(IDBRequest) \
    macro(IDBTransaction) \
    macro(IDBVersionChangeEvent) \
    macro(InputEvent) \
    macro(IntersectionObserver) \
    macro(IntersectionObserverEntry) \
    macro(KeyframeEffect) \
    macro(MediaEncryptedEvent) \
    macro(MediaKeyMessageEvent) \
    macro(MediaKeySession) \
    macro(MediaKeyStatusMap) \
    macro(MediaKeySystemAccess) \
    macro(MediaKeys) \
    macro(MediaSource) \
    macro(MediaStream) \
    macro(MediaStreamEvent) \
    macro(MediaStreamTrack) \
    macro(ModernMediaControls) \
    macro(NavigatorCredentials) \
    macro(NavigatorMediaDevices) \
    macro(NavigatorUserMedia) \
    macro(PasswordCredential) \
    macro(PaymentAddress) \
    macro(PaymentRequest) \
    macro(PaymentRequestUpdateEvent) \
    macro(PaymentResponse) \
    macro(PerformanceEntry) \
    macro(PerformanceEntryList) \
    macro(PerformanceMark) \
    macro(PerformanceMeasure) \
    macro(PerformanceObserver) \
    macro(PerformanceObserverEntryList) \
    macro(PerformanceResourceTiming) \
    macro(RTCDTMFSender) \
    macro(RTCDTMFToneChangeEvent) \
    macro(RTCDataChannel) \
    macro(RTCDataChannelEvent) \
    macro(RTCIceCandidate) \
    macro(RTCIceTransport) \
    macro(RTCPeerConnection) \
    macro(RTCRtpReceiver) \
    macro(RTCRtpSender) \
    macro(RTCRtpTransceiver) \
    macro(RTCSessionDescription) \
    macro(RTCStatsReport) \
    macro(RTCTrackEvent) \
    macro(ReadableByteStreamController) \
    macro(ReadableStream) \
    macro(ReadableStreamBYOBReader) \
    macro(ReadableStreamBYOBRequest) \
    macro(ReadableStreamDefaultController) \
    macro(ReadableStreamDefaultReader) \
    macro(Request) \
    macro(Response) \
    macro(ServiceWorker) \
    macro(ServiceWorkerContainer) \
    macro(ServiceWorkerGlobalScope) \
    macro(ServiceWorkerRegistration) \
    macro(ShadowRoot) \
    macro(StaticRange) \
    macro(VRDisplay) \
    macro(VRDisplayCapabilities) \
    macro(VRDisplayEvent) \
    macro(VREyeParameters) \
    macro(VRFieldOfView) \
    macro(VRFrameData) \
    macro(VRStageParameters) \
    macro(WebGL2RenderingContext) \
    macro(WebGLVertexArrayObject) \
    macro(WebGPUBuffer) \
    macro(WebGPUCommandBuffer) \
    macro(WebGPUCommandQueue) \
    macro(WebGPUComputeCommandEncoder) \
    macro(WebGPUComputePipelineState) \
    macro(WebGPUDepthStencilDescriptor) \
    macro(WebGPUDepthStencilState) \
    macro(WebGPUDrawable) \
    macro(WebGPUFunction) \
    macro(WebGPULibrary) \
    macro(WebGPURenderCommandEncoder) \
    macro(WebGPURenderPassAttachmentDescriptor) \
    macro(WebGPURenderPassColorAttachmentDescriptor) \
    macro(WebGPURenderPassDepthAttachmentDescriptor) \
    macro(WebGPURenderPassDescriptor) \
    macro(WebGPURenderPipelineColorAttachmentDescriptor) \
    macro(WebGPURenderPipelineDescriptor) \
    macro(WebGPURenderPipelineState) \
    macro(WebGPURenderingContext) \
    macro(WebGPUSize) \
    macro(WebGPUTexture) \
    macro(WebGPUTextureDescriptor) \
    macro(WebKitMediaKeyError) \
    macro(WebKitMediaKeyMessageEvent) \
    macro(WebKitMediaKeyNeededEvent) \
    macro(WebKitMediaKeySession) \
    macro(WebKitMediaKeys) \
    macro(WebSocket) \
    macro(WritableStream) \
    macro(XMLHttpRequest) \
    macro(addTrack) \
    macro(appendFromJS) \
    macro(associatedReadableByteStreamController) \
    macro(autoAllocateChunkSize) \
    macro(backingMap) \
    macro(blur) \
    macro(body) \
    macro(byobRequest) \
    macro(caches) \
    macro(cancel) \
    macro(cloneArrayBuffer) \
    macro(close) \
    macro(closeRequested) \
    macro(closed) \
    macro(closedPromiseCapability) \
    macro(collectMatchingElementsInFlatTree) \
    macro(consume) \
    macro(consumeChunk) \
    macro(controlledReadableStream) \
    macro(controller) \
    macro(createReadableStream) \
    macro(customElements) \
    macro(disturbed) \
    macro(document) \
    macro(failureKind) \
    macro(fetch) \
    macro(fetchRequest) \
    macro(fillFromJS) \
    macro(finishConsumingStream) \
    macro(focus) \
    macro(frames) \
    macro(getRemoteStreams) \
    macro(getSenders) \
    macro(getTracks) \
    macro(getUserMedia) \
    macro(header) \
    macro(indexedDB) \
    macro(initializeWith) \
    macro(isDisturbed) \
    macro(isLoading) \
    macro(isSecureContext) \
    macro(localStreams) \
    macro(location) \
    macro(makeGetterTypeError) \
    macro(makeThisTypeError) \
    macro(matchingElementInFlatTree) \
    macro(mediaStreamTrackConstraints) \
    macro(onvrdisplayactivate) \
    macro(onvrdisplayblur) \
    macro(onvrdisplayconnect) \
    macro(onvrdisplaydeactivate) \
    macro(onvrdisplaydisconnect) \
    macro(onvrdisplayfocus) \
    macro(onvrdisplaypresentchange) \
    macro(opener) \
    macro(operations) \
    macro(ownerReadableStream) \
    macro(parent) \
    macro(pendingPullIntos) \
    macro(postMessage) \
    macro(privateGetStats) \
    macro(pull) \
    macro(pullAgain) \
    macro(pulling) \
    macro(queue) \
    macro(queuedAddIceCandidate) \
    macro(queuedCreateAnswer) \
    macro(queuedCreateOffer) \
    macro(queuedSetLocalDescription) \
    macro(queuedSetRemoteDescription) \
    macro(readIntoRequests) \
    macro(readRequests) \
    macro(readableByteStreamAPIEnabled) \
    macro(readableStreamController) \
    macro(reader) \
    macro(readyPromiseCapability) \
    macro(removeTrack) \
    macro(response) \
    macro(responseCacheIsValid) \
    macro(retrieveResponse) \
    macro(self) \
    macro(setBody) \
    macro(setBodyFromInputRequest) \
    macro(setStatus) \
    macro(showModalDialog) \
    macro(startConsumingStream) \
    macro(started) \
    macro(startedPromise) \
    macro(state) \
    macro(storedError) \
    macro(strategy) \
    macro(strategyHWM) \
    macro(streamClosed) \
    macro(streamClosing) \
    macro(streamErrored) \
    macro(streamReadable) \
    macro(streamWaiting) \
    macro(streamWritable) \
    macro(structuredCloneArrayBuffer) \
    macro(structuredCloneArrayBufferView) \
    macro(top) \
    macro(totalQueuedBytes) \
    macro(underlyingByteSource) \
    macro(underlyingSink) \
    macro(underlyingSource) \
    macro(view) \
    macro(webRTCLegacyAPIEnabled) \
    macro(webkit) \
    macro(webkitAudioContext) \
    macro(webkitIDBCursor) \
    macro(webkitIDBDatabase) \
    macro(webkitIDBFactory) \
    macro(webkitIDBIndex) \
    macro(webkitIDBKeyRange) \
    macro(webkitIDBObjectStore) \
    macro(webkitIDBRequest) \
    macro(webkitIDBTransaction) \
    macro(webkitIndexedDB) \
    macro(window) \
    macro(writing)

class WebCoreBuiltinNames {
public:
    explicit WebCoreBuiltinNames(JSC::VM* vm)
        : m_vm(*vm)
        WEBCORE_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(INITIALIZE_BUILTIN_NAMES)
    {
#define EXPORT_NAME(name) m_vm.propertyNames->appendExternalName(name##PublicName(), name##PrivateName());
        WEBCORE_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(EXPORT_NAME)
#undef EXPORT_NAME
    }

    WEBCORE_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(DECLARE_BUILTIN_IDENTIFIER_ACCESSOR)

private:
    JSC::VM& m_vm;
    WEBCORE_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(DECLARE_BUILTIN_NAMES)
};

} // namespace WebCore
