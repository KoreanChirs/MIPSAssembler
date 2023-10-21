#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <bitset>

//la명령어를 처음에 주소 할당할 때부터 검사 가자. 그래서 처음부터 제대로 주소 할당하자.
using namespace std;


long long data_address = 0x10000000;
long long text_address = 0x400000;
bool istext = false;
bool isdata = false;
string real_whole_ans;

vector<string> all_reserved = {"la",".data",".word",".text","addu","and","jr","nor","or","sltu","sll","srl","subu","addiu","andi","beq","bne","lui","lw","ori","sltiu","sw","lb","sb","j","jal"};
vector<string> all_instruction = {"la","addu","and","jr","nor","or","sltu","sll","srl","subu","addiu","andi","beq","bne","lui","lw","ori","sltiu","sw","lb","sb","j","jal"};
string Rformat[] = {"addu","and","jr","nor","or","sltu","sll","srl","subu"};
string Lformat[] = {"addiu","andi","beq","bne","lui","lw","ori","sltiu","sw","lb","sb"};
string jformat[] = {"j","jal"};
string specialformat = "la";

// first version of (address,data/instruction) 
vector<long long> adrss_vec;
vector<string> insturction_vec;
unordered_map<long long, string> memory_map; 

// label address 
unordered_map<string, long long> label_map;

// data section (address, data(string form))
vector<long long> data_memory_adress;
vector<string> data_memory_data;
unordered_map<long long, string> data_memory_map;

// text section (adress, string)
vector<long long> text_memory_adress;
vector<string> text_memory_data;
unordered_map<long long, string> text_memory_map;

// text section(adress, binary) address is same as above.
unordered_map<long long, long long> text_memory_add_bin;
vector<long long> text_memory_bin;


bool check_string_in_all_reserved(string str){
    auto i = find(all_reserved.begin(),all_reserved.end(),str);
    if(i == all_reserved.end()){
        return false;
    }
    else{
        return true;
    }
}

bool check_string_in_all_instruction(string str){
    auto i = find(all_instruction.begin(),all_instruction.end(),str);
    if(i == all_instruction.end()){
        return false;
    }
    else{
        return true;
    }
}

void generate_memory_map(string entireMIPScode){
    string value;
    stringstream ss(entireMIPScode);
    string now_vector;
    long long now_address;
    while(ss >> value){
        if(value.back() == ':'){
            value.erase(value.length()-1,1);
            if(isdata){
                label_map.insert(pair<string, long long>(value,data_address));
            }
            else{
                label_map.insert(pair<string, long long>(value,text_address));
            }
        }
        else if(!check_string_in_all_reserved(value)){
            now_vector += (value + string(" "));
        }
        else{
            if(value == string(".data")){
                isdata = true;
                istext = false;
            }
            else if(value == string(".text")){
                isdata = false;
                istext = true;
            }
            else if(value == string("la")){
                if(!now_vector.empty()){
                    memory_map.insert(pair<long long,string>(now_address,now_vector));
                    adrss_vec.push_back(now_address);
                    insturction_vec.push_back(now_vector);
                    }
                now_vector = string("");
                string one;
                string two;
                ss >> one;
                ss >> two;
                int address_of_label = label_map[two];
                stringstream st;
                st << hex << address_of_label;
                string adress_str_hex = st.str();
                bitset<32> adress_(address_of_label);
                string adress_str_binary = adress_.to_string();
                string front_address = adress_str_hex.substr(0,4);
                string end_address = adress_str_hex.substr(4,4);
                if(end_address == string("0000")){
                    now_address = text_address;
                    now_vector += string("lui") + string(" ") + one + string(" ") + string("0x") + front_address + string(" ");
                    memory_map.insert(pair<long long,string>(now_address,now_vector));
                    adrss_vec.push_back(now_address);
                    insturction_vec.push_back(now_vector);
                    text_address += 4;
                    now_vector = string("");
                }
                else{
                    now_address = text_address;
                    now_vector += string("lui") + string(" ") + one + string(" ") + string("0x") + front_address + string(" ");
                    memory_map.insert(pair<long long,string>(now_address,now_vector));
                    adrss_vec.push_back(now_address);
                    insturction_vec.push_back(now_vector);
                    text_address += 4;
                    now_vector = string("");
                    now_address = text_address;
                    now_vector += string("ori") + string(" ") + one + string(" ") + one + string(" ") + string("0x") + end_address + string(" ");
                    memory_map.insert(pair<long long,string>(now_address,now_vector));
                    adrss_vec.push_back(now_address);
                    insturction_vec.push_back(now_vector);
                    text_address += 4;
                    now_vector = string("");
                }
            }
            else{
                if(!now_vector.empty()){
                    memory_map.insert(pair<long long,string>(now_address,now_vector));
                    adrss_vec.push_back(now_address);
                    insturction_vec.push_back(now_vector);
                    }
                now_vector = string("");
                if(isdata){
                    now_vector += value + string(" ");
                    now_address = data_address;
                    data_address += 4;
                }
                else{
                    now_vector += value + string(" ");
                    now_address = text_address;
                    text_address += 4;
                }
            }
        }        
    }
    
    if(now_vector != string("")){
        memory_map.insert(pair<long long,string>(now_address,now_vector));
        adrss_vec.push_back(now_address);
        insturction_vec.push_back(now_vector);
    }
}

