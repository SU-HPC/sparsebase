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

struct customParam : preprocess::PreprocessParams {
  customParam(int h) : hyperparameter(h) {}
  int hyperparameter;
};
vertex_type *degree_reorder_csr(std::vector<format::Format *> formats,
                                preprocess::PreprocessParams *params) {
  format::CSR<vertex_type, edge_type, value_type> *csr =
      formats[0]->As<format::CSR<vertex_type, edge_type, value_type>>();
  customParam *cast_params = static_cast<customParam *>(params);
  cout << "Custom hyperparameter: " << cast_params->hyperparameter << endl;
  vertex_type n = csr->get_dimensions()[0];
  vertex_type *counts = new vertex_type[n]();
  auto row_ptr = csr->get_row_ptr();
  for (vertex_type u = 0; u < n; u++) {
    counts[row_ptr[u + 1] - row_ptr[u] + 1]++;
  }
  for (vertex_type u = 1; u < n; u++) {
    counts[u] += counts[u - 1];
  }
  vertex_type *sorted = new vertex_type[n];
  memset(sorted, -1, sizeof(vertex_type) * n);
  vertex_type *mr = new vertex_type[n]();
  for (vertex_type u = 0; u < n; u++) {
    vertex_type ec = counts[row_ptr[u + 1] - row_ptr[u]];
    sorted[ec + mr[ec]] = u;
    mr[ec]++;
  }
  auto *inverse_permutation = new vertex_type[n];
  for (vertex_type i = 0; i < n; i++) {
    inverse_permutation[sorted[i]] = i;
  }
  delete[] mr;
  delete[] counts;
  delete[] sorted;
  return inverse_permutation;
}
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: ./custom_order <uedgelist_file>\n";
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
  format::Format *con = g.get_connectivity();

  // ReorderInstance<vertex_type, edge_type, value_type, GenericReorder>
  // orderer;
  preprocess::GenericReorder<vertex_type, edge_type, value_type> orderer;
  orderer.RegisterFunction(
      {format::CSR<vertex_type, edge_type, value_type>::get_format_id_static()},
      degree_reorder_csr);
  customParam params{10};
  vertex_type *order = orderer.GetReorder(con, &params, {&cpu_context});

  vertex_type n = con->get_dimensions()[0];
  auto xadj =
      con->As<format::CSR<vertex_type, edge_type, value_type>>()->get_row_ptr();
  auto adj =
      con->As<format::CSR<vertex_type, edge_type, value_type>>()->get_col();
  cout << "According to degree order: " << endl;
  cout << "First vertex, ID: " << order[0]
       << ", Degree: " << xadj[order[0] + 1] - xadj[order[0]] << endl;
  cout << "Last vertex, ID: " << order[n - 1]
       << ", Degree: " << xadj[order[n - 1] + 1] - xadj[order[n - 1]] << endl;

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
    if (xadj[u + 1] - xadj[u] > xadj[v + 1] - xadj[v]) {
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
  } else {
    cout << "Degree Order is incorrect!" << endl;
    return 1;
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
