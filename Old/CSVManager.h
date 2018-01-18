#ifndef CSVMANAGER_H
#define CSVMANAGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

class CSVManager
{
public:
    CSVManager();
    ~CSVManager();

private:
    vector<vector<string>> m_Text;

public:
    void addInformation(int size_x, int size_y, string type, double time);

    friend fstream& operator>>(fstream& fin, CSVManager& manager)
    {
        string line;
        while (getline(fin, line))
        {
                vector<string> row;
                stringstream iss(line);
                string val;

                while (getline(iss, val, ';'))
                {
                    row.push_back(val);
                }
                manager.m_Text.push_back(row);
        }

        return fin;
    }

    friend fstream& operator<<(fstream& fon, const CSVManager& manager)
    {
        for (int i = 0; i < manager.m_Text.size(); i++)
        {
            for (int j = 0; j < manager.m_Text.at(i).size(); j++)
            {
                fon << manager.m_Text.at(i).at(j) + ";";
            }
            fon << "\n";
        }

        return fon;
    }

};


#endif // CSVMANAGER_H
