// MemPurge - ui/ParentConsole.h
// Console output for CLI mode. Writes only when AttachConsole succeeded.
#pragma once

#include "engine/UniqueHandle.h"

namespace mp
{
    namespace ui
    {
        class ParentConsole
        {
        public:
            ParentConsole();
            ~ParentConsole();

            ParentConsole(const ParentConsole&) = delete;
            ParentConsole& operator=(const ParentConsole&) = delete;

            bool attached() const noexcept;

            void WriteLine(const CString& text);

        private:
            bool attached_;
            bool utf8_;
            HANDLE out_;
            engine::UniqueHandle console_;
        };

    } // namespace ui
} // namespace mp
