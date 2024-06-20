//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,28 2011
//    Module Name               : string.cpp
//    Module Funciton           : 
//                                This module and stdio.h countains the
//                                standard C lib input/output operation functions.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "stdio.h"

//------------------------------------------------------------------------
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#define ZEROPAD     1               // Pad with zero
#define SIGN        2               // Unsigned/signed long
#define PLUS        4               // Show plus
#define SPACE       8               // Space if plus
#define LEFT        16              // Left justified
#define SPECIAL     32              // 0x
#define LARGE       64              // Use 'ABCDEF' instead of 'abcdef'
#define LONGLONG    128             // 64 bits integer.

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static size_t strnlen(const char *s, size_t count)
{
  const char *sc;
  for (sc = s; *sc != '\0' && count--; ++sc);
  return sc - s;
}

static int skip_atoi(const char **s)
{
  int i = 0;
  while (is_digit(**s)) i = i*10 + *((*s)++) - '0';
  return i;
}

/* Revised to support 64 bits integer. */
static char *number(char *str, long long num, int base, int size, int precision, int type)
{
  char c, sign, tmp[66];
  char *dig = digits;
  int i;

  if (type & LARGE)  dig = upper_digits;
  if (type & LEFT) type &= ~ZEROPAD;
  if (base < 2 || base > 36) return 0;
  
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (type & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (type & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  if (type & SPECIAL)
  {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }

  i = 0;

  if (num == 0)
    tmp[i++] = '0';
  else
  {
    while (num != 0)
    {
      tmp[i++] = dig[((unsigned long long) num) % (unsigned) base];
      num = ((unsigned long long) num) / (unsigned) base;
    }
  }

  if (i > precision) precision = i;
  size -= precision;
  if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
  if (sign) *str++ = sign;
  
  if (type & SPECIAL)
  {
    if (base == 8)
      *str++ = '0';
    else if (base == 16)
    {
      *str++ = '0';
      *str++ = digits[33];
    }
  }

  if (!(type & LEFT)) while (size-- > 0) *str++ = c;
  while (i < precision--) *str++ = '0';
  while (i-- > 0) *str++ = tmp[i];
  while (size-- > 0) *str++ = ' ';

  return str;
}

static char *eaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  char *dig = digits;
  int i, len;

  if (type & LARGE)  dig = upper_digits;
  len = 0;
  for (i = 0; i < 6; i++)
  {
    if (i != 0) tmp[len++] = ':';
    tmp[len++] = dig[addr[i] >> 4];
    tmp[len++] = dig[addr[i] & 0x0F];
  }

  if (!(type & LEFT)) while (len < size--) *str++ = ' ';
  for (i = 0; i < len; ++i) *str++ = tmp[i];
  while (len < size--) *str++ = ' ';

  return str;
}

static char *iaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  int i, n, len;

  len = 0;
  for (i = 0; i < 4; i++)
  {
    if (i != 0) tmp[len++] = '.';
    n = addr[i];
    
    if (n == 0)
      tmp[len++] = digits[0];
    else
    {
      if (n >= 100) 
      {
        tmp[len++] = digits[n / 100];
        n = n % 100;
        tmp[len++] = digits[n / 10];
        n = n % 10;
      }
      else if (n >= 10) 
      {
        tmp[len++] = digits[n / 10];
        n = n % 10;
      }

      tmp[len++] = digits[n];
    }
  }

  if (!(type & LEFT)) while (len < size--) *str++ = ' ';
  for (i = 0; i < len; ++i) *str++ = tmp[i];
  while (len < size--) *str++ = ' ';

  return str;
}

#ifndef NOFLOAT

char *ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);
char *fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf);

