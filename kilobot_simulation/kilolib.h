#pragma once
#include "robot.h"
#include "ir_channel.h"
#undef RGB

#define ir 1
#define NORMAL 1
#define tolerance 60

typedef double distance_measurement_t;

//communication data struct without distance it should be 9 bytes max
struct message_t {
	unsigned char type = 0;
	unsigned char data[9];
	unsigned char crc;
};

class kilobot : public robot
{
public:
	bool left_ready = false;
	bool right_ready = false;
	int kilo_turn_right = 50;
	int kilo_straight_left = 50;
	int kilo_straight_right = 50;
	int kilo_turn_left = 50;
	int turn_right=0;
	int turn_left=0;

	double distance_measurement;
	bool message_sent = false;

	struct rgb { double red, green, blue; };

	rgb RGB(double r, double g, double b)
	{
		rgb c;
		c.red = r;
		c.green = g;
		c.blue = b;
		return c;
	}

	unsigned char message_crc(message_t *m)
	{
		int crc = 0;
		for (int i = 0;i < 9;i++)
		{
			crc += m->data[i];
		}
		return crc % 256;
	}

	void set_color(rgb c)
	{
		color[0] = c.red;
		color[1] = c.green;
		color[2] = c.blue;
	}

	virtual void setup()=0;

	void robot::init()
	{
		battery = radius * 60 * 60 * 2 + gauss_rand(rand()) * 60 * 60;
		setup();
	}

	virtual void loop() = 0;
	virtual void message_rx(message_t *message, distance_measurement_t *distance_measurement) = 0;

	void robot::controller()
	{
		if (message_sent)
		{
			tx_request = 0;
			message_sent = false;
			message_tx_success();
		}
		this->loop();
		motor_command = 4;
		if (right_ready && turn_right == kilo_turn_right)
		{
			motor_command -= 2;
		}
		else right_ready = false;
		if (left_ready && turn_left == kilo_turn_left)
		{
			motor_command -= 1;
		}
		else left_ready = false;
		if (message_tx())
			tx_request = ir;
		else
			tx_request = 0;
	}

	void kilo_init()
	{

	}

	void spinup_motors()
	{
		left_ready = true;
		right_ready = true;
	}

	void set_motors(char l, char r)
	{
		turn_left = l;
		turn_right = r;
	}

	void delay(int i)
	{

	}

	double robot::comm_out_criteria(double x, double y, int sd) //stardard circular transmission area
	{
		if (sd) return 0; // it's more than 10 cm away
		double d = distance(x,y,pos[0],pos[1]);
		if (d < 12 * radius)
			return d;
		return 0;
	}

	bool robot::comm_in_criteria(double x, double y, double d, void *cd) 
	{
		distance_measurement = d;
		message_rx((message_t *)cd, &distance_measurement);
		return true;
	}

	unsigned char estimate_distance(distance_measurement_t *d)
	{
		if (*d < 255)
			return (unsigned char)*d;
		else
			return 255;
	}

	void robot::sensing(int features, int type[], int x[], int y[], int value[])
	{
	}

	virtual void message_tx_success() =0;
	virtual message_t *message_tx() =0;

	void *robot::get_message()
	{
		void *m = this->message_tx();
		this->message_tx_success();
		return m;
	}

	void robot::received()
	{
		message_sent = true;
	}

	char *robot::get_debug_info(char *buffer, char *rt)
	{
		return buffer;
	}
};