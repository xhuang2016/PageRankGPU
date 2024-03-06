#include "CSR.cpp"
#include <cmath>
#include <thread>
#include <iomanip>
#include <ctime>

// struct other{
//   std::vector<double>* pr_1;
//   double d;
//   int V;
//   const std::vector<int>* row_offset_1;
//   const std::vector<int>* row_offset_2;
//   const std::vector<int>* column_index_1;
//   const std::vector<int>* column_index_2;
//   const std::vector<int>* danglingNodes;
// };

void thread_fun_read(std::string ,edges& ,int ,std::vector<int>& ,std::vector<int>&);
// void thread_fun_compute(int ,int ,std::vector<double>& ,other&);



int main(int argc, char const *argv[]) {
  // read the graph and transpose graph into CSR format
  edges e;
  edges r_e;
  std::vector<int> row_offset_1,row_offset_2;
  std::vector<int> column_index_1,column_index_2;
  std::thread thread_1(thread_fun_read,argv[1],std::ref(e),0,std::ref(row_offset_1),std::ref(column_index_1));
  std::thread thread_2(thread_fun_read,argv[1],std::ref(r_e),1,std::ref(row_offset_2),std::ref(column_index_2));
  // std::cout<<"hardware_concurrency()"<<std::thread::hardware_concurrency()<<std::endl;
  thread_1.join();
  thread_2.join();
  // exit(1);

  const double d = 0.15;
  const int max_iter = std::stoi(argv[2]);
  const int V = row_offset_1.size()-1;
  std::vector<double> pr_1(V, 1/double((V)));
  std::vector<double> pr_2(V, 0.0);

  // dangling nodes;
  std::vector<int> danglingNodes;
  for(int i=0;i<V;i++){
    if(row_offset_1[i+1] == row_offset_1[i]){
      danglingNodes.push_back(i);
    }
  }
  // other o;
  // o.pr_1 = &pr_1;
  // o.d = d;
  // o.V = V;
  // o.row_offset_1 = &row_offset_1;
  // o.row_offset_2 = &row_offset_2;
  // o.column_index_1 = &column_index_1;
  // o.column_index_2 = &column_index_2;
  // o.danglingNodes = &danglingNodes;
  // compute pagerank
  // set converge criteria
  const double  cc = 0.0000000001;
  int itertion_times = 0;
  // clock_t time_start = clock();
  for(int n_iter=0;n_iter<max_iter;n_iter++){
    double cc_sum = 0.0;
    itertion_times++;
    // no parallel method
    for(int i = 0;i<V;i++){
      // random access pagerank part;
      for(int j = 0;j<V;j++){
        pr_2[i] += d/(V)*pr_1[j];
      }
      // dangling nodes pagerank part;
      for(int j = 0;j<danglingNodes.size();j++){
        pr_2[i] += (1-d)/(V)*pr_1[danglingNodes[j]];
      }
      // inedge's pagerank part;
      for(int j = row_offset_2[i];j<row_offset_2[i+1];j++){
        pr_2[i] += (1-d)*pr_1[column_index_2[j]]/double(row_offset_1[column_index_2[j]+1]-row_offset_1[column_index_2[j]]);
      }
    }

    // thread fail
    // int hardware_concurrency = int(std::thread::hardware_concurrency());
    // std::thread thread_compute[hardware_concurrency];
    // int each_thread_V = V/hardware_concurrency+1;
    // for(int k = 0; k<hardware_concurrency-1;k++){
    //   thread_compute[k] = std::thread(thread_fun_compute,k*each_thread_V+0,k*(each_thread_V+1)-1,std::ref(pr_2) ,std::ref(o));
    // }
    // thread_compute[hardware_concurrency-1] = std::thread(thread_fun_compute,(hardware_concurrency-1)*each_thread_V,V-1,std::ref(pr_2),std::ref(o));
    //
    // for(int k = 0; k<hardware_concurrency;k++){
    //   thread_compute[k].join();
    // }

    for(int j = 0;j<V;j++){
      cc_sum += std::fabs(pr_1[j]-pr_2[j]);
    }

    if(cc_sum<=cc){
      // clock_t time_end = clock();
      // double elapsed_secs = double(time_end - time_start) / CLOCKS_PER_SEC;
      // std::cout<<elapsed_secs<<std::endl;
      // for(std::vector<double>::iterator iter=pr_2.begin();iter != pr_2.end();iter++){
      //   std::cout<<*iter<<std::endl;
      // }
      std::string s = argv[1];
      s = s.substr(s.find("/")+1,s.find(".")-s.find("/")-1);
      // std::cout<<s+"_pagerank_"+std::to_string(itertion_times)+".txt"<<std::endl;
      std::fstream file("output/"+s+"_pagerank_"+std::to_string(itertion_times)+"thread.txt",std::ios::out);
      file<<std::setprecision(9);
      std::cout<<"start writing to file"<<std::endl;
      for(std::vector<double>::iterator iter=pr_2.begin();iter != pr_2.end();iter++){
        file<<*iter<<"\n";
      }
      // std::cout<<"itertion_times:"<<itertion_times<<std::endl;
      file.close();
      break;
    }
    // std::cout<<"itertion_times"<<itertion_times<<std::endl;
    pr_1 = pr_2;
    // o.pr_1 = &pr_1;
    for(std::vector<double>::iterator iter=pr_2.begin();iter != pr_2.end();iter++){
      *iter = 0;
    }
  }
  // check the summation of the pagerank
  // double sum =0;
  // std::cout<<"start"<<std::endl;
  // for(std::vector<double>::iterator iter=pr_1.begin();iter != pr_1.end();iter++){
  //   std::cout<<*iter<<" ";
  //   sum += *iter;
  // }
  // std::cout<<std::endl<<"sum:"<<sum<<std::endl;;
  return 0;
}

// void thread_fun_compute(int start,int end, std::vector<double>& pr_2,other& o){
//   for(int i = start;i<=end;i++){
//   //   // random access pagerank part;
//     for(int j = 0;j<o.V;j++){
//       pr_2[i] += o.d/(o.V)*(*o.pr_1)[j];
//     }
//     // dangling nodes pagerank part;
//     for(int j = 0;j<(o.danglingNodes->size());j++){
//       pr_2[i] += (1-o.d)/(o.V)*(*o.pr_1)[(*o.danglingNodes)[j]];
//     }
//     // inedge's pagerank part;
//     for(int j = (*o.row_offset_2)[i];j<(*o.row_offset_2)[i+1];j++){
//       pr_2[i] += (1-o.d)*(*o.pr_1)[(*o.column_index_2)[j]]/double((*o.row_offset_1)[(*o.column_index_2)[j]+1]-(*o.row_offset_1)[(*o.column_index_2)[j]]);
//     }
//   }
// }


void thread_fun_read(std::string s, edges& e,int flag,std::vector<int>& row_offset,std::vector<int>& column_index){
 graphInfo g = read(s,e,flag);
 CSR* ptr = new CSR(e,g);

 row_offset = ptr->row_offset;
 column_index = ptr->column_index;

 delete ptr;
}
