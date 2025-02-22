#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

int addr_bit;
int blk_size;
int num_of_sets; 
int asso;

class node{
    public:
        bool v_bit; // valid bit
        bool NRU_bit;
        string tag;
        node(){v_bit = false; NRU_bit = true;} // initialize a block
};

bool hit(node* curr_set, string ref_tag){
    for(int i = 0;i < asso;i++){
        if (ref_tag == curr_set[i].tag){
            if (curr_set[i].v_bit == true)
                return true;
        }
    }
    return false;
}

void replacement(node* curr_set, string ref_tag){
    for(int i = 0;i < asso;i ++){
        if (curr_set[i].NRU_bit == 1){
            // i^th block is the victim block
            // do the replacement
            curr_set[i].tag = ref_tag;
            curr_set[i].NRU_bit = 0;
            curr_set[i].v_bit = 1;
            return;
        }
    }
    // case of "full"
    for(int i = 0;i < asso;i ++){
        curr_set[i].NRU_bit = 1;
    }
    // replace head block
    curr_set[0].tag = ref_tag;
    curr_set[0].NRU_bit = 0;
    curr_set[0].v_bit = 1;
    return;
}

int bin_to_int(string str){
    int sum = 0;
    for (auto a : str){
        sum = sum*2 + (a - '0');
    }
    return sum;
}

int main(int argc, char *argv[]){
	ifstream cache_setting;
	ifstream ref_f;
	ofstream report_file;
	cache_setting.open((string)argv[1]);
    /*if (!cache_setting.is_open()){
        cout << "fail to open" << endl;
        return 1;
    }*/
    ref_f.open((string)argv[2]);
    report_file.open((string)argv[3]);


    //cache setting init
    int temp_int[4];
    string temp;
    for(int i = 0;i < 4;i++){
        cache_setting >> temp >> temp_int[i];
    }
	addr_bit = temp_int[0];
    blk_size = temp_int[1];
    num_of_sets = temp_int[2];
    asso = temp_int[3];
    /*for (int i = 0;i < 4;i ++){
        cout << "readed cache setting: " << endl;
        cout << temp_int[i] << endl;
    }*/


    //reference list init
    vector<string> ref_lst;
    string bench_str;
    getline(ref_f , bench_str);
    while(!ref_f.eof()){
        ref_f >> temp;
        ref_lst.push_back(temp);
    }
    ref_lst.pop_back();
    ref_lst.pop_back();
    /*cout << "ref by file: " << endl;
    for (auto a : ref_lst){
        cout << a << endl;
    }*/


    //handle indexes
    int offset_bit = log2(blk_size);
    int indexing_bit_count = log2(num_of_sets);
    int tag_length = addr_bit - offset_bit - indexing_bit_count;
    vector<string> ref_indexes;
    int j = 0;
    for(auto a : ref_lst){
        ref_indexes.push_back(a.substr(tag_length , indexing_bit_count));
        j++;
    }
    int num_of_ref = j;


	//initialize cache data structure
	node **cache_line = new node*[num_of_sets];
	for (int i = 0;i < num_of_sets;i++){
        cache_line[i] = new node[asso];
	}
    //cout << "end of alloc" << endl;


    //start to ref
    bool ref_result[num_of_ref];
    string tag;
    int miss_count = 0;
    for (int i = 0;i < num_of_ref;i++){
        tag = ref_lst[i].substr(0 , tag_length);
        int curr_set_num = bin_to_int(ref_indexes[i]);
        ref_result[i] = hit( cache_line[curr_set_num] , tag ); //knowing hit or miss
        if (!ref_result[i]){
            //miss, need replacement
            replacement( cache_line[curr_set_num] , tag );
            miss_count ++;
        }
    }

    //handle output file
    report_file << "Address bits: " << addr_bit << endl;
    report_file << "Block size: " << blk_size << endl;
    report_file << "Cache sets: " << num_of_sets << endl;
    report_file << "Associativity: " << asso << '\n' << endl;
    report_file << "Offset bit count: " << offset_bit << endl;
    report_file << "Indexing bit count: " << indexing_bit_count << endl;
    report_file << "Indexing bits: " ;
    //output indexes of indexing bits
    int tempindex = (addr_bit - 1) - tag_length;
    for (int i = tempindex;i > (tempindex - indexing_bit_count);i --){
        report_file << i;
        if (i != 1 + (tempindex - indexing_bit_count)){
            //output space unless the last time
            report_file << ' ';
        }
    }
    report_file << endl << endl;
    report_file << bench_str << endl;
    for (int i = 0;i < num_of_ref;i ++){
        report_file << ref_lst[i] << ' ' << ((ref_result[i] == true)?"hit":"miss") << endl;
    }
    report_file << ".end" << endl << endl << "Total cache miss count: " << miss_count;


    //release alloc
    for (int i = 0;i < num_of_sets;i ++){
        delete [] cache_line[i];
    }
    delete [] cache_line;

	return 0;
}


