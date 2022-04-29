#include <bitset>
#include <fstream>
#include <conio.h>
#include <iostream>

#define BIN_FILE_PATH "testFile.bin"
#define NUM_IN_PAGE 128

using namespace std;

struct Page
{
	int *number = new int[NUM_IN_PAGE];
	bitset<NUM_IN_PAGE> bitmap;
};

class VirtualArray
{

private:

	Page *currentPage = new Page;
	fstream file;
	int currentPageIndex = 0;
	int getPageNumFromIndex(int index) { return index / NUM_IN_PAGE; }
	int getPageNumOffsetFromIndex(int index) { return index % (NUM_IN_PAGE + (NUM_IN_PAGE / 4)); }
	//int getPageBegin(int pageNum) { return 3 + (NUM_IN_PAGE * sizeof(int) * (pageNum + 1) + ((NUM_IN_PAGE / 8) * pageNum)); } //returns amount of bytes from file begin to requested page
	int getPageBegin(int pageNum) { return 2 + (NUM_IN_PAGE * sizeof(int) * (pageNum) + ((NUM_IN_PAGE / 8) * pageNum)); } //returns amount of bytes from file begin to requested page
	//int getBitmapBegin(int pageNum) { return 3 + ((NUM_IN_PAGE * sizeof(int) * (pageNum + 1)) + ((NUM_IN_PAGE / 8) * pageNum)); } //returns same but for bitmap
	int getBitmapBegin(int pageNum) { return 2 + ((NUM_IN_PAGE * sizeof(int) * (pageNum + 1)) + ((NUM_IN_PAGE / 8) * pageNum)); } //returns same but for bitmap

public:

	void createVirtualArray(int amountOfPages = 1)
	{

		if (!file.is_open()) file.open(BIN_FILE_PATH, ios::binary | ios::out);
		currentPageIndex = 0;
		currentPage->number = new int[NUM_IN_PAGE];
		file << 'V' << 'A'; //FILE MASK

		file.flush();

	}

	void writePage(Page pageToWrite, int pageIndex)
	{

		char* intBytes = new char[512]; //char array for int to char conversion

		cout << "Page begins with " << getPageBegin(pageIndex) << " byte\n";
		cout << "Page bitmap begins with " << getBitmapBegin(pageIndex) << " byte\n";

		file.seekg(getPageBegin(pageIndex));
		memcpy_s(intBytes, sizeof(intBytes), pageToWrite.number, sizeof(pageToWrite.number));

		for (int i = 0; i < NUM_IN_PAGE * sizeof(int); i += 4) { file << intBytes[i] << intBytes[i + 1] << intBytes[i + 2] << intBytes[i + 3]; }
		for (int i = 0; i < NUM_IN_PAGE / 8; i++) { file << static_cast<char>(bitset<8>(pageToWrite.bitmap[i]).to_ulong()); }

		file.flush();
		delete[] intBytes;

	}

	void loadPage(int pageToLoadIndex)
	{

		if(pageToLoadIndex == currentPageIndex) { return; }
		char* buffer = new char[NUM_IN_PAGE * 4];
		bitset<8> charBuffer;

		file.seekg(getPageBegin(pageToLoadIndex));
		file.read(buffer, NUM_IN_PAGE * 4);

		memcpy_s(buffer, sizeof(buffer), currentPage->number, sizeof(currentPage->number));

		file.seekg(getBitmapBegin(pageToLoadIndex));
		file.read(buffer, NUM_IN_PAGE / 8);

		for (int i = 0; i < NUM_IN_PAGE / 8; i++) 
		{
			charBuffer = buffer[i];
			for (int j = 0; j < 8; j++) { currentPage->bitmap[j + (i * 8)] = charBuffer[j]; }
		}
		
		delete[] buffer;


	}

	void writeNumberToPage(Page pageToWriteTo, int numberIndex, int numberToWrite) 
	{

		numberIndex %= NUM_IN_PAGE;
		pageToWriteTo.number[numberIndex] = numberToWrite; 
		pageToWriteTo.bitmap[numberIndex] = 1;

	}

	void swapPages(int pageToLoadIndex)
	{
		cout << "\nSwapping!\n";
		writePage(*currentPage, currentPageIndex);
		currentPageIndex = pageToLoadIndex;
		currentPage = new Page;
		loadPage(pageToLoadIndex);

	}

	void writeNumberToArray(int inputNum, int index)
	{

		cout << "\nCurrent page index: " << currentPageIndex << ", trying to write to page " << getPageNumFromIndex(index);
		if (getPageNumFromIndex(index) != currentPageIndex) swapPages(getPageNumFromIndex(index));
		writeNumberToPage(*currentPage, index, inputNum);

	}

	int readNumberFromArray(int index)
	{

		cout << "\nCurrent page index: " << currentPageIndex << ", trying to read from page " << getPageNumFromIndex(index);
		if (getPageNumFromIndex(index) != currentPageIndex) swapPages(getPageNumFromIndex(index));
		index %= NUM_IN_PAGE;
		return currentPage->number[index];

	}

	void printPageInfo()
	{

		cout << endl;
		for (int i = 0; i < NUM_IN_PAGE; i++) { cout << i + (NUM_IN_PAGE * currentPageIndex) << ". " << currentPage->number[i] << " | " << currentPage->bitmap[i] << endl; }

	}

	void closeFileStream()
	{

		file.flush();
		file.close();

	}

};

void virtualArrayMenu()
{

	VirtualArray memArray;
	int num, index;

	do
	{

		cout << "\nVirtual array\n1. Create virtual array.\n2. Write to array.\n3. Read from array.\n4. Print array info.\n5. Exit.";

		switch (_getch())
		{

		case('1'):
			
			memArray.createVirtualArray();
			cout << "\nVirtual array created!\n";
			break;

		case('2'):
			
			cout << "\nInteger to write: ";
			cin >> num;
			cout << "Index: ";
			cin >> index;

			memArray.writeNumberToArray(num, index);
			cout << "\nWrite complete!\n";

			break;

		case('3'):

			cout << "\nIndex of integer to read: ";
			cin >> index;
			cout << "\nIndex " << index << " number " << memArray.readNumberFromArray(index) << endl;

			break;

		case('4'):
			memArray.printPageInfo();
			break;

		case('5'):
			memArray.closeFileStream();
			return;

		default:
			cout << "\nWrond input!\n";
			break;

		}

	} while (1);

}

int main()
{

	virtualArrayMenu();

}

//SAVING AND READING STILL DOES NOT WORK