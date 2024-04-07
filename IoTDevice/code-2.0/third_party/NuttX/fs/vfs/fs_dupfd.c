/****************************************************************************
 * fs/vfs/fs_dupfd.c
 *
 *   Copyright (C) 2007-2009, 2011-2014, 2017 Gregory Nutt. All rights
 *     reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
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
 * 3. Neither the name NuttX nor the names of its contributors may be
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"

#include "assert.h"
#include "errno.h"
#include "sched.h"

#include "fs/fs.h"
#include "fs/file.h"

#include "fs/vnode.h"
#include "stdlib.h"
#include "string.h"
#if CONFIG_NFILE_DESCRIPTORS > 0

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_dup
 *
 * Description:
 *   Equivalent to the non-standard fs_dupfd() function except that it
 *   accepts a struct file instance instead of a file descriptor and does
 *   not set the errno variable.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

int file_dup(struct file *filep, int minfd)
{
  int fd2;
  int ret;
  int err,len,rellen;
  struct file *filep2 = NULL;
  char *fullpath = NULL;
  char *relpath = NULL;

  /* Verify that fd is a valid, open file descriptor */

  if ((filep->f_vnode == NULL) || (filep->f_path == NULL))
    {
      set_errno(EBADF);
      return VFS_ERROR;
    }

  len = strlen(filep->f_path);
  fullpath = (char *)zalloc(len + 1);
  if (fullpath == NULL)
    {
      set_errno(ENOMEM);
      return VFS_ERROR;
    }

  /* Then allocate a new file descriptor for the vnode */

  fd2 = files_allocate(filep->f_vnode, filep->f_oflags, filep->f_pos, filep->f_priv, minfd);
  if (fd2 < 0)
    {
      free(fullpath);
      set_errno(EMFILE);
      return VFS_ERROR;
    }

  ret = fs_getfilep(fd2, &filep2);

  (void)strncpy_s(fullpath, len + 1, filep->f_path, len);
  if (filep->f_relpath != NULL)
    {
      rellen = strlen(filep->f_relpath);
      relpath = (char *)zalloc(rellen + 1);
      (void)strncpy_s(relpath, rellen + 1, filep->f_relpath, rellen);
    }
  filep2->f_path = fullpath;
  filep2->f_relpath = relpath;
  filep2->f_priv = filep->f_priv;

  if (ret < 0)
    {
      goto errout_with_vnode;
    }

  return fd2;

errout_with_vnode:
  clear_fd(fd2);
  free(fullpath);
  filep2->f_oflags  = 0;
  filep2->f_pos     = 0;
  filep2->f_vnode   = NULL;
  filep2->f_priv    = NULL;
  filep2->f_path    = NULL;
  filep2->f_relpath = NULL;
  filep2->f_mapping = NULL;
  err               = -ret;
  set_errno(err);
  return VFS_ERROR;
}

/****************************************************************************
 * Name: fs_dupfd OR dup
 *
 * Description:
 *   Clone a file descriptor 'fd' to an arbitray descriptor number (any value
 *   greater than or equal to 'minfd'). If socket descriptors are
 *   implemented, then this is called by dup() for the case of file
 *   descriptors.  If socket descriptors are not implemented, then this
 *   function IS dup().
 *
 * Returned Value:
 *   fs_dupfd is sometimes an OS internal function and sometimes is a direct
 *   substitute for dup().  So it must return an errno value as though it
 *   were dup().
 *
 ****************************************************************************/

int fs_dupfd(int fd, int minfd)
{
  struct file *filep;

  /* Get the file structure corresponding to the file descriptor. */

  int ret = fs_getfilep(fd, &filep);
  if (ret < 0)
    {
      /* The errno value has already been set */
      return VFS_ERROR;
    }

  /* Let file_dup() do the real work */

  return file_dup(filep, minfd);
}

#endif /* CONFIG_NFILE_DESCRIPTORS > 0 */
