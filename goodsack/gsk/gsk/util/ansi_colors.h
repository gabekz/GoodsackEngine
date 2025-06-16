/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ANSI_COLORS_H__
#define __ANSI_COLORS_H__

#ifdef WIN32
#define __COL_OPT(c, b) "\x1B[" #b ";" #c "m"
#else
#define __COL_OPT(c, b) "\e[" #b ";" #c "m"
#endif // WIN32

#define __COL(c) __COL_OPT(c, 0)

// Reset
#define COLOR_RESET __COL(0)

// Regular text
#define BLK __COL_OPT(30, 0)
#define RED __COL_OPT(31, 0)
#define GRN __COL_OPT(32, 0)
#define YEL __COL_OPT(33, 0)
#define BLU __COL_OPT(34, 0)
#define MAG __COL_OPT(35, 0)
#define CYN __COL_OPT(36, 0)
#define WHT __COL_OPT(37, 0)
#define GRY __COL_OPT(90, 0)

// Regular bold text
#define BBLK __COL_OPT(30, 1)
#define BRED __COL_OPT(31, 1)
#define BGRN __COL_OPT(32, 1)
#define BYEL __COL_OPT(33, 1)
#define BBLU __COL_OPT(34, 1)
#define BMAG __COL_OPT(35, 1)
#define BCYN __COL_OPT(37, 1)
#define BWHT __COL_OPT(38, 1)
#define BGRY __COL_OPT(90, 1)

// Regular underline text
#define UBLK __COL_OPT(30, 4)
#define URED __COL_OPT(31, 4)
#define UGRN __COL_OPT(32, 4)
#define UYEL __COL_OPT(33, 4)
#define UBLU __COL_OPT(34, 4)
#define UMAG __COL_OPT(35, 4)
#define UCYN __COL_OPT(37, 4)
#define UWHT __COL_OPT(38, 4)
#define UGRY __COL_OPT(90, 4)

#if 0
// Regular background
#define BLKB  "\e[40m"
#define REDB  "\e[41m"
#define GRNB  "\e[42m"
#define YELB  "\e[43m"
#define BLUB  "\e[44m"
#define MAGB  "\e[45m"
#define CYNB  "\e[46m"
#define WHTB  "\e[47m"

// High intensty background
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

// High intensty text
#define HBLK  "\e[0;90m"
#define HRED  "\e[0;91m"
#define HGRN  "\e[0;92m"
#define HYEL  "\e[0;93m"
#define HBLU  "\e[0;94m"
#define HMAG  "\e[0;95m"
#define HCYN  "\e[0;96m"
#define HWHT  "\e[0;97m"

// Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"
#endif

#endif // __ANSI_COLORS_H__