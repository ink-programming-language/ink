// Translated from solution.cpp.

func printList(list: dynamic)
{
  write("list : ");
  {
    typeof(((*list)).begin()) = (((*list)).begin());
    while ((li != ((*list)).end()))
    {
      write((*li), ",");
      li += 1;
    }
  }
  write("\n");
}

func printT_VPII(v: dynamic)
{
  write("list : ");
  {
    typeof((v).begin()) = ((v).begin());
    while ((li != (v).end()))
    {
      write(" ( ", ((*li)).first, " , ", ((*li)).second, " ) ,");
      li += 1;
    }
  }
  write("\n");
}

var n: dynamic;

var m: dynamic;

var nodeToListPointer: dynamic;

var listPointers: dynamic;

var oldEdges: dynamic;

var validEdges: dynamic;

func buildValidEdgeSetFromGraph(buildGraph: dynamic, graus: dynamic, lastUsedEdgeIndex: dynamic)
{
  if ((buildGraph.size() == m))
  {
    return true;
  }
  {
    typeof((lastUsedEdgeIndex + 1)) = ((lastUsedEdgeIndex + 1));
    while ((i <= ((static_cast(validEdges.size()) - 1))))
    {
      var edge = validEdges[i];
      if (cpp_binary((graus[edge.first] >= 2), "or", (graus[edge.second] >= 2)))
      {
        i += 1;
        continue;
      }
      buildGraph.insert(edge);
      graus[edge.first] += 1;
      graus[edge.second] += 1;
      var resolved = buildValidEdgeSetFromGraph(buildGraph, graus, i);
      if (resolved)
      {
        return true;
      }
      buildGraph.erase(edge);
      graus[edge.first] -= 1;
      graus[edge.second] -= 1;
      i += 1;
    }
  }
  return false;
}

