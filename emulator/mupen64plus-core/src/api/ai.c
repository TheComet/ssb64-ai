#include "api/ai.h"
#include "callbacks.h"
#include <stddef.h>

static void (*l_VICallback)(void* Context) = NULL;
static void* l_Context = NULL;

void AISetVICallback(void (*pFunc)(void*), void* Context)
{
#if defined(M64P_AI)
    l_VICallback = pFunc;
    l_Context = Context;
#else
    DebugMessage(M64MSG_ERROR, "AI modifications aren't enabled (recompile with -DM64P_AI=ON)");
#endif
}

void AIDoVICallback(void)
{
    if (l_VICallback)
        l_VICallback(l_Context);
}

void AIInit(void)
{
}

void AIDeinit(void)
{
    l_VICallback = NULL;
    l_Context = NULL;
}
