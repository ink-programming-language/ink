// Translated from solution.cpp.

var INF: dynamic = 1e9;

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(t: dynamic) -> dynamic
{
  write(t);
}

func print(p: dynamic) -> dynamic
{
  write("{");
  print(p.first);
  write(",");
  print(p.second);
  write("}");
}

func print(v: dynamic) -> dynamic
{
  write("[ ");
  for (var i: dynamic in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic) -> dynamic
{
  write("[ ");
  for (var i: dynamic in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic) -> dynamic
{
  write("[ ");
  for (var i: dynamic in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic) -> dynamic
{
  write("[ ");
  for (var i: dynamic in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((((i % 2) == 0) && (s[i] == cpp_char(")"))))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            if ((s[j] == cpp_char("(")))
            {
              ans.push_back([(i + 1), (j + 1)]);
              var t: dynamic = s.substr(i, ((j - i) + 1));
              reverse(t.begin(), t.end());
              s.replace(i, ((j - i) + 1), t);
              break;
            }
            j += 1;
          }
        }
      } else if ((((i % 2) == 1) && (s[i] == cpp_char("("))))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            if ((s[j] == cpp_char(")")))
            {
              ans.push_back([(i + 1), (j + 1)]);
              var t: dynamic = s.substr(i, ((j - i) + 1));
              reverse(t.begin(), t.end());
              s.replace(i, ((j - i) + 1), t);
              break;
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  var count: dynamic = (n / 2);
  var val: dynamic = cpp_construct((n + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      val[(i + 1)] = (val[i] + ( ((s[i] == cpp_char("("))) ? 1 : -1));
      i += 1;
    }
  }
  if ((count > k))
  {
    count -= k;
    {
      var i: dynamic = 1;
      while (((i <= n) && count))
      {
        if ((val[i] == 0))
        {
          ans.push_back([i, (i + 1)]);
          i += 1;
          count -= 1;
        }
        i += 1;
      }
    }
  }
  write(ans.size(), "\n");
  for (var x: dynamic in ans)
  {
    write(x.first, " ", x.second, "\n");
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
