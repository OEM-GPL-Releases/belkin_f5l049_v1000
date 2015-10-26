/*
 * Socket-level I/O control calls.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995 by Ralf Baechle
 */
#ifndef _ASM_SOCKIOS_H
#define _ASM_SOCKIOS_H

#include <asm/ioctl.h>

/* Socket-level I/O control calls. */
#define FIOGETOWN	_IOR('f', 123, int)
#define FIOSETOWN 	_IOW('f', 124, int)

#define SIOCATMARK	_IOR('s', 7, int)
#define SIOCSPGRP	_IOW('s', 8, pid_t)
#define SIOCGPGRP	_IOR('s', 9, pid_t)

#define SIOCGSTAMP	0x8906		/* Get stamp (timeval) */
#define SIOCGSTAMPNS	0x8907		/* Get stamp (timespec) */

#define SIOCGMIIPHY	0x8947		/* Get address of MII PHY in use. */
#define SIOCGMIIREG	0x8948		/* Read MII PHY register.	*/
#define SIOCSMIIREG	0x8949		/* Write MII PHY register.	*/

/*
 *	These 16 ioctls are available to devices via the do_ioctl() device
 *	vector. Each device should include this file and redefine these names
 *	as their own. Because these are device dependent it is a good idea
 *	_NOT_ to issue them to random objects and hope.
 *
 *	THESE IOCTLS ARE _DEPRECATED_ AND WILL DISAPPEAR IN 2.5.X -DaveM
 */
 
#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

#define SET_ETH_AUTO    SIOCDEVPRIVATE  
#define SET_ETH_10H     SIOCDEVPRIVATE+1
#define SET_ETH_10F     SIOCDEVPRIVATE+2
#define SET_ETH_100H    SIOCDEVPRIVATE+3
#define SET_ETH_100F    SIOCDEVPRIVATE+4
#define GET_PHYREG_SHOW SIOCDEVPRIVATE+5

#endif /* _ASM_SOCKIOS_H */
