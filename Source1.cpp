#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cmath>
#include<cstring>
#include<string>
using namespace std;

ifstream src;
ifstream input;
ofstream of("mytext.txt");
//ofstream fout("out.txt");
istream &is = cin;
int current = 0;
const int maxn = 100;
int stacktop = 0;
int status = 1;

//const int maxn1 = 100;  //to be decieded
//const int maxn2 = 30;   //to be decieded
//char string[maxn1][maxn2];
char stack[4 * 1024 * 1024];
map<string, int>regismap;
//to deal with hazards
//The hazards that could occur:
//NOTOCE: In practice, you should start with step 5 
//STEP 1
//No 1 : deal with the storage, check if storage is true, if it is false,then you can not go on with this step
//STEP 2
//No 2 : get the registers, check the A1,A2,A3 in changeRegis[32], it is flase,then you can not let it in
//No 3 : check if the instruction is jump or syscall, branch, you shouldn't let anything in
//STEP 3
//No 4 : change the registers, at this time, you should turn changeRegis[x] to false.
//STEP 4
//No 5: if you deal with stotage at this step, change storage to false
//STEP 5
//No 6: change changeRegis[x] to true,storage,

//The function of changeRegis is to see if the value of registers should be changed but has not been changed yet
bool changeRegis[32];
bool storage = true;
int regis[32];
//int value[4 * 1024 * 1024];
//int numOfValue = 32;
int lo = 0, hi = 0;
void origin() {
	for (int e = 0;e < 32;++e) {
		regis[e] = 0;
		changeRegis[e] = true;
		//value[e] = 0;
	}
	memset(stack, 0, sizeof(stack));
	regis[29] = 4 * 1024 * 1024;
	regismap.insert(pair<string, int>("zero", 0));
	regismap.insert(pair<string, int>("at", 1));
	regismap.insert(pair<string, int>("v0", 2));
	regismap.insert(pair<string, int>("v1", 3));
	regismap.insert(pair<string, int>("a0", 4));
	regismap.insert(pair<string, int>("a1", 5));
	regismap.insert(pair<string, int>("a2", 6));
	regismap.insert(pair<string, int>("a3", 7));
	regismap.insert(pair<string, int>("t0", 8));
	regismap.insert(pair<string, int>("t1", 9));
	regismap.insert(pair<string, int>("t2", 10));
	regismap.insert(pair<string, int>("t3", 11));
	regismap.insert(pair<string, int>("t4", 12));
	regismap.insert(pair<string, int>("t5", 13));
	regismap.insert(pair<string, int>("t6", 14));
	regismap.insert(pair<string, int>("t7", 15));
	regismap.insert(pair<string, int>("s0", 16));
	regismap.insert(pair<string, int>("s1", 17));
	regismap.insert(pair<string, int>("s2", 18));
	regismap.insert(pair<string, int>("s3", 19));
	regismap.insert(pair<string, int>("s4", 20));
	regismap.insert(pair<string, int>("s5", 21));
	regismap.insert(pair<string, int>("s6", 22));
	regismap.insert(pair<string, int>("s7", 23));
	regismap.insert(pair<string, int>("t8", 24));
	regismap.insert(pair<string, int>("t9", 25));
	regismap.insert(pair<string, int>("k0", 26));
	regismap.insert(pair<string, int>("k1", 27));
	regismap.insert(pair<string, int>("gp", 28));
	regismap.insert(pair<string, int>("sp", 29));
	regismap.insert(pair<string, int>("fp", 30));
	regismap.insert(pair<string, int>("ra", 31));
}

struct instruct {
	int type;
	int num;
	//int(*f) (int a, int b);
	int Rdest;
	int Rsrc, Src2;
	string label;
	bool judge;
	instruct(int t = 0, int n = 0, int  x = 0, int y = 0, int z = 0, string p = "\0", bool j = false) :type(t), num(n), Rdest(x), Rsrc(y), Src2(z), label(p), judge(j) {
	}  //???????
};
vector<instruct> text;
map<string, int> stackmap;
map<string, int>instructmap;

//1 add Rdest, Rsrc1, Src2 Rdest = Rsrc1 + Src2
int math1(int Rsrc, int Src2) {
	return (Rsrc + Src2);
}
//2 addu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 + Src2
unsigned int math2(int Rsrc, int Src2) {
	return ((unsigned int)Rsrc + (unsigned int)Src2);
}
//3 addiu Rdest, Rsrc1, Imm(???)Rdest = Rsrc1 + Imm
unsigned int math3(int Rsrc, int Imm) {
	return ((unsigned int)Rsrc + Imm);
}
//4 sub Rdest, Rsrc1, Src2 Rdest = Rsrc1 - Src2
int math4(int Rsrc, int Src2) {
	return (Rsrc - Src2);
}
//5 subu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 - Src2
unsigned int math5(unsigned int Rsrc, unsigned int Src2) {
	return (Rsrc - Src2);
}
//6 mul Rdest, Rsrc1, Src2 Rdest = Rsrc1 * Src2
int math6(int Rsrc, int Src2) {
	return (Rsrc * Src2);
}
//7 mulu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 * Src2
unsigned int math7(unsigned int Rsrc, unsigned int Src2) {
	return (Rsrc*Src2);
}
//8 mul Rdest, Src2 ??, ? 32 ??? lo, ? 32 ??? hi
void math8(int Rdest, int Src2, int &part1, int &part2) {
	long long tmp = Rdest*Src2;
	part1 = tmp / (pow(2, 32)); //hi
	part2 = tmp - part1*(pow(2, 32));//lo
									 //return 0;
}
//9 mulu Rdest, Src2(???) ??, ? 32 ??? lo, ? 32 ??? hi
void math9(int Rdest, int Src2, unsigned int &part1, unsigned int &part2) {
	long long tmp = (unsigned int)Rdest*(unsigned int)Src2;
	part1 = tmp / (pow(2, 32)); //hi
	part2 = tmp - part1*(pow(2, 32));//lo
									 //return 0;
}
//10 div Rdest, Rsrc1, Src2 Rdest = Rsrc1 / Src2
int math10(int Rsrc, int Src2) {
	return (Rsrc / Src2);
}
//11 divu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 / Src2
unsigned int math11(int Rsrc, int Src2) {
	return ((unsigned int)Rsrc / (unsigned int)Src2);
}
//12 div Rsrc1, Rsrc2 lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
void math12(int Rsrc, int Src2, int & part1, int & part2) {
	part2 = Rsrc / Src2;
	part1 = Rsrc%Src2;
	//return 0;
}
//13 divu Rsrc1, Rsrc2(???)lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
void math13(int Rsrc, int Src2, unsigned int & part1, unsigned int part2) {
	part2 = (unsigned int)Rsrc / (unsigned int)Src2;
	part1 = Rsrc%Src2;
	//return 0;
}
//14 xor Rdest, Rsrc1, Src2 Rdest = Rsrc1 Src2
int math14(int Rsrc, int Src2) {
	return (Rsrc^Src2);
}
//15 xoru Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 Src2
unsigned int math15(int Rsrc, int Src2) {
	return ((unsigned int)Rsrc ^ (unsigned int)Src2);
}
//16 neg Rdest, Rsrc Rdest = Rsrc ??
int math16(int Rsrc) {
	return (Rsrc*(-1));
}
//17 negu Rdest, Rsrc(???)Rdest = Rsrc ??
unsigned int math17(int Rsrc) {
	return (((unsigned int)Rsrc)*(-1));
}
//18 rem Rdest, Rsrc1, Src2 Rdest = Rsrc1 % Src2
int math18(int Rsrc, int Src2) {
	return (Rsrc%Src2);
}
//19 remu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 % Src2
unsigned int math19(int Rsrc, int Src2) {
	return ((unsigned int)Rsrc % (unsigned int)Src2);
}

