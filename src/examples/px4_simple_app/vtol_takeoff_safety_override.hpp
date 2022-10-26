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
 * @file vtol_takeoff_monitoring.c
 *
 * Implementation of the VTOL Takeoff Safety Override task for protecting a vehicle from extra damage during a roll over on takeoff
 *
 * This detects if a roll over is immenent and disarms in order to prevent additional damage to the motors, props, or other hardware.
 *
 * @author Brandon W. Banks <pumabanks@gmail.com>
 */

#include <lib/mathlib/mathlib.h>
#include <lib/parameters/param.h>
#include <lib/perf/perf_counter.h>
#include <matrix/math.hpp>
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <math.h>

#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/takeoff_status.h>
#include <uORB/topics/vehicle_angular_velocity.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_control_mode.h>
#include <uORB/topics/vehicle_local_position.h>

using matrix::Eulerf;
using matrix::Quatf;

using uORB::SubscriptionData;

using namespace time_literals;

#define OVERRIDE_DISABLED (int)-1
#define OVERRIDE_ENABLED_CHECK_LIMITS (int)0
#define OVERRIDE_ENABLED_CHECK_LIMITS_AND_RATES (int)1

class VTOL_Takeoff_Safety_Override: public ModuleBase<VTOL_Takeoff_Safety_Override>, public ModuleParams, public px4::WorkItem
{
public:
	VTOL_Takeoff_Safety_Override();
	~VTOL_Takeoff_Safety_Override();

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	bool init();

private:
	bool _under_max_height;
	bool _roll_limit_exceeded;
	bool _pitch_limit_exceeded;
	bool _roll_rate_limit_exceeded;
	bool _pitch_rate_limit_exceeded;
	uint _takeoff_state;

	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};

	uORB::Subscription _vehicle_angular_velocity_sub{ORB_ID(vehicle_angular_velocity)};
	uORB::Subscription _vehicle_local_position_sub{ORB_ID(vehicle_local_position)};

	uORB::SubscriptionCallbackWorkItem _vehicle_attitude_sub{this, ORB_ID(vehicle_attitude)};
	uORB::SubscriptionCallbackWorkItem _takeoff_status_sub{this, ORB_ID(takeoff_status)};
	uORB::SubscriptionCallbackWorkItem _vehicle_control_mode_sub{this, ORB_ID(vehicle_control_mode)};

	vehicle_attitude_s _vehicle_attitude {}; /**< vehicle attitude */
	takeoff_status_s _takeoff_status {}; /**< takeoff status */
	vehicle_angular_velocity_s _vehicle_angular_velocity {};	/**< vehicle angular velocity */
	vehicle_control_mode_s _vehicle_control_mode {}; /**< vehicle control mode */
	vehicle_local_position_s _vehicle_local_position{}; /**< vehicle local position */

	perf_counter_t	_loop_perf; /**< loop performance counter */

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::VTOL_TSO_ENABLE>) _param_override_enabled,
		(ParamFloat<px4::params::VTOL_TSO_P_MAX>) _param_max_pitch,
		(ParamFloat<px4::params::VTOL_TSO_R_MAX>) _param_max_roll,
		(ParamFloat<px4::params::VTOL_TSO_PR_MAX>) _param_max_pitch_rate,
		(ParamFloat<px4::params::VTOL_TSO_RR_MAX>) _param_max_roll_rate,
		(ParamFloat<px4::params::VTOL_TSO_H_MAX>) _param_max_height
	)

	void Run() override;
	/**
	 * Update our local parameter cache.
	 */
	void parameters_update(bool force = false);

	// helper functions
	bool AreLimitsExceeded();
	void AbortTakeoff();
	void NotifyUserOfAbortedTakeoff();
};