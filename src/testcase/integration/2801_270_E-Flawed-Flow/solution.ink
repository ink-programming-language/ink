// Translated from solution.cpp.

var inflow: dynamic = cpp_array(200005);

var outflow: dynamic = cpp_array(200005);

var cost: dynamic = cpp_array(200005);

var A: dynamic = cpp_array(200005);

var B: dynamic = cpp_array(200005);

var ans: dynamic = cpp_array(200005);

var adj: dynamic = cpp_array(200005);

var it: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  memset(ans, -1, cpp_sizeof((ans)));
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  scanf("%d%d", (&N), (&M));
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&a), (&b), (&cost[i]));
      adj[a].push_back(make_pair(b, i));
      adj[b].push_back(make_pair(a, i));
      outflow[a] += cost[i];
      outflow[b] += cost[i];
      A[i] = a;
      B[i] = b;
      i += 1;
    }
  }
  var Q: dynamic = cpp_uninitialized();
  Q.push(1);
  while ((!Q.empty()))
  {
    var x: dynamic = Q.front();
    Q.pop();
    {
      it = adj[x].begin();
      while ((it != adj[x].end()))
      {
        var y: dynamic = it->first;
        var id: dynamic = it->second;
        if ((ans[id] != -1))
        {
          it += 1;
          continue;
        }
        ans[id] =  (((A[id] == x))) ? 0 : 1;
        inflow[y] += cost[id];
        outflow[y] -= cost[id];
        if (((inflow[y] == outflow[y]) && (y != N)))
        {
          Q.push(y);
        }
        it += 1;
      }
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
}
