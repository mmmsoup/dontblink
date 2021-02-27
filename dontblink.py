import json
import os
import sys
import threading
import time
import Xlib
import Xlib.display

data_dir = os.getenv("HOME")+"/.dontblink"

launch_flags = []

config_data = {}

xdisplay = Xlib.display.Display()
root = xdisplay.screen().root

obscuring_windows = []

wallpaper_stage = 1
wallpaper_total_stages = 6

def load_config_data(location):
    global config_data

    try:
        print(location)
        config_data = json.loads(open(location, "r").read())
    except FileNotFoundError:
        print("ERROR (FATAL): could not load config file")
        quit()
    except json.decoder.JSONDecodeError:
        print("ERROR (FATAL): could not decode config file")
        quit()

    print("→ loaded ruleset")
    return

def set_root(stage):
    global data_dir

    # set static wallpaper with feh
    try:
        launch_flags.index("--no-animation")

        os.popen("feh --bg-fill \'"+data_dir+"/static/"+str(stage)+".png\'")

    # set animated wallpaper with asetroot
    except:
        cmd = "asetroot \'"+data_dir+"/anim/"+str(stage)+"/\' -t 50 "
        #os.popen("$(killall asetroot && "+cmd+" ) || "+cmd)
        #asetroot_thread = threading.Thread(target=set_root_anim, args=[stage,])
        #asetroot_thread.start()
        os.popen("killall asetroot")
        time.sleep(0.2)
        os.popen(cmd)

def handle_event(xevent):
    global obscuring_windows, wallpaper_stage, wallpaper_total_stages

    if xevent.type == Xlib.X.ConfigureNotify:
        has_obscuring_window_old = True if len(obscuring_windows) != 0 else False

        for rule in config_data["rules"]:
            if xevent.x == rule["x"] and xevent.y == rule["y"] and xevent.width == rule["width"] and xevent.height == rule["height"]:
                try:
                    obscuring_windows.index((xevent.window, rule["tag"]))
                except ValueError:
                    obscuring_windows.append((xevent.window, rule["tag"]))
            else:
                try:
                    obscuring_windows.pop(obscuring_windows.index((xevent.window, rule["tag"])))
                except ValueError:
                    pass
        
        # code to execute if there is now a window obscuring the desktop
        if has_obscuring_window_old == False and len(obscuring_windows) == 1:
            if wallpaper_stage == wallpaper_total_stages:
                wallpaper_stage = 1
            else:
                wallpaper_stage += 1
            set_root(wallpaper_stage)

if __name__ == '__main__':
    load_config_data(data_dir+"/conf.json")
    
    set_root(wallpaper_stage)

    root.change_attributes(event_mask=Xlib.X.SubstructureNotifyMask)
    while True:
        handle_event(xdisplay.next_event())

