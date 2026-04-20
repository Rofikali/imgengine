### 3. COMPLETE ALLOWED / FORBIDDEN MATRIX

    ✅ Allowed = ✓                     ❌ Forbidden = X
    FROM \ TO     core  memory  arch  pipeline  runtime  api  cmd  io  plugins  obs  security
    -----------------------------------------------------------------------------------------
    core           —      X      X      X         X        X    X    X     X       X     X
    memory         ✓      —      X      X         X        X    X    X     X       ✓     X
    arch           ✓      ✓      —      X         X        X    X    X     X       X     X
    pipeline       ✓      ✓      ✓      —         X        X    X    X     ✓       ✓     X
    runtime        ✓      ✓      X      ✓         —        X    X    ✓     ✓       ✓     ✓
    api            ✓      X      X      ✓         ✓        —    X    ✓     ✓       ✓     ✓
    cmd            ✓      X      X      X         X        ✓    —    X     X       X     X
    io             ✓      ✓      X      X         X        X    X    —     X       X     ✓
    plugins        ✓      X      X      ✓         ✓        X    X    X     —       X     X
    observability  ✓      ✓      X      X         X        X    X    X     X       —     X
    security       ✓      ✓      X      X         X        X    X    ✓     X       X     —

### FINAL ALLOWED / FORBIDDEN MATRIX

    FROM \ TO     core  memory  arch  pipeline  runtime  api  cmd  io  plugins  obs  security
    -----------------------------------------------------------------------------------------
    core           —      ❌      ❌      ❌        ❌       ❌   ❌   ❌    ❌       ❌    ❌
    memory         ✓      —      ❌      ❌        ❌       ❌   ❌   ❌    ❌       ⚠️    ❌
    arch           ✓      ✓      —      ❌        ❌       ❌   ❌   ❌    ❌       ❌    ❌
    pipeline       ✓      ✓      ✓      —         ❌       ❌   ❌   ❌    ❌       ✓     ❌
    runtime        ✓      ✓      ❌      ✓         —       ❌   ❌   ⚠️    ✓        ✓     ✓
    api            ✓      ❌      ❌      ❌        ✓       —    ❌   ✓     ✓        ✓     ✓
    cmd            ✓      ❌      ❌      ❌        ❌       ✓    —    ❌    ❌       ❌    ❌
    io             ✓      ✓      ❌      ❌        ❌       ❌   ❌   —     ❌       ❌    ⚠️
    plugins        ✓      ❌      ❌      ✓         ✓       ❌   ❌   ❌    —        ❌    ❌
    observability  ✓      ⚠️      ❌      ❌        ❌       ❌   ❌   ❌    ❌       —     ❌
    security       ✓      ✓      ❌      ❌        ❌       ❌   ❌   ⚠️    ❌       ❌    —, FINAL MENTAL MODEL
        USER
         ↓
        cmd
         ↓
        api        ← validation + contract
         ↓
      runtime      ← scheduling + workers
         ↓
      pipeline     ← execution logic (deterministic)
         ↓
        arch       ← SIMD kernels
         ↓
       memory      ← allocation
         ↓
        core       ← types only

    Side systems:

    security      → gatekeeper (before io)
    io            → external world bridge
    plugins       → runtime-controlled extensions
    observability → silent recorder
    startup       → system bootstrap
