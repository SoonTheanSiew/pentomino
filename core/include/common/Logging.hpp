// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_COMMON_LOGGING_HPP
#define PENTO_CORE_COMMON_LOGGING_HPP

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "common/Status.hpp"

namespace Pento
{
    // Reads log levels from the PENTO_LOG environment variable, for example
    // PENTO_LOG=info,decode=debug. Call it before the graph is built.
    void LoadLogLevels();

    // Gives a node, an edge or another class a name and its own logger.
    // Log lines show the name, and the level can be set per name.
    class Logging
    {
      public:
        const std::string &Name() const;

        // Used by Graph to name a node that was given no name.
        void SetNameIfEmpty(std::string name);

      protected:
        explicit Logging(std::string name);

        // Example: Debug("decoded frame {}", index);
        template <class... Args>
        void Debug(spdlog::format_string_t<Args...> format,
                   Args &&...args) const;
        template <class... Args>
        void Info(spdlog::format_string_t<Args...> format,
                  Args &&...args) const;
        template <class... Args>
        void Warn(spdlog::format_string_t<Args...> format,
                  Args &&...args) const;
        template <class... Args>
        void Error(spdlog::format_string_t<Args...> format,
                   Args &&...args) const;

        // Logs "status: detail" as an error and returns the status.
        // Example: return Fail(Status::FileNotFound, path);
        Status Fail(Status status, std::string_view detail) const;

      private:
        std::string m_name;
        std::shared_ptr<spdlog::logger> m_logger;
    };

    // Templates are defined here, below the class, because every caller
    // needs to see them.

    template <class... Args>
    void Logging::Debug(spdlog::format_string_t<Args...> format,
                        Args &&...args) const
    {
        m_logger->debug(format, std::forward<Args>(args)...);
    }

    template <class... Args>
    void Logging::Info(spdlog::format_string_t<Args...> format,
                       Args &&...args) const
    {
        m_logger->info(format, std::forward<Args>(args)...);
    }

    template <class... Args>
    void Logging::Warn(spdlog::format_string_t<Args...> format,
                       Args &&...args) const
    {
        m_logger->warn(format, std::forward<Args>(args)...);
    }

    template <class... Args>
    void Logging::Error(spdlog::format_string_t<Args...> format,
                        Args &&...args) const
    {
        m_logger->error(format, std::forward<Args>(args)...);
    }

}  // namespace Pento

#endif  // PENTO_CORE_COMMON_LOGGING_HPP
