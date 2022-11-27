#include "_CSSRosterChange.h"

#include "stdafx.h"
#include "_CSSRosterChange.h"

void WriteByteVec(std::vector<unsigned char> Bytes, u32 Address, unsigned char addressReg, unsigned char manipReg, std::size_t numToWrite)
{
	SetRegister(addressReg, Address); // Load destination address into register

	numToWrite = std::min(numToWrite, Bytes.size()); // Set numToWrite to whichever is smaller, the passed in value or the size of the vec.

	unsigned long fullWordCount = numToWrite / 4; // Get the number of 4 byte words we can make from the given vec.
	unsigned long currWord = 0;
	unsigned long offsetIntoBytes = 0;

	// For each word we can make...
	for (unsigned long i = 0; i < fullWordCount; i++)
	{
		// ... grab it, then store it in our manip register.
		currWord = lava::bytesToFundamental<unsigned long>(Bytes.data() + offsetIntoBytes);
		SetRegister(manipReg, currWord);
		// Then write a command which stores the packed word into the desired location.
		STW(manipReg, addressReg, offsetIntoBytes);
		offsetIntoBytes += 0x04;
	}
	// For the remaining few bytes...
	for (offsetIntoBytes; offsetIntoBytes < numToWrite; offsetIntoBytes++)
	{
		// ... just write them one by one, no other choice really.
		SetRegister(manipReg, Bytes[offsetIntoBytes]);
		STB(manipReg, addressReg, offsetIntoBytes);
	}
}

void cssRosterChange() // Adapted from CSS Roster Change via Code Menu [QuickLava]
{
	// If CSS Rosters are enabled
	if (CSS_VERSION_SETTING_INDEX != -1)
	{
		int reg1 = 6;
		int reg2 = 5;
		unsigned long charListAddress = 0x80680DE0;
		unsigned long randListAddress = 0x80680E80;
		unsigned long randomRangeStoreLocation = 0x800017C0;

		ASMStart(0x80682928); // Hooks the second instruction of "__ct/[muSelCharTask]/mu_selchar.o".
		SetRegister(reg1, CSS_VERSION_SETTING_INDEX); // Load the location of the CSS Roster Line into our first register.
		LWZ(reg2, reg1, Line::VALUE); // Then Look 0x08 past that address to get the selected index of the CSS Roster Line

		// There needs to at least be one pair of lists for every defined option in the Code Menu line!
		assert(ROSTER_LIST.size() <= ROSTER_CHAR_ID_LISTS.size() && ROSTER_LIST.size() <= ROSTER_RAND_ID_LISTS.size());

		for (std::size_t i = 0; i < ROSTER_LIST.size(); i++) // For each Roster...
		{
			If(reg2, EQUAL_I, i); // ... add a case for that index...
			{
				WriteByteVec(ROSTER_CHAR_ID_LISTS[i], charListAddress, reg1, reg2); // ... which overwrites the char list...
				WriteByteVec(ROSTER_RAND_ID_LISTS[i], randListAddress, reg1, reg2); // ... and the random char list.
				SetRegister(reg1, randomRangeStoreLocation);		// \ 
				SetRegister(reg2, ROSTER_RAND_ID_LISTS[i].size());	// |
				STW(reg2, reg1, 0x00);								// / Additionally, store the length of the random list for later.
			}EndIf();
		}
		ASMEnd(0x7c0802a6); // Restore the instruction replaced by the branch; mflr	r0.

		reg1 = 23;
		ASMStart(0x806857F0); // Hooks the instruction which sets the range for random character rolls.
		SetRegister(reg1, randomRangeStoreLocation); // Load the address we stored the random character count in before...
		LWZ(reg1, reg1, 0x00); // ... and load the number stored there into r23.
		ASMEnd();
	}
}