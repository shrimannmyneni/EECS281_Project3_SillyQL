// Project identifier: C0F4DFE8B340D81183C208F70F9D2D797908754D

//STL Libraries
#include <iostream>
#include <queue>
#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <iterator>
#include <map>
#include <unordered_map>
#include <getopt.h>
#include <sstream>

#include "TableEntry.h"
//other files

enum class IndexType : uint8_t {
    HASH,
    BST,
    NONE
};

struct Table {
    //default constructor?

    std::string table_name;
    bool quiet = false;

    std::vector<EntryType> columnTypes;
    std::vector<std::string> columnNames;
    std::vector<std::vector<TableEntry>> table2D;
    std::map<TableEntry, std::vector<size_t>>bst;
    std::unordered_map <TableEntry, std::vector<size_t>> hashTable;
    IndexType index_type = IndexType::NONE;
    size_t col_of_generated_index;
    bool can_use_generated_index = false;

struct equal {
private:
    size_t index_;
    TableEntry entry_;
    EntryType columnType_;
    static constexpr double EPSILON = 1e-9;

public:
    equal(size_t index, const TableEntry& entry, EntryType columnType)
        : index_(index), entry_(entry), columnType_(columnType) {}

    bool operator()(const std::vector<TableEntry>& table_entry) const {
        if (columnType_ == EntryType::Double) {
            std::ostringstream entryStream, tableStream;
            
            entryStream << entry_;
            tableStream << table_entry[index_];

            std::string entryString = entryStream.str();
            std::string tableString = tableStream.str();

            double entryValue = std::stof(entryString);
            double tableValue = std::stof(tableString);

            double diff = std::abs(entryValue - tableValue);
            return diff < EPSILON;
        } else {
            switch (columnType_) {
                case EntryType::Int:
                    return table_entry[index_] == entry_;
                case EntryType::String:
                    return table_entry[index_] == entry_;
                case EntryType::Bool:
                    return table_entry[index_] == entry_;
                default:
                    throw std::runtime_error("Unsupported type for comparison");
            }
        }
    }
};

    struct less {
        private:
            size_t index_;
            TableEntry entry_;
        public:
            less(size_t index, const TableEntry &entry): index_(index), entry_(entry){};
            bool operator()(const std::vector<TableEntry> &table_entry){
                return table_entry[index_] < entry_;
            }
            operator bool() const{return true;}
    };
    struct greater {
        private:
            size_t index_;
            TableEntry entry_;
        public:
            greater(size_t index, const TableEntry &entry): index_(index), entry_(entry){};
            bool operator()(const std::vector<TableEntry> &table_entry){
                return table_entry[index_] > entry_;
            }
            operator bool() const{return true;}
    };
    

    void insert();
    // ERROR(1) <tablename> is not the name of a table in the database
    void print(); // different in quiet mode
        void print_rows_helper(size_t print_compare_column_index, char op, const TableEntry& value, std::vector<size_t>& columns_to_print);
    //ERROR(1) <tablename> is not the name of a table in the database
    //ERROR(2) <colname> is not the name of a column in the table specified by <tablename>
    //ERROR(3) One (or more) of the <print_colname>s are not the name of a column in the table specified by <tablename> (only print the name of the first such column encountered)
    void delete_rows();
    // ERROR(1) <tablename> is not the name of a table in the database
        void delete_rows_helper(size_t delete_column_index, char op, const TableEntry& value);
    // ERROR(2) <colname> is not the name of a column in the table specified by <tablename>
    void generate();
    
};

class SillyQL {
    private:
    bool quietMode = false;
    std::unordered_map<std::string, Table> DBmap;
    
