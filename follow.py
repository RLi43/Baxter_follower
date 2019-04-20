#!/usr/bin/env python
import argparse
import math
import random

import rospy
from std_msgs.msg import Float64

import baxter_interface
from baxter_interface import CHECK_VERSION


keys_left = ['left_s0', 'left_s1', 'left_e0', 'left_e1', 'left_w0', 'left_w1', 'left_w2']
keys_right = ['right_s0', 'right_s1', 'right_e0', 'right_e1', 'right_w0', 'right_w1', 'right_w2']

angles ={'l':[0,0,0,0,0,0,0],'r':[0,0,0,0,0,0,0]}

# def callback(data):
#     print 'callback!',data.data
#     da = list(data.data)
#     if len(data.data)==14 or len(data.data)==10:
#         if len(data.data)==10:
#             data_l = da[0:4]+[0]+da[4:5]+[0]
#             data_r = da[5:9]+[0]+da[9:10]+[0]
#         else:
#             data_l = da[0:6]
#             data_r = da[7:14]
#         angle_l = dict(zip(keys_left,data_l))
#         angle_r = dict(zip(keys_right,data_r))
#         print angle_l,angle_r
#         global ll,rl
#         ll.move_to_joint_positions(angle_l)
#         rl.move_to_joint_positions(angle_r)
#     else:
#         rospy.loginfo("the count of data is wrong(!=7)")

def cb_ls0(data):
    #print 'ls0:',data.data,' '
    angles['l'][0]=data.data
def cb_ls1(data):
    #print 'ls1:',data.data,' '
    angles['l'][1]=data.data
def cb_le0(data):
    #print 'le0:',data.data,' '
    angles['l'][2]=data.data
def cb_le1(data):
    #print 'le1:',data.data,' '
    angles['l'][3]=data.data
def cb_lw1(data):
    #print 'lw1:',data.data,' '
    angles['l'][5]=data.data

def cb_rs0(data):
    #print 'rs0:',data.data,' '
    angles['r'][0]=data.data
def cb_rs1(data):
    #print 'rs1:',data.data,' '
    angles['r'][1]=data.data    
def cb_re0(data):
    #print 're0:',data.data,' '
    angles['r'][2]=data.data
def cb_re1(data):
    #print 're1:',data.data,' '
    angles['r'][3]=data.data
def cb_rw1(data):
    #print 'rw1:',data.data,' '
    angles['r'][5]=data.data


def main():

    print("Initializing node... ")
    rospy.init_node('human_pose_follower')
    rospy.Subscriber("humanPose/Left/s0", Float64, cb_ls0)
    rospy.Subscriber("humanPose/Left/s1", Float64, cb_ls1)
    rospy.Subscriber("humanPose/Left/e0", Float64, cb_le0)
    rospy.Subscriber("humanPose/Left/e1", Float64, cb_le1)
    rospy.Subscriber("humanPose/Left/w1", Float64, cb_lw1)
    rospy.Subscriber("humanPose/Right/s0", Float64, cb_rs0)
    rospy.Subscriber("humanPose/Right/s1", Float64, cb_rs1)
    rospy.Subscriber("humanPose/Right/e0", Float64, cb_re0)
    rospy.Subscriber("humanPose/Right/e1", Float64, cb_re1)
    rospy.Subscriber("humanPose/Right/w1", Float64, cb_rw1)
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
    ll = baxter_interface.Limb('left')
    rl = baxter_interface.Limb('right')
    ll.set_joint_position_speed(0.7)
    rl.set_joint_position_speed(0.7)
    # ll.move_to_joint_positions({"left_w1":0})
    # rl.move_to_joint_positions({"right_w1":0})
    print("Begin listening")
    i = 1
    while not rospy.is_shutdown():
        print i,' move!'
        i = i+1
        left = dict(zip(keys_left,angles['l']))
        right = dict(zip(keys_right,angles['r']))
        ll.set_joint_position_speed(0.7)
        rl.set_joint_position_speed(0.7)
        ll.move_to_joint_positions(left,timeout=0.25,threshold = 0.2)
        rl.move_to_joint_positions(right,timeout=0.25,threshold = 0.2)
        rate.sleep()
    print("Done.")

if __name__ == '__main__':
    main()
