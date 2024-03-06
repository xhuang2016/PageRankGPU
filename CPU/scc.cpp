#include "CSR.cpp"
#include <stack>
#include <thread>
#include <unordered_set>
#include <iostream>

void thread_fun(std::string , edges& ,int ,std::vector<int>& ,std::vector<int>& );
void DFSUtil(int , bool [],std::vector<int>& ,std::vector<int>& ,std::stack<int>& );

int main(int argc, char const *argv[]) {
  edges e;
  edges r_e;
  std::vector<int> row_offset_1,row_offset_2;
  std::vector<int> column_index_1,column_index_2;
  std::thread thread_1(thread_fun,argv[1],std::ref(e),0,std::ref(row_offset_1),std::ref(column_index_1));
  std::thread thread_2(thread_fun,argv[1],std::ref(r_e),1,std::ref(row_offset_2),std::ref(column_index_2));
  thread_1.join();
  thread_2.join();
  // std::cout<<"finish\n";

  std::stack<int> s;
  int V = row_offset_1.size()-1;
  bool *visited = new bool[V];

  for (int i = 0; i < V; i++){
    visited[i] = false;
  }

  for(int i = 0; i < V; i++){
    DFSUtil(i,visited,row_offset_1,column_index_1,s);
  }


  // std::stack<int> s2;
  // int size = s.size();
  // for(int i = 0; i < size; i++)
  // {
  //   std::cout<<s.top()<<std::endl;
  //   s2.push(s.top());
  //   s.pop();
  // }
  //
  // s = s2;

  for (int i = 0; i < V; i++){
    visited[i] = false;
  }

  std::stack<int> scc;
  std::unordered_set<int> lscc;
  int largeSCC = 0;

  for(int i = 0; i < V; i++){
    if(!visited[s.top()]){
      // std::cout<<"round:"<<i<<std::endl;
      // std::cout<<s.top()<<std::endl;
      DFSUtil(s.top(),visited,row_offset_2,column_index_2,scc);
      if(scc.size() > largeSCC){
        lscc.clear();
        largeSCC = scc.size();
        for(int j = 0; j < largeSCC; j++){
          lscc.insert(scc.top());

          scc.pop();
        }
      }
    }
    s.pop();
  }

  // for(std::unordered_set<int>::iterator iter = lscc.begin();iter != lscc.end();iter++){
  //     std::cout<<*iter<<"\n";
  // }

  std::fstream file(argv[1],std::ios::in);
  std::string str = argv[1];
  str = str.substr(str.find("/")+1,str.find(".")-str.find("/")-1);
  std::fstream ofile("output/" + str + "_scc_edge.txt",std::ios::out);
  edge scc_edge;
  while(getline(file,str,'\n')){
    scc_edge = split(str,"\t");
    if((lscc.find(scc_edge.start) != lscc.end())&&(lscc.find(scc_edge.end)!=lscc.end())){
      ofile<<scc_edge.start<<"\t"<<scc_edge.end<<"\n";
    }
  }
  file.close();
  ofile.close();
  return 0;
}

void thread_fun(std::string s, edges& e,int flag,std::vector<int>& row_offset,std::vector<int>& column_index){
  graphInfo g = read(s,e,flag);
  CSR* ptr = new CSR(e,g);

  row_offset = ptr->row_offset;
  column_index = ptr->column_index;
  delete ptr;
}

void DFSUtil(int v, bool visited[],std::vector<int>& row_offset,std::vector<int>& column_index,std::stack<int>& s)
{
    if(!visited[v]){
      visited[v] = true;
      s.push(v);
      // std::cout<<v<<std::endl;
    }
    for (int i = row_offset[v]; i <row_offset[v+1]; ++i){
      if(!visited[column_index[i]]){
        DFSUtil(column_index[i], visited,row_offset,column_index,s);
      }
    }
}