void generate_data_memory(){
    string front;
    string second;
    int j = 0;
    for(string i : insturction_vec){
        stringstream ss(i);
        ss >> front;
        if(front == string(".word")){
            ss >> second;
            data_memory_adress.push_back(adrss_vec[j]);
            data_memory_data.push_back(second);
            data_memory_map.insert(pair<long long, string>(adrss_vec[j],second));
        }
        j++;
    }
}

void generate_text_memory(){
    string front;
    int j = 0;
    for(string i : insturction_vec){
        stringstream ss(i);
        ss >> front;
        if(check_string_in_all_instruction(front)){
            text_memory_adress.push_back(adrss_vec[j]);
            text_memory_data.push_back(i);
            text_memory_map.insert(pair<long long, string>(adrss_vec[j],i));
        }
        j++;
    }
}

string generate_instrc_to_binary(){
    long long binary = 0;
    string first;
    int j = 0;
    string one;
    string two;
    string three;
    int one_int;
    int two_int;
    int three_int;
    int default_value;
    string whole_ans;

    for(string i : text_memory_data){ 
        stringstream ss(i);
        ss >> first;
        if((first == string("addiu")) || (first == string("ori")) || (first == string("sltiu"))){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            if(three.substr(0,2) == string("0x")){
                three_int = stoul(three,nullptr,16);
            }
            else{
                three_int = stoi(three);
            }
            if(first == string("addiu")){
                default_value = 9;
            }
            else if(first == string("sltiu")){
                default_value = stoul(string("0xb"),nullptr,16);
            }
            else{
                default_value = stoul(string("0xd"),nullptr,16);
            }
            
            bitset<6> a(default_value);
            bitset<5> b(two_int);
            bitset<5> c(one_int);
            bitset<16> d(three_int);
            whole_ans += (a.to_string() + b.to_string() + c.to_string() + d.to_string() + string("\n"));
        }
        else if(first == string("addu")){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            three.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            three_int = stoi(three);
            default_value = 0;
            int six_value = stoul(string("0x21"),nullptr,16);
            bitset<6> a(default_value);
            bitset<5> rs(two_int);
            bitset<5> rt(three_int);
            bitset<5> rd(one_int);
            bitset<5> b(default_value);
            bitset<6> c(six_value);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + rd.to_string() + b.to_string() + c.to_string() + string("\n"));
        }
        else if(first == string("and")){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            three.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            three_int = stoi(three);
            default_value = 0;
            int six_value = stoul(string("0x24"),nullptr,16);
            bitset<6> a(default_value);
            bitset<5> rs(two_int);
            bitset<5> rt(three_int);
            bitset<5> rd(one_int);
            bitset<5> b(default_value);
            bitset<6> c(six_value);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + rd.to_string() + b.to_string() + c.to_string() + string("\n"));
        }
        else if(first == string("andi")){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            if(three.substr(0,2) == string("0x")){
                three_int = stoul(three,nullptr,16);
            }
            else{
                three_int = stoi(three);
            }
            default_value = stoul(string("0xc"),nullptr,16);
            bitset<6> a(default_value);
            bitset<5> rs(two_int);
            bitset<5> rt(one_int);
            bitset<16> imm(three_int);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + imm.to_string() + string("\n"));
        }
        else if(first == string("beq")){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            long long adress = text_memory_adress[j];
            int pc = adress + 4;
            int label_adress = label_map[three];
            int offset = (label_adress-pc)/4;
            default_value = 4;
            bitset<6> a(default_value);
            bitset<5> rs(one_int);
            bitset<5> rt(two_int);
            bitset<16> offsett(offset);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + offsett.to_string() + string("\n"));
        }
        else if(first == string("bne")){
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            long long adress = text_memory_adress[j];
            int pc = adress + 4;
            int label_adress = label_map[three];
            int offset = (label_adress-pc)/4;
            default_value = 5;
            bitset<6> a(default_value);
            bitset<5> rs(one_int);
            bitset<5> rt(two_int);
            bitset<16> offsett(offset);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + offsett.to_string() + string("\n"));
        }
        else if(first == string("j")){
            ss >> one;
            default_value = 2;
            int label_adress = label_map[one];
            int target = label_adress/4;
            bitset<6> a(default_value);
            bitset<26> targett(target);
            whole_ans += (a.to_string() + targett.to_string() + string("\n"));
        }
        else if(first == string("jal")){
            ss >> one;
            default_value = 3;
            int label_adress = label_map[one];
            int target = label_adress/4;
            bitset<6> a(default_value);
            bitset<26> targett(target);
            whole_ans += (a.to_string() + targett.to_string() + string("\n"));
        }
        else if(first == string("jr")){
            ss >> one;
            one.erase(0,1);
            one_int = stoi(one);
            default_value = 0;
            int default_two = 8;
            bitset<6> a(default_value);
            bitset<15> b(default_value);
            bitset<5> rs(one_int);
            bitset<6> c(default_two);
            whole_ans += (a.to_string() + rs.to_string() + b.to_string() + c.to_string() + string("\n"));
        }
        else if(first == string("lui")){
            ss >> one;
            ss >> two;
            one.erase(0,1);
            one_int = stoi(one);
            if(two.substr(0,2) == string("0x")){
                two_int = stoul(two,nullptr,16);
            }
            else{
                two_int = stoi(two);
            }
            default_value = 0;
            int six_value = stoul(string("0xf"),nullptr,16);
            bitset<6> a(six_value);
            bitset<5> b(default_value);
            bitset<5> rt(one_int);
            bitset<16> imm(two_int);
            whole_ans += (a.to_string() + b.to_string() + rt.to_string() + imm.to_string() + string("\n"));
        }
        else if((first == string("lw")) || (first == string("lb")) || (first == string("sw")) || (first == string("sb"))){
            ss >> one;
            ss >> two;
            one.erase(0,1);
            int rt = stoi(one);
            int position_l = two.find('(');
            two.erase(position_l,1);
            int position_2 = two.find(')');
            two.erase(position_2,1);
            int posi = two.find('$');
            string two_front = two;
            string two_end = two;
            two_front.erase(posi,two.length()-posi);
            two_end.erase(0,posi+1);
            int rs = stoi(two_end);
            int offset = stoi(two_front);
            int six_value;
            if(first == string("lw")){
                six_value = stoul(string("0x23"),nullptr,16);
            }
            else if(first == string("lb")){
                six_value = stoul(string("0x20"),nullptr,16);
            }
            else if(first == string("sw")){
                six_value = stoul(string("0x2b"),nullptr,16);
            }
            else if(first == string("sb")){
                six_value = stoul(string("0x28"),nullptr,16);
            }

            bitset<6> a(six_value);
            bitset<5> rss(rs);
            bitset<5> rtt(rt);
            bitset<16> offsett(offset);
            whole_ans += (a.to_string() + rss.to_string() + rtt.to_string() + offsett.to_string() + string("\n"));
        }
        else if((first == string("nor")) || (first == string("or")) || (first == string("sltu")) || (first == string("subu"))){
            string default_end;
            if(first == string("nor")){
                default_end = string("0x27");
            }
            else if(first == string("sltu")){
                default_end = string("0x2b");
            }
            else if(first == string("subu")){
                default_end = string("0x23");
            }
            else{
                default_end = string("0x25");
            }
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            three.erase(0,1);
            one_int = stoi(one);
            two_int = stoi(two);
            three_int = stoi(three);
            default_value = 0;
            int default_end_int = stoul(default_end,nullptr,16);
            bitset<6> a(default_value);
            bitset<5> rs(two_int);
            bitset<5> rt(three_int);
            bitset<5> rd(one_int);
            bitset<5> b(default_value);
            bitset<6> c(default_end_int);
            whole_ans += (a.to_string() + rs.to_string() + rt.to_string() + rd.to_string() + b.to_string() + c.to_string() + string("\n"));
        }
        else if(first == string("sll") || (first == string("srl"))){
            int default_end_value;
            if(first == string("sll")){
                default_end_value = 0;
            }
            else if(first == string("srl")){
                default_end_value = 2;
            }
            ss >> one;
            ss >> two;
            ss >> three;
            one.erase(0,1);
            two.erase(0,1);
            if(three.substr(0,2) == string("0x")){
                three_int = stoul(three,nullptr,16);
            }
            else{
                three_int = stoi(three);
            }
            one_int = stoi(one);
            two_int = stoi(two);
            default_value = 0;
            bitset<6> a(default_value);
            bitset<5> b(default_value);
            bitset<5> rt(two_int);
            bitset<5> rd(one_int);
            bitset<5> shamt(three_int);
            bitset<6> end_(default_end_value);
            whole_ans += (a.to_string() + b.to_string() + rt.to_string() + rd.to_string() + shamt.to_string() + end_.to_string() + string("\n"));
        }
        j++;
    }
    return whole_ans;
}

