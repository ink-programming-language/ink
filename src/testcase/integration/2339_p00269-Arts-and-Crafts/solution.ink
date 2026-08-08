// Translated from solution.cpp.

var NIL = -1;

var INF = 1e9;

class MinimumCostFlow
{
  var edge: dynamic;
}

func insert(f: dynamic, t: dynamic, w: dynamic, c: dynamic)
{
  edge[f].emplace_back(f, t, w, c);
  edge[t].emplace_back(t, f, (-w), 0);
}

func PrimalDual(source: dynamic, sink: dynamic, n: dynamic)
{
  var flow: dynamic;
  var distance: dynamic;
  var result = 0;
  while (true)
  {
    var difference: dynamic;
    var via: dynamic;
    var q: dynamic;
    q.emplace(0, Edge(source, source, 0, 0));
    while ((!q.empty()))
    {
      var dif: dynamic;
      var edge: dynamic;
      tie(dif, edge) = q.top();
      q.pop();
      var prev = edge.from_cpp;
      var current = edge.to;
      if (difference.count(current))
      {
        continue;
      }
      difference[current] = dif;
      via[current] = edge;
      if (edge.count(current))
      {
        for (var e in edge.at(current))
        {
          var residue = (e.capacity - flow[e.from_cpp][e.to]);
          var d = ((e.weight + distance[e.from_cpp]) - distance[e.to]);
          if ((residue > 0))
          {
            q.emplace((dif + d), e);
          }
        }
      }
    }
    for (var d in difference)
    {
      distance[d.first] += d.second;
    }
    if ((!via.count(sink)))
    {
      break;
    }
    var add = INF;
    {
      var v = sink;
      while ((v != via[v].from_cpp))
      {
        add = min(add, (via[v].capacity - flow[via[v].from_cpp][via[v].to]));
        v = via[v].from_cpp;
      }
    }
    {
      var v = sink;
      while ((v != via[v].from_cpp))
      {
        flow[via[v].from_cpp][via[v].to] += add;
        flow[via[v].to][via[v].from_cpp] -= add;
        result += (add * via[v].weight);
        v = via[v].from_cpp;
      }
    }
    n -= add;
    if ((n == 0))
    {
      return result;
    }
  }
  return -1;
}

var memo = cpp_construct(9);

func pow3(n: dynamic)
{
  return if (memo[n]) memo[n] else (cpp_assign(memo[n], "=", (if ((n < 1)) 1 else (3 * pow3((n - 1))))));
}

func main()
{
  var D: dynamic;
  var K: dynamic;
  var L: dynamic;
  while (cpp_comma((((cin >> D) >> K) >> L), ((D | K) | L)))
  {
    for (var i in c)
    {
      for (var j in i)
      {
        read(j);
      }
    }
    var M: dynamic;
    var N: dynamic;
    var P: dynamic;
    read(M, N, P);
    for (var i in r)
    {
      for (var j in i)
      {
        read(j);
      }
    }
    for (var i in t)
    {
      for (var j in i)
      {
        read(j);
      }
    }
    var INF = 1e9;
    var dp = cpp_construct((D + 1), vector(pow3(K), vector((L + 1), INF)));
    dp[0][0][0] = 0;
    {
      var day = 0;
      while ((day < D))
      {
        {
          var ternary = 0;
          while ((ternary < pow3(K)))
          {
            {
              var bought = 0;
              while ((bought <= L))
              {
                dp[(day + 1)][ternary][0] = min(dp[(day + 1)][ternary][0], dp[day][ternary][bought]);
                if ((bought == L))
                {
                  bought += 1;
                  continue;
                }
                {
                  var part = 0;
                  while ((part < K))
                  {
                    var owned = (((ternary / pow3(part))) % 3);
                    if ((2 <= owned))
                    {
                      part += 1;
                      continue;
                    }
                    dp[day][(ternary + pow3(part))][(bought + 1)] = min(dp[day][(ternary + pow3(part))][(bought + 1)], (dp[day][ternary][bought] + c[day][part]));
                    part += 1;
                  }
                }
                bought += 1;
              }
            }
            ternary += 1;
          }
        }
        day += 1;
      }
    }
    var graph: dynamic;
    var no_bag = (P + M);
    var source = ((P + M) + 1);
    var sink = ((P + M) + 2);
    graph.insert(source, no_bag, 0, INF);
    {
      var i = 0;
      while ((i < P))
      {
        graph.insert(source, i, 0, 1);
        i += 1;
      }
    }
    {
      var j = 0;
      while ((j < M))
      {
        graph.insert((P + j), sink, 0, 1);
        j += 1;
      }
    }
    {
      var j = 0;
      while ((j < M))
      {
        var need = 0;
        {
          var part = 0;
          while ((part < K))
          {
            need += (pow3(part) * r[j][part]);
            part += 1;
          }
        }
        if ((dp[D][need][0] != INF))
        {
          graph.insert(no_bag, (P + j), dp[D][need][0], 1);
        }
        {
          var i = 0;
          while ((i < P))
          {
            var remain = need;
            {
              var part = 0;
              while ((part < K))
              {
                if (((r[j][part] - t[i][part]) < 0))
                {
                  remain = -1;
                  break;
                }
                remain -= (pow3(part) * t[i][part]);
                part += 1;
              }
            }
            if ((~remain))
            {
              if ((dp[D][remain][0] != INF))
              {
                graph.insert(i, (P + j), dp[D][remain][0], 1);
              }
            }
            i += 1;
          }
        }
        j += 1;
      }
    }
    write(graph.PrimalDual(source, sink, N), "\n");
  }
}
