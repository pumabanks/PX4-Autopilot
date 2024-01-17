/****************************************************************************
 *
 *   Copyright (c) 2023 PX4 Development Team. All rights reserved.
 *
 * Terms and license agreement copied here
 *
 * NOTE: flying can be dangerous, please code responsibly.
 *
 ****************************************************************************/

/**
 * tell the compiler it only should compile this once
 */
#pragma once

// PX4 includes
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>

#include <uORB/uORB.h>
#include <uORB/SubscriptionInterval.hpp>
#include <uORB/topics/parameter_update.h>

using namespace time_literals;

namespace super_awesome_module
{
class SuperAwesomeModule : public ModuleBase<SuperAwesomeModule>, public ModuleParams,
	public px4::ScheduledWorkItem
{
public:
	SuperAwesomeModule();
	~SuperAwesomeModule() override = default;

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	/** @see ModuleBase */
	int print_status() override;

	bool init();

private:
	/** @see ScheduledWorkItem */
	void Run() override;

	/**
	 * Process parameter updates
	 */
	void parameters_update(bool force);

	/**
	 * For using new parameters
	 */
	bool     _enabled;
	uint32_t _update_interval_ms;

	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};

	/**
	 * Pull in access to the parameters needed
	 * for this module
	 */
	DEFINE_PARAMETERS(
		(ParamBool<px4::params::SAM_EX_ENABLE>) _param_sam_enable,
		(ParamInt<px4::params::SAM_EX_UPDATE>)  _param_sam_update_hz
	);

};

} // namespace super_awesome_module
