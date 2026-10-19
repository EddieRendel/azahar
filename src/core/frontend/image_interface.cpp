// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the misc/licenses/gplv2.txt file included.

#define DDSKTX_IMPLEMENT
#include <algorithm>
#include <cstring>
#include <dds-ktx.h>
#include <lodepng.h>
#include "common/file_util.h"
#include "common/logging/log.h"
#include "core/frontend/image_interface.h"

namespace Frontend {

bool ImageInterface::DecodePNG(std::vector<u8>& dst, u32& width, u32& height,
                               std::span<const u8> src) {
    const u32 lodepng_ret = lodepng::decode(dst, width, height, src.data(), src.size());
    if (lodepng_ret) {
        LOG_ERROR(Frontend, "Failed to decode because {}", lodepng_error_text(lodepng_ret));
        return false;
    }
    return true;
}

bool ImageInterface::DecodeDDS(std::vector<u8>& dst, u32& width, u32& height, ddsktx_format& format,
                               std::vector<std::pair<u32, u32>>& mips, std::span<const u8> src) {
    ddsktx_texture_info tc{};
    const int size = static_cast<int>(src.size());

    if (!ddsktx_parse(&tc, src.data(), size, nullptr)) {
        LOG_ERROR(Frontend, "Failed to decode");
        return false;
    }
    width = tc.width;
    height = tc.height;
    format = tc.format;

    // Keep the file's own mip chain: compressed formats can't have their mip levels generated
    // on the GPU, so without it the smaller levels of a mipmapped texture stay empty.
    dst.clear();
    mips.clear();
    for (int mip = 0; mip < std::max(tc.num_mips, 1); mip++) {
        ddsktx_sub_data sub_data{};
        ddsktx_get_sub(&tc, &sub_data, src.data(), size, 0, 0, mip);
        const auto offset = static_cast<u32>(dst.size());
        const auto bytes = static_cast<u32>(sub_data.size_bytes);
        dst.resize(dst.size() + bytes);
        std::memcpy(dst.data() + offset, sub_data.buff, bytes);
        mips.emplace_back(offset, bytes);
    }

    return true;
}

bool ImageInterface::EncodePNG(const std::string& path, u32 width, u32 height,
                               std::span<const u8> src) {
    std::vector<u8> out;
    const u32 lodepng_ret = lodepng::encode(out, src.data(), width, height);
    if (lodepng_ret) {
        LOG_ERROR(Frontend, "Failed to encode {} because {}", path,
                  lodepng_error_text(lodepng_ret));
        return false;
    }

    FileUtil::IOFile file{path, "wb"};
    if (file.WriteBytes(out.data(), out.size()) != out.size()) {
        LOG_ERROR(Frontend, "Failed to save encode to path {}", path);
        return false;
    }

    return true;
}

} // namespace Frontend
