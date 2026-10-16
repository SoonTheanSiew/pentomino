// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew

#ifndef PENTO_CORE_NODE_SOURCE_SOURCE_HPP
#define PENTO_CORE_NODE_SOURCE_SOURCE_HPP

#include "node/Node.hpp"

#include <string>

namespace Pento
{
    // A node with no input and one output, such as a file reader or a
    // camera. It reads or makes its own data, and its Step() returns
    // EndOfStream when there is no more.
    class Source : public Node
    {
      public:
        // The output must be connected, or the source reads for nothing.
        Status CheckConnections() const override;

      protected:
        explicit Source(std::string name);
    };

}  // namespace Pento

#endif  // PENTO_CORE_NODE_SOURCE_SOURCE_HPP
