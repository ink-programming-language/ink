// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

var int_cpp = dynamic;

class LCA
{
  var N: dynamic;
  var logN: dynamic;
  var G: dynamic;
  var depth: dynamic;
  var parent: dynamic;
  func LCA(size: dynamic)
  {
      this->N = cpp_construct(size);
      this->G = cpp_construct(size);
      this->depth = cpp_construct(size);
      logN = 0;
      {
        var x = 1;
        while ((x < N))
        {
          logN += 1;
          x *= 2;
        }
      }
      parent.assign(max(logN, 1), vector(N));
    }
  func add_edge(u: dynamic, v: dynamic)
  {
      G[u].push_back(v);
      G[v].push_back(u);
    }
  func build()
  {
      var Q: dynamic;
      Q.emplace(0, -1);
      while ((!Q.empty()))
      {
        var p = Q.front();
        Q.pop();
        var cur = p.first;
        var prev = p.second;
        parent[0][cur] = prev;
        if ((prev == -1))
        {
          depth[cur] = 0;
        } else
        {
          depth[cur] = (depth[prev] + 1);
        }
        for (var next in G[cur])
        {
          if ((next != prev))
          {
            Q.emplace(next, cur);
          }
        }
      }
      {
        var k = 1;
        while ((k < logN))
        {
          {
            var v = 0;
            while ((v < N))
            {
              if ((parent[(k - 1)][v] == -1))
              {
                parent[k][v] = -1;
              } else
              {
                parent[k][v] = parent[(k - 1)][parent[(k - 1)][v]];
              }
              v += 1;
            }
          }
          k += 1;
        }
      }
    }
  func lca(u: dynamic, v: dynamic)
  {
      if ((depth[u] > depth[v]))
      {
        swap(u, v);
      }
      {
        var k = 0;
        while ((k < logN))
        {
          if ((((((depth[v] - depth[u])) >> k)) & 1))
          {
            v = parent[k][v];
          }
          k += 1;
        }
      }
      if ((u == v))
      {
        return u;
      }
      {
        var k = (logN - 1);
        while ((k >= 0))
        {
          if ((parent[k][u] != parent[k][v]))
          {
            u = parent[k][u];
            v = parent[k][v];
          }
          k -= 1;
        }
      }
      return parent[0][u];
    }
}

var sqrtN = 400;

class SqrtDecomposition
{
  var N: dynamic;
  var K: dynamic;
  var data: dynamic;
  var addpart: dynamic;
  var addall: dynamic;
  func SqrtDecomposition(n: dynamic)
  {
      this->N = cpp_construct(n);
      K = ((((N + sqrtN) - 1)) / sqrtN);
      data.assign((K * sqrtN), 0);
      addpart.assign(K, 0);
      addall.assign(K, 0);
    }
  func put(a: dynamic, b: dynamic, value: dynamic)
  {
      {
        var k = 0;
        while ((k < K))
        {
          var l = (k * sqrtN);
          var r = (((k + 1)) * sqrtN);
          if (((r <= a) || (b <= l)))
          {
            k += 1;
            continue;
          }
          if (((a <= l) && (r <= b)))
          {
            addall[k] += value;
          } else
          {
            {
              var i = max(a, l);
              while ((i < min(b, r)))
              {
                data[i] += value;
                addpart[k] += value;
                i += 1;
              }
            }
          }
          k += 1;
        }
      }
    }
  func get(a: dynamic, b: dynamic)
  {
      var ret = 0;
      {
        var k = 0;
        while ((k < K))
        {
          var l = (k * sqrtN);
          var r = (((k + 1)) * sqrtN);
          if (((r <= a) || (b <= l)))
          {
            k += 1;
            continue;
          }
          if (((a <= l) && (r <= b)))
          {
            ret += ((addall[k] * sqrtN) + addpart[k]);
          } else
          {
            {
              var i = max(a, l);
              while ((i < min(b, r)))
              {
                ret += (data[i] + addall[k]);
                i += 1;
              }
            }
          }
          k += 1;
        }
      }
      return ret;
    }
}

var MAX_N = 150000;

var IN = cpp_array(MAX_N);

var IN2 = cpp_array(MAX_N);

var OUT = cpp_array(MAX_N);

var OUT2 = cpp_array(MAX_N);

var in_cpp: dynamic;

var out: dynamic;

var G: dynamic;

func dfs()
{
  var S: dynamic;
  S.push(make_pair(+1, make_pair(0, -1)));
  while ((!S.empty()))
  {
    var pp = S.top();
    S.pop();
    var t = pp.first;
    var cur = pp.second.first;
    var prev = pp.second.second;
    if ((t == +1))
    {
      IN[cur] = cpp_update(in_cpp, "++");
      IN2[cur] = out;
      S.push(make_pair(-1, make_pair(cur, prev)));
      for (var next in G[cur])
      {
        if ((next != prev))
        {
          S.push(make_pair(+1, make_pair(next, cur)));
        }
      }
    } else
    {
      OUT[cur] = cpp_update(out, "++");
      OUT2[cur] = in_cpp;
    }
  }
}

func main()
{
  var N: dynamic;
  var Q: dynamic;
  scanf("%lld %lld", (&N), (&Q));
  G.resize(N);
  REP(i, (N - 1));
  {
    var a: dynamic;
    var b: dynamic;
    scanf("%lld %lld", (&a), (&b));
    G[a].push_back(b);
    G[b].push_back(a);
    lca.add_edge(a, b);
  }
  lca.build();
  in_cpp = 0;
  out = 0;
  dfs();
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var t: dynamic;
    var a: dynamic;
    var b: dynamic;
    scanf("%lld %lld %lld", (&t), (&a), (&b));
    if ((!t))
    {
      var c = lca.lca(a, b);
      var d1 = (plus.get(0, (IN[a] + 1)) - minus.get(0, IN2[a]));
      var d2 = (plus.get(0, (IN[b] + 1)) - minus.get(0, IN2[b]));
      var d3 = (plus.get(0, (IN[c] + 1)) - minus.get(0, IN2[c]));
      printf("%lld\n", ((d1 + d2) - (d3 * 2)));
    } else
    {
      plus.put((IN[a] + 1), OUT2[a], b);
      minus.put(IN2[a], OUT[a], b);
    }
  }
