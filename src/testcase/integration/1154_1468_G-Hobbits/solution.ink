// Translated from solution.cpp.

var debug = cpp_expression("#include<bits/stdc++.h> using namespace");

var pb = cpp_expression("#include<");

var mk = cpp_expression("#include<");

var ins = cpp_expression("#inclu");

var era = cpp_expression("#incl");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func lowbit(x: dynamic)
{
  return cpp_expression("#inc");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include<bits/std");
}

var INF = 0x3f3f3f3f;

var INFLL = 0x3f3f3f3f3f3f3f3f;

var PI = acos(-1.0);

func chkmin(a: dynamic, b: dynamic)
{
  return if ((b < a)) cpp_comma(cpp_assign(a, "=", b), true) else false;
}

func chkmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), true) else false;
}

var w: dynamic;

var t: dynamic;

var f: dynamic;

func slo(na: dynamic, nb: dynamic)
{
  if ((na.fi == nb.fi))
  {
    return cpp_cast(INF);
  } else
  {
    return ((db)((nb.se - na.se)) / ((nb.fi - na.fi)));
  }
}

func sq(u: dynamic)
{
  return (u * u);
}

func dis(na: dynamic, nb: dynamic)
{
  return sqrt((sq((nb.fi - na.fi)) + sq((nb.se - na.se))));
}

func its(naa: dynamic, nab: dynamic, nba: dynamic, nbb: dynamic)
{
  var l = naa.fi;
  var r = nab.fi;
  while (((r - l) > 1e-10))
  {
    var mid = (((l + r)) / 2);
    if ((slo(mk(mid, (naa.se + (((mid - naa.fi)) * slo(naa, nab)))), nba) > slo(nba, nbb)))
    {
      r = mid;
    } else
    {
      l = mid;
    }
  }
  return mk(l, (naa.se + (((l - naa.fi)) * slo(naa, nab))));
}

var N = (2e5 + 5);

var n: dynamic;

var h: dynamic;

var nd = cpp_array(N);

var ans: dynamic;

func solve()
{
  scanf("%d%d", (&n), (&h));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lf%lf", (&nd[i].fi), (&nd[i].se));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      ans += dis(nd[i], nd[(i + 1)]);
      i += 1;
    }
  }
  nd[(n + 1)] = mk(nd[n].fi, (nd[n].se + h));
  var pos = n;
  var ite = n;
  while ((pos > 1))
  {
    while (((pos > 1) && (slo(nd[(pos - 1)], nd[pos]) <= slo(nd[pos], nd[(n + 1)]))))
    {
      pos -= 1;
    }
    ite = (pos - 1);
    while (((ite > 0) && (slo(nd[ite], nd[pos]) > slo(nd[pos], nd[(n + 1)]))))
    {
      ans -= dis(nd[ite], nd[(ite + 1)]);
      ite -= 1;
    }
    if ((!ite))
    {
      break;
    }
    ans -= dis(its(nd[ite], nd[(ite + 1)], nd[pos], nd[(n + 1)]), nd[(ite + 1)]);
    pos = ite;
  }
  printf("%.10lf\n", ans);
}

func main()
{
  w = scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
  }
  solve();
  return 0;
}
