// SPDX-License-Identifier: MIT
#pragma once
#include "recovery_lifecycle.hpp"
#include <cstdio>
namespace bytebeat::recovery {
// Writer-only formatting: never called by allocation, Init or audio callbacks.
class Reporter {
    uint64_t last_{};bool wrote_{};
public:
    bool sample(const Registry::Snapshot& s,uint64_t now,char* out,size_t bytes) noexcept {
        if(wrote_&&now-last_<1000)return false;wrote_=true;last_=now;
        const bool degraded=s.sources>=source_capacity||s.parameters>=parameter_capacity||s.events>=event_capacity||s.source_refused||s.parameter_refused||s.event_refused||s.id_reuse;
        int n=std::snprintf(out,bytes,
            "# recovery_capacity cumulative_not_songs sources=%u/%u parameters=%u/%u events=%u/%u source_refused=%llu parameter_refused=%llu event_refused=%llu refused_tracked=%llu id_reuse=%llu late_calls=%llu overlaps=%llu cleanup_waits=%llu status=%s\n",
            s.sources,source_capacity,s.parameters,parameter_capacity,s.events,event_capacity,
            (unsigned long long)s.source_refused,(unsigned long long)s.parameter_refused,(unsigned long long)s.event_refused,
            (unsigned long long)s.refused,(unsigned long long)s.id_reuse,(unsigned long long)s.late_calls,(unsigned long long)s.overlaps,(unsigned long long)s.cleanup_waits,
            degraded?"degraded_new_work_may_be_refused_exit_normally_restart_no_slot_reuse":"retained_slots_no_session_headroom_qualification");
        return n>0&&size_t(n)<bytes;
    }
};
}
