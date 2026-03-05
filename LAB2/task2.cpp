#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cmath>

using namespace std;

const unsigned int WITHDRAW_PERMISSION = 1;
const unsigned int DEPOSIT_PERMISSION = 2;
const unsigned int TRANSFER_PERMISSION = 4;
const unsigned int VIP_FLAG = 8;

const unsigned int XOR_KEY = 0xAB;

unsigned int encodeTransaction(unsigned int type, unsigned int amount) {
    return (type << 28) | (amount & 0x0FFFFFFF);
}

unsigned int decodeType(unsigned int encoded) {
    return (encoded >> 28) & 0xF;
}

unsigned int decodeAmount(unsigned int encoded) {
    return encoded & 0x0FFFFFFF;
}

class Account {
protected:
    int accountId;
    string name;
    double balance;
    unsigned int permissions;
    vector<double> transactions;

public:
    Account() : accountId(0), name(""), balance(0), permissions(0) {}
    Account(int id, string holderName, double startingBalance, unsigned int perms)
        : accountId(id), name(holderName), balance(startingBalance), permissions(perms) {}

    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual void saveToFile(ofstream& out) = 0;
    virtual void loadFromFile(ifstream& in) = 0;
    virtual string accountType() = 0;
    virtual ~Account() {}

    int getId() { return accountId; }
    string getName() { return name; }
    double getBalance() { return balance; }
    unsigned int getPermissions() { return permissions; }
    vector<double>& getTransactions() { return transactions; }

    void setPermissions(unsigned int perms) { permissions = perms; }

    bool hasPermission(unsigned int flag) {
        return (permissions & flag) != 0;
    }

    void showPermissions() {
        cout << "  Withdraw : " << (hasPermission(WITHDRAW_PERMISSION) ? "Yes" : "No") << endl;
        cout << "  Deposit  : " << (hasPermission(DEPOSIT_PERMISSION) ? "Yes" : "No") << endl;
        cout << "  Transfer : " << (hasPermission(TRANSFER_PERMISSION) ? "Yes" : "No") << endl;
        cout << "  VIP      : " << (hasPermission(VIP_FLAG) ? "Yes" : "No") << endl;
    }

    void showTransactionHistory() {
        if (transactions.empty()) {
            cout << "  No transactions yet." << endl;
            return;
        }
        for (int i = 0; i < (int)transactions.size(); i++) {
            if (transactions[i] >= 0)
                cout << "  + " << transactions[i] << endl;
            else
                cout << "  - " << -transactions[i] << endl;
        }
    }

    void showCompressedTransactions() {
        if (transactions.empty()) {
            cout << "  No transactions to compress." << endl;
            return;
        }
        for (int i = 0; i < (int)transactions.size(); i++) {
            unsigned int type;
            unsigned int amount;
            if (transactions[i] >= 0) {
                type = 1;
                amount = (unsigned int)transactions[i];
            } else {
                type = 2;
                amount = (unsigned int)(-transactions[i]);
            }
            unsigned int encoded = encodeTransaction(type, amount);
            unsigned int decodedType = decodeType(encoded);
            unsigned int decodedAmount = decodeAmount(encoded);
            string label = (decodedType == 1) ? "Deposit" : (decodedType == 2) ? "Withdrawal" : "Transfer";
            cout << "  Encoded: " << encoded
                 << " -> Type: " << label
                 << ", Amount: " << decodedAmount << endl;
        }
    }

    void display() {
        cout << "========================================" << endl;
        cout << "  Account Type : " << accountType() << endl;
        cout << "  Account ID   : " << accountId << endl;
        cout << "  Holder Name  : " << name << endl;
        cout << "  Balance      : " << balance << endl;
        cout << "  Permissions  : " << permissions << endl;
        showPermissions();
        cout << "  Transactions :" << endl;
        showTransactionHistory();
        cout << "  Compressed   :" << endl;
        showCompressedTransactions();
        cout << "========================================" << endl;
    }
};

class SavingsAccount : public Account {
    double interestRate;
public:
    SavingsAccount() : Account(), interestRate(0.05) {}
    SavingsAccount(int id, string holderName, double startingBalance, unsigned int perms)
        : Account(id, holderName, startingBalance, perms), interestRate(0.05) {}

    string accountType() { return "Savings"; }

    void deposit(double amount) {
        if (!hasPermission(DEPOSIT_PERMISSION)) {
            cout << "  Permission denied: cannot deposit." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "  Invalid deposit amount." << endl;
            return;
        }
        balance += amount;
        transactions.push_back(amount);
        cout << "  Deposited " << amount << ". New balance: " << balance << endl;
    }