    public:
    void get_options(int argc, char* argv[]);
    void read_input();
    //database commands
    void create();
    // ERROR(1) A table named <tablename> already exists in the database
    void quit();
    // ERROR(1) Every interactive session or redirected input file should end with a QUIT command.
    void comment(); 
    void remove();
    // ERROR(1) Possible Error: <tablename> is not the name of a table in the database
    void join(const Table& table1, const Table& table2);
    //ERROR(1) <tablenameX> is not the name of a table in the database
    //ERROR(2) One (or more) of the <colname>s or <print_colname>s are not the name of a column in the table specified by <tablenameX> (only print the name of the first such column encountered)
    void generate_col_idx(const Table& table, size_t column_idx, std::unordered_map<TableEntry, std::vector<size_t>>& umap);
    void handle_command(std::string& command);
};

 //unordered map from table name to the table class (that I create)
    void SillyQL::get_options(int argc, char* argv[]){
        int optionIdx = 0, option = 0;
        opterr = false;

        struct option longOpts [] = {
            {"help", no_argument, nullptr, 'h'},
            {"quiet", no_argument, nullptr, 'q'},
            {nullptr, no_argument, nullptr, '\0'}
        };
        
        while ((option = getopt_long(argc, argv, "hq", longOpts, &optionIdx)) != -1){
            switch(option){
                case 'h':
                    std::cerr << "Help Mode\n";
                    exit(0);

                case 'q':
                    quietMode = true;
                    std::cerr << "Quiet Mode\n";
                    break;

                default:
                    std::cerr << "Unknown option\n";
                    exit(1);
            }
        }
    }//end of get_options

    void SillyQL::read_input(){
        std::string cmd;
        do {
            std::cout << "% ";
            std::cin >> cmd;
            if (std::cin.fail()) {
                std::cerr << "Reading from cin has failed" << '\n';
                exit(1);
            } 
            handle_command(cmd);
        } 
    while (cmd != "QUIT");
    } 

       void SillyQL::handle_command(std::string& command){
            if (command[0] == 'C'){
                create();
            } else if (command[0] == 'R'){
                remove();
            } else if (command[0] == '#'){
                comment();
            } else if (command[0] == 'I'){
                std::cin >> command; // "INTO"
                std::cin >> command; // <tablename>
                auto it = DBmap.find(command);
                if (it == DBmap.end()){
                    std::cout << "Error during INSERT: " << command << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else {
                    DBmap[command].insert();
                }
            } else if (command[0] == 'P'){
                std::cin >> command; // "FROM"
                std::cin >> command; //<tablename>
                auto it = DBmap.find(command);
                if (it == DBmap.end()){
                    std::cout << "Error during PRINT: " << command << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else {
                    DBmap[command].print();
                }
            } else if (command[0] == 'D'){
                std::cin >> command;// "FROM"
                std::cin >> command; //<tablename>
                auto it = DBmap.find(command);
                if (it == DBmap.end()){
                    std::cout << "Error during DELETE: " << command << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else {
                    DBmap[command].delete_rows();
                }
            } else if (command[0] == 'J'){
                std::string name_of_table1;
                std::string name_of_table2;

                std::cin >> name_of_table1; //Name of Table 1
                auto it_tb1 = DBmap.find(name_of_table1);
                std::cin >> command; // "AND"
                std::cin >> name_of_table2; // Name of Table 2
                auto it_tb2 = DBmap.find(name_of_table2);


                if (it_tb1 == DBmap.end()){
                    std::cout << "Error during JOIN: " << name_of_table1 << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else if (it_tb2 == DBmap.end()){
                    std::cout << "Error during JOIN: " << name_of_table2 << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else {
                    join(DBmap[name_of_table1], DBmap[name_of_table2]);
                }
            } else if (command[0] == 'G'){
                std::string table_name;
                std::cin >> table_name; // FOR
                std::cin >> table_name; // <tablename>
                auto it = DBmap.find(table_name);
                if (it == DBmap.end()){
                    std::cout << "Error during GENERATE: " << table_name << " does not name a table in the database\n";
                    std::getline(std::cin, command);
                } else {
                    DBmap[table_name].generate();
                }
            } else if (command[0] == 'Q'){
                std::cout << "Thanks for being silly!\n";
            } else {
                std::cout << "Error: unrecognized command\n";
                std::getline(std::cin, command);
            }
    }// end of read input


    // DB commands
//could be create thats causing the issue
void SillyQL::create(){
    //  std::cerr << "create\n";
    std::string input;
    std::string tableName;
    std::cin >> tableName;

    auto found_it = DBmap.find(tableName);
    if (found_it != DBmap.end()){
        std::cout << "Error during CREATE: Cannot create already existing table " << tableName << '\n';
        std::getline(std::cin, input);
        return;
    }

    Table t;
    t.table_name = tableName;

    size_t N;
    std::cin >> N;

    t.columnNames.reserve(N);
    t.columnTypes.reserve(N);

    for (size_t i = 0; i < N; ++i){
        std::cin >> input;

        if (input == "string"){
            t.columnTypes.push_back(EntryType::String);
        } else if (input == "bool"){
            t.columnTypes.push_back(EntryType::Bool);
        } else if (input == "int"){
            t.columnTypes.push_back(EntryType::Int);
        } else if (input == "double"){
            t.columnTypes.push_back(EntryType::Double);
        }
    }

    for (size_t i = 0; i < N; ++i){
            std::cin >> input;
            t.columnNames.push_back(input);
    }

    if (quietMode){
        t.quiet = true;
    }

    // //assign table to key in unordered map
    DBmap.emplace(tableName, t); // right here

    //New table <tablename> with column(s) <colname1> <colname2> ... <colnameN> created
    std::cout << "New table " << tableName << " with column(s) ";
    for (size_t i = 0; i < N; ++i){
        std::cout << t.columnNames[i] << " ";
    }
    std::cout << "created\n";

} // end of DB::create

void SillyQL::remove() {
    std::string table_name;
    std::cin >> table_name;

    auto it = DBmap.find(table_name);
    if (it != DBmap.end()) {
        DBmap.erase(it);
        std::cout << "Table " << table_name << " removed\n"; 
    } else {
        std::cout << "Error during REMOVE: " << table_name << " does not name a table in the database\n"; 
    }
}

void SillyQL::comment(){
    std::string line;
    std::getline(std::cin, line);

    //finish
} // end of DB::comment

void Table::insert(){
    size_t N;
    std::cin >> N;

    std::string junk;
    std::cin >> junk;

    size_t curr_size = table2D.size();
    table2D.resize(curr_size + N);

    for (size_t i = curr_size; i < curr_size + N; ++i){
        table2D[i].reserve(columnNames.size());

        for (size_t j = 0; j < columnNames.size(); ++j){

            std::string input;
            std::cin >> input;

            if (columnTypes[j] == EntryType::String){
                table2D[i].emplace_back(TableEntry(input));
            } else if (columnTypes[j] == EntryType::Bool){
                if (input == "true"){ 
                    table2D[i].emplace_back(TableEntry(true));
                }
                else{ 
                    table2D[i].emplace_back(TableEntry(false));
                }
            } else if (columnTypes[j] == EntryType::Int){ 
                table2D[i].emplace_back(TableEntry(std::stoi(input)));
            } else if (columnTypes[j] == EntryType::Double){
                table2D[i].emplace_back(TableEntry(std::stof(input)));
            } /*else if (columnTypes[j] == EntryType::Double){
                std::ostringstream oss;
                oss << input;
                std::string val_str = oss.str();

                size_t dec_pos = val_str.find('.');
                int decimal_places = static_cast<int>(val_str.length() - dec_pos - 1);

                double rounder = std::pow(10,decimal_places);

                double d = std::round(std::stof(input) * rounder) / rounder;
           
                table2D[i].emplace_back(TableEntry(d));
            }*/
            }
    }

        if (index_type == IndexType::HASH){
            for (size_t i = curr_size; i < curr_size + N; ++i){
                hashTable[table2D[i][col_of_generated_index]].push_back(i);
            }
            can_use_generated_index = (hashTable.size() == 0) ? false : true; 
        } else if (index_type == IndexType::BST){
            for (size_t i = curr_size; i < curr_size + N; ++i){
                bst[table2D[i][col_of_generated_index]].push_back(i);
            }
            can_use_generated_index = (bst.size() == 0) ? false : true; 
        }



    //     std::cout << "Debugging Table: " << table_name << std::endl;
    // std::cout << "Column Names: ";
    // for (const auto& columnName : columnNames) {
    //     std::cout << columnName << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "Table Data:" << std::endl;
    // for (const auto& row : table2D) {
    //     for (const auto& entry : row) {
    //         std::cout << entry << " ";
    //     }
    //     std::cout << std::endl;
    // }


    std::cout << "Added " << N << " rows to " << table_name << " from position " << curr_size << " to " << curr_size + N - 1 << '\n';

}// end of Table::insert

void Table::print(){
    size_t N;
    std::cin >> N;
    std::vector<size_t> print_table_col_idxs;
    std::string name_of_col;

    for (size_t i = 0; i < N; ++i){
        std::cin >> name_of_col;
        auto it = std::find(columnNames.begin(), columnNames.end(), name_of_col);
         if (it == columnNames.end()){
            std::cout << "Error during PRINT: " << name_of_col << " does not name a column in " << table_name << '\n';
            std::getline(std::cin, name_of_col);
            return;
        }
        size_t comp_idx = static_cast<size_t>(std::distance(columnNames.begin(), it));
        print_table_col_idxs.push_back(comp_idx);
    }

        std::cin >> name_of_col; // WHERE / ALL

        //ALL
        if (name_of_col == "ALL"){
        if (!quiet){
            for (size_t c = 0; c < print_table_col_idxs.size(); ++c){
                std::cout << columnNames[print_table_col_idxs[c]] << " ";
            }
            std::cout << '\n';

            for (size_t i = 0; i < table2D.size(); ++i){
                for (size_t c = 0; c < print_table_col_idxs.size(); ++c){
                    std::cout << table2D[i][print_table_col_idxs[c]] << " ";
                }
                std::cout << '\n';
            }
        }
        std::cout << "Printed " << table2D.size() << " matching rows from " << table_name << '\n';
        return;
        }

        //WHERE: check if colname is valid
        std::cin >> name_of_col; // WHERE colname
        auto it = std::find(columnNames.begin(), columnNames.end(), name_of_col);
        if (it == columnNames.end()){
            std::cout << "Error during PRINT: " << name_of_col << " does not name a column in " << table_name << '\n';
            std::getline(std::cin, name_of_col);
            return;
        }
        size_t comp_idx = static_cast<size_t>(std::distance(columnNames.begin(), it));
        
        char opr;
        std::cin >> opr;


        switch(columnTypes[comp_idx]){
            case EntryType::Bool :{
                bool b;
                std::cin >> b;
                TableEntry t(b);
                print_rows_helper(comp_idx, opr, t, print_table_col_idxs);
                break;
            }
            case EntryType::String :{
                std::string s;
                std::cin >> s;
                TableEntry t(s);
                print_rows_helper(comp_idx, opr, t, print_table_col_idxs);
                break;
            }
            case EntryType::Double :{
                double d;
                std::cin >> d;
                TableEntry t(d);
                print_rows_helper(comp_idx, opr, t, print_table_col_idxs);
                break;
            }
            case EntryType::Int :{
                int i;
                std::cin >> i;
                TableEntry t(i);
                print_rows_helper(comp_idx, opr, t, print_table_col_idxs);
                break;
            }
        }
    }

void Table::print_rows_helper(size_t print_compare_column_index, char op, const TableEntry& value, std::vector<size_t>& columns_to_print){
        //size of col rows to output will tell how many rows we need to print
        size_t num_rows_printed = 0;

        if (!quiet){
            for (size_t i = 0; i < columns_to_print.size(); ++i){
                std::cout << columnNames[columns_to_print[i]] << " ";
            }
            std::cout << '\n';
        }

        switch (op){
            case '<':
            {
                    if (index_type == IndexType::BST && print_compare_column_index == col_of_generated_index && can_use_generated_index){

                        auto search_for_idx = bst.lower_bound(value);
                        for (auto x = bst.begin(); x != search_for_idx; x++) {
                            for (size_t r : x->second) {
                                if (!quiet) {
                                    for (size_t c : columns_to_print) {
                                        std::cout << table2D[r][c] << " ";
                                    }
                                    std::cout << '\n';
                                }
                            num_rows_printed++;
                            }
                        }

                    } else {

                        for (size_t row = 0; row < table2D.size(); ++row){
                            if (table2D[row][print_compare_column_index] < value){
                            if (!quiet){
                            for (size_t i = 0; i < columns_to_print.size(); ++i){
                                std::cout << table2D[row][columns_to_print[i]] << " ";
                            }
                            std::cout << '\n';
                            }
                            ++num_rows_printed;
                            }
                        }

                    }
                std::cout << "Printed " << num_rows_printed << " matching rows from " << table_name << '\n';
                break;
            }
            case '>':
            {
                if (index_type == IndexType::BST && print_compare_column_index == col_of_generated_index && can_use_generated_index){
                        auto search_for_idx = bst.upper_bound(value);

                        for (auto x = search_for_idx; x != bst.end(); ++x) {
                            for (size_t r : x->second) {
                                if (!quiet) {
                                    for (size_t c : columns_to_print) {
                                        std::cout << table2D[r][c] << " ";
                                    }
                                std::cout << '\n';
                                }
                            ++num_rows_printed;
                            }
                        }

                    } else {

                        for (size_t row = 0; row < table2D.size(); ++row){
                            if (table2D[row][print_compare_column_index] > value){
                            if (!quiet){
                            for (size_t i = 0; i < columns_to_print.size(); ++i){
                                std::cout << table2D[row][columns_to_print[i]] << " ";
                            }
                            std::cout << '\n';
                            }
                            ++num_rows_printed;
                            }
                        }
                        
                    }
                std::cout << "Printed " << num_rows_printed << " matching rows from " << table_name << '\n';
                break;
            }
            case '=':
            {   
   
            if (index_type == IndexType::BST && print_compare_column_index == col_of_generated_index && can_use_generated_index) {
                auto range = bst.equal_range(value); // Get range of elements equal to value

                for (auto it = range.first; it != range.second; ++it) {
                    for (size_t r : it->second) {
                        if (!quiet) {
                            for (size_t col_idx : columns_to_print) {
                                std::cout << table2D[r][col_idx] << " ";
                            }
                            std::cout << '\n';
                        }
                        ++num_rows_printed;
                    }
                }

                // auto idx = bst.find(value);
                // if (idx != bst.end()){
                //     num_rows_printed += idx->second.size();
                //     if (!quiet){
                //         for (size_t r : idx->second){
                //             for (size_t c: columns_to_print){
                //                 std::cout << table2D[r][c] << " ";
                //             }
                //             std::cout << '\n';
                //         }
                //     }
                // }

                } else if (index_type == IndexType::HASH && col_of_generated_index == print_compare_column_index && can_use_generated_index){
                        
                        auto it = hashTable.find(value); // Find the entry in hash table
                        if (it != hashTable.end()) {
                        for (size_t r : it->second) {
                        if (!quiet) {
                            for (size_t col_idx : columns_to_print) {
                                std::cout << table2D[r][col_idx] << " ";
                            }
                            std::cout << '\n';
                        }
                        ++num_rows_printed;
                        }
                }
                } else {
                for (size_t row = 0; row < table2D.size(); ++row){
                    if (table2D[row][print_compare_column_index] == value){
                        if (!quiet){
                            for (size_t i = 0; i < columns_to_print.size(); ++i){
                                std::cout << table2D[row][columns_to_print[i]] << " ";
                            }
                            std::cout << '\n';
                        }
                        ++num_rows_printed;
                    }
                }
            }

            std::cout << "Printed " << num_rows_printed << " matching rows from " << table_name << '\n';
            break;

            default:
                std::cerr << "Operator must be one of >, <, =\n";
                return;
        }
    }
}

void Table::delete_rows(){
        std::string colname;
        std::cin >> colname; // WHERE
        std::cin >> colname; // colname
       
        auto it = std::find(columnNames.begin(), columnNames.end(), colname);
        
        if (it == columnNames.end()){
            std::cout << "Error during DELETE: " << colname << " does not name a column in " << table_name << '\n';
            std::getline(std::cin, colname);
            return;
        }
        size_t comp_idx = static_cast<size_t>(std::distance(columnNames.begin(), it));

        // std::cout << "Index of column to compare: " << comp_idx;

        char opr;
        std::cin >> opr;

        switch(columnTypes[comp_idx]){
            case EntryType::Bool :{
                bool b;
                std::cin >> b;
                TableEntry t(b);
                delete_rows_helper(comp_idx, opr, t);
                break;
            }
            case EntryType::String :{
                std::string s;
                std::cin >> s;
                TableEntry t(s);
                delete_rows_helper(comp_idx, opr, t);
                break;
            }
            case EntryType::Double :{
                double d;
                std::cin >> d;
                TableEntry t(d);
                delete_rows_helper(comp_idx, opr, t);
                break;
            }
            case EntryType::Int :{
                int i;
                std::cin >> i;
                TableEntry t(i);
                delete_rows_helper(comp_idx, opr, t);
                break;
            }
        }
    }

void Table::delete_rows_helper(size_t delete_column_index, char op, const TableEntry& value){
        size_t num_rows_deleted = 0;
        size_t initial_size = table2D.size();

        switch (op){
            case '<':
            {
                less lessComp(delete_column_index, value);
                table2D.erase(remove_if(table2D.begin(), table2D.end(), lessComp), table2D.end());
                // for (size_t i = 0; i < table2D.size(); ++i){
                //     if (less{delete_column_index, value}(table2D[i])){
                //         rows_to_delete.push_back(i);
                //     }
                // }

                break;
            }
            case '>':
            {
                greater greaterComp(delete_column_index, value);
                table2D.erase(remove_if(table2D.begin(), table2D.end(), greaterComp), table2D.end());
                // for (size_t i = 0; i < table2D.size(); ++i){
                //     if (greater{delete_column_index, value}(table2D[i])){
                //         rows_to_delete.push_back(i);
                //     }
                // }

                break;
            }
            case '=':
            {
                equal equalComp(delete_column_index, value, columnTypes[delete_column_index]);
                table2D.erase(remove_if(table2D.begin(), table2D.end(), equalComp), table2D.end());
                // for (size_t i = 0; i < table2D.size(); ++i){
                //     if (equal{delete_column_index, value}(table2D[i])){
                //         rows_to_delete.push_back(i);
                //     }
                // }

                break;
            }
            default:
                std::cerr << "Operator must be one of >, <, =\n";
                return;
        }

        num_rows_deleted = initial_size - table2D.size();
        std::cout << "Deleted " << num_rows_deleted << " rows from " << table_name << '\n';

        if (index_type == IndexType::HASH){
            //regenerate Hash index
            hashTable.clear();
            for (size_t i = 0; i < table2D.size(); ++i){
                hashTable[table2D[i][col_of_generated_index]].push_back(i);
            }
            can_use_generated_index = (hashTable.size() == 0) ? false : true;
        } else if (index_type == IndexType::BST){
            //regenerate Bst index
            bst.clear();
            for (size_t i = 0; i < table2D.size(); ++i){
                bst[table2D[i][col_of_generated_index]].push_back(i);
            }
            can_use_generated_index = (bst.size() == 0) ? false : true;
        }
        
    }

void SillyQL::join(const Table& table1, const Table& table2){
        std::string table1_column_to_compare;
        std::string table2_column_to_compare;
        std::string junk;

        std::cin >> junk; //WHERE
        std::cin >> table1_column_to_compare; // Table 1's column to compare

        auto tb1_it = std::find(table1.columnNames.begin(), table1.columnNames.end(), table1_column_to_compare);
        if (tb1_it == table1.columnNames.end()){
            std::cout << "Error during JOIN: " << table1_column_to_compare << " does not name a column in " << table1.table_name << '\n';
            std::getline(std::cin, junk);
            return;
        }

        std::cin >> junk; // =
        std::cin >> table2_column_to_compare; // Table 2's column to compare


        auto tb2_it = std::find(table2.columnNames.begin(), table2.columnNames.end(), table2_column_to_compare);
        if (tb2_it == table2.columnNames.end()){
            std::cout << "Error during JOIN: " << table2_column_to_compare << " does not name a column in " << table2.table_name << '\n';
            std::getline(std::cin, junk);
            return;
        }

        //Now that we know that both table names exist
        std::cin >> junk; // "AND"
        std::cin >> junk; // "PRINT"

        size_t N; // num of columns to print from both tables
        std::cin >> N;

        std::string column_name_to_search;
        int table_num;

        std::vector< std::pair <int, size_t> > cols_ov;

        for (size_t i = 0; i < N; ++i){
            std::cin >> column_name_to_search;
            std::cin >> table_num;

            const Table& table_to_search = (table_num == 1) ? table1 : table2;
            auto tb_it = std::find(table_to_search.columnNames.begin(), table_to_search.columnNames.end(), column_name_to_search);
            if (tb_it == table_to_search.columnNames.end()){
                std::cout << "Error during JOIN: " << column_name_to_search << " does not name a column in " << table_to_search.table_name << '\n';
                std::getline(std::cin, junk);
                return;
            } else {
                size_t col_idx = static_cast<size_t>(std::distance(table_to_search.columnNames.begin(), tb_it));
                cols_ov.push_back(std::pair(table_num, col_idx));
            }
        }

        if (!table1.quiet){
            for (size_t i = 0; i < cols_ov.size(); ++i){
                const Table& output_table = (cols_ov[i].first == 1) ? table1 : table2;
                std::cout << output_table.columnNames[cols_ov[i].second] << " ";
            }
            std::cout << '\n';
        }
        
        size_t tb1_col_to_compare_idx = static_cast<size_t>(std::distance(table1.columnNames.begin(), tb1_it));
        size_t tb2_col_to_compare_idx = static_cast<size_t>(std::distance(table2.columnNames.begin(), tb2_it));

        size_t num_rows_printed = 0;
        
        //generate a hash map for the values of the desired column in table 2
        std::unordered_map<TableEntry, std::vector<size_t>> join_hash;
        generate_col_idx(table2, tb2_col_to_compare_idx, join_hash);

        for (size_t i = 0; i < table1.table2D.size(); ++i){
            const TableEntry& t1val = table1.table2D[i][tb1_col_to_compare_idx];

            auto hash_it = join_hash.find(t1val);
            if (hash_it != join_hash.end()){
                for (size_t j = 0; j < hash_it->second.size(); ++j){
                    if (!quietMode){
                    for (size_t k = 0; k < cols_ov.size(); ++k){
                        const Table& table_to_print = (cols_ov[k].first == 1) ? table1 : table2;
                        size_t row_idx_to_print = (cols_ov[k].first == 1) ? i : hash_it->second[j];
                        std::cout << table_to_print.table2D[row_idx_to_print][cols_ov[k].second] << " ";
                    }
                    std::cout << '\n';
                    }
                    ++num_rows_printed;
                }
            }
        }
        std::cout << "Printed " << num_rows_printed << " rows from joining " << table1.table_name << " to " << table2.table_name << '\n';
    }// end of join()

// helper functions
void SillyQL::generate_col_idx(const Table& table, size_t column_idx, std::unordered_map<TableEntry, std::vector<size_t>>& umap){
        for (size_t i = 0; i < table.table2D.size(); ++i){
            umap[table.table2D[i][column_idx]].emplace_back(i);
        }
    }

void Table::generate(){
    std::string input_indextype, col_name;
    std::cin >> input_indextype >> col_name >> col_name >> col_name;

    //CHEKCKING IF THE column name exists
    auto column_it = std::find(columnNames.begin(), columnNames.end(), col_name);
    if (column_it == columnNames.end()){
        std::cout << "Error during GENERATE: " << col_name << " does not name a column in " << table_name << '\n';
        std::getline(std::cin, col_name);
        return;
    }

    //If an invalid index request is made, do not discard any existing index.
    if (input_indextype != "hash" && input_indextype != "bst"){
        return;
    }

    if (index_type != IndexType::NONE){
        if (index_type == IndexType::HASH){
            hashTable.clear();
            bst.clear();
        } else {
            bst.clear();
            hashTable.clear();
        }
    }
    
    //now we initialize the new index depending on if hash or bst
    size_t column_idx = static_cast<size_t>(std::distance(columnNames.begin(), column_it));
    col_of_generated_index = column_idx;
    if (input_indextype == "hash"){

        index_type = IndexType::HASH;

        for (size_t i = 0; i < table2D.size(); ++i){
            hashTable[table2D[i][column_idx]].push_back(i);
        }

        can_use_generated_index = (hashTable.size() == 0) ? false : true;

    std::cout << "Created hash index for table " << table_name << " on column " << 
    col_name << ", with " << hashTable.size() << " distinct keys\n";

    } else {
        index_type = IndexType::BST;

        for (size_t i = 0; i < table2D.size(); ++i){
            auto it = bst.find(table2D[i][column_idx]);

            if (it != bst.end()) {
                it->second.push_back(i);
            } else {
                bst[table2D[i][column_idx]] = std::vector<size_t>{i};
            }
        }
        can_use_generated_index = (bst.size() == 0) ? false : true;

        std::cout << "Created bst index for table " << table_name << " on column " << 
        col_name << ", with " << bst.size() << " distinct keys\n";
    }
}

int main(int argc, char* argv[]){
    std::ios_base::sync_with_stdio(false);
    std::cin >> std::boolalpha;
    std::cout << std::boolalpha;
    SillyQL ImFinna;
    ImFinna.get_options(argc, argv);
    ImFinna.read_input();
    return 0;
};  

     // for (size_t i = 0; i < table1.table2D.size(); ++i){
        //     const TableEntry& t1val = table1.table2D[i][tb1_col_to_compare_idx];
            
        //     for (size_t j = 0; j < table2.table2D.size(); ++j){
        //         const TableEntry& t2val = table2.table2D[j][tb2_col_to_compare_idx];

        //         if(t1val == t2val){
        //             if(!table1.quiet){
        //                 for (size_t k = 0; k < cols_ov.size(); ++k){
        //                     const Table& table_to_print = (cols_ov[k].first == 1) ? table1 : table2;
        //                     size_t row_idx_to_print = (cols_ov[k].first == 1) ? i : j;
        //                     std::cout << table_to_print.table2D[row_idx_to_print][cols_ov[k].second] << " ";
        //                 }
        //                 std::cout << '\n';
        //             }
        //             ++num_rows_printed;
        //         }
        //     }
        // }
