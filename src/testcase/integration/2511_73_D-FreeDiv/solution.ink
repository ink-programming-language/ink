// Translated from solution.cpp.

var k: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var aa: dynamic = cpp_array(1000010);

var f: dynamic = cpp_array(1000010);

func dfs(v: dynamic) -> dynamic
{
  f[v] = true;
  var res: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < int_cpp(aa[v].size())))
    {
      var a: dynamic = aa[v][i];
      if ((!f[a]))
      {
        res += dfs(a);
      }
      i += 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  read(n, m, k);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      aa[x].push_back(y);
      aa[y].push_back(x);
      i += 1;
    }
  }
  var s: dynamic = 2;
  var q: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!f[i]))
      {
        s += (min(dfs(i), k) - 2);
        q += 1;
      }
      i += 1;
    }
  }
  if ((k == 1))
  {
    write(max((q - 2), 0));
  } else if ((s >= 0))
  {
    write(0);
  } else
  {
    write(((((-s) + 1)) / 2));
  }
}
