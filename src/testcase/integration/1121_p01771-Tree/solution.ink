// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

var int_cpp: dynamic = dynamic;

class LCA
{
  var N: dynamic = cpp_uninitialized();
  var logN: dynamic = cpp_uninitialized();
  var G: dynamic = cpp_uninitialized();
  var depth: dynamic = cpp_uninitialized();
  var parent: dynamic = cpp_uninitialized();
  func LCA(size: dynamic) -> dynamic
  {
      self->N = cpp_construct(size);
      self->G = cpp_construct(size);
      self->depth = cpp_construct(size);
      logN = 0;
      {
        var x: dynamic = 1;
        while ((x < N))
        {
          logN += 1;
          x *= 2;
        }
      }
      parent.assign(max(logN, 1), vector(N));
    }
  func add_edge(u: dynamic, v: dynamic) -> dynamic
  {
      G[u].push_back(v);
      G[v].push_back(u);
    }
  func build() -> dynamic
  {
      var Q: dynamic = cpp_uninitialized();
      Q.emplace(0, -1);
      while ((!Q.empty()))
      {
        var p: dynamic = Q.front();
        Q.pop();
        var cur: dynamic = p.first;
        var prev: dynamic = p.second;
        parent[0][cur] = prev;
        if ((prev == -1))
        {
          depth[cur] = 0;
        } else
        {
          depth[cur] = (depth[prev] + 1);
        }
        for (var next: dynamic in G[cur])
        {
          if ((next != prev))
          {
            Q.emplace(next, cur);
          }
        }
      }
      {
        var k: dynamic = 1;
        while ((k < logN))
        {
          {
            var v: dynamic = 0;
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
  func lca(u: dynamic, v: dynamic) -> dynamic
  {
      if ((depth[u] > depth[v]))
      {
        swap(u, v);
      }
      {
        var k: dynamic = 0;
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
        var k: dynamic = (logN - 1);
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

var sqrtN: dynamic = 400;

class SqrtDecomposition
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var data: dynamic = cpp_uninitialized();
  var addpart: dynamic = cpp_uninitialized();
  var addall: dynamic = cpp_uninitialized();
  func SqrtDecomposition(n: dynamic) -> dynamic
  {
      self->N = cpp_construct(n);
      K = ((((N + sqrtN) - 1)) / sqrtN);
      data.assign((K * sqrtN), 0);
      addpart.assign(K, 0);
      addall.assign(K, 0);
    }
  func put(a: dynamic, b: dynamic, value: dynamic) -> dynamic
  {
      {
        var k: dynamic = 0;
        while ((k < K))
        {
          var l: dynamic = (k * sqrtN);
          var r: dynamic = (((k + 1)) * sqrtN);
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
              var i: dynamic = max(a, l);
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
  func get(a: dynamic, b: dynamic) -> dynamic
  {
      var ret: dynamic = 0;
      {
        var k: dynamic = 0;
        while ((k < K))
        {
          var l: dynamic = (k * sqrtN);
          var r: dynamic = (((k + 1)) * sqrtN);
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
              var i: dynamic = max(a, l);
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

var MAX_N: dynamic = 150000;

var IN: dynamic = cpp_array(MAX_N);

var IN2: dynamic = cpp_array(MAX_N);

var OUT: dynamic = cpp_array(MAX_N);

var OUT2: dynamic = cpp_array(MAX_N);

var in_cpp: dynamic = cpp_uninitialized();

var out: dynamic = cpp_uninitialized();

var G: dynamic = cpp_uninitialized();

func dfs() -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  S.push(make_pair(+1, make_pair(0, -1)));
  while ((!S.empty()))
  {
    var pp: dynamic = S.top();
    S.pop();
    var t: dynamic = pp.first;
    var cur: dynamic = pp.second.first;
    var prev: dynamic = pp.second.second;
    if ((t == +1))
    {
      IN[cur] = cpp_update(in_cpp, "++");
      IN2[cur] = out;
      S.push(make_pair(-1, make_pair(cur, prev)));
      for (var next: dynamic in G[cur])
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

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  scanf("%lld %lld", (&N), (&Q));
  G.resize(N);
  REP(i, (N - 1));
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
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

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var t: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%lld %lld %lld", (&t), (&a), (&b));
    if ((!t))
    {
      var c: dynamic = lca.lca(a, b);
      var d1: dynamic = (plus.get(0, (IN[a] + 1)) - minus.get(0, IN2[a]));
      var d2: dynamic = (plus.get(0, (IN[b] + 1)) - minus.get(0, IN2[b]));
      var d3: dynamic = (plus.get(0, (IN[c] + 1)) - minus.get(0, IN2[c]));
      printf("%lld\n", ((d1 + d2) - (d3 * 2)));
    } else
    {
      plus.put((IN[a] + 1), OUT2[a], b);
      minus.put(IN2[a], OUT[a], b);
    }
  }
