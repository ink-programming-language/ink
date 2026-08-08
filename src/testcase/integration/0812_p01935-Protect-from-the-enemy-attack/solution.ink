// Translated from solution.cpp.

var BIG_NUM = cpp_expression("#include<b");

var MOD = cpp_expression("#include<b");

var EPS = cpp_expression("#include<bi");

var NUM = cpp_expression("#inc");

class Edge
{
  func Edge(arg_to: dynamic, arg_capacity: dynamic, arg_rev_index: dynamic)
  {
      to = arg_to;
      capacity = arg_capacity;
      rev_index = arg_rev_index;
    }
  var to: dynamic;
  var capacity: dynamic;
  var rev_index: dynamic;
}

var V: dynamic;

var E: dynamic;

var G = cpp_array(NUM);

var CAP1 = cpp_array(NUM);

var dist = cpp_array(NUM);

var cheked_index = cpp_array(NUM);

var can_visit = cpp_array(NUM);

func add_edge(from_cpp: dynamic, to: dynamic, capacity: dynamic)
{
  G[from_cpp].push_back(Edge(to, capacity, G[to].size()));
  G[to].push_back(Edge(from_cpp, capacity, (G[from_cpp].size() - 1)));
  if ((capacity == 1))
  {
    CAP1[from_cpp].push_back((G[from_cpp].size() - 1));
    CAP1[to].push_back((G[to].size() - 1));
  }
}

func bfs(source: dynamic)
{
  {
    var i = 0;
    while ((i < V))
    {
      dist[i] = -1;
      i += 1;
    }
  }
  var Q: dynamic;
  dist[source] = 0;
  Q.push(source);
  while ((!Q.empty()))
  {
    var node_id = Q.front();
    Q.pop();
    {
      var i = 0;
      while ((i < G[node_id].size()))
      {
        var e = G[node_id][i];
        if (((e.capacity > 0) && (dist[e.to] < 0)))
        {
          dist[e.to] = (dist[node_id] + 1);
          Q.push(e.to);
        }
        i += 1;
      }
    }
  }
}

func dfs(node_id: dynamic, sink: dynamic, flow: dynamic)
{
  if ((node_id == sink))
  {
    return flow;
  }
  {
    var i = cheked_index[node_id];
    while ((i < G[node_id].size()))
    {
      var e = G[node_id][i];
      if (((e.capacity > 0) && (dist[node_id] < dist[e.to])))
      {
        var tmp_flow = dfs(e.to, sink, min(flow, e.capacity));
        if ((tmp_flow > 0))
        {
          e.capacity -= tmp_flow;
          G[e.to][e.rev_index].capacity += tmp_flow;
          return tmp_flow;
        }
      }
      i += 1;
    }
  }
  return 0;
}

func recursive(node_id: dynamic)
{
  can_visit[node_id] = true;
  {
    var i = 0;
    while ((i < G[node_id].size()))
    {
      if (((can_visit[G[node_id][i].to] == false) && (G[node_id][i].capacity > 0)))
      {
        recursive(G[node_id][i].to);
      }
      i += 1;
    }
  }
}

func max_flow(source: dynamic, sink: dynamic)
{
  var flow = 0;
  var add: dynamic;
  while (true)
  {
    bfs(source);
    if ((dist[sink] < 0))
    {
      break;
    }
    {
      var i = 0;
      while ((i < V))
      {
        cheked_index[i] = 0;
        i += 1;
      }
    }
    while (((cpp_assign(add, "=", dfs(source, sink, BIG_NUM))) > 0))
    {
      flow += add;
    }
  }
  return flow;
}

func main()
{
  scanf("%d %d", (&V), (&E));
  var from_cpp: dynamic;
  var to: dynamic;
  var capacity: dynamic;
  {
    var loop = 0;
    while ((loop < E))
    {
      scanf("%d %d %d", (&from_cpp), (&to), (&capacity));
      add_edge(from_cpp, to, capacity);
      loop += 1;
    }
  }
  var source = 0;
  var sink = (V - 1);
  var flow = max_flow(source, sink);
  if ((flow >= 10002))
  {
    printf("-1\n");
    return 0;
  }
  var index: dynamic;
  {
    var i = 0;
    while ((i < V))
    {
      {
        var k = 0;
        while ((k < CAP1[i].size()))
        {
          index = CAP1[i][k];
          if ((G[i][index].capacity == 0))
          {
            {
              var a = 0;
              while ((a < V))
              {
                can_visit[a] = false;
                a += 1;
              }
            }
            recursive(i);
            if ((can_visit[G[i][index].to] == false))
            {
              flow -= 1;
              printf("%d\n", flow);
              return 0;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  if ((flow > 10000))
  {
    printf("-1\n");
  } else
  {
    printf("%d\n", flow);
  }
  return 0;
}