    void withdraw(double amount) {
        if (!hasPermission(WITHDRAW_PERMISSION)) {
            cout << "  Permission denied: cannot withdraw." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "  Invalid withdrawal amount." << endl;
            return;
        }
        if (amount > balance) {
            cout << "  Insufficient funds." << endl;
            return;
        }
        balance -= amount;
        transactions.push_back(-amount);
        cout << "  Withdrew " << amount << ". New balance: " << balance << endl;
    }

    void applyInterest() {
        double interest = balance * interestRate;
        balance += interest;
        transactions.push_back(interest);
        cout << "  Interest of " << interest << " applied. New balance: " << balance << endl;
    }

    void saveToFile(ofstream& out) {
        out << "ACCOUNT Savings" << endl;
        out << accountId << " " << name << " " << balance << " " << permissions << endl;
        out << "TRANSACTIONS" << endl;
        for (int i = 0; i < (int)transactions.size(); i++) {
            out << transactions[i] << endl;
        }
        out << endl;
    }

    void loadFromFile(ifstream& in) {
        in >> accountId >> name >> balance >> permissions;
        string line;
        getline(in, line);
        getline(in, line);
        if (line.find("TRANSACTIONS") == string::npos) {
            return;
        }
        transactions.clear();
        while (getline(in, line)) {
            if (line.empty() || line.find("ACCOUNT") != string::npos) {
                break;
            }
            double val;
            stringstream ss(line);
            if (ss >> val) {
                transactions.push_back(val);
            }
        }
    }
};

class CurrentAccount : public Account {
    double overdraftLimit;
public:
    CurrentAccount() : Account(), overdraftLimit(1000) {}
    CurrentAccount(int id, string holderName, double startingBalance, unsigned int perms)
        : Account(id, holderName, startingBalance, perms), overdraftLimit(1000) {}

    string accountType() { return "Current"; }

    void deposit(double amount) {
        if (!hasPermission(DEPOSIT_PERMISSION)) {
            cout << "  Permission denied: cannot deposit." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "  Invalid deposit amount." << endl;
            return;
        }
        balance += amount;
        transactions.push_back(amount);
        cout << "  Deposited " << amount << ". New balance: " << balance << endl;
    }

    void withdraw(double amount) {
        if (!hasPermission(WITHDRAW_PERMISSION)) {
            cout << "  Permission denied: cannot withdraw." << endl;
            return;
        }
        if (amount <= 0) {
            cout << "  Invalid withdrawal amount." << endl;
            return;
        }
        if (amount > balance + overdraftLimit) {
            cout << "  Exceeds overdraft limit." << endl;
            return;
        }
        balance -= amount;
        transactions.push_back(-amount);
        cout << "  Withdrew " << amount << ". New balance: " << balance << endl;
    }

    void saveToFile(ofstream& out) {
        out << "ACCOUNT Current" << endl;
        out << accountId << " " << name << " " << balance << " " << permissions << endl;
        out << "TRANSACTIONS" << endl;
        for (int i = 0; i < (int)transactions.size(); i++) {
            out << transactions[i] << endl;
        }
        out << endl;
    }

    void loadFromFile(ifstream& in) {
        in >> accountId >> name >> balance >> permissions;
        string line;
        getline(in, line);
        getline(in, line);
        if (line.find("TRANSACTIONS") == string::npos) {
            return;
        }
        transactions.clear();
        while (getline(in, line)) {
            if (line.empty() || line.find("ACCOUNT") != string::npos) {
                break;
            }
            double val;
            stringstream ss(line);
            if (ss >> val) {
                transactions.push_back(val);
            }
        }
    }
};

void saveAllAccounts(vector<Account*>& accounts, const string& filename) {
    ofstream out(filename.c_str());
    if (!out.is_open()) {
        cout << "  Could not open file for writing." << endl;
        return;
    }
    for (int i = 0; i < (int)accounts.size(); i++) {
        accounts[i]->saveToFile(out);
    }
    out.close();
    cout << "  All accounts saved to " << filename << endl;
}

void saveAllAccountsEncrypted(vector<Account*>& accounts, const string& filename) {
    stringstream buffer;
    for (int i = 0; i < (int)accounts.size(); i++) {
        ofstream tempStream;
        string tempFile = "temp_save.txt";
        tempStream.open(tempFile.c_str());
        accounts[i]->saveToFile(tempStream);
        tempStream.close();

        ifstream tempIn(tempFile.c_str());
        string line;
        while (getline(tempIn, line)) {
            buffer << line << "\n";
        }
        tempIn.close();
        remove(tempFile.c_str());
    }

    string plainText = buffer.str();
    ofstream out(filename.c_str(), ios::binary);
    for (int i = 0; i < (int)plainText.size(); i++) {
        char encrypted = plainText[i] ^ XOR_KEY;
        out.put(encrypted);
    }
    out.close();
    cout << "  All accounts saved (encrypted) to " << filename << endl;
}

