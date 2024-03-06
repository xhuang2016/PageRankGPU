#include "preprocess.cpp"
#include "CSR.cpp"
#include <stack>
#include <thread>
#include <unordered_set>

void thread_fun(std::string , edges& ,int ,std::vector<int>& ,std::vector<int>& );
void DFSUtil(int , bool [],std::vector<int>& ,std::vector<int>& ,std::stack<int>& );
void DFS(int v ,bool visited[], bool explored[], std::vector<int>& row_offset,std::vector<int>& column_index,std::stack<int>& s);

int main(int argc, char const *argv[]) {
  std::string original_file = argv[1];
  pre.IO_eliminate(argv[1],wrapper1);
  edges e;
  edges r_e;
  std::vector<int> row_offset_1,row_offset_2;
  std::vector<int> column_index_1,column_index_2;
  std::string modified_file = original_file.substr(0,original_file.find("."))+"_modified.txt";
  std::thread thread_1(thread_fun,modified_file,std::ref(e),0,std::ref(row_offset_1),std::ref(column_index_1));
  std::thread thread_2(thread_fun,modified_file,std::ref(r_e),1,std::ref(row_offset_2),std::ref(column_index_2));
  thread_1.join();
  thread_2.join();
  // std::cout<<"finish\n";

  std::stack<int> s;
  int V = row_offset_1.size()-1;
  bool *visited = new bool[V];
  bool *explored = new bool[V];

  for (int i = 0; i < V; i++){
    visited[i] = false;
  }

  for(int i = 0; i < V; i++){
    // printf("node: %d\n", i);
    DFS(i,visited,explored,row_offset_1,column_index_1,s);
  }

// std::cout<<"col1:"<<std::endl;
//   for(std::vector<int>::iterator iter=row_offset_1.begin();iter!=row_offset_1.end();iter++){
//     std::cout<<*iter<<std::endl;
//   }
//   std::cout<<"col:"<<std::endl;
//
//
//   for(std::vector<int>::iterator iter=column_index_1.begin();iter!=column_index_1.end();iter++){
//     std::cout<<*iter<<std::endl;
//   }

  // std::stack<int> s2;
  // int size = s.size();
  // for(int i = 0; i < size; i++)
  // {
  //   // std::cout<<s.top()<<std::endl;
  //   s2.push(s.top());
  //   s.pop();
  // }
  //
  // s = s2;

  // exit(0);
  for (int i = 0; i < V; i++){
    visited[i] = false;
  }

  std::stack<int> scc;
  std::unordered_set<int> lscc;
  int largeSCC = 0;

  for(int i = 0; i < V; i++){
    if(!visited[s.top()]){
      scc = std::stack<int>();
      // std::cout<<"round:"<<i<<std::endl;
      // std::cout<<s.top()<<std::endl;
      DFSUtil(s.top(),visited,row_offset_2,column_index_2,scc);
      if(scc.size() > largeSCC){
        // std::cout<<"gaibianlscc"<<std::endl;
        // std::cout<<"scc.size():"<<scc.size()<<std::endl;
        lscc.clear();
        largeSCC = scc.size();
        for(int j = 0; j < largeSCC; j++){
          lscc.insert(scc.top());
          scc.pop();
        }
        // for(std::unordered_set<int>::iterator iter=lscc.begin();iter!=lscc.end();iter++)
        // {
        //   std::cout<<*iter<<std::endl;
        // }
      }
    }
    s.pop();
  }
  // std::cout<<"lscc is:"<<std::endl;
  // for(std::unordered_set<int>::iterator iter = lscc.begin();iter != lscc.end();iter++){
  //     std::cout<<*iter<<"\n";
  // }
  // std::cout<<"dkjsdhfdsahfdkjshad hsddsfd"<<std::endl;

  std::fstream file(modified_file,std::ios::in);
  std::string str1 = modified_file.substr(0,modified_file.find("."));
  std::fstream ofile( str1 + "_scc_edge.txt",std::ios::out);
  // std::cout<<str1<<std::endl;
  edge scc_edge;
  std::string str2;
  while(getline(file,str2,'\n')){
    scc_edge = split(str2,"\t");
    if((lscc.find(scc_edge.start) != lscc.end())&&(lscc.find(scc_edge.end)!=lscc.end())){
      ofile<<scc_edge.start<<"\t"<<scc_edge.end<<"\n";
    }
  }
  file.close();
  ofile.close();
  pre2.IO_reorder(str1 + "_scc_edge.txt",wrapper2);
  return 0;
}

void thread_fun(std::string s, edges& e,int flag,std::vector<int>& row_offset,std::vector<int>& column_index){
  graphInfo g = read(s,e,flag);
  CSR* ptr = new CSR(e,g);

  row_offset = ptr->row_offset;
  column_index = ptr->column_index;
  delete ptr;
}

void DFS(int v ,bool visited[],bool explored[], std::vector<int>& row_offset,std::vector<int>& column_index,std::stack<int>& s){
    // printf("node: %d\n", v);
    if(!visited[v]){
      std::stack<int> tmp;
      visited[v] = true;
      tmp.push(v);
      while(!tmp.empty()){
        int cur = tmp.top();
        // printf("current top:%d\n", cur);
        int child = 0;
        for(int i = row_offset[cur];i<row_offset[cur+1];++i){
          if(!visited[column_index[i]]){
            visited[column_index[i]] = true;
            tmp.push(column_index[i]);
            child++;
            // printf("push column into tmp: %d\n", column_index[i]);
            break;
          }
        }
        if(child == 0){
          s.push(cur);
          // printf("push into stack S: %d\n", cur);
          tmp.pop();
        }
      }
    }
}

void DFSUtil(int v, bool visited[],std::vector<int>& row_offset,std::vector<int>& column_index,std::stack<int>& s)
{
    std::stack<int> tmp;
    if(!visited[v]){
      tmp.push(v);
      s.push(v);
      visited[v] = true;
      while(!tmp.empty()){
        int cur = tmp.top();
        tmp.pop();
        for (int i = row_offset[cur]; i <row_offset[cur+1]; ++i){
          if(!visited[column_index[i]]){
            visited[column_index[i]] = true;
            s.push(column_index[i]);
            tmp.push(column_index[i]);
          }
        }
      }
    }
}
