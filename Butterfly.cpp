#include <iostream>
using namespace std;

int main() {
    int n;
    cout << "Enter the size: ";
    cin >> n;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= i; j++)
            cout << (j == 1 || j == i ? '*' : ' ');

        cout << string(2 * (n - i), ' ');

        for (int j = 1; j <= i; j++)
            cout << (j == 1 || j == i ? '*' : ' ');

        cout << endl;
    }

    for (int i = n - 1; i >= 1; i--) {
        for (int j = 1; j <= i; j++)
            cout << (j == 1 || j == i ? '*' : ' ');

        cout << string(2 * (n - i), ' ');

        for (int j = 1; j <= i; j++)
            cout << (j == 1 || j == i ? '*' : ' ');

        cout << endl;
    }

    return 0;
}