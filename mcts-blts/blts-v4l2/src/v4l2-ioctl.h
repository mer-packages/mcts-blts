/* v4l2-ioctl.h -- IF for v4l2 core ioctls

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

#ifndef V4L2_IOCTL_H
#define V4L2_IOCTL_H

#include <asm/types.h>		/* for videodev2.h */
#include <linux/videodev2.h>
#include <linux/version.h>

/* Backward compatibility for old kernels. Debug API changed after Linux 2.6.28;
   old-style VIDIOC_G_CHIP_IDENT went away in 2.6.30.
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,28)
#warning " *** Compiling with ancient kernel headers. Expect breakage."
#define v4l2_dbg_register v4l2_register
#define VIDIOC_DBG_G_CHIP_IDENT VIDIOC_G_CHIP_IDENT
#define v4l2_dbg_chip_ident v4l2_chip_ident
#endif

int ioctl_VIDIOC_CROPCAP(int fd, struct v4l2_cropcap *argp, int extra);
int ioctl_VIDIOC_DBG_G_REGISTER(int fd, struct v4l2_dbg_register *argp, int extra);
int ioctl_VIDIOC_DBG_S_REGISTER(int fd, struct v4l2_dbg_register *argp, int extra);
int ioctl_VIDIOC_ENCODER_CMD(int fd, struct v4l2_encoder_cmd *argp, int extra);
int ioctl_VIDIOC_TRY_ENCODER_CMD(int fd, struct v4l2_encoder_cmd *argp, int extra);
int ioctl_VIDIOC_ENUMAUDIO(int fd, struct v4l2_audio *argp, int extra);
int ioctl_VIDIOC_ENUMAUDOUT(int fd, struct v4l2_audioout *argp, int extra);
int ioctl_VIDIOC_ENUM_FMT(int fd, struct v4l2_fmtdesc *argp, int extra);
int ioctl_VIDIOC_ENUM_FRAMESIZES(int fd, struct v4l2_frmsizeenum *argp, int extra);
int ioctl_VIDIOC_ENUM_FRAMEINTERVALS(int fd, struct v4l2_frmivalenum *argp, int extra);
int ioctl_VIDIOC_ENUMINPUT(int fd, struct v4l2_input *argp, int extra);
int ioctl_VIDIOC_ENUMOUTPUT(int fd, struct v4l2_output *argp, int extra);
int ioctl_VIDIOC_ENUMSTD(int fd, struct v4l2_standard *argp, int extra);
int ioctl_VIDIOC_G_AUDIO(int fd, struct v4l2_audio *argp, int extra);
int ioctl_VIDIOC_S_AUDIO(int fd, struct v4l2_audio *argp, int extra);
int ioctl_VIDIOC_G_AUDOUT(int fd, struct v4l2_audioout *argp, int extra);
int ioctl_VIDIOC_S_AUDOUT(int fd, struct v4l2_audioout *argp, int extra);
int ioctl_VIDIOC_DBG_G_CHIP_IDENT(int fd, struct v4l2_dbg_chip_ident *argp, int extra);
int ioctl_VIDIOC_G_CROP(int fd, struct v4l2_crop *argp, int extra);
int ioctl_VIDIOC_S_CROP(int fd, struct v4l2_crop *argp, int extra);
int ioctl_VIDIOC_G_CTRL(int fd, struct v4l2_control *argp, int extra);
int ioctl_VIDIOC_S_CTRL(int fd, struct v4l2_control *argp, int extra);
int ioctl_VIDIOC_G_ENC_INDEX(int fd, struct v4l2_enc_idx *argp, int extra);
int ioctl_VIDIOC_G_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra);
int ioctl_VIDIOC_S_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra);
int ioctl_VIDIOC_TRY_EXT_CTRLS(int fd, struct v4l2_ext_controls *argp, int extra);
int ioctl_VIDIOC_G_FBUF(int fd, struct v4l2_framebuffer *argp, int extra);
int ioctl_VIDIOC_S_FBUF(int fd, struct v4l2_framebuffer *argp, int extra);
int ioctl_VIDIOC_G_FMT(int fd, struct v4l2_format *argp, int extra);
int ioctl_VIDIOC_S_FMT(int fd, struct v4l2_format *argp, int extra);
int ioctl_VIDIOC_TRY_FMT(int fd, struct v4l2_format *argp, int extra);
int ioctl_VIDIOC_G_FREQUENCY(int fd, struct v4l2_frequency *argp, int extra);
int ioctl_VIDIOC_S_FREQUENCY(int fd, struct v4l2_frequency *argp, int extra);
int ioctl_VIDIOC_G_INPUT(int fd, int *argp, int extra);
int ioctl_VIDIOC_S_INPUT(int fd, int *argp, int extra);
int ioctl_VIDIOC_G_JPEGCOMP(int fd, struct v4l2_jpegcompression *argp, int extra);
int ioctl_VIDIOC_S_JPEGCOMP(int fd, struct v4l2_jpegcompression *argp, int extra);
int ioctl_VIDIOC_G_MODULATOR(int fd, struct v4l2_modulator *argp, int extra);
int ioctl_VIDIOC_S_MODULATOR(int fd, struct v4l2_modulator *argp, int extra);
int ioctl_VIDIOC_G_OUTPUT(int fd, int *argp, int extra);
int ioctl_VIDIOC_S_OUTPUT(int fd, int *argp, int extra);
int ioctl_VIDIOC_G_PARM(int fd, struct v4l2_streamparm *argp, int extra);
int ioctl_VIDIOC_S_PARM(int fd, struct v4l2_streamparm *argp, int extra);
int ioctl_VIDIOC_G_PRIORITY(int fd, enum v4l2_priority *argp, int extra);
int ioctl_VIDIOC_S_PRIORITY(int fd, enum v4l2_priority *argp, int extra);
int ioctl_VIDIOC_G_SLICED_VBI_CAP(int fd, struct v4l2_sliced_vbi_cap *argp, int extra);
int ioctl_VIDIOC_G_STD(int fd, v4l2_std_id *argp, int extra);
int ioctl_VIDIOC_S_STD(int fd, v4l2_std_id *argp, int extra);
int ioctl_VIDIOC_G_TUNER(int fd, struct v4l2_tuner *argp, int extra);
int ioctl_VIDIOC_S_TUNER(int fd, struct v4l2_tuner *argp, int extra);
int ioctl_VIDIOC_LOG_STATUS(int fd, int extra);
int ioctl_VIDIOC_OVERLAY(int fd, int *argp, int extra);
int ioctl_VIDIOC_QBUF(int fd, struct v4l2_buffer *argp, int extra);
int ioctl_VIDIOC_DQBUF(int fd, struct v4l2_buffer *argp, int extra);
int ioctl_VIDIOC_QUERYBUF(int fd, struct v4l2_buffer *argp, int extra);
int ioctl_VIDIOC_QUERYCAP(int fd, struct v4l2_capability *argp, int extra);
int ioctl_VIDIOC_QUERYCTRL(int fd, struct v4l2_queryctrl *argp, int extra);
int ioctl_VIDIOC_QUERYMENU(int fd, struct v4l2_querymenu *argp, int extra);
int ioctl_VIDIOC_QUERYSTD(int fd, v4l2_std_id *argp, int extra);
int ioctl_VIDIOC_REQBUFS(int fd, struct v4l2_requestbuffers *argp, int extra);
int ioctl_VIDIOC_STREAMON(int fd, const int *argp, int extra);
int ioctl_VIDIOC_STREAMOFF(int fd, const int *argp, int extra);

#endif  /* V4L2_IOCTL_H */
