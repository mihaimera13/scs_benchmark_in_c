#include "sorting.h"
#include <algorithm>


 void swap(unsigned int &a, unsigned int &b)
{
    unsigned int aux = a;
    a = b;
    b = aux;
}

///BubbleSort
void bubbleSort(unsigned int v[],int n)
{
    bool ok;
    do
    {
        ok = 0;
        for(int i=0; i<n-1; i++)
        {
            if(v[i]>v[i+1])
            {
                ok = 1;
                swap(v[i],v[i+1]);
            }
        }

    }
    while(ok);
}

///InsertionSort
void insertionSort(unsigned int v[],int n)
{
    int j;
    for(int i=1; i<n; i++)
    {
        int foo = v[i];
        for(j=i-1; j>=0 && foo<v[j]; j--)
            v[j+1] = v[j];
        v[j+1] = foo;
    }

}

///SelectionSort
void selectionSort(unsigned int v[],int n)
{
    int foo;
    for(int i=0; i<n-1; i++)
    {
        foo = i;
        for(int j=i; j<n; j++)
            if(v[j]<v[foo])
                foo = j;

        if(foo!=i)
            swap(v[i],v[foo]);
    }
}

///Merge arrays
void mergeArrays(unsigned int x[], unsigned int y[],int n,int m,unsigned int c[])
{
    int i = 0, j = 0;
    int p = 0;
    while(i < n && j < m)
        if(x[i] < y[j])
            c[p ++] = x[i ++];
        else
            c[p ++] = y[j ++];
    while(i < n)
        c[p ++] = x[i ++];
    while(j < m)
        c[p ++] = y[j ++];
}
