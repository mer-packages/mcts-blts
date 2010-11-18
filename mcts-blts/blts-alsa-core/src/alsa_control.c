/* alsa_control.c -- ALSA control IF functionality

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "alsa_util.h"
#include "alsa_control.h"
#include "alsa_ioctl.h"

static const char* element_type_to_string(snd_ctl_elem_type_t type)
{
	switch(type)
	{
	case SNDRV_CTL_ELEM_TYPE_NONE:
		return "none";
	case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
		return "boolean";
	case SNDRV_CTL_ELEM_TYPE_INTEGER:
		return "integer";
	case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
		return "enumerated";
	case SNDRV_CTL_ELEM_TYPE_BYTES:
		return "bytes";
	case SNDRV_CTL_ELEM_TYPE_IEC958:
		return "IEC958";
	case SNDRV_CTL_ELEM_TYPE_INTEGER64:
		return "integer64";
	default:
		return "UNKNOWN";
	}
}

static const char* interface_to_string(snd_ctl_elem_iface_t iface)
{
	switch(iface)
	{
	case SNDRV_CTL_ELEM_IFACE_CARD:
		return "global control";
	case SNDRV_CTL_ELEM_IFACE_HWDEP:
		return "hardware dependent device";
	case SNDRV_CTL_ELEM_IFACE_MIXER:
		return "virtual mixer device";
	case SNDRV_CTL_ELEM_IFACE_PCM:
		return "PCM device";
	case SNDRV_CTL_ELEM_IFACE_RAWMIDI:
		return "RawMidi device";
	case SNDRV_CTL_ELEM_IFACE_TIMER:
		return "timer device";
	case SNDRV_CTL_ELEM_IFACE_SEQUENCER:
		return "sequencer client";
	default:
		return "UNKNOWN";
	}
}

const char* power_state_to_string(int state)
{
	switch(state)
	{
	case SNDRV_CTL_POWER_D0:
		return "full on (SNDRV_CTL_POWER_D0)";
	case SNDRV_CTL_POWER_D1:
		return "partial on (SNDRV_CTL_POWER_D1)";
	case SNDRV_CTL_POWER_D2:
		return "partial on (SNDRV_CTL_POWER_D2)";
	case SNDRV_CTL_POWER_D3hot:
		return "off, with power (SNDRV_CTL_POWER_D3hot)";
	case SNDRV_CTL_POWER_D3cold:
		return "off, without power (SNDRV_CTL_POWER_D3cold)";
	default:
		return "UNKNOWN";
	}
}

ctl_device* control_open(int card)
{
	char filename[MAX_DEV_NAME_LEN];
	ctl_device* hw = NULL;

	if(card < 0 || card >= MAX_CARDS)
	{
		BLTS_ERROR("Invalid card index %d\n", card);
		return NULL;
	}

	hw = calloc(1, sizeof(ctl_device));
	if(!hw)
	{
		BLTS_LOGGED_PERROR("Failed to allocate ctl_device structure");
		return NULL;
	}

	sprintf(filename, "/dev/snd/controlC%i", card);

	BLTS_TRACE("Opening control %s\n", filename);
	hw->card = card;
	hw->fd = open(filename, O_RDWR);
	if(!hw->fd)
	{
		BLTS_ERROR("Failed to open device '%s'\n", filename);
		return NULL;
	}

	if(ioctl_ctl_pversion(hw, &hw->protocol))
		goto error_exit;

	BLTS_TRACE("Protocol version %d.%d.%d\n",
		SNDRV_PROTOCOL_MAJOR(hw->protocol),
		SNDRV_PROTOCOL_MINOR(hw->protocol),
		SNDRV_PROTOCOL_MICRO(hw->protocol));

	if(hw->protocol < SNDRV_PROTOCOL_VERSION(2, 0, 6))
	{
		BLTS_ERROR("Protocol version < 2.0.6 not supported\n");
		goto error_exit;
	}

	return hw;

error_exit:
	if(hw)
	{
		if(hw->fd)
			close(hw->fd);
		free(hw);
	}

	return NULL;
}

ctl_element* control_read_by_id(ctl_device* hw, unsigned int id)
{
	int err = 0;
	unsigned int t, count;
	struct snd_ctl_elem_list list;
	ctl_element* element = NULL;

	element = calloc(1, sizeof(ctl_element));
	if(!element)
	{
		BLTS_LOGGED_PERROR("Failed to allocate control element");
		goto error_exit;
	}

	/* Query number of control elements */
	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto error_exit;
	count = list.count;

	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	list.offset = 0;
	list.space = count;
	list.pids = calloc(count, sizeof(struct snd_ctl_elem_id));
	if(!list.pids)
	{
		BLTS_LOGGED_PERROR("Failed to allocate control element ids");
		goto error_exit;
	}

	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto error_exit;

	for(t = 0; t < list.used; t++)
	{
		memset(&element->info, 0, sizeof(struct snd_ctl_elem_info));
		element->info.id = list.pids[t];
		err = ioctl_ctl_elem_info(hw, &element->info);
		if(err)
			goto error_exit;

		memset(&element->value, 0, sizeof(struct snd_ctl_elem_value));
		element->value.id = list.pids[t];
		err = ioctl_ctl_elem_read(hw, &element->value);
		if(err)
			goto error_exit;

		if(element->info.id.numid == id)
		{
			free(list.pids);
			return element;
		}
	}

