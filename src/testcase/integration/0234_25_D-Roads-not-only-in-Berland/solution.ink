// Translated from solution.cpp.

var undef = -1;

var n = 0;

var visited = cpp_array(1000);

var graph: dynamic;

var delegates: dynamic;

var road_list: dynamic;

func init()
{
  delegates.reserve((1000 + 10));
  graph.reserve((1000 + 10));
  {
    var i = 0;
    while ((i < 1000))
    {
      visited[i] = undef;
      graph.push_back(list());
      delegates.push_back(undef);
      i += 1;
    }
  }
  read(n);
  var a = 0;
  var b = 0;
  {
    var i = 1;
    while ((i < n))
    {
      read(a, b);
      graph[(a - 1)].push_back((b - 1));
      graph[(b - 1)].push_back((a - 1));
      i += 1;
    }
  }
}

func solve()
{
  var comp = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((visited[i] == undef))
      {
        explore(i, comp, undef);
        delegates[comp] = i;
        comp += 1;
      }
      i += 1;
    }
  }
  write((comp - 1), "\n");
  var i = 0;
  while (((comp - 1) != 0))
  {
    var sIt = road_list.begin();
    var p = (*sIt);
    write((p.first + 1), " ", (p.second + 1), " ");
    write((delegates[i] + 1), " ", (delegates[(i + 1)] + 1), "\n");
    road_list.erase(sIt);
    i += 1;
    comp -= 1;
  }
}

func explore(i: dynamic, component: dynamic, comefrom: dynamic)
{
  visited[i] = component;
  {
    var it = graph[i].begin();
    while ((it != graph[i].end()))
    {
      if ((visited[(*it)] == undef))
      {
        explore((*it), component, i);
      } else if ((comefrom != (*it)))
      {
        if ((road_list.find(make_pair((*it), i)) == road_list.end()))
        {
          road_list.insert(make_pair(i, (*it)));
        }
      }
      it += 1;
    }
  }
}

func main()
{
  init();
  solve();
  return 0;
}
