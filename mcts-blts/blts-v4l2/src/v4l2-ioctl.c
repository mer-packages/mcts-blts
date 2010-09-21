/* v4l2-ioctl.c -- v4l2 core ioctls

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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <blts_log.h>
#include "v4l2-ioctl.h"
#include "v4l2-profiler.h"
#define X 1
#define CALL_IOCTL(a, b)						\
	ioctl_profile(#b);						\
	if(ioctl(a, b) < 0)						\
	{								\
		err = errno;						\
		BLTS_TRACE(#b" failed - %s (%d)\n", strerror(err), err); \
		err = -1;						\
	}

#define CALL_IOCTL2(a, b, c)						\
	ioctl_profile(#b);						\
	if(ioctl(a, b, c) < 0)						\
	{								\
		err = errno;						\
		BLTS_TRACE(#b" failed - %s (%d)\n", strerror(err), err); \
		err = -1;						\
	}

int ioctl_VIDIOC_CROPCAP(int fd, struct v4l2_cropcap *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_CROPCAP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_CROPCAP, argp);
	return err;
}


int ioctl_VIDIOC_DBG_G_REGISTER(int fd, struct v4l2_dbg_register *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_DBG_G_REGISTER, argp);
		}
	   	while (-1 == err && EINTR == errno);
	   	return err;
	}

	CALL_IOCTL2(fd, VIDIOC_DBG_G_REGISTER, argp);
	return err;
}

int ioctl_VIDIOC_DBG_S_REGISTER(int fd, struct v4l2_dbg_register *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_DBG_S_REGISTER, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_DBG_S_REGISTER, argp);
	return err;
}

int ioctl_VIDIOC_ENCODER_CMD(int fd, struct v4l2_encoder_cmd *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENCODER_CMD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENCODER_CMD, argp);
	return err;
}

int ioctl_VIDIOC_TRY_ENCODER_CMD(int fd, struct v4l2_encoder_cmd *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_TRY_ENCODER_CMD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_TRY_ENCODER_CMD, argp);
	return err;
}

int ioctl_VIDIOC_ENUMAUDIO(int fd, struct v4l2_audio *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUMAUDIO, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUMAUDIO, argp);
	return err;
}

int ioctl_VIDIOC_ENUMAUDOUT(int fd, struct v4l2_audioout *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUMAUDOUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUMAUDOUT, argp);
	return err;
}

int ioctl_VIDIOC_ENUM_FMT(int fd, struct v4l2_fmtdesc *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUM_FMT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUM_FMT, argp);
	return err;
}

int ioctl_VIDIOC_ENUM_FRAMESIZES(int fd, struct v4l2_frmsizeenum *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUM_FRAMESIZES, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUM_FRAMESIZES, argp);
	return err;
}

int ioctl_VIDIOC_ENUM_FRAMEINTERVALS(int fd, struct v4l2_frmivalenum *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUM_FRAMEINTERVALS, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUM_FRAMEINTERVALS, argp);
	return err;
}

int ioctl_VIDIOC_ENUMINPUT(int fd, struct v4l2_input *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUMINPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUMINPUT, argp);
	return err;
}

int ioctl_VIDIOC_ENUMOUTPUT(int fd, struct v4l2_output *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUMOUTPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUMOUTPUT, argp);
	return err;
}

int ioctl_VIDIOC_ENUMSTD(int fd, struct v4l2_standard *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_ENUMSTD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_ENUMSTD, argp);
	return err;
}

int ioctl_VIDIOC_G_AUDIO(int fd, struct v4l2_audio *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_AUDIO, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_AUDIO, argp);
	return err;
}

int ioctl_VIDIOC_S_AUDIO(int fd, struct v4l2_audio *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_AUDIO, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_AUDIO, argp);
	return err;
}

int ioctl_VIDIOC_G_AUDOUT(int fd, struct v4l2_audioout *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_AUDOUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_AUDOUT, argp);
	return err;
}

int ioctl_VIDIOC_S_AUDOUT(int fd, struct v4l2_audioout *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_AUDOUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_AUDOUT, argp);
	return err;
}

int ioctl_VIDIOC_DBG_G_CHIP_IDENT(int fd, struct v4l2_dbg_chip_ident *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_DBG_G_CHIP_IDENT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_DBG_G_CHIP_IDENT, argp);
	return err;
}

int ioctl_VIDIOC_G_CROP(int fd, struct v4l2_crop *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_CROP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_CROP, argp);
	return err;
}

int ioctl_VIDIOC_S_CROP(int fd, struct v4l2_crop *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_CROP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_CROP, argp);
	return err;
}

int ioctl_VIDIOC_G_CTRL(int fd, struct v4l2_control *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_CTRL, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_CTRL, argp);
	return err;
}

int ioctl_VIDIOC_S_CTRL(int fd, struct v4l2_control *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_CTRL, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_CTRL, argp);
	return err;
}

int ioctl_VIDIOC_G_ENC_INDEX(int fd, struct v4l2_enc_idx *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_ENC_INDEX, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_ENC_INDEX, argp);
	return err;
}

int ioctl_VIDIOC_G_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_EXT_CTRLS, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_EXT_CTRLS, argp);
	return err;
}

int ioctl_VIDIOC_S_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_EXT_CTRLS, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_EXT_CTRLS, argp);
	return err;
}

int ioctl_VIDIOC_TRY_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_TRY_EXT_CTRLS, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_TRY_EXT_CTRLS, argp);
	return err;
}

int ioctl_VIDIOC_G_FBUF(int fd, struct v4l2_framebuffer *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_FBUF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_FBUF, argp);
	return err;
}

int ioctl_VIDIOC_S_FBUF(int fd, struct v4l2_framebuffer *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_FBUF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_FBUF, argp);
	return err;
}

int ioctl_VIDIOC_G_FMT(int fd, struct v4l2_format *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_FMT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_FMT, argp);
	return err;
}

int ioctl_VIDIOC_S_FMT(int fd, struct v4l2_format *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_FMT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_FMT, argp);
	return err;
}

int ioctl_VIDIOC_TRY_FMT(int fd, struct v4l2_format *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_TRY_FMT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_TRY_FMT, argp);
	return err;
}

int ioctl_VIDIOC_G_FREQUENCY(int fd, struct v4l2_frequency *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_FREQUENCY, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_FREQUENCY, argp);
	return err;
}

int ioctl_VIDIOC_S_FREQUENCY(int fd, struct v4l2_frequency *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_FREQUENCY, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_FREQUENCY, argp);
	return err;
}

int ioctl_VIDIOC_G_INPUT(int fd, int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_INPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_INPUT, argp);
	return err;
}

int ioctl_VIDIOC_S_INPUT(int fd, int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_INPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_INPUT, argp);
	return err;
}

int ioctl_VIDIOC_G_JPEGCOMP(int fd, struct v4l2_jpegcompression *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_JPEGCOMP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_JPEGCOMP, argp);
	return err;
}

int ioctl_VIDIOC_S_JPEGCOMP(int fd, struct v4l2_jpegcompression *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_JPEGCOMP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_JPEGCOMP, argp);
	return err;
}

int ioctl_VIDIOC_G_MODULATOR(int fd, struct v4l2_modulator *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_MODULATOR, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_MODULATOR, argp);
	return err;
}

int ioctl_VIDIOC_S_MODULATOR(int fd, struct v4l2_modulator *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_MODULATOR, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_MODULATOR, argp);
	return err;
}

int ioctl_VIDIOC_G_OUTPUT(int fd, int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_OUTPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_OUTPUT, argp);
	return err;
}

int ioctl_VIDIOC_S_OUTPUT(int fd, int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_OUTPUT, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_OUTPUT, argp);
	return err;
}

int ioctl_VIDIOC_G_PARM(int fd, struct v4l2_streamparm *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_PARM, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_PARM, argp);
	return err;
}

int ioctl_VIDIOC_S_PARM(int fd, struct v4l2_streamparm *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_PARM, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_PARM, argp);
	return err;
}

int ioctl_VIDIOC_G_PRIORITY(int fd, enum v4l2_priority *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_PRIORITY, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_PRIORITY, argp);
	return err;
}

int ioctl_VIDIOC_S_PRIORITY(int fd, enum v4l2_priority *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_PRIORITY, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_PRIORITY, argp);
	return err;
}

int ioctl_VIDIOC_G_SLICED_VBI_CAP(int fd, struct v4l2_sliced_vbi_cap *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_SLICED_VBI_CAP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_SLICED_VBI_CAP, argp);
	return err;
}

int ioctl_VIDIOC_G_STD(int fd, v4l2_std_id *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_STD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_STD, argp);
	return err;
}

int ioctl_VIDIOC_S_STD(int fd, v4l2_std_id *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_STD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_STD, argp);
	return err;
}

int ioctl_VIDIOC_G_TUNER(int fd, struct v4l2_tuner *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_G_TUNER, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_G_TUNER, argp);
	return err;
}

int ioctl_VIDIOC_S_TUNER(int fd, struct v4l2_tuner *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_S_TUNER, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_S_TUNER, argp);
	return err;
}

int ioctl_VIDIOC_LOG_STATUS(int fd, int extra)
{
	int err = 0;

	if(extra)
	{
		do
		{
			CALL_IOCTL(fd, VIDIOC_LOG_STATUS);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL(fd, VIDIOC_LOG_STATUS);
	return err;
}

int ioctl_VIDIOC_OVERLAY(int fd, int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_OVERLAY, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_OVERLAY, argp);
	return err;
}

int ioctl_VIDIOC_QBUF(int fd, struct v4l2_buffer *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QBUF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QBUF, argp);
	return err;
}

int ioctl_VIDIOC_DQBUF(int fd, struct v4l2_buffer *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_DQBUF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_DQBUF, argp);
	return err;
}

int ioctl_VIDIOC_QUERYBUF(int fd, struct v4l2_buffer *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QUERYBUF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QUERYBUF, argp);
	return err;
}

int ioctl_VIDIOC_QUERYCAP(int fd, struct v4l2_capability *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QUERYCAP, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QUERYCAP, argp);
	return err;
}

int ioctl_VIDIOC_QUERYCTRL(int fd, struct v4l2_queryctrl *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QUERYCTRL, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QUERYCTRL, argp);
	return err;
}

int ioctl_VIDIOC_QUERYMENU(int fd, struct v4l2_querymenu *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QUERYMENU, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QUERYMENU, argp);
	return err;
}

int ioctl_VIDIOC_QUERYSTD(int fd, v4l2_std_id *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_QUERYSTD, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_QUERYSTD, argp);
	return err;
}

int ioctl_VIDIOC_REQBUFS(int fd, struct v4l2_requestbuffers *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_REQBUFS, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_REQBUFS, argp);
	return err;
}

int ioctl_VIDIOC_STREAMON(int fd, const int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_STREAMON, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_STREAMON, argp);
	return err;
}

int ioctl_VIDIOC_STREAMOFF(int fd, const int *argp, int extra)
{
	int err = 0;
	if(!argp)
		return -EINVAL;

	if(extra)
	{
		do
		{
			CALL_IOCTL2(fd, VIDIOC_STREAMOFF, argp);
		}
		while (-1 == err && EINTR == errno);
		return err;
	}

	CALL_IOCTL2(fd, VIDIOC_STREAMOFF, argp);
	return err;
}
