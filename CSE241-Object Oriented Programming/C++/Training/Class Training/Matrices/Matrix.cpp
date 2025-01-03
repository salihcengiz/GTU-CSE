#include "Matrix.h"

int Matrix2D::getNumOfRows() const {
    return Matrix.size(); //Vektörün eleman sayısı row sayısına eşit
}

int Matrix2D::getNumOfColumns() const {
    return Matrix[0].size(); //Vektörün alt vektörünün eleman sayısı column sayısına eşit
}

Matrix2D::Matrix2D(int row, int column) {
    if(row < 1 || column < 1)
        exit(-1);
    else{
        //Matrix = vector<vector<int> > (row, vector<int>(column, 0)); satırı da aynı işlevi görör
        vector<int> rowV(column, 0);
        for(int i = 0; i < row; ++i)
            Matrix.push_back(rowV);
    }
}

bool Matrix2D::isValid(int row, int column) const{
    if(row < 0 || column < 0 || row >= getNumOfRows() || column >= getNumOfColumns())
        return false;
    else
        return true;
}

bool Matrix2D::areSameSize(const Matrix2D& other) const {
    if(getNumOfRows() == other.getNumOfRows() && getNumOfColumns() == other.getNumOfColumns())
        return true;
    else
        return false;
}

void Matrix2D::setElement(int row, int column, int value) {
    if(isValid(row, column))
        Matrix[row][column] = value;
    else
        exit(-1);
}

int Matrix2D::getElement(int row, int column) const {
    if(isValid(row, column))
        return Matrix[row][column];
    else
        exit(-1);
}

Matrix2D Matrix2D::operator+(const Matrix2D& other) const {
    if(areSameSize(other)) {
        Matrix2D result = other;
        for(int r = 0; r < getNumOfRows(); ++r) {
            for(int c = 0; c < getNumOfColumns(); ++c)
                result.Matrix[r][c] += Matrix[r][c];
        }
        return result;
    }
    else
        return false;
}

bool Matrix2D::operator==(const Matrix2D& other) const {

    //Sadece tek satırla da fonksiyonu yazmak mümkün:
    //return data == other.data; => vector kütüphanesinde bu işlem tanımlı

    if(areSameSize(other)) {
        for(int r = 0; r < getNumOfRows(); ++r) {
            for(int c = 0; c < getNumOfColumns(); ++c) {
                if(Matrix[r][c] != other.Matrix[r][c])
                    return false;
            }
        }
        return true;
    }
    return false;
}

Matrix2D operator-(const Matrix2D& matrix){
    Matrix2D result = matrix;
    for(int r = 0; r < result.getNumOfRows(); ++r) {
        for(int c = 0; c < result.getNumOfColumns(); ++c)
            result.setElement(r, c, -result.getElement(r, c));
    }
    return result;
}

ostream& operator<<(ostream& out, const Matrix2D matrix) {
	for (int r = 0; r < matrix.getNumOfRows(); ++r){
		for (int c = 0; c < matrix.getNumOfColumns(); ++c)
			out << matrix.getElement(r, c) << " ";
		out << endl;
	}
	return out;
}