#include "plugin.h"
#include "version.h"
#include "osal_dynamiclib.h"
#include "m64p_common.h"
#include "m64p_plugin.h"
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define FAIL(errcode, label) do { \
            result = errcode; \
            goto label; \
        } while(0)

/* Actual input plugin */
static struct {
    m64p_handle Handle;
    ptr_PluginStartup PluginStartup;
    ptr_PluginGetVersion PluginGetVersion;
    ptr_PluginShutdown PluginShutdown;
    ptr_ControllerCommand ControllerCommand;
    ptr_GetKeys GetKeys;
    ptr_InitiateControllers InitiateControllers;
    ptr_ReadController ReadController;
    ptr_SDL_KeyDown SDL_KeyDown;
    ptr_SDL_KeyUp SDL_KeyUp;
    ptr_RomOpen RomOpen;
    ptr_RomClosed RomClosed;
    ptr_RenderCallback RenderCallback;
} ActualInputPlugin = {0};

static struct {
    int Override;
    BUTTONS Buttons;
} Controller[4];

/* This is the entry point from m64py's point of view */
EXPORT m64p_error CALL PluginStartupCucked(m64p_dynlib_handle CoreLibHandle,
                                           void* Context, void (*DebugCallback)(void *, int, const char *),
                                           const char* ActualPluginPath)
{
    m64p_error result;
    (void)CoreLibHandle;

    if (ActualInputPlugin.Handle)
        FAIL(M64ERR_ALREADY_INIT, double_init);

    result = osal_dynlib_open(&ActualInputPlugin.Handle, ActualPluginPath);
    if (result != M64ERR_SUCCESS)
    {
        if (DebugCallback)
        {
            char msgbuf[1024];
            snprintf(msgbuf, 1024, "Failed to load actual input plugin %s", ActualPluginPath);
            DebugCallback(Context, M64MSG_ERROR, msgbuf);
        }
        FAIL(M64ERR_FILES, open_actual_lib_failed);
    }

#define REQUIRE_FUNC(name) \
    if ((ActualInputPlugin.name = (ptr_##name)osal_dynlib_getproc(ActualInputPlugin.Handle, #name)) == NULL) \
        FAIL(M64ERR_INCOMPATIBLE, getproc_failed);

#define OPTIONAL_FUNC(name) \
    (ActualInputPlugin.name = (ptr_##name)osal_dynlib_getproc(ActualInputPlugin.Handle, #name))

    REQUIRE_FUNC(PluginStartup);
    REQUIRE_FUNC(PluginShutdown);
    REQUIRE_FUNC(PluginGetVersion);
    REQUIRE_FUNC(ControllerCommand);
    REQUIRE_FUNC(GetKeys);
    REQUIRE_FUNC(InitiateControllers);
    REQUIRE_FUNC(ReadController);
    REQUIRE_FUNC(SDL_KeyDown);
    REQUIRE_FUNC(SDL_KeyUp);
    REQUIRE_FUNC(RomOpen);
    REQUIRE_FUNC(RomClosed);
    OPTIONAL_FUNC(RenderCallback);

    return M64ERR_SUCCESS;

    getproc_failed               : osal_dynlib_close(ActualInputPlugin.Handle);
    open_actual_lib_failed       : memset(&ActualInputPlugin, 0, sizeof(ActualInputPlugin));
    double_init                  : return result;
}

EXPORT m64p_error CALL PluginShutdownCucked(void)
{
    if (!ActualInputPlugin.Handle)
        return M64ERR_NOT_INIT;

    osal_dynlib_close(ActualInputPlugin.Handle);
    memset(&ActualInputPlugin, 0, sizeof(ActualInputPlugin));
    memset(Controller, 0, sizeof(Controller));

    return M64ERR_SUCCESS;
}

EXPORT int CALL GetControllerOverride(int ControllerIdx)
{
    if (ControllerIdx < 0 || ControllerIdx > 3)
        return 0;

    return Controller[ControllerIdx].Override;
}

EXPORT void CALL SetControllerOverride(int ControllerIdx, int enable)
{
    if (ControllerIdx < 0 || ControllerIdx > 3)
        return;

    Controller[ControllerIdx].Override = enable;
}

EXPORT BUTTONS CALL GetControllerButtons(int ControllerIdx)
{
    BUTTONS ret = {0};
    if (ControllerIdx < 0 || ControllerIdx > 3)
        return ret;

    return Controller[ControllerIdx].Buttons;
}

EXPORT void CALL SetControllerButtons(int ControllerIdx, BUTTONS buttons)
{
    if (ControllerIdx < 0 || ControllerIdx > 3)
        return;

    Controller[ControllerIdx].Buttons = buttons;
}

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                   void (*DebugCallback)(void *, int, const char *))
{
    if (ActualInputPlugin.Handle == NULL)
        return M64ERR_SUCCESS;

    return ActualInputPlugin.PluginStartup(CoreLibHandle, Context, DebugCallback);
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    return ActualInputPlugin.PluginShutdown();
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    if (ActualInputPlugin.Handle)
        return ActualInputPlugin.PluginGetVersion(PluginType, PluginVersion, APIVersion, PluginNamePtr, Capabilities);

    /* Version info has to work even when the cucked version wasn't initialized */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_INPUT;

    if (PluginVersion != NULL)
        *PluginVersion = PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = INPUT_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = PLUGIN_NAME;

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

/******************************************************************
  Function: ControllerCommand
  Purpose:  To process the raw data that has just been sent to a
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none

  note:     This function is only needed if the DLL is allowing raw
            data, or the plugin is set to raw

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
EXPORT void CALL ControllerCommand(int Control, unsigned char *Command)
{
    ActualInputPlugin.ControllerCommand(Control, Command);
}

/******************************************************************
  Function: GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with
            the controller state.
  output:   none
*******************************************************************/
EXPORT void CALL GetKeys( int ControllerIdx, BUTTONS *Keys )
{
    if (ControllerIdx < 0 || ControllerIdx > 3)
        return;

    if (Controller[ControllerIdx].Override)
    {
        memcpy(Keys, &Controller[ControllerIdx].Buttons, sizeof(BUTTONS));
        return;
    }

    ActualInputPlugin.GetKeys(ControllerIdx, Keys);
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
{
    ActualInputPlugin.InitiateControllers(ControlInfo);
}

/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
EXPORT void CALL ReadController(int Control, unsigned char *Command)
{
    ActualInputPlugin.ReadController(Control, Command);
}

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL RomClosed(void)
{
    ActualInputPlugin.RomClosed();
}

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the
            emulation thread)
  input:    none
  output:   none
*******************************************************************/
EXPORT int CALL RomOpen(void)
{
    return ActualInputPlugin.RomOpen();
}

/******************************************************************
  Function: SDL_KeyDown
  Purpose:  To pass the SDL_KeyDown message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL SDL_KeyDown(int keymod, int keysym)
{
    ActualInputPlugin.SDL_KeyDown(keymod, keysym);
}

/******************************************************************
  Function: SDL_KeyUp
  Purpose:  To pass the SDL_KeyUp message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYUP message.
  output:   none
*******************************************************************/
EXPORT void CALL SDL_KeyUp(int keymod, int keysym)
{
    ActualInputPlugin.SDL_KeyUp(keymod, keysym);
}

EXPORT void CALL RenderCallback(void)
{
    if (ActualInputPlugin.RenderCallback)
        ActualInputPlugin.RenderCallback();
}
