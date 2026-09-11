// Translated from solution.cpp.

var grundy: dynamic = cpp_array(101, 201);

func dfs(w: dynamic, b: dynamic) -> dynamic
{
  if ((grundy[w][b] >= 0))
  {
    return grundy[w][b];
  }
  var st: dynamic = cpp_uninitialized();
  if ((w > 0))
  {
    st.insert(dfs((w - 1), b));
  }
  if ((b > 0))
  {
    st.insert(dfs((w + 1), (b - 1)));
  }
  {
    var i: dynamic = 1;
    while (((i <= b) && (i <= w)))
    {
      st.insert(dfs(w, (b - i)));
      i += 1;
    }
  }
  var res: dynamic = 0;
  while (st.count(res))
  {
    res += 1;
  }
  return cpp_assign(grundy[w][b], "=", res);
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  fill(grundy[0], grundy[201], -1);
  grundy[0][0] = 0;
  var n: dynamic = cpp_uninitialized();
  read(n);
  var v: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var w: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(w, b);
      v ^= dfs(w, b);
      i += 1;
    }
  }
  if ((v == 0))
  {
    write(1, "\n");
  } else
  {
    write(0, "\n");
  }
  return 0;
}
