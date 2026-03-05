#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

const unsigned int PRIMARY_KEY = 1;
const unsigned int NOT_NULL = 2;
const unsigned int UNIQUE = 4;

class Column {

    string name;
    string type;
    unsigned int constraints;

    public:
        Column(string n, string t, unsigned int constr){
            name = n;
            type = t;
            constraints = constr;
        }

        string getName(){return name;}
        string getType(){return type;}
        unsigned int getConstraints(){return constraints;}

        bool isPrimaryKey() {return(constraints & PRIMARY_KEY)!= 0;}
        bool isNotNull(){return(constraints & NOT_NULL)!= 0;}
        bool isUnique(){return(constraints & UNIQUE)!= 0;}
};

class Row {
    vector<string> values;

    public:
        Row(vector<string> vec){
            values = vec;
        }

        vector<string> getValues() {return values;}

        string getValue(int index){
            if(index >= 0 && index < (int)values.size())
                return values[index];
            return "";
        }
        int getSize() {return values.size();}
};

class Table {
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

    public:
        Table(string name, vector<Column> col){
            tableName = name;
            columns = col;
        }

        // destructor frees all dynamically allocated rows
        ~Table(){
            for(int i =0;i< (int)rows.size();i++){
                delete rows[i];
            }
            rows.clear();
        }

        void addRow(Row* r){
            rows.push_back(r);
        }

        void deleteRow(int index){
            if(index >= 0 && index < (int)rows.size()){
                delete rows[index];
                rows.erase(rows.begin() + index);
            }
        }

        string getTableName(){return tableName;}
        vector<Column>& getColumns(){return columns;}
        vector<Row*>& getRows(){return rows;}
        int getColumnCount(){return columns.size();}
        int getRowCount(){return rows.size();}
};


Table* createTable(){
    char tableNameBuffer[256];
    int numCols;
    cout<<"Enter table name: ";
    cin>> tableNameBuffer;
    cout<<"Enter number of columns: ";
    cin>> numCols;

    vector<Column> tempColumns;

    for(int i=0; i< numCols; i++){
        char colName[256];
        char colType[256];
        unsigned int constraints = 0;

        cout<<"\nColumn "<<i+1<<":"<<endl;
        cout<<"  Enter name: ";
        cin>>colName;
        cout<<"  Enter type (int/string): ";
        cin>> colType;
        cout<<"  Enter constraints(1=PK, 2=NotNull, 4=Unique, combine e.g. 3=PK+NotNull): ";
        cin>> constraints;

        tempColumns.push_back(Column(colName,colType,constraints));
    }

    Table* newTable = new Table(tableNameBuffer,tempColumns);
    cout<<"\nTable '"<< tableNameBuffer <<"' created successfully!"<<endl;
    return newTable;
}


void insertInto(Table* table){
    int numCols = table->getColumnCount();
    vector<string> values;

    cout<<"Enter "<<numCols<<" values for table '"<<table->getTableName()<<"':"<<endl;

    for(int i=0; i< numCols; i++){
        char valBuffer[256];
        cout<<"  "<<table->getColumns()[i].getName()
            <<" ("<<table->getColumns()[i].getType()<<"): ";
        cin>> valBuffer;
        values.push_back(string(valBuffer));
    }

    // validate all constraints using bitwise AND checks
    for(int i=0; i< numCols; i++){
        Column& col = table->getColumns()[i];
        string val = values[i];

        if(col.isNotNull() && val.empty()){
            cout<<"Error: '"<<col.getName()<<"' cannot be null."<<endl;
            return;
        }

        // check primary key duplicates
        if(col.isPrimaryKey()){
            for(int j=0; j< table->getRowCount(); j++){
                if(table->getRows()[j]->getValue(i) == val){
                    cout<<"Error: Duplicate primary key '"<<val<<"' in '"<<col.getName()<<"'."<<endl;
                    return;
                }
            }
        }

        
        if(col.isUnique()){
            for(int j=0; j< table->getRowCount(); j++){
                if(table->getRows()[j]->getValue(i) == val){
                    cout<<"Error: Duplicate value '"<<val<<"' in unique column '"<<col.getName()<<"'."<<endl;
                    return;
                }
            }
        }

     
        if(col.getType() == "int"){
            bool valid = true;
            for(int k=0; k< (int)val.length(); k++){
                if(!isdigit(val[k]) && !(k==0 && val[k]=='-')){
                    valid = false;
                    break;
                }
            }
            if(!valid){
                cout<<"Error: '"<<col.getName()<<"' expects int, got '"<<val<<"'."<<endl;
                return;
            }
        }
    }

    //allocate rows
    Row* newRow = new Row(values);
    table->addRow(newRow);
    cout<<"Record inserted."<<endl;
}


