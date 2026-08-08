// Translated from solution.cpp.

var N: dynamic;

var s = cpp_array(10000);

var par = cpp_array(26);

var sz = cpp_array(26);

func init()
{
  {
    var i = 0;
    while ((i < 26))
    {
      par[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
}

func find(x: dynamic)
{
  return if ((x == par[x])) x else cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic)
{
  x = find(x);
  y = find(y);
  if ((x == y))
  {
    return;
  }
  if ((sz[x] < sz[y]))
  {
    swap(x, y);
  }
  par[y] = x;
  sz[x] += sz[y];
}

func solve()
{
  var in_cpp = [0];
  var out = [0];
  init();
  {
    var i = 0;
    while ((i < N))
    {
      var o = (s[i][0] - cpp_char("a"));
      var e = (s[i][(s[i].size() - 1)] - cpp_char("a"));
      in_cpp[o] += 1;
      out[e] += 1;
      unite(o, e);
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = 0;
    while ((i < 26))
    {
      if ((in_cpp[i] != out[i]))
      {
        write("NG", "\n");
        return;
      }
      if ((in_cpp[i] && (i == par[i])))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  if ((cnt > 1))
  {
    write("NG", "\n");
    return;
  }
  write("OK", "\n");
}

func main()
{
  cin.tie();
  ios_base.sync_with_stdio(false);
  while (cpp_comma((cin >> N), N))
  {
    {
      var i = 0;
      while ((i < N))
      {
        read(s[i]);
        i += 1;
      }
    }
    solve();
  }
  return 0;
}
