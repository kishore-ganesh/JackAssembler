#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#define A_COMMAND 0
#define C_COMMAND 1
#define L_COMMAND 2
#define INVALID_COMMAND 3
#define COMMENT 4

using namespace std;

string trim(string input)
{
    int l = 0;
    int r = input.size() - 1;
    while (input[l] == ' ' || input[l] == '\r')
    {
        l++;
    }

    while (input[r] == ' ' || input[r] == '\r')
    {
        r--;
    }

    int length = r - l + 1;

    return input.substr(l, length);
}

class SymbolTable
{
    map<string, string> Table;

  public:
    SymbolTable()
    {
        Table["SCREEN"] = "16384";
        Table["KBD"] = "24576";
        for (int i = 0; i < 16; i++)
        {
            string Register = "R" + to_string(i);
            Table[Register] = to_string(i);
        }

        Table["SP"] = "0";
        Table["LCL"] = "1";
        Table["ARG"] = "2";
        Table["THIS"] = "3";
        Table["THAT"] = "4";
    }

    bool contains(string key)
    {
        key = trim(key);
        return (Table.find(key) != Table.end());
    }

    string getAddress(string key)
    {
        key = trim(key);
        return Table[key];
    }

    void addEntry(string key, string value)
    {
        key = trim(key);
        Table[key] = value;
    }
};

class Parser
{
    string currentCommand;
    int currentCommandType;
    int firstFree;
    ifstream instream;
    SymbolTable table;

  public:
    Parser(char *path)
    {
        instream.open(path);
        firstFree=16;
    }

    bool isNotNumber(string sym)
    {
        for (int i = 0; i < sym.size(); i++)
        {
            if (!(sym[i] >= '0' && sym[i] <= '9'))
            {
                return true;
            }
        }

        return false;
    }
    void firstPass()
    {
        int instructionCount = 0;
        while (hasMoreCommands())
        {
            advance();
            if (commandType() != INVALID_COMMAND&&commandType()!=COMMENT)
            {
                

                if (commandType() == L_COMMAND)
                {

                    string sym = symbol();
                    if (isNotNumber(sym))
                    {
                        // if (!table.contains(sym))
                        // {

                            cout<<sym<<" "<<instructionCount<<endl;
                            table.addEntry(sym, to_string(instructionCount));
                    
                        // }
                    }
                }

                else{
                    instructionCount++;
                }
            }
        }

        instream.clear();
        instream.seekg(0);
    }

    bool hasMoreCommands()
    {
        return !instream.eof();
    }

    void advance()
    {
        getline(instream, currentCommand);
        currentCommand=trim(currentCommand);
        // cout << "C" << currentCommand << endl;
    }

    int commandType()
    {
        if (currentCommand[0] == '/' && currentCommand[1] == '/')
        {
            currentCommandType = COMMENT;
            return COMMENT;
        }
        if (currentCommand.find("@") != -1)
        {
            currentCommandType = A_COMMAND;
            return A_COMMAND;
        }

        else if (currentCommand.find("=") != -1 || currentCommand.find(";") != -1)
        {
            currentCommandType = C_COMMAND;
            return C_COMMAND;
        }

        else if (currentCommand[0] == '(' && currentCommand[currentCommand.size() - 1] == ')')
        {
            currentCommandType = L_COMMAND;
            return L_COMMAND;
        }

        else
        {
            currentCommandType = INVALID_COMMAND;
            return INVALID_COMMAND;
        }
    }

    string symbol()
    {

        if (currentCommandType == A_COMMAND)
        {

            string sym = currentCommand.substr(1);

            if(!isNotNumber(sym)){
                return sym;
            }
            if (!table.contains(sym))
            {
                table.addEntry(sym, to_string(firstFree));
                firstFree++;
            }

               return table.getAddress(sym);
        
            
        }

        else if (currentCommandType == L_COMMAND)
        {
            int substrSize = currentCommand.size() - 2;
            string sym = currentCommand.substr(1, substrSize);
            return sym;
        }

        else
        {
            return "INVALID";
        }
    }

    string dest()
    {
        if (currentCommandType == C_COMMAND)
        {
            string destString = "";
            int index = currentCommand.find("=");
            if (index != -1)
            {
                return currentCommand.substr(0, index);
            }
            else
            {
                return "";
            }
        }

        else
        {
            return "INVALID";
        }
    }

    string comp()
    {
        if (currentCommandType == C_COMMAND)
        {
            int rindex = currentCommand.find(";");
            int lindex = currentCommand.find("=");
            if (lindex == -1)
            {
                return currentCommand.substr(0, rindex);
            }

            else if (rindex == -1)
            {
                return currentCommand.substr(lindex + 1);
            }

            else
            {
                return currentCommand.substr(lindex + 1, rindex - lindex - 1);
            }
        }

        else
        {
            return "INVALID";
        }
    }

    string jump()
    {
        if (currentCommandType == C_COMMAND)
        {
            int index = currentCommand.find(";");
            if (index != -1)
            {
                return currentCommand.substr(index + 1);
            }
            else
            {
                return "";
            }
        }

        else
        {
            return "INVALID";
        }
    }
};

