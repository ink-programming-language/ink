// Translated from solution.cpp.

var INF: dynamic = (1 << 58);

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
}

func dijkstra(st: dynamic, G: dynamic, d: dynamic) -> dynamic
{
  var que: dynamic = cpp_uninitialized();
  fill(d.begin(), d.end(), INF);
  for (var s: dynamic in st)
  {
    d[s] = 0;
    que.push(P(0, s));
  }
  while ((!que.empty()))
  {
    var p: dynamic = que.top();
    que.pop();
    var v: dynamic = p.second;
    if ((d[v] < p.first))
    {
      continue;
    }
    {
      var i: dynamic = 0;
      while ((i < G[v].size()))
      {
        var e: dynamic = G[v][i];
        if ((d[e.to] > (d[v] + e.cost)))
        {
          d[e.to] = (d[v] + e.cost);
          que.push(P(d[e.to], e.to));
        }
        i += 1;
      }
    }
  }
}

var a: dynamic = cpp_array(200000);

var b: dynamic = cpp_array(200000);

var c: dynamic = cpp_array(200000);

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  read(N, M, R, Q);
  var id: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      read(a[i], b[i], c[i]);
      id[a[i]] = cpp_assign(id[b[i]], "=", 0);
      i += 1;
    }
  }
  var K: dynamic = 0;
  for (var v: dynamic in id)
  {
    id[v.first] = cpp_update(K, "++");
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      G[id[b[i]]].push_back([id[a[i]], c[i]]);
      i += 1;
    }
  }
  var d: dynamic = cpp_array(10);
  {
    var r: dynamic = 0;
    while ((r < R))
    {
      d[r] = vector(K);
      var st: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < M))
        {
          if (((b[i] % R) == r))
          {
            st.push_back(id[b[i]]);
          }
          i += 1;
        }
      }
      dijkstra(st, G, d[r]);
      r += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var q: dynamic = 0;
    while ((q < Q))
    {
      var x: dynamic = cpp_array(2);
      var z: dynamic = cpp_uninitialized();
      read(x[0], x[1], z);
      var s: dynamic = 0;
      {
        var r: dynamic = 0;
        while ((r < R))
        {
          var ok: dynamic = true;
          var cost_sum: dynamic = 0;
          {
            var i: dynamic = 0;
            while ((i < 2))
            {
              if (((x[i] % R) != r))
              {
                if (((!id.count(x[i])) || (d[r][id[x[i]]] == INF)))
                {
                  ok = false;
                } else
                {
                  cost_sum += d[r][id[x[i]]];
                }
              }
              i += 1;
            }
          }
          if ((ok && (z > cost_sum)))
          {
            s = max(s, (z - cost_sum));
          }
          r += 1;
        }
      }
      sum += s;
      q += 1;
    }
  }
  write(sum, "\n");
}
