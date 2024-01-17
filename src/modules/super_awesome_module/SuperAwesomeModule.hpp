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
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>


namespace super_awesome_module
{
class SuperAwesomeModule : public ModuleBase<SuperAwesomeModule>,
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

	bool init();

private:
	void Run() override;
};

} // namespace super_awesome_module
