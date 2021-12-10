#ifndef _SPARSEOBJECT_HPP
#define _SPARSEOBJECT_HPP

#include "SparseFormat.hpp"
#include "SparseReader.hpp"

namespace sparsebase{

  class SparseObject{
    public:
      virtual ~SparseObject();
      virtual void verify_structure() = 0;
  };

  template <typename ID_t, typename NNZ_t>
  class AbstractSparseObject : public SparseObject{
    protected:
      SparseFormat<ID_t, NNZ_t> *connectivity;
    public:
      virtual ~AbstractSparseObject();
      SparseFormat<ID_t, NNZ_t> * get_connectivity();
  };

  template<typename v_t, typename e_t>
  class Graph : public AbstractSparseObject<v_t, e_t>{
    public:
      Graph(SparseFormat<v_t, e_t> * _connectivity);
      Graph(SparseReader<v_t, e_t> * r);
      void initialize_info_from_connection();
      virtual ~Graph();
      void verify_structure();
      v_t n;
      e_t m;
  };

  //template<typename v_t, typename e_t, typename t_t>
  //class TemporalGraph : public AbstractSparseObject<v_t, e_t>{
  //  public:
  //    TemporalGraph(SparseFormat<v_t, e_t, t_t> * _connectivity){
  //      // init temporal graph
  //    }
  //    TemporalGraph(SparseReader<v_t, e_t, t_t> * r){
  //      // init temporal graph from file
  //    }
  //    virtual ~TemporalGraph(){};
  //    void verify_structure(){
  //      // check order
  //      if (this->connectivity->get_order() != 2) //throw error
  //      // check dimensions
  //    }
  //    v_t n;
  //    e_t m;
  //    // ...
  //};

}

#endif