error_exit:
	if(element)
		free(element);
	if(list.pids)
		free(list.pids);

	return 0;
}

ctl_element* control_read(ctl_device* hw, const char* name)
{
	int err = 0;
	unsigned int t, count;
	struct snd_ctl_elem_list list;
	ctl_element* element = NULL;

	element = calloc(1, sizeof(ctl_element));
	if(!element)
	{
		BLTS_LOGGED_PERROR("Failed to allocate control element");
		goto error_exit;
	}

	/* Query number of control elements */
	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto error_exit;
	count = list.count;

	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	list.offset = 0;
	list.space = count;
	list.pids = calloc(count, sizeof(struct snd_ctl_elem_id));
	if(!list.pids)
	{
		BLTS_LOGGED_PERROR("Failed to allocate control element ids");
		goto error_exit;
	}

	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto error_exit;

	for(t = 0; t < list.used; t++)
	{
		memset(&element->info, 0, sizeof(struct snd_ctl_elem_info));
		element->info.id = list.pids[t];
		err = ioctl_ctl_elem_info(hw, &element->info);
		if(err)
			goto error_exit;

		memset(&element->value, 0, sizeof(struct snd_ctl_elem_value));
		element->value.id = list.pids[t];
		err = ioctl_ctl_elem_read(hw, &element->value);
		if(err)
			goto error_exit;

		if(!strncmp((const char*)list.pids[t].name, name, strlen(name)))
		{
			free(list.pids);
			return element;
		}
	}

error_exit:
	if(element)
		free(element);
	if(list.pids)
		free(list.pids);

	return 0;
}

static int set_check_contol_value(ctl_element* element, int chan,
	long long value)
{
	/* TODO: take "step" into account (min, max, step) */
	switch(element->info.type)
	{
	case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
		if(value > 0)
			value = 1;
	case SNDRV_CTL_ELEM_TYPE_INTEGER:
		if(value < element->info.value.integer.min)
			value = element->info.value.integer.min;
		if(value > element->info.value.integer.max)
			value = element->info.value.integer.max;
		element->value.value.integer.value[chan] = value;
		break;
	case SNDRV_CTL_ELEM_TYPE_INTEGER64:
		if(value < element->info.value.integer64.min)
			value = element->info.value.integer64.min;
		if(value > element->info.value.integer64.max)
			value = element->info.value.integer64.max;
		element->value.value.integer64.value[chan] = value;
		break;
	case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
		if(value < 0 || value > element->info.value.enumerated.items - 1)
		{
			BLTS_ERROR(
				"Cannot set enumerated control element value, out of range.\n");
			return -EINVAL;
		}
		element->value.value.enumerated.item[chan] = value;
		break;
	case SNDRV_CTL_ELEM_TYPE_BYTES:
		if(value < 0 || value > 0xFF)
		{
			BLTS_ERROR("Cannot set byte control element value, out of range.\n");
			return -EINVAL;
		}
		element->value.value.bytes.data[chan] = value;
		break;
	/* TODO: implementation for this */
	case SNDRV_CTL_ELEM_TYPE_IEC958:
	case SNDRV_CTL_ELEM_TYPE_NONE:
	default:
		BLTS_ERROR("Unsupported element type\n");
		return -EINVAL;
	}

	return 0;
}

int control_write(ctl_device* hw, ctl_element* element, long long value)
{
	unsigned int i;
	int err;

	for(i = 0; i < element->info.count; i++)
	{
		err = set_check_contol_value(element, i, value);
		if(err)
			return err;
	}

	return ioctl_ctl_elem_write(hw, &element->value);
}

