#include <graphviz/gvc.h>
#include <stdio.h>
#include <stdlib.h>



int main(void) {
#ifndef NO_LAYOUT_OR_RENDERING
    // set up a graphviz context - but only once even for multiple graphs
  GVC_t *gvc = gvContext();
#endif

    // Create a simple digraph
    Agraph_t *g = agopen("MyGraph", Agdirected, 0);
    Agnode_t *n = agnode(g, "n", 1);
    Agnode_t *m = agnode(g, "m", 1);
    Agedge_t *e = agedge(g, n, m, 0, 1);

    // Set an attribute - in this case one that affects the visible rendering
    agsafeset(n, "color", "red", "");

    // Use the directed graph layout engine
  gvLayout(gvc, g, "dot");

  // Output in .dot format
  gvRender(gvc, g, "png", fopen("test.png", "w"));
  gvFreeLayout(gvc, g);

    agclose(g);

    return EXIT_SUCCESS;
}
