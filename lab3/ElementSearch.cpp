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


struct ThreadData {
    const vector<int>* arr;
    int target;
    int* result;
    int start_idx;
    int end_idx;
    pthread_spinlock_t* lock;
};


void* searchPart(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_result = -1;
    
    for (int i = data->end_idx - 1; i >= data->start_idx; --i) {
        if (data->arr->at(i) == data->target) {
            local_result = i;
            break;
        }
    }
    
    if (local_result != -1) {
        pthread_spin_lock(data->lock);
        if (local_result > *(data->result)) {
            *(data->result) = local_result;
        }
        pthread_spin_unlock(data->lock);
    }

    pthread_exit(NULL);
}


int findElementParallel(const vector<int>& arr, int target, int num_threads) {
    vector<pthread_t> threads(num_threads);
    vector<ThreadData> thread_data(num_threads);
    pthread_spinlock_t lock;
    int result = -1;
    
    pthread_spin_init(&lock, 0);
    
    int chunk_size = arr.size() / num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].arr = &arr;
        thread_data[i].target = target;
        thread_data[i].result = &result;
        thread_data[i].start_idx = i * chunk_size;
        thread_data[i].lock = &lock;

        if(i == num_threads - 1){
            thread_data[i].end_idx = arr.size();
        }
        else{
            thread_data[i].end_idx = (i + 1) * chunk_size;
        }
        
        pthread_create(&threads[i], nullptr, searchPart, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    pthread_spin_destroy(&lock);
    
    return result;
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


    res = -2;
    
    start = chrono::high_resolution_clock::now();

    res = findElementParallel(arr, 4, 5);

    end = chrono::high_resolution_clock::now();

    cout << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    printVector(input_arr);
    cout << res << endl;
}