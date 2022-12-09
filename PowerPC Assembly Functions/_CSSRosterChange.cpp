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
		unsigned long charListAddress = 0x80680DE0;
		unsigned long randListAddress = 0x80680E80;
		unsigned long randomRangeStoreLocation = 0x800017C0;
		unsigned long temporaryRegBackupLocation = randomRangeStoreLocation + 0x04;

		// There needs to at least be one pair of lists for every defined option in the Code Menu line!
		assert(ROSTER_LIST.size() <= ROSTER_CHAR_ID_LISTS.size() && ROSTER_LIST.size() <= ROSTER_RAND_ID_LISTS.size());

		int reg1 = 6;
		int reg2 = 5;
		ASMStart(0x80682928); // Hooks the second instruction of "__ct/[muSelCharTask]/mu_selchar.o".
		SetRegister(reg1, CSS_VERSION_SETTING_INDEX); // Load the location of the CSS Roster Line into our first register.
		LWZ(reg2, reg1, Line::VALUE); // Then Look 0x08 past that address to get the selected index of the CSS Roster Line
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


		// Roster-Dependent L-Loads
		// There needs to at least be one pair of L-Loads for every defined option in the Code Menu line!
		assert(ROSTER_LIST.size() <= ROSTER_L_LOAD_PAIRS.size());

		reg1 = 29;
		reg2 = 15;
		ASMStart(0x8068482C);
		SetRegister(reg2, CSS_VERSION_SETTING_INDEX); // Load the location of the CSS Roster Line into our first register.
		LWZ(reg2, reg2, Line::VALUE); // Then Look 0x08 past that address to get the selected index of the CSS Roster Line
		for (std::size_t i = 0; i < ROSTER_LIST.size(); i++) // For each Roster...
		{
			If(reg2, EQUAL_I, i); // ... add a case for that index...
			{
				// ... in which we check against each specified L-Load...
				const std::vector<std::pair<unsigned char, unsigned char>>* currPairVec = &ROSTER_L_LOAD_PAIRS[i];
				for (std::size_t u = 0; u < currPairVec->size(); u++)
				{
					const std::pair<unsigned char, unsigned char>* currPair = &currPairVec->at(u);
					If(reg1, EQUAL_I, currPair->first);
					{
						// ... and set r29 if we find a match.
						SetRegister(reg1, currPair->second);
					}EndIf();
				}
			}EndIf();
		}
		SetRegister(reg2, 0);
		ASMEnd();

		reg1 = 5;
		reg2 = 3;
		ASMStart(0x80685BE4);
		SetRegister(reg2, CSS_VERSION_SETTING_INDEX); // Load the location of the CSS Roster Line into our first register.
		LWZ(reg2, reg2, Line::VALUE); // Then Look 0x08 past that address to get the selected index of the CSS Roster Line
		for (std::size_t i = 0; i < ROSTER_LIST.size(); i++) // For each Roster...
		{
			If(reg2, EQUAL_I, i); // ... add a case for that index...
			{
				// ... in which we check against each specified L-Load...
				const std::vector<std::pair<unsigned char, unsigned char>>* currPairVec = &ROSTER_L_LOAD_PAIRS[i];
				for (std::size_t u = 0; u < currPairVec->size(); u++)
				{
					const std::pair<unsigned char, unsigned char>* currPair = &currPairVec->at(u);
					If(reg1, EQUAL_I, currPair->second);
					{
						// ... and set r29 if we find a match.
						SetRegister(reg1, currPair->first);
					}EndIf();
				}
			}EndIf();
		}
		SetRegister(reg2, 0);
		ASMEnd(0x90a1003c); // Restore original instruction (stw r5, 0x3C(r1))

		reg1 = 3;
		reg2 = 12;
		int reg3 = 8;
		ASMStart(0x806847BC);
		SetRegister(reg2, temporaryRegBackupLocation); // Backup the contents of reg3
		STW(reg3, reg2, 0x00);
		SetRegister(reg3, 0x00);
		SetRegister(reg2, CSS_VERSION_SETTING_INDEX); // Load the location of the CSS Roster Line into our first register.
		LWZ(reg2, reg2, Line::VALUE); // Then Look 0x08 past that address to get the selected index of the CSS Roster Line
		for (std::size_t i = 0; i < ROSTER_LIST.size(); i++) // For each Roster...
		{
			If(reg2, EQUAL_I, i); // ... add a case for that index...
			{
				// ... in which we check against each specified L-Load...
				const std::vector<std::pair<unsigned char, unsigned char>>* currPairVec = &ROSTER_L_LOAD_PAIRS[i];
				for (std::size_t u = 0; u < currPairVec->size(); u++)
				{
					// ... and if we have a match...
					const std::pair<unsigned char, unsigned char>* currPair = &currPairVec->at(u);
					If(reg1, EQUAL_I, currPair->first);
					{
						// ... set reg3 to 1.
						SetRegister(reg3, 0x01);
					}EndIf();
				}
			}EndIf();
		}
		// Check if reg3 was set, this is used once we step back out of the branch.
		CMPI(reg3, 0x01, 0);
		SetRegister(reg2, temporaryRegBackupLocation);
		LWZ(reg3, reg2, 0x00); // Restore the contents of reg3
		ASMEnd();
	}
}