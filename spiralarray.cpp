#include <iostream>
using namespace std;

void spiralMatrix(int rows, int cols) {
    int matrix[100][100] = {};
    int top = 0, bottom = rows - 1;
    int left = 0, right = cols - 1;
    int num = 1;

    while (top <= bottom && left <= right) {
        for (int i = left; i <= right; i++)
            matrix[top][i] = num++;
        top++;

        for (int i = top; i <= bottom; i++)
            matrix[i][right] = num++;
        right--;

        for (int i = right; i >= left; i--)
            matrix[bottom][i] = num++;
        bottom--;

        for (int i = bottom; i >= top; i--)
            matrix[i][left] = num++;
        left++;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            cout << matrix[i][j];
            if (j < cols - 1) cout << "\t";
        }
        cout << endl;
    }
}

int main() {
    int rows, cols;
    cout << "Enter rows and cols: ";
    cin >> rows >> cols;
    spiralMatrix(rows, cols);
    return 0;
}