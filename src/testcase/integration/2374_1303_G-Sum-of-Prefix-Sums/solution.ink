// Translated from solution.cpp.

var N: dynamic = (2e5 + 100);

var is_query: dynamic = (-((1 << 62)));

class Line
{
  var m: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var succ: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      if ((rhs.b != is_query))
      {
        return (m < rhs.m);
      }
      var s: dynamic = succ();
      if ((!s))
      {
        return 0;
      }
      var x: dynamic = rhs.m;
      return ((b - s->b) < (((s->m - m)) * x));
    }
}

class CHT
{
  func bad(y: dynamic) -> dynamic
  {
      var z: dynamic = next(y);
      if ((y == begin()))
      {
        if ((z == end()))
        {
          return 0;
        }
        return ((y->m == z->m) && (y->b <= z->b));
      }
      var x: dynamic = prev(y);
      if ((z == end()))
      {
        return ((y->m == x->m) && (y->b <= x->b));
      }
      return ((((x->b - y->b)) * ((z->m - y->m))) >= (((y->b - z->b)) * ((y->m - x->m))));
    }
  func insert_line(m: dynamic, b: dynamic) -> dynamic
  {
      var y: dynamic = insert([m, b]);
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
  func eval(x: dynamic) -> dynamic
  {
      var l: dynamic = (*lower_bound([x, is_query]));
      return ((l.m * x) + l.b);
    }
}

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var sz: dynamic = cpp_array(N);

var big: dynamic = cpp_array(N);

var dead: dynamic = cpp_array(N);

var nei: dynamic = cpp_array(N);

func input() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      nei[cpp_update(u, "--")].push_back(cpp_update(v, "--"));
      nei[v].push_back(u);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
}

func dfs(v: dynamic, par: dynamic, len: dynamic, sigma1: dynamic, sigma2: dynamic, sum: dynamic, vec: dynamic) -> dynamic
{
  vec->push_back(pair(pair(sum, len), pair(sigma1, sigma2)));
  for (var u: dynamic in nei[v])
  {
    if (((dead[u] == false) && (u != par)))
    {
      dfs(u, v, (len + 1), (sigma1 + (((len + 1)) * a[u])), (sigma2 + ((sum + a[u]))), (sum + a[u]), vec);
    }
  }
}

func go(v: dynamic, par: dynamic = -1) -> dynamic
{
  sz[v] = 1;
  big[v] = -1;
  for (var u: dynamic in nei[v])
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

func get_cen(v: dynamic) -> dynamic
{
  go(v);
  var n: dynamic = sz[v];
  while (((big[v] != -1) && ((2 * sz[big[v]]) > n)))
  {
    v = big[v];
  }
  return v;
}

func solve(v: dynamic = 0) -> dynamic
{
  var cen: dynamic = get_cen(v);
  dead[cen] = true;
  var data: dynamic = cpp_uninitialized();
  data.insert_line(1, a[cen]);
  for (var u: dynamic in nei[cen])
  {
    if ((dead[u] == false))
    {
      solve(u);
      var vec: dynamic = cpp_uninitialized();
      dfs(u, cen, 1, a[u], a[u], a[u], (&vec));
      for (var p: dynamic in vec)
      {
        ans = max(ans, (p.second.first + data.eval(p.first.first)));
        ans = max(ans, ((((p.first.second + 1)) * a[cen]) + p.second.second));
      }
      for (var p: dynamic in vec)
      {
        data.insert_line((p.first.second + 1), (p.second.second + (((p.first.second + 1)) * a[cen])));
      }
    }
  }
  data.clear();
  data.insert_line(1, a[cen]);
  reverse(nei[cen].begin(), nei[cen].end());
  for (var u: dynamic in nei[cen])
  {
    if ((dead[u] == false))
    {
      var vec: dynamic = cpp_uninitialized();
      dfs(u, cen, 1, a[u], a[u], a[u], (&vec));
      for (var p: dynamic in vec)
      {
        ans = max(ans, (p.second.first + data.eval(p.first.first)));
      }
      for (var p: dynamic in vec)
      {
        data.insert_line((p.first.second + 1), (p.second.second + (((p.first.second + 1)) * a[cen])));
      }
    }
  }
  dead[cen] = false;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  input();
  solve();
  write(ans, cpp_char("\n"));
}

func __cpp_lambda_1() -> dynamic
{
  return  ((next(y) == end())) ? 0 : (&(*next(y)));
}

func __cpp_lambda_2() -> dynamic
{
  return (&(*y));
}
