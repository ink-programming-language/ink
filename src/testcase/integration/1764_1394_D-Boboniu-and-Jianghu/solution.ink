// Translated from solution.cpp.

var inf: dynamic = 1e12;

var MAXN: dynamic = 500005;

var gph: dynamic = cpp_array(MAXN);

var up: dynamic = cpp_array(MAXN);

var dn: dynamic = cpp_array(MAXN);

var t: dynamic = cpp_array(MAXN);

var h: dynamic = cpp_array(MAXN);

func dfs(x: dynamic, p: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var tot: dynamic = 0;
  var sum: dynamic = 0;
  for (var i: dynamic in gph[x])
  {
    if ((i != p))
    {
      dfs(i, x);
      if ((h[i] > h[x]))
      {
        up[i] = (-inf);
      }
      if ((h[i] < h[x]))
      {
        dn[i] = (-inf);
      }
      v.emplace_back(up[i], dn[i]);
      sum += up[i];
    }
  }
  sort((v).begin(), (v).end(), __cpp_lambda_1);
  up[x] = cpp_assign(dn[x], "=", (-inf));
  {
    var foo: dynamic = sum;
    var in_cpp: dynamic = ((cpp_cast((gph[x]).size())) - 1);
    var out: dynamic = 1;
    up[x] = max(up[x], (foo + (min(in_cpp, out) * t[x])));
    for (var i: dynamic in v)
    {
      foo += (i.second - i.first);
      in_cpp -= 1;
      out += 1;
      up[x] = max(up[x], (foo + (min(in_cpp, out) * t[x])));
    }
  }
  {
    var foo: dynamic = sum;
    var in_cpp: dynamic = (cpp_cast((gph[x]).size()));
    var out: dynamic = 0;
    dn[x] = max(dn[x], (foo + (min(in_cpp, out) * t[x])));
    for (var i: dynamic in v)
    {
      foo += (i.second - i.first);
      in_cpp -= 1;
      out += 1;
      dn[x] = max(dn[x], (foo + (min(in_cpp, out) * t[x])));
    }
  }
}

func solve() -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var x: dynamic = 1;
  for (var i: dynamic in gph[1])
  {
    dfs(i, 1);
    if ((h[i] > h[x]))
    {
      up[i] = (-inf);
    }
    if ((h[i] < h[x]))
    {
      dn[i] = (-inf);
    }
    v.emplace_back(up[i], dn[i]);
    sum += up[i];
  }
  sort((v).begin(), (v).end(), __cpp_lambda_2);
  var foo: dynamic = sum;
  var in_cpp: dynamic = (cpp_cast((gph[x]).size()));
  var out: dynamic = 0;
  var dap: dynamic = (-inf);
  dap = max(dap, (foo + (min(in_cpp, out) * t[x])));
  for (var i: dynamic in v)
  {
    foo += (i.second - i.first);
    in_cpp -= 1;
    out += 1;
    dap = max(dap, (foo + (min(in_cpp, out) * t[x])));
  }
  return dap;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&t[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&h[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d", (&u), (&v));
      gph[u].push_back(v);
      gph[v].push_back(u);
      i += 1;
    }
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ret += ((1 * (cpp_cast((gph[i]).size()))) * t[i]);
      i += 1;
    }
  }
  write((ret - solve()), "\n");
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.second - a.first) > (b.second - b.first));
}

func __cpp_lambda_2(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.second - a.first) > (b.second - b.first));
}
