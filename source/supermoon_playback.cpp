// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
#define ADAPTER_STREAMING_BUILD
// Playback entry; diagnostic builds are separate and cannot be hot-armed.
// The generated header supplies the reviewed carrier/profile evidence; changing
// its arming data requires separate qualification. The payload DllMain is inert;
// the explicit export activates once after the proxy worker's startup gates.
#define ADAPTER_PLAYBACK_BUILD
#include "supermoon_playback_config.hpp"
#include "supermoon_adapter.cpp"
