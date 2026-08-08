// Translated from solution.cpp.

var k: dynamic;

var n: dynamic;

var m: dynamic;

var aa = cpp_array(1000010);

var f = cpp_array(1000010);

func dfs(v: dynamic)
{
  f[v] = true;
  var res = 1;
  {
    var i = 0;
    while ((i < int_cpp(aa[v].size())))
    {
      var a = aa[v][i];
      if ((!f[a]))
      {
        res += dfs(a);
      }
      i += 1;
    }
  }
  return res;
}

func main()
{
  read(n, m, k);
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      aa[x].push_back(y);
      aa[y].push_back(x);
      i += 1;
    }
  }
  var s = 2;
  var q = 0;
  {
    var i = 1;
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
