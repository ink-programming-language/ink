// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(UL i = 0; i < (n); i++)");
}

var N: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var P: dynamic = cpp_array(18, 100000);

var D: dynamic = cpp_array(100000);

var C: dynamic = cpp_array(100000);

var E: dynamic = cpp_array(100000);

func LCA(a: dynamic, b: dynamic) -> dynamic
{
  if ((C[a] < C[b]))
  {
    swap(a, b);
  }
  var d: dynamic = (C[a] - C[b]);
  {
    var i: dynamic = 17;
    while ((i != (~0)))
    {
      if ((((1 << i)) & d))
      {
        a = P[a][i];
      }
      i -= 1;
    }
  }
  if ((a == b))
  {
    return a;
  }
  {
    var i: dynamic = 17;
    while ((i != (~0)))
    {
      if ((P[a][i] == P[b][i]))
      {
        i -= 1;
        continue;
      }
      a = P[a][i];
      b = P[b][i];
      i -= 1;
    }
  }
  return P[a][0];
}

func MP(a: dynamic, b: dynamic) -> dynamic
{
  var X: dynamic = D[LCA(a, b)];
  if ((D[a] < D[b]))
  {
    swap(a, b);
  }
  var Y: dynamic = (((D[a] - D[b]) + X) + X);
  {
    var i: dynamic = 17;
    while ((i != (~0)))
    {
      if (((D[P[a][i]] * 2) <= Y))
      {
        i -= 1;
        continue;
      }
      a = P[a][i];
      i -= 1;
    }
  }
  if ((X == D[a]))
  {
    return [a, a];
  } else
  {
    return [a, P[a][0]];
  }
}

func Dist(a: dynamic, b: dynamic) -> dynamic
{
  var X: dynamic = D[LCA(a, b)];
  return (((D[a] + D[b]) - X) - X);
}

func main() -> dynamic
{
  scanf("%u%u", (&N), (&Q));
  rep(i, (N - 1));
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    scanf("%u%u%u", (&u), (&v), (&w));
    u -= 1;
    v -= 1;
    E[u].push_back([v, w]);
    E[v].push_back([u, w]);
  }
  {
    P[0][0] = 0;
    D[0] = 0;
    C[0] = 0;
    var G: dynamic = cpp_uninitialized();
    G.push(0);
    while (G.size())
    {
      var p: dynamic = G.front();
      G.pop();
      for (var e: dynamic in E[p])
      {
        if ((P[p][0] == e.first))
        {
          continue;
        }
        P[e.first][0] = p;
        D[e.first] = (D[p] + e.second);
        C[e.first] = (C[p] + 1);
        G.push(e.first);
      }
    }
  }
  rep(i, 17);
  rep(j, N)[j][(i + 1)] = P[P[j][i]][i];
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    scanf("%u%u%u", (&a), (&b), (&c));
    a -= 1;
    b -= 1;
    c -= 1;
    var ans: dynamic = (~0);
    rep(t, 3);
    {
      var mp: dynamic = MP(a, b);
      var q1: dynamic = max(max(Dist(mp.first, a), Dist(mp.first, b)), Dist(mp.first, c));
      var q2: dynamic = max(max(Dist(mp.second, a), Dist(mp.second, b)), Dist(mp.second, c));
      ans = min(ans, min(q1, q2));
      swap(a, b);
      swap(b, c);
    }
    printf("%u\n", ans);
  }
