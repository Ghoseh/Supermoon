// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <memory>
#include <utility>
namespace bytebeat::playlist {
// Startup-worker payload ownership. Unadopted private payload destruction
// joins its worker even if separate lifecycle wrappers were exposed earlier.
// Such early wrappers cannot access the unready Core. After adopt(), residency is
// permanent for this process; this class provides no reclaim/unload operation.
template<class T> class ResidentHandoff {
    std::unique_ptr<T> pending_;
public:
    explicit ResidentHandoff(std::unique_ptr<T> value):pending_(std::move(value)){}
    ResidentHandoff(const ResidentHandoff&)=delete;
    ResidentHandoff& operator=(const ResidentHandoff&)=delete;
    T* adopt() noexcept {return pending_.release();}
};
}
