// Translated from solution.cpp.

var INF = 1e9;

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(t: dynamic)
{
  write(t);
}

func print(p: dynamic)
{
  write("{");
  print(p.first);
  write(",");
  print(p.second);
  write("}");
}

func print(v: dynamic)
{
  write("[ ");
  for (var i in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic)
{
  write("[ ");
  for (var i in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic)
{
  write("[ ");
  for (var i in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func print(v: dynamic)
{
  write("[ ");
  for (var i in v)
  {
    print(i);
    write(" ");
  }
  write("]");
}

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var s: dynamic;
  read(s);
  var ans: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((i % 2) == 0) && (s[i] == cpp_char(")"))))
      {
        {
          var j = (i + 1);
          while ((j < n))
          {
            if ((s[j] == cpp_char("(")))
            {
              ans.push_back([(i + 1), (j + 1)]);
              var t = s.substr(i, ((j - i) + 1));
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
          var j = (i + 1);
          while ((j < n))
          {
            if ((s[j] == cpp_char(")")))
            {
              ans.push_back([(i + 1), (j + 1)]);
              var t = s.substr(i, ((j - i) + 1));
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
  var count = (n / 2);
  var val = cpp_construct((n + 1), 0);
  {
    var i = 0;
    while ((i < n))
    {
      val[(i + 1)] = (val[i] + (if ((s[i] == cpp_char("("))) 1 else -1));
      i += 1;
    }
  }
  if ((count > k))
  {
    count -= k;
    {
      var i = 1;
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
  for (var x in ans)
  {
    write(x.first, " ", x.second, "\n");
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
