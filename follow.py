#!/usr/bin/env python
import argparse
import math
import random

import rospy
from std_msgs.msg import Float64

import baxter_interface
from baxter_interface import CHECK_VERSION


angles = [0]*7

def cb_s0(data):
    angles[0]=data.data
def cb_s1(data):
    angles[1]=data.data
def cb_e0(data):
    angles[2]=data.data
def cb_e1(data):
    angles[3]=data.data
def cb_w1(data):
    angles[5]=data.data

def main():
    arg_fmt = argparse.RawDescriptionHelpFormatter
    parser = argparse.ArgumentParser(formatter_class=arg_fmt,
                                     description=main.__doc__)
    required = parser.add_argument_group('required arguments')
    required.add_argument(
        "-l", "--limb", required=True, choices=['left', 'right'],
        help="specify the limb (the control limb)"
    )
    args = parser.parse_args(rospy.myargv()[1:])

    name = args.limb
    print("Initializing node... ")
    rospy.init_node('human_pose_follower'+name)
    rospy.Subscriber("humanPose/"+name+"/s0", Float64, cb_s0)
    rospy.Subscriber("humanPose/"+name+"/s1", Float64, cb_s1)
    rospy.Subscriber("humanPose/"+name+"/e0", Float64, cb_e0)
    rospy.Subscriber("humanPose/"+name+"/e1", Float64, cb_e1)
    rospy.Subscriber("humanPose/"+name+"/w1", Float64, cb_w1)
    keys = [name+'_s0', name+'_s1', name+'_e0', name+'_e1', name+'_w0', name+'_w1', name+'_w2']

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
    rate = rospy.Rate(20)
    li = baxter_interface.Limb(name)
    li.set_joint_position_speed(0.8)
    print("Begin listening")
    while not rospy.is_shutdown():
        angle = dict(zip(keys,angles))
        #li.set_joint_position_speed(0.8)
        li.move_to_joint_positions(angle,timeout=0.3,threshold=0.2)
        rate.sleep()
    print("Done.")

if __name__ == '__main__':
    main()
