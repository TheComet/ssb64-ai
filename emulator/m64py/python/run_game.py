import m64py
from os import getcwd
from os.path import join

data_path = join(getcwd(), "data")

emu = m64py.Emulator(
        corelib_path="./libmupen64plus.so", 
        config_path=data_path,
        data_path=data_path)
emu.log_message_callback = lambda level, msg: print(f"{level}: {msg}")
emu.rsp_plugin = "./mupen64plus-rsp-cxd4-sse2.so"
emu.input_plugin = "./mupen64plus-input-sdl.so"
emu.audio_plugin = "./mupen64plus-audio-sdl.so"
emu.video_plugin = "./mupen64plus-video-glide64mk2.so"

game = emu.load_ssb64_rom("./Super Smash Bros. (U) [!].z64")

def menu_screen(frame):
    game.set_fighters(Fighter.PIKACHU, None, Fighter.PIKACHU, None)
    game.set_stage(Stage.DREAMLAND)
    game.start_match()

def match_screen(frame):
    print(f"Frame {frame} game state:")

    # Print out fighter related state
    for fighter_idx in range(0, 1):
        fighter = game.get_fighter(fighter_idx)
        ID = fighter_idx*2+1
        print(f"  P{ID} x, y           : {fighter.position}")
        print(f"  P{ID} vx, vy         : {fighter.velocity}")
        print(f"  P{ID} ax, ay         : {fighter.acceleration}")
        print(f"  P{ID} orientation    : {fighter.orientation}")
        print(f"  P{ID} movement frame : {fighter.movement_frame}")
        print(f"  P{ID} movement state : {fighter.movement_state}")
        print(f"  P{ID} shield         : {fighter.shield_health}")
        print(f"  P{ID} shield recover : {fighter.shield_break_recovery_timer}")
        print(f"  P{ID} percent        : {fighter.percent}%")
        print(f"  P{ID} is grounded    : {fighter.is_grounded}")
        print(f"  P{ID} is invincible  : {fighter.is_invincible}")
        print(f"  P{ID} stocks         : {fighter.stocks}")

    # Print out stage related state
    stage = game.get_stage()
    print(f"whispy : {stage.whispy}")

def result_screen(frame):
    # start new match? evaluate results?
    pass


def frame_callback(frame):
    #screen = game.current_screen
    #if screen in (SSB64.MAIN_MENU, SSB64.CHARACTER_SELECT, SSB64.STAGE_SELECT):
    #    menu_screen(frame)
    #elif screen == SSB64.MATCH:
    #    match_screen(frame)
    #elif screen == SSB64.RESULT_SCREEN:
    #    result_screen(frame)
    #else:
    #    print(f"Warning: Unknown screen {screen}")
    try:
        match_screen(frame)
    except Exception as e:
        print(e)

emu.frame_callback = frame_callback
emu.execute()

