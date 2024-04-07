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

/**
 * @devices tv, phone, tablet, wearable
 */
export interface CheckPackageHasInstalledResponse {
  /**
   * Whether the application exists, or whether the native application has been installed.
   * @devices tv, phone, tablet, wearable
   */
  result: boolean;
}

export interface CheckPackageHasInstalledOptions {
  /**
   * Application bundle name.
   * @devices tv, phone, tablet, wearable
   */
  bundleName: string;

  /**
   * Called when native applications are installed.
   * @devices tv, phone, tablet, wearable
   */
  success?: (data: CheckPackageHasInstalledResponse) => void;

  /**
   * Called when native applications fail to be installed.
   * @devices tv, phone, tablet, wearable
   */
  fail?: (data: any, code: number) => void;

  /**
   * Called when the execution is completed.
   * @devices tv, phone, tablet, wearable
   */
  complete?: () => void;
}

/**
 * @devices tv, phone, tablet, wearable
 */
export default class Package {
  /**
   * Checks whether an application exists, or whether a native application has been installed.
   * @param options Options.
   * @devices tv, phone, tablet, wearable
   */
  static hasInstalled(options: CheckPackageHasInstalledOptions): void;
}
