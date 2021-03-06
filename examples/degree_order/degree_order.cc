#include <iostream>

#include "sparsebase/format/format.h"
#include "sparsebase/object/object.h"
#include "sparsebase/preprocess/preprocess.h"
#include "sparsebase/utils/io/reader.h"

#include <set>

using namespace std;
using namespace sparsebase;

using vertex_type = unsigned int;
using edge_type = unsigned int;
using value_type = unsigned int;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: ./degree_order <uedgelist_file>\n";
    cout
        << "Hint: You can use the edgelist: examples/data/com-dblp.uedgelist\n";
    return 1;
  }
  cout << "F t re  s sp r e!" << endl;
  string file_name = argv[1];
  context::CPUContext cpu_context;

  cout << "********************************" << endl;

  cout << "Reading graph from " << file_name << "..." << endl;
  object::Graph<vertex_type, edge_type, value_type> g;
  g.ReadConnectivityFromEdgelistToCSR(file_name);
  cout << "Number of vertices: " << g.n_ << endl;
  cout << "Number of edges: " << g.m_ << endl;

  cout << "********************************" << endl;

  cout << "Sorting the vertices according to degree (degree ordering)..."
       << endl;

  preprocess::DegreeReorder<vertex_type, edge_type, value_type> orderer(1);
  format::Format *con = g.get_connectivity();
  vertex_type *order = orderer.GetReorder(con, {&cpu_context});
  vertex_type n = con->get_dimensions()[0];
  auto row_ptr =
      con->As<format::CSR<vertex_type, edge_type, value_type>>()->get_row_ptr();
  auto col =
      con->As<format::CSR<vertex_type, edge_type, value_type>>()->get_col();
  cout << "According to degree order: " << endl;
  cout << "First vertex, ID: " << order[0]
       << ", Degree: " << row_ptr[order[0] + 1] - row_ptr[order[0]] << endl;
  cout << "Last vertex, ID: " << order[n - 1]
       << ", Degree: " << row_ptr[order[n - 1] + 1] - row_ptr[order[n - 1]]
       << endl;

  cout << "********************************" << endl;

  cout << "Checking the correctness of the ordering..." << endl;
  auto *permutation = new vertex_type[n];
  for (vertex_type i = 0; i < n; i++) {
    permutation[order[i]] = i;
  }
  bool order_is_correct = true;
  set<vertex_type> check;
  for (vertex_type new_u = 0; new_u < n - 1 && order_is_correct; new_u++) {
    vertex_type u = permutation[new_u];
    if (check.find(u) == check.end()) {
      check.insert(u);
    } else {
      order_is_correct = false;
    }
    vertex_type v = permutation[new_u + 1];
    if (row_ptr[u + 1] - row_ptr[u] > row_ptr[v + 1] - row_ptr[v]) {
      cout << "Degree Order is incorrect!" << endl;
      order_is_correct = false;
      return 1;
    }
  }
  vertex_type v = permutation[n - 1];
  if (check.find(v) == check.end()) {
    check.insert(v);
  } else {
    order_is_correct = false;
  }
  if (order_is_correct) {
    cout << "Order is correct." << endl;
  }
  delete[] permutation;
  preprocess::Transform<vertex_type, edge_type, value_type> transformer(order);
  format::Format *csr = transformer.GetTransformation(con, {&cpu_context});
  auto *n_row_ptr =
      csr->As<format::CSR<vertex_type, edge_type, value_type>>()->get_row_ptr();
  auto *n_col =
      csr->As<format::CSR<vertex_type, edge_type, value_type>>()->get_col();
  cout << "Checking the correctness of the transformation..." << endl;
  bool transform_is_correct = true;
  for (vertex_type i = 0; i < n - 1 && transform_is_correct; i++) {
    if (n_row_ptr[i + 2] - n_row_ptr[i + 1] < n_row_ptr[i + 1] - n_row_ptr[i]) {
      cout << "Transformation is incorrect!" << endl;
      transform_is_correct = false;
      return 1;
    }
  }
  if (transform_is_correct) {
    cout << "Transformation is correct." << endl;
  }
  return 0;
}
