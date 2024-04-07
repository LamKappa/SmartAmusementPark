/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_CLIENT_SERVICE_H
#define HKS_CLIENT_SERVICE_H

#include "hks_type.h"
#include "hks_param.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t HksServiceInitialize(void);

int32_t HksServiceProvision(const struct HksBlob *keybox, const struct HksBlob *challengeIn);

int32_t HksServiceProvisionVerify(const struct HksBlob *srcData, const struct HksBlob *challengeIn);

int32_t HksServiceGenerateKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, struct HksBlob *keyOut);

int32_t HksServiceProcessInit(uint32_t msgId, const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, uint64_t *operationHandle);

int32_t HksServiceProcessUpdate(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData);

int32_t HksServiceProcessFinal(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData);

int32_t HksServiceSign(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *signature);

int32_t HksServiceVerify(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, const struct HksBlob *signature);

int32_t HksServiceEncrypt(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *plainText, struct HksBlob *cipherText);

int32_t HksServiceDecrypt(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *cipherText, struct HksBlob *plainText);

int32_t HksServiceDeleteKey(const struct HksBlob *processName, const struct HksBlob *keyAlias);

int32_t HksServiceKeyExist(const struct HksBlob *processName, const struct HksBlob *keyAlias);

int32_t HksServiceSignWithDeviceKey(const struct HksBlob *processName, uint32_t keyId,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *signature);

int32_t HksServiceGetKeyParamSet(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksParamSet *paramSet);

int32_t HksServiceGenerateRandom(const struct HksBlob *processName, struct HksBlob *random);

int32_t HksServiceImportKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *key);

int32_t HksServiceExportPublicKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksBlob *key);

int32_t HksServiceAgreeKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *privateKey, const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey);

int32_t HksServiceDeriveKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *mainKey, struct HksBlob *derivedKey);

int32_t HksServiceMac(const struct HksBlob *processName, const struct HksBlob *key,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *mac);

int32_t HksServiceGetKeyInfoList(const struct HksBlob *processName, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount);

int32_t HksServiceAttestKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain);

int32_t HksServiceGetCertificateChain(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain);

int32_t HksServiceWrapKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *targetKeyAlias, const struct HksParamSet *paramSet, struct HksBlob *wrappedData);

int32_t HksServiceUnwrapKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *targetKeyAlias, const struct HksBlob *wrappedData, const struct HksParamSet *paramSet);

int32_t HksServiceExportTrustCerts(const struct HksBlob *processName, struct HksBlob *certChain);

#ifdef __cplusplus
}
#endif

#endif
