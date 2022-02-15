// referenced geeks for geeks:
// https://www.geeksforgeeks.org/optimal-page-replacement-algorithm/
// https://www.geeksforgeeks.org/program-for-least-recently-used-lru-page-replacement-algorithm/

#include <iostream>
#include <vector>
#include <bits/stdc++.h>
using namespace std;

// pagelist = list of integers
// f = number of frames
// toprint = do we print things?

// search function
bool search(int key, vector<int>& fr)
{
    for(int i = 0; i < fr.size(); i++)
    {
        if(fr[i] == key)
            return true;
    }
    return false;
}

// prediction function for optimal
int predict(vector<int>& pg, vector<int>& fr, int index)
{
    int res = -1;
    int farthest = index;
    for(int i = 0; i < fr.size(); i++)
    {
        int j;
        for(j = index; j < pg.size(); j++)
        {
            if(fr[i] == pg[j])
            {
                if(j > farthest)
                {
                    farthest = j;
                    res = i;
                }
                break;
            }
        }

        // if page never referenced in future, return it
        if(j == pg.size())
            return i;
    }

    // if all frames not in future, then return 0
    // else return res
    return (res == -1) ? 0 : res;
}

// optimal algorithm
int Optimal(vector<int>& pagelist, int f, bool toPrint = false)
{
    int faults = 0;
    int hits = 0;
    // create frame list and init it as empty
    vector<int> frames;

    // look through list to see if page is on a frame
    for(int i = 0; i < pagelist.size(); i++)
    {
        if(toPrint)
        {
            cout << "Page " << pagelist[i] << " accessed. ";
        }

        if(search(pagelist[i], frames))
        {
            hits++;
            if(toPrint)
            {
                cout << endl;
            }
            continue;
        }
        if(!(search(pagelist[i], frames)) && toPrint){
            cout << "Page fault. ";
        }

        // if space available in frames
        if(frames.size() < f)
        {
            frames.push_back(pagelist[i]);
        } else {
            int j = predict(pagelist, frames, i+1);
            // print replacement
            if(toPrint)
            {
                cout << "Page " << frames[j] << " replaced";
            }
            frames[j] = pagelist[i];
        }
        if(toPrint)
        {
            cout << endl;
        }
    }


    // return number of page faults
    faults = pagelist.size() - hits;
    return faults;
}

// LRU algorithm

int LRU(vector<int>& pagelist, int f, bool toPrint = false)
{
    int faults = 0;
    // create frame list and init it as empty
    unordered_set<int> frames;
    // store LRU indexes of pages
    unordered_map<int, int> indexes;
    for(int i = 0; i < pagelist.size(); i++)
    {
        // if to print
        if(toPrint)
        {
            cout << "Page " << pagelist[i] << " accessed. ";
        }

        // check if frames can have more pages
        if(frames.size() < f)
        {
            // add page to frames if not present - page fault
            if(frames.find(pagelist[i]) == frames.end())
            {
                frames.insert(pagelist[i]);
                faults++;
                if(toPrint)
                {
                    cout << "Page fault. ";
                }
            }

            // store recently used index
            indexes[pagelist[i]] = i;

        // if frames is full, then perform LRU
        } else {
            // check if already present in frames
            if(frames.find(pagelist[i]) == frames.end())
            {
                int lru = INT_MAX;
                int val;
                // using an iterator
                for(auto it = frames.begin(); it != frames.end(); it++)
                {
                    if(indexes[*it] < lru)
                    {
                        lru = indexes[*it];
                        val = *it;
                    }
                }

                // remove the page
                if(toPrint)
                {
                    cout << "Page fault. Page " << val << " replaced";
                }
                frames.erase(val);

                // insert page
                frames.insert(pagelist[i]);
                faults++;
            }

            // update current page index
            indexes[pagelist[i]] = i;

        }
        if(toPrint)
        {
            cout << endl;
        }
    }

    // return number of page faults
    return faults;
}

// int main()
// {
//     vector<int> pagelist = {1, 2, 3, 1, 2, 4, 1, 5, 1, 37, 4};

//     cout << "LRU" << endl;
//     LRU(pagelist, 3, true);
//     cout << endl;
//     cout << "optimal" << endl;
//     Optimal(pagelist, 3, true);
//     cout << endl;
//     cout << "end" << endl;
//     return 0;
// }