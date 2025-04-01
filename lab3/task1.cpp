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


void writeMatrix(const string& filename) {
    const size_t size = 3;
    double matrix[size][size] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    ofstream out(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(matrix), sizeof(matrix));
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


int main(){

    writeMatrix("files/m1.bin");
    writeMatrix("files/m2.bin");

    size_t size1, size2;
    vector<double> matrix1 = readMatrix("files/m1.bin", size1);
    vector<double> matrix2 = readMatrix("files/m2.bin", size2);

    vector<double> res;
    
    auto start = std::chrono::high_resolution_clock::now();

    res = multiplyMatrices(matrix1, matrix2, size1);

    auto end = std::chrono::high_resolution_clock::now();

    cout << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    printMatrix(res, size1);

    return 0;
}