string bin_to_hex(){
    string hex_ans;
    stringstream ss(real_whole_ans);
    string i;
    while(ss >> i){
        bitset<32> bin(i);
        unsigned long hex_value = bin.to_ulong();
        stringstream st;
        st << hex << hex_value;
        hex_ans += string("0x") + st.str() + string("\n");
    }
    return hex_ans;
}
int main(int argc, char* argv[]){
    ifstream assembly_file(argv[1]);
    string output_modify_name = string(argv[1]);
    output_modify_name.erase(output_modify_name.length()-2,2);
    string output_binary_name = output_modify_name + string(".txt");
    string output_hex_name = output_modify_name + string(".o");
    ofstream binary_outfile(output_binary_name, std::ios::binary);
    ofstream hex_outfile(output_hex_name);
    string whole_mips;
    string line;
    int data_;
    while(getline(assembly_file,line)){
        whole_mips += (line + string(" ")); 
    }
    generate_memory_map(whole_mips);
    generate_data_memory();
    generate_text_memory();
    string whole_answer = generate_instrc_to_binary();
    int len_data = data_memory_adress.size() * 4;
    int len_text = text_memory_adress.size() * 4;
    bitset<32> len_data_(len_data);
    bitset<32> len_text_(len_text);
    real_whole_ans += len_text_.to_string() + string("\n");
    real_whole_ans += len_data_.to_string() + string("\n");
    real_whole_ans += whole_answer;
    binary_outfile.write((len_text_.to_string() + string("\n")).c_str() ,len_text_.to_string().length()+1); 
    binary_outfile.write((len_data_.to_string() + string("\n")).c_str() ,len_data_.to_string().length()+1); 
    binary_outfile.write(whole_answer.c_str() ,whole_answer.length()); 
    for(string i : data_memory_data){
        if(i.substr(0,2) == string("0x")){
            data_ = stoul(i,nullptr,16);
        }
        else{
            data_ = stoi(i);
        }
        bitset<32> bts(data_);
        binary_outfile.write((bts.to_string() + string("\n")).c_str() ,bts.to_string().length()+1); 
        real_whole_ans += bts.to_string() + string("\n");
    }
    string hex_ans = bin_to_hex();
    hex_outfile.write(hex_ans.c_str(),hex_ans.length());
    assembly_file.close();
    binary_outfile.close();
}


