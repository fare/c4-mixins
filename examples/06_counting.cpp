// 06_counting.cpp
//
// Legacy counting example.
//
// This preserves a traditional mixin-literature style example: one mixin adds
// behavior to count traversal events while the base class defines traversal.

#include <c4/mixins.hpp>

#include <iostream>
#include <string_view>

template<class Self, class Super>
struct GraphBase : Super {
  void visit_node(std::string_view name) {
    std::cout << "visit node " << name << "\n";
  }

  void visit_edge(std::string_view from, std::string_view to) {
    std::cout << "visit edge " << from << " -> " << to << "\n";
  }
};

template<class Self, class Super>
struct Counting : Super {
  using c4_parents = c4::parents<GraphBase>;

  int nodes_visited = 0;
  int edges_visited = 0;

  void visit_node(std::string_view name) {
    ++nodes_visited;
    Super::visit_node(name);
  }

  void visit_edge(std::string_view from, std::string_view to) {
    ++edges_visited;
    Super::visit_edge(from, to);
  }
};

template<class Self, class Super>
struct Labeled : Super {
  using c4_parents = c4::parents<GraphBase>;

  void visit_node(std::string_view name) {
    std::cout << "[label] ";
    Super::visit_node(name);
  }
};

template<class Self, class Super>
struct CountingGraph : Super {
  using c4_parents = c4::parents<Counting, Labeled>;
};

using MyGraph = c4::instantiate<c4_examples::CountingGraph>;

int main() {
  MyGraph graph;

  graph.visit_node("A");
  graph.visit_edge("A", "B");
  graph.visit_node("B");

  std::cout << "nodes: " << graph.nodes_visited << "\n";
  std::cout << "edges: " << graph.edges_visited << "\n";
}
