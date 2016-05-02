#include "InverseKinematics.h"

bool InverseKinematics::GetInverseKinematics(std::vector<float>& cartesian, std::vector<float>& articular)
{
    //T O D O :   T H I S   I S   A   V E R Y   I M P O R T A N T   T O - D O !!!!!!!!!
    //Dimensions of the arms should be taken from the robot description (urdf file in the planning/knowledge/hardware/justina.xml)
    //Values of D1, D2, D3 and D4 correspond to Denavig-Hartenberg parameters and are given in the urdf
    //In the origin tag of each joint.
    articular.clear();
    for(int i=0; i<7;i++) articular.push_back(0);

    //In the urdf, in each joint, originZ corresponds to the 'D' params of Denavit-Hartenberg
    //and originX corresponds to 'A' params of DenavitHartenberg. originR are the alpha parameters ant originYaw are the Theta parameters
    //in URDF:
    //<joint name="la_2_joint" type="revolute"><origin xyz="0.0603 0 0" rpy="1.5708 0 0"/>
    //<joint name="la_3_joint" type="revolute"><origin xyz="0.0 0 0" rpy="1.5708 0 1.5708"/><!--Transformation from link2 to link3 when theta2 = 0 -->
    //<joint name="la_4_joint" type="revolute"><origin xyz="0 0 0.27" rpy="-1.5708 0 -1.5708"/>
    //<joint name="la_5_joint" type="revolute"><origin xyz="0.0 0 0" rpy="1.5708 0 0"/><!--Transformation from link4 to link5 when theta4 = 0 -->
    //<joint name="la_6_joint" type="revolute"><origin xyz="0 0 0.2126" rpy="-1.5708 0 0"/>
    //<joint name="la_7_joint" type="revolute"><origin xyz="0.0 0 0" rpy="1.5708 0 0"/><!--Transformation from link6 to link7 when theta6 = 0 -->
    //<joint name="la_grip_center_joint" type="fixed"><origin xyz="0 0 0.13" rpy="0 -1.5708 3.141592"/>
    float dhD[7] = {0, 0, 0.27, 0, 0.2126, 0, 0.13};
    float dhA[7] = {0.0603, 0, 0, 0, 0, 0, 0};
    float dhAlpha[7] = {1.5708, 1.5708, -1.5708, 1.5708, -1.5708, 1.5708, -1.5708};
    float dhTheta[7] = {0, 1.5708, -1.5708, 0, 0, 0, 3.141592};
    
    //Aux variables
    float x = cartesian[0];
    float y = cartesian[1];
    float z = cartesian[2];
    float roll = cartesian[3];
    float pitch = cartesian[4];
    float yaw = cartesian[5];
    float elbowAngle = cartesian[6];
    float r, alpha, beta, gamma;

    //Desired rotation matrix from base to final actuator
    //Desired orientation of the gripper is given by roll-pitch-yaw angles
    //RPY Matrix is a rotation first, of yaw degrees over Z, then, pitch degrees over CURRENT Y, and roll degrees over CURRENT x
    //RPY is also a rotation of roll over BASE X, then pitch over BASE Y, then yaw over BASE Z
    tf::Transform R07;
    tf::Quaternion q;
    q.setRPY(roll, pitch, yaw);
    R07.setIdentity();
    R07.setRotation(q);

    //First, we use the desired orientation to caculate the position of the center of the wrist.
    //i.e, which position should have the center of the wrist such that, with the desired orientation rpy,
    //the final effector had the desired position xyz
    tf::Vector3 WristPosition(0,0,0);
    WristPosition[0] = dhD[6];
    WristPosition = R07 * WristPosition;
    //Now, WristPosition has the desired position for the center of the wrist
    WristPosition[0] = x - WristPosition[0];
    WristPosition[1] = y - WristPosition[1];
    WristPosition[2] = z - WristPosition[2];
    x = WristPosition[0];
    y = WristPosition[1];
    z = WristPosition[2];

    std::cout << "InverseKinematics.->WristPos: " << WristPosition[0] << " " << WristPosition[1] << " " << WristPosition[2] << std::endl;
    std::cout << "InverseKinematics.->XYZ before correcting dhA0: " << x << " " << y << " " << z << std::endl;
    //We correct the displacement caused by the distance dhA0
    articular[0] = atan2(y, x);
    x = x - dhA[0] * cos(articular[0]);
    y = y - dhA[0] * sin(articular[0]);
    std::cout << "InverseKinematics.->XYZ after correcting dhA0: " << x << " " << y << " " << z << std::endl;
    r = sqrt(x*x + y*y + (z-dhD[0])*(z-dhD[0]));

    if(r >= (dhD[2] + dhD[4]))
    {
        std::cout << "InverseKinematics.->Cannot calculate inverse kinematics u.u Point is out of workspace." << std::endl;
        return false;
    }

    alpha = atan2((z-dhD[0]), sqrt(x*x + y*y));
    gamma = acos((-dhD[2]*dhD[2] - dhD[4]*dhD[4] + r*r)/(-2*dhD[2]*dhD[4]));
    beta = asin(dhD[4] * sin(gamma) / r);
    
    //This solution alwasy considers only the elbow-up solution. We still need to check the elbow-down solution.
    float tunningRadiusElbow = dhD[2] * sin(beta);  //Tunning radius of the elbow
    //Elbow position w.r.t. Oelbow frame
    tf::Vector3 Pelbow;
    Pelbow[0] = 0;
    Pelbow[1] = -tunningRadiusElbow * cos(elbowAngle);
    Pelbow[2] = -tunningRadiusElbow * sin(elbowAngle);

    // //Transformación del sistema sobre el que gira el codo al sistema base
    tf::Matrix3x3 oReRot;
    tf::Vector3 oReTrans;
    oReRot[0][0] = cos(articular[0]) * cos(-alpha);
    oReRot[1][0] =sin(articular[0]) * cos(-alpha);
    oReRot[2][0] =-sin(-alpha);

    oReRot[0][1] =-sin(articular[0]);
    oReRot[1][1] =cos(articular[0]);
    oReRot[2][1] =0;

    oReRot[0][2] =cos(articular[0]) * sin(-alpha);
    oReRot[1][2] =sin(articular[0]) * sin(-alpha);
    oReRot[2][2] =cos(-alpha);

    oReTrans[0] =dhD[2] * cos(beta) * cos(alpha) * cos(articular[0]);
    oReTrans[1] =dhD[2] * cos(beta) * cos(alpha) * sin(articular[0]);
    oReTrans[2] =dhD[2] * cos(beta) * sin(alpha) + dhD[0];

    tf::Transform oRe(oReRot, oReTrans);    //Homogénea del sistema Oelbow al sistema base, asumiendo que dhA0 fuera cero

    Pelbow = oRe * Pelbow; //Transformo coordenadas de posición del codo con respecto al sistema base

    articular[0] =atan2(Pelbow[1] + dhA[0]*sin(articular[0]), Pelbow[0] + dhA[0]*cos(articular[0]));
    articular[1] =atan2(Pelbow[2] - dhD[0], sqrt(Pelbow[0] * Pelbow[0] + Pelbow[1] * Pelbow[1]));
    articular[2] =0;
    articular[3] =0;
    
    //Calculation of Denavit-Hartenberg transforms
    tf::Transform R40;
    R40.setIdentity();
    for(size_t i=0; i < 4; i++)
    {
        tf::Transform temp;
        temp.setOrigin(tf::Vector3(dhA[i]*cos(articular[i]), dhA[i]*sin(articular[i]), dhD[i]));
        q.setRPY(dhAlpha[i],0,articular[i] + dhTheta[i]);
        temp.setRotation(q);
        R40 = R40 * temp;
    }
    R40 = R40.inverse();

    WristPosition = R40 * WristPosition;

    articular[2] = atan2(WristPosition[1], WristPosition[0]);
    articular[3] = M_PI / 2 - atan2(WristPosition[2], sqrt(WristPosition[0] * WristPosition[0] + WristPosition[1] * WristPosition[1]));
    if(articular[3] > M_PI) articular[3] -= 2*M_PI;
    if(articular[3] <= -M_PI) articular[3] += 2*M_PI;

    R40.setIdentity();
    for(size_t i=0; i < 4; i++)
    {
        tf::Transform temp;
        temp.setOrigin(tf::Vector3(dhA[i]*cos(articular[i]), dhA[i]*sin(articular[i]), dhD[i]));
        q.setRPY(dhAlpha[i],0,articular[i] + dhTheta[i]);
        temp.setRotation(q);
        R40 = R40 * temp;
    }
    R40 = R40.inverse();

    //A partir de aquí se calculan los ángulos de orientación
    tf::Transform tfR47;    // Matriz 4R7 de orientación de la muñeca deseada implicita
    tfR47 = R40 * R07;
    tf::Matrix3x3 R47;
    R47 = tfR47.getBasis();
    
    if((1 - fabs(R47[2][0])) < 0.0001)
    {
        //It means the second movement of the wrist, i.e. servo5 (6th servo) has an angle of zero
        //Thus, the desired orientation can be reached with an infinity number of combinations (a singularity)
        //of servos 5th and 7th, so, it is choosen to move only the last servo
        articular[4] = 0;
        articular[5] = 0;
        articular[6] = atan2(R47[1][1], R47[0][1]);
    }
    else
    {
        float phi1, phi2, theta1, theta2, psi1, psi2, cost1, cost2;
        phi1 = atan2(R47[1][0], R47[0][0]);
        theta1 = atan2(sqrt(1 - R47[2][0]*R47[2][0]), R47[2][0]);
        psi1 = atan2(R47[2][2], -R47[2][1]);
        cost1 = fabs(phi1) + fabs(theta1) + fabs(psi1);

        phi2 = atan2(-R47[1][0], -R47[0][0]);
        theta2 = atan2(-sqrt(1 - R47[2][0]*R47[2][0]), R47[2][0]);
        psi2 = atan2(-R47[2][2], R47[2][1]);
        cost2 = fabs(phi2) + fabs(theta2) + fabs(psi2);
        if(cost1 < cost2)
        {
            articular[4] = phi1;
            articular[5] = theta1;
            articular[6] = psi1;
        }
        else
        {
            articular[4] = phi2;
            articular[5] = theta2;
            articular[6] = psi2;
        }
    }
    
    std::cout <<"InverseKinematics.->Calculated angles: ";
    for(size_t i=0; i< articular.size(); i++)
        std::cout << articular[i] << "  ";
    std::cout << std::endl;

    return true;
}

bool InverseKinematics::GetInverseKinematics(float x, float y, float z, float roll, float pitch, float yaw, std::vector<float>& articular)
{
}

bool InverseKinematics::GetInverseKinematics(float x, float y, float z, std::vector<float>& articular)
{
}

bool InverseKinematics::GetInverseKinematics(geometry_msgs::Pose& cartesian, std::vector<float>& articular)
{
}

bool InverseKinematics::GetInverseKinematics(nav_msgs::Path& cartesianPath, std::vector<std_msgs::Float32MultiArray> articularPath)
{
}