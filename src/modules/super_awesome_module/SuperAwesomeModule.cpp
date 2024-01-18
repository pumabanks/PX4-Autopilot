/****************************************************************************
 *
 *   Copyright (c) 2023 PX4 Development Team. All rights reserved.
 *
 * Terms and license agreement copied here
 *
 * NOTE: flying can be dangerous, please code responsibly.
 *
 ****************************************************************************/


#include "SuperAwesomeModule.hpp"

using namespace time_literals;
using namespace matrix;
namespace super_awesome_module
{
/**
 * Class constructor
 */
SuperAwesomeModule::SuperAwesomeModule() :
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default),
	_enabled(true),
	_update_interval_ms(100_ms)
{
}

/**
 * Module initialization, returns true if successful
 */
bool SuperAwesomeModule::init()
{
	parameters_update(true);
	ScheduleOnInterval(_update_interval_ms);
	return _enabled;
}

/**
 * Process parameter updates
 */
void SuperAwesomeModule::parameters_update(bool force)
{
	if (_parameter_update_sub.updated() || force) {

		// clear update flag
		parameter_update_s update{};
		_parameter_update_sub.copy(&update);

		// update parameters from storage
		updateParams();

		// update module parameters
		_enabled = _param_sam_enable.get();
		_update_interval_ms = 1000_ms / _param_sam_update_hz.get();
	}
}

/**
 * This is where we will add our custom functionality
 */
void SuperAwesomeModule::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
	}

	hrt_abstime now = hrt_absolute_time();

	/**
	 * Send a message every 5 seconds
	 */
	if (now - _last_periodic_message > 5_s) {

		// ex: calculate delta time in micro-seconds (us)
		hrt_abstime dt_us = now - _last_periodic_message;

		// ex: convert to seconds
		float dt_s = (now - _last_periodic_message) / 1e6;

		// always save timestamp for the last message sent
		_last_periodic_message = now;

		// send out a message
		PX4_INFO("Periodic message (1/2): last call was %llu us ago (%.1f seconds)", dt_us, (double)(dt_s));
		PX4_WARN("Periodic message (2/2): CPU load: %.2f and RAM usage: %.2f", (double)_cpuload.load, (double)_cpuload.ram_usage);
	}

	/**
	 * if the cpuload topic has new data published
	 * then process the new data
	 *
	 * this example simply warns the user if the
	 * cpu load is too high
	 *
	 * Note: this is only marginally helpful, will
	 *  expand or remove in future examples
	 *
	 */
	if (_cpuload_sub.update(&_cpuload)) {

		// check threshold
		bool cpu_high = _cpuload.load > super_awesome_topic_s::HIGH_CPU_LOAD_WARNING;

		// check for time elapsed between messages
		bool time_for_message = now - _last_warning_message > super_awesome_topic_s::WARN_MESSAGE_INTERVAL_US;

		if (cpu_high && time_for_message) {

			_last_warning_message = now;
			PX4_WARN("HIGH CPU LOAD");
		}
	}
}

/**
 * CLI status feedback
 *  - called when px4 shell command "super_awesome_module status" is used
 */
int SuperAwesomeModule::print_status()
{
	uint32_t freq = 1000_ms / _update_interval_ms;

	PX4_INFO("Running at %u Hz", freq);

	return PX4_OK;
}

/**
 * Create the task for this module
 */
int SuperAwesomeModule::task_spawn(int argc, char *argv[])
{
	SuperAwesomeModule *instance = new SuperAwesomeModule();

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

/**
 * Allows the addition and processing of cli arguments
 */
int SuperAwesomeModule::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

/**
 * Default help message for cli when no or invalid arguments are given
 */
int SuperAwesomeModule::print_usage(const char *reason)
{
	if (reason) {
		PX4_ERR("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Super Awesome Module example.
)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("super_awesome_module", "example");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();
	return 0;
}

/**
 * Make this module available for the PX4 stack
 */
extern "C" __EXPORT int super_awesome_module_main(int argc, char *argv[])
{
	return SuperAwesomeModule::main(argc, argv);
}

} // namespace super_awesome_module
