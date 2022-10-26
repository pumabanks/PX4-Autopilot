/****************************************************************************
 *
 *   Copyright (c) 2013-2020 PX4 Development Team. All rights reserved.
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
 * AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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
 * @file vtol_takeoff_safety_override_params.c
 *
 * Parameters defined by the VTOL Takeoff Safety Override task for protecting a vehicle from extra damage during a roll over on takeoff
 *
 * This detects if a roll over is immenent and disarms in order to prevent additional damage to the motors, props, or other hardware.
 *
 *
 * @author Brandon W. Banks <pumabanks@gmail.com>
 */

/**
 * Setting to enable/disable VTOL Takeoff Safety Override
 *
 * @value -1 DISABLED
 * @value 0 Check Pitch and Roll limits only
 * @value 1 Check Pitch and Roll rate limits also
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_INT32(VTOL_TSO_ENABLE, 0);

/**
 * Max allowed PITCH before aborting a takeoff
 *
 * @unit 
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_FLOAT(VTOL_TSO_P_MAX, 45.0f);

/**
 * Max allowed ROLL before aborting a takeoff
 *
 * @unit deg
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_FLOAT(VTOL_TSO_R_MAX, 45.0f);

/**
 * Max allowed PITCH RATE before aborting a takeoff
 *
 * @unit deg/s
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_FLOAT(VTOL_TSO_PR_MAX, 25.0f);

/**
 * Max allowed ROLL RATE before aborting a takeoff
 *
 * @unit deg/s
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_FLOAT(VTOL_TSO_RR_MAX, 25.0f);

/**
 * Max height to abort a takeoff (i.e if higher an abort will not be allowed)
 *
 * @unit m
 * @group VTOL Takeoff Safety Override
 */
PARAM_DEFINE_FLOAT(VTOL_TSO_H_MAX, 10.0f);
