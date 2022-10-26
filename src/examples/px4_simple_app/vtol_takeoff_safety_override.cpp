/****************************************************************************
 *
 *   Copyright (c) 2012-2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/**
 * @file vtol_takeoff_safety_override.cpp
 *
 * Implementation of the VTOL Takeoff Safety Override task for protecting a vehicle from extra damage during a roll over on takeoff
 *
 * This detects if a roll over is immenent and disarms in order to prevent additional damage to the motors, props, or other hardware.
 *
 * @author Brandon W. Banks <pumabanks@gmail.com>
 */

#include "vtol_takeoff_safety_override.hpp"



/**
 * UUV pos_controller app start / stop handling function
 *
 * @ingroup apps
 */
extern "C" __EXPORT int vtol_takeoff_safety_override_main(int argc, char *argv[]);


VTOL_Takeoff_Safety_Override::VTOL_Takeoff_Safety_Override():
	ModuleParams(nullptr),
	WorkItem(MODULE_NAME, px4::wq_configurations::nav_and_controllers),
	/* performance counters */
	_loop_perf(perf_alloc(PC_ELAPSED, MODULE_NAME": cycle"))
{
}

VTOL_Takeoff_Safety_Override::~VTOL_Takeoff_Safety_Override()
{
	perf_free(_loop_perf);
}

bool VTOL_Takeoff_Safety_Override::init()
{
	if (!_vehicle_attitude_sub.registerCallback() ||
		!_takeoff_status_sub.registerCallback() ||
		!_vehicle_control_mode_sub.registerCallback())
	{
		PX4_ERR("callback registration failed");
		return false;
	}

	PX4_WARN("VTOL Takeoff Safety Override Task Started");
	return true;
}

void VTOL_Takeoff_Safety_Override::parameters_update(bool force)
{
	// check for parameter updates
	if (_parameter_update_sub.updated() || force) {
		// clear update
		parameter_update_s pupdate;
		_parameter_update_sub.copy(&pupdate);

		// update parameters from storage
		updateParams();
	}
}

void VTOL_Takeoff_Safety_Override::Run()
{
	if (should_exit()) {
		_vehicle_attitude_sub.unregisterCallback();
		_takeoff_status_sub.unregisterCallback();
		_vehicle_control_mode_sub.unregisterCallback();
		exit_and_cleanup();
		return;
	}

	parameters_update();

	perf_begin(_loop_perf);

	_takeoff_status_sub.update(&_takeoff_status);
	_vehicle_control_mode_sub.update(&_vehicle_control_mode);

	// only monitor if enabled and trying to takeoff
	if(_param_override_enabled.get() >= OVERRIDE_ENABLED_CHECK_LIMITS && _takeoff_status.takeoff_state > takeoff_status_s::TAKEOFF_STATE_SPOOLUP)
	{
		// check if we are still under the max height (relative altitude in meters)
		if (_vehicle_local_position_sub.update(&_vehicle_local_position))
		{
			_under_max_height = -_vehicle_local_position.z <= _param_max_height.get();
		}

		// monitor for immenent rollover if under the max height
		if(_under_max_height)
		{
			if(_vehicle_attitude_sub.update(&_vehicle_attitude))
			{
				// check if beyond limits
				Eulerf eulerAngles(Quatf(_vehicle_attitude.q));

				_roll_limit_exceeded = fabsf(math::degrees(eulerAngles.phi())) > _param_max_roll.get();
				_pitch_limit_exceeded = fabsf(math::degrees(eulerAngles.theta())) > _param_max_pitch.get();
				
				if(_param_override_enabled.get() >= OVERRIDE_ENABLED_CHECK_LIMITS_AND_RATES)
				{
					_vehicle_angular_velocity_sub.update(&_vehicle_angular_velocity);

					// check if rates are beyond limits
					_roll_rate_limit_exceeded = fabsf(math::degrees(_vehicle_angular_velocity.xyz[0])) > _param_max_roll_rate.get();
					_pitch_rate_limit_exceeded =  fabsf(math::degrees(_vehicle_angular_velocity.xyz[1])) > _param_max_pitch_rate.get();
				}
				
				if(AreLimitsExceeded())
				{
					AbortTakeoff();
				}
			}
		} 
	}

	perf_end(_loop_perf);
}