void loadAllAccounts(vector<Account*>& accounts, const string& filename) {
    ifstream in(filename.c_str());
    if (!in.is_open()) {
        cout << "  Could not open file for reading." << endl;
        return;
    }

    for (int i = 0; i < (int)accounts.size(); i++) {
        delete accounts[i];
    }
    accounts.clear();

    string line;
    while (getline(in, line)) {
        if (line.find("ACCOUNT") != string::npos) {
            string type = line.substr(8);
            Account* fresh = NULL;
            if (type == "Savings") {
                fresh = new SavingsAccount();
            } else if (type == "Current") {
                fresh = new CurrentAccount();
            }
            if (fresh) {
                fresh->loadFromFile(in);
                accounts.push_back(fresh);
            }
        }
    }
    in.close();
    cout << "  Loaded " << accounts.size() << " accounts from " << filename << endl;
}

void loadAllAccountsEncrypted(vector<Account*>& accounts, const string& filename) {
    ifstream in(filename.c_str(), ios::binary);
    if (!in.is_open()) {
        cout << "  Could not open encrypted file for reading." << endl;
        return;
    }

    string decrypted = "";
    char ch;
    while (in.get(ch)) {
        decrypted += (char)(ch ^ XOR_KEY);
    }
    in.close();

    string tempFile = "temp_load.txt";
    ofstream tempOut(tempFile.c_str());
    tempOut << decrypted;
    tempOut.close();

    loadAllAccounts(accounts, tempFile);
    remove(tempFile.c_str());
}

Account* findAccount(vector<Account*>& accounts, int id) {
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (accounts[i]->getId() == id) {
            return accounts[i];
        }
    }
    return NULL;
}

void transferFunds(vector<Account*>& accounts, int fromId, int toId, double amount) {
    Account* sender = findAccount(accounts, fromId);
    Account* receiver = findAccount(accounts, toId);

    if (!sender || !receiver) {
        cout << "  One or both accounts not found." << endl;
        return;
    }
    if (!sender->hasPermission(TRANSFER_PERMISSION)) {
        cout << "  Sender does not have transfer permission." << endl;
        return;
    }
    if (amount <= 0) {
        cout << "  Invalid transfer amount." << endl;
        return;
    }
    if (sender->getBalance() < amount) {
        cout << "  Sender has insufficient funds." << endl;
        return;
    }

    sender->withdraw(amount);
    receiver->deposit(amount);
    cout << "  Transferred " << amount << " from " << fromId << " to " << toId << endl;
}

void showMonthlySummary(vector<Account*>& accounts) {
    double monthlyDeposits[12];
    double monthlyWithdrawals[12];
    double monthlyNet[12];

    for (int i = 0; i < 12; i++) {
        monthlyDeposits[i] = 0;
        monthlyWithdrawals[i] = 0;
        monthlyNet[i] = 0;
    }

    for (int a = 0; a < (int)accounts.size(); a++) {
        vector<double>& txns = accounts[a]->getTransactions();
        for (int t = 0; t < (int)txns.size(); t++) {
            int monthIndex = t % 12;
            if (txns[t] >= 0) {
                monthlyDeposits[monthIndex] += txns[t];
            } else {
                monthlyWithdrawals[monthIndex] += (-txns[t]);
            }
        }
    }

    string monthNames[12] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    };

    cout << "========================================" << endl;
    cout << "       Monthly Summary Report" << endl;
    cout << "========================================" << endl;
    for (int i = 0; i < 12; i++) {
        monthlyNet[i] = monthlyDeposits[i] - monthlyWithdrawals[i];
        if (monthlyDeposits[i] > 0 || monthlyWithdrawals[i] > 0) {
            cout << "  " << monthNames[i] << ":" << endl;
            cout << "    Deposits    : " << monthlyDeposits[i] << endl;
            cout << "    Withdrawals : " << monthlyWithdrawals[i] << endl;
            cout << "    Net Change  : " << monthlyNet[i] << endl;
        }
    }
    cout << "========================================" << endl;
}

void cleanUp(vector<Account*>& accounts) {
    for (int i = 0; i < (int)accounts.size(); i++) {
        delete accounts[i];
    }
    accounts.clear();
}

