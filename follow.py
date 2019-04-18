#!/usr/bin/env python
import argparse
import math
import random

import rospy
from std_msgs.msg import Float64MultiArray

import baxter_interface
from baxter_interface import CHECK_VERSION
global limb,keys


def callback(data):
    #rospy.loginfo(rospy.get_caller_id() + "I heard %s", data.data)
    print('callback!')
    global keys,limb
    if len(data.data)==7 or len(data.data)==5:
        #keys = ['left_s0', 'left_s1', 'left_e0', 'left_e1', 'left_w0', 'left_w1', 'left_w2']
        #rospy.loginfo(keys+" "+data.data+" "+limb)
        if len(data.data)==5:
            joints_name = [limb+'_s0',limb+'_s1',limb+'_e0',limb+'_e1',limb+'_w1']
            angles = dict(zip(joints_name,data.data))
            angles[limb+'_w0'] = 0
            angles[limb+'_w2'] = 0
        else:
            angles = dict(zip(keys, data.data))
        print angles
        Limb = baxter_interface.Limb(limb)
        Limb.move_to_joint_positions(angles)
    else:
        rospy.loginfo("the count of data is wrong(!=7)")
    
def follow():    
    print("follow humanPose")
    rospy.spin()

def main():
    arg_fmt = argparse.RawDescriptionHelpFormatter
    parser = argparse.ArgumentParser(formatter_class=arg_fmt,
                                     description=main.__doc__)
    parser.add_argument(
        '-l', '--limb', choices=['left', 'right'], required=True,
        help="the limb to test"
    )
    args = parser.parse_args(rospy.myargv()[1:])

    print("Initializing node... ")
    rospy.init_node('human_pose_follower')
    print("set subscriber... ")
    rospy.Subscriber("humanPose", Float64MultiArray, callback)
    print("Getting robot state... ")
    rs = baxter_interface.RobotEnable(CHECK_VERSION)
    init_state = rs.state().enabled

    def clean_shutdown():
        print("\nExiting follower...")
        if not init_state:
            print("Disabling robot...")
            rs.disable()
    rospy.on_shutdown(clean_shutdown)

    print("Enabling robot... ")
    rs.enable()
    global limb,keys
    limb = args.limb
    if limb == 'left':
        keys = ['left_s0', 'left_s1', 'left_e0', 'left_e1', 'left_w0', 'left_w1', 'left_w2']
    else:
        keys = ['right_s0', 'right_s1', 'right_e0', 'right_e1', 'right_w0', 'right_w1', 'right_w2']
    print("Begin listening")
    follow()
    print("Done.")

if __name__ == '__main__':
    main()
