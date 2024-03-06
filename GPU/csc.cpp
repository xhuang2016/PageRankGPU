#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <time.h>

// #ifndef test
// #define test 1
// #endif

struct edge{
  int start;
  int end;
  double weight;
};

struct cmp{
  bool operator()(edge a, edge b){
    if(a.start != b.start){
      return a.start > b.start;
    }
    else{
      return a.end > b.end;
    }
  }
};

struct graphInfo{
  int max_node;
  int edge_Number;
};

typedef std::priority_queue<edge, std::vector<edge>, cmp> edges;

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

graphInfo read2(std::string filePath, edges& edges,int flags,std::vector<int>& col1,std::vector<int>& col2){
  graphInfo g;
  g.max_node = 0;
  g.edge_Number = 0;
  std::fstream file;
  file.open(filePath, std::ios::in);
  std::string str;
  // 0 for graph, 1 for transpose graph
  while(getline(file,str,'\n')){
    if(str.at(0) != '#'){
      edge e = split(str,"\t");
      col1.push_back(e.start);
      col2.push_back(e.end);
      if(flags){
        int tmp = e.start;
        e.start = e.end;
        e.end = tmp;
      }
      if(g.max_node < std::max(e.start,e.end)){
        g.max_node = std::max(e.start,e.end);
      }
      edges.push(e);
      g.edge_Number++;
    }
  }
  file.close();
  return g;
}

graphInfo read(std::string filePath, edges& edges,int flags){
  graphInfo g;
  g.max_node = 0;
  g.edge_Number = 0;
  std::fstream file;
  file.open(filePath, std::ios::in);
  std::string str;
  // 0 for graph, 1 for transpose graph
  while(getline(file,str,'\n')){
    if(str.at(0) != '#'){
      edge e = split(str,"\t");
      if(flags){
        int tmp = e.start;
        e.start = e.end;
        e.end = tmp;
      }
      if(g.max_node < std::max(e.start,e.end)){
        g.max_node = std::max(e.start,e.end);
      }
      edges.push(e);
      g.edge_Number++;
    }
  }
  file.close();
  return g;
}

class csc{
public:
  typedef int nodeIndex;
  std::vector<nodeIndex> row_offset;
  std::vector<nodeIndex> column_index;
  std::vector<double> weight_value;
  int max_node;
  int edge_Number;

  csc(edges& edges,graphInfo g):row_offset(g.max_node+2),column_index(),weight_value(){
    max_node = g.max_node+2;
    edge_Number = g.edge_Number;
    // std::cout<<g.max_node<<" "<<g.edge_Number<<std::endl;
    column_index.reserve(edge_Number);
    weight_value.reserve(edge_Number);
    for(int i = 0;i < max_node; i++)
    {
      row_offset[i] = 0;
    }
    int cur_node = 0;
    int prefix_sum = 0;
    while(!edges.empty()){
      if( edges.top().start != cur_node){
        for(int j = cur_node+1;j<=edges.top().start;j++){
          row_offset[j] = prefix_sum;
        }
        cur_node = edges.top().start;
      }
      column_index.push_back(edges.top().end);
      weight_value.push_back(edges.top().weight);
      prefix_sum ++;

      // std::cout<<edges.top().start<<" "<<edges.top().end<<std::endl;
      edges.pop();
    }
    for(int i = cur_node+1;i < max_node;i++){
      row_offset[i] = prefix_sum;
    }
  }
  ~csc(){};
};