void selectAll(Table* table){
    if(table->getRowCount() == 0){
        cout<<"Table '"<<table->getTableName()<<"' is empty."<<endl;
        return;
    }

    int numCols = table->getColumnCount();

//cleaner otput
    vector<int> widths;
    for(int i=0; i< numCols; i++){
        int w = table->getColumns()[i].getName().length();
        for(int j=0; j< table->getRowCount(); j++){
            int len = table->getRows()[j]->getValue(i).length();
            if(len > w) w = len;
        }
        widths.push_back(w + 2);
    }

    cout<<endl;
    for(int i=0; i< numCols; i++)
        cout<<left<<setw(widths[i])<<table->getColumns()[i].getName();
    cout<<endl;

    for(int i=0; i< numCols; i++)
        for(int j=0; j< widths[i]; j++) cout<<"-";
    cout<<endl;

    for(int i=0; i< table->getRowCount(); i++){
        Row* row = table->getRows()[i];
        for(int j=0; j< numCols; j++)
            cout<<left<<setw(widths[j])<<row->getValue(j);
        cout<<endl;
    }
    cout<<endl;
}


void selectWhere(Table* table){
    char colBuffer[256];
    char valBuffer[256];
    cout<<"  Column name: ";
    cin>>colBuffer;
    cout<<"  Value to match: ";
    cin>>valBuffer;

    string colName = colBuffer;
    string matchVal = valBuffer;

    int colIndex = -1;
    for(int i=0; i< table->getColumnCount(); i++){
        if(table->getColumns()[i].getName() == colName){
            colIndex = i;
            break;
        }
    }
    if(colIndex == -1){
        cout<<"Error: Column '"<<colName<<"' not found."<<endl;
        return;
    }

    int numCols = table->getColumnCount();
    vector<int> widths;
    for(int i=0; i< numCols; i++){
        int w = table->getColumns()[i].getName().length();
        for(int j=0; j< table->getRowCount(); j++){
            int len = table->getRows()[j]->getValue(i).length();
            if(len > w) w = len;
        }
        widths.push_back(w + 2);
    }

    cout<<endl;
    for(int i=0; i< numCols; i++)
        cout<<left<<setw(widths[i])<<table->getColumns()[i].getName();
    cout<<endl;
    for(int i=0; i< numCols; i++)
        for(int j=0; j< widths[i]; j++) cout<<"-";
    cout<<endl;

    int found = 0;
    for(int i=0; i< table->getRowCount(); i++){
        if(table->getRows()[i]->getValue(colIndex) == matchVal){
            Row* row = table->getRows()[i];
            for(int j=0; j< numCols; j++)
                cout<<left<<setw(widths[j])<<row->getValue(j);
            cout<<endl;
            found++;
        }
    }

    if(found == 0)
        cout<<"No records found where "<<colName<<" = "<<matchVal<<endl;
    cout<<endl;
}


void deleteFrom(Table* table){
    char colBuffer[256];
    char valBuffer[256];
    cout<<"  Column name: ";
    cin>>colBuffer;
    cout<<"  Value to match: ";
    cin>>valBuffer;

    string colName = colBuffer;
    string matchVal = valBuffer;

    int colIndex = -1;
    for(int i=0; i< table->getColumnCount(); i++){
        if(table->getColumns()[i].getName() == colName){
            colIndex = i;
            break;
        }
    }
    if(colIndex == -1){
        cout<<"Error: Column '"<<colName<<"' not found."<<endl;
        return;
    }

    int deleted = 0;
    for(int i= table->getRowCount()-1; i>=0; i--){
        if(table->getRows()[i]->getValue(colIndex) == matchVal){
            table->deleteRow(i);
            deleted++;
        }
    }

    if(deleted > 0)
        cout<<deleted<<" record(s) deleted."<<endl;
    else
        cout<<"No matching records found."<<endl;
}


