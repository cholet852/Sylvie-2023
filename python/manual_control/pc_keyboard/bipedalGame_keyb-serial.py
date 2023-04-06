# PC Keyboard and Serial control for Sylvie 2021 (Bipedal Robot Lower Body)

import sys
# insert at 1, 0 is the script path (or '' in REPL)
sys.path.insert(1, '../inverse_kinematics')
sys.path.insert(1, '../registry')

from time import sleep
import serial
import keyboard
import os

import nanoik_v2 as nanoik
import bipedalGame_data as bipedalGame
import sylvieos_data as sylvieOS

ser = serial.Serial('/dev/ttyUSB0', 115200)

# Our global variables
previous_menu = 0
menu = 0      
encoded_command = ""

link_1 = 33.4 # Default thigh length
link_2 = 30.5 # Default leg length

hypotenuse = 62.7 # Default hypotenuse after alignment
foot_dist = 14.3 # Default distance between foot gearboxes

ee_zL = 0
ee_xL = 0.001

ee_zR = 0
ee_xR = 0.001

ee_zL_sr = 0
ee_zR_sr = 0

speed = {
    "ver":   [0.0, 0.25, 0.5, 1.5, 2.0, 2.5, 3.0],
    "hor":   [0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0],
    "sr":    [0.0, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5],
    "angle": [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 5.0] 
}

default_speed = [0, 50, 45, 40, 35, 30, 25, 20, 10]
balance_pid = [24.0, 8.0, 0.01]

