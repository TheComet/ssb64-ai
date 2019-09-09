import m64py

emu = m64py.Emulator("./libmupen64plus.so.2.0.0", ".", ".")
emu.log_message_callback = lambda level, msg: print(f"{level}: {msg}")
emu.input_plugin = "./mupen64plus-input-sdl.so"
emu.audio_plugin = "./mupen64plus-audio-sdl.so"
emu.video_plugin = "./mupen64plus-video-glide64mk2.so"
emu.rsp_plugin = "./mupen64plus-rsp-cxd4-sse2.so"

game = emu.load_ssb64_rom("./Super Smash Bros. (U) [!].z64")
game.set_tournament_rules()
game.set_character(0, m64py.PIKACHU)
game.set_character(1, None)
game.set_character(2, m64py.PIKACHU)
game.set_character(3, None)
game.set_stage(m64py.DREAMLAND)

def frame_callback(frame):
    print(f"Frame {frame} game state:")
    for player_id in (0, 2):
        x, y          = game.read_player_position(player_id)
        orientation   = game.read_player_orientation(player_id)
        anim_state    = game.read_player_anim_state(player_id)
        anim_progress = game.read_player_anim_progress(player_id)
        shield        = game.read_player_shield_health(player_id)
        percent       = game.read_player_damage(player_id)
        whispy        = game.read_whispy_wind()

        print(f"  P{player_id+1} x, y        : {x}, {y}")
        print(f"  P{player_id+1} orientation : {orientation}")
        print(f"  P{player_id+1} state       : {anim_state} ({100*anim_progress:.1f}%)")
        print(f"  P{player_id+1} shield      : {100*shield:.1f}%")
        print(f"  P{player_id+1} percent     : {percent}")
    print(f"  whispy         : {whispy}")

emu.frame_callback = frame_callback
emu.execute()