class Code
{

  public:
    string dest(string mnemonic)
    {
        string destCode = "000";
        if (mnemonic.find("M") != -1)
        {
            destCode[2] = '1';
        }

        if (mnemonic.find("D") != -1)
        {
            destCode[1] = '1';
        }

        if (mnemonic.find("A") != -1)
        {
            destCode[0] = '1';
        }

        return destCode;
    }
    string comp(string mnemonic)
    {
        // cout<<mnemonic<<" DE"<<endl;
        mnemonic = trim(mnemonic);
        if (mnemonic == "0")
        {
            return "0101010";
        }

        else if (mnemonic == "1")
        {
            return "0111111";
            ;
        }

        else if (mnemonic == "-1")
        {
            return "0111010";
        }

        else if (mnemonic == "D")
        {
            return "0001100";
        }

        else if (mnemonic == "A")
        {
            return "0110000";
        }

        else if (mnemonic == "M")
        {
            return "1110000";
        }

        else if (mnemonic == "!D")
        {
            return "0001101";
        }

        else if (mnemonic == "!A")
        {
            return "0110001";
        }

        else if (mnemonic == "!M")
        {
            return "1110001";
        }
        else if (mnemonic == "-D")
        {
            return "0001111";
        }

        else if (mnemonic == "-A")
        {
            return "0110011";
        }
        else if (mnemonic == "-M")
        {
            return "1110011";
        }
        else if (mnemonic == "D+1")
        {
            return "0011111";
        }
        else if (mnemonic == "A+1")
        {
            return "0110111";
        }
        else if (mnemonic == "M+1")
        {
            return "1110111";
        }
        else if (mnemonic == "D-1")
        {
            return "0001110";
        }
        else if (mnemonic == "A-1")
        {
            return "0110010";
        }
        else if (mnemonic == "M-1")
        {
            return "1110010";
        }
        else if (mnemonic == "D+A")
        {
            return "0000010";
        }
        else if (mnemonic == "D+M")
        {
            return "1000010";
        }
        else if (mnemonic == "D-A")
        {
            return "0010011";
        }
        else if (mnemonic == "D-M")
        {
            return "1010011";
        }
        else if (mnemonic == "A-D")
        {
            return "0000111";
        }
        else if (mnemonic == "M-D")
        {
            return "1000111";
        }
        else if (mnemonic == "D&A")
        {
            return "0000000";
        }
        else if (mnemonic == "D&M")
        {
            return "1000000";
        }
        else if (mnemonic == "D|A")
        {
            return "0010101";
            ;
        }
        else if (mnemonic == "D|M")
        {
            return "1010101";
            ;
        }
    }
    string jump(string mnemonic)
    {
        string jmpCode = "000";
        mnemonic = trim(mnemonic);
        if (mnemonic.find("G") != -1)
        {
            jmpCode[2] = '1';
        }

        if (mnemonic.find("L") != -1)
        {
            jmpCode[0] = '1';
        }

        if (mnemonic.find("E") != -1)
        {
            jmpCode[1] = '1';
        }

        if (mnemonic == "JNE")
        {
            jmpCode = "101";
        }

        if (mnemonic == "JMP")
        {
            jmpCode = "111";
        }

        return jmpCode;
    }

    string toBinary(string numstr)
    {
        string output = "";
        int number = stoi(numstr);
        while (number != 0)
        {
            string bit = number % 2 == 0 ? "0" : "1";
            output += bit;
            number /= 2;
        }

        if (output.size() < 15)
        {
            int deficit = 15 - output.size();
            for (int i = 0; i < deficit; i++)
            {
                output += "0";
            }
        }
        reverse(output.begin(), output.end());
        return output;
    }
};

//Parser contains first and second pass
class FileWriter
{
    ofstream outstream;

  public:
    FileWriter(char *path)
    {
        outstream.open(path);
    }

    void writeLine(string line)
    {
        outstream << line;
        outstream << "\n";
    }
};

int main(int argc, char *argv[])
{
    char *inputPath = argv[1];
    char *outputPath = argv[2];

    Parser parser(inputPath);
    FileWriter writer(outputPath);
    Code code;

    parser.firstPass();

    while (parser.hasMoreCommands())
    {
        parser.advance();
        string instruction = "";

        if (parser.commandType() == A_COMMAND)
        {
            string number = parser.symbol();
            string binaryNumber = code.toBinary(number);
            instruction = "0" + binaryNumber;

            // cout<<parser.symbol()<<endl;
        }

        if (parser.commandType() == C_COMMAND)
        {
            string dest = parser.dest();
            string comp = parser.comp();
            string jump = parser.jump();

            string destCode = code.dest(dest);
            string compCode = code.comp(comp);
            string jumpCode = code.jump(jump);
            // cout<<comp<<endl;
            // cout<<compCode<<endl;
            instruction = "111" + compCode + destCode + jumpCode;

            // cout<<parser.dest()<<endl;
            // cout<<parser.comp()<<endl;
            // cout<<parser.jump()<<endl;
        }

        if (instruction.size() == 16)
        {
            writer.writeLine(instruction);
        }
    }
}
