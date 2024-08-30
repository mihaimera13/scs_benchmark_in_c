#include <sys/stat.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "sorting.h"
#include <bitset>
#include <Windows.h>
#include <time.h>
#include <chrono>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef struct MyData {
    unsigned int* array;
    unsigned long len;
}MYDATA,*PMYDATA;

typedef struct Measurement {
    LARGE_INTEGER start;
    LARGE_INTEGER end;
}CHRONOMETER;

using namespace std;

DWORD WINAPI bubbleThread(LPVOID lpParam)
{
    PMYDATA param;
    param = (PMYDATA)lpParam;

    bubbleSort(param->array, param->len);

    return 0;
}


DWORD WINAPI insertionThread(LPVOID lpParam)
{
    PMYDATA param;
    param = (PMYDATA)lpParam;

    insertionSort(param->array, param->len);

    return 0;
}


DWORD WINAPI selectionThread(LPVOID lpParam)
{
    PMYDATA param;
    param = (PMYDATA)lpParam;

    selectionSort(param->array, param->len);

    return 0;
}


unsigned int nr_of_threads;
unsigned int data_size;
bool bubble_sort;
bool insertion_sort;
bool selection_sort;

int main(int argc, char *argv[])
{

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    CHRONOMETER full_program;

    QueryPerformanceCounter(&(full_program.start));

    time_t now = time(nullptr);
    srand(time(NULL));
    if (argc > 1)
    {
        nr_of_threads = atoi(argv[1]);
        data_size = atoi(argv[2]);
        switch (argc)
        {
        case 4:
            switch (atoi(argv[3]))
            {
            case 1:
                bubble_sort = true;
                break;
            case 2:
                insertion_sort = true;
                break;
            case 3:
                selection_sort = true;
                break;
            default: 
                cout<< "Something went wrong! 1";
                exit(1);
                break;
            }
            break;
        case 5:
            switch (atoi(argv[3]))
            {
            case 1:
                bubble_sort = true;
                break;
            case 2:
                insertion_sort = true;
                break;
            case 3:
                selection_sort = true;
            default:
                cout << "Something went wrong! 2";
                exit(1);
                break;
            }
            switch (atoi(argv[4]))
            {
            case 1:
                bubble_sort = true;
                break;
            case 2:
                insertion_sort = true;
                break;
            case 3:
                selection_sort = true;
                break;
            default:
                cout << "Something went wrong! 3";
                exit(1);
                break;
            }
            break;
        case 6:
            bubble_sort = true;
            insertion_sort = true;
            selection_sort = true;
            break;
        default:
            cout<<"Something went wrong! 4";
            exit(1);
            break;
        }
    }
    else
    {
        nr_of_threads = 1;
        data_size = 65536;
        bubble_sort = true;
        insertion_sort = true;
        selection_sort = true;
    }

    fstream output;
    fstream results;

    char filename[100] = "data";
    char nowtime[26];
    ctime_s(nowtime, sizeof nowtime, &now);
    nowtime[strlen(nowtime) - 1] = '\0';
    strcat_s(filename, sizeof filename, nowtime);
    strcat_s(filename, sizeof filename, ".txt");
    for (char& c : filename) {
        if (c == ' ' || c == ':') {
            c = '_';
        }
    }

    char filename2[100] = "results";
    strcat_s(filename2, sizeof filename2, nowtime);
    strcat_s(filename2, sizeof filename2, ".txt");
    for (char& c : filename2) {
        if (c == ' ' || c == ':') {
            c = '_';
        }
    }
    output.open(filename, ios::out);
    if (!output)
        exit(1);

    results.open(filename2, ios::out);
    if (!results)
        exit(1);

    fstream file;
    file.open("data_file.bin", ios::in | ios::out | ios::trunc | ios::binary);
    if (!file)
        exit(1);

    results << "Performance frequency: " << frequency.QuadPart << " counts/second" << endl;
    results << "Data size: " << data_size * sizeof(unsigned int) << " bytes" << endl;
    unsigned int random;

    CHRONOMETER file_writing;

    QueryPerformanceCounter(&(file_writing.start));

    for (int i = 1; i <= data_size; i++)
    {
        random = rand();
        file << random << " ";
    }

    QueryPerformanceCounter(&(file_writing.end));

    auto duration_file_writing = (file_writing.end.QuadPart - file_writing.start.QuadPart) * 1000000 / frequency.QuadPart;

    results << "Writing to file duration: " << duration_file_writing << " microseconds" << endl;

    double fw_freq = (double)data_size * sizeof(unsigned int) / duration_file_writing;

    results << "File writing frequency: " << fw_freq << " bytes/microsecond"<<endl<<endl<<endl;

    file.seekg(SEEK_SET);

    if (bubble_sort)
    {
        CHRONOMETER bubblesort;

        QueryPerformanceCounter(&(bubblesort.start));

        output << "*************BUBBLE SORT RESULT*************" << endl;

        int** matrix = new int* [nr_of_threads];
        for (int i = 0; i < nr_of_threads; i++)
        {
            matrix[i] = new int[data_size/nr_of_threads];
        }

        int values = 0;

        CHRONOMETER bs_file_reading;

        QueryPerformanceCounter(&(bs_file_reading.start));

        for (int i = 0; i < nr_of_threads; i++) 
        {
            for (int j = 0; j <= data_size / nr_of_threads && values < data_size; j++) 
            {
                file >> matrix[i][j];
                values++;
            }
        }

        QueryPerformanceCounter(&(bs_file_reading.end));

        auto duration_bs_file_reading = (bs_file_reading.end.QuadPart - bs_file_reading.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "File reading of bubblesort duration: "   << duration_bs_file_reading << " microseconds" << endl;

        PMYDATA* vect = new PMYDATA[nr_of_threads];
        HANDLE* hVect = new HANDLE[nr_of_threads];

        long length = data_size / nr_of_threads + 1;

        CHRONOMETER bs_sorting;

        QueryPerformanceCounter(&(bs_sorting.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            vect[i] = (PMYDATA)malloc(sizeof(MYDATA));
            vect[i]->array = new unsigned int[length*sizeof(unsigned int)];
            vect[i]->len = length;
            memcpy(vect[i]->array, matrix[i], length * sizeof(unsigned int));
            if (i == nr_of_threads-1)
            {
                vect[i]->len = data_size - i * length;
            }
            hVect[i] = CreateThread(NULL, 0, bubbleThread, vect[i], 0, NULL);
        }

        WaitForMultipleObjects(nr_of_threads, hVect, TRUE, INFINITE);

        QueryPerformanceCounter(&(bs_sorting.end));

        auto duration_bs_sorting = (bs_sorting.end.QuadPart - bs_sorting.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Bubblesort data sorting duration: "   << duration_bs_sorting << " microseconds" << endl;
        results << "    Average per-thread duration: " << duration_bs_sorting / nr_of_threads << " microseconds/thread" << endl;

        unsigned int* merged_array = new unsigned int[data_size];
        int size = 0;

        CHRONOMETER bs_merging;

        QueryPerformanceCounter(&(bs_merging.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            unsigned int* aux = new unsigned int[data_size];
            mergeArrays(merged_array, vect[i]->array, size, vect[i]->len, aux);
            size += vect[i]->len;
            memcpy(merged_array, aux, size*sizeof(unsigned int));
        }

        QueryPerformanceCounter(&(bs_merging.end));

        auto duration_bs_merging = (bs_merging.end.QuadPart - bs_merging.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Bubblesort merging duration: "   << duration_bs_merging << " microseconds" << endl;
        
        for (int i = 0; i < data_size; i++)
            output << merged_array[i] << endl;
        output << endl;

        for (int i = 0; i < nr_of_threads; i++)
        {
            delete[] vect[i]->array;
            free(vect[i]);
        }

        delete[] vect;
        delete[] hVect;
        delete[] merged_array;

        QueryPerformanceCounter(&(bubblesort.end));

        auto duration_bubblesort = (bubblesort.end.QuadPart - bubblesort.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Bubblesort duration: "   << duration_bubblesort << " microseconds" << endl<<endl<<endl;
    }

    if (insertion_sort)
    {
        CHRONOMETER insertionsort;

        QueryPerformanceCounter(&(insertionsort.start));

        output << "*************INSERTION SORT RESULT*************" << endl;

        int** matrix = new int* [nr_of_threads];
        for (int i = 0; i < nr_of_threads; i++)
        {
            matrix[i] = new int[data_size / nr_of_threads];
        }

        int values = 0;
        file.seekg(SEEK_SET);

        CHRONOMETER is_file_reading;

        QueryPerformanceCounter(&(is_file_reading.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            for (int j = 0; j <= data_size / nr_of_threads && values < data_size; j++)
            {
                file >> matrix[i][j];
                values++;
            }
        }

        QueryPerformanceCounter(&(is_file_reading.end));

        auto duration_is_file_reading = (is_file_reading.end.QuadPart - is_file_reading.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "File reading of insertion sort duration: " << duration_is_file_reading << " microseconds" << endl;

        PMYDATA* vect = new PMYDATA[nr_of_threads];
        HANDLE* hVect = new HANDLE[nr_of_threads];

        long length = data_size / nr_of_threads + 1;

        CHRONOMETER is_sorting;

        QueryPerformanceCounter(&(is_sorting.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            vect[i] = (PMYDATA)malloc(sizeof(MYDATA));
            vect[i]->array = new unsigned int[length * sizeof(unsigned int)];
            vect[i]->len = length;
            memcpy(vect[i]->array, matrix[i], length * sizeof(unsigned int));
            if (i == nr_of_threads - 1)
            {
                vect[i]->len = data_size - i * length;
            }
            hVect[i] = CreateThread(NULL, 0, insertionThread, vect[i], 0, NULL);
        }

        WaitForMultipleObjects(nr_of_threads, hVect, TRUE, INFINITE);

        QueryPerformanceCounter(&(is_sorting.end));

        auto duration_is_sorting = (is_sorting.end.QuadPart - is_sorting.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Insertion sort data sorting duration: "   << duration_is_sorting << " microseconds "<<endl;
        results << "    Average per-thread duration: " << duration_is_sorting / nr_of_threads << " microseconds/thread" << endl;

        unsigned int* merged_array = new unsigned int[data_size];
        int size = 0;

        CHRONOMETER is_merging;

        QueryPerformanceCounter(&(is_merging.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            unsigned int* aux = new unsigned int[data_size];
            mergeArrays(merged_array, vect[i]->array, size, vect[i]->len, aux);
            size += vect[i]->len;
            memcpy(merged_array, aux, size * sizeof(unsigned int));
        }

        QueryPerformanceCounter(&(is_merging.end));

        auto duration_is_merging = (is_merging.end.QuadPart - is_merging.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Insertion sort merging duration: "   << duration_is_merging << " microseconds" << endl;

        for (int i = 0; i < data_size; i++)
            output << merged_array[i] << endl;
        output << endl;

        for (int i = 0; i < nr_of_threads; i++)
        {
            delete[] vect[i]->array;
            free(vect[i]);
        }

        delete[] vect;
        delete[] hVect;
        delete[] merged_array;

        QueryPerformanceCounter(&(insertionsort.end));

        auto duration_insertionsort = (insertionsort.end.QuadPart - insertionsort.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Insertion sort duration: "   << duration_insertionsort << " microseconds" << endl<<endl<<endl;
    }

    if (selection_sort)
    {
        CHRONOMETER selectionsort;

        QueryPerformanceCounter(&(selectionsort.start));

        output << "*************SELECTION SORT RESULT*************" << endl;

        int** matrix = new int* [nr_of_threads];
        for (int i = 0; i < nr_of_threads; i++)
        {
            matrix[i] = new int[data_size / nr_of_threads];
        }

        int values = 0;
        file.seekg(SEEK_SET);

        CHRONOMETER ss_file_reading;

        QueryPerformanceCounter(&(ss_file_reading.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            for (int j = 0; j <= data_size / nr_of_threads && values < data_size; j++)
            {
                file >> matrix[i][j];
                values++;
            }
        }

        QueryPerformanceCounter(&(ss_file_reading.end));
        auto duration_ss_file_reading = (ss_file_reading.end.QuadPart - ss_file_reading.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "File reading of selection sort duration: "   << duration_ss_file_reading << " microseconds" << endl;

        PMYDATA* vect = new PMYDATA[nr_of_threads];
        HANDLE* hVect = new HANDLE[nr_of_threads];

        long length = data_size / nr_of_threads + 1;

        CHRONOMETER ss_sorting;

        QueryPerformanceCounter(&(ss_sorting.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            vect[i] = (PMYDATA)malloc(sizeof(MYDATA));
            vect[i]->array = new unsigned int[length * sizeof(unsigned int)];
            vect[i]->len = length;
            memcpy(vect[i]->array, matrix[i], length * sizeof(unsigned int));
            if (i == nr_of_threads - 1)
            {
                vect[i]->len = data_size - i * length;
            }
            hVect[i] = CreateThread(NULL, 0, selectionThread, vect[i], 0, NULL);
        }

        WaitForMultipleObjects(nr_of_threads, hVect, TRUE, INFINITE);

        QueryPerformanceCounter(&(ss_sorting.end));
        auto duration_ss_sorting = (ss_sorting.end.QuadPart - ss_sorting.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Selection sort data sorting duration: "   << duration_ss_sorting << " microseconds" << endl;
        results << "    Average per-thread duration: " << duration_ss_sorting / nr_of_threads << " microseconds/thread" << endl;

        unsigned int* merged_array = new unsigned int[data_size];
        int size = 0;

        CHRONOMETER ss_merging;

        QueryPerformanceCounter(&(ss_merging.start));

        for (int i = 0; i < nr_of_threads; i++)
        {
            unsigned int* aux = new unsigned int[data_size];
            mergeArrays(merged_array, vect[i]->array, size, vect[i]->len, aux);
            size += vect[i]->len;
            memcpy(merged_array, aux, size * sizeof(unsigned int));
        }

        QueryPerformanceCounter(&(ss_merging.end));

        auto duration_ss_merging = (ss_merging.end.QuadPart - ss_merging.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Selection sort merging duration : "   << duration_ss_merging << " microseconds" << endl;

        for (int i = 0; i < data_size; i++)
            output << merged_array[i] << endl;
        output << endl;

        for (int i = 0; i < nr_of_threads; i++)
        {
            delete[] vect[i]->array;
            free(vect[i]);
        }

        delete[] vect;
        delete[] hVect;
        delete[] merged_array;
       
        QueryPerformanceCounter(&(selectionsort.end));
        auto duration_selectionsort = (selectionsort.end.QuadPart - selectionsort.start.QuadPart) * 1000000 / frequency.QuadPart;
        results << "Selection sort duration: "   << duration_selectionsort << " microseconds" << endl << endl<<endl;
    }
    output.close();
    file.close();

    QueryPerformanceCounter(&(full_program.end));

    auto duration_full_program = (full_program.end.QuadPart - full_program.start.QuadPart) * 1000000 / frequency.QuadPart;
    results << "Full program duration: "   <<duration_full_program<<" microseconds" << endl;

    return 0;
}
