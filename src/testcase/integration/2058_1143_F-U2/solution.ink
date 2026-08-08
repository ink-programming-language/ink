// Translated from solution.cpp.

var MAXN = (1e5 + 7);

var p = cpp_array(MAXN);

var n: dynamic;

var res = 0;

func cw(a: dynamic, b: dynamic, c: dynamic)
{
  return (((((b.first - a.first)) * ((c.second - b.second))) - (((c.first - b.first)) * ((b.second - a.second)))) < 0);
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(p[i].first, p[i].second);
      p[i].second -= (p[i].first * p[i].first);
      i += 1;
    }
  }
  sort((p + 1), ((p + 1) + n));
  var hull: dynamic;
  hull.push_back(p[1]);
  {
    var i = 2;
    while ((i <= n))
    {
      if ((cw(p[1], p[i], p[n]) || (i == n)))
      {
        while (((cpp_cast(hull.size()) > 1) && (!cw(hull[(cpp_cast(hull.size()) - 2)], hull.back(), p[i]))))
        {
          hull.pop_back();
        }
        hull.push_back(p[i]);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < cpp_cast(hull.size())))
    {
      if ((hull[i].first != hull[(i - 1)].first))
      {
        res += 1;
      }
      i += 1;
    }
  }
  write(res);
  return 0;
}
