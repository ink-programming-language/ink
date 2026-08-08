// Translated from solution.cpp.

var inf = 1e12;

var MAXN = 500005;

var gph = cpp_array(MAXN);

var up = cpp_array(MAXN);

var dn = cpp_array(MAXN);

var t = cpp_array(MAXN);

var h = cpp_array(MAXN);

func dfs(x: dynamic, p: dynamic)
{
  var v: dynamic;
  var tot = 0;
  var sum = 0;
  for (var i in gph[x])
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
    var foo = sum;
    var in_cpp = ((cpp_cast((gph[x]).size())) - 1);
    var out = 1;
    up[x] = max(up[x], (foo + (min(in_cpp, out) * t[x])));
    for (var i in v)
    {
      foo += (i.second - i.first);
      in_cpp -= 1;
      out += 1;
      up[x] = max(up[x], (foo + (min(in_cpp, out) * t[x])));
    }
  }
  {
    var foo = sum;
    var in_cpp = (cpp_cast((gph[x]).size()));
    var out = 0;
    dn[x] = max(dn[x], (foo + (min(in_cpp, out) * t[x])));
    for (var i in v)
    {
      foo += (i.second - i.first);
      in_cpp -= 1;
      out += 1;
      dn[x] = max(dn[x], (foo + (min(in_cpp, out) * t[x])));
    }
  }
}

func solve()
{
  var v: dynamic;
  var sum = 0;
  var x = 1;
  for (var i in gph[1])
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
  var foo = sum;
  var in_cpp = (cpp_cast((gph[x]).size()));
  var out = 0;
  var dap = (-inf);
  dap = max(dap, (foo + (min(in_cpp, out) * t[x])));
  for (var i in v)
  {
    foo += (i.second - i.first);
    in_cpp -= 1;
    out += 1;
    dap = max(dap, (foo + (min(in_cpp, out) * t[x])));
  }
  return dap;
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&t[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&h[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d", (&u), (&v));
      gph[u].push_back(v);
      gph[v].push_back(u);
      i += 1;
    }
  }
  var ret = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ret += ((1 * (cpp_cast((gph[i]).size()))) * t[i]);
      i += 1;
    }
  }
  write((ret - solve()), "\n");
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return ((a.second - a.first) > (b.second - b.first));
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return ((a.second - a.first) > (b.second - b.first));
}
