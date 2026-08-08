// Translated from solution.cpp.

var inflow = cpp_array(200005);

var outflow = cpp_array(200005);

var cost = cpp_array(200005);

var A = cpp_array(200005);

var B = cpp_array(200005);

var ans = cpp_array(200005);

var adj = cpp_array(200005);

var it: dynamic;

func main()
{
  memset(ans, -1, cpp_sizeof((ans)));
  var N: dynamic;
  var M: dynamic;
  scanf("%d%d", (&N), (&M));
  {
    var i = 0;
    while ((i < M))
    {
      var a: dynamic;
      var b: dynamic;
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
  var Q: dynamic;
  Q.push(1);
  while ((!Q.empty()))
  {
    var x = Q.front();
    Q.pop();
    {
      it = adj[x].begin();
      while ((it != adj[x].end()))
      {
        var y = it->first;
        var id = it->second;
        if ((ans[id] != -1))
        {
          it += 1;
          continue;
        }
        ans[id] = if (((A[id] == x))) 0 else 1;
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
    var i = 0;
    while ((i < M))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
}
