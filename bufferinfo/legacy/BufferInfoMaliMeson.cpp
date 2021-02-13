/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "hwc-bufferinfo-mali-meson"

#include "BufferInfoMaliMeson.h"

#include <log/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <cinttypes>

#include "gralloc_priv.h"

namespace android {

LEGACY_BUFFER_INFO_GETTER(BufferInfoMaliMeson);

#if defined(MALI_GRALLOC_INTFMT_AFBC_BASIC) && \
    defined(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16)
uint64_t BufferInfoMaliMeson::ConvertGrallocFormatToDrmModifiers(
    uint64_t flags) {
  uint64_t features = 0UL;

  if (flags & MALI_GRALLOC_INTFMT_AFBC_BASIC) {
    if (flags & MALI_GRALLOC_INTFMT_AFBC_WIDEBLK)
      features |= AFBC_FORMAT_MOD_BLOCK_SIZE_32x8;
    else
      features |= AFBC_FORMAT_MOD_BLOCK_SIZE_16x16;
  }

  if (flags & MALI_GRALLOC_INTFMT_AFBC_SPLITBLK)
    features |= (AFBC_FORMAT_MOD_SPLIT | AFBC_FORMAT_MOD_SPARSE);

  if (flags & MALI_GRALLOC_INTFMT_AFBC_TILED_HEADERS)
    features |= AFBC_FORMAT_MOD_TILED;

  if (features)
    return DRM_FORMAT_MOD_ARM_AFBC(features | AFBC_FORMAT_MOD_YTR);

  return 0;
}
#else
uint64_t BufferInfoMaliMeson::ConvertGrallocFormatToDrmModifiers(
    uint64_t /* flags */) {
  return 0;
}
#endif

int BufferInfoMaliMeson::ConvertBoInfo(buffer_handle_t handle,
                                       hwc_drm_bo_t *bo) {
  auto const *hnd = reinterpret_cast<private_handle_t const *>(handle);
  if (!hnd)
    return -EINVAL;

  if (!(hnd->usage & GRALLOC_USAGE_HW_FB))
    return -EINVAL;

  uint32_t fmt = ConvertHalFormatToDrm(hnd->req_format);
  if (fmt == DRM_FORMAT_INVALID)
    return -EINVAL;

  bo->modifiers[0] = BufferInfoMaliMeson::ConvertGrallocFormatToDrmModifiers(
      hnd->internal_format);

  bo->width = hnd->width;
  bo->height = hnd->height;
  bo->hal_format = hnd->req_format;
  bo->format = fmt;
  bo->usage = hnd->usage;
  bo->prime_fds[0] = hnd->share_fd;
  bo->pitches[0] = hnd->byte_stride;
  bo->offsets[0] = 0;

  return 0;
}

}  // namespace android
