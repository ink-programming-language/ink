// Translated from solution.cpp.

var INF = (1 << 58);

class edge
{
  var to: dynamic;
  var cost: dynamic;
}

func dijkstra(st: dynamic, G: dynamic, d: dynamic)
{
  var que: dynamic;
  fill(d.begin(), d.end(), INF);
  for (var s in st)
  {
    d[s] = 0;
    que.push(P(0, s));
  }
  while ((!que.empty()))
  {
    var p = que.top();
    que.pop();
    var v = p.second;
    if ((d[v] < p.first))
    {
      continue;
    }
    {
      var i = 0;
      while ((i < G[v].size()))
      {
        var e = G[v][i];
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

var a = cpp_array(200000);

var b = cpp_array(200000);

var c = cpp_array(200000);

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var M: dynamic;
  var R: dynamic;
  var Q: dynamic;
  read(N, M, R, Q);
  var id: dynamic;
  {
    var i = 0;
    while ((i < M))
    {
      read(a[i], b[i], c[i]);
      id[a[i]] = cpp_assign(id[b[i]], "=", 0);
      i += 1;
    }
  }
  var K = 0;
  for (var v in id)
  {
    id[v.first] = cpp_update(K, "++");
  }
  {
    var i = 0;
    while ((i < M))
    {
      G[id[b[i]]].push_back([id[a[i]], c[i]]);
      i += 1;
    }
  }
  var d = cpp_array(10);
  {
    var r = 0;
    while ((r < R))
    {
      d[r] = vector(K);
      var st: dynamic;
      {
        var i = 0;
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
  var sum = 0;
  {
    var q = 0;
    while ((q < Q))
    {
      var x = cpp_array(2);
      var z: dynamic;
      read(x[0], x[1], z);
      var s = 0;
      {
        var r = 0;
        while ((r < R))
        {
          var ok = true;
          var cost_sum = 0;
          {
            var i = 0;
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
