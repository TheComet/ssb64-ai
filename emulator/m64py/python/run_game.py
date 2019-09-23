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
game.set_tournament_rules()
game.set_character(0, m64py.PIKACHU)
game.set_character(1, m64py.PIKACHU)
game.set_character(2, m64py.PIKACHU)
game.set_character(3, None)
game.set_stage(m64py.DREAMLAND)

def frame_callback(frame):
    print(f"Frame {frame} game state:")
    for player_id in (0, 1):
        x, y          = game.read_player_position(player_id)
        vx, vy        = game.read_player_launch_velocity(player_id)
        orientation   = game.read_player_orientation(player_id)
        anim_state    = game.read_player_anim_state(player_id)
        anim_progress = game.read_player_anim_progress(player_id)
        shield        = game.read_player_shield_health(player_id)
        percent       = game.read_player_damage(player_id)
        whispy        = game.read_whispy_wind()

        print(f"  P{player_id+1} x, y        : {x}, {y}")
        print(f"  P{player_id+1} vx, vy      : {vx}, {vy}")
        print(f"  P{player_id+1} orientation : {orientation}")
        print(f"  P{player_id+1} state       : {anim_state} ({100*anim_progress:.1f}%)")
        print(f"  P{player_id+1} shield      : {shield}")
        print(f"  P{player_id+1} percent     : {percent}%")
    print(f"  whispy         : {whispy}")

emu.frame_callback = frame_callback
emu.execute()

