#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cmath>
#include <fstream>
#include <chrono>

using namespace std;

void printMatrix(const vector<double>& m, size_t size){
    for(size_t i = 0; i < size; ++i){
        for(size_t j = 0; j < size; ++j){
            cout << m[i*size + j] << " ";
        }
        cout << endl;
    }
}


void writeMatrix(const string& filename, vector<double>& m) {

    ofstream out(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(m.data()), m.size() * sizeof(double));
    out.close();
}


vector<double> readMatrix(const string& filename, size_t& size) {
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

    size = (size_t)sqrt(fileSize / sizeof(double));
    if (fileSize != size * size * sizeof(double)) {
        cout << "Error: File" + filename + "size doesn't correspond to square matrix";
        close(fd);
        exit(1);
    }

    vector<double> matrix(size * size);
    ssize_t bytesRead = read(fd, matrix.data(), fileSize);
    if (bytesRead != fileSize) {
        cout << "Failed to read matrix data" + filename;
        close(fd);
        exit(1);
    }

    close(fd);
    return matrix;
}


vector<double> multiplyMatrices(const vector<double>& m1, 
    const vector<double>& m2, size_t size) {
    vector<double> res(size * size, 0.0);

    for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < size; ++k) {
            double temp = m1[i * size + k];
            for (size_t j = 0; j < size; ++j) {
                res[i * size + j] += temp * m2[k * size + j];
            }
        }
    }

    return res;
}


struct ThreadData {
    const vector<double>* m1;
    const vector<double>* m2;
    vector<double>* res;

    size_t size;
    size_t start_row;
    size_t end_row;

    ThreadData(){
        this->m1 = nullptr;
        this->m2 = nullptr;
        this->res = nullptr;

        this->size = -1;
        this->start_row = -1;
        this->end_row = -1;
    };

    void setValues(const vector<double>* mat1, const vector<double>* mat2,
        vector<double>* result, size_t matrix_size,
        size_t start, size_t end)
        {
            this->m1 = mat1;
            this->m2 = mat2;
            this->res = result;

            this->size = matrix_size;
            this->start_row = start;
            this->end_row = end;
    }
};


void* multiplyPart(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    const vector<double>* m1 = data->m1;
    const vector<double>* m2 = data->m2;
    vector<double>* res = data->res;

    size_t size = data->size;

    for (size_t i = data->start_row; i < data->end_row; ++i) {
        for (size_t k = 0; k < size; ++k) {
            double temp = m1->at(i * size + k);
            for (size_t j = 0; j < size; ++j) {
                res->at(i * size + j) += temp * m2->at(k * size + j);
            }
        }
    }

    pthread_exit(NULL);
}


vector<double> multiplyMatricesParallel(const vector<double>& m1, 
    const vector<double>& m2, size_t size, int num_threads) 
    {
    vector<double> res(size * size, 0.0);
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    size_t rows_per_thread = size / num_threads;
    size_t remaining_rows = size % num_threads;
    size_t current_row = 0;

    for (int i = 0; i < num_threads; ++i) {
        size_t rows_for_this_thread = rows_per_thread;
        if(remaining_rows != 0){
            ++rows_for_this_thread;
            --remaining_rows;
        }

        thread_data[i].setValues(&m1, &m2, &res, size, current_row, current_row + rows_for_this_thread);

        pthread_create(&threads[i], NULL, multiplyPart, (void*)&thread_data[i]);

        current_row = thread_data[i].end_row;
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    return res;
}


int main(){

    vector<double> input1 = {
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1
    };

    vector<double> input2 = {
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1,
        1, 2, 3, 4, 5, 5, 4, 3, 2, 1
    };

    writeMatrix("files/m1.bin", input1);
    writeMatrix("files/m2.bin", input2);

    size_t size1, size2;
    vector<double> matrix1 = readMatrix("files/m1.bin", size1);
    vector<double> matrix2 = readMatrix("files/m2.bin", size2);

    vector<double> res;
    
    auto start = chrono::high_resolution_clock::now();

    res = multiplyMatrices(matrix1, matrix2, size1);

    auto end = chrono::high_resolution_clock::now();

    cout << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    printMatrix(res, size1);


    res.clear();
    
    start = chrono::high_resolution_clock::now();

    res = multiplyMatricesParallel(matrix1, matrix2, size1, 10);

    end = chrono::high_resolution_clock::now();

    cout << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    printMatrix(res, size1);

    return 0;
}