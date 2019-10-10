import m64py
import traceback
from os import getcwd
from os.path import join


class Gym(object):
    def __init__(self):
        def message_callback(level, msg):
            if level < 3:
                print(f"{level}: {msg}")

        data_path = join(getcwd(), "data")

        self.emu = m64py.Emulator(
                corelib_path="./libmupen64plus.so", 
                config_path=data_path,
                data_path=data_path)
        self.emu.log_message_callback = message_callback
        self.emu.frame_callback = self.frame_callback

        # Probably want to uncomment this for training
        #self.emu.audio_plugin = None
        #self.emu.video_plugin = None
        #self.emu.speed_limiter = False

        self.game = self.emu.load_ssb64_rom("./Super Smash Bros. (U) [!].z64")
        self.emu.load_state("data/savestates/pika-vs-pika_dreamland.m64savestate")

        self.fighters = tuple()
        self.stage = None
        self.old_match_in_progress = False

    def on_match_begin(self):
        # can access game settings and load the appropriate AI here?

        # Traditionally, slots 2 and 4 are used on the n64 because this spawns
        # both players on the side plats on Dreamland
        self.fighters = (self.game.get_fighter(2), self.game.get_fighter(4))
        self.stage = self.game.get_stage()

        # Override controllers if you want to control the fighers from code
        #self.fighters[0].controller.override = True
        #self.fighters[1].controller.override = True

    def on_match_end(self):
        # load savestate for the next match? evaluate results?
        pass

    def on_next_frame(self, frame):
        print("========================")
        print(f"Frame {frame} game state")
        print("========================")

        # Print out fighter related state
        for n, fighter in enumerate(self.fighters):
            print(f"Player {n+1} -------------")
            print(f"    character      : {fighter.character}")
            print(f"    x, y           : {fighter.position}")
            print(f"    vx, vy         : {fighter.velocity}")
            print(f"    ax, ay         : {fighter.acceleration}")
            print(f"    orientation    : {fighter.orientation}")
            print(f"    movement frame : {fighter.movement_frame}")
            print(f"    movement state : {fighter.movement_state}")
            print(f"    shield         : {fighter.shield_health}")
            print(f"    shield recover : {fighter.shield_break_recovery_timer}")
            print(f"    percent        : {fighter.percent}%")
            print(f"    is grounded    : {fighter.is_grounded}")
            print(f"    is invincible  : {fighter.is_invincible}")
            print(f"    stocks         : {fighter.stocks}")
            print("")

        # Print out stage related state
        print(f"whispy : {self.stage.whispy}")

    def frame_callback(self, frame):
        match_in_progress = self.game.is_match_in_progress

        if match_in_progress and not self.old_match_in_progress:
            self.old_match_in_progress = True
            self.on_match_begin()
        elif not match_in_progress and self.old_match_in_progress:
            self.old_match_in_progress = False
            self.on_match_end()
        if match_in_progress:
            self.on_next_frame(frame)

    def run(self):
        self.emu.execute()


gym = Gym()
gym.run()

