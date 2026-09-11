// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var pb: dynamic = cpp_expression("#include<");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

var INF: dynamic = 1001001001001001001;

var par: dynamic = cpp_array(111111, 20);

var A: dynamic = cpp_array(111111);

var N: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(111111);

var dep: dynamic = cpp_array(111111);

var latte: dynamic = cpp_array(111111);

var dp: dynamic = cpp_array(111111, 20);

var lis: dynamic = cpp_array(111111);

func dfs(v: dynamic, p: dynamic, d: dynamic) -> dynamic
{
  dep[v] = d;
  par[0][v] = p;
  latte[v] = A[v];
  lis[v].pb(0);
  lis[v].pb(0);
  for (var u: dynamic in G[v])
  {
    if ((u == p))
    {
      continue;
    }
    dfs(u, v, (d + 1));
    latte[v] += latte[u];
    lis[v].pb(latte[u]);
  }
}

func lca(a: dynamic, b: dynamic) -> dynamic
{
  if ((dep[a] < dep[b]))
  {
    swap(a, b);
  }
  rep(i, 20);
  if (((((dep[a] - dep[b])) >> i) & 1))
  {
    a = par[i][a];
  }
  if ((a == b))
  {
    return a;
  }
  {
    var i: dynamic = 19;
    while ((i >= 0))
    {
      if ((par[i][a] != par[i][b]))
      {
        a = par[i][a];
        b = par[i][b];
      }
      i -= 1;
    }
  }
  return par[0][a];
}

func main() -> dynamic
{
  read(N);
  rep(i, N);
  read(A[i]);
  rep(i, (N - 1));
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    a -= 1;
    b -= 1;
    G[a].pb(b);
    G[b].pb(a);
  }
  dfs(0, -1, 0);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      var p: dynamic = par[0][i];
      if ((lis[p][0] == latte[i]))
      {
        dp[0][i] = lis[p][1];
      } else
      {
        dp[0][i] = lis[p][0];
      }
      i += 1;
    }
  }
  rep(i, 19);
  {
  }
  var sumall: dynamic = accumulate(A, (A + N), 0);
  var Q: dynamic = cpp_uninitialized();
  read(Q);
  while (cpp_update(Q, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    a -= 1;
    b -= 1;
    var p: dynamic = lca(a, b);
    var ans: dynamic = 0;
    rep(lattemalta, 2);
    {
      swap(a, b);
      var x: dynamic = a;
      if ((a == p))
      {
        continue;
      }
      chmax(ans, lis[a][0]);
      var d: dynamic = ((dep[x] - dep[p]) - 1);
      rep(i, 20);
      {
        if (((d >> i) & 1))
        {
          chmax(ans, dp[i][x]);
          x = par[i][x];
        }
      }
      a = x;
    }
    {
      var flaga: dynamic = false;
      var flagb: dynamic = false;
      for (var x: dynamic in lis[p])
      {
        if (((!flaga) && (x == latte[a])))
        {
          flaga = true;
          continue;
        }
        if (((!flagb) && (x == latte[b])))
        {
          flagb = true;
          continue;
        }
        chmax(ans, x);
        break;
      }
    }
    chmax(ans, (sumall - latte[p]));
    write(ans, "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    sort(all(lis[i]));
    reverse(all(lis[i]));
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((par[i][j] == -1))
      {
        par[(i + 1)][j] = -1;
        dp[(i + 1)][j] = dp[i][j];
      } else
      {
        par[(i + 1)][j] = par[i][par[i][j]];
        dp[(i + 1)][j] = max(dp[i][par[i][j]], dp[i][j]);
      }
    }
