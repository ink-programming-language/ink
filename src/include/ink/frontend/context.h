#ifndef INK_FRONTEND_CONTEXT_H
#define INK_FRONTEND_CONTEXT_H

#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/source/source_manager.h"
#include <utility>

namespace ink::frontend
{
  // Borrows services shared by frontend stages; both services must outlive this view.
  class FrontendContext
  {
    public:
      explicit FrontendContext(core::SourceManager &Sources, core::DiagnosticEngine &Diagnostics)
          : Sources(Sources),
            Diagnostics(Diagnostics)
      {
      }

      auto &diagnosticEngine(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Diagnostics);
      }

      auto &sourceManager(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Sources);
      }

    private:
      core::SourceManager &Sources;
      core::DiagnosticEngine &Diagnostics;
  };
} // namespace ink::frontend

#endif
