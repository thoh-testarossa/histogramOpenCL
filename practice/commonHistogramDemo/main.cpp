//This is a demo histogram program without the usage of OpenCL
//It will be a standard to measure the performance of the parallel version histogram generation program

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstdlib>
#include <vector>

using namespace std;

#define DATASETSIZE 16384
#define USELESSFIELDLENGTH 16
#define HISTOGRAMINTERNALNUM 10

class data
{
public:
    data();
    void setOtherData(string otherData);
    void setValueToBeHistogramed(int value);
    int returnValue();
    string returnOtherData();
private:
    string *otherData;
    int valueToBeHistogramed;
};

class equalWidthHistogram
{
public:
    equalWidthHistogram(vector<data> &dataset, int internalNum);
    vector<vector<int>> returnHistogram();
private:
    int internalNum;
    int hInternal;
    int *internalMark;
    int *internalCount;
};

class equalHeightHistogram
{
public:
    equalHeightHistogram(vector<data> &dataset, int internalNum);
    vector<vector<int>> returnHistogram();
private:
    void datasetQsort(vector<data> &dataset, int s, int f);
    int internalNum;
    int hHeightLimit;
    int *internalMark;
    int *internalCount;
};

void data::setValueToBeHistogramed(int value)
{
    this->valueToBeHistogramed = value;
}

void data::setOtherData(string otherData)
{
    *this->otherData = otherData;
}

data::data()
{
    this->otherData = new string("");
    this->setOtherData((string("")));
    this->setValueToBeHistogramed(0);
}

int data::returnValue()
{
    return this->valueToBeHistogramed;
}

string data::returnOtherData()
{
    return *this->otherData;
}

data randomDataGeneration()
{
    data a = data();
    string uselessField = string("");
    for(int i = 0; i < USELESSFIELDLENGTH; i++)
        uselessField += (char)('A' + rand()%26);
    a.setOtherData(uselessField);
    int ab = rand()%32768;
    ab *= 32768;
    ab += rand()%32768;
    a.setValueToBeHistogramed(ab);
    return a;
}

vector<int> minmaxInDataset(vector<data> &dataset)
{
    vector<int> result = vector<int>();
    for(int i = 0; i < dataset.size(); i++)
    {
        if(i == 0)
        {
            result.push_back(dataset[i].returnValue());
            result.push_back(dataset[i].returnValue());
        }
        else
        {
            if(dataset[i].returnValue() < result[0]) result[0] = dataset[i].returnValue();
            else if(dataset[i].returnValue() > result[1]) result[1] = dataset[i].returnValue();
        }
    }
    return result;
}

equalWidthHistogram::equalWidthHistogram(vector<data> &dataset, int internalNum)
{
    //First, check the lower & upper frontiers of the dataset
    //At this stage we just do a linear scan for the whole dataset
    this->internalNum = internalNum;
    vector<int> minmax = minmaxInDataset(dataset);
    int maxInternal = minmax[1] - minmax[0], modifiedMaxInternal;
    //Second, generate the internal marks for the histogram
    int min, max;
    if(maxInternal % internalNum == 0)
    {
        modifiedMaxInternal = maxInternal;
        min = minmax[0];
    }
    else
    {
        modifiedMaxInternal = maxInternal - maxInternal % internalNum + internalNum;
        min = minmax[0] - (internalNum - maxInternal % internalNum) / 2;
    }
    //max = min + modifiedMaxInternal;
    this->hInternal = modifiedMaxInternal / internalNum;
    this->internalMark = new int [internalNum + 1];
    for(int i = 0; i <= internalNum; i++)
        internalMark[i] = min + i * (this->hInternal);
    //Third, calculate histogram
    this->internalCount = new int [internalNum];
    for(int i = 0; i < internalNum; i++)
        this->internalCount[i] = 0;
    for(int i = 0; i < dataset.size(); i++)
        this->internalCount[(dataset[i].returnValue() - min) / (this->hInternal)]++;
}

vector<vector<int>> equalWidthHistogram::returnHistogram()
{
    //Return the internal marks & counts of the histogram as a two-element vector of integer vectors.
    vector<vector<int>> result = vector<vector<int>>();
    vector<int> histogramInternalMark = vector<int>();
    vector<int> histogramCount = vector<int>();
    for(int i = 0; i <= this->internalNum; i++)
    {
        if(i < this->internalNum)
            histogramCount.push_back(this->internalCount[i]);
        histogramInternalMark.push_back(this->internalMark[i]);
    }
    result.push_back(histogramInternalMark);
    result.push_back(histogramCount);
    return result;
}

