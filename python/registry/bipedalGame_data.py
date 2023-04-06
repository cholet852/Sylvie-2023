# Bipedal robot movement command grid   
    
version = "2.00" # Program version

def menuOneText(l_leg_angles, l_leg_offsets, r_leg_angles, r_leg_offsets, l_ee_pos, r_ee_pos, sr_angles, sr_offsets, waist_angle, waist_offset, encoded_command):
    print("Current left leg angles: " + l_leg_angles[0] + " (" + l_leg_offsets[0] + "), " + l_leg_angles[1] + " (" + l_leg_offsets[1] + "), " + l_leg_angles[2] + " (" + l_leg_offsets[2] + ")")
    print("Current right leg angles: " + r_leg_angles[0] + " (" + r_leg_offsets[0] + "), " + r_leg_angles[1] + " (" + r_leg_offsets[1] + "), " + r_leg_angles[2] + " (" + r_leg_offsets[2] + ")")
    print("")
    print("Left Leg End Effector position: ", l_ee_pos[0], l_ee_pos[1])
    print("Right Leg End Effector position: ", r_ee_pos[0], r_ee_pos[1])
    print("")
    print("Current hip and foot angles: ", sr_angles[0] + " (" + sr_offsets[0] + "), " + sr_angles[1] + " (" + sr_offsets[1] + "), " + sr_angles[2] + " (" + sr_offsets[2] + "), " + sr_angles[3] + " (" + sr_offsets[3] + ")")
    print("Current waist joint angle: ", waist_angle + " (" + waist_offset + ")")
    print("Current encoded command: ", encoded_command)
    print("")
    print("Use WASD,TFGH,IJKL,ZX,CV to move the lower body.")

