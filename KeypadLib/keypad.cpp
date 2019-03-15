/*	 Copyright (c) 2015 Rune Langøy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
  Example is available in keypad.h

*/

#include "mbed.h"
#include "keypad.h"

using namespace mbed;

Keypad::Keypad(PinName col0, PinName col1, PinName col2, PinName col3,
               PinName row0,PinName row1, PinName row2, PinName row3):
    _cols(col0,col1,col2,col3) ,_rows(row0,row1,row2,row3)    {    }

void Keypad::enablePullUp()
{
    _cols.mode(PullUp);
}

bool Keypad::getKeyPressed()
{
    _rows = 0;              // Ground all keys
    if(_cols.read()==0xff)  //Chk if key is pressed
        return false;

    return true;
}

int Keypad::getKeyIndex()
{
    if (!getKeyPressed())
        return -1;

    //Scan rows and cols and return switch index
    for(int i=0; i<4; i++) {
        _rows = ~(0x01 << i);
        for(int j=0; j<4; j++)
            if (  !( (_cols.read()>> j )& 0x1 ))
                return  j + (i*4);
    }
    return -1;
}

char Keypad::getKey()
{
    int k = getKeyIndex();
    if(k != -1)
        return  keys[k];

    return '*';
}
