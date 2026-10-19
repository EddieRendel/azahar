// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the misc/licenses/gplv2.txt file included.

#pragma once

#include <span>
#include <string>
#include <utility>
#include <vector>
#include <dds-ktx.h>
#include "common/common_types.h"

namespace Frontend {

/**
 * Utility class that provides image decoding/encoding to the custom texture manager.
 * Can be optionally overriden by frontends to provide a custom implementation.
 */
class ImageInterface {
public:
    virtual ~ImageInterface() = default;

    virtual bool DecodePNG(std::vector<u8>& dst, u32& width, u32& height, std::span<const u8> src);
    /// Decodes every mip level of the first array layer / face into dst, level after level.
    /// mips receives each level's (offset, size) in dst; level 0 comes first.
    virtual bool DecodeDDS(std::vector<u8>& dst, u32& width, u32& height, ddsktx_format& format,
                           std::vector<std::pair<u32, u32>>& mips, std::span<const u8> src);
    virtual bool EncodePNG(const std::string& path, u32 width, u32 height, std::span<const u8> src);
};

} // namespace Frontend
