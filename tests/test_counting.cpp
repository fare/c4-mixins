#include <c4/mixins.hpp>

#include <cassert>
#include <string>

struct graph_log_base : c4::mixin {
  std::string log;
};

template<class Self, class Super>
struct GraphBase : Super {
  void visit_node(char name) {
    this->log += "node:";
    this->log += name;
    this->log += ";";
  }

  void visit_edge(char from, char to) {
    this->log += "edge:";
    this->log += from;
    this->log += "->";
    this->log += to;
    this->log += ";";
  }
};

template<class Self, class Super>
struct Counting : Super {
  using c4_parents = c4::parents<GraphBase>;

  int nodes_visited = 0;
  int edges_visited = 0;

  void visit_node(char name) {
    ++nodes_visited;
    Super::visit_node(name);
  }

  void visit_edge(char from, char to) {
    ++edges_visited;
    Super::visit_edge(from, to);
  }
};

template<class Self, class Super>
struct Labeled : Super {
  using c4_parents = c4::parents<GraphBase>;

  void visit_node(char name) {
    this->log += "label;";
    Super::visit_node(name);
  }
};

template<class Self, class Super>
struct CountingGraph : Super {
  using c4_parents = c4::parents<Counting, Labeled>;
};

using graph_class =
    c4::instantiate<CountingGraph, graph_log_base>;

int main() {
  graph_class graph;

  graph.visit_node('A');
  graph.visit_edge('A', 'B');
  graph.visit_node('B');

  assert(graph.nodes_visited == 2);
  assert(graph.edges_visited == 1);
  assert(graph.log == "label;node:A;edge:A->B;label;node:B;");
}
