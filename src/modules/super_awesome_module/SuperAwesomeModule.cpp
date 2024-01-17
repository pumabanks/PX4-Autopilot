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
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
{
}

/**
 * Module initialization, returns true if successful
 */
bool SuperAwesomeModule::init()
{
	ScheduleOnInterval(100_ms); // 10 Hz
	return true;
}

/**
 * This is where we will add our custom functionality
 * Remember this function should get called at 10Hz
 * because that is the schedule we set for it
 */
void SuperAwesomeModule::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
	}

	// TO-DO: Insert custom code here
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
