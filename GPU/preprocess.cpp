#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <unordered_map>
#include <iterator>
#include <utility>
#include <map>

class preprocess{
private:
  std::unordered_map<int,int> nodes_hash;
  std::set<int> nodes;
  std::fstream fout;
public:
  class edge{
  public:
    int start;
    int end;
    double weight;

    bool operator==(edge& e1){
      return (this->start==e1.start)&&(this->end==e1.end);
    }

    bool operator<(const edge& e1) const{
      return (e1.start==this->start)?(this->end<e1.end):(this->start<e1.start);
    }
  };
  std::set<edge> edges;

  edge split(const std::string& s, const std::string& delim){
    edge e;
    std::size_t pos = 0;
    std::size_t delim_len = delim.length();
    if(delim_len == 0) return e;
    std::size_t find_pos1 = s.find(delim, pos);
    e.start = std::stoi(s.substr(0,find_pos1));
    std::size_t find_pos2 = s.find(delim, find_pos2+delim_len);
    if(find_pos2 != std::string::npos){
      e.end = std::stoi(s.substr(find_pos1+delim_len,find_pos2-find_pos1));
      e.weight = std::stod(s.substr(find_pos2+delim_len));
    }
    else{
      e.end = std::stoi(s.substr(find_pos1+delim_len));
      e.weight = 1.0;
    }
    return e;
  }

  int creatNodeList(edge e){
    nodes.insert(e.start);
    nodes.insert(e.end);
    return 0;
  }

  int reorderNodes(){
    int count = 0;
    for(std::set<int>::iterator iter = nodes.begin();iter != nodes.end();iter++)
    {
      nodes_hash.insert(std::pair<int,int>(*iter,count));
      count++;
    }
    return 0;
  }

  int IO_reorder(std::string fileName, int (*ptr)(edge)){
    std::fstream fin(fileName,std::ios::in);
    std::string outPut = fileName.substr(0,fileName.find("."))+"_result.txt";
    fout.open(outPut,std::ios::out);
    std::string str;
    if(fin.is_open()&&fout.is_open()){

      while(getline(fin,str,'\n')){
        edge e = split(str,"\t");
        creatNodeList(e);
      }
      reorderNodes();

      fin.clear();
      fin.seekg(0,std::ios::beg);
      while(getline(fin,str,'\n')){
        edge e = split(str,"\t");
        ptr(e);
      }
      std::cout<<"total nodes:"<<nodes.size()<<std::endl;
      std::cout<<"total edges:"<<edges.size()<<std::endl;
      fin.close();
      fout.close();
      return 0;
    }
    return 1;
  }

  int eliminateNodes(edge e){
    fout<<nodes_hash[e.start]<<" "<<nodes_hash[e.end]<<"\n";
    return 0;
  }

  int eliminateRedundantEdge(edge e){
    if(edges.insert(e).second){
      fout<<e.start<<" "<<e.end<<"\n";
    }
    return 0;
  }

  int eliminateSelfLoop(edge e){
    if(e.start != e.end){
      fout<<e.start<<" "<<e.end<<"\n";
    }
    return 0;
  }

  int eliminate3to1(edge e){
    if((e.start != e.end)&&(edges.insert(e).second)){
      fout<<nodes_hash[e.start]<<"\t"<<nodes_hash[e.end]<<"\n";
    }
    return 0;
  }

  int eliminate(edge e){
    if((e.start != e.end)&&(edges.insert(e).second)){
      fout<<e.start<<"\t"<<e.end<<"\n";
    }
    return 0;

  }
  int IO_eliminate(std::string fileName, int (*ptr)(edge)){
    std::fstream fin(fileName,std::ios::in);
    std::string outPut = fileName.substr(0,fileName.find("."))+"_modified.txt";
    fout.open(outPut,std::ios::out);
    std::string str;
    if(fin.is_open()&&fout.is_open()){

      while(getline(fin,str,'\n')){
        if(str.compare(0,1,"#")){
        edge e = split(str,"\t");
        ptr(e);
        }
      }
      // std::cout<<"total nodes:"<<nodes.size()<<std::endl;
      std::cout<<"total edges:"<<edges.size()<<std::endl;
      fin.close();
      fout.close();
      return 0;
    }
    return 1;
  }
};

preprocess pre;
int wrapper1(preprocess::edge e){
  pre.eliminate(e);
  return 0;
}

preprocess pre2;

int wrapper2(preprocess::edge e){
  pre2.eliminate3to1(e);
  return 0;
}

// int main(int argc, char const *argv[]) {
//   if(argv[2] == "0"){
//     pre.IO(argv[1],wrapper);
//   }else{
//     pre.IO2(argv[1],wrapper);
//   }
//   return 0;
// }
