// Translated from solution.cpp.

var NIL: dynamic = -1;

var INF: dynamic = 1e9;

class MinimumCostFlow
{
  var edge: dynamic = cpp_uninitialized();
}

func insert(f: dynamic, t: dynamic, w: dynamic, c: dynamic) -> dynamic
{
  edge[f].emplace_back(f, t, w, c);
  edge[t].emplace_back(t, f, (-w), 0);
}

func PrimalDual(source: dynamic, sink: dynamic, n: dynamic) -> dynamic
{
  var flow: dynamic = cpp_uninitialized();
  var distance: dynamic = cpp_uninitialized();
  var result: dynamic = 0;
  while (true)
  {
    var difference: dynamic = cpp_uninitialized();
    var via: dynamic = cpp_uninitialized();
    var q: dynamic = cpp_uninitialized();
    q.emplace(0, Edge(source, source, 0, 0));
    while ((!q.empty()))
    {
      var dif: dynamic = cpp_uninitialized();
      var edge: dynamic = cpp_uninitialized();
      tie(dif, edge) = q.top();
      q.pop();
      var prev: dynamic = edge.from_cpp;
      var current: dynamic = edge.to;
      if (difference.count(current))
      {
        continue;
      }
      difference[current] = dif;
      via[current] = edge;
      if (edge.count(current))
      {
        for (var e: dynamic in edge.at(current))
        {
          var residue: dynamic = (e.capacity - flow[e.from_cpp][e.to]);
          var d: dynamic = ((e.weight + distance[e.from_cpp]) - distance[e.to]);
          if ((residue > 0))
          {
            q.emplace((dif + d), e);
          }
        }
      }
    }
    for (var d: dynamic in difference)
    {
      distance[d.first] += d.second;
    }
    if ((!via.count(sink)))
    {
      break;
    }
    var add: dynamic = INF;
    {
      var v: dynamic = sink;
      while ((v != via[v].from_cpp))
      {
        add = min(add, (via[v].capacity - flow[via[v].from_cpp][via[v].to]));
        v = via[v].from_cpp;
      }
    }
    {
      var v: dynamic = sink;
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

var memo: dynamic = cpp_construct(9);

func pow3(n: dynamic) -> dynamic
{
  return  (memo[n]) ? memo[n] : (cpp_assign(memo[n], "=", ( ((n < 1)) ? 1 : (3 * pow3((n - 1))))));
}

func main() -> dynamic
{
  var D: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  while (cpp_comma((((cin >> D) >> K) >> L), ((D | K) | L)))
  {
    for (var i: dynamic in c)
    {
      for (var j: dynamic in i)
      {
        read(j);
      }
    }
    var M: dynamic = cpp_uninitialized();
    var N: dynamic = cpp_uninitialized();
    var P: dynamic = cpp_uninitialized();
    read(M, N, P);
    for (var i: dynamic in r)
    {
      for (var j: dynamic in i)
      {
        read(j);
      }
    }
    for (var i: dynamic in t)
    {
      for (var j: dynamic in i)
      {
        read(j);
      }
    }
    var INF: dynamic = 1e9;
    var dp: dynamic = cpp_construct((D + 1), vector(pow3(K), vector((L + 1), INF)));
    dp[0][0][0] = 0;
    {
      var day: dynamic = 0;
      while ((day < D))
      {
        {
          var ternary: dynamic = 0;
          while ((ternary < pow3(K)))
          {
            {
              var bought: dynamic = 0;
              while ((bought <= L))
              {
                dp[(day + 1)][ternary][0] = min(dp[(day + 1)][ternary][0], dp[day][ternary][bought]);
                if ((bought == L))
                {
                  bought += 1;
                  continue;
                }
                {
                  var part: dynamic = 0;
                  while ((part < K))
                  {
                    var owned: dynamic = (((ternary / pow3(part))) % 3);
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
    var graph: dynamic = cpp_uninitialized();
    var no_bag: dynamic = (P + M);
    var source: dynamic = ((P + M) + 1);
    var sink: dynamic = ((P + M) + 2);
    graph.insert(source, no_bag, 0, INF);
    {
      var i: dynamic = 0;
      while ((i < P))
      {
        graph.insert(source, i, 0, 1);
        i += 1;
      }
    }
    {
      var j: dynamic = 0;
      while ((j < M))
      {
        graph.insert((P + j), sink, 0, 1);
        j += 1;
      }
    }
    {
      var j: dynamic = 0;
      while ((j < M))
      {
        var need: dynamic = 0;
        {
          var part: dynamic = 0;
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
          var i: dynamic = 0;
          while ((i < P))
          {
            var remain: dynamic = need;
            {
              var part: dynamic = 0;
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
