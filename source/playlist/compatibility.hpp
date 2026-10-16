// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#pragma once
#include <filesystem>
#include <map>
#include <string>

namespace bytebeat::compatibility {
enum class Decision {yes,no};
enum class Result {activate,disabled,validation_failed,recovery_declined};
// Exited is deliberately NOT a clean-exit verdict. A PID is matched together
// with its creation time; inaccessible owners fail closed instead of guessing.
enum class AttemptState {none,exited,live};
struct Inputs {bool verified_build{},native_profile_available{},sites_valid{},unexpected_previous_active{},recovery_allowed{};};
// Yes is permission to validate an available profile, never permission to patch
// unknown addresses. UI, hashes and native validation remain worker responsibilities.
Result evaluate(Inputs,Decision) noexcept;
class Store {
    std::filesystem::path directory_;
    void* lock_{};
    std::map<std::string,Decision> decisions_;
public:
    explicit Store(const std::filesystem::path&);
    ~Store();
    Store(const Store&)=delete;Store& operator=(const Store&)=delete;
    bool known(const std::string&) const;
    Decision get(const std::string&) const;
    void remember(const std::string&,Decision);
    void reset();
    bool unexpected_previous_active() const;
    void mark_active(const std::string& build);
    void mark_clean();
    AttemptState previous_attempt() const;
    void begin_attempt(const std::string& build);
    void record_activation_return(unsigned result);
};
}
