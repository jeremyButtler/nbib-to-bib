// This is just some code I do not want to discard yet. 
// becaue it has some notes or ideas that I may be able
// to use later.

unsigned short log2US(
    unsigned short numInUS // Number to find log2 of
){ /******************************************************\
   ' Fun-05 Sec-01 Sub-01 TOC: log2US
   '  - Finds the log base 2 of an input number
   '  - from:
   '    http://graphics.stanford.edu/~seander/bithacks.html
   '  - This is a veriation that will be slower, but can
   '    handle a short being less or morre than 16 bits
   \******************************************************/

   register unsigned short retUS = 0;   // Return value
   register unsigned short shiftUS = 0; // For shifting
   register unsigned short
       halfShiftUS = (sizeof(unsigned short) << 2);
   register unsigned short
       checkUS =
           (~sizeof(unsigned short)+sizeof(unsigned short))
        >> halfShiftUS;

   /* Logic is:
      - ~sizeof + sizeof gets me the max unsigned short
        value
      - >> (sizeof() << 2) will remove the first half of
        the ones leaving me with 0xFF for a 16 bit short
  */

   register unsigned short
      shiftValUS = log2GetCntVal(sizeof(short));

   /* These pionts check to see if the value is greater
      than certain powers of two and shift out the checked
      higher bits.
        - numInUS > 0xFF... or 0x3 gives only a 1 or 0 if
          numInUS is greater then 0xN
        - the << N moves the bit to the highest return
          value postion. I am guessing that their are
          maxium values for the log2 return
        - retUS |= shiftUS adds the statements singe bit
          to retUS. It is likely here so that retUS does
          not get overwriten and does not mess up numInUS
      This then ends with retUS |= (numInUS >> 1). I am
      guesing at this point any bit left in numInUS is 
      a power of two. Not really sure though
   */

   halfShiftUS >> 2; // Reduce half shift down
   retUS = (numInUS > checkUS) << 3;
   numInUS >>= retUS; // Shift by value from retUS
   checkUS >>= halfShiftUS;

   while(checkUS != 0)
   { // While have shifting to do
       halfShiftUS >> 2; // Reduce half shift down
       shiftUS = (numInUS > checkUS) << 2;
       numInUS >>= shiftUS; // Shift by value from shiftUS
       retUS |= shiftUS;   // Store in shifted value
       checkUS >>= halfShiftUS;
   } // While have shifting to do

   shiftUS = (numInUS > 0x3) << 1;
   numInUS >>= shiftUS; // Shift by value from shiftUS
   retUS |= shiftUS;   // Store in shifted value

   return retUS |= (numInUS >> 1);
} // log2US

/*--------------------------------------------------------\
| Output: Returns value to start count by or 0 if invalid
\--------------------------------------------------------*/
unsigned char log2GetCntVal(
    unsigned char numBytesUC // Bytes in data type (sizeof)
){/******************************************************\
  ' Fun-06 TOC: Sec-01 Sub-01: log2GetCntVal
  '  - Returns the value to start count down for log2UX
  '    functions. Use sizeof(datatype) to get numBytesUC
  \******************************************************/

  switch(numBytesUC)
  { // Switch; check the number of bytes
    case 1: return 2;
    case 2: return 3;
    case 4: return 4;
    case 8: return 5;
    case 16: return 6;
    default: return 0; // No idea
  } // Switch; check the number of bytes
    
  return 0; // No idea
} // log2GetCntVal

unsigned long log2UL(
    unsigned long numInUL // Number to find log2 of
){ /******************************************************\
   ' Fun-03 Sec-01 Sub-01 TOC: log2UL
   '  - Finds the log base 2 of an input number
   '  - from:
   '    http://graphics.stanford.edu/~seander/bithacks.html
   \******************************************************/

   register unsigned long retUL;   // Return value
   register unsigned long shiftUL; // For shifting

   /* These pionts check to see if the value is greater
      than certain powers of two and shift out the checked
      higher bits.
        - numInUL > 0xFF... or 0x3 gives only a 1 or 0 if
          numInUL is greater then 0xN
        - the << N moves the bit to the highest return
          value postion. I am guessing that their are
          maxium values for the log2 return
        - retUL |= shiftUL adds the statements singe bit
          to retUL. It is likely here so that retUL does
          not get overwriten and does not mess up numInUL
      This then ends with retUL |= (numInUL >> 1). I am
      guesing at this point any bit left in numInUL is 
      a power of two. Not really sure though
   */

   retUL = (numInUL > 0xFFFFFFFF) << 5;
   numInUL >>= retUL; // Shift by value from retUL
  
   shiftUL = (numInUL > 0xFFFF) << 4;
   numInUL >>= shiftUL; // Shift by value from shiftUL
   retUL |= shiftUL;   // Store in shifted value

   shiftUL = (numInUL > 0xFF) << 3;
   numInUL >>= shiftUL; // Shift by value from shiftUL
   retUL |= shiftUL;   // Store in shifted value

   shiftUL = (numInUL > 0xF) << 2;
   numInUL >>= shiftUL; // Shift by value from shiftUL
   retUL |= shiftUL;   // Store in shifted value

   shiftUL = (numInUL > 0x3) << 1;
   numInUL >>= shiftUL; // Shift by value from shiftUL
   retUL |= shiftUL;   // Store in shifted value

   return retUL |= (numInUL >> 1);
} // log2UL

            /*********************************************\
            * Fun-03 Sec-3 Sub-3: 1 character after break
            \*********************************************/

            else if(*cntUS - lenPadUC == 1)
            { // Else if only one character after break
               swapCAry[0] = *(dupCStr - 1);
               dupCStr -= 2; // Move to white space

               // Add in the line break
               memcpy(dupCStr, lineBreakCStr, lenBreakUC);
               dupCStr += lenBreakUC; // Move past break

               // Add in the padding
               memcpy(dupCStr, padCStr, lenPadUC);
               dupCStr += lenPadUC; // Move past padding

               // Adjust the counters
               (*numCharInDupUL) +=
                   lenPadUC
                 + lenBreakUC
                 - 1; // -1 for the space I used

               *cntUS = lenPadUC;

                *dupCStr = swapCAry[0];
                ++dupCStr;
            } // Else if only one character after break


