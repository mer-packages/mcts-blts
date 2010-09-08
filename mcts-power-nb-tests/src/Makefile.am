# Copyright (C) 2010 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

#AUTOMAKE_OPTIONS=foreign
power_PROGRAMS = chk_cpufreq_perf chk_cpuidle chk_cpufreq_ondemand chk_deepest_cstat check_pm_qos check_tickless_busy 
LDFLAGS = -I./lib -g -Wall -lpthread
LDADD = sysfs.o

power_SCRIPTS = pm_acpi_acadapter_001.sh pm_acpi_thermal_001.sh pm_gfx_dpms_002.sh pm_nbusb_autosusp_001.sh pm_qos_cpudma_001.sh pm_qos_cpudma_005.sh pm_acpi_battery_001.sh pm_acpi_video_001.sh pm_gfx_port_002.sh pm_nbusb_autosusp_002.sh pm_qos_cpudma_002.sh pm_tickless_enabled.sh pm_acpi_button_001.sh pm_audio_001.sh pm_nbusb_autoenable_001.sh pm_nbusb_autosusp_003.sh pm_qos_cpudma_003.sh pm_tickless_sce_busy.sh pm_acpi_fan_001.sh pm_nbusb_autoenable_002.sh pm_nbusb_autosusp_004.sh pm_qos_cpudma_004.sh pm_tickless_sce_idle.sh check_timer_interrupt.py  check_usb_autosusp.py  check_usb_autosusp_working.py  check_usb_susp_enabling.py  usb.py pm_gfx_dpms_001.sh functions

powerdir = /opt/$(PACKAGE)
EXTRA_DIST = $(power_SCRIPTS) $(power_PROGRAMS)


