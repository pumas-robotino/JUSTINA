#!/usr/bin/env python
import sys
import rospy
import Dynamixel
from std_msgs.msg import Float32MultiArray
from geometry_msgs.msg import TransformStamped
from sensor_msgs.msg import JointState
import tf

def printHelp():
    print "LEFT ARM NODE BY MARCOSOfT. Options:"
    
def main(portName1, portBaud1, portName2, portBaud2):
    print "INITIALIZING MOBILE BASE BY MARCOSOFT..."
    ###Connection with ROS
    rospy.init_node("left_arm")
    br = tf.TransformBroadcaster()
    loop = rospy.Rate(10)
    jointStates = JointState()
    jointStates.name = ["la_1_joint", "la_2_joint", "la_3_joint", "la_4_joint", "la_5_joint", "la_6_joint", "la_7_joint"]
    jointStates.position = [0, 0, 0, 0, 0, 0, 0]
    pubJointStates = rospy.Publisher("/joint_states", JointState, queue_size = 1)
    ###Communication with dynamixels:
    dynMan1 = Dynamixel.DynamixelMan(portName1, portBaud1)
    dynMan2 = Dynamixel.DynamixelMan(portName2, portBaud2)
    tempAngle = 0
    while not rospy.is_shutdown():
        bitsPerRadian0 = (4095)/((251)*(3.141592/180)) 
        pos0 = float(-(2094-dynMan1.GetPresentPosition(0))/bitsPerRadian0)

        bitsPerRadian1 = (4095)/((251)*(3.141592/180))
        pos1 = float(-(3127-dynMan1.GetPresentPosition(1))/bitsPerRadian1)
        
        bitsPerRadian2 = (4095)/((360)*(3.141592/180))
        pos2 = float(-(1798-dynMan1.GetPresentPosition(2))/bitsPerRadian2)

        bitsPerRadian3 = (4095)/((360)*(3.141592/180))
        pos3 = float(-(1997-dynMan1.GetPresentPosition(3))/bitsPerRadian3)

        bitsPerRadian4 = (4095)/((360)*(3.141592/180))
        pos4 = float(-(2050-dynMan1.GetPresentPosition(4))/bitsPerRadian4)

        bitsPerRadian5 = (4095)/((360)*(3.141592/180))
        pos5 = float((1774-dynMan1.GetPresentPosition(5))/bitsPerRadian5)

        bitsPerRadian6 = (4095)/((360)*(3.141592/180))
        pos6 = float(-(2048-dynMan1.GetPresentPosition(6))/bitsPerRadian6)
               
        bitsPerRadian7 = (1023)/((300)*(3.141592/180))
        posD21 = float((512-dynMan2.GetPresentPosition(7))/bitsPerRadian7)
        
        bitsPerRadian8 = (1023)/((300)*(3.141592/180))
        posD22 = float((512-dynMan2.GetPresentPosition(107))/bitsPerRadian8)
        
        #print "Poses: " + str(pos0) + "  " + str(pos1) + "  " + str(pos2) + "  " + str(pos3) + "  " + str(pos4) + "  " + str(pos5) + "  " + str(pos6) + "  " + str(posD21) + "  " + str(posD22)
        jointStates.header.stamp = rospy.Time.now()
        tempAngle = tempAngle + 0.1
        jointStates.position[0] = pos0
        jointStates.position[1] = pos1
        jointStates.position[2] = pos2
        jointStates.position[3] = pos3
        jointStates.position[4] = pos4
        jointStates.position[5] = pos5
        jointStates.position[6] = pos6
        pubJointStates.publish(jointStates)
        loop.sleep()

if __name__ == '__main__':
    try:
        if "--help" in sys.argv:
            printHelp()
        elif "-h" in sys.argv:
            printHelp()
        else:
            portName1 = "/dev/ttyUSB0"
            portName2 = "/dev/ttyUSB1"
            portBaud1 = 115200
            portBaud2 = 115200
            if "--port1" in sys.argv:
                portName1 = sys.argv[sys.argv.index("--port1") + 1]
            if "--port2" in sys.argv:
                portName2 = sys.argv[sys.argv.index("--port2") + 1]
            if "--baud1" in sys.argv:
                portBaud1 = int(sys.argv[sys.argv.index("--baud1") + 1])
            if "--baud2" in sys.argv:
                portBaud2 = int(sys.argv[sys.argv.index("--baud2") + 1])
            main(portName1, portBaud1, portName2, portBaud2)
    except rospy.ROSInterruptException:
        pass