//1 seq Rdest, Rsrc1, Src2 Rdest = Rsrc1 == Src2
//2 sge Rdest, Rsrc1, Src2 Rdest = Rsrc1 >= Src2
//3 sgt Rdest, Rsrc1, Src2 Rdest = Rsrc1 > Src2
//4 sle Rdest, Rsrc1, Src2 Rdest = Rsrc1 <= Src2
//5 slt Rdest, Rsrc1, Src2 Rdest = Rsrc1 < Src2
//6 sne Rdest, Rsrc1, Src2 Rdest = Rsrc1 != Src2
int compare(int num, int b, int c) {
	bool flag = false;
	switch (num) {
	case 1: {if (b == c) flag = true; break;}
	case 2: {if (b >= c) flag = true; break;}
	case 3: {if (b > c) flag = true; break;}
	case 4: {if (b <= c) flag = true; break;}
	case 5: {if (b < c) flag = true; break;}
	case 6: {if (b != c) flag = true; break;}
	}
	if (flag == true) return 1;
	else return 0;
}

int jump(int num, int Rsrc, int Src2) {
	switch (num) {
	case 1: {
		//1 b label goto label
		return 1;
	}
	case 2: {
		//2 beq Rsrc1, Src2, label if (Rsrc1 == Src2) goto label
		return (Rsrc == Src2);
	}
	case 3: {
		//3 bne Rsrc1, Src2, label if (Rsrc1 != Src2) goto label
		return (Rsrc != Src2);
	}
	case 4: {
		//4 bge Rsrc1, Src2, label if (Rsrc1 >= Src2) goto label
		return (Rsrc >= Src2);
	}
	case 5: {
		//5 ble Rsrc1, Src2, label if (Rsrc1 <= Src2) goto label
		return (Rsrc <= Src2);
	}
	case 6: {
		//6 bgt Rsrc1, Src2, label if (Rsrc1 > Src2) goto label
		return (Rsrc > Src2);
	}
	case 7: {
		//7 blt Rsrc1, Src2, label if (Rsrc1 < Src2) goto label
		return (Rsrc < Src2);
	}
	case 8: {
		//8 beqz Rsrc, label if (Rsrc1 == 0) goto label
		return (Rsrc == 0);
	}
	case 9: {
		//9 bnez Rsrc, label if (Rsrc1 != 0) goto label
		return (Rsrc != 0);
	}
	case 10: {
		//10 blez Rsrc, label if (Rsrc1 <= 0) goto label
		return (Rsrc <= 0);
	}
	case 11: {
		//11 bgez Rsrc, label if (Rsrc1 >= 0) goto label
		return (Rsrc >= 0);
	}
	case 12: {
		//12 bgtz Rsrc, label if (Rsrc1 > 0) goto label
		return (Rsrc > 0);
	}
	case 13: {
		//13 bltz Rsrc, label if (Rsrc1 < 0) goto label
		return (Rsrc < 0);
	}
	case 14: {
		//14 j label goto label
		return 1;
	}
	case 15: {
		//15 jr Rsrc goto ???? in Rsrc
		return 1;
	}
	case 16: {
		//16 jal label $31 = ????????, goto label
		return (current + 1);
	}
	case 17: {
		//17 jalr Rsrc $31 = ????????, goto ???? in Rsrc
		return (current + 1);
	}
	}
}

//1 la Rdest, address Rdest = address
//2 lb Rdest, address Rdest = data[address:address + 1]
//3 lh Rdest, address Rdest = data[address:address + 2]
//4 lw Rdest, address Rdest = data[address:address + 4]
int load(int Rsrc, int Imm) {   //notice: it is wrong!!!!
	return (Rsrc + Imm);    //Nothing left
}

//1 sb Rsrc, address data[address:address + 1] = Rsrc
//2 sh Rsrc, address data[address:address + 2] = Rsrc
//3 sw Rsrc, address data[address:address + 4] = Rsrc
int store(int Rsrc, int Imm) {   //Seems that there is nothing left to do
	return (Rsrc + Imm);
}

//1 move Rdest, Rsrc Rdest = Rsrc
//2 mfhi Rdest Rdest = hi
//3 mflo Rdest Rdest = lo
//void move(int a, int b) {
//}

//1 nop ????, ???????
//2 syscall ??????????????????
//void special() {}


