// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <cmath>
#include <cstring>
namespace supermoon::ui::register_progress {
// Match the pinned renderer's double duration calculation, float conversion,
// and final float multiplication by its runtime loop count.
inline bool value(uint64_t position,uint64_t length,int loops,float& result) noexcept {
    if(!length||position>length||loops<1||loops>1024)return false;
    const float duration=float((60.0/120.0)*16.0*8.0)*float(loops);
    const double fraction=double(position)/double(length);
    result=float(fraction*double(duration));
    if(!std::isfinite(result)||result<0||result>duration)return false;
    return true;
}
inline uint64_t token(float value) noexcept {
    uint32_t bits{};std::memcpy(&bits,&value,4);return (uint64_t(1)<<32)|bits;
}
}