void saveToFile(Table* table){
    char fileBuffer[256];
    cout<<"Enter filename: ";
    cin>>fileBuffer;

    ofstream fout(fileBuffer);
    if(!fout.is_open()){
        cout<<"Error: Could not open file."<<endl;
        return;
    }

    fout<<"TABLE "<<table->getTableName()<<endl;

    for(int i=0; i< table->getColumnCount(); i++){
        Column& col = table->getColumns()[i];
        fout<<col.getName()<<" "<<col.getType()<<" "<<col.getConstraints()<<endl;
    }


    fout<<"DATA"<<endl;

    for(int i=0; i< table->getRowCount(); i++){
        Row* row = table->getRows()[i];
        for(int j=0; j< row->getSize(); j++){
            if(j > 0) fout<<" ";
            fout<<row->getValue(j);
        }
        fout<<endl;
    }

    fout.close();
    cout<<"Table saved to '"<<fileBuffer<<"'."<<endl;
}


Table* loadFromFile(){
    char fileBuffer[256];
    cout<<"Enter filename: ";
    cin>>fileBuffer;

    ifstream fin(fileBuffer);
    if(!fin.is_open()){
        cout<<"Error: Could not open file."<<endl;
        return NULL;
    }

    char lineBuffer[256];
    string tableName = "";
    vector<Column> columns;


    fin.getline(lineBuffer, 256);
    string firstLine = lineBuffer;
    if(firstLine.substr(0,6) == "TABLE "){
        tableName = firstLine.substr(6);
    } else {
        cout<<"Error: Invalid file format."<<endl;
        fin.close();
        return NULL;
    }

    while(fin.getline(lineBuffer, 256)){
        string line = lineBuffer;
        if(line == "DATA") break;

        stringstream ss(line);
        string cName, cType;
        unsigned int cFlag;
        ss >> cName >> cType >> cFlag;
        columns.push_back(Column(cName, cType, cFlag));
    }

    Table* table = new Table(tableName, columns);

    // read rows
    int numCols = columns.size();
    while(fin.getline(lineBuffer, 256)){
        string line = lineBuffer;
        if(line.empty()) continue;

        stringstream ss(line);
        vector<string> values;
        string val;
        for(int i=0; i< numCols; i++){
            ss >> val;
            values.push_back(val);
        }
        Row* newRow = new Row(values);
        table->addRow(newRow);
    }

    fin.close();
    cout<<"Table '"<<tableName<<"' loaded from '"<<fileBuffer<<"'."<<endl;
    return table;
}


int main(){
    Table* currentTable = NULL;
    char cmdBuffer[256];
    bool running = true;

    cout<<"================================"<<endl;
    cout<<"    Mini Database Engine"<<endl;
    cout<<"================================"<<endl;

    while(running){
        cout<<"\n--- Menu ---"<<endl;
        cout<<"1) CREATE TABLE"<<endl;
        cout<<"2) INSERT INTO"<<endl;
        cout<<"3) SELECT * FROM"<<endl;
        cout<<"4) SELECT WHERE"<<endl;
        cout<<"5) DELETE FROM"<<endl;
        cout<<"6) SAVE TO FILE"<<endl;
        cout<<"7) LOAD FROM FILE"<<endl;
        cout<<"8) EXIT"<<endl;
        cout<<"Enter choice: ";
        cin>>cmdBuffer;

        int choice = atoi(cmdBuffer);

        switch(choice){
            case 1:
                if(currentTable != NULL){
                    delete currentTable;
                    currentTable = NULL;
                }
                currentTable = createTable();
                break;

            case 2:
                if(currentTable == NULL)
                    cout<<"No table exists. Create one first."<<endl;
                else
                    insertInto(currentTable);
                break;

            case 3:
                if(currentTable == NULL)
                    cout<<"No table exists."<<endl;
                else
                    selectAll(currentTable);
                break;

            case 4:
                if(currentTable == NULL)
                    cout<<"No table exists."<<endl;
                else
                    selectWhere(currentTable);
                break;

            case 5:
                if(currentTable == NULL)
                    cout<<"No table exists."<<endl;
                else
                    deleteFrom(currentTable);
                break;

            case 6:
                if(currentTable == NULL)
                    cout<<"No table exists."<<endl;
                else
                    saveToFile(currentTable);
                break;

            case 7:
                if(currentTable != NULL){
                    delete currentTable;
                    currentTable = NULL;
                }
                currentTable = loadFromFile();
                break;

            case 8:
                running = false;
                break;

            default:
                cout<<"Invalid choice."<<endl;
                break;
        }
    }
//cleanup
    if(currentTable != NULL)
        delete currentTable;

    cout<<"Goodbye!"<<endl;
    return 0;
}
