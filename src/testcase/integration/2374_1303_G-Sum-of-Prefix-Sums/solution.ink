// Translated from solution.cpp.

var N = (2e5 + 100);

var is_query = (-((1 << 62)));

class Line
{
  var m: dynamic;
  var b: dynamic;
  var succ: dynamic;
  func operator_less(rhs: dynamic)
  {
      if ((rhs.b != is_query))
      {
        return (m < rhs.m);
      }
      var s = succ();
      if ((!s))
      {
        return 0;
      }
      var x = rhs.m;
      return ((b - s->b) < (((s->m - m)) * x));
    }
}

class CHT
{
  func bad(y: dynamic)
  {
      var z = next(y);
      if ((y == begin()))
      {
        if ((z == end()))
        {
          return 0;
        }
        return ((y->m == z->m) && (y->b <= z->b));
      }
      var x = prev(y);
      if ((z == end()))
      {
        return ((y->m == x->m) && (y->b <= x->b));
      }
      return ((((x->b - y->b)) * ((z->m - y->m))) >= (((y->b - z->b)) * ((y->m - x->m))));
    }
  func insert_line(m: dynamic, b: dynamic)
  {
      var y = insert([m, b]);
      if (bad(y))
      {
        erase(y);
        return;
      }
      while (((next(y) != end()) && bad(next(y))))
      {
        erase(next(y));
      }
      y->succ = __cpp_lambda_1;
      while (((y != begin()) && bad(prev(y))))
      {
        erase(prev(y));
      }
      if ((y != begin()))
      {
        prev(y)->succ = __cpp_lambda_2;
      }
    }
  func eval(x: dynamic)
  {
      var l = (*lower_bound([x, is_query]));
      return ((l.m * x) + l.b);
    }
}

var n: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var sz = cpp_array(N);

var big = cpp_array(N);

var dead = cpp_array(N);

var nei = cpp_array(N);

func input()
{
  read(n);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      nei[cpp_update(u, "--")].push_back(cpp_update(v, "--"));
      nei[v].push_back(u);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
}

func dfs(v: dynamic, par: dynamic, len: dynamic, sigma1: dynamic, sigma2: dynamic, sum: dynamic, vec: dynamic)
{
  vec->push_back(pair(pair(sum, len), pair(sigma1, sigma2)));
  for (var u in nei[v])
  {
    if (((dead[u] == false) && (u != par)))
    {
      dfs(u, v, (len + 1), (sigma1 + (((len + 1)) * a[u])), (sigma2 + ((sum + a[u]))), (sum + a[u]), vec);
    }
  }
}

func go(v: dynamic, par: dynamic = -1)
{
  sz[v] = 1;
  big[v] = -1;
  for (var u in nei[v])
  {
    if (((dead[u] == false) && (u != par)))
    {
      go(u, v);
      if (((big[v] == -1) || (sz[u] > sz[big[v]])))
      {
        big[v] = u;
      }
      sz[v] += sz[u];
    }
  }
}

func get_cen(v: dynamic)
{
  go(v);
  var n = sz[v];
  while (((big[v] != -1) && ((2 * sz[big[v]]) > n)))
  {
    v = big[v];
  }
  return v;
}

func solve(v: dynamic = 0)
{
  var cen = get_cen(v);
  dead[cen] = true;
  var data: dynamic;
  data.insert_line(1, a[cen]);
  for (var u in nei[cen])
  {
    if ((dead[u] == false))
    {
      solve(u);
      var vec: dynamic;
      dfs(u, cen, 1, a[u], a[u], a[u], (&vec));
      for (var p in vec)
      {
        ans = max(ans, (p.second.first + data.eval(p.first.first)));
        ans = max(ans, ((((p.first.second + 1)) * a[cen]) + p.second.second));
      }
      for (var p in vec)
      {
        data.insert_line((p.first.second + 1), (p.second.second + (((p.first.second + 1)) * a[cen])));
      }
    }
  }
  data.clear();
  data.insert_line(1, a[cen]);
  reverse(nei[cen].begin(), nei[cen].end());
  for (var u in nei[cen])
  {
    if ((dead[u] == false))
    {
      var vec: dynamic;
      dfs(u, cen, 1, a[u], a[u], a[u], (&vec));
      for (var p in vec)
      {
        ans = max(ans, (p.second.first + data.eval(p.first.first)));
      }
      for (var p in vec)
      {
        data.insert_line((p.first.second + 1), (p.second.second + (((p.first.second + 1)) * a[cen])));
      }
    }
  }
  dead[cen] = false;
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  input();
  solve();
  write(ans, cpp_char("\n"));
}

func __cpp_lambda_1()
{
  return if ((next(y) == end())) 0 else (&(*next(y)));
}

func __cpp_lambda_2()
{
  return (&(*y));
}
