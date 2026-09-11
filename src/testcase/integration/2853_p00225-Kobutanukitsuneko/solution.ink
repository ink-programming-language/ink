// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(10000);

var par: dynamic = cpp_array(26);

var sz: dynamic = cpp_array(26);

func init() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      par[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
}

func find(x: dynamic) -> dynamic
{
  return  ((x == par[x])) ? x : cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic) -> dynamic
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

func solve() -> dynamic
{
  var in_cpp: dynamic = [0];
  var out: dynamic = [0];
  init();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var o: dynamic = (s[i][0] - cpp_char("a"));
      var e: dynamic = (s[i][(s[i].size() - 1)] - cpp_char("a"));
      in_cpp[o] += 1;
      out[e] += 1;
      unite(o, e);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  cin.tie();
  ios_base.sync_with_stdio(false);
  while (cpp_comma((cin >> N), N))
  {
    {
      var i: dynamic = 0;
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
