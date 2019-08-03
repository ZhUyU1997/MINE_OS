#ifndef _TPYES_H
#define _TPYES_H

typedef unsigned char			UINT8;
typedef unsigned short			UINT16;
typedef unsigned int			UINT32;

typedef unsigned char			u8;
typedef unsigned short			u16;
typedef unsigned int			u32;
typedef signed int				s32;

typedef signed char				s8_t;
typedef unsigned char			u8_t;

typedef signed short			s16_t;
typedef unsigned short			u16_t;

typedef signed int				s32_t;
typedef unsigned int			u32_t;

typedef signed long long		s64_t;
typedef unsigned long long		u64_t;

typedef signed long long		intmax_t;
typedef unsigned long long		uintmax_t;

typedef unsigned char			__u8;
typedef unsigned short			__u16;
typedef unsigned int			__u32;

#ifndef U8
typedef unsigned char			U8;
#endif
#ifndef U16
typedef unsigned short			U16;
#endif
#ifndef U32
typedef unsigned int			U32;
#endif

typedef unsigned char			uint8_t;
typedef signed char				int8_t;
typedef unsigned short			uint16_t;
typedef signed short			int16_t;
typedef unsigned int			uint32_t;
typedef signed int				int32_t;

typedef unsigned char			uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short	vu_short;
typedef volatile unsigned char	vu_char;

typedef signed int				ptrdiff_t;
typedef signed int				intptr_t;
typedef unsigned int 			uintptr_t;

typedef unsigned long			size_t;
typedef signed   long			ssize_t;

typedef unsigned long			time_t;
typedef unsigned long			clock_t;

typedef unsigned int			uid_t;
typedef unsigned int			gid_t;
typedef unsigned int			pid_t;

typedef unsigned int			dev_t;
typedef unsigned int			ino_t;
typedef unsigned int			mode_t;
typedef unsigned int			nlink_t;

typedef long int				off_t;
typedef long long				loff_t;

typedef signed int				bool_t;
typedef unsigned int			irq_flags_t;

typedef unsigned int			virtual_addr_t;
typedef unsigned int			virtual_size_t;
typedef unsigned int			physical_addr_t;
typedef unsigned int			physical_size_t;

#endif /* _TPYES_H */
