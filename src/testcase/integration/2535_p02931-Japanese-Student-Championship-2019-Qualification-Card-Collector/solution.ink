// Translated from solution.cpp.

func REP(i: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(int)(m) ; i < (int) (n) ; ++i )");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<i");
}

var inf = (1e9 + 7);

var longinf = (1 << 60);

var mod = (1e9 + 7);

class UnionFind
{
  var par: dynamic;
  var cnt: dynamic;
  func UnionFind(n: dynamic)
  {
      this->par = cpp_construct(n, -1);
      this->cnt = cpp_construct(n);
    }
  func find(x: dynamic)
  {
      if ((par[x] < 0))
      {
        return x;
      }
      return cpp_assign(par[x], "=", find(par[x]));
    }
  func unite(x: dynamic, y: dynamic)
  {
      x = find(x);
      y = find(y);
      var ok = ((((-par[x]) > cnt[x]) || ((-par[y]) > cnt[y])));
      if ((x == y))
      {
        cnt[x] += 1;
      } else if ((par[x] > par[y]))
      {
        par[y] += par[x];
        par[x] = y;
        cnt[y] += (cnt[x] + 1);
      } else
      {
        par[x] += par[y];
        par[y] = x;
        cnt[x] += (cnt[y] + 1);
      }
      return ok;
    }
  func same(x: dynamic, y: dynamic)
  {
      return (find(x) == find(y));
    }
  func size(x: dynamic)
  {
      return (-par[find(x)]);
    }
}

func main()
{
  var n: dynamic;
  var h: dynamic;
  var w: dynamic;
  read(n, h, w);
  var a = cpp_array(n);
  var b = cpp_array(n);
  var c = cpp_array(n);
  rep(i, n);
  read(a[i], b[i], c[i]);
  rep(i, n)[i] = i;
  sort(ord.begin(), ord.end(), __cpp_lambda_1);
  var ans = 0;
  var uf = cpp_construct((h + w));
  for (var i in ord)
  {
    a[i] -= 1;
    b[i] -= 1;
    if (uf.unite(a[i], (b[i] + h)))
    {
      ans += c[i];
    }
  }
  write(ans, "\n");
  return 0;
}

func __cpp_lambda_1(x: dynamic, y: dynamic)
{
  return (c[x] > c[y]);
}
