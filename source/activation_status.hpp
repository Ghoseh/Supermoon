// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
namespace bytebeat::activation {
enum class Stage:uint32_t {preflight,lifecycle_exposed,core_ready,publication_started,published};
inline const char* name(uint32_t stage) noexcept {
    switch(Stage(stage)){
    case Stage::preflight:return "activation_stage_preflight";
    case Stage::lifecycle_exposed:return "activation_stage_lifecycle_hooks_may_be_exposed";
    case Stage::core_ready:return "activation_stage_core_ready";
    case Stage::publication_started:return "activation_stage_publication_started";
    case Stage::published:return "activation_stage_published";
    }return "activation_stage_unknown";
}
inline const char* failure(uint32_t stage,uint32_t result=0) noexcept {
    if(stage==uint32_t(Stage::preflight)&&result==1)return "activation_refused_exposure_unknown_exit_normally_no_retry";
    return stage==uint32_t(Stage::preflight)?"activation_refused_preflight_no_retry":
        "activation_refused_hooks_may_be_exposed_exit_normally_no_retry";
}
}
