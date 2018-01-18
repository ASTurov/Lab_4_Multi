#include "CSVManager.h"
#include <string>
#include <sstream>

CSVManager::CSVManager()
{

}

CSVManager::~CSVManager()
{
    m_Text.clear();
}

void CSVManager::addInformation(int size_x, int size_y, string type, double time)
{
    if (!m_Text.size() == 0)
    {
        int numberLine = -1;
        int column = -1;

        for (int i = 0; i < m_Text.size(); i++)
        {
            if(m_Text.at(i).at(0) == to_string(size_x) + "x" + to_string(size_y))
            {
                numberLine = i;
            }
        }

        for (int i = 0; i < m_Text.at(0).size(); i++)
        {
            if (m_Text.at(0).at(i) == type)
            {
                column = i;
            }
        }

        vector <string> line;
        if (column == -1)
        {
            m_Text.at(0).push_back(type);
            column = m_Text.size() - 1;
        }

        line.resize(m_Text.at(0).size());

        if (numberLine == -1)
        {
            line[0] =  to_string(size_x) + "x" + to_string(size_y);
            line[column] = to_string(time);
            m_Text.push_back(line);
        }
        else
        {
            m_Text.at(numberLine).resize(m_Text.at(0).size());
            m_Text.at(numberLine).at(column) = to_string(time);
        }
    }
    else
    {
        m_Text.resize(2);
        m_Text.at(0).push_back("Size");
        m_Text.at(0).push_back(type);
        m_Text.at(1).push_back(to_string(size_x) + "x" + to_string(size_y));
        m_Text.at(1).push_back(to_string(time));
    }
}