void equalHeightHistogram::datasetQsort(vector<data> &dataset, int s, int f)
{
    //Sort dataset by key "value" with qsort method
    if(s >= f);
    else
    {
        int c = s, d = f;
        data tmp;
        while(c < d)
        {
            while(c < d && dataset[c].returnValue() <= dataset[d].returnValue())
                c++;
            tmp = dataset[c];
            dataset[c] = dataset[d];
            dataset[d] = tmp;
            while(c < d && dataset[c].returnValue() <= dataset[d].returnValue())
                d--;
            tmp = dataset[c];
            dataset[c] = dataset[d];
            dataset[d] = tmp;
        }
        this->datasetQsort(dataset, s, c - 1);
        this->datasetQsort(dataset, d + 1, f);
    }
}

equalHeightHistogram::equalHeightHistogram(vector<data> &dataset, int internalNum)
{
    //First, find all k-equal diversion points.
    //How to find all k-equal diversion points?
    //Sort is an approach to do that but any other better ways?
    //In this program we just simply use qsort to solve this problem, a parallel sort algorithm is need
    int dsize = dataset.size();
    this->datasetQsort(dataset, 0, dsize - 1);
    //Second, generate the internal marks for the histogram
    this->internalNum = internalNum;
    this->internalMark = new int [internalNum + 1];
    this->internalMark[0] = dataset[0].returnValue();
    this->internalMark[internalNum] = dataset[dsize - 1].returnValue();
    for(int i = 1; i < internalNum; i++)
        this->internalMark[i] = dataset[(i * dsize) / internalNum].returnValue();
    this->internalMark[internalNum]++;
    //Third, calculate histogram
    this->internalCount = new int [internalNum];
    for(int i = 0; i < internalNum; i++)
        this->internalCount[i] = 0;
    //...but actually from the definition of ehh, we can use a sorted datasort to "calculate" histogram counts without scan the whole dataset
    //for(int i = 0; i < internalNum; i++)
        //internalCount[i] = ((i + 1) * dsize) / internalNum - (i * dsize) / internalNum;
    //This part also provide a scan method to check the answer calculated from sorted dataset
    int chk = 0;
    for(int i = 0; i < dsize; i++)
    {
        while(dataset[i].returnValue() >= this->internalMark[chk + 1])
            chk++;
        this->internalCount[chk]++;
    }
}

vector<vector<int>> equalHeightHistogram::returnHistogram()
{
    //Return the internal marks & counts of the histogram as a two-element vector of integer vectors.
    //The copy of the approach which equal-width histogram uses.
    vector<vector<int>> result = vector<vector<int>>();
    vector<int> histogramInternalMark = vector<int>();
    vector<int> histogramCount = vector<int>();
    for(int i = 0; i <= this->internalNum; i++)
    {
        if(i < this->internalNum)
            histogramCount.push_back(this->internalCount[i]);
        histogramInternalMark.push_back(this->internalMark[i]);
    }
    result.push_back(histogramInternalMark);
    result.push_back(histogramCount);
    return result;
}

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    //Initialize
    vector<data> dataset = vector<data>();
    data tmp = data();
    //Importing a dataset
    ifstream datain = ifstream("dataset.txt", ios::in);
    //Data generation part. Only executed when a dataset can't be found
    if(!datain.is_open())
    {
        srand(time(0));
        ofstream dataGen = ofstream("dataset.txt", ios::out);
        for(int i = 0; i < DATASETSIZE; i++)
        {
            tmp = randomDataGeneration();
            dataset.push_back(tmp);
            dataGen << tmp.returnOtherData() << " " << tmp.returnValue() << endl;
        }
        dataGen.close();
    }
    else
    {
        int value;
        string otherData = string("");
        while(datain >> otherData >> value)
        {
            tmp.setOtherData(otherData);
            tmp.setValueToBeHistogramed(value);
            dataset.push_back(tmp);
        }
    }
    //cerr << dataset[16383].returnOtherData() << endl;
    //Generate a equal-width histogram
    equalWidthHistogram ewh = equalWidthHistogram(dataset, HISTOGRAMINTERNALNUM);
    vector<vector<int>> result_ewh = ewh.returnHistogram();
    //Generate a equal-height histogram
    equalHeightHistogram ehh = equalHeightHistogram(dataset, HISTOGRAMINTERNALNUM);
    vector<vector<int>> result_ehh = ehh.returnHistogram();
    return 0;
}