int control_write_single(ctl_device* hw, ctl_element* element, int chan,
	long long value)
{
	int err;

	err = set_check_contol_value(element, chan, value);
	if(err)
		return err;

	return ioctl_ctl_elem_write(hw, &element->value);
}

int contol_value_by_name(ctl_device* hw, ctl_element* element, char *name,
	long long *value)
{
	struct snd_ctl_elem_info info;
	unsigned int t;
	int err;

	if(element->info.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED)
	{
		for(t = 0; t < element->info.value.enumerated.items; t++)
		{
			memset(&info, 0, sizeof(info));
			info.id = element->info.id;
			info.value.enumerated.item = t;
			err = ioctl_ctl_elem_info(hw, &info);
			if(err)
				return err;
			if(!strcmp(info.value.enumerated.name, name))
			{
				*value = t;
				return 0;
			}
		}
		BLTS_ERROR("Control element does not have enumerated value '%s'\n", name);
		return -EINVAL;
	}
	else
	{
		/* TODO: use real min/max values from element info */
		if(!strcmp(name, "off"))
			*value = 0;
		else if(!strcmp(name, "on"))
			*value = INT_MAX;
		else if(!strcmp(name, "max"))
			*value = INT_MAX;
		else if(!strcmp(name, "min"))
			*value = 0;
		else
		{
			BLTS_ERROR("Unrecognized control element value '%s'\n", name);
			return -EINVAL;
		}
	}

	return 0;
}

int control_print_card_info(ctl_device* hw)
{
	int err;
	snd_ctl_card_info_t info;

	err = ioctl_ctl_hw_card_info(hw, &info);
	if(err)
		return err;

	BLTS_DEBUG("Card information:\n");
	BLTS_DEBUG("\tCard number: %d\n", info.card);
	BLTS_DEBUG("\tID: %s\n", info.id);
	BLTS_DEBUG("\tDriver: %s\n", info.driver);
	BLTS_DEBUG("\tName: %s\n", info.name);
	BLTS_DEBUG("\tLong name: %s\n", info.longname);
	BLTS_DEBUG("\tMixer name: %s\n", info.mixername);
	BLTS_DEBUG("\tComponents: %s\n", info.components);

	return 0;
}

int control_print_element_value(ctl_device* hw, ctl_element* element)
{
	unsigned int i, p;
	int err = 0;

	BLTS_DEBUG("  Element %d\n", element->info.id.numid);
	BLTS_DEBUG("    name: %s\n", element->info.id.name);
	BLTS_DEBUG("    iface: %s\n", interface_to_string(element->info.id.iface));
	BLTS_DEBUG("    device: %d:%d\n",
		element->info.id.device, element->info.id.subdevice);
	BLTS_DEBUG("    type: %s\n", element_type_to_string(element->info.type));
	BLTS_DEBUG("    num values: %d\n", element->info.count);
	BLTS_DEBUG("    value(s):\n");

	for(i = 0; i < element->info.count; i++)
	{
		switch(element->info.type)
		{
		case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
			BLTS_DEBUG("      %s (on/off)\n",
				element->value.value.integer.value[i]?"on":"off");
			break;
		case SNDRV_CTL_ELEM_TYPE_INTEGER:
			BLTS_DEBUG("      %d (%d...%d, step %d)\n",
				element->value.value.integer.value[i],
				element->info.value.integer.min,
				element->info.value.integer.max,
				element->info.value.integer.step);
			break;
		case SNDRV_CTL_ELEM_TYPE_INTEGER64:
			BLTS_DEBUG("      %ld (%ld...%ld, step %d)\n",
				element->value.value.integer64.value[i],
				element->info.value.integer64.min,
				element->info.value.integer64.max,
				element->info.value.integer64.step);
			break;
		case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
			{
				BLTS_DEBUG("      %d ( ", element->value.value.enumerated.item[i]);
				for(p = 0; p < element->info.value.enumerated.items; p++)
				{
						element->info.value.enumerated.item = p;
						err = ioctl_ctl_elem_info(hw, &element->info);
						if(err)
							return err;
						BLTS_DEBUG("\"%d: %s\" ", p,
							element->info.value.enumerated.name);
				}
				BLTS_DEBUG(")\n");
			}
			break;
		/* TODO: implementation for these */
		case SNDRV_CTL_ELEM_TYPE_BYTES:
		case SNDRV_CTL_ELEM_TYPE_IEC958:
		case SNDRV_CTL_ELEM_TYPE_NONE:
		default:
			BLTS_DEBUG("      N/A\n");
			break;
		}
	}
	return 0;
}

