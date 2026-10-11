#ifndef INK_FRONTEND_CONTEXT_H
#define INK_FRONTEND_CONTEXT_H

#include "ink/core/diagnostic.h"
#include "ink/core/source_manager.h"

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

      core::DiagnosticEngine &diagnosticEngine() noexcept
      {
        return Diagnostics;
      }

      const core::DiagnosticEngine &diagnosticEngine() const noexcept
      {
        return Diagnostics;
      }

      core::SourceManager &sourceManager() noexcept
      {
        return Sources;
      }

      const core::SourceManager &sourceManager() const noexcept
      {
        return Sources;
      }

    private:
      core::SourceManager &Sources;
      core::DiagnosticEngine &Diagnostics;
  };
} // namespace ink::frontend

#endif
