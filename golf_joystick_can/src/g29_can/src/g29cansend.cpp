// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/u_int8.hpp"
#include "sensor_msgs/msg/joy.hpp"

#include "can_msgs/msg/frame.hpp"
can_msgs::msg::Frame CanMsg;
can_msgs::msg::Frame CanMsg2;
can_msgs::msg::Frame CanMsgAcc;
uint8_t gear = 1;
float Steering_joy,Steering_set;
using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
  : Node("g29_can")
  {
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
              "joy", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
              
    frames_pub_ = this->create_publisher<can_msgs::msg::Frame>("/to_can_bus", 10);
    
    using namespace std::chrono_literals;

    timer_ = this->create_wall_timer(
      10ms, std::bind(&MinimalSubscriber::timer_callback, this));
  }

private:
    void topic_callback(const sensor_msgs::msg::Joy::SharedPtr msg) const
  {
    //RCLCPP_INFO(this->get_logger(), "I heard: '%f'", msg->axes[0]);
    static int longmode = 1;
      float Acc = 0;
      float Set_Throttle=0;
      float SetBrakePos = 0;
      CanMsg.id = 0x401;
      CanMsg.dlc = 8;
      if(longmode == 1)
      {
	      Set_Throttle = (-(msg->axes[5] - 1.0)) < 1.0 ? 0.0 : -msg->axes[5] * 100.0;
	      CanMsg.data[0] = ((uint8_t*)&Set_Throttle)[0];
	      CanMsg.data[1] = ((uint8_t*)&Set_Throttle)[1];
	      CanMsg.data[2] = ((uint8_t*)&Set_Throttle)[2];
	      CanMsg.data[3] = ((uint8_t*)&Set_Throttle)[3];
	      SetBrakePos = -(msg->axes[2] - 1.0) * 50.0;
	      CanMsg.data[4] = ((uint8_t*)&SetBrakePos)[0];
	      CanMsg.data[5] = ((uint8_t*)&SetBrakePos)[1];
	      CanMsg.data[6] = ((uint8_t*)&SetBrakePos)[2];
	      CanMsg.data[7] = ((uint8_t*)&SetBrakePos)[3];
	      }
      else
      {
	      	for(int i = 0; i<8;i++)
	      	{
	      	 CanMsg.data[i] = 0;
	      	}
	      	
	      	
	      	if(-(msg->axes[2] - 1.0) > 0)
	      	{
	      	 Acc = (msg->axes[2] - 1.0) * 2.5;
	      	}
	      	else{
	      	Acc = -(msg->axes[5] - 1.0);
	      	}
	      	
      }
    
    CanMsgAcc.id = 0x400;
    CanMsgAcc.dlc = 8;
    CanMsgAcc.data[0] = ((uint8_t*)&Acc)[0];
    CanMsgAcc.data[1] = ((uint8_t*)&Acc)[1];
    CanMsgAcc.data[2] = ((uint8_t*)&Acc)[2];
    CanMsgAcc.data[3] = ((uint8_t*)&Acc)[3];
    
    
    
    CanMsg2.id = 0x402;
    CanMsg2.dlc = 8;
    
     if(msg->buttons[4] == 1)
     CanMsg2.data[0] = 1;
     else if(msg->buttons[5] == 1)
     CanMsg2.data[0] = 2;
     else if(msg->buttons[8] == 1)
     CanMsg2.data[0] = 3;
     else
     CanMsg2.data[0] = 0;



	if(msg->buttons[2] == 1)
	gear = 1;
	else if(msg->buttons[0] == 1)
	gear = 4;
	else if(msg->buttons[1] == 1)
	gear = 2;
	else if(msg->buttons[3] == 1)
	gear = 3;
	CanMsg2.data[3] = gear;
	
	
	CanMsg2.data[4] = msg->buttons[10];
	
	
	static int oldbuttonval = 0;
	if(msg->buttons[9] == 1 && oldbuttonval == 0)
	{
	  longmode = longmode == 1 ? 0 : 1;
	}
	oldbuttonval = msg->buttons[9] ;
	CanMsg2.data[7] = longmode;

	RCLCPP_INFO(this->get_logger(), "%d",longmode);
	
	Steering_joy = msg->axes[0] * -470.0;
	
 
 
    
  }
  void timer_callback()
  {
  	Steering_set = 0.97 * Steering_set + 0.03 * Steering_joy;
  	can_msgs::msg::Frame CanMsg3;
	CanMsg3.id = 0x403;
    	CanMsg3.dlc = 8;
    	CanMsg3.data[0] = ((uint8_t*)&Steering_set)[0];
 	CanMsg3.data[1] = ((uint8_t*)&Steering_set)[1];
      	CanMsg3.data[2] = ((uint8_t*)&Steering_set)[2];
      	CanMsg3.data[3] = ((uint8_t*)&Steering_set)[3];
  	
  	frames_pub_->publish(CanMsg);
  	frames_pub_->publish(CanMsg2);
  	frames_pub_->publish(CanMsg3);
  	frames_pub_->publish(CanMsgAcc);
  	
     RCLCPP_INFO(this->get_logger(), "Set steering: '%f'", Steering_set);


    
     //RCLCPP_INFO(this->get_logger(), "long mode: '%d'",longmode);
  }
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
  rclcpp::Publisher<can_msgs::msg::Frame>::SharedPtr frames_pub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