char str[maxn];
int i = 0;
int readInt() {  // start with '-' or number, end with the word after the number
	while ((str[i]<'0' || str[i]>'9') && str[i] != '-')  i++;
	bool minus = false;
	int answer = 0;
	if (str[i] == '-') {
		minus = true;
		i++;
	}
	while (str[i] >= '0'&&str[i] <= '9') answer = answer * 10 + str[i++] - '0';
	//i++;
	if (minus == false) return answer;
	else return (answer*(-1));
}
int readRegis(bool & x) {   //start with '$', end with the word after the register
							///cout << "\n                                    read reg: " << str << endl;
	int answer;
	char registmp[2];
	//string match;
	while (str[i] != '$' && str[i] != '-' && (str[i]<'0' || str[i]>'9')) {
		i++;
	}
	if (str[i] == '$') {
		i++;
		if (str[i] >= '0'&&str[i] <= '9') answer = readInt();
		else {
			registmp[0] = str[i++];
			registmp[1] = str[i++];
			if (registmp[0] == 'z'&&registmp[1] == 'e') {
				answer = 0;i += 2;
			}
			else {
				string match(registmp, 2);
				answer = regismap[match];
			}
		}
		x = false;
		//cout << "\n                         " << answer << "!!!\n";
		return answer;
	}
	else {
		x = true;
		return (readInt());
	}
}
bool islabel() {
	//int t = strlen(str);
	//if (str[t - 1] != ':') return false;
	int flag = 1;

	while (str[i] != '\0'&&str[i] != '\n') {
		if (str[i] == '"') {
			if (flag == 1) flag = 0;
			else if (flag == 0) flag = 2;
		}
		if (str[i] == ':' && flag == 1) {
			char strtmp[maxn];
			int h = 0, i = 0;
			while (str[i] == ' ' || str[i] == '\t') i++;
			//strncpy_s(strtmp, str, t - 1);
			if (flag == 1) while (str[i] != ':'&&str[i] != '\n'&&str[i] != '\0') strtmp[h++] = str[i++];
			//if (flag == 2) while (str[i] != '\n'&&str[i] != '\0') strtmp[h++] = str[i++];
			string a(strtmp, h);
			if (status == 1) {
				stackmap.insert(pair<string, int>(a, stacktop));
			}
			else {
				instructmap.insert(pair<string, int>(a, current));
			}
			return true;
		}
		i++;
	}
	return false;
}
string getlabel(int x) {  //x=1,instruct label; x=0,stack label;
	while (str[i] != ' '&&str[i] != ',') {
		i++;    //first find the ' ' before the label, then read the label
	}
	while (str[i] == ' ' || str[i] == ',') {
		i++;
	}
	int length = strlen(str);
	int q = i, u = 0;
	char strtmp[maxn];
	for (;q <= length&&str[i] != ' '&&str[i] != '\0'&&str[i] != '\n';q++) {
		strtmp[u] = str[i++];
		u++;
	}
	string a(strtmp, u);
	return a;
}
int tmp[3];
//instruct * v;
bool publicUse;
void getAddress(int x, int y, int numregis) {   //address is stored in label
												//cout << endl << str << "      ";
	tmp[numregis - 1] = readRegis(publicUse);   //If address is an int, then it is stored in Src2
												//cout << " the first register " << tmp[numregis - 1] << endl;
	string addString;
	int addInt;
	bool isInt = false;
	int a, b;
	while (str[i] == 32 || str[i] == ',') { i++; }
	//cout << "**********************************   " << str[i] << " " << int(str[i]) << endl;
	if ((str[i] >= '0'&&str[i] <= '9') || (str[i] == '-')) {
		//int a, b;
		//cout << "====================== readInt()    " << str + i << endl;
		a = readInt();
		//cout << "====================== readReg()    " << str + i << endl;
		b = readRegis(publicUse);  //the register is stored in Src2
								   //addInt = regis[b] + a; //if the register being used is Rdest,then offset is stored in Rsrc
		isInt = true;          //if the register being used is Rsrc,then offset is stored in Rdest
	}
	else {
		i--;
		addString = getlabel(1);
	}
	if (numregis == 1) {
		instruct v;
		if (isInt == false) v = instruct(x, y, tmp[0], -1, -1, addString, false);
		else v = instruct(x, y, tmp[0], a, b, "\0", true);
		text.push_back(v);
		current++;
	}
	else {
		instruct v;
		if (isInt == false) v = instruct(x, y, -1, tmp[1], -1, addString, false);
		else v = instruct(x, y, a, tmp[1], b, "\0", true);
		//cout << b << endl;
		text.push_back(v);
		current++;
	}
}
//register, number B,label C,address D;
int number;
string jumplabel;
int y;
void A1A2A3(int x, int y) {
	for (int circle = 0;circle <= 2;++circle) tmp[circle] = readRegis(publicUse);
	instruct v(x, y, tmp[0], tmp[1], tmp[2], "\0", publicUse);
	text.push_back(v);
	current++;
}
void A1A3(int x, int y) {
	tmp[0] = readRegis(publicUse);
	tmp[2] = readRegis(publicUse);
	instruct v(x, y, tmp[0], -1, tmp[2], "\0", publicUse);
	text.push_back(v);
	current++;
}
void A1A2(int x, int y) {
	tmp[0] = readRegis(publicUse);
	tmp[1] = readRegis(publicUse);
	instruct v(x, y, tmp[0], tmp[1], -1, "\0", publicUse);
	text.push_back(v);
	current++;
}
void A2A3(int x, int y) {
	tmp[1] = readRegis(publicUse);
	tmp[2] = readRegis(publicUse);
	instruct v(x, y, -1, tmp[1], tmp[2], "\0", publicUse);
	text.push_back(v);
	current++;
}
void A1A2B(int x, int y) {
	tmp[0] = readRegis(publicUse);
	tmp[1] = readRegis(publicUse);
	number = readInt();
	instruct v(x, y, tmp[0], tmp[1], number, "\0", true);
	text.push_back(v);
	current++;
}
void A1B(int x, int y) {
	tmp[0] = readRegis(publicUse);
	number = readInt();
	instruct v(x, y, tmp[0], -1, number, "\0", true);
	text.push_back(v);
	current++;
}
void A1(int x, int y) {
	tmp[0] = readRegis(publicUse);
	instruct v(x, y, tmp[0], -1, -1, "\0", publicUse);
	text.push_back(v);
	current++;
}
void A2(int x, int y) {
	tmp[1] = readRegis(publicUse);
	instruct v(x, y, -1, tmp[1], -1, "\0", publicUse);
	text.push_back(v);
	current++;
}
void C(int x, int y, int z) {
	jumplabel = getlabel(z);
	instruct v(x, y, -1, -1, -1, jumplabel, publicUse);
	text.push_back(v);
	current++;
}