static void cfltcvt(double value, char *buffer, char fmt, int precision)
{
  int decpt, sign, exp, pos;
  char *digits = NULL;
  char cvtbuf[80];
  int capexp = 0;
  int magnitude;

  if (fmt == 'G' || fmt == 'E')
  {
    capexp = 1;
    fmt += 'a' - 'A';
  }

  if (fmt == 'g')
  {
    digits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
    magnitude = decpt - 1;
    if (magnitude < -4  ||  magnitude > precision - 1)
    {
      fmt = 'e';
      precision -= 1;
    }
    else
    {
      fmt = 'f';
      precision -= decpt;
    }
  }

  if (fmt == 'e')
  {
    digits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

    if (sign) *buffer++ = '-';
    *buffer++ = *digits;
    if (precision > 0) *buffer++ = '.';
    memcpy(buffer, digits + 1, precision);
    buffer += precision;
    *buffer++ = capexp ? 'E' : 'e';

    if (decpt == 0)
    {
      if (value == 0.0)
        exp = 0;
      else
        exp = -1;
    }
    else
      exp = decpt - 1;

    if (exp < 0)
    {
      *buffer++ = '-';
      exp = -exp;
    }
    else
      *buffer++ = '+';

    buffer[2] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[1] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[0] = (exp % 10) + '0';
    buffer += 3;
  }
  else if (fmt == 'f')
  {
    digits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
    if (sign) *buffer++ = '-';
    if (*digits)
    {
      if (decpt <= 0)
      {
        *buffer++ = '0';
        *buffer++ = '.';
        for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
        while (*digits) *buffer++ = *digits++;
      }
      else
      {
        pos = 0;
        while (*digits)
        {
          if (pos++ == decpt) *buffer++ = '.';
          *buffer++ = *digits++;
        }
      }
    }
    else
    {
      *buffer++ = '0';
      if (precision > 0)
      {
        *buffer++ = '.';
        for (pos = 0; pos < precision; pos++) *buffer++ = '0';
      }
    }
  }

  *buffer = '\0';
}

static void forcdecpt(char *buffer)
{
  while (*buffer)
  {
    if (*buffer == '.') return;
    if (*buffer == 'e' || *buffer == 'E') break;
    buffer++;
  }

  if (*buffer)
  {
    int n = strlen(buffer);
    while (n > 0) 
    {
      buffer[n + 1] = buffer[n];
      n--;
    }

    *buffer = '.';
  }
  else
  {
    *buffer++ = '.';
    *buffer = '\0';
  }
}

static void cropzeros(char *buffer)
{
  char *stop;

  while (*buffer && *buffer != '.') buffer++;
  if (*buffer++)
  {
    while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
    stop = buffer--;
    while (*buffer == '0') buffer--;
    if (*buffer == '.') buffer--;
    while (*++buffer = *stop++);
  }
}

