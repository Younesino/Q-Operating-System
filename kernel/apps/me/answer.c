#include "answer.h"

string answer(string question, uint8 offset)
{
    if (streql(question,"exit"))
    {
        querying = false;
        return "Terminated by User.";
    }
    else
    {
        bool over = false;
        int tmp = offset;

        newline();

        while (!over)
        {
            if (streql(splitArg(question, tmp),""))
            {
                over = true;
            }
            else
            {
                print(" | ",0x0A);
                print(toUpper(splitArg(question, tmp)),0x0D);
                print(" ",0x0D);
                print(sort(splitArg(question, tmp)),0x0D);
            }
            tmp++;
        }

        question = "qquestion>  ";
        return "Not Terminated.";
    }
}
