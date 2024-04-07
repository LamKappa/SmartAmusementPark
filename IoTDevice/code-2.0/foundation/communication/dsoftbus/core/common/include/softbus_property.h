/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SOFTBUS_PROPERTY_H
#define SOFTBUS_PROPERTY_H

#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// softbus default defined value
#define MAX_BYTES_LENGTH (4 * 1024 * 1024)  // Bytes max size 4M
#define MAX_MESSAGE_LENGTH (4 * 1024)   // Message max size 4K
#define MAX_BUF_LENGTH (MAX_MESSAGE_LENGTH + 64)

int GetPropertyString(const char *string, char *target, size_t targetLen);

int GetPropertyInt(const char *string, int *target);

int GetPropertyDouble(const char *string, double *target);

int GetPropertyBool(const char *string, bool *target);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* SOFTBUS_PROPERTY_H */