int control_print_element_list(ctl_device* hw)
{
	int err = 0;
	unsigned int t, count;
	struct snd_ctl_elem_list list;
	ctl_element element;

	/* Query number of control elements */
	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto cleanup;
	count = list.count;

	memset(&list, 0, sizeof(struct snd_ctl_elem_list));
	list.offset = 0;
	list.space = count;
	list.pids = calloc(count, sizeof(struct snd_ctl_elem_id));
	if(!list.pids)
	{
		BLTS_LOGGED_PERROR("Failed to allocate control element ids");
		err = -ENOMEM;
		goto cleanup;
	}

	err = ioctl_ctl_elem_list(hw, &list);
	if(err)
		goto cleanup;

	BLTS_DEBUG("Control element information\n");
	BLTS_DEBUG("  Count of element ids: %d (all: %d)\n", list.used, list.count);

	for(t = 0; t < list.used; t++)
	{
		memset(&element.info, 0, sizeof(struct snd_ctl_elem_info));
		element.info.id = list.pids[t];
		err = ioctl_ctl_elem_info(hw, &element.info);
		if(err)
			goto cleanup;

		memset(&element.value, 0, sizeof(struct snd_ctl_elem_value));
		element.value.id = list.pids[t];
		err = ioctl_ctl_elem_read(hw, &element.value);
		if(err)
			goto cleanup;

		err = control_print_element_value(hw, &element);
		if(err)
			break;
	}

cleanup:
	if(list.pids)
		free(list.pids);

	return err;
}

int control_close(ctl_device* hw)
{
	if(!hw)
		return -EINVAL;

	if(hw->fd)
		close(hw->fd);

	free(hw);

	return 0;
}

int control_enumerate_devices(ctl_device* hw)
{
	int err;
	int dev;
	struct snd_hwdep_info hwdep_info;
	struct snd_pcm_info pcm_info;
	struct snd_rawmidi_info rawmidi_info;

	BLTS_DEBUG("\n");
	dev = -1;
	while(1)
	{
		err = ioctl_ctl_pcm_next_dev(hw, &dev);
		if(err)
			return err;
		if(dev == -1)
			break;

		err = ioctl_ctl_pcm_info(hw, dev, 0, 0, &pcm_info);
		if(err)
			return err;

		BLTS_DEBUG("PCM '%s':\n", pcm_info.name);
		BLTS_DEBUG("\tcard: %d\n", pcm_info.card);
		BLTS_DEBUG("\tdevice: %d\n", pcm_info.device);
		BLTS_DEBUG("\tid: %s\n", pcm_info.id);
		BLTS_DEBUG("\tsubname: %s\n", pcm_info.subname);
		BLTS_DEBUG("\tsubdevice count: %d\n", pcm_info.subdevices_count);
	}

	BLTS_DEBUG("\n");
	dev = -1;
	while(1)
	{
		err = ioctl_ctl_hwdep_next_dev(hw, &dev);
		if(err)
			break; /* hwdep is optional */
		if(dev == -1)
			break;

		err = ioctl_ctl_hwdep_info(hw, dev, &hwdep_info);
		if(err)
			return err;

		BLTS_DEBUG("HW dep '%s'\n", hwdep_info.name);
		BLTS_DEBUG("\tdevice: %d\n", hwdep_info.device);
		BLTS_DEBUG("\tcard: %d\n", hwdep_info.card);
		BLTS_DEBUG("\tid: %s\n", hwdep_info.id);
		BLTS_DEBUG("\tiface: %d\n", hwdep_info.iface);
	}

	BLTS_DEBUG("\n");
	dev = -1;
	while(1)
	{
		err = ioctl_ctl_rawmidi_next_dev(hw, &dev);
		if(err)
			break; /* rawmidi is optional */
		if(dev == -1)
			break;

		err = ioctl_ctl_rawmidi_info(hw, dev, 0, 0, &rawmidi_info);
		if(err)
			return err;

		BLTS_DEBUG("Rawmidi '%s':\n", rawmidi_info.name);
		BLTS_DEBUG("\tcard: %d\n", rawmidi_info.card);
		BLTS_DEBUG("\tdevice: %d\n", rawmidi_info.device);
		BLTS_DEBUG("\tid: %s\n", rawmidi_info.id);
		BLTS_DEBUG("\tsubname: %s\n", rawmidi_info.subname);
		BLTS_DEBUG("\tsubdevice count: %d\n", rawmidi_info.subdevices_count);
	}

	return 0;
}

