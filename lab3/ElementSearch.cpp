#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>
#include <chrono>

using namespace std;


void printVector(const vector<int>& arr){
    for(const int element: arr)
        cout << element << " ";
    cout << endl;
}


void writeVector(const string& filename, vector<int>& arr) {

    ofstream out(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(arr.data()), arr.size() * sizeof(int));
    out.close();
}


vector<int> readVector(const string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        cout << ("Failed to open file " + filename);
        exit(1);
    }

    struct stat fileInfo;
    if (fstat(fd, &fileInfo) == -1) {
        cout << "Failed to get file size of " + filename;
        close(fd);
        exit(1);
    }

    off_t fileSize = fileInfo.st_size;

    size_t size = fileSize / sizeof(int);

    vector<int> arr(size);
    ssize_t bytesRead = read(fd, arr.data(), fileSize);
    if (bytesRead != fileSize) {
        cout << "Failed to read vector data" + filename;
        close(fd);
        exit(1);
    }

    close(fd);
    return arr;
}


int findElement(const vector<int>& arr, int target){
    size_t size = arr.size();
    for(size_t i = size - 1; i != 0; --i){
        if(arr.at(i) == target)
            return i;
    }
    return -1;
}


int main(){
    vector<int> input_arr = {1, 2, 3, 4, 5, 6, 7, 4, 8, -11};

    writeVector("files/arr.bin", input_arr);

    vector<int> arr = readVector("files/arr.bin");

    int res;
    
    auto start = chrono::high_resolution_clock::now();

    res = findElement(arr, 4);

    auto end = chrono::high_resolution_clock::now();

    cout << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    printVector(input_arr);
    cout << res << endl;
}