gbx = {
    "name":       ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a"],
    "angle":      [17.0, 0.0, 0.0, 102.48, 102.48, 10.07, 10.07, 47.55, 47.55, 20.0, 20.0],
    "old_angle":  [17.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    "offset":     [0.0, 0.0, 0.0, -70.0, -70.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    "old_offset": [0.0, 0.0, 0.0, -70.0, -70.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],    
    "dir":        [True, True, True, False, False, True, True, True, True, False, True],
    "balancing":  [False, False, False, False, False, False, False, False, False, False, False],
    "solvedik":   [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
}

on_startup = True
speed_setting = 1
d_speed_setting = 3

# Quick rounder and stringifyer

def quickRnd(val, check_plus=False):
    val_pt1 = str(float(round(val, 2)))
    
    newVal = val_pt1
    
    if check_plus == True:
        if val > 0:
            newVal = "+" + val_pt1
    
    return newVal

# Reset menu

def reset_menu():
    global on_startup
    global previous_menu

    on_startup = False
    previous_menu = -1	
    
# Show key pressed
    
def show_key(keyboard_key):
    os.system('clear')
    print("KEYBOARD KEY [" + keyboard_key + "] PRESSED")

    sleep(0.25)
    reset_menu()

# Apply forward kinematics, and show key pressed
    
def fwd_key(name, keyboard_key):
    os.system('clear')
    print("KEYBOARD KEY [" + keyboard_key + "] PRESSED")
    full_angle = gbx["angle"][name] + gbx["offset"][name] 

    if gbx["balancing"][name] == False:
        gbx_n = gbx["name"][name]
        gbx_s = str(default_speed[d_speed_setting])
        gbx_a = quickRnd(full_angle)
        
        quick_command = gbx_n + "s" + gbx_s + ";" + gbx_n + "m" + gbx_a + "\n"
        ser.write(quick_command.encode("utf-8"))
    
    gbx["old_offset"][name] = gbx["offset"][name] 
    
    # print("ENCODED COMMAND: ", quick_command) # Debug only
    # print("NEW OFFSET: ", str(gbx["offset"][name])) # Debug only

    sleep(0.25)
    # sleep(1) # Debug only
    reset_menu()

# Toggle joint movement angles per key press

def switch_speed(dial, n_type=1):
    os.system('clear')
    n_max = [0, 6, 8]
    global speed_setting
    global d_speed_setting
    c_val = [0, speed_setting, d_speed_setting]
    c_string = ["", "ANGLE SPEED", "MOVEMENT SPEED"]
    
    if dial == "up":
        if c_val[n_type] < n_max[n_type]:
            c_val[n_type] += 1
    elif dial == "down":
        if c_val[n_type] > 1: 
            c_val[n_type] -= 1
    
    speed_setting = c_val[n_type] if n_type == 1 else speed_setting
    d_speed_setting = c_val[n_type] if n_type == 2 else d_speed_setting
    print(c_string[n_type] + " SET TO [" + str(c_val[n_type]) + "]")

    sleep(0.25)
    reset_menu()

def switch_pid(dial, n_type=0):
    os.system('clear')
    d_value = [0.0, 0.0, 0.0]

    if dial == "up":
        d_value = [0.1, 0.1, 0.01]
    elif dial == "down":
        d_value = [-0.1, -0.1, -0.01]
 
    balance_pid[n_type] += d_value[n_type]
    
    d_letter = ["p", "i", "d"]
    print("PID k" + d_letter[n_type] + " SET TO [" + quickRnd(balance_pid[n_type]) + "]")          

    quick_command = "3" + d_letter[n_type] + quickRnd(balance_pid[n_type])
    quick_command += ";4" + d_letter[n_type] + quickRnd(balance_pid[n_type])
    quick_command += ";0" + d_letter[n_type] + quickRnd(balance_pid[n_type]) + "\n"
    
    # print("ENCODED COMMAND: ", quick_command)
    
    ser.write(quick_command.encode("utf-8"))

    sleep(0.25)
    # sleep(3) # debug only
    reset_menu()

# Main loop

while True:
    
    if menu == 0:
        os.system('clear')

        ee_z = 0
        ee_x = 0.01
        
        sylvieOS.splash()

        homing_required = str(input("Enter servo homing menu? [y/n]: "))
        
        if homing_required == "y":
            os.system('clear')        
            print("Warning: Please ensure that both legs are as aligned as possible!")
          
            homing_q1 = str(input("Home the thighs? [y/n]: "))
            if homing_q1 == "y":
                ser.write("3h;4h\n".encode("utf-8"))
                print("Waiting for thighs homing...")
                sleep(10)
                ser.write("3m32.48;4m32.48\n".encode("utf-8"))
                print("Waiting for thighs start position...")                
                sleep(5)            
            
            homing_q2 = str(input("Home the knees? [y/n]: "))
            if homing_q2 == "y":
                ser.write("5h;6h\n".encode("utf-8"))
                print("Waiting for knees homing...")                
                sleep(10)
                ser.write("5m10.07;6m10.07\n".encode("utf-8"))
                print("Waiting for knees start position...")  
                sleep(5)              
            
            homing_q3 = str(input("Home the legs? [y/n]: "))
            if homing_q3 == "y":
                ser.write("7h;8h\n".encode("utf-8"))
                print("Waiting for legs homing...")                  
                sleep(15)
                ser.write("7m47.55;8m47.55\n".encode("utf-8"))
                print("Waiting for legs start position...") 
                sleep(5)
                
            homing_q4 = str(input("Home the right foot? [y/n]: "))
            if homing_q4 == "y":
                ser.write("9h\n".encode("utf-8"))
                print("Waiting for right foot homing...")                
                sleep(10)
                ser.write("9m20\n".encode("utf-8"))
                print("Waiting for right foot start position...")                
                sleep(5)
                
            homing_q5 = str(input("Home the left foot? [y/n]: "))
            if homing_q5 == "y":
                ser.write("ah\n".encode("utf-8"))
                print("Waiting for left foot homing...")
                sleep(10)
                ser.write("am20\n".encode("utf-8"))
                print("Waiting for left foot start position...")
                sleep(5)

            homing_q6 = str(input("Home the waist? [y/n]: "))
            if homing_q6 == "y":
                ser.write("0h\n".encode("utf-8"))
                print("Waiting for waist homing...")
                sleep(10)
                ser.write("0m17\n".encode("utf-8"))
                print("Waiting for waist start position...")
                sleep(5)

            os.system('clear')
            print("Use Keys 4 and 5 to straighten up or lower the android torso.")
            sleep(5)
        else:
            os.system('clear')
            print("Warning: It is not recommended to use Inverse Kinematics without homing.")
            menu_q1 = str(input("Stick to Forward Kinematics. Press 'y' to continue or 'n' to exit [y/n]: "))
            if menu_q1 == "y":
                for i in range(0, 11):
                    gbx["angle"][i] = 0.0
                    gbx["old_angle"][i] = 0.0
                    gbx["offset"][i] = 0.0
                    gbx["old_offset"][i] = 0.0 
            else: 
                exit()
            
        ee_zL = hypotenuse
        ee_zR = hypotenuse
        ee_zL_sr = hypotenuse
        ee_zR_sr = hypotenuse
        menu = 1

    elif menu == 1:
        solvedik_left = nanoik.solveKinematicsSide(ee_zL, ee_xL, link_1, link_2) 
        solvedik_right = nanoik.solveKinematicsSide(ee_zR, ee_xR, link_1, link_2)
        solvedik_front = nanoik.solveKinematicsFront(ee_zL_sr, ee_zR_sr, foot_dist)

        gbx["solvedik"][3] = solvedik_right[0]
        gbx["solvedik"][4] = solvedik_left[0]
        gbx["solvedik"][5] = solvedik_right[1]
        gbx["solvedik"][6] = solvedik_left[1]
        gbx["solvedik"][7] = solvedik_right[2]
        gbx["solvedik"][8] = solvedik_left[2]
        gbx["solvedik"][9] = solvedik_front
        gbx["solvedik"][10] = solvedik_front

        # Handle startup and post-startup calculations

        if menu != previous_menu:
			
            if on_startup == True:
                for i in range(3, 11):
                    gbx["old_angle"][i] = gbx["solvedik"][i]
            else:
                encoded_command = ""
                new_speed = default_speed[d_speed_setting]
                i_diff = -1
                i_char = ""

                for i in range(3, 11):
                    gbx_diff = 0.0
                    if gbx["dir"][i] == True:
                        gbx_diff = gbx["solvedik"][i] - gbx["old_angle"][i]
                    else:
                        gbx_diff = gbx["old_angle"][i] - gbx["solvedik"][i]
                        
                    gbx["angle"][i] += gbx_diff
                    full_angle = gbx["angle"][i] + gbx["offset"][i]
                    
                    if gbx_diff != 0.0 or gbx["offset"][i] != gbx["old_offset"][i]:
                        if i_diff == -1:
                            i_diff = abs(gbx_diff)
                        else:
                            if abs(gbx_diff) > 0:
                                new_speed = (i_diff / abs(gbx_diff)) * default_speed[d_speed_setting]
                                if new_speed < 10:
                                    new_speed = 10
                                elif new_speed > 70:
                                    new_speed = 70
                        
                        if gbx["balancing"][i] == False:
                            gbx_n = gbx["name"][i]
                            gbx_s = str(int(new_speed))
                            gbx_a = quickRnd(full_angle)   
                            
                            encoded_command += i_char + gbx_n + "s" + gbx_s + ";" + gbx_n + "m" + gbx_a
                            
                            i_char = ";"
                        
                    gbx["old_angle"][i] = gbx["solvedik"][i]
                    gbx["old_offset"][i] = gbx["offset"][i]

                encoded_command += "\n" # Add \n terminator after the loop
                ser.write(encoded_command.encode("utf-8"))

                sleep(0.25)

            os.system('clear')
            
            # Print joint angles on screen

            r_leg_angles = [quickRnd(gbx["angle"][3]), quickRnd(gbx["angle"][5]), quickRnd(gbx["angle"][7])]
            l_leg_angles = [quickRnd(gbx["angle"][4]), quickRnd(gbx["angle"][6]), quickRnd(gbx["angle"][8])]

            r_leg_offsets = [quickRnd(gbx["offset"][3], True), quickRnd(gbx["offset"][5], True), quickRnd(gbx["offset"][7], True)]
            l_leg_offsets = [quickRnd(gbx["offset"][4], True), quickRnd(gbx["offset"][6], True), quickRnd(gbx["offset"][8], True)]

            r_ee_pos = round(ee_zR, 3), round(ee_xR, 3)
            l_ee_pos = round(ee_zL, 3), round(ee_xL, 3)

            sr_angles = [quickRnd(gbx["angle"][1]), quickRnd(gbx["angle"][2]), quickRnd(gbx["angle"][9]), quickRnd(gbx["angle"][10])] 
            sr_offsets = [quickRnd(gbx["offset"][1], True), quickRnd(gbx["offset"][2], True), quickRnd(gbx["offset"][9], True), quickRnd(gbx["offset"][10], True)]
            waist_angle = quickRnd(gbx["angle"][0])
            waist_offset = quickRnd(gbx["offset"][0], True)
     
            bipedalGame.menuOneText(l_leg_angles, l_leg_offsets, r_leg_angles, r_leg_offsets, l_ee_pos, r_ee_pos, sr_angles, sr_offsets, waist_angle, waist_offset, encoded_command)

            previous_menu = menu
            
        # Keyboard Control

        if keyboard.is_pressed('1'):
            nanoik.drawRadarSide(solvedik_left[0], solvedik_left[1], solvedik_left[2], link_1, link_2, "blue")
            show_key('1')
        elif keyboard.is_pressed('2'):
            nanoik.drawRadarSide(solvedik_right[0], solvedik_right[1], solvedik_right[2], link_1, link_2, "red")
            show_key('2')
        elif keyboard.is_pressed('3'):
            nanoik.drawRadarFront((gbx["angle"][10] + 70), ee_zL, ee_zR, foot_dist, "green")
            show_key('3')
        elif keyboard.is_pressed('w'):
            ee_zL = ee_zL - speed["ver"][speed_setting]
            show_key('w')
        elif keyboard.is_pressed('s'):
            ee_zL = ee_zL + speed["ver"][speed_setting]
            show_key('s')
        elif keyboard.is_pressed('d'):
            ee_xL = ee_xL + speed["hor"][speed_setting]
            show_key('d')
        elif keyboard.is_pressed('a'):
            ee_xL = ee_xL - speed["hor"][speed_setting]
            show_key('a')
        elif keyboard.is_pressed('t'):
            ee_zR = ee_zR - speed["ver"][speed_setting]
            show_key('t')
        elif keyboard.is_pressed('g'):
            ee_zR = ee_zR + speed["ver"][speed_setting]
            show_key('g')
        elif keyboard.is_pressed('h'):
            ee_xR = ee_xR + speed["hor"][speed_setting]
            show_key('h')
        elif keyboard.is_pressed('f'):
            ee_xR = ee_xR - speed["hor"][speed_setting]
            show_key('f')
        elif keyboard.is_pressed('i'):
            ee_zL = ee_zL - speed["ver"][speed_setting]
            ee_zR = ee_zR - speed["ver"][speed_setting]
            show_key('i')
        elif keyboard.is_pressed('k'):
            ee_zL = ee_zL + speed["ver"][speed_setting]
            ee_zR = ee_zR + speed["ver"][speed_setting]
            show_key('k')
        elif keyboard.is_pressed('l'):
            ee_xL = ee_xL + speed["hor"][speed_setting]
            ee_xR = ee_xR + speed["hor"][speed_setting]
            show_key('l')
        elif keyboard.is_pressed('j'):
            ee_xL = ee_xL - speed["hor"][speed_setting]
            ee_xR = ee_xR - speed["hor"][speed_setting]
            show_key('j')

        elif keyboard.is_pressed('z'):
            ee_zL = ee_zL - speed["sr"][speed_setting]
            ee_zR = ee_zR + speed["sr"][speed_setting]

            ee_zL_sr = ee_zL_sr - speed["sr"][speed_setting]
            ee_zR_sr = ee_zR_sr + speed["sr"][speed_setting]

            show_key('z')			
        elif keyboard.is_pressed('x'):
            ee_zL = ee_zL + speed["sr"][speed_setting]
            ee_zR = ee_zR - speed["sr"][speed_setting]

            ee_zL_sr = ee_zL_sr + speed["sr"][speed_setting]
            ee_zR_sr = ee_zR_sr - speed["sr"][speed_setting]
                                   
            show_key('x')
            
        elif keyboard.is_pressed('c'):
            gbx["angle"][0] += speed["angle"][speed_setting]
            if gbx["balancing"][0] == False:
                quick_command = "0m" + quickRnd(gbx["angle"][0]) + "\n"
                ser.write(quick_command.encode("utf-8"))
            show_key('c')
        elif keyboard.is_pressed('v'):
            gbx["angle"][0] -= speed["angle"][speed_setting]
            if gbx["balancing"][0] == False:            
                quick_command = "0m" + quickRnd(gbx["angle"][0]) + "\n"
                ser.write(quick_command.encode("utf-8"))                
            show_key('v')
        elif keyboard.is_pressed('b'):
            gbx["angle"][1] += speed["angle"][speed_setting]
            gbx["angle"][2] -= speed["angle"][speed_setting]
            gbx["offset"][9] -= speed["angle"][speed_setting] 
            gbx["offset"][10] += speed["angle"][speed_setting]
            
            full_angle_9 = gbx["angle"][9] + gbx["offset"][9]            
            full_angle_10 = gbx["angle"][10] + gbx["offset"][10] 
            
            gbx["old_offset"][9] = gbx["offset"][9] 
            gbx["old_offset"][10] = gbx["offset"][10]
                                                
            quick_command = "1m" + quickRnd(gbx["angle"][1]) + ";2m" + quickRnd(gbx["angle"][2])
            quick_command += ";9m" + quickRnd(full_angle_9) + ";am" + quickRnd(full_angle_10) + "\n"            
            
            show_key('b')
            ser.write(quick_command.encode("utf-8"))
        elif keyboard.is_pressed('n'):
            gbx["angle"][1] -= speed["angle"][speed_setting]
            gbx["angle"][2] += speed["angle"][speed_setting]
            gbx["offset"][9] += speed["angle"][speed_setting] 
            gbx["offset"][10] -= speed["angle"][speed_setting]

            full_angle_9 = gbx["angle"][9] + gbx["offset"][9]            
            full_angle_10 = gbx["angle"][10] + gbx["offset"][10] 
            
            gbx["old_offset"][9] = gbx["offset"][9] 
            gbx["old_offset"][10] = gbx["offset"][10]
                                                
            quick_command = "1m" + quickRnd(gbx["angle"][1]) + ";2m" + quickRnd(gbx["angle"][2])
            quick_command += ";9m" + quickRnd(full_angle_9) + ";am" + quickRnd(full_angle_10) + "\n"
            
            show_key('n')
            ser.write(quick_command.encode("utf-8"))
            
        elif keyboard.is_pressed('4'):
            gbx["offset"][3] += speed["angle"][speed_setting]            
            gbx["offset"][4] += speed["angle"][speed_setting]
            show_key('4')
        elif keyboard.is_pressed('5'):
            gbx["offset"][3] -= speed["angle"][speed_setting]
            gbx["offset"][4] -= speed["angle"][speed_setting]
            show_key('5')                                 
        elif keyboard.is_pressed('6'):
            gbx["offset"][5] += speed["angle"][speed_setting]
            gbx["offset"][6] += speed["angle"][speed_setting]
            show_key('6')            
        elif keyboard.is_pressed('7'):
            gbx["offset"][5] -= speed["angle"][speed_setting]
            gbx["offset"][6] -= speed["angle"][speed_setting]
            show_key('7')            
        elif keyboard.is_pressed('8'):
            gbx["offset"][7] += speed["angle"][speed_setting]
            gbx["offset"][8] += speed["angle"][speed_setting]
            show_key('8')            
        elif keyboard.is_pressed('9'):
            gbx["offset"][7] -= speed["angle"][speed_setting]
            gbx["offset"][8] -= speed["angle"][speed_setting]
            show_key('9')            
           
        # Arrow keys   
            
        elif keyboard.is_pressed('up'):
            switch_speed('up')  
        elif keyboard.is_pressed('down'):
            switch_speed('down')          
        elif keyboard.is_pressed('o'):
            switch_speed('up', 2)
        elif keyboard.is_pressed('p'):
            switch_speed('down', 2)
               
        elif keyboard.is_pressed('right'):
            os.system('clear')
            print("NAVIGATING TO MENU 2")

            sleep(1)

            on_startup = False
            menu = 2
            previous_menu

    elif menu == 2:
        if menu != previous_menu:
            os.system('clear')    
            print("Forward Kinematics and Balancing Menu")
            print("This is where gearboxes can be adjusted one at a time")
            print("Press Q,W,E,R,T,Y,A,S,D,F,G,H,etc. to move one servo at a time")
            print("Press 1,2 to enter or exit balance mode for thighs")
            print("Press 3,4 to enter or exit balance mode for waist")
            print("")
            print("PID is: (" + quickRnd(balance_pid[0]) + ", " + quickRnd(balance_pid[1]) + ", " + quickRnd(balance_pid[2]) + ")")
                    
            previous_menu = menu

        # Left leg

        if keyboard.is_pressed('q'):
            gbx["offset"][4] += speed["angle"][speed_setting]
            fwd_key(4, 'q')            
        elif keyboard.is_pressed('w'):
            gbx["offset"][4] -= speed["angle"][speed_setting]
            fwd_key(4, 'w') 
        elif keyboard.is_pressed('e'):
            gbx["offset"][6] += speed["angle"][speed_setting]
            fwd_key(6, 'e')
        elif keyboard.is_pressed('r'):
            gbx["offset"][6] -= speed["angle"][speed_setting]
            fwd_key(6, 'r')
        elif keyboard.is_pressed('t'):
            gbx["offset"][8] += speed["angle"][speed_setting]
            fwd_key(8, 't')
        elif keyboard.is_pressed('y'):
            gbx["offset"][8] -= speed["angle"][speed_setting]
            fwd_key(8, 'y')

        # Right leg

        elif keyboard.is_pressed('a'):
            gbx["offset"][3] += speed["angle"][speed_setting]
            fwd_key(3, 'a')
        elif keyboard.is_pressed('s'):
            gbx["offset"][3] -= speed["angle"][speed_setting]
            fwd_key(3, 's')
        elif keyboard.is_pressed('d'):
            gbx["offset"][5] += speed["angle"][speed_setting]
            fwd_key(5, 'd')
        elif keyboard.is_pressed('f'):
            gbx["offset"][5] -= speed["angle"][speed_setting]
            fwd_key(5, 'f')
        elif keyboard.is_pressed('g'):
            gbx["offset"][7] += speed["angle"][speed_setting]
            fwd_key(7, 'g')
        elif keyboard.is_pressed('h'):
            gbx["offset"][7] -= speed["angle"][speed_setting]
            fwd_key(7, 'h')
            
        # Hips
            
        elif keyboard.is_pressed('u'):
            gbx["angle"][1] += speed["angle"][speed_setting]
            quick_command = "1m" + str(gbx["angle"][1]) + "\n"
            ser.write(quick_command.encode("utf-8")) 
            show_key('u')           
        elif keyboard.is_pressed('i'):
            gbx["angle"][1] -= speed["angle"][speed_setting]
            quick_command = "1m" + str(gbx["angle"][1]) + "\n"
            ser.write(quick_command.encode("utf-8"))
            show_key('i')               
        elif keyboard.is_pressed('z'):
            gbx["angle"][2] += speed["angle"][speed_setting]
            quick_command = "2m" + str(gbx["angle"][2]) + "\n"
            ser.write(quick_command.encode("utf-8"))
            show_key('z')             
        elif keyboard.is_pressed('x'):
            gbx["angle"][2] -= speed["angle"][speed_setting]
            quick_command = "2m" + str(gbx["angle"][2]) + "\n"
            ser.write(quick_command.encode("utf-8"))  
            show_key('x')           
          
        # Feet  
            
        elif keyboard.is_pressed('c'):
            gbx["offset"][9] += speed["angle"][speed_setting]
            fwd_key(9, 'c')
        elif keyboard.is_pressed('v'):
            gbx["offset"][9] -= speed["angle"][speed_setting]
            fwd_key(9, 'v')
        elif keyboard.is_pressed('b'):
            gbx["offset"][10] += speed["angle"][speed_setting]
            fwd_key(10, 'b')
        elif keyboard.is_pressed('n'):
            gbx["offset"][10] -= speed["angle"][speed_setting]
            fwd_key(10, 'n')

        # Balance

        elif keyboard.is_pressed('1'):
            quick_command = "3b;4b\n" 
            gbx["balancing"][3] = True
            gbx["balancing"][4] = True
            show_key('1')
            ser.write(quick_command.encode("utf-8")) 
        elif keyboard.is_pressed('2'):
            quick_command = "3x;4x\n" 
            gbx["balancing"][3] = False
            gbx["balancing"][4] = False           
            show_key('2')
            ser.write(quick_command.encode("utf-8")) 
        elif keyboard.is_pressed('3'):
            quick_command = "0b\n" 
            gbx["balancing"][0] = True            
            show_key('3')
            ser.write(quick_command.encode("utf-8"))             
        elif keyboard.is_pressed('4'):
            quick_command = "0x\n" 
            gbx["balancing"][0] = False            
            show_key('4')
            ser.write(quick_command.encode("utf-8"))  
        elif keyboard.is_pressed('5'):
            quick_command = "3l15;3n45;4l15;4n45;0l15;0n45\n"    
            show_key('5')
            ser.write(quick_command.encode("utf-8"))   
        elif keyboard.is_pressed('6'):
            quick_command = "3l30;3n60;4l30;4n60;0l30;0n60\n"    
            show_key('6')
            ser.write(quick_command.encode("utf-8"))               
        elif keyboard.is_pressed('7'):
            quick_command = "3l40;3n80;4l40;4n80;0l40;0n80\n"    
            show_key('7')
            ser.write(quick_command.encode("utf-8")) 
        elif keyboard.is_pressed('8'):
            quick_command = "3l60;3n90;4l60;4n90;0l60;0n90\n"    
            show_key('8')
            ser.write(quick_command.encode("utf-8"))    
        elif keyboard.is_pressed('9'):
            switch_pid('up', 0)          
        elif keyboard.is_pressed('0'):
            switch_pid('down', 0)
        elif keyboard.is_pressed('o'):
            switch_pid('up', 1)          
        elif keyboard.is_pressed('p'):
            switch_pid('down', 1)            
        elif keyboard.is_pressed('l'):
            switch_pid('up', 2)          
        elif keyboard.is_pressed('m'):
            switch_pid('down', 2)    
            
        # Arrow keys    
            
        elif keyboard.is_pressed('up'):
            switch_speed('up')  
        elif keyboard.is_pressed('down'):
            switch_speed('down')  
        elif keyboard.is_pressed('o'):
            switch_speed('up', 2)
        elif keyboard.is_pressed('p'):
            switch_speed('down', 2)            
            
        elif keyboard.is_pressed('left'):
            os.system('clear')
            print("RETURNING TO MENU 1")

            sleep(1)

            on_startup = False
            menu = 1
            previous_menu = -1
