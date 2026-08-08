// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(ll (i)=(k);(i)<(n);(i)++)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<iostr");
}

var N: dynamic;

var Q: dynamic;

var edges: dynamic;

var query: dynamic;

func input()
{
  var a: dynamic;
  var b: dynamic;
  read(N);
  edges.resize(N);
  read(Q);
}

func search(went: dynamic, route: dynamic)
{
  var now = route.back();
  for (var next in edges[now])
  {
    if (((route.size() > 1) && (route[(route.size() - 2)] == next)))
    {
      continue;
    }
    if (went[next])
    {
      route.push_back(next);
      return true;
    }
    went[next] = true;
    route.push_back(next);
    if (search(went, route))
    {
      return true;
    }
    went[next] = false;
    route.pop_back();
  }
  return false;
}

func output(route: dynamic)
{
  for (var q in query)
  {
    if ((binary_search(route.begin(), route.end(), q.first) && binary_search(route.begin(), route.end(), q.second)))
    {
      write(2, "\n");
    } else
    {
      write(1, "\n");
    }
  }
}

func main()
{
  input();
  var subroute: dynamic;
  var route: dynamic;
  subroute.push_back(0);
  search(went, subroute);
  var id = 0;
  while ((subroute[id] != subroute.back()))
  {
    id += 1;
  }
  FOR(i, (id + 1), subroute.size()).push_back(subroute[i]);
  sort(route.begin(), route.end());
  output(route);
  read(N);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    read(a, b);
    a -= 1;
    b -= 1;
    edges[a].push_back(b);
    edges[b].push_back(a);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    read(a, b);
    a -= 1;
    b -= 1;
    query.push_back([a, b]);
  }