func resolveBruteForce()
{
  var validEdgeSet: dynamic;
  {
    typeof(1) = (1);
    while ((i <= ((n - 1))))
    {
      {
        typeof((i + 1)) = ((i + 1));
        while ((j <= (n)))
        {
          validEdgeSet.insert(pair(i, j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    typeof(m);
    while (cpp_assign(i, "=", 0))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      if ((u > v))
      {
        swap(u, v);
      }
      oldEdges.insert(pair(u, v));
      validEdgeSet.erase(pair(u, v));
      i += 1;
    }
  }
  validEdges = vector((validEdgeSet).begin(), (validEdgeSet).end());
  var result: dynamic;
  var graus = vector((n + 1), 0);
  var resolved = buildValidEdgeSetFromGraph(result, graus, -1);
  if (cpp_unary("not", resolved))
  {
    write("-1", "\n");
  } else
  {
    {
      typeof((result).begin()) = ((result).begin());
      while ((e != (result).end()))
      {
        var edge = (*e);
        write(edge.first, " ", edge.second, "\n");
        e += 1;
      }
    }
  }
}

func main()
{
  read(n, m);
  if ((n < 10))
  {
    resolveBruteForce();
    return EXIT_SUCCESS;
  }
  {
    typeof(m) = 0;
    while ((i < (m)))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      var listEdgeNode_u = null;
      var listEdgeNode_v = null;
      if (nodeToListPointer.count(u))
      {
        listEdgeNode_u = nodeToListPointer[u];
      }
      if (nodeToListPointer.count(v))
      {
        listEdgeNode_v = nodeToListPointer[v];
      }
      if (cpp_binary((listEdgeNode_v == null), "and", (listEdgeNode_u == null)))
      {
        listEdgeNode_u = cpp_assign(listEdgeNode_v, "=", cpp_new());
        listEdgeNode_u->push_back(u);
        listEdgeNode_u->push_back(v);
        listPointers.insert(listEdgeNode_u);
        nodeToListPointer[u] = cpp_assign(nodeToListPointer[v], "=", listEdgeNode_u);
      } else if ((listEdgeNode_v == listEdgeNode_u))
      {
      } else if (cpp_binary((listEdgeNode_v == null), "or", (listEdgeNode_u == null)))
      {
        var listEdgeNode = if ((listEdgeNode_v == null)) listEdgeNode_u else listEdgeNode_v;
        if ((listEdgeNode->front() == u))
        {
          listEdgeNode->push_front(v);
        } else if ((listEdgeNode->front() == v))
        {
          listEdgeNode->push_front(u);
        } else if ((listEdgeNode->back() == v))
        {
          listEdgeNode->push_back(u);
        } else if ((listEdgeNode->back() == u))
        {
          listEdgeNode->push_back(v);
        } else
        {
        }
        nodeToListPointer[u] = cpp_assign(nodeToListPointer[v], "=", listEdgeNode);
      } else
      {
        if ((listEdgeNode_v->front() == listEdgeNode_u->front()))
        {
          listEdgeNode_u->reverse();
          listEdgeNode_u->splice(listEdgeNode_u->end(), (*listEdgeNode_v));
          cpp_delete((listEdgeNode_v));
          listPointers.erase(listEdgeNode_v);
          nodeToListPointer[v] = listEdgeNode_u;
        } else if ((listEdgeNode_v->back() == listEdgeNode_u->back()))
        {
          listEdgeNode_v->reverse();
          listEdgeNode_u->splice(listEdgeNode_u->end(), (*listEdgeNode_v));
          cpp_delete((listEdgeNode_v));
          listPointers.erase(listEdgeNode_v);
          nodeToListPointer[v] = listEdgeNode_u;
        } else if ((listEdgeNode_v->front() == listEdgeNode_u->back()))
        {
          listEdgeNode_u->splice(listEdgeNode_u->end(), (*listEdgeNode_v));
          cpp_delete((listEdgeNode_v));
          listPointers.erase(listEdgeNode_v);
          nodeToListPointer[v] = listEdgeNode_u;
        } else if ((listEdgeNode_v->front() == listEdgeNode_u->back()))
        {
          listEdgeNode_v->splice(listEdgeNode_v->end(), (*listEdgeNode_u));
          cpp_delete((listEdgeNode_u));
          listPointers.erase(listEdgeNode_u);
          nodeToListPointer[u] = listEdgeNode_v;
        } else
        {
        }
      }
      i += 1;
    }
  }
  var subgraph = null;
  {
    var numNodes = 0;
    {
      typeof((listPointers).begin()) = ((listPointers).begin());
      while ((lp != (listPointers).end()))
      {
        var g = (*lp);
        if ((g->size() > numNodes))
        {
          numNodes = g->size();
          subgraph = g;
        }
        lp += 1;
      }
    }
  }
  if (((m / 2) <= subgraph->size()))
  {
    var oddList = cpp_new();
    var evenList = cpp_new();
    var odd = true;
    {
      typeof(((*subgraph)).begin()) = (((*subgraph)).begin());
      while ((v != ((*subgraph)).end()))
      {
        if (odd)
        {
          oddList->push_back((*v));
        } else
        {
          evenList->push_back((*v));
        }
        odd = (!odd);
        v += 1;
      }
    }
    oddList->splice(oddList->end(), (*evenList));
    cpp_delete((evenList));
    listPointers.erase(subgraph);
    cpp_delete((subgraph));
    subgraph = oddList;
    if (((subgraph->size() % 2) == 0))
    {
      var ite1: dynamic;
      var ite2: dynamic;
      ite1 = cpp_assign(ite2, "=", subgraph->begin());
      ite1 += 1;
      swap((*ite1), (*ite2));
    }
  } else
  {
    listPointers.erase(subgraph);
  }
  var it = subgraph->begin();
  {
    typeof((listPointers).begin()) = ((listPointers).begin());
    while ((listPointerIte != (listPointers).end()))
    {
      var listToMergePtr = (*listPointerIte);
      {
        typeof(((*listToMergePtr)).begin()) = (((*listToMergePtr)).begin());
        while ((uIte != ((*listToMergePtr)).end()))
        {
          if ((it == subgraph->end()))
          {
            it = subgraph->begin();
          }
          subgraph->insert(cpp_update(it, "++"), (*uIte));
          uIte += 1;
        }
      }
      cpp_delete((listToMergePtr));
      listPointerIte += 1;
    }
  }
  {
    write(subgraph->back(), " ", subgraph->front(), "\n");
    var numPrintedEdges = 1;
    var lastU = subgraph->front();
    subgraph->pop_front();
    {
      typeof(((*subgraph)).begin()) = (((*subgraph)).begin());
      while ((uIte != ((*subgraph)).end()))
      {
        if ((numPrintedEdges == m))
        {
          break;
        }
        write(lastU, " ", (*uIte), "\n");
        lastU = (*uIte);
        numPrintedEdges += 1;
        uIte += 1;
      }
    }
  }
}