void A2C(int x, int y, int z) {
	tmp[1] = readRegis(publicUse);
	jumplabel = getlabel(z);
	instruct v(x, y, -1, tmp[1], -1, jumplabel, publicUse);
	text.push_back(v);
	current++;
}
void A2A3C(int x, int y, int z) {
	tmp[1] = readRegis(publicUse);
	tmp[2] = readRegis(publicUse);
	jumplabel = getlabel(z);
	instruct v(x, y, -1, tmp[1], tmp[2], jumplabel, publicUse);
	text.push_back(v);
	current++;
}
void A1D(int x, int y) {
	getAddress(x, y, 1);
}
void A2D(int x, int y) {
	getAddress(x, y, 2);
}
void SYS(int x, int y) {
	instruct v(x, y, -1, -1, -1, "\0", publicUse);
	text.push_back(v);
	current++;
}
void NOP(int x, int y) {
	instruct v(x, y, -1, -1, -1, "\0", publicUse);
	text.push_back(v);
	current++;
}
void lookat(int x, int y)
{
	//switch (x) {
	//case 1: {
	//	switch (y) {
	//	case 1: cout << "add" << endl;break;
	//	case 2:cout << "addu" << endl;break;
	//	case 3:cout << "addiu" << endl;break;
	//	case 4:cout << "sub" << endl;break;
	//	case 5:cout << "subu" << endl;break;
	//	case 6:cout << "mul 3" << endl;break;
	//	case 7:cout << "mulu 3" << endl;break;
	//	case 8:cout << "mul 2" << endl;break;
	//	case 9:cout << "mulu 2" << endl;break;
	//	case 10:cout << "div" << endl;break;
	//	case 11:cout << "divu" << endl;break;
	//	case 12:cout << "div" << endl;break;
	//	case 13:cout << "divu" << endl;break;
	//	case 14:cout << "xor" << endl;break;
	//	case 15:cout << "xoru" << endl;break;
	//	case 16:cout << "neg" << endl;break;
	//	case 17:cout << "negu" << endl;break;
	//	case 18:cout << "rem" << endl;break;
	//	case 19:cout << "remu" << endl;break;
	//	}
	//	break;
	//}
	//case 2:cout << "li" << endl;break;
	//case 3: {
	//	switch (y) {
	//	case 1: cout << "seq" << endl;break;
	//	case 2:cout << "sge" << endl;break;
	//	case 3:cout << "sgt" << endl;break;
	//	case 4:cout << "sle" << endl;break;
	//	case 5:cout << "slt" << endl;break;
	//	case 6:cout << "sne" << endl;break;
	//	}
	//	break;
	//}
	//case 4: {
	//	switch (y) {
	//	case 1: cout << "b" << endl;break;
	//	case 2:cout << "beq" << endl;break;
	//	case 3:cout << "bne" << endl;break;
	//	case 4:cout << "bge" << endl;break;
	//	case 5:cout << "ble" << endl;break;
	//	case 6:cout << "bgt" << endl;break;
	//	case 7:cout << "blt" << endl;break;
	//	case 8:cout << "beqz" << endl;break;
	//	case 9:cout << "bnez" << endl;break;
	//	case 10:cout << "blez" << endl;break;
	//	case 11:cout << "bgez" << endl;break;
	//	case 12:cout << "bgtz" << endl;break;
	//	case 13:cout << "bltz" << endl;break;
	//	case 14:cout << "j" << endl;break;
	//	case 15:cout << "jr" << endl;break;
	//	case 16:cout << "jal" << endl;break;
	//	case 17:cout << "jalr" << endl;break;
	//	}
	//	break;
	//}
	//case 5: {
	//	switch (y) {
	//	case 1: cout << "la" << endl;break;
	//	case 2:cout << "lb" << endl;break;
	//	case 3:cout << "lh" << endl;break;
	//	case 4:cout << "lw" << endl;break;
	//	}
	//	break;
	//}
	//case 6: {
	//	switch (y) {
	//	case 1: cout << "sb" << endl;break;
	//	case 2:cout << "sh" << endl;break;
	//	case 3:cout << "sw" << endl;break;
	//	}
	//	break;
	//}
	//case 7: {
	//	switch (y) {
	//	case 1: cout << "move" << endl;break;
	//	case 2:cout << "mfhi" << endl;break;
	//	case 3:cout << "mflo" << endl;break;
	//	}
	//	break;
	//}
	//case 8: {
	//	switch (y) {
	//	case 1: cout << "nop" << endl;break;
	//	case 2:cout << "syscall" << endl;break;
	//	}
	//	break;
	//}
	//}
}
int main(int argc, char *argv[]) {
	//int main() {
	src.open(argv[1]);
	origin();
	char ch1, ch2, ch3, ch4;

	//int point;
	//char labelchar[maxn];
	//int numans;

	//src.open("bulgarian-5110379024-wuhang.s");
	//input.open("bulgarian-5110379024-wuhang.in");

	while (src.getline(str, 100, '\n')) {
		/*if (text.size() == 270)
		cout << "!";*/
		i = 0;
		if (islabel() == true)  continue;
		i = 0;
		int n;
		while (str[i] == ' ' || str[i] == '\t')  i++;
		ch1 = str[i];
		switch (ch1) {

			//begin with '.'
		case '.': {
			//of << "stacktop  " << stacktop << endl;
			//cout << "stacktop  " << stacktop << endl;
			for (int d = 0;d < stacktop;++d) {
				//of << stack[d] << "  ";
				//cout << stack[d] << "  ";
			}
			//of << endl;
			//cout << endl;
			ch2 = str[i + 1];
			switch (ch2) {
			case 'a': {
				if (str[i + 2] == 'l') { //.align n ?????????? 2^n byte ??.
					i += 6;
					while (str[i] == ' ' || str[i] == '\t') i++;
					n = str[i] - '0';
					int tmp1 = 1 << n;
					int tmp2 = ceil((double)stacktop / tmp1);
					stacktop = tmp2 * (1 << n);
				}
				if (str[i + 2] == 's') {
					if (str[i + 6] == 'z') { //.asciiz str ??? string ????, ??????.
						while (str[i] != '"') i++;
						i++;
						while (str[i] != '"') {
							if (str[i] == '\\') {
								switch (str[i + 1]) {
								case 't': { stack[stacktop++] = '\t';i += 2;break; }
								case '0': { stack[stacktop++] = '\0';i += 2;break; }
								case 'n': { stack[stacktop++] = '\n';i += 2;break; }
								case '\\': { stack[stacktop++] = '\\';i += 2;break; }
								case  'r': { stack[stacktop++] = '\r';i += 2;break; }
								case '\'': { stack[stacktop++] = '\'';i += 2;break;}
								case '\"': { stack[stacktop++] = '\"';i += 2;break;}
								case '\?': { stack[stacktop++] = '\?';i += 2;break;}
										   //default:cout << "BE CAREFUL !!!!!!!!!!!!!";
								}
							}
							else stack[stacktop++] = str[i++];
						}
						stack[stacktop++] = '\0';
					}
					else {
						while (str[i] != '"') i++;
						i++;
						while (str[i] != '"') {
							if (str[i] == '\\') {
								switch (str[i + 1]) {
								case 't': { stack[stacktop++] = '\t';i += 2;break; }
								case '0': { stack[stacktop++] = '\0';i += 2;break; }
								case 'n': { stack[stacktop++] = '\n';i += 2;break; }
								case '\\': { stack[stacktop++] = '\\';i += 2;break; }
								case  'r': { stack[stacktop++] = '\r';i += 2;break; }
								case '\'': { stack[stacktop++] = '\'';i += 2;break;}
								case '\"': { stack[stacktop++] = '\"';i += 2;break;}
								case '\?': { stack[stacktop++] = '\?';i += 2;break;}
								}
							}
							else stack[stacktop++] = str[i++];
						}
					}
				}
				break;
			}
			case 'b': {  //.byte b1, ..., bn ? n ? byte ?????????
						 //char read;
				i += 5;
				while (str[i] != '\n') {
					while (str[i] == ' ' || str[i] == '\t' || str[i] == ',') i++;
					int readword = readInt();
					stack[stacktop++] = (char)readword;
				}
				break;
			}
			case 'd': {  //.data ???????
				status = 1;
				break;
			}
			case 'h': {   //.half h1, ..., hn ? n ? halfWord ?????????.
				i += 5;
				while (str[i] != '\n') {
					while (str[i] == ' ' || str[i] == '\t' || str[i] == ',') i++;
					int readword = readInt();
					stack[stacktop++] = readword;
					stack[stacktop++] = readword >> 8;
				}
				break;
			}
			case 'w': {  // .word w1, .. ., wn ? n ? Word ?????????.
				i += 5;
				while (str[i] != '\n'&&str[i] != '\0') {
					while (str[i] == ' ' || str[i] == '\t') {
						i++;
					}
					//for (int q = 1;q <= 4;++q) stack[stacktop++] = str[i++];
					int readword = readInt();
					stack[stacktop++] = readword;
					stack[stacktop++] = readword >> 8;
					stack[stacktop++] = readword >> 16;
					stack[stacktop++] = readword >> 24;
				}
				break;
			}
			case 's': { //.space n ?? n ? byte ????
				i += 6;
				while (str[i] == ' ' || str[i] == '\t') i++;
				stacktop += (str[i] - '0');
				break;
			}
			case 't': {  //.text ???????
				status = 0;
				break;
			}
			}
			break;
		}
				  //??????? 1; ?????? 2; ???? 3; ??????? 4; Load ?? 5; Store ?? 6; ?????? 7;???? 8 
				  //register A, number B,label C,address D;
				  //1           //1  1 add Rdest, Rsrc1, Src2 Rdest = Rsrc1 + Src2
				  //1  2 addu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 + Src2
				  //1  3 addiu Rdest, Rsrc1, Imm(???)Rdest = Rsrc1 + Imm
				  //1  4 sub Rdest, Rsrc1, Src2 Rdest = Rsrc1 - Src2
				  //1  5 subu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 - Src2
				  //1  6 mul Rdest, Rsrc1, Src2 Rdest = Rsrc1 * Src2
				  //1  7 mulu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 * Src2
				  //1  8 mul Rdest, Src2 ??, ? 32 ??? lo, ? 32 ??? hi
				  //1  9 mulu Rdest, Src2(???) ??, ? 32 ??? lo, ? 32 ??? hi
				  //1  10 div Rdest, Rsrc1, Src2 Rdest = Rsrc1 / Src2
				  //1  11 divu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 / Src2
				  //1  12 div Rsrc1, Rsrc2 lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
				  //1  13 divu Rsrc1, Rsrc2(???)lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
				  //1  14 xor Rdest, Rsrc1, Src2 Rdest = Rsrc1 Src2
				  //1  15 xoru Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 Src2
				  //1  16 neg Rdest, Rsrc Rdest = Rsrc ??
				  //1  17 negu Rdest, Rsrc(???)Rdest = Rsrc ??
				  //1  18 rem Rdest, Rsrc1, Src2 Rdest = Rsrc1 % Src2
				  //1  19 remu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 % Src2
				  //2		       //2  1 li Rdest, imm Rdest = imm
				  //3		      //3  1 seq Rdest, Rsrc1, Src2 Rdest = Rsrc1 == Src2
				  //3  2 sge Rdest, Rsrc1, Src2 Rdest = Rsrc1 >= Src2
				  //3  3 sgt Rdest, Rsrc1, Src2 Rdest = Rsrc1 > Src2
				  //3  4 sle Rdest, Rsrc1, Src2 Rdest = Rsrc1 <= Src2
				  //3  5 slt Rdest, Rsrc1, Src2 Rdest = Rsrc1 < Src2
				  //3  6 sne Rdest, Rsrc1, Src2 Rdest = Rsrc1 != Src2
				  //4		      //4  1 b label goto label
				  //4  2 beq Rsrc1, Src2, label if (Rsrc1 == Src2) goto label
				  //4  3 bne Rsrc1, Src2, label if (Rsrc1 != Src2) goto label
				  //4  4 bge Rsrc1, Src2, label if (Rsrc1 >= Src2) goto label
				  //4  5 ble Rsrc1, Src2, label if (Rsrc1 <= Src2) goto label
				  //4  6 bgt Rsrc1, Src2, label if (Rsrc1 > Src2) goto label
				  //4  7 blt Rsrc1, Src2, label if (Rsrc1 < Src2) goto label
				  //4  8 beqz Rsrc, label if (Rsrc1 == 0) goto label
				  //4  9 bnez Rsrc, label if (Rsrc1 != 0) goto label
				  //4  10 blez Rsrc, label if (Rsrc1 <= 0) goto label
				  //4  11 bgez Rsrc, label if (Rsrc1 >= 0) goto label
				  //4  12 bgtz Rsrc, label if (Rsrc1 > 0) goto label
				  //4  13 bltz Rsrc, label if (Rsrc1 < 0) goto label
				  //4  14 j label goto label
				  //4  15 jr Rsrc goto ???? in Rsrc
				  //4  16 jal label $31 = ????????, goto label
				  //4  17 jalr Rsrc $31 = ????????, goto ???? in Rsrc
				  //5		      //5  1 la Rdest, address Rdest = address
				  //5  2 lb Rdest, address Rdest = data[address:address + 1]
				  //5  3 lh Rdest, address Rdest = data[address:address + 2]
				  //5  4 lw Rdest, address Rdest = data[address:address + 4]
				  //6		      //6  1 sb Rsrc, address data[address:address + 1] = Rsrc
				  //6  2 sh Rsrc, address data[address:address + 2] = Rsrc
				  //6  3 sw Rsrc, address data[address:address + 4] = Rsrc
				  //7		      //7  1 move Rdest, Rsrc Rdest = Rsrc
				  //7  2 mfhi Rdest Rdest = hi
				  //7  3 mflo Rdest Rdest = lo
				  //8		      //8  1 nop ????, ???????
				  //8  2 syscall ??????????????????


				  //begin with a
		case 'a': {
			// add Rdest, Rsrc1, Src2    Rdest = Rsrc1 + Src2
			// addu Rdest, Rsrc1, Src2   (???)Rdest = Rsrc1 + Src2
			// addiu Rdest, Rsrc1, Imm   (???)Rdest = Rsrc1 + Imm
			//add $13, $12, $9
			//add $8, $8, 1
			switch (str[i + 3]) {
			case ' ': A1A2A3(1, 1);break;
			case 'u':  A1A2A3(1, 2);break;
			case 'i': {
				//cout << "ch1  " << ch1 << "    str[i]  " << str[i] << "str[i+3]" << str[i + 3] << endl;
				A1A2B(1, 3);//cout << str << endl;
				break;
			}
			}
			break;
		}
				  //begin with b
		case 'b': {
			//4  1 b label goto label

			//4  2 beq Rsrc1, Src2, label if (Rsrc1 == Src2) goto label
			//4  8 beqz Rsrc, label if (Rsrc1 == 0) goto label

			//4  3 bne Rsrc1, Src2, label if (Rsrc1 != Src2) goto label
			//4  9 bnez Rsrc, label if (Rsrc1 != 0) goto label

			//4  4 bge Rsrc1, Src2, label if (Rsrc1 >= Src2) goto label
			//4  6 bgt Rsrc1, Src2, label if (Rsrc1 > Src2) goto label
			//4  11 bgez Rsrc, label if (Rsrc1 >= 0) goto label
			//4  12 bgtz Rsrc, label if (Rsrc1 > 0) goto label

			//4  5 ble Rsrc1, Src2, label if (Rsrc1 <= Src2) goto label
			//4  7 blt Rsrc1, Src2, label if (Rsrc1 < Src2) goto label
			//4  10 blez Rsrc, label if (Rsrc1 <= 0) goto label
			//4  13 bltz Rsrc, label if (Rsrc1 < 0) goto label
			if (str[i + 1] == ' ') { C(4, 1, 0);break; }
			if (str[i + 1] == 'e'&&str[i + 3] == ' ') { A2A3C(4, 2, 0); break; }
			if (str[i + 1] == 'e'&&str[i + 3] == 'z') { A2C(4, 8, 0); break; }
			if (str[i + 1] == 'n'&&str[i + 3] == ' ') { A2A3C(4, 3, 0); break; }
			if (str[i + 1] == 'n'&&str[i + 3] == 'z') { A2C(4, 9, 0);break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 'e'&&str[i + 3] == ' ') { A2A3C(4, 4, 0); break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 't'&&str[i + 3] == ' ') { A2A3C(4, 6, 0); break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 'e'&&str[i + 3] == 'z') { A2C(4, 11, 0); break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 't'&&str[i + 3] == 'z') { A2C(4, 12, 0);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 'e'&&str[i + 3] == ' ') { A2A3C(4, 5, 0);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 't'&&str[i + 3] == ' ') { A2A3C(4, 7, 0);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 'e'&&str[i + 3] == 'z') { A2C(4, 10, 0);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 't'&&str[i + 3] == 'z') { A2C(4, 13, 0);break; }
		}
				  //begin with d
		case 'd': {
			//1  10 div Rdest, Rsrc1, Src2 Rdest = Rsrc1 / Src2
			//1  11 divu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 / Src2
			//1  12 div Rsrc1, Rsrc2 lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
			//1  13 divu Rsrc1, Rsrc2(???)lo = Rsrc1 / Src2, hi = Rsrc1 % Src2
			int h = strlen(str), variable = 1;
			for (int t = 0;t < h;++t) {
				if (str[t] == ',') variable++;
			}
			if (variable == 3) {
				if (str[i + 3] == 'u') { A1A2A3(1, 11);break; }
				else { A1A2A3(1, 10);break; }
			}
			else {
				if (str[i + 3] == 'u') { A2A3(1, 13);break; }
				else { A2A3(1, 12);break; }
			}
		}
				  //begin with j
		case 'j': {
			//4  14 j label goto label
			//4  15 jr Rsrc goto ???? in Rsrc
			//4  16 jal label $31 = ????????, goto label
			//4  17 jalr Rsrc $31 = ????????, goto ???? in Rsrc
			if (str[i + 1] == ' ') { C(4, 14, 0);break; }
			if (str[i + 1] == 'r') { A2(4, 15);break; }
			if (str[i + 1] == 'a'&&str[i + 3] == ' ') { C(4, 16, 0);break; }
			if (str[i + 1] == 'a'&&str[i + 3] == 'r') { A2(4, 17);break; }
		}
				  //begin with l
		case 'l': {
			//5  1 la Rdest, address Rdest = address
			//5  2 lb Rdest, address Rdest = data[address:address + 1]
			//5  3 lh Rdest, address Rdest = data[address:address + 2]
			//5  4 lw Rdest, address Rdest = data[address:address + 4]
			//2  1 li Rdest, imm Rdest = imm
			// la $8, _static_0
			//lw $10, _static_1
			//lw $11, -4($fp)
			if (str[i + 1] == 'a') { A1D(5, 1); break; }
			if (str[i + 1] == 'b') { A1D(5, 2); break; }
			if (str[i + 1] == 'h') { A1D(5, 3);break; }
			if (str[i + 1] == 'w') { A1D(5, 4);break; }
			if (str[i + 1] == 'i') { A1B(2, 1);break; }
		}
				  //begin with m
		case 'm': {
			//1  6 mul Rdest, Rsrc1, Src2 Rdest = Rsrc1 * Src2
			//1  7 mulu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 * Src2
			//1  8 mul Rdest, Src2 ??, ? 32 ??? lo, ? 32 ??? hi
			//1  9 mulu Rdest, Src2(???) ??, ? 32 ??? lo, ? 32 ??? hi
			//7  1 move Rdest, Rsrc Rdest = Rsrc
			//7  2 mfhi Rdest Rdest = hi
			//7  3 mflo Rdest Rdest = lo
			if (str[i + 1] == 'u') {
				int h = strlen(str), variable = 1;
				for (int t = 0;t < h;++t) { if (str[t] == ',') variable++; }
				if (variable == 3) {
					if (str[i + 3] == 'u') A1A2A3(1, 7);
					else A1A2A3(1, 6);
				}
				if (variable == 2) {
					if (str[i + 3] == 'u') A1A2A3(1, 9);
					else A1A2A3(1, 8);
				}
				break;
			}
			if (str[i + 1] == 'o') { A1A2(7, 1);break; }
			if (str[i + 1] == 'f'&&str[i + 2] == 'h') { A1(7, 2);break; }
			if (str[i + 1] == 'f'&&str[i + 2] == 'l') { A1(7, 3);break; }
		}
				  //begin with n
		case 'n': {
			//1  16 neg Rdest, Rsrc Rdest = Rsrc ??
			//1  17 negu Rdest, Rsrc(???)Rdest = Rsrc ??
			//8  1 nop ????, ???????
			if (str[i + 1] == 'e'&&str[i + 3] == ' ') { A1A2(1, 16);break; }
			if (str[i + 1] == 'e'&&str[i + 3] == 'u') { A1A2(1, 17);break; }
			if (str[i + 1] == 'o') { NOP(8, 1);break; }
		}
				  //begin with r
		case 'r': {
			//1  18 rem Rdest, Rsrc1, Src2 Rdest = Rsrc1 % Src2
			//1  19 remu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 % Src2
			if (str[i + 3] == ' ') { A1A2A3(1, 18);break; }
			if (str[i + 3] == 'u') { A1A2A3(1, 19);break; }
		}
				  //begin with s
		case 's': {
			//1  4 sub Rdest, Rsrc1, Src2 Rdest = Rsrc1 - Src2
			//1  5 subu Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 - Src2
			//3  1 seq Rdest, Rsrc1, Src2 Rdest = Rsrc1 == Src2
			//3  2 sge Rdest, Rsrc1, Src2 Rdest = Rsrc1 >= Src2
			//3  3 sgt Rdest, Rsrc1, Src2 Rdest = Rsrc1 > Src2
			//3  4 sle Rdest, Rsrc1, Src2 Rdest = Rsrc1 <= Src2
			//3  5 slt Rdest, Rsrc1, Src2 Rdest = Rsrc1 < Src2
			//3  6 sne Rdest, Rsrc1, Src2 Rdest = Rsrc1 != Src2
			//6  1 sb Rsrc, address data[address:address + 1] = Rsrc
			//6  2 sh Rsrc, address data[address:address + 2] = Rsrc
			//6  3 sw Rsrc, address data[address:address + 4] = Rsrc
			//8  2 syscall ??????????????????
			if (str[i + 1] == 'u'&&str[i + 3] == ' ') { A1A2A3(1, 4); break; }
			if (str[i + 1] == 'u'&&str[i + 3] == 'u') { A1A2A3(1, 5);break; }
			if (str[i + 1] == 'e') { A1A2A3(3, 1);break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 'e') { A1A2A3(3, 2);break; }
			if (str[i + 1] == 'g'&&str[i + 2] == 't') { A1A2A3(3, 3);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 'e') { A1A2A3(3, 4);break; }
			if (str[i + 1] == 'l'&&str[i + 2] == 't') { A1A2A3(3, 5);break; }
			if (str[i + 1] == 'n') { A1A2A3(3, 6);break; }
			if (str[i + 1] == 'b') { A2D(6, 1);break; }
			if (str[i + 1] == 'h') { A2D(6, 2);break; }
			if (str[i + 1] == 'w') { A2D(6, 3);break; }
			if (str[i + 1] == 'y') { SYS(8, 2);break; }
		}
				  //begin with x
		case 'x': {
			//1  14 xor Rdest, Rsrc1, Src2 Rdest = Rsrc1 Src2
			//1  15 xoru Rdest, Rsrc1, Src2(???)Rdest = Rsrc1 Src2
			if (str[i + 3] == ' ') { A1A2A3(1, 14);break; }
			if (str[i + 3] == 'u') { A1A2A3(1, 15);break; }
		}
		}
	}
	current = 0;
	//for (auto it = stackmap.begin(); it != stackmap.end(); ++it)
	//{
	//	int x = it->second;
	//   cout << it->first << "=" << it->second <<endl;
	//	cout << stack[it->second] << endl;
	//}
	for (auto it = instructmap.begin(); it != instructmap.end(); ++it)
	{
		int x = it->second;
		//cout << it->first << "=" << it->second << endl;
		for (i = 0;i < 5;++i) { lookat(text[x + i].type, text[x + i].num); }

	}
	/*cout << instructmap["_buffer_init"] << endl;
	cout << instructmap["_func_____built_in_string_less"] << endl;
	cout << instructmap["_end_if_0"] << endl;
	cout << instructmap["_begin_loop_0"] << endl;
	cout << instructmap["_end_if_1"] << endl;
	cout << instructmap["_continue_loop0"] << endl;
	cout << instructmap["_end_loop_0"] << endl;
	cout << instructmap["_end_func_____built_in_string_less"] << endl;
	cout << instructmap["_func_____built_in_string_equal"] << endl;*/
	//string x = "_func_main";
	//cout << instructmap[x] << endl;







	int clock = 0;

	//used in Step 1
	bool canFetch = true;

	//deliver to Step 2
	instruct tmp;
	bool canDecode = false;

	//deliver to Step 3
	int f1 = -1, f2, f3, x1, x2 = -1, x3 = -1, RdestLoca = -1, Rsrc, Src2;
	string x4;
	bool caculate = false;

	//deliver to Step 4
	//in
	int x4ad = -1, x2Stack = -1, f1Step4 = -1, f2Step4 = -1, v0Value;
	//out
	int ans1 = -1, former1 = -1, latter1 = -1, x3Step4 = -1, flag = -1, jumpto = -1, RdestLocaStep4 = -1, pos31 = -1;
	unsigned int ans2 = -1, former2 = -1, latter2 = -1;
	bool whether = 0;
	bool beginStep4 = false;

	//deliver to Step 5
	int ans1Write, former1Write, latter1Write, x3Write, flagWrite, transchar, jumptoStep5, RdestLocaStep5, x2Write, pos31Step5, v0ValueStep5;
	int f1Step5 = -1, f2Step5 = -1;
	unsigned int ans2Write, former2Write, latter2Write;
	bool whetherWrite = 0;
	bool beginStep5 = false;
	current = instructmap["main"];
	current--;
	bool canChange[34];
	int size = text.size();
	for (int l = 0;l < 34;++l) canChange[l] = true;
	bool ifcurrent = true;
	int jumptimedata = -1, jumptimestruc = -1;
	while (current != text.size()) {
		clock++;
		//of << "one clock over------------------------------------------------------------------" << endl;
		//Write Back
		if (beginStep5) {
			//cout << "step 5 ------ f1  " << f1Step5 << ";  f2  " << f2Step5 << endl;
			//of << "step 5 ------ f1  " << f1Step5 << ";  f2  " << f2Step5 << endl;
			unsigned int trans;
			switch (f1Step5) {
			case 1: {
				switch (f2Step5) {
				case 1:case 4:case 6:case 10:case 14:case 16:case 18: {
					if (canChange[RdestLocaStep5] == false) {
						regis[RdestLocaStep5] = ans1Write;
						canChange[RdestLocaStep5] = true;
					}
					break;
				}
				case 2:case 3:case 5:case 7:case 11:case 15:case 17:case 19: {
					trans = (unsigned int)regis[RdestLocaStep5];
					if (canChange[RdestLocaStep5] == false) {
						regis[RdestLocaStep5] = ans2Write;
						canChange[RdestLocaStep5] = true;
					}
					break;
				}
				case 8:case 12: {
					if (canChange[32] == false && canChange[33] == false) {
						hi = former1Write;
						lo = latter1Write;
						canChange[32] = true;
						canChange[33] = true;
					}
					break;
				}
				case 9:case 13: {
					if (canChange[32] == false && canChange[33] == false) {
						trans = (unsigned int)hi;
						trans = (unsigned int)lo;
						hi = former2Write;
						lo = latter2Write;
						canChange[32] = true;
						canChange[33] = true;
					}
					break;
				}
				}
				break;
			}
			case 2: {
				if (canChange[RdestLocaStep5] == false) {
					regis[RdestLocaStep5] = x3Write;
					canChange[RdestLocaStep5] = true;
				}
				break;
			}
			case 3: {
				if (canChange[RdestLocaStep5] == false) {
					regis[RdestLocaStep5] = flagWrite;
					canChange[RdestLocaStep5] = true;
				}
				break;
			}
			case 4: {
				if (canChange[31] == false)
				{
					regis[31] = pos31Step5;
					canChange[31] = true;
				}
				break;
			}
			case 5: {
				if (canChange[RdestLocaStep5] == false) {
					regis[RdestLocaStep5] = transchar;
					canChange[RdestLocaStep5] = true;
				}
				break;
			}
			case 7: {
				if (f2Step5 == 1) {
					if (canChange[RdestLocaStep5] == false) {
						regis[RdestLocaStep5] = x2Write;
						canChange[RdestLocaStep5] = true;
					}
					break;
				}
				if (f2Step5 == 2) {
					if (canChange[RdestLocaStep5] == false) {
						regis[RdestLocaStep5] = hi;
						canChange[RdestLocaStep5] = true;
					}
					break;
				}
				if (f2Step5 == 3) {
					if (canChange[RdestLocaStep5] == false) {
						regis[RdestLocaStep5] = lo;
						canChange[RdestLocaStep5] = true;
					}
					break;
				}
			}
			case 8: {
				if (canChange[2] == false && (regis[2] == 5 || regis[2] == 9)) {
					regis[2] = v0ValueStep5;
					canChange[2] = true;
				}
			}
			}
			if (whetherWrite) { current = jumptoStep5;canFetch = true; whetherWrite = false;ifcurrent = false; }
			//else current++;
			beginStep5 = false;
		}

		//Memory Access        
		if (beginStep4) {
			int f = (1 << 8) - 1;
			//cout << "step 4 ------ f1  " << f1Step4 << ";  f2  " << f2Step4 << endl;
			//of << "step 4 ------ f1  " << f1Step4 << ";  f2  " << f2Step4 << endl;
			if (f1Step4 == 5) {
				int t1 = (int)stack[x4ad], t2 = (int)stack[x4ad + 1], t3 = (int)stack[x4ad + 2], t4 = (int)stack[x4ad + 3];
				if (f2Step4 == 2) { transchar = (t1&f); }
				if (f2Step4 == 3) { transchar = ((t2&f) << 8) | t1; }
				if (f2Step4 == 4) { transchar = ((t4&f) << 24) | ((t3&f) << 16) | ((t2&f) << 8) | (t1&f); }
				if (f2Step4 == 1) transchar = x4ad;
			}
			if (f1Step4 == 6) {
				char h1 = (char)(x2Stack&f), h2 = (char)((x2Stack >> 8)&f), h3 = (char)((x2Stack >> 16)&f), h4 = (char)((x2Stack >> 24)&f);
				if (f2Step4 == 1) { stack[x4ad] = h1; }
				if (f2Step4 == 2) { stack[x4ad] = h1;stack[x4ad + 1] = h2; }
				if (f2Step4 == 3) { stack[x4ad] = h1;stack[x4ad + 1] = h2;stack[x4ad + 2] = h3;stack[x4ad + 3] = h4; }

			}
			ans1Write = ans1;former1Write = former1;latter1Write = latter1;x3Write = x3Step4;flagWrite = flag;jumptoStep5 = jumpto;
			RdestLocaStep5 = RdestLocaStep4;ans2Write = ans2;former2Write = former2;latter2Write = latter2;whetherWrite = whether;
			x2Write = x2Stack;pos31Step5 = pos31;f1Step5 = f1Step4;f2Step5 = f2Step4;whether = false;
			beginStep5 = true;beginStep4 = false;v0ValueStep5 = v0Value;
		}

		//Execution
		if (caculate) {
			//cout << "step 3 ------ f1  " << f1 << ";  f2  " << f2 << endl;
			//of << "step 3 ------ f1  " << f1 << ";  f2  " << f2 << endl;
			switch (f1) {
			case 1: {
				switch (f2) {
				case 1:ans1 = math1(x2, x3);break;
				case 2:ans2 = math2(x2, x3);break;
				case 3:ans2 = math3(x2, x3);break;
				case 4:ans1 = math4(x2, x3);break;
				case 5:ans2 = math5(x2, x3);break;
				case 6:ans1 = math6(x2, x3);break;
				case 7:ans2 = math7(x2, x3);break;
				case 8: {
					math8(x1, x3, former1, latter1);
					canChange[32] = false;
					canChange[33] = false;
					break;
				}
				case 9: {
					math9(x1, x3, former2, latter2);
					canChange[32] = false;
					canChange[33] = false;
					break;
				}
				case 10:ans1 = math10(x2, x3);break;
				case 11:ans2 = math11(x2, x3);break;
				case 12: {
					math12(x2, x3, former1, latter1);
					canChange[32] = false;
					canChange[33] = false;
					break;
				}
				case 13: {
					math13(x2, x3, former2, latter2);
					canChange[32] = false;
					canChange[33] = false;
					break;
				}
				case 14:ans1 = math14(x2, x3);break;
				case 15:ans2 = math15(x2, x3);break;
				case 16:ans1 = math16(x2);break;
				case 17:ans2 = math17(x2);break;
				case 18:ans1 = math18(x2, x3);break;
				case 19:ans2 = math19(x2, x3);break;
				}
				if (f2 != 8 && f2 != 8 && f2 != 12 && f2 != 13)canChange[RdestLoca] = false;
				break;
			}
			case 2:canChange[RdestLoca] = false;
			case 3: {
				flag = compare(f2, x2, x3);
				canChange[RdestLoca] = false;
				break;
			}
			case 4: {
				if (f2 >= 1 && f2 <= 14) { whether = jump(f2, x2, x3); jumpto = instructmap[x4]; }
				else {
					whether = 1;
					if (f2 == 15) jumpto = x2;
					if (f2 == 16) {
						jumpto = instructmap[x4];
						pos31 = jump(f2, x2, x3);
						canChange[31] = false;
					}
					if (f2 == 17) {
						jumpto = x2;
						pos31 = jump(f2, x2, x3);
						canChange[31] = false;
					}
				}
				break;
			}
			case 5: {
				canChange[RdestLoca] = false;
				if (f3 == false) { x4ad = stackmap[x4]; }
				else { x4ad = Rsrc + regis[Src2]; }
				break;
			}
			case 6: {
				x2 = regis[Rsrc];
				if (f3 == false) { x4ad = stackmap[x4]; }
				else { x4ad = RdestLoca + regis[Src2]; }
				break;
			}
			case 7: {canChange[RdestLoca] = false;break;}
			case 8: {
				switch (regis[2]) {
				case 1: {cout << regis[4];break;}
				case 4: {
					int out = regis[4];
					while (stack[out] != '\0') {
						cout << stack[out]; ++out;
					}
					break;
				}
				case 5: {is >> v0Value;canChange[2] = false;break;}
				case 8: {
					int g = 0;
					char read[100];
					cin >> read;
					while (g < (regis[5] - 1) && g < strlen(read)) {
						stack[regis[4] + g] = read[g];
						g++;
					}
					break;
				}
				case 9: {
					v0Value = stacktop;
					canChange[2] = false;
					stacktop += regis[4];break;
				}
				case 10: {return 0;}
				case 17: {return regis[4];break;}
				}
				break;
			}
			}
			x2Stack = x2;x3Step4 = x3;RdestLocaStep4 = RdestLoca;f1Step4 = f1;f2Step4 = f2;
			caculate = false;beginStep4 = true;
		}
		if (jumptimedata > 0) {
			jumptimedata--;
			continue;
		}
		//Instruction Decode & Data Preparation
		if (canDecode) {

			f1 = tmp.type;
			f2 = tmp.num;
			f3 = tmp.judge;
			//if (f1 == 7 && f2 == 1) {
			//	cout << "stop here" << endl;
			//}
			if (tmp.Rdest >= 0 && tmp.Rdest < 32) x1 = regis[tmp.Rdest];
			x4 = tmp.label;
			RdestLoca = tmp.Rdest;
			Rsrc = tmp.Rsrc;
			Src2 = tmp.Src2;
			//cout << "step 2 ------ f1  " << f1 << ";  f2  " << f2 << endl;
			//of << "step 2 ------ f1  " << f1 << ";  f2  " << f2 << endl;
			if (f1 == 8 && f2 == 2) {
				if (canChange[4] == false || canChange[5] == false || canChange[2] == false) {
					//cout << "data hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
					//of << "data hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
					jumptimedata = 2;
					continue;
				}
			}
			if (RdestLoca >= 0 && RdestLoca < 32) {
				if (canChange[RdestLoca] == false) {
					///of << "data hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
					jumptimedata = 2;
					continue;
				}
			}
			if (Rsrc >= 0 && Rsrc < 32) {
				if (canChange[Rsrc] == false) {
					//of << "data hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
					jumptimedata = 2;
					continue;
				}
			}
			if (Src2 >= 0 && Src2 < 32) {
				if (canChange[Src2] == false) {
					//of << "data hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
					jumptimedata = 2;
					continue;
				}
			}
			if ((f1 == 1 && f2 == 8) || (f1 == 1 && f2 == 9)) {
				if (f3 == true) x2 = tmp.Rsrc;
				x3 = -1;
			}
			else {
				if (tmp.Rsrc >= 0 && tmp.Rsrc < 32) x2 = regis[tmp.Rsrc];
				if (f3 == true) x3 = tmp.Src2;
				else { if (tmp.Src2 >= 0 && tmp.Src2 < 32) x3 = regis[tmp.Src2]; }
			}
			canDecode = false;caculate = true;
			//if (RdestLoca == 2 && x2 == 0&&regis[2]== 4) {
			//	cout << "stop here" << endl;
			//}
		}
		//Instruction Fetch
		if (jumptimestruc > 0) {
			jumptimestruc--;
			continue;
		}
		//if (canFetch) {
		//cout << "clock------" << clock << "          current-------" << current << endl;
		if (ifcurrent) {
			current++;
			//of << " ifcurrent     " << ifcurrent << "      current    " << current << endl; 
		}
		else ifcurrent = true;
		tmp = text[current];
		//cout << "step 1 ------ f1  " << tmp.type << ";  f2  " << tmp.num << endl;
		//of << "step 1 ------ f1  " << tmp.type << ";  f2  " << tmp.num << endl;
		canDecode = true;
		if (tmp.type == 4) {
			//of << "construction hazard ------ f1  " << f1 << ";  f2  " << f2 << endl;
			jumptimestruc = 4;
		}
		//}
		//for (int w = 0;w < 32;++w) of << regis[w] << " ";
		//of << endl;
	}
	//system("pause");
	return 0;
}


