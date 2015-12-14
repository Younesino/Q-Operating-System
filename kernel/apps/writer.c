#include "writer.h"

void printChar(char printData, uint16* curX, uint16* curY)
{
    if(printData == '\t')
    {
        uint8 insCount = *curX % 4;
        insCount = insCount == 0 ? 4 : insCount;
        while(insCount-- > 0)
        {
            // The brackets are meant to be there. DO NOT DELETE
            printCharAt(' ', black, (*curX)++ % sw, *curY);
        }
    }
    else if(printData == '\n')
    {
        // The bracket thing:
        // *ptr++ increments pointer -> derefernces (shorthand for ptr++; *ptr)
        // (*ptr)++ derefernces pointer -> increments (shorthand for *ptr += 1)
        (*curY)++; *curX = 1;
    }
    else
    {
        printCharAt(printData, black, (*curX)++ % sw, *curY);
    }
}

void appendln(strbuilder_t* data, uint16* curX, uint16* curY, uint32* index)
{
    (*curY)++; *curX = 0;
    strbuilder_insertc(data, '\n', (*index)++);
}

void writerHelp()
{
	newline();
	print("Showing help for writer:",magenta);
	newline();
	print("Writer is a simple text editor built into Q OS\n",yellow);
	print("Your text will probably be saved when you exit with Control + Z\n",yellow);
	print("To start a new document simply type 'writer new'\n",yellow);
	print("To access your last document type 'writer' with no arguments.\n",yellow);
	print("After the first line of text saved your document may be shown unformatted.\n",yellow);
	print("Press 'i' after opening writer to start editing your document.",yellow);
}

string initWriter()
{
	string vidmem = (string) 0xb8000;
    char oldmem[strlen(vidmem)];
    strcpy(oldmem, vidmem);

	paintScreen(screen_color);

	drawFrame(header_background, 0, 0, 80, 4);
	printAt("Q OS Text Editor\r\n", header_foreground, 1, 1);
	printAt("Simple Text Editor built for Q OS by Raph Hennessy & Plankp T",desc_foreground,1,2);

	drawBorder(screen_background, 0, 4, 80, sh - 1);

	cursorY = 5;
	cursorX = 1;
	updateCursor();

    // Checking user's capslock state
    bool inCmdMode = true, shiftDown = false, capslDown = capslock;
    uint16 curX = 1, curY = 5;
    uint32 index = 0;
    static int cmdKeys[] = {0x10 /*Q*/, 0x17 /*I*/, 0x18 /*O*/, 0x3A /*<CAPS>*/};
    strbuilder_t data = strbuilder_init();
    int k;
    while(true)
    {
        cursorX = curX % sw;
        cursorY = curY;
        updateCursor();

		if (curX > 79)
		{
			curX = 1;
			curY++;
		}

		if (curY < 5)
		{
			curY = 5;
			curX = 1;
		}
        // The trailing spaces clears out junky characters! Keep them
        printAt(inCmdMode ? "CMD     " : "INS     ", black, 2, 24);
        printAt(itos10(curX - 1), black, 6, 24);
        printAt(":     ", black, 9, 24);
        printAt(itos10(curY - 5), black, 11, 24);
        if(inCmdMode)
        {
            k = waitUntilKey(cmdKeys);
            switch(k)
            {
            case 0x10:
                goto end;
            case 0x17:
                inCmdMode = false;
                break;
            case 0x3A:
                capslDown = !capslDown;
                break;
            case 0x18:
                appendln(&data, &curX, &curY, &index);
                inCmdMode = false;
                break;
            }
        } else {
            k = getAnyKey();
            char charInput = ' ';
            switch(k)
            {
            case 0x01:
                inCmdMode = true;
                break;
            case 0x2A:
            case 0x36:
                shiftDown = true;
                break;
            case 0xAA:
            case 0xB6:
                shiftDown = false;
                break;
            case 0x3A:
                capslDown = !capslDown;
                break;
            case 0x1C:
                appendln(&data, &curX, &curY, &index);
                break;
            case 0x48:
                curY--;
                break;
            case 0x4B:
                curX--;
                index--;
                break;
            case 0x4D:
                curX++;
                index++;
                break;
            case 0x50:
                curY++;
                break;
            case 0x0E:
            {
                strbuilder_rmchar(&data, index);
                // Hahaha... RE-PRINT THE ENTIRE STRB!!!
                // Its all warped strings fault... :(
                curX = 1; curY = 5;
                paintLine(blue, 1, curY, sw - 1);
                for(uint16 loopi = 0; loopi < data.ilist.size; loopi++) {
                    printChar(strbuilder_charAt(data, loopi), &curX, &curY);
                }
                index--;
                break;
            }
            default:
                if(k < 59 && k > 0)
                {
                    if(shiftDown && !capslDown)
                    {
                        charInput = kbShiftChars[k];
                    }
                    else if(capslDown && !shiftDown)
                    {
                        charInput = kbCapslchars[k];
                    }
                    else if(shiftDown && capslDown)
                    {
                        charInput = kbSCModchars[k];
                    }
                    else
                    {
                        charInput = kbLowerChars[k];
                    }
                    if(charInput == 0)
                    {
                        break;
                    }
                    strbuilder_insertc(&data, charInput, index++);
                    // Hahaha... RE-PRINT THE ENTIRE STRB!!!
                    curX = 0; curY = 0;
                    uint16 oldX = 0, oldY = 0, indexClone = index;
                    paintLine(blue, 0, curY, sw);
                    for(uint16 loopi = 0; loopi < data.ilist.size; loopi++) {
                        printChar(strbuilder_charAt(data, loopi), &curX, &curY);
                        if(curY > oldY)
                        {
                            indexClone -= oldX + 1;
                        }
                        oldY = curY;
                        oldX = curX;
                    }
                    curX = indexClone;
                }
                break;
            }
        }
    }
end: // Sorry for the mom spaghetti code
    // Must be last line (before any prints)
    strcpy(vidmem, oldmem);
    string msg = strbuilder_tostr(data);
    if(msg == NULL)
    {
        msg = "";
    }
    strbuilder_destroy(&data);
    capslock = capslDown;
    cursorX = 1;
	cursorY = 5;
    print(msg, black);
    return msg;
}

void writer(string args)
{
	if (streql(splitArg(args, 1),"-h"))
	{
		writerHelp();
	}
	else if (streql(splitArg(args, 1),"new"))
	{
		writing = true;
		writerContents = initWriter();
		writing = false;
	}
	else
	{
	    initWriter();

		writing = true;
		printAt(writerContents,white,1,5);
		writing = false;
	}
}