bool VTOL_Takeoff_Safety_Override::AreLimitsExceeded()
{
	return _roll_limit_exceeded || _pitch_limit_exceeded || _roll_rate_limit_exceeded || _pitch_rate_limit_exceeded;
}

void VTOL_Takeoff_Safety_Override::AbortTakeoff()
{
	// handle abort from takeoff here
	SendDisarmCommand();
	NotifyUserOfAbortedTakeoff();
}

void VTOL_Takeoff_Safety_Override::NotifyUserOfAbortedTakeoff()
{
	if(_roll_limit_exceeded)
	{
		const char* message = "Abort Takeoff Required - roll limit exceeded!";
		mavlink_log_critical(&_mavlink_log_pub, "%s", message);
		events::send(events::ID("abort_takeoff_required_roll_limit_exceeded"), events::Log::Critical, message);
	}

	if(_pitch_limit_exceeded)
	{
		const char* message = "Abort Takeoff Required - pitch limit exceeded!";
		mavlink_log_critical(&_mavlink_log_pub, "%s", message);
		events::send(events::ID("abort_takeoff_required_pitch_limit_exceeded"), events::Log::Critical, message);
	}

	if(_roll_rate_limit_exceeded)
	{
		const char* message = "Abort Takeoff Required - roll rate limit exceeded!";
		mavlink_log_critical(&_mavlink_log_pub, "%s", message);
		events::send(events::ID("abort_takeoff_required_roll_rate_limit_exceeded"), events::Log::Critical, message);
	}

	if(_pitch_rate_limit_exceeded)
	{
		const char* message = "Abort Takeoff Required - pitch rate limit exceeded!";
		mavlink_log_critical(&_mavlink_log_pub, "%s", message);
		events::send(events::ID("abort_takeoff_required_pitch_rate_limit_exceeded"), events::Log::Critical, message);
	}
}

bool VTOL_Takeoff_Safety_Override::SendDisarmCommand()
{
	// create command to disarm vehicle
	vehicle_command_s disarm_command{};
	disarm_command.timestamp = hrt_absolute_time();
	disarm_command.command = vehicle_command_s::VEHICLE_CMD_COMPONENT_ARM_DISARM;
	disarm_command.param1 = vehicle_command_s::ARMING_ACTION_DISARM;
	disarm_command.param2 = FORCE_DISARM_OVERRIDE;

	// register command with current vehicle
	_vehicle_status_sub.update(&_vehicle_status);
	disarm_command.source_system = _vehicle_status.system_id;
	disarm_command.target_system = _vehicle_status.system_id;
	disarm_command.source_component = _vehicle_status.component_id;
	disarm_command.target_component = _vehicle_status.component_id;

	return _vehicle_command_pub.publish(disarm_command);
}

int VTOL_Takeoff_Safety_Override::task_spawn(int argc, char *argv[])
{
	VTOL_Takeoff_Safety_Override *instance = new VTOL_Takeoff_Safety_Override();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

int VTOL_Takeoff_Safety_Override::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}


int VTOL_Takeoff_Safety_Override::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Tbd
### Implementation
Tbd
### Examples
CLI usage example:
$ vtol_takeoff_safety_override start
$ vtol_takeoff_safety_override status
$ vtol_takeoff_safety_override stop
)DESCR_STR");

    PRINT_MODULE_USAGE_NAME("vtol_takeoff_safety_override", "controller");
    PRINT_MODULE_USAGE_COMMAND("start")
    PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

    return 0;
}

int vtol_takeoff_safety_override_main(int argc, char *argv[])
{
    return VTOL_Takeoff_Safety_Override::main(argc, argv);
}