// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#include "common/Logging.hpp"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Pento
{

    // Logs go to stderr, because a pipeline may write its video to stdout.
    static std::shared_ptr<spdlog::logger> MakeLogger(const std::string &name)
    {
        // Names are unique within one graph, but not across two graphs or
        // tests. spdlog throws on a duplicate registered name, so loggers
        // are not registered.
        static const auto SINK = []
        {
            spdlog::set_automatic_registration(false);
            return std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        }();

        auto logger = std::make_shared<spdlog::logger>(name, SINK);
        spdlog::initialize_logger(logger);  // applies the level for this name
        return logger;
    }

    void LoadLogLevels()
    {
        spdlog::cfg::load_env_levels("PENTO_LOG");
    }

    Logging::Logging(std::string name)
        : m_name(std::move(name)), m_logger(MakeLogger(m_name))
    {
    }

    const std::string &Logging::Name() const
    {
        return m_name;
    }

    void Logging::SetNameIfEmpty(std::string name)
    {
        if (!m_name.empty())
            return;

        m_name = std::move(name);
        m_logger = MakeLogger(m_name);
    }

    Status Logging::Fail(Status status, std::string_view detail) const
    {
        Error("{}: {}", StatusName(status), detail);
        return status;
    }

}  // namespace Pento
