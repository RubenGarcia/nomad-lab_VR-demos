/*
# Copyright 2016-2018 Ruben Jesus Garcia Hernandez
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include <stdarg.h>
#include <stdio.h>

#include "NOMADVRLib/eprintf.h"

void eprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );
	va_end( args );

	fprintf(stderr, "%s\n", buffer );
}