static char *flt(char *str, double num, int size, int precision, char fmt, int flags)
{
  char tmp[80];
  char c, sign;
  int n, i;

  // Left align means no zero padding
  if (flags & LEFT) flags &= ~ZEROPAD;

  // Determine padding and sign char
  c = (flags & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (flags & SIGN)
  {
    if (num < 0.0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (flags & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (flags & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  // Compute the precision value
  if (precision < 0)
    precision = 6; // Default precision: 6
  else if (precision == 0 && fmt == 'g')
    precision = 1; // ANSI specified

  // Convert floating point number to text
  cfltcvt(num, tmp, fmt, precision);

  // '#' and precision == 0 means force a decimal point
  if ((flags & SPECIAL) && precision == 0) forcdecpt(tmp);

  // 'g' format means crop zero unless '#' given
  if (fmt == 'g' && !(flags & SPECIAL)) cropzeros(tmp);

  n = strlen(tmp);

  // Output number with alignment and padding
  size -= n;
  if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
  if (sign) *str++ = sign;
  if (!(flags & LEFT)) while (size-- > 0) *str++ = c;
  for (i = 0; i < n; i++) *str++ = tmp[i];
  while (size-- > 0) *str++ = ' ';

  return str;
}

#endif

int _hx_vsprintf(char *buf, const char *fmt, va_list args)
{
  int len;
  unsigned long long num;
  int i, base;
  char *str;
  char *s;

  int flags;            // Flags to number()

  int field_width;      // Width of output field
  int precision;        // Min. # of digits for integers; max number of chars for from string
  int qualifier;        // 'h', 'l', or 'L' for integer fields

  for (str = buf; *fmt; fmt++)
  {
    if (*fmt != '%')
    {
      *str++ = *fmt;
      continue;
    }
                  
    // Process flags
    flags = 0;
repeat:
    fmt++; // This also skips first '%'
    switch (*fmt)
    {
      case '-': flags |= LEFT; goto repeat;
      case '+': flags |= PLUS; goto repeat;
      case ' ': flags |= SPACE; goto repeat;
      case '#': flags |= SPECIAL; goto repeat;
      case '0': flags |= ZEROPAD; goto repeat;
    }
          
    // Get field width
    field_width = -1;
    if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
    else if (*fmt == '*')
    {
      fmt++;
      field_width = va_arg(args, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    // Get the precision
    precision = -1;
    if (*fmt == '.')
    {
      ++fmt;    
      if (is_digit(*fmt))
        precision = skip_atoi(&fmt);
      else if (*fmt == '*')
      {
        ++fmt;
        precision = va_arg(args, int);
      }
      if (precision < 0) precision = 0;
    }

    // Get the conversion qualifier
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      fmt++;
    }
	/* support 'll'(64 bits integer) qualifier. */
	if (*fmt == 'l')
	{
		flags |= LONGLONG;
		fmt++;
	}

    // Default base
    base = 10;

    switch (*fmt)
    {
      case 'c':
        if (!(flags & LEFT)) while (--field_width > 0) *str++ = ' ';
        *str++ = (unsigned char) va_arg(args, int);
        while (--field_width > 0) *str++ = ' ';
        continue;

      case 's':
        s = va_arg(args, char *);
        if (!s) s = "<NULL>";
        len = strnlen(s, precision);
        if (!(flags & LEFT)) while (len < field_width--) *str++ = ' ';
        for (i = 0; i < len; ++i) *str++ = *s++;
        while (len < field_width--) *str++ = ' ';
        continue;

      case 'p':
        if (field_width == -1)
        {
          field_width = 2 * sizeof(void *);
          flags |= ZEROPAD;
        }
        str = number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
        continue;

      case 'n':
        if (qualifier == 'l')
        {
          long *ip = va_arg(args, long *);
          *ip = (str - buf);
        }
        else
        {
          int *ip = va_arg(args, int *);
          *ip = (str - buf);
        }
        continue;

      case 'A':
        flags |= LARGE;

      case 'a':
        if (qualifier == 'l')
          str = eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
        else
          str = iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
        continue;

      // Integer number formats - set up the flags and "break"
      case 'o':
        base = 8;
        break;

      case 'X':
        flags |= LARGE;

      case 'x':
        base = 16;
        break;

      case 'd':
      case 'i':
        flags |= SIGN;

      case 'u':
        break;

#ifndef NOFLOAT

      case 'E':
      case 'G':
      case 'e':
      case 'f':
      case 'g':
        str = flt(str, va_arg(args, double), field_width, precision, *fmt, flags | SIGN);
        continue;

#endif

      default:
        if (*fmt != '%') *str++ = '%';
        if (*fmt)
          *str++ = *fmt;
        else
          --fmt;
        continue;
    }

	if (qualifier == 'l')
	{
		if (flags & LONGLONG)
		{
			num = va_arg(args, unsigned long long);
		}
		else
		{
			num = va_arg(args, unsigned long);
		}
	}
    else if (qualifier == 'h')
    {
      if (flags & SIGN)
        num = va_arg(args, short);
      else
        num = va_arg(args, unsigned short);
    }
    else if (flags & SIGN)
      num = va_arg(args, int);
    else
      num = va_arg(args, unsigned int);

    str = number(str, num, base, field_width, precision, flags);
  }

  *str = '\0';
  return str - buf;
}

int _hx_sprintf(char *buf, const char *fmt, ...)
{
  va_list args;
  int n;

  va_start(args, fmt);
  n = _hx_vsprintf(buf, fmt, args);
  va_end(args);

  return n;
}

#if 0
//A mutex object to sychronize the console accessing between
//different kernel threads.If do not enforce the sychronization,
//the output will mix when multiple threads are accessing the
//output console concurrently.
//The mutex object will be initialized when the _hx_sprintf is
//first called.
static __MUTEX* mtx = NULL;

int _hx_printf(const char* fmt,...)
{
	//Define local buffer according kernel thread's stack size.
#if DEFAULT_STACK_SIZE <= 2048
	char buff[128];
#else
	char buff[256];
#endif
	va_list args;
	int n;
	int i = 0;
	DWORD x, y; //Cursor position control.
	WORD wr = 0x0700;

	//Try to acquire the synchronizing mutex object,create it
	//if it is not exist.
	//We will ignore this step if in process of system initialization
	//or in interrupt context.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		if(NULL == mtx)
		{
			mtx = (__MUTEX*)ObjectManager.CreateObject(
				&ObjectManager,
				NULL,
				OBJECT_TYPE_MUTEX);
			if(NULL == mtx)
			{
				return -1;
			}
			//Initialize it.
			if(!mtx->Initialize((__COMMON_OBJECT*)mtx))  //Can not initialize.
			{
				ObjectManager.DestroyObject(
					&ObjectManager,
					(__COMMON_OBJECT*)mtx);
				return -1;
			}
		}
		//Try to acquire the mutex object.
		mtx->WaitForThisObject((__COMMON_OBJECT*)mtx);
	}

	va_start(args,fmt);
	n = _hx_vsprintf(buff,fmt,args);
	va_end(args);

	//Print out the string buffer.
	while(buff[i])
	{
		if('\n' == buff[i])
		{
			GotoHome();
			ChangeLine();
			i ++;
			continue;
		}
		if('\r' == buff[i])
		{
			GotoHome();
			i ++;
			continue;
		}
		if ('\t' == buff[i])
		{
			CD_GetCursorPos((WORD*)&x, (WORD*)&y);
			x = x + TAB_SPACE_NUM - (x % TAB_SPACE_NUM);
			CD_SetCursorPos((WORD)x,(WORD)y);
			i++;
			continue;
		}
		wr += buff[i];
		PrintCh(wr);
		wr -= buff[i];
		i ++;
	}
	//Release the mutex object.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		mtx->ReleaseMutex((__COMMON_OBJECT*)mtx);
	}

	return 0;
}
#endif

#if defined(__CFG_SYS_SMP)
/* Spin lock used by _hx_printk routine. */
static __SPIN_LOCK sl_printk = SPIN_LOCK_INIT_VALUE;
#endif

int _hx_printk(const char* fmt, ...)
{
	//Define local buffer according kernel thread's stack size.
#if DEFAULT_STACK_SIZE <= 2048
	char buff[128];
#else
	char buff[256];
#endif
	va_list args;
	int n;
	int i = 0;
	DWORD x, y; //Cursor position control.
	WORD wr = 0x0700;
	unsigned long ulFlags = 0;

	__ENTER_CRITICAL_SECTION_SMP(sl_printk, ulFlags);

	va_start(args, fmt);
	n = _hx_vsprintf(buff, fmt, args);
	va_end(args);

	/* Show out the formated string buffer. */
	while (buff[i])
	{
		if ('\n' == buff[i])
		{
			GotoHome();
			ChangeLine();
			i++;
			continue;
		}
		if ('\r' == buff[i])
		{
			GotoHome();
			i++;
			continue;
		}
		if ('\t' == buff[i])
		{
			CD_GetCursorPos((WORD*)&x, (WORD*)&y);
			x = x + TAB_SPACE_NUM - (x % TAB_SPACE_NUM);
			CD_SetCursorPos((WORD)x, (WORD)y);
			i++;
			continue;
		}
		wr += buff[i];
		PrintCh(wr);
		wr -= buff[i];
		i++;
	}

	__LEAVE_CRITICAL_SECTION_SMP(sl_printk, ulFlags);

	return 0;
}

//_hx_snprintf routine,same as snprintf routine in libc.
int _hx_snprintf(char *buf,size_t n, const char *fmt, ...)
{
  va_list args;
  int len;
#if DEFAULT_STACK_SIZE < 2048
  char buff[128];
#else
  char buff[256];
#endif

  va_start(args, fmt);
  len = _hx_vsprintf(buff, fmt, args);
  va_end(args);

  //Now copy the result into buf.
  strncpy(buf,buff,n - 1);
  buf[n - 1] = 0;

  return n;
}

//vfprintf's implementation.
int _hx_vfprintf(void* stream,const char* fmt,va_list args)
{
	return 0;
}