int main() {
    vector<Account*> accounts;
    string dataFile = "accounts.txt";
    string encryptedFile = "accounts_encrypted.dat";
    int choice;
    bool running = true;

    while (running) {
        cout << endl;
        cout << "========================================" << endl;
        cout << "    Secure File-Based Banking System" << endl;
        cout << "========================================" << endl;
        cout << "  1. Create Account" << endl;
        cout << "  2. Deposit" << endl;
        cout << "  3. Withdraw" << endl;
        cout << "  4. Show Account" << endl;
        cout << "  5. Save to File" << endl;
        cout << "  6. Load from File" << endl;
        cout << "  7. Transfer Funds" << endl;
        cout << "  8. Monthly Summary" << endl;
        cout << "  9. Apply Interest (Savings)" << endl;
        cout << " 10. Save Encrypted" << endl;
        cout << " 11. Load Encrypted" << endl;
        cout << " 12. Show All Accounts" << endl;
        cout << "  0. Exit" << endl;
        cout << "========================================" << endl;
        cout << "  Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            int id;
            string holderName;
            double startingBalance;
            int typeChoice;
            unsigned int perms = 0;

            cout << "  Account type (1=Savings, 2=Current): ";
            cin >> typeChoice;
            cout << "  Account ID: ";
            cin >> id;
            cout << "  Holder name: ";
            cin >> holderName;
            cout << "  Starting balance: ";
            cin >> startingBalance;

            int canWithdraw, canDeposit, canTransfer, isVip;
            cout << "  Allow withdraw? (1=Yes 0=No): ";
            cin >> canWithdraw;
            cout << "  Allow deposit? (1=Yes 0=No): ";
            cin >> canDeposit;
            cout << "  Allow transfer? (1=Yes 0=No): ";
            cin >> canTransfer;
            cout << "  VIP account? (1=Yes 0=No): ";
            cin >> isVip;

            if (canWithdraw) perms = perms | WITHDRAW_PERMISSION;
            if (canDeposit) perms = perms | DEPOSIT_PERMISSION;
            if (canTransfer) perms = perms | TRANSFER_PERMISSION;
            if (isVip) perms = perms | VIP_FLAG;

            Account* fresh = NULL;
            if (typeChoice == 1) {
                fresh = new SavingsAccount(id, holderName, startingBalance, perms);
            } else {
                fresh = new CurrentAccount(id, holderName, startingBalance, perms);
            }
            accounts.push_back(fresh);
            cout << "  Account created successfully." << endl;

        } else if (choice == 2) {
            int id;
            double amount;
            cout << "  Account ID: ";
            cin >> id;
            cout << "  Amount to deposit: ";
            cin >> amount;
            Account* target = findAccount(accounts, id);
            if (target) {
                target->deposit(amount);
            } else {
                cout << "  Account not found." << endl;
            }

        } else if (choice == 3) {
            int id;
            double amount;
            cout << "  Account ID: ";
            cin >> id;
            cout << "  Amount to withdraw: ";
            cin >> amount;
            Account* target = findAccount(accounts, id);
            if (target) {
                target->withdraw(amount);
            } else {
                cout << "  Account not found." << endl;
            }

        } else if (choice == 4) {
            int id;
            cout << "  Account ID: ";
            cin >> id;
            Account* target = findAccount(accounts, id);
            if (target) {
                target->display();
            } else {
                cout << "  Account not found." << endl;
            }

        } else if (choice == 5) {
            saveAllAccounts(accounts, dataFile);

        } else if (choice == 6) {
            loadAllAccounts(accounts, dataFile);

        } else if (choice == 7) {
            int fromId, toId;
            double amount;
            cout << "  From Account ID: ";
            cin >> fromId;
            cout << "  To Account ID: ";
            cin >> toId;
            cout << "  Amount: ";
            cin >> amount;
            transferFunds(accounts, fromId, toId, amount);

        } else if (choice == 8) {
            showMonthlySummary(accounts);

        } else if (choice == 9) {
            int id;
            cout << "  Account ID: ";
            cin >> id;
            Account* target = findAccount(accounts, id);
            if (target) {
                if (target->accountType() == "Savings") {
                    SavingsAccount* savingsPtr = dynamic_cast<SavingsAccount*>(target);
                    if (savingsPtr) {
                        savingsPtr->applyInterest();
                    }
                } else {
                    cout << "  Interest only applies to Savings accounts." << endl;
                }
            } else {
                cout << "  Account not found." << endl;
            }

        } else if (choice == 10) {
            saveAllAccountsEncrypted(accounts, encryptedFile);

        } else if (choice == 11) {
            loadAllAccountsEncrypted(accounts, encryptedFile);

        } else if (choice == 12) {
            if (accounts.empty()) {
                cout << "  No accounts in system." << endl;
            } else {
                for (int i = 0; i < (int)accounts.size(); i++) {
                    accounts[i]->display();
                }
            }

        } else if (choice == 0) {
            running = false;

        } else {
            cout << "  Invalid choice." << endl;
        }
    }

    cleanUp(accounts);
    cout << "  All memory freed. Goodbye." << endl;
    return 0;
}
