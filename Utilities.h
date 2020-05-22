/*    
    Copyright 2020, Network Research Lab at the University of Toronto.

    This file is part of CottonCandy.

    CottonCandy is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CottonCandy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CottonCandy.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef HEADER_UTILITIES
#define HEADER_UTILITIES


/**
 * Helper function for timing. On Arduino, it can be done using millis().
 */
unsigned long getTimeMillis();


/**
 * Helper function for sleeping. On Arduino, it can be done using delay().
 */
void sleepForMillis(unsigned long time);

#endif